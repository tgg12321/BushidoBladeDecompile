/* EXPLORATION PROBE, NOT A SUBMISSION -- s1 (2026-09-07).  Score 8 (build_insns 96 vs
 * target 91).  Rejected because the arming loop `for (k = 0; k < 2; k++) lim = 0xC8;` is a
 * dead loop with no semantic purpose: it exists only to make loop.c hoist `lim` twice.
 * That is a coercion construct in no sanctioned family, and it costs 5 final insns the
 * target does not have.  Banked ONLY as the measured proof of the moved_once mechanism.
 *
 * WHAT IT PROVES (tmp/grind/func_8007526C/dumps/text1b.loop after this body was applied):
 *   Loop from 20 to 43: 4 real insns.                    <- the arming loop, processed FIRST
 *   Insn 31: regno 76 (life 69), global move-insn savings 1  moved to 298
 *   Loop from 14 to 285: 95 real insns.                  <- the real loop
 *   Insn 298: regno 76 ... halved since already moved  moved to 308     <- 0xC8 still hoists
 *   Insn 251: regno 126 (life 1), move-insn savings 1 NOT DESIRABLE     <- constant stays!
 *   Insn 257 / 263 / 266: same, NOT DESIRABLE                           <- 1, 3, 4 stay!
 * i.e. move_movables' `if (moved_once[regno]) insn_count *= 2;` (tools/gcc-2.7.2/loop.c:
 * 1609-1611) doubled insn_count from 95 to 190 for every movable scanned after the first,
 * so 122*1*1 = 122 < 190 left all four switch constants inside the loop while the 0xC8
 * movable (122*1*... >= 190 via its own doubled check) still hoisted to the pre-header --
 * EXACTLY the target's shape, with no build-flag change.  Score 13 -> 8.
 * The open problem is only the 5-insn cost of the arming loop; see hypotheses.md [s1] F1.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 k;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        for (k = 0; k < 2; k++) {
            lim = 0xC8;
        }
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
