void func_8003D52C(u8 *fmt, s32 first_arg, ...) {
    u8 buf[0x400];
    u8 seg[0x100];
    register va_list ap asm("s3");
    s32 cur_arg;
    s32 seen_pct;
    register u8 *seg_ptr asm("s1");
    s32 ch;

    cur_arg = first_arg;
    seen_pct = 0;
    seg_ptr = seg;
    va_start(ap, first_arg);
    buf[0] = 0;

    ch = *fmt++;
    if (ch != 0) {
        s32 pct = 0x25;
        u8 *buf_ptr = buf;
        u8 *seg_start = seg_ptr;

        do {
            if (ch == pct) {
                if (seen_pct == 0) {
                    seen_pct = 1;
                } else {
                    *seg_ptr = 0;
                    sprintf(buf_ptr + strlen(buf), seg_start, cur_arg);
                    seg_ptr = seg_start;
                    cur_arg = va_arg(ap, s32);
                }
            }
            *seg_ptr++ = ch;
            ch = *fmt++;
        } while (ch != 0);
    }

    *seg_ptr = 0;
    sprintf(buf + strlen(buf), seg, cur_arg);

    ch = buf[0];
    if (ch != 0) {
        u8 *p = &buf[1];
        do {
            s32 row = D_800A3360;
            if (row >= 0x1A) goto done;

            if (ch == 0x20) goto inc_col;
            if (ch == 0x0A) {
                D_800A335C = 0;
                D_800A3360 = row + 1;
                goto check_wrap;
            }
            if (ch == 0x7E) {
                ch = *p++;
                if (ch == 0) goto done;
                if (ch == 0x63 || ch == 0x43) {
                    s32 d1, d2, d3;
                    d1 = *p++;
                    if (d1 == 0) goto done;
                    d2 = *p++;
                    if (d2 == 0) goto done;
                    d3 = *p++;
                    if (d3 == 0) goto done;
                    D_800A3364 = ((d1 - 0x30) << 5) | ((d2 - 0x30) << 13) | ((d3 - 0x30) << 21);
                }
                goto check_wrap;
            }
            func_8003D39C(D_800A335C * 8 + 0x10, row * 8 + 0x10, ch, D_800A3364);
        inc_col:
            D_800A335C = D_800A335C + 1;
        check_wrap:
            if (D_800A335C >= 0x4C) {
                D_800A335C = 0;
                D_800A3360 = D_800A3360 + 1;
            }
            ch = *p++;
        } while (ch != 0);
    }
done:;
}
