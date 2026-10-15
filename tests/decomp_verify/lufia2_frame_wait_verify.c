#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/system.h"

typedef struct Event { uint32_t address; uint8_t value,write; } Event;
typedef struct TestBus {
    SnesVerifyBus bus;
    Event trace[262144];
    unsigned used,clock_reads,delay,periodic,phase,calls,fail_call;
    uint16_t clock;
    uint8_t mask;
} TestBus;

static TestBus reference_bus,native_bus;
static TestBus *fetch_bus;
static uint32_t fetch_address;
static unsigned fetch_left,suite,number,boundaries,returns,unwinds,mask_shift=8u,clock_high;
static const uint32_t entries[]={0x83900cu,0x8eb550u,0x868b48u,0x86974fu,0x869748u};
static const uint32_t polls[]={0x83900eu,0x8eb552u,0x868b4eu,0x869752u,0x869748u};
static const uint32_t terminals[]={0x839012u,0x8eb556u,0x868b54u,0x869757u,0x86974eu};

static void trace(TestBus*t,uint32_t address,uint8_t value,unsigned write){
    if(t->used==262144u){fputs("trace limit\n",stderr);exit(1);}
    t->trace[t->used++]=(Event){address,value,(uint8_t)write};
}
static uint8_t read_bus(void*context,uint32_t address){
    TestBus*t=context;
    unsigned fetch=t==fetch_bus&&fetch_left&&address==fetch_address;
    if(fetch){--fetch_left;++fetch_address;}
    unsigned changed=0;
    if(!fetch&&address==t->clock){
        ++t->clock_reads;
        changed=t->periodic?(t->clock_reads%2u==0u):(t->clock_reads==t->delay+2u);
        if(changed){t->phase^=1u;uint16_t updated=(uint16_t)(t->clock+clock_high);if(updated<0x2000u)t->bus.wram[updated]^=t->mask;}
    }
    uint8_t value=SnesVerifyBusRead(&t->bus,address);
    if(!fetch&&address==(uint16_t)(t->clock+clock_high)&&address>=0x2000u&&t->phase)value^=t->mask;
    if(!fetch)trace(t,address,value,0u);
    return value;
}
static void write_bus(void*context,uint32_t address,uint8_t value){
    TestBus*t=context;SnesVerifyBusWrite(&t->bus,address,value);trace(t,address,value,1u);
}
static void load(Interp816*r,const Lufia2CpuState*n,uint32_t pc){
    r->a=n->accumulator;r->x=n->x;r->y=n->y;r->sp=n->stack;r->dp=n->direct_page;r->db=n->data_bank;r->k=(uint8_t)(pc>>16);r->pc=(uint16_t)pc;
    r->c=n->carry;r->z=n->zero;r->n=n->negative;r->v=n->overflow;r->d=n->decimal;r->i=n->irq_disable;r->mf=n->accumulator_is_8_bit;r->xf=n->index_is_8_bit;
    r->e=r->irqWanted=r->nmiWanted=r->waiting=r->stopped=0;interp816_set_brk_hook_enabled(r,false);
}
static void store(Lufia2CpuState*n,const Interp816*r){
    n->accumulator=r->a;n->x=r->x;n->y=r->y;n->stack=r->sp;n->direct_page=r->dp;n->data_bank=r->db;n->program_bank=r->k;
    n->carry=r->c;n->zero=r->z;n->negative=r->n;n->overflow=r->v;n->decimal=r->d;n->irq_disable=r->i;n->accumulator_is_8_bit=r->mf;n->index_is_8_bit=r->xf;
}
static void step(Interp816*r,TestBus*t){
    uint32_t pc=SnesVerifyPc24(r);uint8_t op=SnesVerifyBusRead(&t->bus,pc);
    fetch_bus=t;fetch_address=pc;
    switch(op){
    case 0x20u:fetch_left=3u;break;
    case 0xa5u:case 0xa6u:case 0xc5u:case 0xe2u:case 0xf0u:case 0xd0u:fetch_left=2u;break;
    case 0x48u:case 0x08u:case 0x28u:case 0x68u:case 0x60u:case 0x3au:fetch_left=1u;break;
    default:fetch_left=0u;break;
    }
    if(!fetch_left){fprintf(stderr,"opcode %02X at%06X\n",op,pc);exit(1);}
    interp816_runOpcode(r);fetch_left=0u;
}
static uint8_t child(void*context,Lufia2CpuState*n,uint32_t target,uint32_t site,uint8_t frame){
    TestBus*t=context;
    if(target!=0x86974fu||site!=0x869748u||frame!=2u){fputs("child contract\n",stderr);exit(1);}
    ++t->calls;if(t->calls==t->fail_call)return 0u;
    Interp816*r=interp816_init(t,read_bus,write_bus);load(r,n,target);
    uint16_t back_stack=(uint16_t)(n->stack+2u);unsigned steps;
    for(steps=0;steps<128u;++steps){if(r->sp==back_stack&&SnesVerifyPc24(r)==0x86974bu)break;step(r,t);}
    if(steps==128u){fputs("wait child did not return\n",stderr);exit(1);}
    store(n,r);interp816_free(r);return 1u;
}
static Lufia2ExecutionResult invoke(const Lufia2Memory*m,Lufia2CpuState*n){
    switch(suite){
    case 0:return Lufia2FieldWaitForNextFrame(m,n);
    case 1:return Lufia2ItemRegionWaitForNextFrame(m,n);
    case 2:return Lufia2MenuWaitForNextFrame(m,n);
    case 3:return Lufia2WorldWaitForNextFrame(m,n);
    default:return Lufia2WorldWaitFrames(m,n,child,m->context);
    }
}
static void reset_bus(TestBus*t,const Lufia2CpuState*n,unsigned delay,unsigned periodic,unsigned fail){
    memset(t->bus.wram,(int)(number*73u),SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&t->bus);SnesVerifyBusResetMmio(&t->bus);t->bus.wm_address=0;
    t->used=t->clock_reads=t->calls=t->phase=0;t->delay=delay;t->periodic=periodic;t->fail_call=fail;
    t->clock=(uint16_t)(n->direct_page+0x40u);t->mask=(uint8_t)(1u<<(mask_shift<8u?mask_shift:number%8u));
}
static int run_case(unsigned delay,unsigned special,unsigned count,unsigned fail){
    Lufia2CpuState n={0};n.program_bank=(uint8_t)(entries[suite]>>16);n.data_bank=(uint8_t)number;
    n.accumulator=(uint16_t)(number*197u);n.x=(uint16_t)(number*83u);n.y=(uint16_t)(number*113u);
    n.accumulator_is_8_bit=(uint8_t)((number>>1)&1u);n.index_is_8_bit=(uint8_t)(number&1u);
    if(n.index_is_8_bit){n.x&=255u;n.y&=255u;}
    n.stack=(uint16_t)(0x1f20u+number%221u);n.direct_page=(uint16_t)(number*187u);
    n.carry=number&1u;n.zero=(number>>1)&1u;n.negative=(number>>2)&1u;n.overflow=(number>>3)&1u;n.decimal=(number>>4)&1u;n.irq_disable=(number>>5)&1u;
    if(special==1u)n.direct_page=0;
    if(special==2u)n.direct_page=(uint16_t)(n.stack-(n.accumulator_is_8_bit?1u:2u)-0x40u);
    if(special==3u)n.direct_page=(uint16_t)(n.stack-0x40u);
    if(suite==4u){n.direct_page=0;n.accumulator=(uint16_t)count;if(n.accumulator_is_8_bit)n.accumulator=(uint16_t)(0x5a00u|(count&255u));}
    Lufia2CpuState initial=n;
    reset_bus(&native_bus,&n,delay,suite==4u,fail);reset_bus(&reference_bus,&n,delay,suite==4u,fail);
    Lufia2Memory memory={read_bus,write_bus,&native_bus,NULL,NULL};
    Lufia2ExecutionResult result=invoke(&memory,&n);
    Interp816*r=interp816_init(&reference_bus,read_bus,write_bus);load(r,&initial,entries[suite]);
    uint32_t stop=result.flow==LUFIA2_EXECUTION_CHILD_UNWOUND?0x86974fu:result.pc;
    unsigned steps;
    for(steps=0;steps<4000000u;++steps){
        uint32_t pc=SnesVerifyPc24(r);
        if(pc==stop&&r->sp==n.stack&&reference_bus.clock_reads==native_bus.clock_reads&&reference_bus.calls==native_bus.calls)break;
        if(suite==4u&&pc==0x869748u)++reference_bus.calls;
        step(r,&reference_bus);
    }
    Lufia2CpuState expected=initial;store(&expected,r);expected.resume_pc=n.resume_pc;
    unsigned event=0;
    while(event<native_bus.used&&event<reference_bus.used&&native_bus.trace[event].address==reference_bus.trace[event].address&&native_bus.trace[event].value==reference_bus.trace[event].value&&native_bus.trace[event].write==reference_bus.trace[event].write)++event;
    int same=steps<4000000u&&!memcmp(&expected,&n,sizeof n)&&!memcmp(native_bus.bus.wram,reference_bus.bus.wram,SNES_VERIFY_WRAM_SIZE)&&native_bus.used==reference_bus.used&&event==native_bus.used;
    if(!same){fprintf(stderr,"wait mismatch suite%u case%u delay%u special%u count%u flow%u PC%06X/%06X steps%u CPU%d WRAM%d events%u/%u first%u reads%u/%u calls%u/%u S%04X/%04X A%04X/%04X\n",suite,number,delay,special,count,result.flow,result.pc,SnesVerifyPc24(r),steps,memcmp(&expected,&n,sizeof n),memcmp(native_bus.bus.wram,reference_bus.bus.wram,SNES_VERIFY_WRAM_SIZE),native_bus.used,reference_bus.used,event,native_bus.clock_reads,reference_bus.clock_reads,native_bus.calls,reference_bus.calls,n.stack,r->sp,n.accumulator,r->a);return 1;}
    if(result.flow==LUFIA2_EXECUTION_RETURNED){if(result.pc!=terminals[suite])return 1;++returns;}
    else if(result.flow==LUFIA2_EXECUTION_BOUNDARY){if(result.pc!=polls[suite]||(suite==4u?native_bus.calls!=4096u:native_bus.clock_reads!=65537u))return 1;++boundaries;}
    else if(result.flow==LUFIA2_EXECUTION_CHILD_UNWOUND){if(!fail||result.pc!=0x869748u)return 1;++unwinds;}
    else return 1;
    interp816_free(r);return 0;
}
static int guards(void){
    unsigned count=0;
    for(suite=0;suite<5u;++suite)for(unsigned bank=0;bank<256u;++bank)if(bank!=(entries[suite]>>16)){
        Lufia2CpuState n={0};n.program_bank=(uint8_t)bank;n.stack=0x1ff0u;n.accumulator=0xf0f0u;
        Lufia2CpuState expected=n;expected.resume_pc=entries[suite];
        reset_bus(&native_bus,&n,0,0,0);Lufia2Memory m={read_bus,write_bus,&native_bus,NULL,NULL};
        Lufia2ExecutionResult r=invoke(&m,&n);
        if(r.flow!=LUFIA2_EXECUTION_BOUNDARY||r.pc!=entries[suite]||memcmp(&n,&expected,sizeof n)||native_bus.used)return 1;
        ++count;
    }
    suite=4u;Lufia2CpuState n={0};n.program_bank=0x86u;n.stack=0x1ff0u;
    Lufia2CpuState expected=n;expected.resume_pc=0x869748u;reset_bus(&native_bus,&n,0,0,0);
    Lufia2Memory m={read_bus,write_bus,&native_bus,NULL,NULL};Lufia2ExecutionResult r=Lufia2WorldWaitFrames(&m,&n,NULL,NULL);
    if(r.flow!=LUFIA2_EXECUTION_BOUNDARY||r.pc!=0x869748u||memcmp(&n,&expected,sizeof n)||native_bus.used)return 1;
    printf("PASS %u wrong-bank/null-child guards before bus access\n",count+1u);return 0;
}
int main(int argc,char**argv){
    uint8_t*rom;size_t size;if((argc!=2&&argc!=4)||!SnesVerifyLoadFile(argv[1],&rom,&size)||size!=0x280000u)return 2;
    if(argc==4){uint32_t pc=(uint32_t)strtoul(argv[2],NULL,16);rom[((pc>>16)&127u)*32768u+(pc&32767u)]=(uint8_t)strtoul(argv[3],NULL,16);}
    if(!SnesVerifyBusInit(&reference_bus.bus,rom,size)||!SnesVerifyBusInit(&native_bus.bus,rom,size))return 2;
    if(guards())return 1;
    unsigned total=0;
    for(suite=0;suite<4u;++suite){
        for(number=0;number<4096u;++number){if(run_case(number%32u,number%4u,0,0))return 1;++total;}
        for(number=0;number<4u;++number)for(unsigned delay=65535u;delay<=65537u;++delay){if(run_case(delay,1u,0,0))return 1;++total;}
        for(number=0;number<4u;++number)for(mask_shift=0;mask_shift<8u;++mask_shift)for(unsigned alias=2u;alias<=3u;++alias){if(run_case(0u,alias,0,0))return 1;++total;}
        mask_shift=8u;clock_high=1u;
        for(number=0;number<4u;++number){if(run_case(0u,1u,0,0))return 1;++total;}
        clock_high=0u;
    }
    suite=4u;const unsigned counts[]={0u,1u,2u,7u,255u,256u,4095u,4096u,4097u,65535u};
    for(number=0;number<4u;++number)for(unsigned i=0;i<sizeof counts/sizeof counts[0];++i){if(run_case(0,1u,counts[i],0))return 1;++total;}
    for(number=0;number<4u;++number)for(unsigned fail=1;fail<4u;++fail){if(run_case(0,1u,7u,fail))return 1;++total;}
    printf("PASS %u original-ROM frame-wait cases: returns%u boundaries%u unwinds%u; CPU/WRAM/ordered bus\n",total,returns,boundaries,unwinds);
    if(!returns||!boundaries||unwinds!=12u)return 1;
    SnesVerifyBusDestroy(&reference_bus.bus);SnesVerifyBusDestroy(&native_bus.bus);free(rom);return 0;
}
