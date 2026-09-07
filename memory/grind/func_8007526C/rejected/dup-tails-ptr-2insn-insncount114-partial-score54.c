/*
 * s13: 10 copies of a 2-insn tail (i++; p += 2;) on the pointer-IV chassis.
 * .loop insn_count 114 -> the threshold DECAY cascade rejects only the LAST TWO constants
 * (lim moves at 122->119, c1 at 119->116, c2 at 116->113, then 113 < 114 rejects c3/c4).
 * MEASURED: score 54, build_insns 99.  Establishes the decay staircase: >=114 rejects 2,
 * >=117 rejects 3, >=120 rejects all four.
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
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                    i++;
                    p += 2;
                    break;
                }
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
