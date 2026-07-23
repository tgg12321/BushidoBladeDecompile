/* REJECTED (s2 structural) — derive bitpos = a3_off << 3 fresh each iter
 * (drop carried bitpos=0 init + bitpos+=0x10). sandbox = 14, build_insns=66.
 *
 *   do { ... arg3 = arg3 + a3_off; bitpos = a3_off << 3; ... }
 *   (a3_off steps 0,2 -> bitpos 0,16 = correct; a3_off is low-pri so its
 *    extra ref does NOT steal from i, unlike the i<<4 form.)
 *
 * INTENT: shorten bitpos live range (fresh, not loop-carried) to raise its
 * allocno pri above sum. IT WORKS at the RA layer: greg ALLOCDBG shows
 * bitpos(79) livelen 57->24, pri 5000 -> $t1($9); sum -> $t2($10); return
 * `move $2,$10` = TARGET RA.
 *
 * WHY REJECTED: recompute DROPS the loop-carried `addiu $t1,$t1,0x10` that
 * TARGET HAS (build_insns 66 vs 67) and emits an inline `sll` instead ->
 * structural opcode diffs. Same failure MODE as recompute-bitpos-i-shl4.c
 * (also 14). Target's bitpos IS loop-carried (long livelen) yet still wins
 * $t1 — so the flip cannot come from decoupling bitpos. KILLED: the entire
 * "recompute bitpos to shorten its live range" family is dead regardless of
 * the source induction var (i<<4 -> 14, a3_off<<3 -> 14).
 */
