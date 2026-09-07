/*
 * s13: maximal duplication -- 11 tail copies PLUS the 4-store blocks duplicated into the
 * nested-if arms of cases 1 and 3.  .loop insn_count 130, all four constants 'not desirable'.
 * MEASURED: score 67, build_insns 117.  The store-block duplication is what does not merge:
 * +40 loop-time insns bought +23 emitted words, the worst exchange rate of the session.
 */
void func_8007526C(void) {
    u8 *p;
    s32 i;
    s32 lim;

    p = D_800A36A0;
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
                    *(u16 *)(p + 8) = 0;
                    *(u16 *)(p + 0xC) = lim;
                    *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                    *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                    i++;
                    p += 2;
                    break;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                i++;
                p += 2;
                break;
            }
            i++;
            p += 2;
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 8) = lim;
                    *(u16 *)(p + 0xC) = lim;
                    *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                    *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                    i++;
                    p += 2;
                    break;
                }
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                i++;
                p += 2;
                break;
            }
            i++;
            p += 2;
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
                i++;
                p += 2;
                break;
            }
            i++;
            p += 2;
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
                i++;
                p += 2;
                break;
            }
            i++;
            p += 2;
            break;
        default:
            i++;
            p += 2;
            break;
        }
    } while (i < 2);
}
