void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 *p = (s32 *)((u8 *)recs + 0xC);
    s32 i = n - 1;
    if (i != -1) {
        do {
            if (*p != 0) {
                *p += delta;
            }
            p += 0x68 / 4;
        } while (--i != -1);
    }
}
