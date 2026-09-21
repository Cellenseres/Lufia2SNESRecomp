/* Observational, bounded desktop capture. No ROM patches or guest bus reads
 * for diagnostics. Buffers are written only after recording ends. */
#include "lufia2_gameplay_capture.h"
#include "common_rtl.h"
#include "snes/snes.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MAKE_DIR(p) mkdir(p, 0755)
#endif

enum { FRAMES = 900, CALLS = 48, RAM_SIZE = 0x20000,
       HITS = 16384, EDGES = 4096, BUS = 65536, WAITS = 6, EXAMPLES = 16,
       TARGETS = 16 };
extern Snes *g_snes;
extern int snes_frame_counter;
int lufia2_capture_active;
uint32_t lufia2_capture_pc;

typedef struct State {
    uint32_t pc;
    unsigned frame;
    uint16_t a, x, y, s, d, tick;
    uint8_t db, pb, p, e, open_bus, nmi, irq, hirq, virq;
    uint64_t cycles, master;
} State;
typedef struct Hit { uint32_t pc; unsigned mx; uint64_t count; } Hit;
typedef struct Edge {
    uint32_t site, target; unsigned mx, op; uint64_t count, missing, excluded;
} Edge;
typedef struct BusEvent { uint32_t pc, address; unsigned value, write, width, call; } BusEvent;
typedef struct Call {
    State entry, exit;
    uint32_t site, target, expected_pc;
    unsigned op, frame, end_frame, complete, native_child, bus_first, bus_count;
    uint64_t begin_ns, duration_ns;
    const Interp816 *context;
    uint8_t *before, *after;
} Call;
typedef struct Wait {
    uint64_t count, equal, unequal, d_nonzero, wide;
    unsigned n_examples, equal_examples, unequal_examples, last_example_frame;
    State examples[EXAMPLES];
} Wait;
typedef struct Frame {
    uint32_t input;
    int guest_frame;
    uint64_t guest_ns, work_ns, master_start, master_end, insns;
    uint8_t tick_before, tick_after;
} Frame;
static struct {
    Hit hit[HITS]; Edge edge[EDGES]; BusEvent bus[BUS]; Call call[CALLS];
    Wait wait[WAITS]; Frame frame[FRAMES];
    unsigned frames, calls, buses, dropped, target_count[TARGETS], target_last[TARGETS];
    uint32_t target[TARGETS]; unsigned target_total;
    uint64_t actor_hits[2][256];
    unsigned actor_captured[2][256], actor_last_seen[2][256];
    uint64_t instructions, begin_ns, before_insns, nmi_writes, non_opcode_steps;
    State nmi_before[EXAMPLES], nmi_after[EXAMPLES];
    unsigned nmi_examples;
    int pending, failed, in_frame;
    char directory[256];
} s;
static const uint32_t wait_pc[WAITS] = {
    0x83900e, 0x869752, 0x868b4e, 0x85ec96, 0x848d4f, 0x808084
};
static uint32_t canonical(uint32_t p) {
    /* Normalize LoROM execution mirrors only in the ROM half. RAM remains
     * distinguishable: $80:0067 is not treated as immutable ROM. */
    if ((p & 0xffffu) >= 0x8000 && (p >> 16) < 0x40) p |= 0x800000;
    return p;
}
static void add_target(uint32_t pc) {
    pc = canonical(pc & 0xffffffu);
    for (unsigned n=0; n<s.target_total; ++n)
        if (s.target[n] == pc) return;
    if (s.target_total < TARGETS) s.target[s.target_total++] = pc;
}
static void parse_targets(const char *text) {
    if (!text || !*text) return;
    while (*text && s.target_total < TARGETS) {
        char *end = NULL;
        unsigned long value = strtoul(text, &end, 16);
        if (end == text) {
            while (*text && *text != ',') ++text;
        } else {
            add_target((uint32_t)value);
            text = end;
        }
        while (*text == ',' || *text == ';' || *text == ' ' || *text == '\t') ++text;
    }
}
static void configure_targets(void) {
    const char *configured = getenv("SNESRECOMP_DECOMP_CAPTURE_TARGETS");
    parse_targets(configured);
    if (!s.target_total)
        parse_targets("83C7F8,83D508,83C1B4,80E365,80E566");
}
static int target_index(uint32_t pc) {
    pc = canonical(pc);
    for (unsigned n=0; n<s.target_total; ++n)
        if (s.target[n] == pc) return (int)n;
    return -1;
}
static State state(const CpuState *c, const Interp816 *i, uint32_t pc) {
    State r = {0};
    r.pc = pc; r.frame = s.frames; r.a = i->a; r.x = i->x; r.y = i->y; r.s = i->sp;
    r.d = i->dp; r.db = i->db; r.pb = i->k; r.e = i->e;
    r.p = i->c | (i->z << 1) | (i->i << 2) | (i->d << 3) |
        (i->xf << 4) | (i->mf << 5) | (i->v << 6) | (i->n << 7);
    r.open_bus = c->open_bus; r.cycles = c->cycles; r.master = c->master_cycles;
    if (g_snes) {
        r.nmi = g_snes->nmiEnabled; r.irq = g_snes->inIrq;
        r.hirq = g_snes->hIrqEnabled; r.virq = g_snes->vIrqEnabled;
    }
    if ((uint32_t)i->dp + 0x41u < 0x2000u)
        r.tick = (uint16_t)(c->ram[i->dp + 0x40] | ((unsigned)c->ram[i->dp + 0x41] << 8));
    return r;
}
static void print_state(FILE *f, const State *r) {
    fprintf(f, "{\"pc\":%u,\"frame\":%u,\"a\":%u,\"x\":%u,\"y\":%u,\"s\":%u,\"d\":%u,"
        "\"db\":%u,\"pb\":%u,\"p\":%u,\"e\":%u,\"open_bus\":%u,"
        "\"nmi_enabled\":%u,\"irq_pending\":%u,\"hirq\":%u,\"virq\":%u,"
        "\"tick\":%u,\"cycles\":%llu,\"master\":%llu}",
        r->pc,r->frame,r->a,r->x,r->y,r->s,r->d,r->db,r->pb,r->p,r->e,r->open_bus,
        r->nmi,r->irq,r->hirq,r->virq,r->tick,
        (unsigned long long)r->cycles,(unsigned long long)r->master);
}
int L2CaptureBefore(const CpuState *c, const Interp816 *i, uint32_t pc, unsigned op) {
    (void)op;
    /* runOpcode can service an interrupt or idle without executing the
     * prefetched opcode. Do not classify such a step as a call or return. */
    if (i->stopped || (i->waiting && !i->nmiWanted && !i->irqWanted) ||
            i->nmiWanted || (!i->i && i->irqWanted)) {
        ++s.non_opcode_steps;
        return 0;
    }
    ++s.instructions;
    uint32_t key = canonical(pc);
    unsigned mx = (i->mf << 1) | i->xf;
    unsigned h = (key * 2654435761u + mx) & (HITS - 1);
    unsigned n;
    for (n = 0; n < HITS; ++n) {
        Hit *p = &s.hit[(h + n) & (HITS - 1)];
        if (!p->count) { p->pc = key; p->mx = mx; }
        if (p->pc == key && p->mx == mx) { ++p->count; break; }
    }
    if (n == HITS) ++s.dropped;
    for (n = 0; n < WAITS; ++n) if (key == wait_pc[n]) {
        Wait *w = &s.wait[n]; ++w->count;
        w->d_nonzero += i->dp != 0; w->wide += !i->mf;
        int sample = 0;
        if (n < 5 && (uint32_t)i->dp + 0x41 < 0x2000) {
            unsigned value = c->ram[i->dp + 0x40];
            if (!i->mf) value |= (unsigned)c->ram[i->dp + 0x41] << 8;
            unsigned a = i->a & (i->mf ? 255u : 65535u);
            w->equal += a == value; w->unequal += a != value;
            /* Reserve examples for both polling and release conditions;
             * thousands of equal comparisons must not fill the entire set. */
            if (a == value && w->equal_examples < EXAMPLES / 2 &&
                    (!w->equal_examples || s.frames - w->last_example_frame >= 30)) {
                ++w->equal_examples; sample = 1;
            } else if (a != value && w->unequal_examples < EXAMPLES / 2 &&
                    (!w->unequal_examples || s.frames != w->last_example_frame)) {
                ++w->unequal_examples; sample = 1;
            }
        } else if (!w->n_examples || s.frames - w->last_example_frame >= 30) {
            sample = 1;
        }
        if (sample && w->n_examples < EXAMPLES) {
            w->examples[w->n_examples++] = state(c, i, pc);
            w->last_example_frame = s.frames;
        }
    }
    if (key == 0x80869b && s.nmi_examples < EXAMPLES)
        s.nmi_before[s.nmi_examples] = state(c, i, pc);
    return 1;
}
void L2CaptureAfter(const CpuState *c, const Interp816 *i, uint32_t pc, unsigned op) {
    if (canonical(pc) == 0x80869b) {
        ++s.nmi_writes;
        if (s.nmi_examples < EXAMPLES)
            s.nmi_after[s.nmi_examples++] = state(c, i, ((uint32_t)i->k << 16) | i->pc);
    }
    if (s.pending < 0) return;
    Call *p = &s.call[s.pending];
    if (p->context != i || (op != 0x60 && op != 0x6b)) return;
    unsigned width = p->op == 0x22 ? 3 : 2;
    if (i->sp != (uint16_t)(p->entry.s + width)) return;
    p->exit = state(c, i, ((uint32_t)i->k << 16) | i->pc);
    p->complete = p->exit.pc == p->expected_pc ? 1 : 2;
    p->end_frame = s.frames; p->duration_ns = SDL_GetTicksNS() - p->begin_ns;
    memcpy(p->after, c->ram, RAM_SIZE);
    p->bus_count = s.buses - p->bus_first;
    s.pending = -1;
}
void L2CaptureCall(const CpuState *c, const Interp816 *i, uint32_t site,
                   uint32_t target, unsigned op, int body, int bounce) {
    uint32_t key = canonical(target);
    unsigned mx = (i->mf << 1) | i->xf;
    unsigned h = (site * 2654435761u ^ target ^ mx ^ op) & (EDGES - 1), n;
    for (n = 0; n < EDGES; ++n) {
        Edge *p = &s.edge[(h + n) & (EDGES - 1)];
        if (!p->count) { p->site=site; p->target=target; p->mx=mx; p->op=op; }
        if (p->site==site && p->target==target && p->mx==mx && p->op==op) {
            ++p->count; p->missing += !body; p->excluded += !bounce; break;
        }
    }
    if (n == EDGES) ++s.dropped;
    if (s.pending >= 0) {
        if (body && bounce) s.call[s.pending].native_child = 1;
        return;
    }
    const int tracked = target_index(key);
    int which = key == 0x83c7f8 ? 0 : key == 0x83d508 ? 1 : -1;
    unsigned actor = 0;
    if (tracked < 0 || body || i->e) return;

    /* Actor-target hit accounting must happen before snapshot throttling.
     * Otherwise actor_index describes only whichever slot happened to pass
     * the global time gate instead of the runtime actor distribution. */
    if (which >= 0) {
        if ((unsigned)i->dp + 0xa7u >= 0x2000u) return;
        actor = c->ram[i->dp + 0xa7u];
        ++s.actor_hits[which][actor];
        s.actor_last_seen[which][actor] = s.frames;
    }

    if (s.calls == CALLS || s.target_count[tracked] >= (which >= 0 ? 16u : 8u)) return;

    /* Generic targets are sampled over time. Actor targets instead use the
     * per-index balancing below so multiple distinct slots in one update can
     * be captured rather than always selecting the first slot every 30 frames. */
    if (which < 0 && s.target_count[tracked] &&
            s.frames - s.target_last[tracked] < 30)
        return;

    if (which >= 0) {
        unsigned least = s.actor_captured[which][actor];
        for (unsigned j=0; j<256; ++j)
            if (s.actor_hits[which][j] && s.frames-s.actor_last_seen[which][j] <= 30 &&
                    s.actor_captured[which][j] < least)
                least = s.actor_captured[which][j];
        if (s.actor_captured[which][actor] != least) return;
    }

    unsigned width = op == 0x22 ? 3 : 2;
    if ((unsigned)i->sp + width >= 0x2000) return; /* only native WRAM stack */
    Call *p = &s.call[s.calls];
    p->before = malloc(RAM_SIZE); p->after = calloc(1, RAM_SIZE);
    if (!p->before || !p->after) {
        free(p->before); free(p->after); p->before=p->after=NULL; ++s.dropped; return;
    }
    p->entry = state(c, i, target); p->site=site; p->target=target; p->op=op;
    p->context=i; p->frame=s.frames; p->bus_first=s.buses;
    uint16_t ret = (uint16_t)(c->ram[i->sp + 1] | ((unsigned)c->ram[i->sp + 2] << 8));
    unsigned bank = width == 3 ? c->ram[i->sp + 3] : i->k;
    p->expected_pc = (bank << 16) | (uint16_t)(ret + 1u);
    memcpy(p->before, c->ram, RAM_SIZE);
    p->begin_ns = SDL_GetTicksNS();
    s.pending = (int)s.calls++;
    ++s.target_count[tracked]; s.target_last[tracked]=s.frames;
    if (which >= 0) ++s.actor_captured[which][actor];
}
void L2CaptureBus(uint32_t address, unsigned value, int write) {
    if (s.pending < 0) return;
    if (s.buses == BUS) { ++s.dropped; return; }
    BusEvent *p = &s.bus[s.buses++];
    p->pc=lufia2_capture_pc; p->address=address; p->value=value;
    p->write=(unsigned)write & 1u; p->width=(write & 2) ? 2u : 1u; p->call=(unsigned)s.pending;
}
void L2CaptureAbort(void) {
    if (lufia2_capture_active) {
        s.failed=1;
        if (s.pending >= 0) s.call[s.pending].bus_count=s.buses-s.call[s.pending].bus_first;
        s.pending=-1;
    }
}

