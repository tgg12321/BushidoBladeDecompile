/* s16 (2026-08-20) — the 2026-08-20 or-tree-shape-shift carve-out's
 * operand-order/grouping axis measured DEAD for this function on the
 * current text1a_post chassis (baseline sandbox --disable all = 8).
 *
 * (1) The ruling record's named grouping (decisions.md:9161 "(b|r)|g"):
 *       func_80041398((b | (r << 16)) | (g << 8));
 *     sandbox = 16, build_insns = 80 (combine refolds the reassociated
 *     tree and LOSES 2 insns vs target). DEAD.
 *
 * (2) Outer operand swap, association preserved:
 *       func_80041398(((r << 16) | (g << 8)) | b);
 *     sandbox = 9, build_insns = 82. FALSE-arm lbu order UNCHANGED
 *     ([r,g,b]) and the final or flips to `or a0,a0,v1` (b RIGHT) vs
 *     target's `or a0,v1,a0` (b LEFT). Source operand order reaches ONLY
 *     the final-or rs/rt, never the lbu schedule. DEAD.
 *
 * (3) Nested swap b | ((g<<8)|(r<<16)): not measured — the nested or is
 *     canonicalized (both (1)/(2) show `or a0,a0,v0` with r-sll left
 *     regardless of source), and the lbu's are STATEMENT insns the
 *     expression tree cannot move. Predicted byte-identical to baseline.
 *
 * MECHANISM (s16 SCHEDDBG, tmp/grind/func_80041688/s16/scheddbg.log,
 * block=18): the three color lbu's are statement insns; all get
 * ADJPRI-boosted to LAUNCH_PRIORITY (2130706433) via birthing_insn_p
 * (deaths=0, birth=1 i.e. reg_n_sets==1), so b-lbu (insn 191) is picked
 * at clock=4 as soon as its consumer or (198) schedules, sinking it below
 * the slls; g/r lbu's picked at clocks 7/8 emit first -> [r,g,b]. No
 * OR-expression order/grouping can alter this: the family's lever surface
 * does not reach statement-insn scheduling in this function. The carve-out
 * unpark is honored instead by the reg_n_sets lever (see candidate.c).
 */
