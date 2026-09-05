void func_80034F88(void) {
    s32 *p;
    s32 i;
    FileFlags *q;

    p = func_80077D00();
    q = &D_80106A73;
    q->b0 = 0;
    q->b1 = 0;
    q->b2 = 0;
    if (p[8] & 1) {
        q->b0 = 1;
    }
    if (p[8] & 2) {
        q->b1 = 1;
    }
    if (p[8] & 4) {
        q->b2 = 1;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
