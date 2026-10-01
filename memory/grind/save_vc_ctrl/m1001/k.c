void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 i = n - 1;
    s32 *p;
    if (i != -1) {
        p = (s32 *)(recs + 6);
        do {
            if (*p != 0) {
                *p += delta;
            }
            p += 0x1A;
        } while (--i != -1);
    }
}
