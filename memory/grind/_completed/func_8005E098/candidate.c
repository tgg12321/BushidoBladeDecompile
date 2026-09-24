typedef struct {
    void *p0;
    void *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg3;
    s32 width;
    s32 height;
    s32 pad20;
    s32 pad24;
    u8 byte28;
    u8 byte29;
    u8 byte2A;
    u8 byte2B;
    s32 pad2C;
    s16 d[2];
} S5E098;
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} T5E098;
extern s32 D_8009B488;
extern u8 D_8009B48E;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
s32 func_8005E098(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    S5E098 s;
    T5E098 *tile;
    s32 cur;
    s32 mode_off;
    s32 end_off;
    s16 i;
    s16 j;
    s16 v;
    Unk8009B400Record *p;

    tile = (T5E098 *)arg2;
    s.byte28 = 0;
    s.height = 0;
    s.width = 0;
    s.zero10 = 0;
    s.p0 = &D_8009B398[1];
    cur = arg2 + 0xA0;
    mode_off = arg2 + 0x2F8;
    end_off = arg2 + 0x304;
    s.arg3 = arg3;
    for (i = 0; i < 2; i++) {
        if (arg0 < 0) {
            s.p1 = &D_8009B488;
            if (arg1 == 1) {
                D_8009B48E = 0x2D;
            } else {
                D_8009B48E = 0x3C;
            }
        } else {
            s.p1 = &D_8009B458[0][i];
        }
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
        if (arg0 < 0) {
            break;
        }
    }

    s.p0 = &D_8009B398[0];
    s.byte28 = 0;
    s.zero10 = 0;
    s.height = 0x16;
    s.arg3 = arg3;
    for (j = 0; j < 2; j++) {
        if (j) {
            s.d[0] = s.d[1] = arg0;
        } else {
            s.d[0] = s.d[1] = arg1;
        }
        v = s.d[0] / 10;
        s.d[1] = s.d[1] % 10;
        s.d[0] = v % 10;
        for (i = 0; i < 2; i++) {
            if (s.d[i] == 0 && i == 0 && arg0 < 0) {
                i++;
            }
            p = &D_8009B400[s.d[i]];
            s.p1 = p;
            if (j != 0) {
                p->unk0 = 0x50;
            } else {
                p->unk0 = 0x209;
            }
            if (s.d[i] == 1) {
                s.width = i * 20 + 3;
            } else {
                s.width = i * 20;
            }
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
        }
        if (arg0 < 0) {
            break;
        }
    }

    s.byte29 = 0xFF;
    s.byte2B = 0x10;
    s.byte2A = 0x10;
    s.byte28 = 1;
    s.width = 0;
    s.zero10 = 0;
    s.arg3 = arg3;
    for (j = 0; j < 2; j++) {
        SetTile(tile);
        tile->r0 = 0xFF;
        tile->g0 = 0x10;
        tile->b0 = 0x10;
        tile->x0 = 0x209 - j * 0x1C1;
        tile->y0 = 0x24;
        tile->w = 0x30;
        tile->h = 1;
        SetSemiTrans(tile, 0);
        AddPrim(D_800A374C + arg3 * 4, (s32)tile);
        tile++;
        s.height = 0x24;
        s.p0 = &D_8009B398[2];
        s.p1 = &D_8009B458[j + 1][0];
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
        s.p0 = &D_8009B398[3];
        s.p1 = &D_8009B458[j + 1][1];
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
        if (arg0 < 0) {
            break;
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B398[0], 0), 0);
    AddPrim(D_800A374C + arg3 * 4, mode_off);
    return end_off - arg2;
}
