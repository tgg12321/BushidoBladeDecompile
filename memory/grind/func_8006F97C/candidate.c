typedef struct DescF97C {
    s32 header;
    s32 table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8  has_color;
    u8  col_r;
    u8  col_g;
    u8  col_b;
} DescF97C;
extern void func_80070188(s32);
extern void func_80073200(s32);
void func_8006F97C(s32 *arg0) {
    DescF97C s;
    s16 shift[4];
    u16 rect[4];
    s32 *ctx;
    s32 cells;
    s16 i;
    s16 row;
    s16 col;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.semi = 0;
    s.col_r = s.col_g = s.col_b = 0x70;
    ctx = *(s32 **)(D_800A35A8 + 0x60);
    s.header = ctx[0];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x34, arg0[6]);
    arg0[6] += 0xC;

    s.ot_idx = 0xC;
    s.x = 0x82;
    s.y = 0x86;
    s.header = ctx[0];
    cells = s.header + 0x24;
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 5) {
            s.header = s.header + 12 + i * 12;
            if (((s16 *)D_800A35C4)[i] != 0) {
                s.x += (rsin((((s32 *)D_800A35C4)[2] * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((((s32 *)D_800A35C4)[2] << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (((s16 *)D_800A35C4)[i] != 0x1E) {
                s.y += (((s16 *)D_800A35C4)[i] * rsin((((s32 *)D_800A35C4)[2] * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table = cells;
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);

    s.x = 0x17E;
    s.y = 0x86;
    s.header = ctx[0];
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 4) {
            s.header = s.header + 12 + i * 12;
            if (((s16 *)D_800A35C4)[i] != 0) {
                s.x += (rsin((((s32 *)D_800A35C4)[2] * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((((s32 *)D_800A35C4)[2] << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (((s16 *)D_800A35C4)[i] != 0x1E) {
                s.y += (((s16 *)D_800A35C4)[i] * rsin((((s32 *)D_800A35C4)[2] * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table += *(u8 *)(s.header + 2) * 8;
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);

    s.x = 0;
    s.y = 0;
    s.header = ctx[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x2C, arg0[6]);
    arg0[6] += 0xC;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 5; col++) {
            if (((u32)D_800A35BC < 2 || D_800A35BC == 2 || D_800A35BC == 3) && col == 4 &&
                (row == 1 || row == 3)) {
                continue;
            }
            if (D_8009BC7C[D_8009BC40[col][row].value] & 1) {
                s.y = 0;
                s.x = 0;
                s.has_color = 0;
                for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
                    s32 rec = i * 3;

                    if (D_800A3560[rec] == 0xFF) {
                        shift[i] = 7;
                    } else {
                        shift[i] = 9;
                    }
                    if (col == D_800A358C[i] && row == D_800A3588[i] && ((s16 *)D_800A35C4)[i] != 0) {
                        s.has_color = 1;
                        s.col_r = s.col_g = s.col_b =
                            ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << shift[i]) + i * 511) * 63) >> 12) - 0x40;
                        break;
                    }
                }
                s.header = ctx[D_8009BC40[col][row].value + 1];
                cells = s.header + 0xC;
                s.table = cells;
                s.out = arg0[4];
                s.ot_idx = 0xA;
                arg0[4] = func_8007352C((s32)&s);
            } else {
                s.y = col << 4;
                s.x = row * 116 + (row >> 1) * 20;
                s.has_color = 0;
                s.header = ctx[21];
                cells = s.header + 0xC;
                s.table = cells;
                s.out = arg0[4];
                s.ot_idx = 0xA;
                arg0[4] = func_8007352C((s32)&s);
            }
        }
    }

    s.y = 0;
    s.x = 0;
    s.has_color = 0;
    s.header = ctx[22];
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[4];
    s.ot_idx = 1;
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 4, arg0[6]);
    arg0[6] += 0xC;
    rect[2] = 0xF3;
    rect[0] = 0xC6;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 1);
    func_80070188((s32)arg0);
    func_8006ECF4((s32)arg0);
    D_800A32E8 = D_800A3564;
    D_800A32E9 = D_800A3554;
    func_80072E10((s32)arg0);
    func_80073200((s32)arg0);
}
