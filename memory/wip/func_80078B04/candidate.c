s32 func_80078B04(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    u16 *p;
    s32 result;
    if (v < 3) {
        p = (u16 *)(D_8009BD6C + v * 0x10);
        result = *p;
    } else {
        result = 0;
    }
    return result;
}
