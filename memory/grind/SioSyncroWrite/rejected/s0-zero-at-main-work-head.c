/* REJECTED s1 (2026-08-18): moving the counter init `s0 = 0;` from before
 * `goto main_work;` to the HEAD of the main_work block, hoping reorg would
 * steal it from the (single-predecessor) taken thread into the entry beqz
 * delay slot the way target has it (target idx 15: `beqz ...; addu s0,zero,zero`),
 * collapsing our `j main_work; move s2,zero` trampoline pair:
 *
 *     if (*flag != 0) return -1;
 *     goto main_work;
 *     ...
 * main_work:
 *     s0 = 0;
 *     { volatile u16 *p_ae2 = ... }
 *
 * Measured: sandbox 31 (from 23). The trampoline pair DID collapse
 * (build_insns 160 -> 159) but the score regressed 8 points — the init at
 * main_work's head perturbs scheduling/allocation of the whole main_work
 * block (same failure family as the 2026-08-06 negative "hoist s0=0 above
 * the flag test": 27 -> 33). The entry-cluster fix must come WITH the
 * arg-home (a1 -> s4 in prologue) fix, not standalone: in target the beqz
 * delay slot is free for s0=0 precisely because the arg-home happened at
 * insn 3, whereas ours occupies that delay slot with `move s3,a1`.
 * Do not re-try s0-placement alone; attack the arg-home first.
 */
