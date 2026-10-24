#ifndef LUFIA2_PPU_ELASTIC_CERTIFICATE_H
#define LUFIA2_PPU_ELASTIC_CERTIFICATE_H
#include "snes/ppu.h"
#include <string.h>
/* A frame's owned entry policy selects the arrays that require comparison.
 * Counts and bounds remain live checks: newly enabled mappings cannot escape
 * capture, and unused segment storage has no pixel dependency. */
typedef struct Lufia2PpuElasticCertificate {
    uint8_t band[kPpuWsElasticBands*4],layer[kPpuWsElasticBands*4];
    unsigned count;
    bool valid;
} Lufia2PpuElasticCertificate;
static inline bool Lufia2PpuElasticPrepare(Lufia2PpuElasticCertificate *c,const Ppu *p) {
    c->count=0;c->valid=false;
    for(unsigned band=0;band<kPpuWsElasticBands;++band) for(unsigned layer=0;layer<4;++layer) {
        const unsigned count=p->wsElasticNSeg[band][layer];
        if(count>kPpuWsElasticSegs) return false;
        if(count) {c->band[c->count]=(uint8_t)band;c->layer[c->count++]=(uint8_t)layer;}
    }
    c->valid=true;return true;
}
static inline bool Lufia2PpuElasticEqual(const Lufia2PpuElasticCertificate *c,const Ppu *a,const Ppu *b) {
    if(!c->valid || memcmp(a->wsElasticY0,b->wsElasticY0,sizeof b->wsElasticY0) ||
       memcmp(a->wsElasticY1,b->wsElasticY1,sizeof b->wsElasticY1) ||
       memcmp(a->wsElasticNSeg,b->wsElasticNSeg,sizeof b->wsElasticNSeg)) return false;
    for(unsigned i=0;i<c->count;++i) {
        const unsigned band=c->band[i],layer=c->layer[i],count=a->wsElasticNSeg[band][layer];
        if(memcmp(a->wsElasticSeg[band][layer],b->wsElasticSeg[band][layer],count*sizeof(PpuWsElasticSeg))) return false;
    }
    return true;
}
#endif
