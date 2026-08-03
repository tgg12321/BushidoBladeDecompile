/* REJECTED — grind session s3 (2026-08-03, modality: structural).
 *
 * Base for every measurement below: memory/grind/func_80060544/candidate.c
 * (`sandbox func_80060544 --disable all` == 2, build_insns 133 == target_insns).
 * Harness: tmp/grind/func_80060544/s3/sweep3.py.
 *
 * WHY THESE ARE DEAD.  The whole residual is the placement of
 * `addu $a1,$zero,$zero` inside the Case3 arm (target emits it FIRST in the
 * block, our build emits it between the `la` pair and the `sw p_static`).
 * s2 killed every intra-arm statement ordering and every call-argument
 * respelling.  s3 attacked the two structural axes that were left — the SHAPE
 * of the i-dispatch that creates the Case3 basic block, and the SHAPE of the
 * enclosing loop — plus the declaration-order / type-narrowing levers.
 * All measured >= 2; nothing moved the a1 set-up.
 *
 * ---------------------------------------------------------- dispatch shape
 *   d_ifelse           18  (insns 126) — natural if / else-if chain
 *   d_last_first       22  (insns 122) — if/else chain, (i == last) tested FIRST
 *   d_switch           36  (insns 137) — real `switch (i)` with case 1/2 shared
 *   d_goto_last_first  14  (insns 126) — goto ladder, (i == last) hoisted first
 * Each of these COLLAPSES blocks (insn count moves off 133 in every case), so
 * the m2c goto ladder currently in candidate.c is not merely one spelling among
 * many: it is the only dispatch shape that reproduces target's block structure.
 * The four-way `i<3 / i>0 / i==0 / i==last` ladder with three separate labelled
 * arms and a shared `Skip:` join IS what the original compiled to.
 *
 *   Rejected form (representative — d_ifelse):
 *       if (i == 0) {
 *           stat = (s32)(&D_8009B7D8);
 *           s.p_static = (s32 *)stat;
 *       } else if (i < 3) {
 *           stat = (s32)(&D_8009B800);
 *           s.p_static = (s32 *)stat;
 *       } else if (i == last) {
 *           stat = (s32)(&D_8009B7D0);
 *           s.p_static = (s32 *)stat;
 *           s.pad0C = mid_off;
 *           mid_off = func_80073728(&s, 0);
 *       }
 *
 * -------------------------------------------------------------- loop shape
 *   l_inc_swap          2  (insns 133) — `idx += 0xC;` before `i += 1;`  INERT
 *   l_idx_inc_early     4  (insns 132) — `idx += 0xC;` moved to the top of Skip
 *   l_for               6  (insns 133) — `for (i = 0; i < 4; i++)` header form
 *                                        (also destroys the s1 `last = 3`
 *                                        prologue lever, which needs `i = 0;`
 *                                        and `idx = 0;` as separate statements
 *                                        with `last = 3;` BETWEEN them)
 *   l_while             2  (insns 133) — `while (i < 4) { ... }`          INERT
 *
 * ------------------------------------------- declaration order / type axis
 *   t_stat_ptr          2  (insns 133) — `s32 *stat;` carrying the table
 *                                        pointer as a pointer, no casts   INERT
 *   t_stat_decl_last    2  (insns 133) — `stat` declared last             INERT
 *   t_stat_decl_first   2  (insns 133) — `stat` declared first            INERT
 *   c_gameobj_cast      2  (insns 133) — `func_80073728((GameObj *)(&s), 0)`,
 *                                        the spelling every OTHER call site of
 *                                        this callee in text1b.c uses     INERT
 *
 * The four INERT results matter as much as the regressions: the arm's local
 * spelling, the carrier's type, and the declaration order are all folded away
 * by GCC before the placement decision is made.  Combined with s2's five inert
 * call-argument respellings, the C-level surface around this basic block is
 * measured flat in every direction.
 */
