void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    s32 innerBound;
    s32 emptyVal;
    u8 *base;
    u8 *p;
    u8 *end_p;

    g_disp_enable = DISP_LOADING;
    if (i < D_800A389B) {
        innerBound = D_800A3874;
        emptyVal = 3;
        base = &D_800F65F8;
    outer:
        {
            s32 useReal = (i < innerBound);
            p = base;
            end_p = base + 2;
        inner:
            if (useReal) {
                acc |= ((s32)*p) << shift;
            } else {
                acc |= emptyVal << shift;
            }
            p++;
            shift += 2;
            if ((s32)p < (s32)end_p) {
                goto inner;
            }
        }
        base += 2;
        i++;
        if (i < D_800A389B) {
            goto outer;
        }
    }

    D_800A3784 = acc;
}
