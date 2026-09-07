/* REJECTED (s11, 2026-09-07) -- score 47, build_insns 94 (target 91).
 * Structural re-derivation using the u16-element array model transplanted from the
 * matched sibling func_80075670 (src/text1b.c:6666, `((s16 *)(work + i * 2))[K/2]`):
 * `u16 *w = (u16 *)D_800A36A0;` with w[4]/w[6]/w[8]/w[10]/w[12]/w[28]/w[30] for byte
 * offsets 8/0xC/0x10/0x14/0x18/0x38/0x3C, `w++` as the per-iteration bump.
 * Measured on HEAD 7ab27738 chassis, pure C, no FAKE constructs present.
 * The explicit `w++` defeats the giv that the byte-offset chassis gets for free
 * (`p = base + i * 2` -> Insn 25 giv reg 73 mult 2), so the induction-variable shape
 * diverges from the target's `addiu $a0,$a0,2` back-edge delay slot.  Sibling-style
 * indexing is NOT transplantable onto this function.  An if/else-chain variant of the
 * same model measured score 60 / build_insns 90. */
void func_8007526C(void) {
    u16 *w;
    s32 i;
    s32 lim;

    w = (u16 *)D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        switch (*(u8 *)&w[8]) {
        case 1:
            w[4] = w[4] + 0xA;
            w[6] = w[6] + 0xA;
            if ((s16)w[6] >= 0xC8) {
                if ((w[8] >> 8) == 0) {
                    w[8] = w[8] + 1;
                }
                w[4] = 0;
                w[6] = lim;
                w[10] = w[12];
                w[30] = w[28];
            }
            break;
        case 3:
            w[6] = w[6] + 0xA;
            if ((s16)w[6] >= 0xC8) {
                w[4] = lim;
                w[6] = lim;
                w[10] = w[12];
                w[30] = w[28];
                if ((w[8] >> 8) == 0) {
                    w[8] = w[8] + 1;
                }
            }
            break;
        case 2:
            w[6] = w[6] - 0xA;
            if ((s16)w[6] <= 0) {
                w[4] = 0;
                w[6] = 0;
                w[8] = 0;
            }
            break;
        case 4:
            w[4] = w[4] - 0xA;
            w[6] = w[6] - 0xA;
            if ((s16)w[6] <= 0) {
                w[4] = 0;
                w[6] = 0;
                w[8] = 0;
            }
            break;
        }
        i++;
        w++;
    } while (i < 2);
}
