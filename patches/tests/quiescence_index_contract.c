#include "patches/quiescence_index.h"
#include <stdio.h>

typedef struct Record {uint32_t pc; uint64_t read,write; unsigned payload; int valid;} Record;
static uint32_t s_rng=0xc5849010u;
static uint32_t random32(void) {
    s_rng^=s_rng<<13; s_rng^=s_rng>>17; s_rng^=s_rng<<5; return s_rng;
}
static int same_key(const Record *a,const Record *b) {
    return a->pc==b->pc && a->read==b->read && a->write==b->write;
}
static int check(const L2QIndex *index,const Record *ring,const Record *query) {
    uint64_t candidates=L2QCandidates(index,query->pc,query->read,query->write);
    uint64_t expected=0;
    unsigned linear=64,indexed=64;
    for(unsigned i=0;i<64;++i) {
        if(ring[i].valid && L2QBucket(ring[i].pc,ring[i].read,ring[i].write)==
                L2QBucket(query->pc,query->read,query->write)) expected |= UINT64_C(1)<<i;
        if(ring[i].valid && same_key(&ring[i],query) && ring[i].payload==query->payload && linear==64) linear=i;
    }
    if(candidates!=expected) return 0;
    for(unsigned i=L2QPop(&candidates);i<64;i=L2QPop(&candidates)) {
        if(ring[i].valid && same_key(&ring[i],query) && ring[i].payload==query->payload) {indexed=i;break;}
    }
    return indexed==linear;
}
int L2QIndexSelfTest(void) {
    L2QIndex index; Record ring[64];
    L2QInit(&index); memset(ring,0,sizeof(ring));
    unsigned checks=0;
    int ok=1;
    /* Every 16-bit mask in each quarter, plus full-width edge masks. */
    for(unsigned shift=0;shift<64 && ok;shift+=16) for(unsigned bits=0;bits<65536 && ok;++bits) {
        uint64_t mask=(uint64_t)bits<<shift;
        for(unsigned slot=0;slot<64 && ok;++slot)
            if(((uint64_t)bits<<shift)&(UINT64_C(1)<<slot)) ok=L2QPop(&mask)==slot;
        ok=ok && L2QPop(&mask)==64; ++checks;
    }
    uint64_t full=UINT64_MAX;
    for(unsigned i=0;i<64;++i) ok=ok && L2QPop(&full)==i;
    ok=ok && !full; ++checks;
    /* Duplicate keys: first equal slot must win. Erasing a step=0 slot must
     * remove its old membership, even when it is reused in another bucket. */
    for(unsigned i=0;i<64;++i) {
        ring[i]=(Record){0x83900e,UINT64_MAX,UINT64_MAX,7,1};
        L2QSet(&index,i,ring[i].pc,ring[i].read,ring[i].write,1);
    }
    for(unsigned i=0;i<64 && ok;++i) {
        Record query=ring[i]; ok=check(&index,ring,&query); ++checks;
        ring[i].valid=0; L2QSet(&index,i,0,0,0,0);
    }
    /* Arbitrary overwrites, resets, epoch wrap, collisions and changing
     * non-indexed state. No monotonic-epoch assumption is permitted. */
    for(unsigned n=0;n<30000 && ok;++n) {
        unsigned slot=random32()&63u;
        Record r;
        if((n&7u)==0) r=ring[random32()&63u];
        else {
            r.pc=0x830000u|(random32()&255u);
            r.read=(n%3u) ? random32()&31u : UINT64_MAX-(random32()&31u);
            r.write=(n%5u) ? random32()&15u : (uint64_t)random32()<<32;
            r.payload=random32()&7u; r.valid=(random32()&7u)!=0;
        }
        ring[slot]=r; L2QSet(&index,slot,r.pc,r.read,r.write,r.valid);
        for(unsigned q=0;q<8 && ok;++q) {
            Record query=ring[random32()&63u];
            if(q&1u) query.payload^=1u;
            ok=check(&index,ring,&query); ++checks;
        }
        if(n%997u==0) {L2QInit(&index); memset(ring,0,sizeof(ring));}
    }
    fprintf(stderr,"[quiescence-index-test] %s checks=%u membership=checked ascending_order=checked collisions=checked overwrite_reset_wrap=checked first_match=checked devices=not-tested\n",ok?"PASS":"FAIL",checks);
    return ok?0:1;
}
