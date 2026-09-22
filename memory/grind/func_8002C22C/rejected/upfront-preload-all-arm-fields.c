/* REJECTED (s3, 2026-09-22): preload ALL six per-arm scratchpad/global fields
 * (v1,v0,a2,a0,a1,a3) into named locals BEFORE any store, matching the literal
 * register-load order visible in asm/funcs/func_8002C22C.s lines 22-33 (target
 * loads v1/v0/a2/a0/a1/a3=D_80102108 up front, THEN starts the store/add
 * sequence). Measured WORSE than the s2/s3 chassis: sandbox --disable all =
 * 211 (vs 196 for the chassis this was tried against), build_insns 231 vs 238.
 * Chassis: s3 candidate.c with a0/a1/a3 added as named locals holding
 * *(s32*)0x1F800054, *(s32*)0x1F800058 (then reloaded *0x5C), and
 * D_80102108/D_801020E4, in both arms. No FAKE constructs, no pins, zero
 * asm — pure C restructuring. kill_scope: instance — this exact "hoist every
 * field read into a named local ahead of the branch-arm's first store" form,
 * on this chassis, current toolchain (-mel -msoft-float). NOT a class claim:
 * partial preloads (e.g. only a3) were not tried.
 */
extern s32 D_80102314;
void func_8002C22C(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2, a0, a1, a3;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    if (D_800A3824 & 1) {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        v1 = *(s32 *)0x1F80004C;
        v0 = *(s32 *)0x1F800048;
        a2 = *(s32 *)0x1F800050;
        a0 = *(s32 *)0x1F800054;
        a1 = *(s32 *)0x1F800058;
        a3 = D_80102108;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + a1;
        a1 = *(s32 *)0x1F80005C;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + a0;
        *(s32 *)0x1F800360 = v0;
        v0 = D_801020FC;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + a1;
        *(s32 *)0x1F800370 = v0;
        v0 = v0 + a3;
        *(s32 *)0x1F800370 = v0;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        v1 = *(s32 *)0x1F800004;
        v0 = *(s32 *)0x1F800000;
        a2 = *(s32 *)0x1F800008;
        a0 = *(s32 *)0x1F80000C;
        a1 = *(s32 *)0x1F800010;
        a3 = D_801020E4;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + a1;
        a1 = *(s32 *)0x1F800014;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + a0;
        *(s32 *)0x1F800360 = v0;
        v0 = D_801020D8;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + a1;
        *(s32 *)0x1F800370 = v0;
        v0 = v0 + a3;
        *(s32 *)0x1F800370 = v0;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    *(s32 *)0x1F800368 = a2;
    *(s32 *)0x1F800374 = d_v1;
    *(s32 *)0x1F800378 = d_a0;
    *(s32 *)0x1F800374 = d_v1 + d_v0;
    *(s32 *)0x1F800378 = d_a0 + d_a1;
    /* (tail identical to candidate.c) */
}
