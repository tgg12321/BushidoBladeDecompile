/* REJECTED -- s21 (2026-09-07, rederive).  score 21, build_insns 97, outer insn_count 98.
 * Ordinary C, semantically exact: case 3's two identical resets `*(u16 *)(p + 8) = lim;
 * *(u16 *)(p + 0xC) = lim;` written as `for (k = 8; k <= 0xC; k += 4) *(u16 *)(p + k) = lim;`.
 * This is the first REAL (non-phony) nested loop found for this function -- the .loop dump
 * shows "Loop from 115 to 140: 5 real insns", biv 76 verified and eliminated, both givs
 * combined -- so it satisfies loop.c:570's CODE_LABEL scan_start requirement that every
 * do{}while(0) spelling fails.  It does NOT arm moved_once: `lim` has no set inside the
 * inner loop, so the inner scan builds no movable for regno 75 and the outer loop still
 * moves all four dispatch constants.  It costs 4 emitted words (build 93 -> 97).  Banked as
 * the price of a real inner loop in this function: any arming carried by one starts 4 words
 * over budget, and the budget is zero. */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 k;

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
                for (k = 8; k <= 0xC; k += 4) {
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
