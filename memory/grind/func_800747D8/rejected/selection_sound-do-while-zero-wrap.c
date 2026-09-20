/* REJECTED (session 3, permuter modality) — regressed the honest floor.
 * Baseline (candidate.c): `s32 sound = field64; if (sound==0) sound=4; else sound=0;`
 * measures sandbox --disable all score 6, build_insns 208 (== target_insns).
 *
 * A decomp-permuter campaign (tmp/grind/func_800747D8/s3/perm_ws, 21,884
 * iterations, 8 jobs, base_score 265 in the permuter's own unmasked metric)
 * surfaced output-170-1, which wraps the same-variable-reuse if/else in the
 * SOTN-sanctioned `do { ... } while (0);` construct
 * (.claude/rules/do-while-zero-exception.md, owner ruling 2026-07-06,
 * sanctioned for ANY codegen effect including register allocation):
 *
 *   s32 sound = MENU_800747D8->field64;
 *   do {
 *       if (sound == 0) {
 *           sound = 4;
 *       } else {
 *           sound = 0;
 *       }
 *   } while (0);
 *   func_8005C650(sound, 0x7F, 0x7F);
 *
 * Applied verbatim to src/text1b.c in place of candidate.c's plain if/else
 * (no other changes) and measured with `sandbox func_800747D8 --disable all
 * --diff`:
 *
 *   score REGRESSED 6 -> 7, build_insns grew by 1 (a new source-level insert
 *   hunk appeared at target[96]/ours[96], plus hunks 18-19 shifted by +1
 *   insn each). The do-while(0) wrap did NOT fold away identically on this
 *   already-branching if/else the way it does on the sanctioned family's
 *   usual targets (e.g. defeating LICM on a straight-line block) — here it
 *   left a residual extra branch/jump the target does not have.
 *
 * Verdict: KILLED (instance). This specific do-while(0) wrap, on this
 * chassis, with the same-variable-reuse selection_sound spelling, is worse
 * than the plain if/else baseline. Does not generalize to "do-while(0)
 * wrapping never helps this residual" (only this exact wrap point was
 * tried) — see hypotheses.md for the kill_scope statement.
 */
