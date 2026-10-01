void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 i;
    s32 *p = (s32 *)((u8 *)recs + 0xC);
    for (i = n - 1; i != -1; i--) {
        if (*p != 0) {
            *p += delta;
        }
        p += 0x68 / 4;
    }
}
