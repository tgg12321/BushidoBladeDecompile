/* REJECTED (session 2, structural) — confirms and explains the session-1
 * two-var-split / default-then-override kills with a NAMED mechanism.
 *
 *     s32 flag = MENU_800747D8->field64;
 *     s32 sound = 4;
 *     if (flag != 0) {
 *         sound = 0;
 *     }
 *     func_8005C650(sound, 0x7F, 0x7F);
 *
 * MEASURED (session 2): sandbox --disable all score 10, build_insns 205 (3
 * SHORT of target 208) — reproduces the session-1 default-then-override
 * result exactly. `--diff` this session additionally shows WHERE the 3
 * insns go missing (tmp/grind/func_800747D8/s2/diff_defaultoverride.txt,
 * hunks 17-19): our build folds the entire flag-test + literal-select
 * sequence to
 *
 *     lbu   a0,0x64(v0)
 *     sltiu a0,a0,1
 *     sll   a0,a0,0x2
 *
 * i.e. `a0 = (flag == 0) * 4` — a fully BRANCHLESS arithmetic fold of the
 * `flag != 0 ? 0 : 4` ternary. Target keeps a real branch here (`beqz v0,...`
 * + delay-slot `li a0,4` + fallthrough `move a0,zero`), so the branchless
 * fold is strictly WRONG-SHAPE, not just wrong-register — it eats 3 real
 * instructions (the branch, and one of the two literal-set insns merge into
 * the shift). Mechanism: GCC's constant-propagation/combine recognizes the
 * canonical `single-use-flag ? const_a : const_b` shape (a FRESH pseudo
 * `flag`, dead immediately after the test, feeding a two-constant
 * conditional-select into a SEPARATE result pseudo `sound`) as a candidate
 * for its if-conversion-style branchless synthesis (the store-flag /
 * `movsi_internal2`-with-`REG_EQUAL` idiom visible in the .combine dump at
 * tmp/grind/func_800747D8/dumps/text1b.combine:66263+, insns 278/282/290/295
 * for the *baseline* two-value shape — same insns, this variant lets a LATER
 * pass (jump2/final combine) go one step further and collapse the whole
 * thing to sltiu+sll because `flag` has no other use). kill_scope: instance
 * (this exact flag+literal spelling, this chassis, no FAKE constructs).
 * See also rejected/selection_sound-two-var-split.c and
 * rejected/selection_sound-default-then-override.c (session 1) and
 * rejected/selection_sound-single-shot-no-intermediate.c (session 1) — all
 * four spellings that introduce ANY separate/isolated single-use test value
 * fold identically to this branchless shape. The KEY, confirmed this
 * session: reusing the SAME variable across load + test + result (as the
 * new candidate.c does, with plain `sound = 4;` / `sound = 0;` literal
 * assignments instead of the old `sound -= sound;` self-subtract) reaches
 * build_insns == target_insns == 208 exactly like the self-subtract
 * baseline, because the read-modify dependency on ONE pseudo across the
 * load and the branch defeats the same-pseudo-isolation precondition this
 * branchless fold needs. Branch source order (== 0 first vs != 0 first)
 * measured IDENTICAL (tmp/grind/func_800747D8/s2/diff_swapped.txt) — GCC
 * normalizes branch polarity independent of source order for this shape.
 */
