/* REJECTED -- s21 (2026-09-07, rederive).  score 29, build_insns 98, outer insn_count 99.
 * The fix for s21/d: `lim` is assigned ONLY inside the real inner loop, so after loop.c
 * hoists it to the inner pre-header it is the outer loop's single set of regno 75.  Removing
 * the loop-top `lim = 0xC8;` has a second, decisive consequence that this probe discovered:
 * the outer loop body now BEGINS with the switch's jump-to-dispatch, so loop.c:545 retargets
 * scan_start to the dispatch tree, and the outer movables are scanned dispatch-FIRST.  The
 * dump lists the four constants (insns 252/258/264/267) ahead of every body movable, so any
 * arming movable in an arm body is processed too late to double insn_count for them.  Under a
 * switch carrier the only movables that can precede the dispatch constants are those emitted
 * between the loop top and the switch statement. */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 k;

    base = D_800A36A0;
    i = 0;
    do {
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
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                for (k = 8; k <= 0xC; k += 4) {
                    lim = 0xC8;
                    *(u16 *)(p + k) = lim;
                }
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
