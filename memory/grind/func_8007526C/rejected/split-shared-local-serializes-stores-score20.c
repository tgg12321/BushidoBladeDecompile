/* REJECTED s15: one shared local `v` for all six +-0xA sites.  score 20, build_insns 87,
 * loop insn_count 121 (all four constants 'not desirable' -- the mechanism works).  Dead
 * because the single reused local creates a false dependency chain: the `sh` of the p+8
 * value is forced out immediately instead of being deferred, so the build loses the
 * target's `lhu $v1,8 / lhu $v0,0xC / addiu / addiu / sh 0xC / ... / sh $v1,8` interleave
 * and drops 4 words below the 91-word target.  Fixed by giving each site its own local.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 v;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            v = *(u16 *)(p + 8);
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            *(u16 *)(p + 8) = v;
            v = *(u16 *)(p + 0xC);
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            *(u16 *)(p + 0xC) = v;
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
            v = *(u16 *)(p + 0xC);
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            v = v + 2;
            *(u16 *)(p + 0xC) = v;
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
            v = *(u16 *)(p + 0xC);
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            *(u16 *)(p + 0xC) = v;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v = *(u16 *)(p + 8);
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            *(u16 *)(p + 8) = v;
            v = *(u16 *)(p + 0xC);
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            v = v - 2;
            *(u16 *)(p + 0xC) = v;
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
