/* s2 (structural, 2026-07-20) — the phantom-frame-slot mechanism does NOT
 * fire in func_80049A2C's shape via any of the H1/H2 axes.
 *
 * Sandbox measurements (all with dummy[2] REMOVED from candidate.c, baseline
 * score=12 from frame-collapse; goal was to reach 0 by triggering the
 * reload/alter_reg stale-ref phantom of phantom-frame-slots-gcc272):
 *
 * A. Dead HImode-bitwise pair after the fade-check:
 *      { s16 fp_a = *p_anim; s16 fp_b = (s16)temp_v1;
 *        if ((fp_a & ~fp_b) & 1) { (void)fp_a; } }
 *    -> score 12, insns 126. Fully DCE'd. No phantom.
 *
 * B. Dead HImode-bitwise pair with TWO independent lh loads
 *    (from D_800EF980 base):
 *      { s16 fp_a = *p_anim; s16 fp_b = *((s16*)new_var8);
 *        if ((fp_a & ~fp_b) & 1) { (void)fp_a; } }
 *    -> score 12, insns 126. Fully DCE'd. No phantom.
 *
 * C. Widen a1_val from s16 to s32:
 *    -> score 12, insns 126. No frame effect.
 *
 * D. Widen new_var2 from s16 to s32:
 *    -> score 13, insns 126. STRICTLY WORSE (extra sh->sw diff or similar).
 *
 * E. Live HImode-bitwise embedded in a1_val's real computation
 *    (fold-safe zero-injection):
 *      a1_val = (s16)((fp_a * 2) | ((fp_a & ~fp_b) & 0));
 *    -> score 12, insns 126. Constant-fold ate the injected bitwise.
 *
 * F. Remove the recomputation of a1_val at the second obj-init block
 *    (share the value across the func_800417D0 call):
 *    -> score 34, insns 123 (LOST 3 target insns — the 2nd lh+nop+sll
 *       are genuinely in target bytes). Cannot share.
 *
 * MECHANISM WHY H1 FAILED: per phantom-frame-slots-gcc272 s3 mechanism —
 * reload's alter_reg fires when combine eliminates a *sign-extension of a
 * paradoxical subreg* (redundant HImode->SImode). Preconditions:
 *   (1) the source must be an lh/lb sign-extending load (already-extended
 *       in register), AND
 *   (2) the value must feed an HImode computation whose result never uses
 *       the upper bits (bitwise AND with low mask, & 1, & 0xFF).
 *   (3) flow's reg_n_refs count must survive combine's deletion.
 *
 * In func_80049A2C every dead-injected HImode bitwise expression is fully
 * DCE'd (dead statement removal) BEFORE reaching combine, so combine has
 * no work to elide and no stale ref is left. The tslLineG5Init witness
 * works because its bitwise expression *gates real emitted stores* — the
 * expression is live, its result matters at runtime, but combine still
 * proves the sign-extension redundant.
 *
 * BLOCKER FOR THIS FUNCTION: there is no place in the body where we can
 * legitimately gate a real store behind an HImode-bitwise predicate
 * without adding emitted instructions the target does not have — the
 * target already sits at 126 insns matching exactly, and every store is
 * unconditional.
 *
 * NB: the H1 hypothesis in state.json still lists `int fp_slot = temp_v1
 * & 0xFF;` as a probe — that would be an SImode (int) bitwise on a u8;
 * even if it survived DCE it would not trigger the HImode-paradoxical-
 * subreg combine path. Do not re-run it.
 *
 * REMAINING HYPOTHESES (unmeasured):
 *   H4 (new): the target may require a source-declared unwritten-tail
 *   local (the func_80037540 "capacity-declared buffer" family) — which
 *   is currently under OWNER-ESCALATION for that function. If it rules
 *   FOR the family, dummy[2] may become sanctionable here as an
 *   annotated FAKE with the same reasoning.
 *   H5 (new): named-local-fake-exception SOTN-shape 'dead scalar decl
 *   for RA' — a /* FAKE */-annotated `s32 dummy;` (single decl, no
 *   array) might qualify AFTER measured H1/H2/H3 exhaustion; ledger
 *   evidence now supports the exhaustion prong. Requires layer-2 vet.
 */
/* This file is a NOTE — no compilable form. See candidate.c for the
 * best-form on disk (identical to s1). */
