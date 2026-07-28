/* REJECTED s2: score 39, 170 insns. Dropping the named dy2 and writing
 * (dy * 2) inline at all three use sites: CSE does NOT keep a single
 * call-crossing temp — the sll is recomputed post-call (+2 insns).
 * The named dy2 local (callee-save $s0 across the sqrt call) is load-bearing. */
