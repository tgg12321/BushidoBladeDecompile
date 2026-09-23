typedef struct {
    s32 p0;
    s32 p1;
    s32 chain;
    s32 pad0C;
    s32 flag10;
    s32 n14;
    s32 x18;
    s32 y1C;
    s32 pad20;
    s32 pad24;
    s8 flag28;
    s8 c29, c2A, c2B;
} S_6B120;

typedef struct {
    s16 x, y, w, h;
} S_6B120_rect;

void func_8006B120(s32 *arg0) {
    S_6B120 s;
    S_6B120_rect r;
    s32 *tbl;
    s32 i;
    s32 rec0;

    s.flag28 = 0;
    s.n14 = 10;
    tbl = *(s32 **)(arg0[1] + 0x28);
    rec0 = tbl[0];
    s.y1C = 0;
    s.x18 = 0;
    s.flag10 = 0;
    s.p0 = rec0;
    {
        s32 p1 = s.p0 + 0xC;
        s.p1 = p1;
    }
    s.chain = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    SetDrawMode(arg0[7], 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(D_800A374C + 0x28, arg0[7]);
    arg0[7] += 0xC;
    s.x18 = 0;
    i = 0;

    do {
        s.p0 = tbl[i + 1];
        if ((((u32)D_800A34F8 >> 10) & 7) == i) {
            s.flag28 = 1;
            s.y1C = *(s16 *)(D_800A34FC + 0xE);
            s.flag10 = 0;
            s.c29 = s.c2A = s.c2B = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 47) >> 12) - 0x80;
        } else {
            s.flag28 = 0;
            s.y1C = 0;
            s.flag10 = 1;
        }
        {
            s32 p1 = s.p0 + 0xC;
            s.p1 = p1;
        }
        s.chain = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        i++;
    } while (i < 6);

    s.y1C = 0;
    i = 0;
    do {
        s.p0 = tbl[i + 7];
        s.x18 = 0;
        s.flag28 = 0;
        if ((((u32 *)D_800A3524)[8] & 1) == i) {
            s.flag10 = 0;
            if (!(D_800A34F8 & 0x1C00)) {
                s32 c;

                s.flag28 = 1;
                s.x18 = *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) - 0x80;
                s.c2B = c;
                s.c2A = c;
                s.c29 = c;
            }
        } else {
            s.flag10 = 1;
        }
        {
            s32 p1 = s.p0 + 0xC;
            s.p1 = p1;
        }
        s.chain = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.p0 = tbl[i + 9];
        s.x18 = 0;
        s.flag28 = 0;
        if (((((u32 *)D_800A3524)[8] >> 1) & 1) == i) {
            s.flag10 = 0;
            if ((D_800A34F8 & 0x1C00) == 0x400) {
                s32 c;

                s.flag28 = 1;
                s.x18 = *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) - 0x80;
                s.c2B = c;
                s.c2A = c;
                s.c29 = c;
            }
        } else {
            s.flag10 = 1;
        }
        {
            s32 p1 = s.p0 + 0xC;
            s.p1 = p1;
        }
        s.chain = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.p0 = tbl[i + 11];
        s.x18 = 0;
        s.flag28 = 0;
        if (((((u32 *)D_800A3524)[8] >> 2) & 1) == i) {
            s.flag10 = 0;
            if ((D_800A34F8 & 0x1C00) == 0x800) {
                s32 c;

                s.flag28 = 1;
                s.x18 = *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) - 0x80;
                s.c2B = c;
                s.c2A = c;
                s.c29 = c;
            }
        } else {
            s.flag10 = 1;
        }
        {
            s32 p1 = s.p0 + 0xC;
            s.p1 = p1;
        }
        s.chain = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        i++;
    } while (i < 2);

    tbl = *(s32 **)(arg0[1] + 0x28);
    s.p0 = tbl[1];
    SetDrawMode(arg0[7], 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(D_800A374C + 0x28, arg0[7]);
    arg0[7] += 0xC;
    r.w = 0xAF;
    r.x = 0xE8;
    r.y = 0x25;
    r.h = 1;
    func_80069898((GameObj *)arg0, (u16 *)&r, 0x11);
}
