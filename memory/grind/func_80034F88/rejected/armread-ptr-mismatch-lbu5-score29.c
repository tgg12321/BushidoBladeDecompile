/*
 * s9. The target's lbu 5 / sb 5 counts WITHOUT any boundary construct and
 * WITHOUT volatile: read the byte through the SYMBOL outside the arm and
 * through the POINTER inside it, so the two reads of a block hash
 * differently and neither is forwarded.  29 at 47 insns; the residual is
 * address materialisation - lui 6 where the target has 4, because every
 * symbol-spelled access carries its own `lui $at` + `%lo(...)($at)`.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val2 = D_80106A73 | 1;
    if (!c) {
        val2 = *ptr;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val2 = D_80106A73 | 2;
    if (!c) {
        val2 = *ptr;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val2 = D_80106A73 | 4;
    if (!c) {
        val2 = *ptr;
    }
    D_80106A73 = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
