/* REJECTED -- s21 (2026-09-07, rederive).  score 28, build_insns 97, outer insn_count 99.
 * s21's c_innerloop with `lim = 0xC8;` added INSIDE the inner loop so that regno 75 is a
 * movable of the inner (earlier-scanned) loop.  The arming half works exactly as s19
 * predicted: the inner scan prints "Insn 126: regno 75 (life 69), global move-insn savings 1
 * moved to 300", so moved_once[75] is set before the outer loop is scanned.  The doubling
 * still never fires, because the moved insn is deposited in the INNER loop's pre-header,
 * which is inside the outer loop -- regno 75 now has TWO sets in the outer loop (the loop-top
 * `lim = 0xC8;` at insn 19 and the moved one), so scan_loop builds no outer movable for it at
 * all (loop.c:706) and there is nothing left to double insn_count.  The outer .loop list
 * contains only the four dispatch constants, all four moved.  A nested arming loop and a
 * loop-top movable of the same pseudo are mutually exclusive. */
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
