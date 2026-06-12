/*
 * Form: "dual named-intermediate (vs + arg) with identity bounce through vs"
 * Region B replacement for sys_VSync (src/ings2.c).
 *
 * Honest sandbox floor: 6 (build_insns 82 == target).  HEAD candidate floor 7.
 *
 * Session 5 (2026-06-12) probed this and 12+ near-isomorphic shapes.  The
 * dual-intermediate-with-vs-first family ALL hit floor 6 (M5, R1, R2, R4, R7,
 * R8, R9, Q5, Q6, Q7).  Variants with only `arg`, or only `vs`, or with `arg`
 * declared first, scored 7.
 *
 * The lever's mechanism: introducing `s32 vs = *D_800A1510; s0_val = vs;`
 * creates a v0-living named intermediate that biases sched1's INSN_PRIORITY
 * chain for the volatile-load result, AND the `s0_val = vs;` move becomes a
 * jal-delay-slot candidate that reorg.c picks up (the build's `move s0, v0`
 * winds up in the delay slot).  Target instead has `addiu a0, a0, 1` in the
 * delay slot and reads `lw s0, 0(v0)` directly into s0 — so M5 is structurally
 * 6 instructions off target, even though build_insns finally matches at 82.
 *
 * CHEAT-REVIEWER VERDICT (2026-06-12): FAIL.
 *
 * Reviewer's 6-test analysis (verbatim summary):
 *   - Test 1 (semantic purpose): `s32 vs = *D_800A1510; s0_val = vs;` is an
 *     identity bounce — semantically identical to `s0_val = *D_800A1510;`.
 *     `s32 arg = g_sys_dma_region + 1; func(arg, 1);` is semantically
 *     identical to `func(g_sys_dma_region + 1, 1);`. Neither construct has
 *     any observable effect distinct from the inline form.
 *   - Test 2 (human-programmer test): No programmer writing "read hardware
 *     register, then issue DMA wait call" would naturally introduce these
 *     intermediates. The minimal HEAD form is exactly what idiomatic C looks
 *     like for this operation.
 *   - Test 3 (GCC-internals justification): The session's stated lever
 *     mechanism IS in terms of sched1 INSN_PRIORITY / LUID — explicitly a
 *     GCC-internals manipulation.  The declaration-order sensitivity (only
 *     `vs` first AND `arg` second works; swap order → floor 7) is textbook
 *     evidence of LUID-ordering coercion.
 *   - Test 5 (family check): NOT covered by the SOTN-accepted
 *     named-intermediate-declaration-order family.  That family names a
 *     genuine sub-computation (e.g. Region A's `s32 base = D_800A151C - 1`
 *     names a real frame-offset sub-expression; SOTN's `randy` names a
 *     sub-coordinate).  An identity bounce `vs = read; dest = vs` has no
 *     sub-computation to name; it's a pure codegen coercion.
 *   - Test 6 (naming): `vs` is ambiguous between "vsync state" and the
 *     register-coloured "value via v0"; `arg` is generic.
 *
 * Rule references: [[no-new-park-categories]] cheats-by-any-spelling section
 * (the dead-intermediate / identity-bounce family); [[inline-asm-policy]]
 * expanded cheat catalog; [[review-discipline-before-commit]] default-FAIL
 * standard.
 *
 * THIS FORM IS PRESERVED AS A "DON'T RE-DERIVE THIS" WARNING.  Future
 * sessions may discover other near-shapes (R1, R2, R4, R7, R8, R9 of session
 * 5's sweep) that hit floor 6 via the same mechanism — they are all in this
 * family and all FAIL the same way.
 */

/* Region B replacement — REJECTED */
{
    s32 vs = *D_800A1510;
    s32 arg = g_sys_dma_region + 1;
    s0_val = vs;
    func_80082A14(arg, 1);
}
