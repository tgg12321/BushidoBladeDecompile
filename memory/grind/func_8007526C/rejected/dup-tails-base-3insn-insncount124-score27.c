/*
 * s13: THE FIRST ORDINARY-C FORM THAT REJECTS ALL FOUR SWITCH CONSTANTS.
 * 10 copies of the real 3-insn loop tail (i++; p = base + i * 2;) duplicated into every
 * switch-arm exit path (no store-block duplication).  .loop: 'Loop from 17 to 362: 124 real
 * insns' and all four comparison constants print 'not desirable'; only lim (regno 75) moves.
 * MEASURED: score 27, build_insns 102 vs target 91.  Dead because the 10 duplicated tails do
 * NOT fully cross-jump back: 34 extra loop-time insns cost 8-11 extra emitted words.
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
                    i++;
                    p = base + i * 2;
                    break;
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
