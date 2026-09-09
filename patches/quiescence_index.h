#ifndef LUFIA2_QUIESCENCE_INDEX_H
#define LUFIA2_QUIESCENCE_INDEX_H
#include <stdint.h>
#include <string.h>

/* Candidate index for a 64-slot ring, NOT a replacement equality predicate.
 * Equal (PC, read epoch, write epoch) tuples always choose the same bucket.
 * Collisions are intentional: the original full comparison rejects them.
 * Each ring owner gets its own index, including nested bridge invocations. */
typedef struct L2QIndex {
    uint64_t buckets[64], live;
    uint8_t slot_bucket[64];
} L2QIndex;
extern int lufia2_quiescence_index_enabled;
int L2QIndexSelfTest(void);

static inline unsigned L2QBucket(uint32_t pc, uint64_t read_epoch, uint64_t write_epoch) {
    return (unsigned)(pc ^ (pc >> 6) ^ read_epoch ^ (read_epoch >> 32) ^
                      write_epoch ^ (write_epoch >> 32)) & 63u;
}
static inline void L2QInit(L2QIndex *index) { memset(index,0,sizeof(*index)); }
static inline uint64_t L2QCandidates(const L2QIndex *index, uint32_t pc,
                                    uint64_t read_epoch, uint64_t write_epoch) {
    return index->buckets[L2QBucket(pc,read_epoch,write_epoch)];
}
static inline void L2QSet(L2QIndex *index, unsigned slot, uint32_t pc,
                         uint64_t read_epoch, uint64_t write_epoch, int valid) {
    uint64_t bit=UINT64_C(1)<<slot;
    if(index->live & bit) index->buckets[index->slot_bucket[slot]] &= ~bit;
    index->live &= ~bit;
    if(valid) {
        unsigned bucket=L2QBucket(pc,read_epoch,write_epoch);
        index->slot_bucket[slot]=(uint8_t)bucket;
        index->buckets[bucket] |= bit;
        index->live |= bit;
    }
}
/* Return the least significant occupied slot. Ascending slot order preserves
 * the original first-match/break choice, including its repeat counter. This
 * uses portable unsigned C, without MSVC/ARM intrinsics or undefined shifts. */
static inline unsigned L2QPop(uint64_t *mask) {
    uint64_t value=*mask;
    if(!value) return 64;
    unsigned bit=0;
    if(!(value & UINT64_C(0xffffffff))) {bit+=32; value>>=32;}
    if(!(value & UINT64_C(0xffff))) {bit+=16; value>>=16;}
    if(!(value & UINT64_C(0xff))) {bit+=8; value>>=8;}
    if(!(value & UINT64_C(0xf))) {bit+=4; value>>=4;}
    if(!(value & UINT64_C(3))) {bit+=2; value>>=2;}
    if(!(value & UINT64_C(1))) ++bit;
    *mask &= *mask-UINT64_C(1);
    return bit;
}
#endif
