void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 *p;
    s32 i;
    recs = (s16 *)((u8 *)recs + 0xC);
    for (i = n - 1; i != -1; i--) {
        p = (s32 *)recs;
        if (*p != 0) {
            *p += delta;
        }
        recs += 0x68 / 2;
    }
}
