/* s8 REJECTED (score 15, build_insns 201, chassis = s8 floor-8 vE).
 * v12 assigned at the if/else join with NO top-of-body initializer, while the
 * two later `+0xC` table pointers still live in their own `tbl` local.  The
 * `li ...,0x12` lands at the CORRECT target[114] slot but the pseudo takes s1,
 * the function drops to three callee-saved registers and an 88-byte frame
 * (target: s0-s3, 96 bytes), and the prologue/epilogue diverge.  This form is
 * HALF of the winning lever - see candidate.c lever 3; it only works when the
 * table-pointer pseudo is simultaneously forced onto s1. */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S73200;
void func_80073200(s32 arg0) {
    S73200 s;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;
    u8 var_v0;
    s32 tbl;
    s32 v12;
    s32 cond;

    s.sp30 = 0;
    s.sp34 = 0;
    s.sp38 = 0x100;
    s.sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    s.sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(D_800A374C + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    s.sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    s.sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        s.sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            s.sp41 = 0xBC;
            var_v0 = 0x78;
        } else {
            s.sp41 = 0xA8;
            var_v0 = 0x6E;
        }
        s.sp42 = var_v0;
        s.sp43 = 0x14;
        s.sp2C = 0x14;
        s.sp1C = s1;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        s.sp1C = s1 + 8;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        s.sp1C = s1 + 0x10;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 2);
        s.sp1C = s1 + 0x18;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        s.sp41 = 0x32;
        s.sp42 = 0x32;
        s.sp43 = 0x5A;
    }
    v12 = 0x12;
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp2C = v12;
    s.sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    s.sp18 = tmp;
    tbl = tmp + 0xC;
    s.sp1C = tbl;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x60), 0);
    AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
    cond = D_800A3580;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (cond < 2) {
        s.sp2C = v12;
        s.sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        s.sp18 = idx;
        tbl = idx + 0xC;
        s.sp1C = tbl;
        s.sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x20), 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
