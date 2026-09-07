/* REJECTED s15: `s16` locals for the six update sites, no split.  score 26, build_insns 92,
 * loop insn_count 93.  Dead: the hypothesis was that 16-bit locals would generate many
 * sll/sra sign-extension insns at loop time that combine folds away for free.  They do not
 * -- the extensions are folded into the lhu/sh before loop.c counts, so six s16 locals buy
 * only +2 loop insns.  s16 at two addends measures 105 (same as s32's 105): the gain is
 * the split, not the type.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s16 v1;
    s16 v2;
    s16 v3;
    s16 v4;
    s16 v5;
    s16 v6;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            v1 = *(u16 *)(p + 8);
            v1 = v1 + 10;
            *(u16 *)(p + 8) = v1;
            v2 = *(u16 *)(p + 0xC);
            v2 = v2 + 10;
            *(u16 *)(p + 0xC) = v2;
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
            v3 = v3 + 10;
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
            v4 = v4 - 10;
            *(u16 *)(p + 0xC) = v4;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v5 = *(u16 *)(p + 8);
            v5 = v5 - 10;
            *(u16 *)(p + 8) = v5;
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 10;
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
