/* REJECTED — only the THIRD flag block stores through `ptr` (`*ptr = val2;`),
 * the other two through the plain symbol; a milder version of the "second use
 * of ptr" probe above.  Honest sandbox 22 at 50 insns vs 18.  Cheaper than the
 * copy-loop variant but still 4 points worse than the floor: the extra use
 * keeps ptr's address pseudo live to the end of the flag section, which
 * removes the third block's own address materialisation. */
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
    *ptr = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
