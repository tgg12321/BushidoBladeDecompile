/* REJECTED (s15b, 2026-09-07) -- the SANCTIONED truthful split's ceiling on this function.
 * All seven update sites rewritten as the owner-sanctioned two-step split of a genuine
 * `a + b` (`v = *(u16 *)(p + K); v = v + 0xA; *(u16 *)(p + K) = v;`), the exact shape the
 * 2026-09-07 17:07 judge ruling left standing.  Measured: loop insn_count 91 -> 99,
 * build_insns 93 -> 92, score 26.  The move_movables bar for this function is insn_count
 * >= 120 (threshold 122, -3 after `lim` is moved), so the entire admissible split family
 * yields +8 of the +29 required, and it costs 13 points of register-assignment divergence
 * on the way.  Skipping case 1's two sites (t_truthskip2) gives 97 / 92 / 26 -- same verdict.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 v1;
    s32 v2;
    s32 v3;
    s32 v4;
    s32 v5;
    s32 v6;
    s32 v7;
    s32 v8;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            v1 = *(u16 *)(p + 8);
            v1 = v1 + 0xA;
            *(u16 *)(p + 8) = v1;
            v2 = *(u16 *)(p + 0xC);
            v2 = v2 + 0xA;
            *(u16 *)(p + 0xC) = v2;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    v3 = *(u16 *)(p + 0x10);
                    v3 = v3 + 1;
                    *(u16 *)(p + 0x10) = v3;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            v4 = *(u16 *)(p + 0xC);
            v4 = v4 + 0xA;
            *(u16 *)(p + 0xC) = v4;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    v5 = *(u16 *)(p + 0x10);
                    v5 = v5 + 1;
                    *(u16 *)(p + 0x10) = v5;
                }
            }
            break;
        case 2:
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 0xA;
            *(u16 *)(p + 0xC) = v6;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v7 = *(u16 *)(p + 8);
            v7 = v7 - 0xA;
            *(u16 *)(p + 8) = v7;
            v8 = *(u16 *)(p + 0xC);
            v8 = v8 - 0xA;
            *(u16 *)(p + 0xC) = v8;
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
