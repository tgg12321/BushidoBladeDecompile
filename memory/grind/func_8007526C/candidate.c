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
