void func_8006BB68(s32 *arg0) {
    S69E18 s;
    u16 rect[4];
    s32 *q;
    s32 i;
    s32 p;

    s.zero1C = 0;
    s.width = 0;
    s.arg2 = 0xA;
    s.byte28 = 0;
    q = *(s32 **)(arg0[1] + 0x2C);
    p = q[0];
    s.zero10 = 0;
    s.p0 = (s32 *)p;
    s.p1 = (s32 *)(p + 0xC);
    s.in_tex = arg0[5];
    i = 0;
    arg0[5] = func_8007352C((s32)&s);
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.p0, 0), 0);
    AddPrim(D_800A374C + 0x28, arg0[7]);
    arg0[7] += 0xC;

    do {
        p = q[1];
        s.p0 = (s32 *)p;
        s.p1 = (s32 *)(p + 0xC);
        if (((D_800A34F8 >> 13) & 7) == i) {
            s.zero1C = *(s16 *)(D_800A34FC + 0xE);
            s.zero10 = 0;
        } else {
            s.zero1C = 0;
            s.zero10 = 1;
        }
        q++;
        s.in_tex = arg0[5];
        i++;
        arg0[5] = func_8007352C((s32)&s);
    } while (i < 3);

    q = *(s32 **)(arg0[1] + 0x28);
    p = q[1];
    s.p0 = (s32 *)p;
    SetDrawMode(arg0[7], 1, 0, func_8006E480(p, 0), 0);
    AddPrim(D_800A374C + 0x28, arg0[7]);
    arg0[7] += 0xC;

    rect[2] = 0xAF;
    rect[0] = 0xE8;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 0x11);
}
