/* Main emulation thread only. Observer state never changes guest state.
 * Samples cover an OUTER scheduler iteration including any nested AOT/LLE.
 * Nested runs cannot start/end samples or change outer phase markers. */
#include "lufia2_bridge_audit.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PC_SLOTS 2048
typedef struct Stats {
    uint64_t samples, ns[L2BA_PHASE_COUNT], hits[L2BA_PHASE_COUNT];
} Stats;
typedef struct PcStats {
    uint32_t pc;
    unsigned mx, used;
    Stats stats;
} PcStats;
static const char *s_names[L2BA_PHASE_COUNT] = {
    "checks", "quiescence", "dispatch_prep", "opcode_excl_bus",
    "bridge_bus_excl_children", "retire_accounting", "snes_sync",
    "cart_sync", "apu_flush_other", "apu_lock_acquire", "apu_sync",
    "state_handoff", "aot_inclusive_uninstrumented_children", "tail"
};
static const char *s_groups[3] = {"83900E", "839010", "other_outer_pc"};
int l2ba_sample_active, l2ba_recording;
static Stats s_stats[3];
static PcStats s_pcs[PC_SLOTS];
static uint64_t s_seen[3], s_parts[L2BA_PHASE_COUNT];
static uint64_t s_last, s_start, s_clock_reads, s_frame_start, s_frame_ns;
static uint64_t s_measured_total, s_pc_overflow;
static unsigned s_group, s_mx, s_frames, s_warmup, s_session, s_countdown;
static uint32_t s_pc, s_random = 0x9251a6bdu;
static int s_phase, s_fake_clock;
static uint64_t s_fake_now;

