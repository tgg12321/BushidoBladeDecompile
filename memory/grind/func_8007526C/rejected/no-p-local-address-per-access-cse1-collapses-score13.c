/*
 * s13: every access spelled *(u16 *)(base + i * 2 + K) with no p local, to test whether
 * pre-strength-reduction address arithmetic is free loop-time insn_count.  IT IS NOT:
 * cse1 runs BEFORE loop and collapses all of it, so .loop still reads 'Loop from 14 to 541:
 * 91 real insns' -- byte-identical result to the candidate.  MEASURED: score 13, build 93.
 */
void func_8007526C(void) {
    u8 *base;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        switch (*(u8 *)(base + i * 2 + 0x10)) {
        case 1:
            *(u16 *)(base + i * 2 + 8) = *(u16 *)(base + i * 2 + 8) + 0xA;
            *(u16 *)(base + i * 2 + 0xC) = *(u16 *)(base + i * 2 + 0xC) + 0xA;
            if ((s16)*(u16 *)(base + i * 2 + 0xC) >= 0xC8) {
                if ((*(u16 *)(base + i * 2 + 0x10) >> 8) == 0) {
                    *(u16 *)(base + i * 2 + 0x10) = *(u16 *)(base + i * 2 + 0x10) + 1;
                }
                *(u16 *)(base + i * 2 + 8) = 0;
                *(u16 *)(base + i * 2 + 0xC) = lim;
                *(u16 *)(base + i * 2 + 0x14) = *(u16 *)(base + i * 2 + 0x18);
                *(u16 *)(base + i * 2 + 0x3C) = *(u16 *)(base + i * 2 + 0x38);
            }
            break;
        case 3:
            *(u16 *)(base + i * 2 + 0xC) = *(u16 *)(base + i * 2 + 0xC) + 0xA;
            if ((s16)*(u16 *)(base + i * 2 + 0xC) >= 0xC8) {
                *(u16 *)(base + i * 2 + 8) = lim;
                *(u16 *)(base + i * 2 + 0xC) = lim;
                *(u16 *)(base + i * 2 + 0x14) = *(u16 *)(base + i * 2 + 0x18);
                *(u16 *)(base + i * 2 + 0x3C) = *(u16 *)(base + i * 2 + 0x38);
                if ((*(u16 *)(base + i * 2 + 0x10) >> 8) == 0) {
                    *(u16 *)(base + i * 2 + 0x10) = *(u16 *)(base + i * 2 + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(base + i * 2 + 0xC) = *(u16 *)(base + i * 2 + 0xC) - 0xA;
            if ((s16)*(u16 *)(base + i * 2 + 0xC) <= 0) {
                *(u16 *)(base + i * 2 + 8) = 0;
                *(u16 *)(base + i * 2 + 0xC) = 0;
                *(u16 *)(base + i * 2 + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(base + i * 2 + 8) = *(u16 *)(base + i * 2 + 8) - 0xA;
            *(u16 *)(base + i * 2 + 0xC) = *(u16 *)(base + i * 2 + 0xC) - 0xA;
            if ((s16)*(u16 *)(base + i * 2 + 0xC) <= 0) {
                *(u16 *)(base + i * 2 + 8) = 0;
                *(u16 *)(base + i * 2 + 0xC) = 0;
                *(u16 *)(base + i * 2 + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
