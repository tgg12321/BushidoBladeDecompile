/* REJECTED forms — all measured s1, all score 16 / 119 insns (d0-init store +
 * its copy deleted). Root cause identical in all four: with `s.p0 = &D_8009B6FC;`
 * placed BEFORE the d1/d0 init pair (or with a chain form putting d0's store
 * second), flow.c's last_mem_set tracker (flow.c:1740/1988) sees
 * `s.d0 = <init>` followed by `s.d0 = arg0%10` with no intervening mem store
 * -> deletes the init store; combine then cascade-deletes the single-use copy.
 *
 * Form 1 (probe1): chain, d0 outer:
 *     s.p0 = &D_8009B6FC;
 *     s.d0 = (s.d1 = arg0);          // sh 4A emitted, 48-init DELETED
 *
 * Form 2 (probe2): hi carrier, p0 first:
 *     s.p0 = &D_8009B6FC;
 *     hi = arg0; s.d1 = hi; s.d0 = hi;   // same deletion
 *
 * Form 3 (probe4): constant-index address spelling:
 *     (&s.d0)[0] = ((s16)arg0) % 10;     // folds to direct mem at EXPAND
 *     (&s.d0)[1] = hi % 10;              // -> rtx_equal at flow -> deleted
 *
 * Form 4 (probe5): pointer variable:
 *     s16 *dp = &s.d0;
 *     dp[0] = ...; dp[1] = ...;          // cse canonicalizes address to
 *                                        // fp+48 before flow -> deleted
 *
 * CONCLUSION (measured): the ONLY flow-safe cheat-free protector for the d0
 * init store is a real store to another slot between it and its overwrite —
 * i.e. source order pair-THEN-p0 (the candidate.c form). Address-spelling
 * games cannot beat expand folding + cse canonicalization.
 *
 * NOTE: the old HEAD chain `s.d1 = (s.d0 = arg0);` keeps both stores (48-init
 * is protected by the 50-store evicting last_mem_set) but emits 48,4A order —
 * target is 4A,48. Wrong order, needs the sh-swap regfix. Also rejected.
 */