static uint64_t now_ns(void) {
    ++s_clock_reads;
    return s_fake_clock ? s_fake_now : SDL_GetTicksNS();
}
static void charge(uint64_t now) {
    s_parts[s_phase] += now - s_last;
    s_last = now;
}
void L2BAMark(int phase) {
    if (!l2ba_sample_active) return;
    charge(now_ns());
    s_phase = phase;
}
int L2BAEnter(int phase) {
    if (!l2ba_sample_active) return -1;
    int previous = s_phase;
    L2BAMark(phase);
    return previous;
}
void L2BALeave(int previous) {
    if (previous >= 0) L2BAMark(previous);
}
static void accumulate(Stats *stats) {
    ++stats->samples;
    for (unsigned j=0;j<L2BA_PHASE_COUNT;++j) {
        stats->ns[j] += s_parts[j];
        if (s_parts[j]) ++stats->hits[j];
    }
}
void L2BAFinish(void) {
    if (!l2ba_sample_active) return;
    uint64_t end = now_ns();
    charge(end);
    s_measured_total += end - s_start;
    accumulate(&s_stats[s_group]);
    unsigned slot = (s_pc ^ (s_pc >> 11) ^ (s_mx * 131u)) & (PC_SLOTS-1);
    for (unsigned n=0;n<PC_SLOTS;++n) {
        PcStats *p = &s_pcs[(slot+n)&(PC_SLOTS-1)];
        if (!p->used || (p->pc==s_pc && p->mx==s_mx)) {
            p->used=1; p->pc=s_pc; p->mx=s_mx;
            accumulate(&p->stats);
            l2ba_sample_active=0;
            return;
        }
    }
    ++s_pc_overflow; /* Aggregate phase totals remain complete. */
    l2ba_sample_active=0;
}
void L2BALoop(uint32_t pc, unsigned mx, int outer_scheduler) {
    if (!outer_scheduler) return;
    L2BAFinish();
    if (!l2ba_recording) return;
    unsigned group = pc==0x83900e ? 0 : pc==0x839010 ? 1 : 2;
    ++s_seen[group];
    if (s_countdown) { --s_countdown; return; }
    /* Randomized spacing prevents locking onto CMP or BEQ parity. Mean
     * interval is 256.5 iterations; no RNG or clock on skipped iterations. */
    s_random ^= s_random << 13;
    s_random ^= s_random >> 17;
    s_random ^= s_random << 5;
    s_countdown = 128u + (s_random & 255u);
    s_group=group; s_pc=pc; s_mx=mx;
    memset(s_parts,0,sizeof(s_parts));
    s_phase=L2BA_CHECKS;
    s_last=s_start=now_ns();
    l2ba_sample_active=1;
}
static uint64_t total(const Stats *s) {
    uint64_t ns=0;
    for (unsigned j=0;j<L2BA_PHASE_COUNT;++j) ns += s->ns[j];
    return ns;
}
static void report(void) {
    uint64_t partition=0, samples=0;
    double estimated[L2BA_PHASE_COUNT]={0};
    fprintf(stderr,"[bridge-audit] result session=%u boundaries=%u scheduler_ms=%.5f clock_reads=%llu pc_overflow=%llu\n",
        s_session,s_frames,(double)s_frame_ns/(s_frames*1e6),
        (unsigned long long)s_clock_reads,(unsigned long long)s_pc_overflow);
    for (unsigned g=0;g<3;++g) {
        Stats *s=&s_stats[g]; partition+=total(s); samples+=s->samples;
        fprintf(stderr,"[bridge-audit] group=%s seen=%llu samples=%llu\n",s_groups[g],
            (unsigned long long)s_seen[g],(unsigned long long)s->samples);
        if (!s->samples) continue;
        for (unsigned j=0;j<L2BA_PHASE_COUNT;++j) {
            double ns=(double)s->ns[j]/s->samples;
            double ms=ns*(double)s_seen[g]/(s_frames*1e6);
            estimated[j]+=ms;
            fprintf(stderr,"[bridge-audit] group=%s phase=%s sampled_ns_per_iteration=%.3f estimated_ms_per_boundary=%.6f nonzero_samples=%llu\n",
                s_groups[g],s_names[j],ns,ms,(unsigned long long)s->hits[j]);
        }
    }
    unsigned order[L2BA_PHASE_COUNT];
    for(unsigned i=0;i<L2BA_PHASE_COUNT;++i) order[i]=i;
    for(unsigned i=1;i<L2BA_PHASE_COUNT;++i) {
        unsigned key=order[i], j=i;
        while(j && estimated[order[j-1]]<estimated[key]) {order[j]=order[j-1];--j;}
        order[j]=key;
    }
    for(unsigned i=0;i<L2BA_PHASE_COUNT;++i)
        fprintf(stderr,"[bridge-audit] rank=%u phase=%s estimated_ms_per_boundary=%.6f\n",
            i+1,s_names[order[i]],estimated[order[i]]);
    fprintf(stderr,"[bridge-audit] accounting=%s samples=%llu sampled_elapsed_ns=%llu exclusive_sum_ns=%llu estimates=unadjusted_sampling clock_overhead=included\n",
        partition==s_measured_total ? "PASS":"FAIL",(unsigned long long)samples,
        (unsigned long long)s_measured_total,(unsigned long long)partition);
    /* Sort indices, never mutate the guest or allocate in the hot loop. */
    unsigned top[16], count=0;
    for (unsigned i=0;i<PC_SLOTS;++i) if(s_pcs[i].used) {
        unsigned k=0;
        while(k<count && total(&s_pcs[top[k]].stats)>=total(&s_pcs[i].stats)) ++k;
        if(k>=16) continue;
        if(count<16) ++count;
        for(unsigned j=count-1;j>k;--j) top[j]=top[j-1];
        top[k]=i;
    }
    for(unsigned i=0;i<count;++i) {
        PcStats *p=&s_pcs[top[i]];
        fprintf(stderr,"[bridge-audit] top_pc=%06X m=%u x=%u e=%u samples=%llu sampled_total_ns=%llu sampled_mean_ns=%.3f attribution=outer_iteration\n",
            p->pc,p->mx&1u,(p->mx>>1)&1u,(p->mx>>2)&1u,
            (unsigned long long)p->stats.samples,(unsigned long long)total(&p->stats),
            (double)total(&p->stats)/p->stats.samples);
    }
}
void L2BAToggle(void) {
    const char *disabled=getenv("LUFIA2_BRIDGE_AUDIT");
    if(disabled && !strcmp(disabled,"0")) {
        fprintf(stderr,"[bridge-audit] disabled by environment; timing-control run\n"); return;
    }
    if(s_warmup || l2ba_recording) {
        fprintf(stderr,"[bridge-audit] already armed; let the finite window finish\n"); return;
    }
    memset(s_stats,0,sizeof(s_stats)); memset(s_seen,0,sizeof(s_seen));
    memset(s_pcs,0,sizeof(s_pcs));
    s_frames=0; s_frame_ns=0; s_clock_reads=0; s_pc_overflow=0; s_measured_total=0;
    s_countdown=128; s_warmup=120; ++s_session;
    uint64_t sum=0, minimum=UINT64_MAX;
    for(unsigned i=0;i<256;++i) {
        uint64_t a=SDL_GetTicksNS(), b=SDL_GetTicksNS(), d=b-a;
        sum+=d; if(d<minimum) minimum=d;
    }
    fprintf(stderr,"[bridge-audit] armed session=%u warmup=120 boundaries=600 scope=outer_scheduler nominal_sample_interval=256.5 clock_pair_min_ns=%llu clock_pair_mean_ns=%.3f\n",
        s_session,(unsigned long long)minimum,(double)sum/256.0);
}
void L2BAFrameBegin(void) {
    if(l2ba_recording) s_frame_start=SDL_GetTicksNS();
}
void L2BAFrameEnd(void) {
    if(s_warmup) {
        if(!--s_warmup) {
            l2ba_recording=1;
            fprintf(stderr,"[bridge-audit] recording session=%u; keep scene steady\n",s_session);
        }
        return;
    }
    if(!l2ba_recording) return;
    L2BAFinish();
    s_frame_ns+=SDL_GetTicksNS()-s_frame_start;
    if(++s_frames==600) {
        l2ba_recording=0;
        report();
        fprintf(stderr,"[bridge-audit] complete session=%u; Ctrl+Shift+F9 starts another window\n",s_session);
    }
}
int L2BASelfTest(void) {
    /* Deterministic observer-only test before game initialization. Nested
     * scopes must partition elapsed time, restore parents, and stay inert
     * outside a sample. The process exits immediately after this test. */
    s_fake_clock=1; s_fake_now=100; s_last=s_start=100;
    s_group=0; s_pc=0x83900e; s_phase=L2BA_CHECKS; l2ba_sample_active=1;
    s_fake_now=110; L2BAMark(L2BA_QUIESCENCE);
    s_fake_now=140; int q=L2BAEnter(L2BA_BUS);
    s_fake_now=160; int b=L2BAEnter(L2BA_APU_LOCK);
    s_fake_now=170; L2BALeave(b);
    s_fake_now=190; L2BALeave(q);
    s_fake_now=200; L2BAMark(L2BA_TAIL);
    L2BALoop(0x123456,0,0); /* nested bridge must not finish outer sample */
    int ok=l2ba_sample_active;
    s_fake_now=220; L2BAFinish();
    Stats *s=&s_stats[0];
    ok=ok && s->samples==1 && s->ns[L2BA_CHECKS]==10 &&
        s->ns[L2BA_QUIESCENCE]==40 && s->ns[L2BA_BUS]==40 &&
        s->ns[L2BA_APU_LOCK]==10 && s->ns[L2BA_TAIL]==20 &&
        total(s)==120 && s_measured_total==120 && !l2ba_sample_active;
    uint64_t reads=s_clock_reads;
    L2BALoop(0x83900e,3,1); L2BAMark(L2BA_BUS); L2BALeave(L2BAEnter(L2BA_AOT));
    ok=ok && s_clock_reads==reads && !l2ba_sample_active;
    fprintf(stderr,"[bridge-audit-test] %s nested_scopes=checked partition=checked disabled_clock=checked devices=not-tested\n",ok?"PASS":"FAIL");
    return ok?0:1;
}
