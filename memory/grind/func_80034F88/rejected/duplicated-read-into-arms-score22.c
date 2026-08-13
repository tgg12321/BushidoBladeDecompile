void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    val = *ptr & 0xF8;
    *ptr = val;
    ptr2 = ptr;
    c = p[8] & 1;
    if (c) {
        c = *ptr2 | 1;
    } else {
        c = *ptr2;
    }
    *ptr2 = c;
    ptr2 = &D_80106A73;
    c = p[8] & 2;
    if (c) {
        c = *ptr2 | 2;
    } else {
        c = *ptr2;
    }
    *ptr2 = c;
    ptr2 = &D_80106A73;
    c = p[8] & 4;
    if (c) {
        c = *ptr2 | 4;
    } else {
        c = *ptr2;
    }
    *ptr2 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
