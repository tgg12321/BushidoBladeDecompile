void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    s32 n;
    u8 *base;
    u8 *p;
    u8 *end_p;

    g_disp_enable = DISP_LOADING;
    if (i < D_800A389B) {
        base = &D_800F65F8;
        do {
            n = (i < D_800A3874);
            p = base;
            end_p = base + 2;
            do {
                s32 v0;
                if (n) {
                    v0 = ((s32)*p) << shift;
                } else {
                    v0 = 3 << shift;
                }
                acc |= v0;
                p++;
                shift += 2;
            } while ((s32)p < (s32)end_p);
            base += 2;
            i++;
            n = D_800A389B;
        } while (i < n);
    }

    D_800A3784 = acc;
}
