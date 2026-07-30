/* REJECTED (s7, forensics) - this was session 6's "most promising, genuinely
 * untried" frontier probe, and it is dead.
 * `s16 w = 640;` at the top, then a SECOND `w = 640;` placed AFTER the may-alias
 * load of D_800A374C (the OT base), on the theory that a memory read invalidates
 * cse's value table and lets the second set survive, giving the pseudo
 * reg_n_sets == 2 and defeating sched1's birthing sink.
 * MEASURED: difflines 10 / 43 insns, bit-identical to the target-order control.
 * RTL dumps (s7/rtl_P3_two_sets_across_otload/): the pseudo (reg/v:HI 75) has 2
 * sets in base.i.rtl and only 1 already in base.i.cse - the FIRST cse deletes it.
 * A memory read invalidates cse's MEMORY equivalences, not its REGISTER value
 * table, so a register whose known value is const 640 makes any later `= 640`
 * redundant no matter what sits in between. */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

extern void *D_800A38B4;
extern s32 D_800A374C;
extern void initPolyG4(u8 *p);
extern void ot_Link(u32 *a0, u32 *a1);

void func_8003553C(void) {
    u8 *p;
    u8 *q;
    u32 *ot;
    s16 w = 640;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x22) = 240;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0xE] = 0x80;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    w = 640;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = w;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = w;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
