/* REJECTED (s7, forensics). Second set of the 640 pseudo obtained by storing
 * 640 to +0x10 and READING IT BACK (`w = *(s16 *)(p + 0x10);`) before storing it
 * to +0x20 - the idea being that a memory round-trip is opaque enough to survive
 * cse and leave the pseudo with two sets.
 * MEASURED: difflines 10 / 43 insns, bit-identical to the target-order control.
 * cse tracks the stored value in its memory table and folds the reload straight
 * back to the constant, so no second set and no extra insn survive. */
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
    s16 w;

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
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = 640;
    w = *(s16 *)(p + 0x10);
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = w;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
