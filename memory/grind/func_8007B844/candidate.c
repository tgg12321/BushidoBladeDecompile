/* func_8007B844 — grind candidate body (sandbox --disable all == 6, HEAD == 7).
 *
 * s5 (permuter, 2026-07-21): floor 6 re-confirmed start+end. Three MORE
 * fresh-seed campaigns on chassis geometries never randomized before
 * (172k iters total, all harvested+stopped in-session): D debug-split +
 * goto-end (60k iters), E struct-typed GpuDevice dispatch (62k iters),
 * F directed debug-arm x tail cross-product (51k iters). Every find was
 * score-equal 135 and fell into the three known classes: fold-transparent
 * temps/rebinds (fn-ptr dispatch temp sandbox-measured NEUTRAL 6 —
 * rejected/permuter_s5_fnptr_dispatch_temp.c; debug-split+two-local combo
 * NEUTRAL 6 — rejected/permuter_s5_debugsplit_twolocal_combo.c), seed
 * re-derivations, and the SAME wrong-semantics store-to-global attractor
 * from s4 (now reproduced in the struct-dispatch basin — 3 of 6 lifetime
 * chassis converge on it). One dead-temp-chain form vetted out as cheat
 * family (dead local written never read). Permuter modality is now dead
 * across FIVE chassis geometries + TWO directed cross-products, ~293k
 * lifetime iters. Frontier: F2 (sched forensics) + F3 (cross-project
 * ClearOTagR research) ONLY — both non-permuter.
 *
 * s4 (permuter, 2026-07-21): floor 6 re-confirmed start+end. Three fresh-seed
 * campaigns (~28 min each, 121k iters total, telemetry in metrics/events.jsonl):
 * A leverB-full-random (43k iters, only find = wrong-semantics store-to-global
 * attractor at equal score), B twolocal-random (42k iters, one novel find
 * output-125-1: arg-staging + tail scalar rebind — permuter 135->125 but
 * sandbox NEUTRAL at 6; banked rejected/permuter_s4_addr_arg_staging_rebind.c),
 * C directed cross-product of neutral levers dispatch x AND-order x exit
 * (36k iters, only the same wrong-semantics attractor). Permuter modality is
 * now measured DEAD around this plateau: random and directed basins converge
 * on fold-transparent rebinds or semantic drift; the weighted permuter metric
 * diverges from the masked honest distance on this residual. Frontier remains
 * F2 (sched-dump forensics) + F3 (cross-project ClearOTagR research).
 *
 * s3 (structural, 2026-07-21): floor 6 re-confirmed start+end. Filled the five
 * structural gaps s2's exhaustion claim had not measured: debug-guard local
 * split (6), dispatch byte-offset re-association (6), goto-end tail (6),
 * two-local AND-into-mask (6 — contrast s2's AND-into-addr = 7), mask-init
 * hoisted before dispatch only (20, build 40 — rejected/
 * mask_init_hoist_before_dispatch.c). Structural axis now exhaustively
 * MEASURED, not just claimed. Live frontier unchanged: F2 (sched-dump
 * forensics) + F3 (cross-project ClearOTagR research), both non-structural.
 *
 * s2 (structural, 2026-07-21): floor 6 re-confirmed; STRUCTURAL AXIS EXHAUSTED.
 * Killed this session: F1 struct-typed dispatch (both spellings neutral at 6),
 * two-local addr-rebind AND (7, decl-order-invariant), block-local/hoisted decl
 * placement (6), s32 mask retype (6), duplicated-tail-into-debug-arm ref-lift
 * (22 — cross-jump fails to re-merge, not byte-neutral). Remaining live axes
 * are F2 (instrumented sched.c dump forensics) and F3 (cross-project
 * ClearOTagR research) — see hypotheses.md.
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
