/* s8 (rederive). score 22 / 51 insns.  Explicit two-label goto diamond per
 * block (if (c) goto setK; ... goto stK; setK: ...; stK:).  jump.c rebuilds
 * the ordinary diamond before cse1 (the goto to the next label is deleted and
 * LABEL_NUSES falls back to 1), so NO cse basic-block boundary is created:
 * lbu 3, exactly as the plain if/else floor form.
 */
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
    if (c) goto set1;
    val2 = val;
    goto st1;
set1:
    val2 = val | 1;
st1:
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    if (c) goto set2;
    val2 = val;
    goto st2;
set2:
    val2 = val | 2;
st2:
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    if (c) goto set3;
    val2 = val;
    goto st3;
set3:
    val2 = val | 4;
st3:
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
