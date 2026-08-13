void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    u8 *ptr3;
    u8 *ptr4;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    val = *ptr & 0xF8;
    *ptr = val;
    ptr2 = ptr;
    c = p[8] & 1;
    val = *ptr2;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr2 = c;
    ptr3 = ptr;
    c = p[8] & 2;
    val = *ptr3;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr3 = c;
    ptr4 = ptr;
    c = p[8] & 4;
    val = *ptr4;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr4 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
