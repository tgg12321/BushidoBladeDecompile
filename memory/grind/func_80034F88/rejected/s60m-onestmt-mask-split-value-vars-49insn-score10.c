/* S60-BEGIN */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    s32 v;
    s32 w;
    s32 c;

    p = func_80077D00();
    q = &D_80106A70[3];

    v = *q;
    *q = v & 0xF8;
    v = *q;
    c = p[8] & 1;
    if (c) {
        c = v | 1;
    } else {
        c = v;
    }
    *q = c;

    q = &D_80106A70[3];
    w = *q;
    c = p[8] & 2;
    if (c) {
        c = w | 2;
    } else {
        c = w;
    }
    *q = c;

    q = &D_80106A70[3];
    w = *q;
    c = p[8] & 4;
    if (c) {
        c = w | 4;
    } else {
        c = w;
    }
    *q = c;

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
/* S60-END */
