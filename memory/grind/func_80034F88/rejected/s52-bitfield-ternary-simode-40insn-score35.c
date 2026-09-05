void func_80034F88(void) {
    s32 *p;
    s32 i;
    FileFlags *q;

    p = func_80077D00();
    q = &D_80106A73;
    q->b0 = (p[8] & 1) ? 1 : 0;
    q->b1 = (p[8] & 2) ? 1 : 0;
    q->b2 = (p[8] & 4) ? 1 : 0;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
