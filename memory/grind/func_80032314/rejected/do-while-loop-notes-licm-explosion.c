/* REJECTED (s2): spelling the loop as `do { ... } while (t1 < 4);` instead of
 * the goto-form.
 *
 * WHY TRIED: flow.c weights reg_n_refs by loop_depth (`reg_n_refs[regno] +=
 * loop_depth`, flow.c:2081/2329/2515/2725), but loop_depth comes ONLY from
 * NOTE_INSN_LOOP_BEG/END notes, which the goto-form never emits. With real
 * loop notes, inside-loop refs double, and the predicted allocno priorities
 * (mflo1 8000 > ent 5714 > walker 5487) land in exactly the target order.
 *
 * MEASURED RESULT: score 59, build_insns 122 (vs 109). The same notes that
 * feed flow's weighting arm loop.c: LICM hoisted ~6 loop-invariant constants
 * (the comparison immediates 4/0x14/0xF/0x11/0x16/0x13 into $t9/$t8/$t7/$t6/
 * $t5/$t3 via li, and &D_80101EC8 into $s1), grew the frame to 24 for s-reg
 * saves, and created a giv (-9($a3) addressing). Target keeps every constant
 * materialized per-iteration — irrecoverable without defeating 6+ separate
 * hoists.
 *
 * ALSO DEAD (analysed, not measured): making loop.c skip the loop by giving it
 * a second entry (`goto entry;` into the body marks the loop invalid in
 * find_and_verify_loops) does not survive: jump1 runs BEFORE loop.c and
 * deletes a jump-to-following-insn (notes are not insns), removing the label
 * ref and restoring the loop's validity. Entering at any non-top point changes
 * emitted instruction order. The loop-notes path is closed both ways.
 *
 * (Body identical to candidate.c except `loop:`/`goto loop` replaced by
 * do/while — not reproduced here; see candidate.c.)
 */
