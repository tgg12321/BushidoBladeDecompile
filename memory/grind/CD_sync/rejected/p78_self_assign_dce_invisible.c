/* s78 forensics probe P1: idx_1495 = idx_1495; placed before `poll:` label.
 *
 * VERDICT: KILLED — byte-neutral (masked=2 unchanged) BUT zero ALLOCDBG /
 * QTYDBG delta vs baseline (0/102 pseudos differ, 0/276 QTYDBG entries differ).
 *
 * Mechanism: flow.c life_analysis / delete_noop_moves recognises
 *   (set p78 p78)
 * as a no-op set and deletes the insn BEFORE local-alloc runs. The pseudo's
 * reg_n_refs is unchanged. Consequently NO byte-neutral single-carrier
 * refs-lift is available for local scalar pointer pseudos p73/p77/p78/p79
 * via self-assign at any position: the DCE erases the extra ref before
 * qty_compare/priority arithmetic can observe it.
 *
 * See also rejected/p78_branch_read_regresses.c (P2: real branch that
 * survives flow.c but emits +3 insns, breaking byte-neutrality).
 */
saved = (*D_800A147C) & 3;
idx_1495 = idx_1495; /* deleted by flow.c delete_noop_moves before local-alloc */
poll:
