/* s6 REJECTED (measured worse, not a cheat): shared-tail accumulation forms.
 * On the 12-form chassis, replacing `pkt = lo | 0xE3000000; return hi | pkt;`
 * with accumulation into lo:
 *   (T_B)  lo = lo | 0xE3000000;  return hi | lo;            -> 13/51
 *   (T_A)  lo = lo | 0xE3000000;  pkt = hi | lo; return pkt; -> 13/51
 * Both produce the target's or-shape (`or rd,rd,const`: dest==src1==lo) but
 * CANNOT free lo from hard_conf ∋ $v0: in T_A the const holder (pseudo 115)
 * becomes the tail block's local qty (QTYDBG row: blk 12, main pass, got $2)
 * and its $v0 assignment overlapping lo's range re-creates the hard conflict;
 * in the 12-form the pkt or-dest qty (80, sugg $2) plays the same role.
 * CONCLUSION (ground truth, s6/c7a0_tA.model.json): NO shared-tail spelling
 * can reach lo=$v0 — the const/or-dest qty always grabs $v0 first because
 * local-alloc cannot see the unallocated global pseudos lo/hi. The fix is
 * structural: per-arm returns (see candidate.c).
 */
