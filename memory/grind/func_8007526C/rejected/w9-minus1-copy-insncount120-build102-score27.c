/* REJECTED -- s14 (2026-09-07, synthesis).  score 27, build_insns 102, loop insn_count 120,
 * all four switch comparison constants `not desirable` (the target's movable shape).
 * FORM: s13's w9 (10 duplicated 3-insn tails) with ONE copy removed -- the case-3
 * inner-if copy -- landing loop insn_count on exactly 120, the measured threshold.
 * MEASURED KILL of s13's frontier item 2: removing a copy that jump2 was NOT merging does
 * NOT return an emitted word.  insn_count fell 124 -> 120 while build_insns stayed at 102,
 * i.e. the emitted cost of this duplication geometry is QUANTIZED and does not respond to
 * copy count.  Removing a second copy would drop insn_count below 120 and re-hoist the
 * constants, so 102 (11 words above the 91-word target) is the floor of this axis as
 * currently spelled.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    p = base;
    i = 0;
    do {
        lim = 0xC8;
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
                i++;
                p = base + i * 2;
                break;
            }
            i++;
            p = base + i * 2;
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
                i++;
                p = base + i * 2;
                break;
            }
            i++;
            p = base + i * 2;
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
                i++;
                p = base + i * 2;
                break;
            }
            i++;
            p = base + i * 2;
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
                i++;
                p = base + i * 2;
                break;
            }
            i++;
            p = base + i * 2;
            break;
        default:
            i++;
            p = base + i * 2;
            break;
        }
    } while (i < 2);
}
