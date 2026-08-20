void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    *ptr = val2;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    *ptr = val2;
    asm volatile("" ::: "memory");

    val = *ptr;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    *ptr = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
