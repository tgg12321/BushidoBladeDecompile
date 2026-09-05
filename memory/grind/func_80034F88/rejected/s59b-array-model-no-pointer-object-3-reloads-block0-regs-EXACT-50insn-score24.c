/* S59-BEGIN */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 raw;
    s32 mv;
    s32 v;
    s32 c;

    p = func_80077D00();

    raw = D_80106A70[3];
    mv = raw & 0xF8;
    D_80106A70[3] = mv;
    mv = raw;
    v = D_80106A70[3];
    c = p[8] & 1;
    if (c) {
        c = v | 1;
    } else {
        c = v;
    }
    D_80106A70[3] = c;
    c = raw;

    v = D_80106A70[3];
    c = p[8] & 2;
    if (c) {
        c = v | 2;
    } else {
        c = v;
    }
    D_80106A70[3] = c;
    c = raw;

    v = D_80106A70[3];
    c = p[8] & 4;
    if (c) {
        c = v | 4;
    } else {
        c = v;
    }
    D_80106A70[3] = c;

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
/* S59-END */
