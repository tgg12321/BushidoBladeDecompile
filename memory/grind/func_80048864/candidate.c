void func_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy) {
    u16 buf[256];
    u16 out[256];
    s16 rect[4];
    u16 *src;
    u16 *dst;
    s32 i;
    u16 p;
    s32 r, g, b, a;

    DrawSync(0);
    rect[0] = sx;
    rect[1] = sy;
    rect[2] = w;
    rect[3] = 1;
    StoreImage(rect, buf);
    DrawSync(0);
    src = buf;
    dst = out;
    for (i = 0; i < w; i++) {
        p = *src;
        if (p == 0) {
            *dst++ = *src++;
            continue;
        }
        r = (p & 0x1F) << 3;
        g = ((p >> 5) & 0x1F) << 3;
        b = ((p >> 10) & 0x1F) << 3;
        a = p & 0x8000;
        src++;
        switch (mode) {
        case 0:
            r = (r * mr) >> 15;
            g = (g * mg) >> 15;
            b = (b * mb) >> 15;
            break;
        case 1:
            r = r * 0x547;
            g = g << 11;
            b = b * 0x2B8;
            r = (r + g + b) >> 15;
            r = (r * mr) >> 12;
            g = (r * mg) >> 12;
            b = (r * mb) >> 12;
            break;
        }
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;
        *dst++ = a | r | (g << 5) | (b << 10);
    }
    rect[0] = dx;
    rect[1] = dy;
    LoadImage(rect, out);
    DrawSync(0);
}
