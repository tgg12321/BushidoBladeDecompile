typedef struct {
    void *p0;
    Unk8009B400Record *p1;
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
    s16 d[3];
} S5D814;
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} T5D814;
typedef struct {
    s16 x;
    s16 y;
} P5D814;
extern Unk8009B400Record D_8009B3C8[3];
extern Unk8009B400Record D_8009B3E0[2];
extern Unk8009B400Record D_8009B3F0;
extern Unk8009B400Record D_8009B3F8;
extern P5D814 D_8009B450[2];
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
s32 func_8005D814(s16 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    S5D814 s;
    T5D814 *tile;
    s32 cur;
    s32 mode_off;
    s32 end_off;
    s16 i;
    s16 j;
    s16 v;
    s16 shown;

    arg1--;
    tile = (T5D814 *)arg2;
    s.byte28 = 0;
    s.height = 0;
    s.width = 0;
    s.arg3 = arg3;
    s.zero10 = 0;
    s.p0 = &D_8009B398[0];
    cur = arg2 + 0xA0;
    mode_off = arg2 + 0x2F8;
    end_off = arg2 + 0x304;
    for (i = 0; i < 3; i++) {
        s.p1 = &D_8009B3C8[i];
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
    }

    s.p0 = &D_8009B398[1];
    for (i = 0; i < 2; i++) {
        s.p1 = &D_8009B3E0[i];
        if (i != 0) {
            if (arg1 == 1) {
                s.p1->unk6 = 0x2D;
            } else {
                s.p1->unk6 = 0x3C;
            }
        }
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
    }

    s.p0 = &D_8009B398[0];
    s.byte28 = 0;
    s.height = 0x16;
    s.zero10 = 0;
    s.arg3 = arg3;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 2; i++) {
            switch (j) {
            case 0:
                s.d[i] = *arg0;
                if (i != 0) {
                    s.d[i] = s.d[i] % 10;
                } else {
                    v = s.d[0] / 10;
                    s.d[0] = v % 10;
                }
                s.p1 = &D_8009B400[s.d[i]];
                if (s.d[i] == 1) {
                    s.width = i * 20 + 3;
                } else {
                    s.width = i * 20;
                }
                s.p1->unk0 = 0x1A2;
                break;
            case 1:
                s.d[i] = *((u8 *)arg0 + 2);
                if (i != 0) {
                    s.d[i] = s.d[i] % 10;
                } else {
                    v = s.d[0] / 10;
                    s.d[0] = v % 10;
                }
                s.p1 = &D_8009B400[s.d[i]];
                if (s.d[i] == 1) {
                    s.width = i * 20 + 3;
                } else {
                    s.width = i * 20;
                }
                s.p1->unk0 = 0x1D3;
                break;
            case 2:
                s.d[i] = *((u8 *)arg0 + 3);
                if (i != 0) {
                    s.d[i] = s.d[i] % 10;
                } else {
                    v = s.d[0] / 10;
                    s.d[0] = v % 10;
                }
                s.p1 = &D_8009B400[s.d[i]];
                if (s.d[i] == 1) {
                    s.width = i * 20 + 3;
                } else {
                    s.width = i * 20;
                }
                s.p1->unk0 = 0x209;
                break;
            }
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
        }
    }

    s.p0 = &D_8009B398[0];
    s.d[0] = s.d[1] = s.d[2] = arg1;
    v = s.d[1] / 10;
    s.d[2] = s.d[2] % 10;
    s.d[1] = v % 10;
    s.d[0] = s.d[0] / 100;
    s.d[1] = s.d[1] % 100;
    s.height = 0x29;
    shown = 0;
    for (j = 0; j < 3; j++) {
        if (shown || s.d[j] != 0 || j == 2) {
            s.p1 = &D_8009B400[s.d[j]];
            s.p1->unk0 = 0x1F3;
            shown = 1;
            if (s.d[j] == 1) {
                s.width = j * 21 + 3;
            } else {
                s.width = j * 21;
            }
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
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
        tile->x0 = D_8009B450[j].x;
        tile->y0 = D_8009B450[j].y;
        tile->h = 1;
        tile->w = 0x238 - D_8009B450[j].x;
        SetSemiTrans(tile, 0);
        AddPrim(g_gpu_ot_ptr + arg3 * 4, (s32)tile);
        tile++;
        s.height = D_8009B450[j].y;
        s.p0 = &D_8009B398[2];
        s.p1 = &D_8009B3F0;
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
        s.p0 = &D_8009B398[3];
        s.p1 = &D_8009B3F8;
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B398[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, mode_off);
    return end_off - arg2;
}
