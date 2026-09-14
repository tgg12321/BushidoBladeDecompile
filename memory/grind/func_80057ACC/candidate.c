extern s32 func_8005763C(s32, s32, s32, s32, s32, s32, s32, s32, s32 *, s32 *);
extern s32 SquareRoot0(s32);

s32 func_80057ACC(s32 arg0, u8 *arg1, s32 arg2, s32 arg3) {
    s32 sp28;
    s32 sp2C;
    s32 best;
    s16 i;
    s16 j;
    s16 k;
    s16 n;
    u8 *poly;
    s32 dx;
    s32 dy;
    s32 d;

    best = 100000;
    for (i = 0; i < arg1[0]; i++) {
        poly = (u8 *)(*(s32 *)(arg1 + 4) + i * 8);
        n = poly[3];
        if (poly[0] & 0x80) {
            n = poly[3] - 1;
        }
        for (j = 0; j < n; j++) {
            k = j + 1;
            if (!(k < poly[3])) {
                k = 0;
            }
            if (func_8005763C(*(s32 *)(arg0 + 0xF4), *(s32 *)(arg0 + 0xFC), arg2, arg3,
                              *(s16 *)(*(s32 *)(poly + 4) + j * 4),
                              *(s16 *)(*(s32 *)(poly + 4) + j * 4 + 2),
                              *(s16 *)(*(s32 *)(poly + 4) + k * 4),
                              *(s16 *)(*(s32 *)(poly + 4) + k * 4 + 2),
                              &sp28, &sp2C) != 0) {
                dx = sp28 - *(s32 *)(arg0 + 0xF4);
                dy = sp2C - *(s32 *)(arg0 + 0xFC);
                d = SquareRoot0(dx * dx + dy * dy);
                if (d < best) {
                    best = d;
                    *(u8 *)(arg0 + 0x360) = i;
                    *(u8 *)(arg0 + 0x361) = j;
                }
            }
        }
    }
    return best;
}
