typedef struct {
    u8 pad0[0xC - 0x0];
    s16 c;
    s16 e;
    u8 pad10[0x3C - 0x10];
    s32 f3c;
    s16 frame;
    u8 pad42[0x6A - 0x42];
    s16 move;
    u8 pad6C[0x8C - 0x6C];
    s16 f8c;
    u8 pad8E[0x92 - 0x8E];
    s16 f92;
    u8 pad94[0x96 - 0x94];
    s16 f96;
    u8 pad98[0xA1 - 0x98];
    u8 win[3];
    u8 padA4[0xAD - 0xA4];
    u8 fad;
    u8 fae;
    u8 padAF[0xF4 - 0xAF];
    Vec3i32 pos;
    u8 pad100[0x114 - 0x100];
    Vec4i32 vel[2];
    Vec4i32 nudge;
    u8 pad144[0x1C8 - 0x144];
    SVec4i16 rot;
    u8 pad1D0[0x1D8 - 0x1D0];
    s16 yaw;
    u8 pad1DA[0x210 - 0x1DA];
    LeafPos seg[3];
    LeafPos seg2[2];
    u8 pad24C[0x26C - 0x24C];
    s16 f26c;
    u8 pad26E[0x286 - 0x26E];
    s16 f286;
    u8 pad288[0x34A - 0x288];
    u8 f34a;
    u8 pad34B[0x44C - 0x34B];
} R8002AB08;
extern void func_8002CA8C(u8 *a0, s32 a1, s32 a2);
extern s32 func_8002CD58(u8 *obj);
static inline void copy_pt(u8 *dst, LeafPos *pts, s32 n) {
    *(LeafPos *)dst = pts[n];
}
void func_8002AB08(s32 mode) {
    u8 *scr = (u8 *)0x1F8002B8;
    s16 *vec = &D_800A37E8;
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
    s32 temp1;
    s32 temp2;
    s32 c;
    s32 nseg;
    s32 idx;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 best;
    s32 temp3;
    s32 j;
    u8 *rec;
    s32 near;
    s32 guard;
    s32 bit;
    s32 diff;
    s32 ang;
    s32 work;
    s32 flag;
    s32 strong;
    u8 nearest_;

    for (i = 0; i < 2; i++) {
        if (mode == 1 && D_800A38AE == i) {
            continue;
        }
        self = (u8 *)g_practice_menu_table + i * 0x44C;
        other = (u8 *)g_practice_menu_table;
        if (i == 0) {
            other += 0x44C;
        }
        mask_a = 0;
        mask_b = 0;
        mask_c = 0;
        npass = 0;
        hit = 0;
        deep = 0;
        if (((R8002AB08 *)other)->f96 == 0 && ((R8002AB08 *)other)->f92 != 0 && ((R8002AB08 *)other)->f3c != 0) {
            npass = ((R8002AB08 *)other)->c != 0x1F;
        }
        if (((R8002AB08 *)other)->f8c != 0) {
            npass++;
        }
        if (npass != 0 && ((u16)((R8002AB08 *)other)->e == 4 || (u16)((R8002AB08 *)other)->e == 5)) {
            npass++;
        }
        if (((u16)((R8002AB08 *)other)->e == 6 || (u16)((R8002AB08 *)other)->e == 7)
            && ((u16)((R8002AB08 *)other)->move == 2 || (u16)((R8002AB08 *)other)->move == 0x1B || (u16)((R8002AB08 *)other)->move == 0x28
                || (u16)((R8002AB08 *)other)->move == 0x26)
            && ((R8002AB08 *)other)->frame == ((R8002AB08 *)other)->win[0]) {
            if (((R8002AB08 *)other)->f34a != 0) {
                ((R8002AB08 *)other)->f34a -= 1;
                *(LeafPos *)(scr + 0x0) = SPAD->unk00[i == 0][1];
                *(LeafPos *)(scr + 0xC) = SPAD->unk00[i == 0][0];
                func_8002A458(self, (s32 *)&hit, (s32 *)&deep, 0);
                mask_a |= hit;
            } else {
                func_80032854(i == 0, 0x32, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
                npass = 0;
            }
        }
        if ((((R8002AB08 *)other)->c == 0x1D || ((R8002AB08 *)other)->c == 0xE) && ((R8002AB08 *)other)->f26c != 0
            && ((u16)((R8002AB08 *)other)->move == 2 || (u16)((R8002AB08 *)other)->move == 0x1B || (u16)((R8002AB08 *)other)->move == 0x28
                || (u16)((R8002AB08 *)other)->move == 0x26)
            && ((R8002AB08 *)other)->frame == ((R8002AB08 *)other)->win[1] && ((R8002AB08 *)other)->f34a != 0) {
            ((R8002AB08 *)other)->f34a -= 1;
            *(LeafPos *)(scr + 0x0) = SPAD->unk48[i == 0][1];
            *(LeafPos *)(scr + 0xC) = SPAD->unk48[i == 0][0];
            func_8002A458(self, (s32 *)&hit, (s32 *)&deep, ((R8002AB08 *)other)->c == 0xE);
            func_80032854(i == 0, 0x2A, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
            mask_a |= hit;
        }
        for (pass = 0; pass < npass; pass++) {
            if (pass == 0) {
                alt = 0;
                temp1 = 0;
                temp2 = 1;
                c = 1;
            } else if (((R8002AB08 *)other)->f8c != 0) {
                alt = 1;
                temp1 = 0;
                temp2 = 1;
                c = 1;
            } else if ((u16)((R8002AB08 *)other)->e == 4 || (u16)((R8002AB08 *)other)->e == 5) {
                alt = 0;
                temp1 = 1;
                temp2 = 2;
                c = 0;
            }
            if (alt == 0) {
                copy_pt(scr + 0x0, i == 0 ? SPAD->unk00[1] : SPAD->unk00[0], temp1);
                copy_pt(scr + 0xC, i == 0 ? SPAD->unk00[1] : SPAD->unk00[0], temp2);
                *(LeafPos *)(scr + 0x18) = ((R8002AB08 *)other)->seg[temp1];
                *(LeafPos *)(scr + 0x24) = ((R8002AB08 *)other)->seg[temp2];
            } else {
                copy_pt(scr + 0x0, i == 0 ? SPAD->unk48[1] : SPAD->unk48[0], temp1);
                copy_pt(scr + 0xC, i == 0 ? SPAD->unk48[1] : SPAD->unk48[0], temp2);
                *(LeafPos *)(scr + 0x18) = ((R8002AB08 *)other)->seg2[temp1];
                *(LeafPos *)(scr + 0x24) = ((R8002AB08 *)other)->seg2[temp2];
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
            work = dx * dx + dy * dy + dz * dz;
            nseg = 2;
            if (work > 6249999) {
                nseg = 4;
            }
            for (idx = 0; idx < nseg; idx++) {
                if (nseg == 2) {
                    if (idx == 0) {
                        *(u8 **)(scr + 0x60) = scr + 0x18;
                        *(u8 **)(scr + 0x64) = scr + 0x24;
                        *(u8 **)(scr + 0x68) = scr + 0xC;
                    } else {
                        *(u8 **)(scr + 0x60) = scr;
                        *(u8 **)(scr + 0x64) = scr + 0xC;
                        *(u8 **)(scr + 0x68) = scr + 0x18;
                    }
                } else {
                    switch (idx) {
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
        *(s32 *)(scr + 0xC8) = ((R8002AB08 *)other)->seg[1].x;
        *(s32 *)(scr + 0xCC) = ((R8002AB08 *)other)->seg[1].y;
        *(s32 *)(scr + 0xD0) = ((R8002AB08 *)other)->seg[1].z;
        for (temp3 = 0; temp3 < 22; temp3++, rec += 0x14) {
            if (hit & (1 << temp3)) {
                dx = SPAD->unkA8[i][temp3].x - *(s32 *)(scr + 0xC8);
                dy = SPAD->unkA8[i][temp3].y - *(s32 *)(scr + 0xCC);
                dz = SPAD->unkA8[i][temp3].z - *(s32 *)(scr + 0xD0);
                work = dx * dx + dy * dy + dz * dz - *(u16 *)(rec + 0xE);
                if (work < best) {
                    best = work;
                    nearest_ = temp3;
                }
            }
        }
        if (mode == 1) {
            func_800274BC(&((R8002AB08 *)other)->vel[0].vx, vec);
            func_80032854(i, 4, (u8 *)&SPAD->unkA8[i][nearest_], vec);
            return;
        }
        near = 0;
        if ((((R8002AB08 *)other)->frame >= ((R8002AB08 *)other)->win[0] - 3 && ((R8002AB08 *)other)->frame < ((R8002AB08 *)other)->win[0])
            || (((R8002AB08 *)other)->frame >= ((R8002AB08 *)other)->win[1] - 3 && ((R8002AB08 *)other)->frame < ((R8002AB08 *)other)->win[1])) {
            near = 1;
        }
        guard = 0;
        if (((u16)((R8002AB08 *)other)->e == 6 || (u16)((R8002AB08 *)other)->e == 7 || ((R8002AB08 *)other)->c == 0x1D
             || ((R8002AB08 *)other)->c == 0xE)
            && (mask_a & (1 << nearest_))) {
            guard = 1;
        }
        if (guard == 0
            && (!((u16)((R8002AB08 *)other)->move == 2 || (u16)((R8002AB08 *)other)->move == 0x1B || (u16)((R8002AB08 *)other)->move == 0x28
                  || (u16)((R8002AB08 *)other)->move == 0x26)
                || near == 0)) {
            bit = 1 << nearest_;
            temp3 = (mask_a & bit) != 0;
            if (temp3) {
                dx = SPAD->unk48[i == 0][0].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk48[i == 0][0].z - ((R8002AB08 *)other)->pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk48[i == 0][1].z - ((R8002AB08 *)other)->pos.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - SPAD->unk48[i == 0][0].x;
                dz = SPAD->unk48[i == 0][1].z - SPAD->unk48[i == 0][0].z;
            } else if (mask_b & bit) {
                dx = SPAD->unk00[i == 0][2].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk00[i == 0][2].z - ((R8002AB08 *)other)->pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk00[i == 0][1].z - ((R8002AB08 *)other)->pos.z;
                temp2 = dx * dx + dz * dz;
                dx = (SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][2].x) / 4;
                dz = (SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][2].z) / 4;
            } else {
                dx = SPAD->unk00[i == 0][0].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk00[i == 0][0].z - ((R8002AB08 *)other)->pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - ((R8002AB08 *)other)->pos.x;
                dz = SPAD->unk00[i == 0][1].z - ((R8002AB08 *)other)->pos.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][0].x;
                dz = SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][0].z;
            }
            if (temp2 < temp1) {
                dx = -dx;
                dz = -dz;
            }
            diff = (((R8002AB08 *)other)->yaw - ratan2(dx, dz)) & 0xFFF;
            if (diff >= 0x800) {
                diff = 0x1000 - diff;
            }
            if (diff < 0x400) {
                work = (ratan2(((R8002AB08 *)other)->vel[alt].vx, ((R8002AB08 *)other)->vel[alt].vz) + 0x800
                     - ((R8002AB08 *)self)->yaw) & 0xFFF;
                if (work >= 0x800) {
                    work = 0x1000 - work;
                }
                work = 0x400 - work;
                if (work < 0) {
                    work = 0;
                }
                dx = dx / 4 + (((R8002AB08 *)other)->vel[temp3].vx * work / 2 >> 11);
                dz = dz / 4 + (((R8002AB08 *)other)->vel[temp3].vz * work / 2 >> 11);
                if ((u16)((R8002AB08 *)other)->move == 3 || (u16)((R8002AB08 *)other)->move == 7 || (u16)((R8002AB08 *)other)->move == 0xD
                    || (u16)((R8002AB08 *)other)->move == 0x2C) {
                    ((R8002AB08 *)self)->nudge.vx += dx / 2;
                    ((R8002AB08 *)self)->nudge.vz += dz / 2;
                } else {
                    ((R8002AB08 *)self)->nudge.vx += dx;
                    ((R8002AB08 *)self)->nudge.vz += dz;
                }
                if (((u16)((R8002AB08 *)self)->move == 0x13 || (u16)((R8002AB08 *)self)->move == 0x1B || (u16)((R8002AB08 *)self)->move == 0x30)
                    && (u16)((R8002AB08 *)other)->move == 0x15) {
                    if ((u16)((R8002AB08 *)self)->e == 6 || (u16)((R8002AB08 *)self)->e == 7 || (u16)((R8002AB08 *)other)->e == 6
                        || (u16)((R8002AB08 *)other)->e == 7) {
                        ((R8002AB08 *)other)->f286 = 5;
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
        ang = (((R8002AB08 *)self)->rot.vy - ((R8002AB08 *)other)->rot.vy) & 0xFFF;
        if (ang >= 0x800) {
            ang = 0x1000 - ang;
        }
        alt = (mask_a & (1 << nearest_)) != 0;
        if (((R8002AB08 *)other)->fae == 0) {
            continue;
        }
        if (((R8002AB08 *)other)->f8c != 0
            && (((R8002AB08 *)other)->frame < ((R8002AB08 *)other)->win[alt] || ((R8002AB08 *)other)->frame > ((R8002AB08 *)other)->win[alt + 2])) {
            continue;
        }
        flag = 0;
        if (!((u16)((R8002AB08 *)other)->e == 6 || (u16)((R8002AB08 *)other)->e == 7)) {
            if ((((R8002AB08 *)other)->c == 0x1D || ((R8002AB08 *)other)->c == 0xE) && alt != 0) {
                flag = 1;
            } else {
                func_800274BC(&((R8002AB08 *)other)->vel[alt].vx, vec);
            }
        }
        if (D_800A3140 == 0) {
            strong = deep & (1 << nearest_);
        } else {
            strong = 1;
        }
        if (mask_b & (1 << nearest_) & ~mask_c) {
            strong = 0;
        }
        func_80027AD8(0, self, nearest_, ang, strong, (Tbl8008E194 *)alt, flag, 0);
        ((R8002AB08 *)other)->fad = 0;
    }
}
