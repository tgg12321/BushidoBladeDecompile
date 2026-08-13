/* REJECTED — the 3-byte copy loop addressed through the existing `ptr` local
 * as `*(ptr - 3 + i)` (i.e. reaching D_80106A70 from &D_80106A73), which is
 * how the sibling at src/code6cac_b.c:4044 spells the same region.  Honest
 * sandbox 31 at 50 insns vs 18.  This was s3-F1's "a second natural use of
 * `ptr` late in the function to lengthen its live range" probe: the live-range
 * extension does happen, and it costs 13 points — `ptr` survives across the
 * whole flag section, so the target's per-block address rematerialisations
 * disappear and the loop's `lui at,%hi(D_80106A70)` is replaced by pointer
 * arithmetic the target does not contain. */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(ptr - 3 + i) = *((u8 *)p + i + 0x17);
    }
}
