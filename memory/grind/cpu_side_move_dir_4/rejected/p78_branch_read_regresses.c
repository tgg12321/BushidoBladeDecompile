/* s78 forensics probe P2: if (idx_1495 == (u8 *)-1) return -1; placed before
 * `poll:` label.
 *
 * VERDICT: KILLED — masked=15, build_insns=163 (+3 vs baseline 160). NOT
 * byte-neutral. The real branch survives flow.c DCE (cannot prove idx_1495
 * != (u8*)-1 at compile time) and emits +3 real instructions (li const, beq,
 * delay+return path). The p78 refs-lift does materialise but rides in on
 * top of a non-neutral perturbation, disqualifying it from the H2-extension
 * isolation criterion.
 *
 * Together with P1 (see p78_self_assign_dce_invisible.c), this closes the
 * H2-extension byte-neutral single-carrier lift frontier for local scalar
 * pointer carriers: DCE deletes anything below the "emits at least one insn"
 * threshold, and anything above it emits at least one insn.
 */
saved = (*D_800A147C) & 3;
if (idx_1495 == (u8 *)-1) { return -1; } /* p78 read survives; +3 emitted insns */
poll:
