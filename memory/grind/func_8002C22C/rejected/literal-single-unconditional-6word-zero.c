/* REJECTED (s3, 2026-09-22): move the 6-word scratchpad zero-init
 * (0x1F800360/364/368/370/374/378 = 0;) OUT of both if/else arms entirely and
 * write it ONCE, unconditionally, immediately before the first `if` — this is
 * the LITERAL structural mirror of asm/funcs/func_8002C22C.s lines 2-18 (a
 * single unconditional 6x `sw zero` block before the `andi/beqz`). Measured
 * WORSE than the s3 chassis (per-arm duplicated 4-word zero-init): sandbox
 * --disable all = 211 (vs 196), build_insns 230 vs 238. Confirms the earlier
 * s2 finding (do-while(0)-wrapped single-arm zero-init was also killed) that
 * something about cse1's basic-block extension through the join label
 * (cse_end_of_basic_block, cse.c:8102-8184) merges/forwards this single
 * unconditional block differently in our fork than duplicating the same
 * stores into both arms — i.e. literal source-structure fidelity to the
 * target's OWN join topology does not reproduce the target's byte count here;
 * the sanctioned duplicated-statement-into-arms spelling
 * ([[cse-block-extension-controls-fold-span]]) does better despite not
 * mirroring the join point. No FAKE constructs, no pins, zero asm.
 * kill_scope: instance — this exact single-block-before-branch spelling, on
 * this chassis, current toolchain (-mel -msoft-float).
 */
extern s32 D_80102314;
void func_8002C22C(void) {
    s32 *t0 = (s32 *)0x1F8002B8;
    s32 *d_tbl = &D_80102314;
    s32 v1, v0, a2;
    s32 d_v1, d_a0, d_v0, d_a1;
    s32 sum_v0_2, sum_a1_2;

    *(s32 *)0x1F800360 = 0;
    *(s32 *)0x1F800364 = 0;
    *(s32 *)0x1F800368 = 0;
    *(s32 *)0x1F800370 = 0;
    *(s32 *)0x1F800374 = 0;
    *(s32 *)0x1F800378 = 0;
    if (D_800A3824 & 1) {
        v1 = *(s32 *)0x1F80004C;
        v0 = *(s32 *)0x1F800048;
        a2 = *(s32 *)0x1F800050;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800058;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F800054;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_80102100;
        d_a0 = D_80102104;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F80005C;
        *(s32 *)0x1F800370 = D_801020FC;
        *(s32 *)0x1F800370 = D_801020FC + D_80102108;
        d_v0 = D_8010210C;
        d_a1 = D_80102110;
    } else {
        v1 = *(s32 *)0x1F800004;
        v0 = *(s32 *)0x1F800000;
        a2 = *(s32 *)0x1F800008;
        *(s32 *)0x1F800364 = v1;
        v1 = v1 + *(s32 *)0x1F800010;
        *(s32 *)0x1F800360 = v0;
        v0 = v0 + *(s32 *)0x1F80000C;
        *(s32 *)0x1F800360 = v0;
        *(s32 *)0x1F800364 = v1;
        d_v1 = D_801020DC;
        d_a0 = D_801020E0;
        *(s32 *)0x1F800368 = a2;
        a2 = a2 + *(s32 *)0x1F800014;
        *(s32 *)0x1F800370 = D_801020D8;
        *(s32 *)0x1F800370 = D_801020D8 + D_801020E4;
        d_v0 = D_801020E8;
        d_a1 = D_801020EC;
    }
    /* (tail identical to candidate.c) */
}
