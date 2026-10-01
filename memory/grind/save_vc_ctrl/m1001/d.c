void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 i = n - 1;
    u8 *p;
    if (i != -1) {
        p = (u8 *)recs + 0xC;
        do {
            if (*(s32 *)p != 0) {
                *(s32 *)p += delta;
            }
            p += 0x68;
        } while (--i != -1);
    }
}
