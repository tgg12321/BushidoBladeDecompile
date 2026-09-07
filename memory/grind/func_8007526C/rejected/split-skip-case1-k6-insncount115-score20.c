/* REJECTED s15: case 1 un-split, cases 3/2/4 split into six addends.  score 20, build_insns
 * 91, loop insn_count 115.  Dead on ARITHMETIC, not shape: at insn_count 115 the
 * threshold decay (loop.c:1904 `threshold -= 3`) still leaves 119 and 116 >= 115 for the
 * first two constants, so regno 124 and 126 hoist and only 127/128 are rejected.  The
 * axis needs insn_count >= 120; eight addends (candidate.c) gives 123.
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
            v3 = *(u16 *)(p + 0xC);
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 1;
            v3 = v3 + 1;
            *(u16 *)(p + 0xC) = v3;
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
            v4 = *(u16 *)(p + 0xC);
            v4 = v4 - 2;
            v4 = v4 - 2;
            v4 = v4 - 2;
            v4 = v4 - 2;
            v4 = v4 - 1;
            v4 = v4 - 1;
            *(u16 *)(p + 0xC) = v4;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v5 = *(u16 *)(p + 8);
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 1;
            v5 = v5 - 1;
            *(u16 *)(p + 8) = v5;
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 2;
            v6 = v6 - 2;
            v6 = v6 - 2;
            v6 = v6 - 2;
            v6 = v6 - 1;
            v6 = v6 - 1;
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
