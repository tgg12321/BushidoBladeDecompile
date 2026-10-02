void func_80048FFC(s32 arg0) {
    RECT rect;
    s16 *ctl;
    s32 x;
    s32 y;
    s32 xf;
    s32 yf;
    s32 dx;
    s32 dy;
    s32 dxf;
    s32 dyf;
    s32 w;
    s32 period;
    s32 i;
    MoveChannel *ch;
    DR_MOVE *p;
    s32 phase;

    ch = &D_800EF848[arg0];
    phase = ch->phase;
    ctl = ch->ctl;
    p = ch->move[D_800A36AC & 1];
    x = ctl[0] & ~0x3F;
    y = ctl[1] & ~0xFF;
    xf = ctl[0] % 64;
    yf = ctl[1] % 256;
    w = ctl[2];
    period = ctl[3];
    dx = ctl[4] & ~0x3F;
    dy = ctl[5] & ~0xFF;
    dxf = ctl[4] % 64;
    dyf = ctl[5] % 256;
    for (i = 0; i < 3; i++) {
        s32 nx = dx + dxf;
        s32 ny = dy + dyf;
        s32 sy = y + yf;
        s32 h1 = period - phase;
        s32 h2 = phase;
        rect.x = x + xf;
        rect.y = sy;
        rect.w = w;
        rect.h = h1;
        SetDrawMove(p, &rect, nx, ny + phase);
        ((OTag *)p)->addr = ((OTag *)&D_800A378C[0xFFF])->addr;
        ((OTag *)&D_800A378C[0xFFF])->addr = (u32)p;
        p++;
        rect.y = sy + h1;
        rect.h = h2;
        SetDrawMove(p, &rect, nx, ny);
        ((OTag *)p)->addr = ((OTag *)&D_800A378C[0xFFF])->addr;
        ((OTag *)&D_800A378C[0xFFF])->addr = (u32)p;
        p++;
        xf >>= 1;
        yf >>= 1;
        dxf >>= 1;
        dyf >>= 1;
        w >>= 1;
        period >>= 1;
        phase >>= 1;
    }
    ch->phase += ctl[6];
    if (ch->phase >= ctl[3]) {
        ch->phase %= ctl[3];
    }
}
