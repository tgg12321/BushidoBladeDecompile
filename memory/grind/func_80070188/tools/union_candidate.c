extern s16 D_800A3530[];
extern s16 D_800A3534[];
void func_80070188(s32 arg0) {
    DescF97C s;
    s32 *sheets;
    s16 *col;
    s16 *row;
    u8 *flags;
    s16 i;
    s16 port;
    s16 port_ofs;
    s32 c;

    s.semi = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    sheets = *(s32 **)(D_800A35A8 + 0x74);
    s.header = sheets[0];
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x20, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3554); i++) {
        col = &D_800A3588[i];
        row = &D_800A358C[i];
        flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
        D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
        port = i - port_ofs;
        if (D_800A3560.rec[i].unk0 == 0xFF) {
            ((s16 *)D_800A35C4)[i] = 0x1E;
            if (D_800A354C & (0x4000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)++;
                    if (*row >= 5) {
                        *row = 0;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            } else if (D_800A354C & (0x1000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)--;
                    if (*row < 0) {
                        *row = 4;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            }
            if (D_800A354C & (0x2000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)++;
                    if (*col > D_800A35B4) {
                        *col = 0;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            } else if (D_800A354C & (0x8000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)--;
                    if (*col < 0) {
                        *col = D_800A35B4;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            }
            D_800A3530[i] = 7;
            D_800A3534[i] = 0;
        } else {
            D_800A3530[i] = 9;
            if (((s16 *)D_800A35C4)[i] != 0) {
                ((s16 *)D_800A35C4)[i]--;
            }
            if (D_800A3534[i] < 12) {
                D_800A3534[i]++;
            }
            s.header = sheets[3];
            *(u8 *)(s.header + 2) = D_800A3534[i];
            s.table = s.header + 0xC;
            s.has_color = 0;
            s.x = *col * 116 + 0x6E + (*col >> 1) * 20;
            s.y = *row * 16 + 0x80;
            s.ot_idx = 7;
            s.out = ((s32 *)arg0)[4];
            ((s32 *)arg0)[4] = func_8007352C((s32)&s);
        }
        if ((D_800A354C & (0x40 << (port * 16))) && D_800A3560.rec[i].unk0 == 0xFF) {
            if (*flags & 1) {
                D_800A3560.rec[i].unk2 = 0xFF;
                D_800A3590[i] = 2;
                D_800A3560.rec[i].unk0 = D_8009BC40[*row][*col].unk1;
                func_8005C650(D_8009BC40[*row][*col].value + 0xB, 0x7F, 0x7F);
                D_800A35C8[0] = 0xF;
                D_800A35C8[1] = 0x14;
                if (D_800A35BC == 2 && (*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 0) {
                    if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                        D_800A3588[1] = 2;
                    } else {
                        D_800A3588[1] = 0;
                    }
                    D_800A358C[1] = 0;
                    if (((s16 *)D_800A35C4)[0] == 0) {
                        D_800A3560.rec[1].unk1 = D_8009BC40[0][D_800A3588[1]].value;
                        D_8009BC7C[D_8009BC40[0][D_800A3588[1]].value] |= 4;
                    }
                    break;
                }
            } else {
                func_8005C650(0xA, 0x7F, 0x7F);
            }
        } else if (D_800A354C & (0x10 << (port * 16))) {
            if (D_800A3578 == 0) {
                func_8005C650(2, 0x7F, 0x7F);
                if (D_800A3560.rec[i].unk0 == 0xFF && D_800A3554 == 0) {
                    D_800A35A0 = -1;
                } else if (D_800A3554 == 1) {
                    if (D_800A3560.rec[D_800A3554].unk0 == 0xFF) {
                        D_800A3554 = 0;
                        D_800A3560.rec[0].unk0 = 0xFF;
                    } else {
                        D_800A3560.rec[D_800A3554].unk0 = 0xFF;
                    }
                    *flags &= ~4;
                } else if (D_800A3560.rec[i].unk0 != 0xFF && (*flags & 1)) {
                    D_800A3560.rec[i].unk0 = 0xFF;
                }
            }
        }
        if (((s16 *)D_800A35C4)[i] != 0) {
            s.has_color = 1;
        } else {
            s.has_color = 0;
        }
        if (D_800A3554 != 0 && i != 0) {
            s.header = sheets[i + 1];
        } else {
            s.header = sheets[i];
        }
        s.table = s.header + 0xC;
        c = ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << D_800A3530[i]) + i * 511) * 63) >> 12) - 0x40;
        s.col_b = c;
        s.col_g = c;
        s.col_r = c;
        s.x = *col * 116 + 0x4E + (*col >> 1) * 20;
        s.y = *row * 16 + 0x80;
        s.ot_idx = 7;
        s.out = ((s32 *)arg0)[4];
        ((s32 *)arg0)[4] = func_8007352C((s32)&s);
    }
    if (D_800A3580 == 0 && D_800A3560.rec[0].unk0 != 0xFF && ((s16 *)D_800A35C4)[0] == 0) {
        if (D_800A35BC == 2 && (*(s32 *)(D_800A3568 + 0x14) & 0x20000)) {
            D_800A3554 = 1;
        }
        if ((D_800A3560.rec[1].unk0 != 0xFF && ((s16 *)D_800A35C4)[1] == 0) || D_800A35B0 + D_800A3554 == 0) {
            D_800A3578 = 1;
            D_800A3558 = 0;
            if (D_800A35BC == 2) {
                D_800A3560.rec[1].unk2 = 0xFF;
                D_800A3590[1] = 2;
            }
            D_800A3584 = 1;
        }
    }
    func_8005C6D0();
}
