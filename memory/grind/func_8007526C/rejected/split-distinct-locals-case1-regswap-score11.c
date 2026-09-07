/* REJECTED s15: distinct local per site, five addends each, ALL SIX sites split.  score 11,
 * build_insns 90, loop insn_count 121, correct movable shape.  Dead because case 1's two
 * updates come out with the hard registers swapped: target `lhu $v1,0x8 / lhu $v0,0xC`,
 * build `lhu $v0,8 / lhu $v1,12`.  Reversing the declaration order (d_k5_revdecl) and
 * swapping the two local NAMES (d_k5_swap12) both measure 11/90 unchanged, so the swap is
 * not a declaration-order effect.  Leaving case 1 un-split removes it (see candidate.c).
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

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            v1 = *(u16 *)(p + 8);
            v1 = v1 + 2;
            v1 = v1 + 2;
            v1 = v1 + 2;
            v1 = v1 + 2;
            v1 = v1 + 2;
            *(u16 *)(p + 8) = v1;
            v2 = *(u16 *)(p + 0xC);
            v2 = v2 + 2;
            v2 = v2 + 2;
            v2 = v2 + 2;
            v2 = v2 + 2;
            v2 = v2 + 2;
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
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 2;
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
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 2;
            *(u16 *)(p + 8) = v5;
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 2;
            v6 = v6 - 2;
            v6 = v6 - 2;
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
