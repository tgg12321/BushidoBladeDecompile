/* REJECTED s15: the four memory-to-memory `*(u16 *)(p+0x14) = *(u16 *)(p+0x18);` copies and
 * the two `+ 1` increments staged through named locals (ordinary C, no split needed to
 * justify it), on top of case-1-un-split four-addend splits.  score 10, build_insns 92,
 * loop insn_count 113.  Dead: eight staged statements buy only +6 loop insns AND cost
 * emitted words (92 vs the 90 the un-staged four-addend form gives), so staging cannot
 * substitute for split depth on the way to insn_count >= 120.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 v3;
    s32 v4;
    s32 v5;
    s32 v6;
    s32 t1;
    s32 t2;
    s32 t3;
    s32 t4;
    s32 t5;
    s32 t6;

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
                    t5 = *(u16 *)(p + 0x10);
                    t5 = t5 + 1;
                    *(u16 *)(p + 0x10) = t5;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                t1 = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x14) = t1;
                t2 = *(u16 *)(p + 0x38);
                *(u16 *)(p + 0x3C) = t2;
            }
            break;
        case 3:
            v3 = *(u16 *)(p + 0xC);
            v3 = v3 + 3;
            v3 = v3 + 3;
            v3 = v3 + 2;
            v3 = v3 + 2;
            *(u16 *)(p + 0xC) = v3;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                t3 = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x14) = t3;
                t4 = *(u16 *)(p + 0x38);
                *(u16 *)(p + 0x3C) = t4;
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    t6 = *(u16 *)(p + 0x10);
                    t6 = t6 + 1;
                    *(u16 *)(p + 0x10) = t6;
                }
            }
            break;
        case 2:
            v4 = *(u16 *)(p + 0xC);
            v4 = v4 - 3;
            v4 = v4 - 3;
            v4 = v4 - 2;
            v4 = v4 - 2;
            *(u16 *)(p + 0xC) = v4;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v5 = *(u16 *)(p + 8);
            v5 = v5 - 3;
            v5 = v5 - 3;
            v5 = v5 - 2;
            v5 = v5 - 2;
            *(u16 *)(p + 8) = v5;
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 3;
            v6 = v6 - 3;
            v6 = v6 - 2;
            v6 = v6 - 2;
            *(u16 *)(p + 0xC) = v6;
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
