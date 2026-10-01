void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 i;
    s32 *p;
    for (i = n - 1, p = (s32 *)((u8 *)recs + 0xC); i != -1; i--, p += 0x68 / 4) {
        if (*p != 0) {
            *p += delta;
        }
    }
}
