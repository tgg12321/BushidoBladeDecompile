extern void func_8002CA8C(u8 *a0, s32 a1, s32 a2);
extern s32 func_8002CD58(u8 *obj);
static inline void copy_pt(u8 *dst, LeafPos *pts, s32 n) {
    *(LeafPos *)dst = pts[n];
}
void func_8002AB08(s32 mode) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 i;
    u8 *other;
    u8 *self;
    u32 hit;
    u32 deep;
    u32 mask_a;
    u32 mask_b;
    u32 mask_c;
    s32 npass;
    s32 pass;
    s32 alt;
    s32 a;
    s32 b;
    s32 c;
    s32 nseg;
    s32 k;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 best;
    s32 d;
    s32 j;
    u8 *rec;
    s32 near;
    s32 guard;
    s32 bit;
    s32 side;
    s32 diff;
    s32 ang;
    s32 w;
    s32 flag;
    s32 strong;

    for (i = 0; i < 2; i++) {
        if (mode == 1 && D_800A38AE == i) {
            continue;
        }
        self = (u8 *)g_practice_menu_table + i * 0x44C;
        other = (u8 *)g_practice_menu_table;
        if (i == 0) {
            other += 0x44C;
        }
        npass = 0;
        mask_a = 0;
        mask_b = 0;
        mask_c = 0;
        hit = 0;
        deep = 0;
        if (*(s16 *)(other + 0x96) == 0 && *(s16 *)(other + 0x92) != 0 && *(s32 *)(other + 0x3C) != 0) {
            npass = *(s16 *)(other + 0xC) != 0x1F;
        }
        if (*(s16 *)(other + 0x8C) != 0) {
            npass++;
        }
        if (npass != 0 && (*(u16 *)(other + 0xE) == 4 || *(u16 *)(other + 0xE) == 5)) {
            npass++;
        }
        if ((*(u16 *)(other + 0xE) == 6 || *(u16 *)(other + 0xE) == 7)
            && (*(u16 *)(other + 0x6A) == 2 || *(u16 *)(other + 0x6A) == 0x1B || *(u16 *)(other + 0x6A) == 0x28
                || *(u16 *)(other + 0x6A) == 0x26)
            && *(s16 *)(other + 0x40) == *(u8 *)(other + 0xA1)) {
            if (*(u8 *)(other + 0x34A) != 0) {
                *(u8 *)(other + 0x34A) -= 1;
                *(LeafPos *)(scr + 0x0) = SPAD->unk00[i == 0][1];
                *(LeafPos *)(scr + 0xC) = SPAD->unk00[i == 0][0];
                func_8002A458(self, (s32 *)&hit, (s32 *)&deep, 0);
                mask_a |= hit;
            } else {
                func_80032854(i == 0, 0x32, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
                npass = 0;
            }
        }
        if ((*(s16 *)(other + 0xC) == 0x1D || *(s16 *)(other + 0xC) == 0xE) && *(s16 *)(other + 0x26C) != 0
            && (*(u16 *)(other + 0x6A) == 2 || *(u16 *)(other + 0x6A) == 0x1B || *(u16 *)(other + 0x6A) == 0x28
                || *(u16 *)(other + 0x6A) == 0x26)
            && *(s16 *)(other + 0x40) == *(u8 *)(other + 0xA2) && *(u8 *)(other + 0x34A) != 0) {
            *(u8 *)(other + 0x34A) -= 1;
            *(LeafPos *)(scr + 0x0) = SPAD->unk48[i == 0][1];
            *(LeafPos *)(scr + 0xC) = SPAD->unk48[i == 0][0];
            func_8002A458(self, (s32 *)&hit, (s32 *)&deep, *(s16 *)(other + 0xC) == 0xE);
            func_80032854(i == 0, 0x2A, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
            mask_a |= hit;
        }
        for (pass = 0; pass < npass; pass++) {
            if (pass == 0) {
                alt = 0;
                a = 0;
                b = 1;
                c = 1;
            } else if (*(s16 *)(other + 0x8C) != 0) {
                alt = 1;
                a = 0;
                b = 1;
                c = 1;
            } else if (*(u16 *)(other + 0xE) == 4 || *(u16 *)(other + 0xE) == 5) {
                alt = 0;
                a = 1;
                b = 2;
                c = 0;
            }
            if (alt == 0) {
                copy_pt(scr + 0x0, i == 0 ? SPAD->unk00[1] : SPAD->unk00[0], a);
                copy_pt(scr + 0xC, i == 0 ? SPAD->unk00[1] : SPAD->unk00[0], b);
                *(LeafPos *)(scr + 0x18) = ((LeafPos *)(other + 0x210))[a];
                *(LeafPos *)(scr + 0x24) = ((LeafPos *)(other + 0x210))[b];
            } else {
                copy_pt(scr + 0x0, i == 0 ? SPAD->unk48[1] : SPAD->unk48[0], a);
                copy_pt(scr + 0xC, i == 0 ? SPAD->unk48[1] : SPAD->unk48[0], b);
                *(LeafPos *)(scr + 0x18) = ((LeafPos *)(other + 0x234))[a];
                *(LeafPos *)(scr + 0x24) = ((LeafPos *)(other + 0x234))[b];
            }
            *(s32 *)(scr + 0x30) = (*(s32 *)(scr + 0x0) + *(s32 *)(scr + 0x18)) / 2;
            *(s32 *)(scr + 0x34) = (*(s32 *)(scr + 0x4) + *(s32 *)(scr + 0x1C)) / 2;
            *(s32 *)(scr + 0x38) = (*(s32 *)(scr + 0x8) + *(s32 *)(scr + 0x20)) / 2;
            *(s32 *)(scr + 0x3C) = (*(s32 *)(scr + 0xC) + *(s32 *)(scr + 0x24)) / 2;
            *(s32 *)(scr + 0x40) = (*(s32 *)(scr + 0x10) + *(s32 *)(scr + 0x28)) / 2;
            *(s32 *)(scr + 0x44) = (*(s32 *)(scr + 0x14) + *(s32 *)(scr + 0x2C)) / 2;
            dx = *(s32 *)(scr + 0xC) - *(s32 *)(scr + 0x24);
            dy = *(s32 *)(scr + 0x10) - *(s32 *)(scr + 0x28);
            dz = *(s32 *)(scr + 0x14) - *(s32 *)(scr + 0x2C);
            d = dx * dx + dy * dy + dz * dz;
            nseg = 2;
            if (d > 6249999) {
                nseg = 4;
            }
            for (k = 0; k < nseg; k++) {
                if (nseg == 2) {
                    if (k == 0) {
                        *(u8 **)(scr + 0x60) = scr + 0x18;
                        *(u8 **)(scr + 0x64) = scr + 0x24;
                        *(u8 **)(scr + 0x68) = scr + 0xC;
                    } else {
                        *(u8 **)(scr + 0x60) = scr;
                        *(u8 **)(scr + 0x64) = scr + 0xC;
                        *(u8 **)(scr + 0x68) = scr + 0x18;
                    }
                } else {
                    switch (k) {
                    case 0:
                        *(u8 **)(scr + 0x60) = scr + 0x24;
                        *(u8 **)(scr + 0x64) = scr + 0x18;
                        *(u8 **)(scr + 0x68) = scr + 0x3C;
                        break;
                    case 1:
                        *(u8 **)(scr + 0x60) = scr + 0x3C;
                        *(u8 **)(scr + 0x64) = scr + 0x30;
                        *(u8 **)(scr + 0x68) = scr + 0x18;
                        break;
                    case 2:
                        *(u8 **)(scr + 0x60) = scr + 0x3C;
                        *(u8 **)(scr + 0x64) = scr + 0x30;
                        *(u8 **)(scr + 0x68) = scr + 0xC;
                        break;
                    case 3:
                        *(u8 **)(scr + 0x60) = scr + 0xC;
                        *(u8 **)(scr + 0x64) = scr;
                        *(u8 **)(scr + 0x68) = scr + 0x30;
                        break;
                    }
                }
                *(LeafPos *)(scr + 0x84) = **(LeafPos **)(scr + 0x60);
                *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);
                for (j = 1; j < 3; j++) {
                    if ((*(s32 **)(scr + 0x60 + j * 4))[0] < *(s32 *)(scr + 0x78)) {
                        *(s32 *)(scr + 0x78) = (*(s32 **)(scr + 0x60 + j * 4))[0];
                    } else if (*(s32 *)(scr + 0x84) < (*(s32 **)(scr + 0x60 + j * 4))[0]) {
                        *(s32 *)(scr + 0x84) = (*(s32 **)(scr + 0x60 + j * 4))[0];
                    }
                    if ((*(s32 **)(scr + 0x60 + j * 4))[1] < *(s32 *)(scr + 0x7C)) {
                        *(s32 *)(scr + 0x7C) = (*(s32 **)(scr + 0x60 + j * 4))[1];
                    } else if (*(s32 *)(scr + 0x88) < (*(s32 **)(scr + 0x60 + j * 4))[1]) {
                        *(s32 *)(scr + 0x88) = (*(s32 **)(scr + 0x60 + j * 4))[1];
                    }
                    if ((*(s32 **)(scr + 0x60 + j * 4))[2] < *(s32 *)(scr + 0x80)) {
                        *(s32 *)(scr + 0x80) = (*(s32 **)(scr + 0x60 + j * 4))[2];
                    } else if (*(s32 *)(scr + 0x8C) < (*(s32 **)(scr + 0x60 + j * 4))[2]) {
                        *(s32 *)(scr + 0x8C) = (*(s32 **)(scr + 0x60 + j * 4))[2];
                    }
                }
                func_8002CA8C(self, func_8002CD58(scr), c);
                hit |= *(s32 *)(scr + 0xB4);
                deep |= *(s32 *)(scr + 0xC4);
                if (alt != 0) {
                    mask_a |= *(s32 *)(scr + 0xB4);
                }
                if (pass == 1 && alt == 0) {
                    mask_b |= *(s32 *)(scr + 0xB4);
                } else {
                    mask_c |= *(s32 *)(scr + 0xB4);
                }
            }
        }
        if (hit == 0) {
            continue;
        }
        rec = &D_800F5F68[i * 0x1B8];
        best = 0x7FFFFFFF;
        *(s32 *)(scr + 0xC8) = *(s32 *)(other + 0x21C);
        *(s32 *)(scr + 0xCC) = *(s32 *)(other + 0x220);
        *(s32 *)(scr + 0xD0) = *(s32 *)(other + 0x224);
        for (j = 0; j < 22; j++, rec += 0x14) {
            if (hit & (1 << j)) {
                dx = SPAD->unkA8[i][j].x - *(s32 *)(scr + 0xC8);
                dy = SPAD->unkA8[i][j].y - *(s32 *)(scr + 0xCC);
                dz = SPAD->unkA8[i][j].z - *(s32 *)(scr + 0xD0);
                d = dx * dx + dy * dy + dz * dz - *(u16 *)(rec + 0xE);
                if (d < best) {
                    best = d;
                    k = j;
                }
            }
        }
        if (mode == 1) {
            func_800274BC((s32 *)(other + 0x114), &D_800A37E8);
            func_80032854(i, 4, (u8 *)&SPAD->unkA8[i][k], &D_800A37E8);
            return;
        }
        near = 0;
        if ((*(s16 *)(other + 0x40) >= *(u8 *)(other + 0xA1) - 3 && *(s16 *)(other + 0x40) < *(u8 *)(other + 0xA1))
            || (*(s16 *)(other + 0x40) >= *(u8 *)(other + 0xA2) - 3 && *(s16 *)(other + 0x40) < *(u8 *)(other + 0xA2))) {
            near = 1;
        }
        guard = 0;
        if ((*(u16 *)(other + 0xE) == 6 || *(u16 *)(other + 0xE) == 7 || *(s16 *)(other + 0xC) == 0x1D
             || *(s16 *)(other + 0xC) == 0xE)
            && (mask_a & (1 << k))) {
            guard = 1;
        }
        if (guard == 0
            && (!(*(u16 *)(other + 0x6A) == 2 || *(u16 *)(other + 0x6A) == 0x1B || *(u16 *)(other + 0x6A) == 0x28
                  || *(u16 *)(other + 0x6A) == 0x26)
                || near == 0)) {
            bit = 1 << k;
            j = (mask_a & bit) != 0;
            if (j) {
                dx = SPAD->unk48[i == 0][0].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk48[i == 0][0].z - *(s32 *)(other + 0xFC);
                a = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk48[i == 0][1].z - *(s32 *)(other + 0xFC);
                b = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - SPAD->unk48[i == 0][0].x;
                dz = SPAD->unk48[i == 0][1].z - SPAD->unk48[i == 0][0].z;
            } else if (mask_b & bit) {
                dx = SPAD->unk00[i == 0][2].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk00[i == 0][2].z - *(s32 *)(other + 0xFC);
                a = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk00[i == 0][1].z - *(s32 *)(other + 0xFC);
                b = dx * dx + dz * dz;
                dx = (SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][2].x) / 4;
                dz = (SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][2].z) / 4;
            } else {
                dx = SPAD->unk00[i == 0][0].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk00[i == 0][0].z - *(s32 *)(other + 0xFC);
                a = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - *(s32 *)(other + 0xF4);
                dz = SPAD->unk00[i == 0][1].z - *(s32 *)(other + 0xFC);
                b = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][0].x;
                dz = SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][0].z;
            }
            if (b < a) {
                dx = -dx;
                dz = -dz;
            }
            diff = (*(s16 *)(other + 0x1D8) - ratan2(dx, dz)) & 0xFFF;
            if (diff >= 0x800) {
                diff = 0x1000 - diff;
            }
            if (diff < 0x400) {
                w = (ratan2(*(s32 *)(other + (alt << 4) + 0x114), *(s32 *)(other + (alt << 4) + 0x11C)) + 0x800
                     - *(s16 *)(self + 0x1D8)) & 0xFFF;
                if (w >= 0x800) {
                    w = 0x1000 - w;
                }
                w = 0x400 - w;
                if (w < 0) {
                    w = 0;
                }
                dx = dx / 4 + (*(s32 *)(other + (j << 4) + 0x114) * w / 2 >> 11);
                dz = dz / 4 + (*(s32 *)(other + (j << 4) + 0x11C) * w / 2 >> 11);
                if (*(u16 *)(other + 0x6A) == 3 || *(u16 *)(other + 0x6A) == 7 || *(u16 *)(other + 0x6A) == 0xD
                    || *(u16 *)(other + 0x6A) == 0x2C) {
                    *(s32 *)(self + 0x134) += dx / 2;
                    *(s32 *)(self + 0x13C) += dz / 2;
                } else {
                    *(s32 *)(self + 0x134) += dx;
                    *(s32 *)(self + 0x13C) += dz;
                }
                if ((*(u16 *)(self + 0x6A) == 0x13 || *(u16 *)(self + 0x6A) == 0x1B || *(u16 *)(self + 0x6A) == 0x30)
                    && *(u16 *)(other + 0x6A) == 0x15) {
                    if (*(u16 *)(self + 0xE) == 6 || *(u16 *)(self + 0xE) == 7 || *(u16 *)(other + 0xE) == 6
                        || *(u16 *)(other + 0xE) == 7) {
                        *(s16 *)(other + 0x286) = 5;
                        func_80032854(i == 0, 0x21,
                                      (u8 *)(i == 0 ? &g_practice_menu_table[1].unk_F4 : &g_practice_menu_table[0].unk_F4), 0);
                        func_80032854(i == 0, 0x2D,
                                      (u8 *)(i == 0 ? &g_practice_menu_table[1].unk_F4 : &g_practice_menu_table[0].unk_F4), 0);
                    } else {
                        D_800A38A8 = 1;
                        D_800A3876 = i;
                    }
                }
            }
        }
        ang = (*(s16 *)(self + 0x1CA) - *(s16 *)(other + 0x1CA)) & 0xFFF;
        if (ang >= 0x800) {
            ang = 0x1000 - ang;
        }
        alt = (mask_a & (1 << k)) != 0;
        if (*(u8 *)(other + 0xAE) == 0) {
            continue;
        }
        if (*(s16 *)(other + 0x8C) != 0
            && (*(s16 *)(other + 0x40) < *(u8 *)(other + alt + 0xA1) || *(s16 *)(other + 0x40) > *(u8 *)(other + alt + 0xA3))) {
            continue;
        }
        flag = 0;
        if (!(*(u16 *)(other + 0xE) == 6 || *(u16 *)(other + 0xE) == 7)) {
            if ((*(s16 *)(other + 0xC) == 0x1D || *(s16 *)(other + 0xC) == 0xE) && alt != 0) {
                flag = 1;
            } else {
                func_800274BC((s32 *)(other + 0x114 + alt * 0x10), &D_800A37E8);
            }
        }
        if (D_800A3140 == 0) {
            strong = deep & (1 << k);
        } else {
            strong = 1;
        }
        if (mask_b & (1 << k) & ~mask_c) {
            strong = 0;
        }
        func_80027AD8(0, self, k, ang, strong, (Tbl8008E194 *)alt, flag, 0);
        *(u8 *)(other + 0xAD) = 0;
    }
}
