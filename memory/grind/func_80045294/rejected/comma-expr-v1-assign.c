/* KILLED s21 (2026-07-19): the "NEXT LEVER" candidate from
 * cse-fold-anon-shift.c — express a0<<4 exactly ONCE and have
 * that expression target v1 directly (not an anonymous pseudo)
 * via a comma-expression inside s4's initializer:
 *   s32 v1;
 *   s32 s4 = (v1 = a0 << 4, *(s32 *)((u8 *)&D_800EED14 + v1));
 *
 * Hypothesis: comma-expression makes v1 the SOLE pseudo for the
 * shift (no anon temp -> no CSE-move copy) while positioning
 * the ashift's LUID inside s4's init tree (later than the
 * would-be i=a0 move16 that follows).
 *
 * Measurement: sandbox --disable all -> score=2, target_insns=83,
 * build_insns=83, rules_dropped=0. Objdump shows the SAME
 * sll-before-move16 prologue order as baseline candidate.c:
 *     sll  v1,s2,0x4       (2630)
 *     sw   s0,16(sp)
 *     move s0,s2           (2638)
 * i.e. the 2-insn residual is UNCHANGED — same shape, same score,
 * same diff bytes.
 *
 * MECHANISM: GCC 2.7.2 lowers `s32 s4 = (v1 = a0<<4, deref);`
 * IDENTICALLY to the two-statement form `s32 v1 = a0<<4; s32 s4 =
 * deref;` at the RTL LUID level. The comma-operator's left operand
 * (assignment expression) is expanded first as its own STMT; the
 * ashift RTX gets the same LUID position it would in the two-stmt
 * form. `s4`'s init RTX is emitted after, at a later LUID. NO
 * nesting of the ashift inside s4's tree occurs because
 * expand_expr routes COMPOUND_EXPR through separate expand_stmt
 * calls for the LHS effect and the RHS value.
 *
 * WHAT IT PROVES: the cse-fold-anon-shift.c "NEXT LEVER" note
 * (2026-07-18) proposed the comma-assignment as a way to eliminate
 * the extra CSE-move copy without pushing sll's LUID back down.
 * The hypothesis is FALSE: the comma-expression does not decouple
 * the two axes. It collapses to the SAME LUID relationship as the
 * baseline candidate.c, reproducing the wrong sched2 order rather
 * than reaching cse-fold-anon-shift's correct-order-plus-copy
 * shape. There is no intermediate spelling between these two
 * mechanically-distinct forms.
 *
 * VERDICT KILLED: comma-expression init merges with the baseline
 * shape at LUID assignment, not with the anonymous-shift shape.
 * The one un-tested un-banked "middle path" between
 * cse-fold-anon-shift.c (sw/move16/sll + copy, score 2 / 84 insns)
 * and baseline (sll/sw/move16, score 2 / 83 insns) is measured
 * dead — no middle path exists at this level of the C front-end.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1;
    s32 s4 = (v1 = a0 << 4, *(s32 *)((u8 *)&D_800EED14 + v1));
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate.c ... */
}
