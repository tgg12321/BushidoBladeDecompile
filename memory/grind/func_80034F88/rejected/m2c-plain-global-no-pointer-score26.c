/*
 * s16 REDERIVE. m2c's output transcribed literally: no pointer object at all,
 * every access a direct `D_80106A73` reference. 26 at 45 insns, lui 458 (target
 * 456). Confirms from a fresh angle what the ledger's anonymous-reference family
 * already showed: with no C pointer object GCC keeps the symbol_ref inline in
 * the MEM and no base register is ever materialised.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;
    {
        s32 v;

        v = D_80106A73 | 1;
        if (!(p[8] & 1)) {
            v = D_80106A73;
        }
        D_80106A73 = v;
    }
    {
        s32 v;

        v = D_80106A73 | 2;
        if (!(p[8] & 2)) {
            v = D_80106A73;
        }
        D_80106A73 = v;
    }
    {
        s32 v;

        v = D_80106A73 | 4;
        if (!(p[8] & 4)) {
            v = D_80106A73;
        }
        D_80106A73 = v;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
