s32 func_8007C86C(s16 arg0, s16 arg1)
{
    s16 x = arg0;
    s16 tx;
    s32 hi;
    s32 lo;
    s32 pkt;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            tx = D_8009BE78 - 1;
        } else {
            tx = x;
        }
    } else {
        tx = 0;
    }
    x = tx;

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    hi = arg1 & 0xFFF;
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        hi = arg1 & 0x3FF;
        hi = hi << 10;
        lo = x & 0x3FF;
    } else {
        hi = hi << 12;
        lo = x & 0xFFF;
    }
    pkt = lo | 0xE4000000;
    return hi | pkt;
}
