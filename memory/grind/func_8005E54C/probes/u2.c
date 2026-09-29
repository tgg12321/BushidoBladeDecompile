/* The 0x2C-byte draw descriptor func_8007352C (SPRT walker) and func_80073728
   (POLY_FT4 walker) consume; same layout as S_6A880. */
typedef struct {
    Unk8009B398Record *header;
    Unk8009B400Record *table;
    s32 sprt_out;
    s32 ft4_out;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
    s32 unk2C;
    s16 unk30[2];
} Env5E54C;
extern Unk8009B398Record D_8009ADB4;
extern Unk8009B400Record D_8009ADC0[3];
extern Unk8009B400Record UesrWorkDef[][3];
extern Unk8009B400Record D_8009B490[2][2];
extern Unk8009B398Record D_8009B4B0;
extern Unk8009B400Record D_8009B4BC[5];
extern Unk8009B398Record D_8009B4E4;
extern Unk8009B398Record D_8009B4F0;
extern Unk8009B400Record D_8009B4FC;
extern Unk8009B400Record D_8009B504;
extern Unk8009B400Record D_8009B50C;
extern Unk8009B400Record D_8009B514;
extern Unk8009B400Record D_8009B51C;
extern Unk8009B398Record D_8009B524;
extern Unk8009B398Record D_8009B530;
extern Unk8009B398Record D_8009B53C;
extern Unk8009B398Record D_8009B548;
extern Unk8009B400Record D_8009B554[3];
extern Unk8009B400Record D_8009B56C[2];
extern Unk8009B400Record D_8009B57C[2];
extern u8 D_8009B58C[];
extern u8 D_800A3270[];
s32 func_8005E54C(u32 arg0, s32 arg1, s32 arg2) {
    union {
        s16 v[2];
        s32 word;
    } vals;
    s16 wins[2];
    Env5E54C s;
    T5E098 *tile;
    s32 cur;
    s32 ft4;
    s32 mode_off;
    s32 end_off;
    s16 i;
    s16 j;
    s16 k;
    s16 c;
    s32 y;

    tile = (T5E098 *)arg1;
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    cur = arg1 + 0xA0;
    ft4 = arg1 + 0x898;
    mode_off = arg1 + 0xBB8;
    end_off = arg1 + 0xBC4;
    s.ot_idx = arg2;
    for (i = 0; i < 2; i++) {
        if (!(D_8009BD38.unk15 >> i & 1)) {
            s.header = &D_8009B524;
        } else {
            s.header = &D_8009B53C;
        }
        s.x = i * 320;
        s.table = D_8009B554;
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
        if (!(D_8009BD38.unk15 >> i & 1)) {
            s.header = &D_8009B530;
            s.table = D_8009B56C;
        } else {
            s.header = &D_8009B548;
            s.table = D_8009B57C;
        }
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
    }

    s.semi = 0;
    s.has_color = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        vals.v[0] = (arg0 >> (i * 4)) & 3;
        vals.v[1] = (arg0 >> (i * 4 + 2)) & 3;
        if (D_8009BD38.unk10 == 2) {
            s.y = i * 24 + 0x44;
        } else if (D_8009BD38.unk10 == 1) {
            s.y = i * 24 + 0x4F;
        } else {
            s.y = i * 34 + 0x4F;
        }
        if (vals.v[0] == 3 || vals.v[1] == 3) {
            s.header = &D_8009B4B0;
            s.x = 0;
            s.y += 2;
            s.table = &D_8009B4BC[D_800A3270[i]];
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
        } else {
            s.header = &D_8009B4E4;
            s.x = 0;
            s.table = &D_8009B514;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            for (j = 0; j < 2; j++) {
                s.x = j * 70;
                if (vals.v[j] > *(j ? &vals.v[0] : &vals.v[1])) {
                    s.table = &D_8009B4FC;
                } else if (vals.v[j] < *(j ? &vals.v[0] : &vals.v[1])) {
                    s.table = &D_8009B504;
                } else {
                    s.table = &D_8009B50C;
                }
                s.sprt_out = cur;
                cur = func_8007352C((s32)&s);
            }
            s.y += 5;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B4F0;
                s.table = &D_8009B51C;
                for (k = 0; k < vals.v[j]; k++) {
                    if (j) {
                        s.x = k * 16 + 0x179;
                    } else {
                        s.x = (1 - k) * 16 + 0xE2;
                    }
                    s.sprt_out = cur;
                    cur = func_8007352C((s32)&s);
                }
            }
        }
    }

    s.header = &D_8009B4E4;
    s.x = 0;
    if (D_8009BD38.unk10 == 2) {
        y = 0xC6;
    } else if (D_8009BD38.unk10 == 1) {
        y = 0xC2;
    } else {
        y = 0xBE;
    }
    s.y = y + 3;
    s.table = &D_8009B514;
    s.sprt_out = cur;
    cur = func_8007352C((s32)&s);
    vals.word = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        if (((arg0 >> (i * 4)) & 3) != 3) {
            vals.v[0] += (arg0 >> (i * 4)) & 3;
        }
        if (((arg0 >> (i * 4 + 2)) & 3) != 3) {
            vals.v[1] += (arg0 >> (i * 4 + 2)) & 3;
        }
    }
    i = y;
    for (j = 0; j < 2; j++) {
        s.header = &D_8009B4F0;
        s.table = &D_8009B51C;
        for (k = 0; k < vals.v[j]; k++) {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;
            } else {
                s.x = 0xF2 - (k >> 1) * 20;
            }
            s.y = i + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B524, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, mode_off);
    mode_off += 0xC;

    s.ot_idx = arg2;
    wins[0] = wins[1] = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        s.header = &D_8009ADB4;
        s.semi = 0;
        vals.v[0] = (arg0 >> (i * 4)) & 3;
        vals.v[1] = (arg0 >> (i * 4 + 2)) & 3;
        s.col_r = s.col_g = s.col_b = 0x40;
        for (j = 0; j < 2; j++) {
            if (vals.v[j] <= *(j ? &vals.v[0] : &vals.v[1])) {
                if (vals.v[j] != 3) {
                    s.has_color = 1;
                } else {
                    s.has_color = 0;
                }
            } else {
                if (vals.v[j] != 3) {
                    wins[j]++;
                }
                s.has_color = 0;
            }
            c = D_8009BD24[j][i].chr;
            if (c >= 12) {
                c -= 2;
            }
            s.table = UesrWorkDef[c];
            s.x = j * 320 + D_8009B58C[c];
            if (D_8009BD38.unk10 == 2) {
                s.y = i * 24 - 8;
            } else if (D_8009BD38.unk10 == 1) {
                s.y = i * 24 + 3;
            } else {
                s.y = i * 34 + 3;
            }
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            if (D_8009BD24[j][i].chr == 8) {
                s.table = D_8009ADC0;
                s.sprt_out = cur;
                cur = func_8007352C((s32)&s);
            }
        }
        s.has_color = 0;
        if (vals.v[0] == 3) {
            s.y += 0x4C;
            s.scale_x = 0x100;
            s.x = 0;
            s.semi = 0;
            s.scale_y = 0x400;
            s.y += 6;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B398[j + 2];
                s.table = D_8009B490[j];
                s.ft4_out = ft4;
                ft4 = func_80073728((s32)&s, 0);
                s.table = &D_8009B490[j][1];
                s.ft4_out = ft4;
                ft4 = func_80073728((s32)&s, 0);
            }
        }
    }

    s.header = &D_8009B398[0];
    s.semi = 0;
    if (D_8009BD38.unk10 == 2) {
        s.y = 0xC9;
    } else if (D_8009BD38.unk10 == 1) {
        s.y = 0xC5;
    } else {
        s.y = 0xC1;
    }
    for (j = 0; j < 2; j++) {
        s.x = j * 70 + 0x113;
        if (wins[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[wins[j]];
        s.table->unk0 = s.table->unk2 = 0;
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
    }

    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 8;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 0x19D;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    if (D_8009BD38.unk10 == 2) {
        tile->x0 = 0x5E;
        tile->y0 = 0xC1;
    } else if (D_8009BD38.unk10 == 1) {
        tile->x0 = 0x5E;
        tile->y0 = 0xBD;
    } else {
        tile->x0 = 0x5E;
        tile->y0 = 0xB9;
    }
    tile->w = 0x1C5;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009ADB4, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, mode_off);
    return end_off - arg1;
}
