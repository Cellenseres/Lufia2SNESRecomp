/* Invoked by --native-wait-selftest, before game/SDL/ROM initialization.
 * Both paths execute on independent local buses. Never run the reference
 * a second time against the live game. No device equivalence claim is made. */
#include "patches/native_patches.h"
#include <stdio.h>
#include <string.h>

extern uint32_t g_interp816_cur_pc;
typedef struct Event { uint32_t address, value, pc, cycles, trace_pc; } Event;
typedef struct TestBus {
    Interp816 *owner;
    uint8_t tick;
    unsigned count, bad, fast, master;
    Event event[128];
} TestBus;
static uint8_t s_test_rom[0x19012];
static unsigned s_cases;

static uint8_t read_test(void *opaque, uint32_t address) {
    TestBus *bus = opaque;
    static const uint8_t code[4] = {0xc5,0x40,0xf0,0xfc};
    uint8_t value=0;
    if (address == 0x40) value=bus->tick;
    else if (address >= 0x83900e && address <= 0x839011)
        value=code[address-0x83900e];
    else bus->bad=1;
    bus->master += address == 0x40 || !bus->fast ? 8 : 6;
    if (bus->count < 128) {
        Event *e=&bus->event[bus->count++];
        e->address=address; e->value=value; e->pc=bus->owner->pc;
        e->cycles=bus->owner->cyclesUsed; e->trace_pc=g_interp816_cur_pc;
    } else bus->bad=1;
    return value;
}
static void write_test(void *opaque, uint32_t address, uint8_t value) {
    (void)address; (void)value; ((TestBus *)opaque)->bad=1;
}
static int same_state(const Interp816 *a, const Interp816 *b) {
#define FIELD(f) if(a->f != b->f) return 0
    FIELD(a); FIELD(x); FIELD(y); FIELD(sp); FIELD(pc); FIELD(dp); FIELD(k); FIELD(db);
    FIELD(c); FIELD(z); FIELD(v); FIELD(n); FIELD(i); FIELD(d); FIELD(xf); FIELD(mf); FIELD(e);
    FIELD(irqWanted); FIELD(nmiWanted); FIELD(waiting); FIELD(stopped);
    FIELD(brkHookEnabled); FIELD(cyclesUsed);
    FIELD(read); FIELD(write); FIELD(read_word); FIELD(write_word);
#undef FIELD
    return 1;
}
static Interp816 initial(unsigned a, unsigned flags, uint16_t pc) {
    Interp816 in;
    memset(&in,0,sizeof(in));
    in.a=(uint16_t)(0xa500u | a); in.x=0x5a3c; in.y=0xb781; in.sp=0x1ffa;
    in.k=0x83; in.db=(uint8_t)(a ^ 0x7e); in.pc=pc;
    in.read=read_test; in.write=write_test; in.brkHookEnabled=true;
    interp816_setFlags(&in,(uint8_t)(flags | 0x20));
    return in;
}
static int compare_step(Interp816 input, unsigned tick, unsigned fast) {
    Interp816 reference=input, native=input;
    TestBus left, right;
    memset(&left,0,sizeof(left)); memset(&right,0,sizeof(right));
    left.owner=&reference; right.owner=&native;
    left.tick=right.tick=(uint8_t)tick; left.fast=right.fast=fast;
    reference.mem=&left; native.mem=&right;
    g_interp816_cur_pc=0xabcdef;
    unsigned reference_cycles=(unsigned)interp816_runOpcode(&reference);
    g_interp816_cur_pc=0xabcdef;
    unsigned native_cycles=Lufia2NativeWaitTryStep(&native,true);
    ++s_cases;
    if (!native_cycles || reference_cycles != native_cycles ||
        !same_state(&reference,&native) || left.bad || right.bad ||
        left.count != right.count || left.master != right.master ||
        memcmp(left.event,right.event,left.count*sizeof(Event))) {
        fprintf(stderr,"[native-wait-test] FAIL case=%u pc=%04X A=%04X tick=%02X P=%02X\n",
            s_cases,input.pc,input.a,tick,interp816_getFlags(&input));
        return 0;
    }
    unsigned master=right.master + (native_cycles-right.count)*6u;
    unsigned expected=input.pc==0x900e ? (fast ? 20u : 24u) :
        (fast ? 12u : 16u) + (input.z ? 6u : 0u);
    return master==expected;
}
static int fallback(Interp816 input, bool outer) {
    TestBus bus;
    memset(&bus,0,sizeof(bus)); bus.owner=&input;
    input.mem=&bus; Interp816 before=input;
    return Lufia2NativeWaitTryStep(&input,outer)==0 && !bus.count &&
        !bus.bad && same_state(&input,&before);
}
int Lufia2NativeWaitSelfTest(void) {
    const uint8_t *saved_rom=g_rom;
    static const uint8_t bytes[4]={0xc5,0x40,0xf0,0xfc};
    memcpy(s_test_rom+0x1900e,bytes,4);
    g_rom=s_test_rom; lufia2_native_wait_enabled=1;
    int ok=1;
    /* Exhaustive 8-bit operands; varied flags, DB, X width and FastROM.
     * Compare branch entry independently: a deadline may split CMP/BEQ. */
    for (unsigned a=0;a<256 && ok;++a) for(unsigned tick=0;tick<256 && ok;++tick) {
        unsigned flags=(a*17u)^tick;
        ok=compare_step(initial(a,flags,0x900e),tick,a&1u) &&
           compare_step(initial(a,flags,0x9010),tick,a&1u);
    }
    /* Every P-bit combination on sign, zero, wrap and carry edge values. */
    static const unsigned edges[]={0,1,0x7f,0x80,0xfe,0xff};
    for(unsigned p=0;p<256 && ok;++p)
        for(unsigned a=0;a<6 && ok;++a) for(unsigned v=0;v<6 && ok;++v)
            ok=compare_step(initial(edges[a],p,0x900e),edges[v],p&1u);

    Interp816 base=initial(0,0x37,0x900e), altered;
    ok=ok && fallback(base,false);
#define REJECT(field,value) do { altered=base; altered.field=value; ok=ok && fallback(altered,true); } while(0)
    REJECT(k,0x03); REJECT(pc,0x900c); REJECT(pc,0x9012); REJECT(mf,false);
    REJECT(dp,1); REJECT(e,true); REJECT(waiting,true); REJECT(stopped,true);
    REJECT(nmiWanted,true); REJECT(irqWanted,true); REJECT(read,NULL);
#undef REJECT
    lufia2_native_wait_enabled=0; ok=ok && fallback(base,true);
    lufia2_native_wait_enabled=1;
    for(unsigned j=0;j<4;++j) {
        s_test_rom[0x1900e+j]^=1; ok=ok && fallback(base,true);
        s_test_rom[0x1900e+j]^=1;
    }
    g_rom=NULL; ok=ok && fallback(base,true); g_rom=saved_rom;
    fprintf(stderr,"[native-wait-test] %s cases=%u registers_flags=checked bus_order=checked cycles=checked guards=checked devices=not-tested\n",
        ok ? "PASS" : "FAIL",s_cases);
    return ok ? 0 : 1;
}
