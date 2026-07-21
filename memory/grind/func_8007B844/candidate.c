/* func_8007B844 — WIP candidate body (sandbox --disable all == 6, HEAD == 7).
 *
 * Apply to src/display.c (replace HEAD's func_8007B844 body) to resume from
 * the score-6 floor. Verify:
 *
 *   & tools/eng.ps1 sandbox func_8007B844 --disable all   # expect "score": 6
 *
 * This is Lever B from .claude/rules/register-alloc-pure-c.md applied to
 * this function: a named intermediate `u32 mask;` that lets cc1 keep the
 * 0xFFFFFF mask in a named pseudo separate from the OT pointer flow. The
 * lever closes the addr-register diff (mine now in $v1 matching target's
 * $v1) but leaves a 6-diff residual cascade because:
 *   - cc1 picks $v0 for the mask (target uses $a0)
 *   - return-value staging (`move $v0, $s0`) emits AFTER the store
 *     (target emits it BEFORE)
 *
 * Mechanism (instrumented via the sched.c finding):
 *   Target requires `move $v0, $s0` (return-staging) scheduled at idx 23
 *   BEFORE the mask/addr chain (idx 24-26). cc1's sched.c gives return-
 *   staging INSN_PRIORITY = 1 (chain depth to jr ra is 1); the mask/addr
 *   chain has priority 4 (lui->addiu->and->sw). Target's schedule requires
 *   return-staging priority >=4. To extend the return-staging pseudo's
 *   chain depth in pure C, a later instruction (the store) must consume
 *   the return-value register $v0 — which in turn requires GCC to keep
 *   `ret_val` as a separate pseudo from `ot` after copy-prop.
 *
 * No pure-C SOTN-allowed construct prevents this copy-prop:
 *   - `u32 *p = ot; *p = ...; return p;` — folds to ot via copy-prop
 *   - `u32 *p = ot; ...; *p = mask; return ot;` — same fold
 *   - Mixed-exit `if (debug>=2) {...; goto end;}` — adds a branch that
 *     doesn't help return-staging chain depth
 *
 * The previously-rejected closing forms (preserved at rejected/) are both
 * FORBIDDEN per the 2026-06-02 cheat catalog:
 *   - rejected/conditional_dead_store.c — Lever D family (forbidden)
 *   - rejected/fnptr_return_type_lie.c — return-type-lie cheat (forbidden)
 *
 * NEXT-SESSION HYPOTHESES (see meta.json next_hypotheses for the full list):
 *   1. Fresh SOTN borderline-research pass — look for community patterns
 *      that affect sched.c return-staging priority specifically.
 *   2. Instrumented BB2_SCHED_DEBUG dump on the score-6 form to identify
 *      what sched.c sees for the return-staging pseudo's REG_DEP_TRUE
 *      predecessors. The closing lever must extend that chain WITHOUT
 *      emitting bytes (combine-foldable) AND without coercing register
 *      allocation in a way that hits the no-new-park-categories family.
 *   3. Sibling cross-reference — does any other thin GPU vtable-dispatch
 *      wrapper match pure-C without the same trick? If so, what natural
 *      C structure made its return-staging priority high enough? Candidates
 *      to check: sibling thin gpu_LinkList-style wrappers in display.c.
 */

u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) g_gpu_debug_func(&D_80015F98, ot, n);
    { u32 *v0 = g_gpu_dev_table; ((void(*)(u32*,s32))v0[11])(ot, n); }
    mask = 0xFFFFFF;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
