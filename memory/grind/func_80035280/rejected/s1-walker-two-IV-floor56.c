void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    u8 *dst;
    s32 i;
    s32 v;
    s32 a;
    s32 b;
    s32 c;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
    v = p[8];
    v = (v & ~1) | (src[3] & 1);
    p[8] = v;
    v = (v & ~2) | (src[3] & 2);
    p[8] = v;
    v = (v & ~4) | (src[3] & 4);
    p[8] = v;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = *src;
        ((u8 *)p)[0x1D + i] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    dst = (u8 *)p;
    for (i = 0; i < 3; i++) {
        a = *(s32 *)(base + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(base + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(base + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *base;
        dst[0x24] = v;
        base += 8;
        dst += 4;
    }
}
