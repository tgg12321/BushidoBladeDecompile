/* REJECTED (s15b, 2026-09-07) -- dead loop-INVARIANT locals cannot buy threshold decay.
 * 11 pairs `zK = 0x101+K; dz = zK;` inside the loop, intended to be registered as movables
 * so each move would fire `threshold -= 3` (tools/gcc-2.7.2/loop.c:1904) and drag the
 * desirability bar under insn_count.  Measured score 13, build_insns 93, loop insn_count 91:
 * the pairs never reach loop.c at all (delete_dead_from_cse, cse.c:8684) and only the usual
 * five movables are listed.  Same class kill as the dead-store variant.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 dz;
    s32 z0;
    s32 z1;
    s32 z2;
    s32 z3;
    s32 z4;
    s32 z5;
    s32 z6;
    s32 z7;
    s32 z8;
    s32 z9;
    s32 z10;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        z0 = 257;
        dz = z0;
        z1 = 258;
        dz = z1;
        z2 = 259;
        dz = z2;
        z3 = 260;
        dz = z3;
        z4 = 261;
        dz = z4;
        z5 = 262;
        dz = z5;
        z6 = 263;
        dz = z6;
        z7 = 264;
        dz = z7;
        z8 = 265;
        dz = z8;
        z9 = 266;
        dz = z9;
        z10 = 267;
        dz = z10;
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
