/* func_80083794 — session 4 (permuter modality) rejected proposals.
 *
 * Two decomp-permuter campaigns, 107,064 total iterations across two
 * structurally different chassis, produced exactly the three closing-form
 * families below. NONE of them lowers the honest sandbox floor (18); two are
 * cheats by the standing catalog and one is byte-neutral.
 *
 * Campaign A — `tmp/grind/func_80083794/s4/ws`, label `minimal-tu-clean-c`,
 *   base = the score-18 floor form, permuter base_score 383, 48,633 iterations
 *   over ~22 min, best permuter score 340 (found twice, at 522 s and 992 s).
 * Campaign B — `tmp/grind/func_80083794/s4/ws2`, label
 *   `class-D-hoisted-la-chassis`, base = the s3 hoisted-`la` form that DOES
 *   produce the target's ascending prologue save order, permuter base_score
 *   1168, 58,431 iterations over ~22 min, best permuter score 383 — i.e. the
 *   search drained straight back OUT of the ascending-save basin and landed
 *   exactly on campaign A's starting score, never going below it.
 *
 * ---------------------------------------------------------------------------
 * (1) REJECTED — CHEAT. Campaign A best, permuter score 340
 *     (`ws/output-340-1`, `ws/output-340-2`).
 *
 *     volatile unsigned char new_var;
 *     if (D_800A2668 == (1 * (new_var = 0))) { ... }
 *
 *     A dead `volatile` scalar local whose only purpose is to occupy a frame
 *     slot and perturb allocation, smuggled into the guard through a
 *     multiply-by-one so it is not syntactically a bare dead store. This is
 *     the volatile-coercion / dead-local family (`.claude/rules/
 *     inline-asm-policy.md` expanded catalog; `dead-vars-local-array`), and
 *     the `new_var` name is itself the naming-announces-intent signal.
 *     MEASURED: 29 emitted instructions against target's 28 — it is strictly
 *     WORSE on the honest metric, not merely inadmissible. The permuter score
 *     fell only because the extra `sb $zero,16($sp)` made the three callee-save
 *     stores contiguous, which its diff algorithm rewards; it does not flip
 *     $s0/$s1, does not shrink the frame (-40, worse than -32), and does not
 *     produce the `ori`.
 *
 * ---------------------------------------------------------------------------
 * (2) REJECTED — BYTE-NEUTRAL, no gradient. Campaign A, permuter score 378
 *     (`ws/output-378-1`, `ws/output-378-2`).
 *
 *     do { while (count != 0) { (*p++)(); count--; } } while (0);
 *
 *     The sanctioned do-while(0) wrapper. MEASURED DIRECTLY in the honest
 *     sandbox with this body spliced into src/ings2.c: score 18, build_insns
 *     28 — identical to the floor form. The permuter's 5-point delta is label-
 *     numbering noise, not a real byte difference. Since it moves nothing, the
 *     do-while(0) carve-out's prerequisites (documented lever-exhaustion,
 *     LABEL_OUTSIDE_LOOP_P / reorg.c mechanism, FAKE annotation) are moot here:
 *     there is nothing for it to buy. Do not re-propose it.
 *
 * ---------------------------------------------------------------------------
 * (3) REJECTED — no gradient. Campaign A, permuter score 383 (= base)
 *     (`ws/output-383-1`); and campaign B's best (`ws2/output-383-1`).
 *
 *     count = 1; D_800A2668 = count; count = (s32)&D_00000000; ...
 *     void (**new_var)(void) = &D_8008D070; ... p = new_var;
 *
 *     Staged-value / named-intermediate respellings of the flag store and the
 *     pointer `la`. Both tie the base score exactly; neither is below it.
 *     Campaign B reaching this form is the load-bearing datum: an unbiased
 *     108k-iteration search seeded INSIDE the ascending-save basin walks out of
 *     it rather than finding a form that keeps class D and recovers the 5
 *     points, which is independent corroboration of session 3's COUPLED-
 *     constraint argument (`rejected/hoisted-la-flips-save-order-but-costs-5.c`)
 *     by search rather than by mechanism.
 *
 * ---------------------------------------------------------------------------
 * The floor form is unchanged; see ../candidate.c.
 */
