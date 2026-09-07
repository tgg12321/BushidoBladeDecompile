/* s14 (2026-09-07, synthesis) RE-MEASURED this body unchanged on the dispatch chassis:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91,
 *   .loop "Loop from 14 to 260: 91 real insns", lim (regno 75) moved and all four switch
 *   comparison constants (regnos 124/126/127/128, life 1, savings 1) moved to the pre-header.
 * MANDATED KILL RE-AUDIT: the closest banked form, rejected/arming-loop-after-main-score5.c,
 *   still measures score 5 / build_insns 93 with all four constants `not desirable`.  Neither
 *   body carries a FAKE construct, so tools/fake_ablate.py has no carrier to strip and every
 *   banked instance kill remains chassis-valid.
 * s14 kept this body because BOTH live frontier items measured dead:
 *   (a) frontier 1 -- "5 outer-exit copies of a tail containing no address recomputation merge
 *       perfectly" is FALSE on this chassis: `i++` alone at the five outer exits measures
 *       96 / 95 (+5 loop insns for +2 emitted words) and the 3-insn tail measures 108 / 97.
 *       s13's perfect merge (w2, +9 / +0) is a property of the POINTER-BUMP chassis only.
 *   (b) frontier 2 -- "each removed non-merging copy returns ~1 emitted word" is FALSE:
 *       w9 minus one copy lands loop insn_count on exactly 120 (all four constants rejected)
 *       with build_insns UNCHANGED at 102.  The duplication axis's emitted cost is quantized
 *       and its floor at insn_count 120 is 11 words above the 91-word target.
 *   (c) NEW CLASS KILL -- `do { ... } while (0);` cannot arm the moved_once doubling: loop.c
 *       calls it phony and returns before scanning (loop.c:568-575, scan_start is not a
 *       CODE_LABEL once jump1 deletes the unreferenced top label).  Measured at three
 *       placements inside the main loop, all 13 / 93 / 91.
 * The two live axes and their measured floors are now: the moved_once ARMING axis, which
 * reproduces the target's movable shape exactly at build_insns 93 / score 5 and whose only
 * residual is the arming loop's own two emitted instructions; and the insn_count >= 120 axis,
 * whose cheapest measured form is build_insns 102 / score 27.  See evidence.md s14.
 */
/* s13 (2026-09-07, structural) RE-MEASURED this body unchanged on HEAD 34eb8142:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91,
 *   .loop "Loop from 14 to 260: 91 real insns" with lim (regno 75) and all four switch
 *   comparison constants (regnos 124/126/127/128, life 1, savings 1) moved to the pre-header.
 * Still the best NON-BANNED form.  s13 kept it because:
 *   (a) local DECLARATION ORDER is completely inert here -- all five permutations of
 *       base/p/i/lim measure score 13 / build 93 / insn_count 91 and only renumber the
 *       pseudos (75 -> 72/73/74), so local-alloc's $8/$9/$10/$11 assignment does not respond
 *       to declaration order (s12 frontier item 3, now killed);
 *   (b) per-access address arithmetic (*(u16 *)(base + i * 2 + K), no p local) is NOT free
 *       loop-time insn_count -- cse1 runs before loop and collapses it back to 91;
 *   (c) the duplicated-tail axis (s12 frontier item 1) DOES reach the goal mechanically --
 *       at insn_count >= 120 all four constants print "not desirable" -- but the cheapest
 *       measured form that gets there is build_insns 102 against a 91-word target.
 * Full table: tmp/grind/func_8007526C/s13/scores.txt.  See evidence.md s13.
 */
/* s12 (2026-09-07, structural) RE-MEASURED this body unchanged on HEAD f2842664:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91.
 * Still the best NON-BANNED form.  s12 kept it because all three of s11's frontier items measured
 * dead: the semantically real two-pass state split arms the moved_once doubling but halves the
 * loop's insn_count so the doubling loses (63/99); arming without a second loop is impossible
 * (moved_once is written only at loop.c:1912, reachable only through the single move_movables call
 * at loop.c:966 inside scan_loop); and every permutation of the switch case-label order is worse
 * than this body's own 1/3/2/4 (31 / 55 / 60).  s12 also priced out the threshold-decay route --
 * the target's whole pre-header is three instructions with no spare slot for a hoisted movable --
 * and found ONE thing that is not dead: loop-time insn_count and emitted build_insns are NOT
 * coupled 1:1 once a real statement is duplicated into the switch arms (jump2 cross-jumps the
 * copies back after loop.c has counted them).  See evidence.md s12.
 */
/* s11 (2026-09-07, rederive) RE-MEASURED this body unchanged on HEAD 7ab27738:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91.
 * It remains the best NON-BANNED form.  s11 kept it because four structurally different
 * re-derivations all measured worse: hand-written if/else dispatch tree (29), the matched
 * sibling func_80075670's u16-element array model (47 / 60), its `s16 i` counter (28), and a
 * named 0xA step holder (13, bit-identical baseline -- cse1 folds it into the addiu immediate).
 * The 13 points are entirely the four switch-comparison constants being hoisted into
 * $8/$9/$10/$11; the target keeps them in-loop in a reused $v0.  See evidence.md s11 for the
 * closed-out loop.c arithmetic.
 */
/* candidate for func_8007526C (src/text1b.c) -- s10 (2026-09-07), ORDINARY C, real do-while loop.
 *
 * MEASURED THIS SESSION on the dispatch chassis with the current unmodified build configuration:
 *   `sandbox func_8007526C --disable all` -> score 13, build_insns 93, target_insns 91
 *   (identical score/insns to the s1 candidate it replaces; that body is kept verbatim below
 *    except for the named 0xC8 holder).
 *
 * WHY THIS BODY AND NOT THE s1 ONE (both score 13): naming the 0xC8 constant as a local
 * `lim` and assigning it at the TOP of the loop body, before the switch, is FREE (score and
 * build_insns unchanged, loop insn_count 92 -> 91) and it moves that movable to POSITION 1 of
 * move_movables' scan list.  Dump proof (tmp/grind/func_8007526C/s10/vB_limtop.loop):
 *     Loop from 14 to 260: 91 real insns.
 *     Insn 19:  regno 75  (life 63), move-insn savings 1  moved to 268   <- lim, scanned FIRST
 *     Insn 226/232/238/241: regno 124/126/127/128 (life 1, savings 1) moved  <- the 1/2/3/4
 * In the s1 body the four switch-comparison constants were scanned FIRST and the 0xC8 movable
 * last, so nothing could ever decay `threshold` ahead of them.  With this body they are scanned
 * SECOND, at threshold 119 instead of 122 (loop.c:1904 `threshold -= 3`), and -- far more
 * importantly -- regno 75 is now the first regno move_movables touches in this loop, which is
 * the ONLY hook by which loop.c:1609-1611's `if (moved_once[regno]) insn_count *= 2;` can fire
 * before the four constants are considered.  s10 measured that hook: with a second (dead) loop
 * after the main loop that also moves `lim`, all four constants print `not desirable`, the
 * 0xC8 still hoists, and the score falls 13 -> 5 (rejected/arming-loop-after-main-score5.c).
 * That arming loop is not admissible C, but the chassis it needs is this one.
 *
 * The whole remaining 13-point residual is still the single move_movables desirability test
 * `(threshold * savings * m->lifetime) >= insn_count` at tools/gcc-2.7.2/loop.c:1631, with
 * threshold = 2 * (1 + n_non_fixed_regs) = 122 (loop.c:532, hard float) against insn_count 91.
 * The byte-proven answer is threshold 58 (-msoft-float), which this pipeline may not spend.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
