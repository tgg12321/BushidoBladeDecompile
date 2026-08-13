/* REJECTED (s11) - Blocks 2/3 spelled through the symbol instead of a pointer local. The if/else join does end cse's extended basic block, but what a symbol reference re-materialises is `lui at` + `%lo`-folded access twice, never the target's shared addend-0 base. 24 / 49 insns. Kills the whole 'natural third base via a symbol reference' family (a1-a7, b8: 21-32).
 * source: tmp/grind/func_80034F88/s11/variants/a1_ptr1_sym23.c
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr = c;

    c = p[8] & 2;
    val = D_80106A73;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    D_80106A73 = c;

    c = p[8] & 4;
    val = D_80106A73;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    D_80106A73 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
