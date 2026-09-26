typedef struct {
    Unk8009B398Record *p0;
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
} S5F1C8;
extern Unk8009B398Record D_8009B5A0[2];
extern Unk8009B400Record D_8009B5B8[2][2];
extern Unk8009B400Record D_8009B5D8[2];
extern Unk8009B400Record D_8009B5E8;
extern Unk8009B400Record D_8009B5F0[2][2];
s32 func_8005F1C8(u8 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    S5F1C8 s;
    T5E098 *tile;
    s32 cur;
    s32 mode_off;
    s32 end_off;
    s16 i;
    s16 j;
    s16 k;
    s16 row;
    u8 wins;
    s16 count;
    s32 x;

    s.byte28 = 0;
    s.height = 0;
    s.zero10 = 0;
    tile = (T5E098 *)arg2;
    cur = arg2 + 0xA0;
    mode_off = arg2 + 0x2F8;
    end_off = arg2 + 0x304;
    s.arg3 = arg3;
    for (i = 0; i < 2; i++) {
        s.p0 = &D_8009B5A0[i];
        wins = (arg1 >> (i * 8)) & 0xFF;
        if (i != 0) {
            count = 2;
        } else {
            count = (((u32)D_8009BD38 >> 14) & 1) + 1;
        }
        for (row = 0; row < 2; row++) {
            for (k = 0; k < count; k++) {
                if (row != 0) {
                    s.width = i * 8 + 0x1C2 - (0x1C - i * 8) * k;
                } else {
                    s.width = (0x1C - i * 8) * k;
                }
                s.p1 = &D_8009B5B8[i][0];
                if (k >= ((wins >> (row * 4)) & 0xF)) {
                    s.p1++;
                }
                s.in_tex = cur;
                cur = func_8007352C((s32)&s);
            }
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B5A0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, mode_off);
    mode_off += 0xC;

    s.byte28 = 0;
    s.height = 0;
    s.zero10 = 0;
    s.p0 = &D_8009B398[1];
    s.arg3 = arg3;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            s.width = j * 550;
            s.p1 = &D_8009B5D8[k];
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
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = 0xFF;
            tile->g0 = 0x10;
            tile->b0 = 0x10;
            tile->x0 = 0x48 + j * 431 + j * (k << 4);
            tile->y0 = k * 20 + 0x24;
            tile->w = 0x42 - k * 16;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3 * 4, (s32)tile);
            tile++;
            s.height = k * 20 + 0x24;
            s.p0 = &D_8009B398[2];
            s.p1 = &D_8009B5F0[j][0];
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
            s.p0 = &D_8009B398[3];
            s.p1 = &D_8009B5F0[j][1];
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
        }
    }

    s.p0 = &D_8009B398[0];
    s.byte28 = 0;
    s.height = 0x16;
    s.zero10 = 0;
    s.arg3 = arg3;
    for (j = 0; j < 2; j++) {
        for (k = 0; k < 3; k++) {
            switch (j) {
            case 0:
                if (k < 2 || (D_8009BD38 & 0x3000) == 0x2000) {
                    s.d[k] = arg0[2];
                    if (k == 0 && (D_8009BD38 & 0x3000) == 0x2000) {
                        s.d[k] = s.d[k] / 100;
                    } else if (k == 0 || (k == 1 && (D_8009BD38 & 0x3000) == 0x2000)) {
                        s.d[k] = s.d[k] / 10;
                    }
                    s.d[k] = s.d[k] % 10;
                    s.p1 = &D_8009B400[s.d[k]];
                    if (s.d[k] == 1) {
                        s.width = k * 20 + 3;
                    } else {
                        s.width = k * 20;
                    }
                }
                break;
            case 1:
                if (k < 2) {
                    s.d[k] = arg0[3];
                    if (k != 0) {
                        s.d[k] = s.d[k] % 10;
                    } else {
                        s16 tens = s.d[k] / 10;

                        s.d[k] = tens % 10;
                    }
                    if ((D_8009BD38 & 0x3000) == 0x2000) {
                        x = k * 20 + 0x48;
                    } else {
                        x = k * 20 + 0x34;
                    }
                    s.p1 = &D_8009B400[s.d[k]];
                    if (s.d[k] == 1) {
                        s.width = x + 3;
                    } else {
                        s.width = x;
                    }
                }
                break;
            }
            if ((D_8009BD38 & 0x3000) == 0x2000) {
                s.p1->unk0 = 0x109;
            } else {
                s.p1->unk0 = 0x113;
            }
            s.in_tex = cur;
            cur = func_8007352C((s32)&s);
        }
        s.p1 = &D_8009B5E8;
        if ((D_8009BD38 & 0x3000) == 0x2000) {
            s.width = j * 6 + 0x145;
        } else {
            s.width = j * 6 + 0x13B;
        }
        s.in_tex = cur;
        cur = func_8007352C((s32)&s);
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B398[1], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, mode_off);
    return end_off - arg2;
}
