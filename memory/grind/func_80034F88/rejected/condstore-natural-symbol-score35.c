/* s8 (rederive). score 35 / 51 insns. The most natural human C for this
 * function (plain symbol, conditional |=).  Every flag access becomes a
 * combine-folded lui/%lo pair (lui 8), which is 4 more lui than the target.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    if (p[8] & 1) {
        D_80106A73 |= 1;
    }
    if (p[8] & 2) {
        D_80106A73 |= 2;
    }
    if (p[8] & 4) {
        D_80106A73 |= 4;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
