s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count;
    s32 idx;
    s32 saved;
    s32 *elem;
    s32 *base;
    s32 status;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    base = g_cd_sector_buf;
    elem = base + idx;
    count = 3;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, a2, 0) == 0) {
        status = 0;
        goto done;
    }
next:
    count--;
    status = -1;
    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
done:
    if (status != 0) {
        return 0;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, a2);
        r ^= 2;
        return (u32)r < 1;
    }
}