static FILE *open_output(const char *name, const char *mode) {
    char path[320]; snprintf(path, sizeof(path), "%s/%s", s.directory, name);
    FILE *f = fopen(path, mode);
    if (!f) fprintf(stderr, "[gameplay-capture] cannot write %s\n", path);
    return f;
}
static int close_output(FILE *f) {
    int bad = ferror(f); if (fclose(f) != 0) bad=1; return !bad;
}
static void stop(void) {
    lufia2_capture_active=0;
    if (s.pending >= 0) {
        s.call[s.pending].complete=0;
        s.call[s.pending].bus_count=s.buses-s.call[s.pending].bus_first;
        s.pending=-1;
    }
    int ok=1;
    FILE *f=open_output("frames.csv", "w");
    if (f) {
        fprintf(f,"seq,guest_frame,input,guest_ns,work_ns,master_start,master_end,lle_insns,tick_before,tick_after\n");
        for(unsigned n=0;n<s.frames;++n) {
            Frame *p=&s.frame[n];
            fprintf(f,"%u,%d,%u,%llu,%llu,%llu,%llu,%llu,%u,%u\n",n,p->guest_frame,p->input,
                (unsigned long long)p->guest_ns,(unsigned long long)p->work_ns,(unsigned long long)p->master_start,
                (unsigned long long)p->master_end,(unsigned long long)p->insns,p->tick_before,p->tick_after);
        } ok &= close_output(f);
    } else ok=0;
    f=open_output("calls.jsonl", "w");
    if(f) {
        for(unsigned n=0;n<s.calls;++n) {
            Call *p=&s.call[n];
            fprintf(f,"{\"id\":%u,\"site\":%u,\"target\":%u,\"op\":%u,\"frame\":%u,"
                "\"end_frame\":%u,\"complete\":%u,\"native_child\":%u,\"expected_pc\":%u,"
                "\"bus_first\":%u,\"bus_count\":%u,\"duration_ns\":%llu,\"entry\":",
                n,p->site,p->target,p->op,p->frame,p->end_frame,p->complete,p->native_child,
                p->expected_pc,p->bus_first,p->bus_count,(unsigned long long)p->duration_ns);
            print_state(f,&p->entry); fprintf(f,",\"exit\":"); print_state(f,&p->exit); fprintf(f,"}\n");
        } ok &= close_output(f);
    } else ok=0;
    for(unsigned n=0;n<s.calls;++n) {
        Call *p=&s.call[n]; char name[64];
        snprintf(name,sizeof(name),"call-%02u-before.wram",n);
        f=open_output(name,"wb");
        if(f) { ok &= fwrite(p->before,1,RAM_SIZE,f)==RAM_SIZE; ok &= close_output(f); } else ok=0;
        if(p->complete) {
            snprintf(name,sizeof(name),"call-%02u-after.wram",n);
            f=open_output(name,"wb");
            if(f) { ok &= fwrite(p->after,1,RAM_SIZE,f)==RAM_SIZE; ok &= close_output(f); } else ok=0;
        }
        free(p->before); free(p->after); p->before=p->after=NULL;
    }
    f=open_output("bus.csv","w");
    if(f) {
        fprintf(f,"call,pc,address,value,write,width\n");
        for(unsigned n=0;n<s.buses;++n) {
            BusEvent *p=&s.bus[n]; fprintf(f,"%u,%u,%u,%u,%u,%u\n",p->call,p->pc,p->address,p->value,p->write,p->width);
        } ok &= close_output(f);
    } else ok=0;
    f=open_output("summary.jsonl","w");
    if(f) {
        fprintf(f,"{\"schema\":\"lufia2 observational capture v2\",\"frames\":%u,"
            "\"instructions\":%llu,\"calls\":%u,\"dropped\":%u,\"aborted\":%d,\"prior_io_ok\":%d,\"nmi_tick_writes\":%llu,\"non_opcode_steps\":%llu,\"sampler\":\"configurable-function-targets-v2\"}\n",
            s.frames,(unsigned long long)s.instructions,s.calls,s.dropped,s.failed,ok,
            (unsigned long long)s.nmi_writes,(unsigned long long)s.non_opcode_steps);
        for (unsigned n=0; n<s.target_total; ++n)
            fprintf(f,"{\"kind\":\"capture_target\",\"target\":%u,\"captured\":%u}\n",
                s.target[n],s.target_count[n]);
        for (unsigned t=0; t<2; ++t) for (unsigned j=0; j<256; ++j)
            if (s.actor_hits[t][j])
                fprintf(f,"{\"kind\":\"actor_index\",\"target\":%u,\"index\":%u,\"calls\":%llu,\"captured\":%u}\n",
                    t ? 0x83d508u : 0x83c7f8u,j,
                    (unsigned long long)s.actor_hits[t][j],s.actor_captured[t][j]);
        for(unsigned n=0;n<HITS;++n) if(s.hit[n].count)
            fprintf(f,"{\"kind\":\"pc\",\"pc\":%u,\"mx\":%u,\"hits\":%llu}\n",
                s.hit[n].pc,s.hit[n].mx,(unsigned long long)s.hit[n].count);
        for(unsigned n=0;n<EDGES;++n) if(s.edge[n].count) {
            Edge *p=&s.edge[n]; fprintf(f,"{\"kind\":\"edge\",\"site\":%u,\"target\":%u,"
                "\"mx\":%u,\"op\":%u,\"calls\":%llu,\"missing\":%llu,\"excluded\":%llu}\n",
                p->site,p->target,p->mx,p->op,(unsigned long long)p->count,
                (unsigned long long)p->missing,(unsigned long long)p->excluded);
        }
        for(unsigned n=0;n<WAITS;++n) {
            Wait *w=&s.wait[n]; fprintf(f,"{\"kind\":\"wait\",\"pc\":%u,\"hits\":%llu,"
                "\"equal\":%llu,\"unequal\":%llu,\"d_nonzero\":%llu,\"wide\":%llu,\"examples\":[",
                wait_pc[n],(unsigned long long)w->count,(unsigned long long)w->equal,
                (unsigned long long)w->unequal,(unsigned long long)w->d_nonzero,(unsigned long long)w->wide);
            for(unsigned j=0;j<w->n_examples;++j) { if(j) fputc(',',f); print_state(f,&w->examples[j]); }
            fprintf(f,"]}\n");
        }
        for(unsigned n=0;n<s.nmi_examples;++n) {
            fprintf(f,"{\"kind\":\"tick_write\",\"entry\":"); print_state(f,&s.nmi_before[n]);
            fprintf(f,",\"exit\":"); print_state(f,&s.nmi_after[n]); fprintf(f,"}\n");
        } ok &= close_output(f);
    } else ok=0;
    f=open_output("COMPLETE.txt","w");
    if(f) { fprintf(f,"io_ok=%d dropped=%u aborted=%d\n",ok,s.dropped,s.failed); ok &= close_output(f); } else ok=0;
    fprintf(stderr,"[gameplay-capture] stopped: %s frames=%u calls=%u io_ok=%d dropped=%u\n",
        s.directory,s.frames,s.calls,ok,s.dropped);
}
void L2CaptureToggle(void) {
    if(lufia2_capture_active) { stop(); return; }
    memset(&s,0,sizeof(s)); s.pending=-1; configure_targets();
    if(MAKE_DIR("captures")!=0 && errno!=EEXIST) {
        fprintf(stderr,"[gameplay-capture] cannot create captures directory\n"); return;
    }
    /* mkdir is the collision check; never overwrite a previous capture. */
    unsigned suffix;
    for(suffix=0;suffix<10000;++suffix) {
        snprintf(s.directory,sizeof(s.directory),"captures/session-%llu-%u",
            (unsigned long long)time(NULL),suffix);
        if(MAKE_DIR(s.directory)==0) break;
        if(errno!=EEXIST) { fprintf(stderr,"[gameplay-capture] mkdir failed\n"); return; }
    }
    if(suffix==10000) return;
    lufia2_capture_active=1;
    fprintf(stderr,"[gameplay-capture] recording %s; Ctrl+Shift+F10 stops; cap=900 frames targets=%u\n",
        s.directory,s.target_total);
}
void L2CaptureShutdown(void) { if(lufia2_capture_active) stop(); }
void L2CaptureFrameBegin(uint32_t input) {
    if(!lufia2_capture_active) return;
    s.in_frame=1; Frame *f=&s.frame[s.frames];
    f->input=input; f->guest_frame=snes_frame_counter;
    f->master_start=g_cpu.master_cycles; f->tick_before=g_cpu.ram ? g_cpu.ram[0x40] : 0;
    s.before_insns=s.instructions; s.begin_ns=SDL_GetTicksNS();
}
void L2CaptureFrameEnd(void) {
    if(!lufia2_capture_active || !s.in_frame) return;
    Frame *f=&s.frame[s.frames]; f->work_ns=SDL_GetTicksNS()-s.begin_ns;
    f->master_end=g_cpu.master_cycles; f->tick_after=g_cpu.ram ? g_cpu.ram[0x40] : 0;
    f->insns=s.instructions-s.before_insns; s.in_frame=0;
    /* Cross-frame routines require a scheduler/device replay, not a leaf
     * function fixture. Keep them explicitly incomplete. */
    if(s.pending>=0) { s.call[s.pending].bus_count=s.buses-s.call[s.pending].bus_first; s.pending=-1; }
    if(++s.frames==FRAMES || s.failed) stop();
}
void L2CaptureGuestEnd(void) {
    if(lufia2_capture_active && s.in_frame)
        s.frame[s.frames].guest_ns=SDL_GetTicksNS()-s.begin_ns;
}
