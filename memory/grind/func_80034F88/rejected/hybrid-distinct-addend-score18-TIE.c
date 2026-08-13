/*
 * s7 — REJECTED (TIE, not an improvement). score 18 / 51 insns,
 * lbu 5 / sb 5 / lui 6.  The floor-18 chassis (pointer local for the &0xF8
 * read-modify-write, plain symbol for block 1) with blocks 2 and 3 moved onto
 * distinct-rtx pointers ((u8*)&D_80106A70+3, (u8*)&D_80106A71+2).
 * This is the best member of the qty-class-breaking family and the first
 * non-volatile form in seven sessions whose flag section reproduces the
 * target's per-block structure: `lui/addiu; lw; lbu 0(base); andi; bnez; ori;
 * move; sb 0(base)` for blocks 2 and 3, differing from target only in register
 * names and in the LO16 addend (3 / 2 against 0).
 * It TIES the floor because the two non-zero addends cost exactly what the two
 * recovered reloads gain (s7/s3-sidebyside listing).  An addend-0 spelling of
 * the same mechanism would score below 18 — but that needs a SECOND declared
 * symbol at 0x80106A73, which is the forbidden alias-rename family.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    u8 *ptr3;
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
    ptr2 = (u8 *)&D_80106A70 + 3;
    D_80106A73 = val2;
    c = p[8] & 2;
    val = *ptr2;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    ptr3 = (u8 *)&D_80106A71 + 2;
    *ptr2 = val2;
    c = p[8] & 4;
    val = *ptr3;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    *ptr3 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
