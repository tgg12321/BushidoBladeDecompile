/* func_80073200 — session 1 (recon) first-draft translation.
 * Derived from asm/funcs/func_80073200.s + m2c reference (tmp/authoring/func_80073200.m2c.c)
 * + sibling S46C/S60C8 struct callers of func_80073728/func_8007352C already in src/text1b.c
 * (func_8005D46C, func_8005FA98, func_800600C8) for the accumulator-field call convention.
 * NOT YET MEASURED AT 0 — starting point for the next (matching) session.
 */
extern s32 D_800A35A8;
extern s16 D_800A3580;
extern void *D_800A35C4;
extern s32 D_800A374C;
extern s32 func_80073728(s32, s32);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);

void func_80073200(s32 arg0) {
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
    s32 sp38;
    s32 sp3C;
    s8 sp40;
    s8 sp41;
    s8 sp42;
    s8 sp43;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;

    sp30 = 0;
    sp34 = 0;
    sp38 = 0x100;
    sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(D_800A374C + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            sp41 = 0xBC;
            sp42 = 0x78;
        } else {
            sp41 = 0xA8;
            sp42 = 0x6E;
        }
        sp43 = 0x14;
        sp2C = 0x14;
        sp1C = s1;
        sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&sp18, 0);
        sp1C = s1 + 8;
        sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&sp18, 1);
        sp1C = s1 + 0x10;
        sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&sp18, 2);
        sp1C = s1 + 0x18;
        sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&sp18, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(D_800A374C + (sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        sp41 = 0x32;
        sp42 = 0x32;
        sp43 = 0x5A;
    }
    sp34 = 0;
    sp30 = 0;
    sp2C = 0x12;
    sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    sp18 = tmp;
    sp1C = tmp + 0xC;
    sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(sp18, 0x60), 0);
    AddPrim(D_800A374C + (sp2C * 4), *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (D_800A3580 < 2) {
        sp2C = 0x12;
        sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        sp18 = idx;
        sp1C = idx + 0xC;
        sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(sp18, 0x20), 0);
        AddPrim(D_800A374C + (sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
