/* s3 REJECTED: V10 t0 chain fully compressed to one statement.
 * masked 9 (baseline 2) — collapses the multi-set t0 birth pattern.
 * Mechanism: single-stmt destination is a single-set pseudo →
 * birthing_insn_p LAUNCHES it → the sll/addu emit in launch-early
 * position, wrong for the target ordering.
 */
    t0 = (s32)((u8 *)tbl_125c + idx_1494[0] * 4);
