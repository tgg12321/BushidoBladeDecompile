/* REJECTED — s14 header-type-correction void*[] pointer flip: MEASUREMENT INERT +
 * OUT-OF-SCOPE per [[header-type-correction-from-use-sites]] scope-limit clause.
 *
 * Form: `extern void *g_player_ptrs[];` (both sites: text1a.c:163, text1a.c:898).
 *
 * Sandbox measurement: score=2, target_insns=82, build_insns=82, rules_dropped=3,
 * cheat_asm_stripped=23 — BYTE-IDENTICAL to baseline.  Scalar->pointer flip
 * produces identical RTL through combine/CSE, does not perturb BB18 sched1.
 *
 * Scope violation: rule line 183-185 explicitly EXCLUDES this variant:
 *   "Does NOT sanction changing a type from a struct/union/pointer to a
 *    scalar or vice versa. Ground-truth structural claims about the binary
 *    need their own SOTN-evidence pathway."
 *
 * The rule covers signedness flips only; scalar<->pointer is out of scope
 * regardless of measurement outcome.  This form is measured purely as a
 * comprehensive exhaustion of the ledger's frontier[1] surface — even a
 * hypothetical byte-improvement here could not be committed via this rule.
 *
 * Combined with header-type-flip-g-player-ptrs-u32.c (s14): every reachable
 * variant of the g_player_ptrs header-type-correction axis is measurement
 * inert (score stays 2).  Frontier[1] KILLED.
 */
