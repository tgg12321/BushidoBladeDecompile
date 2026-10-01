void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 *p = (s32 *)((u8 *)recs + 0xC);
    while (n--) {
        if (*p != 0) {
            *p += delta;
        }
        p += 0x68 / 4;
    }
}
