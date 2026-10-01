extern void func_8002CA8C(u8 *a0, s32 a1, s32 a2);
extern s32 func_8002CD58(u8 *obj);
void func_8002AB08(s32 mode) {
    u8 *scr = (u8 *)0x1F8002B8;
    /* FAKE: second handle to D_800A37E8 (pointer-alias-fake-exception). The target rematerializes
     * the address at each of its three call sites (lui/addiu into $a1/$a3); spelled directly, cse
     * keeps it in one callee-saved register across the first call (sandbox 6). Same local as
     * func_80027AD8's `vec = &D_800A37E8;` in this file. Receipts: memory/grind/func_8002AB08/r11/
     * scores_alias.txt, evidence.md s3. */
    s32 i;

    for (i = 0; i < 2; i++) {
        PracticeMenuRec *other;
        PracticeMenuRec *self;
        u32 hit;
        u32 deep;
        u32 mask_a;
        u32 mask_b;
        u32 mask_c;
        s32 npass;
        s32 pass;
        /* alt: two values -- the pass's blade (0: the unk00 / unk_210 points, 1: the unk48 / unk_234
         * points; also read by the knockback's velocity index, the last pass's blade, as the target
         * does at 0x8002B8C0), then whether the nearest hit came from that blade. The 4/5 arm's
         * `alt = 0;` re-stores the held 0 (target `move $fp,$zero` 0x8002AF18), owner Q85 (rules 9cdb9cd08).
         * Ruling 11 (reused-local-necessity.md); (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 alt;
        /* temp1 / temp2: two values each -- the indices of the pass's two points (per-branch constants,
         * Q20), then the squared distance of the opponent to the blade's first / second point. The
         * unk_8C arm's `temp1 = 0; temp2 = 1;` re-store the held values (target 0x8002AEF0 /
         * 0x8002AEF4, the stores that arm shares with pass 0), owner Q85 (rules 9cdb9cd08). Ruling 11;
         * (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 temp1;
        s32 temp2;
        s32 nseg;
        /* idx: two values -- the triangle index of the segment loop, then the index of the nearest
         * hit, the Q34 plain copy `idx = temp3;` (target `addu $s5,$s4,$zero` 0x8002B538). Ruling 11;
         * (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 idx;
        /* dx / dy / dz: the x / y / z offset between two points, one value per use -- the segment's
         * length vector, a hit point from the reference point, the blade's two points from the opponent,
         * the blade direction, the scaled push (dy: the first two only). Ruling 11; (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 dx;
        s32 dy;
        s32 dz;
        s32 best;
        /* temp3: two values -- the hit-slot counter of the nearest-hit search, then whether the
         * nearest hit came from the alternate blade. Ruling 11; (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 temp3;
        s32 j;
        u8 *rec;
        s32 near;
        s32 guard;
        s32 bit;
        s32 diff;
        s32 ang;
        /* work: four values -- the segment length squared, a hit's distance past its radius, the
         * facing difference to the opponent's velocity, then the push weight. Ruling 11; (D): memory/grind/func_8002AB08/r11/README.md. */
        s32 work;
        s32 flag;
        s32 strong;

        if (mode == 1 && D_800A38AE == i) {
            continue;
        }
        self = &g_practice_menu_table[i];
        other = g_practice_menu_table;
        if (i == 0) {
            other++;
        }
        mask_a = 0;
        mask_b = 0;
        mask_c = 0;
        npass = 0;
        hit = 0;
        deep = 0;
        if (other->unk_96 == 0 && other->unk_92 != 0 && other->unk_3C != 0) {
            npass = other->unk_0C != 0x1F;
        }
        if (other->unk_8C != 0) {
            npass++;
        }
        if (npass != 0 && (other->unk_0E == 4 || other->unk_0E == 5)) {
            npass++;
        }
        if ((other->unk_0E == 6 || other->unk_0E == 7)
            && (other->unk_6A == 2 || other->unk_6A == 0x1B || other->unk_6A == 0x28
                || other->unk_6A == 0x26)
            && other->unk_40 == other->unk_A1[0]) {
            if (other->unk_34A != 0) {
                other->unk_34A -= 1;
                *(LeafPos *)(scr + 0x0) = SPAD->unk00[i == 0][1];
                *(LeafPos *)(scr + 0xC) = SPAD->unk00[i == 0][0];
                func_8002A458((u8 *)self, &hit, &deep, 0);
                mask_a |= hit;
            } else {
                func_80032854(i == 0, 0x32, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
                npass = 0;
            }
        }
        if ((other->unk_0C == 0x1D || other->unk_0C == 0xE) && other->unk_26C != 0
            && (other->unk_6A == 2 || other->unk_6A == 0x1B || other->unk_6A == 0x28
                || other->unk_6A == 0x26)
            && other->unk_40 == other->unk_A1[1] && other->unk_34A != 0) {
            other->unk_34A -= 1;
            *(LeafPos *)(scr + 0x0) = SPAD->unk48[i == 0][1];
            *(LeafPos *)(scr + 0xC) = SPAD->unk48[i == 0][0];
            func_8002A458((u8 *)self, &hit, &deep, other->unk_0C == 0xE);
            func_80032854(i == 0, 0x2A, (u8 *)(i == 0 ? &SPAD->unk00[1][1] : &SPAD->unk00[0][1]), 0);
            mask_a |= hit;
        }
        for (pass = 0; pass < npass; pass++) {
            /* deep_on: func_8002CA8C's third argument (it runs the deep-hit test only when set): one
             * value, written per arm as per-branch constants 1 / 1 / 0 (Q20); the unk_8C arm's
             * `deep_on = 1;` re-stores the held 1 (target 0x8002AEFC), owner Q85 (rules 9cdb9cd08).
             * Ruling 11; (D): memory/grind/func_8002AB08/r11/README.md (deep_on). */
            s32 deep_on;

            if (pass == 0) {
                alt = 0;
                temp1 = 0;
                temp2 = 1;
                deep_on = 1;
            } else if (other->unk_8C != 0) {
                alt = 1;
                temp1 = 0;
                temp2 = 1;
                deep_on = 1;
            } else if (other->unk_0E == 4 || other->unk_0E == 5) {
                alt = 0;
                temp1 = 1;
                temp2 = 2;
                deep_on = 0;
            }
            if (alt == 0) {
                *(LeafPos *)(scr + 0x0) = SPAD->unk00[i == 0][temp1];
                *(LeafPos *)(scr + 0xC) = SPAD->unk00[i == 0][temp2];
                *(LeafPos *)(scr + 0x18) = other->unk_210[temp1];
                *(LeafPos *)(scr + 0x24) = other->unk_210[temp2];
            } else {
                *(LeafPos *)(scr + 0x0) = SPAD->unk48[i == 0][temp1];
                *(LeafPos *)(scr + 0xC) = SPAD->unk48[i == 0][temp2];
                *(LeafPos *)(scr + 0x18) = other->unk_234[temp1];
                *(LeafPos *)(scr + 0x24) = other->unk_234[temp2];
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
                func_8002CA8C((u8 *)self, func_8002CD58(scr), deep_on);
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
        *(s32 *)(scr + 0xC8) = other->unk_210[1].x;
        *(s32 *)(scr + 0xCC) = other->unk_210[1].y;
        *(s32 *)(scr + 0xD0) = other->unk_210[1].z;
        for (temp3 = 0; temp3 < 22; temp3++, rec += 0x14) {
            if (hit & (1 << temp3)) {
                dx = SPAD->unkA8[i][temp3].x - *(s32 *)(scr + 0xC8);
                dy = SPAD->unkA8[i][temp3].y - *(s32 *)(scr + 0xCC);
                dz = SPAD->unkA8[i][temp3].z - *(s32 *)(scr + 0xD0);
                work = dx * dx + dy * dy + dz * dz - *(u16 *)(rec + 0xE);
                if (work < best) {
                    best = work;
                    idx = temp3;
                }
            }
        }
        if (mode == 1) {
            func_800274BC(&other->unk_114[0].vx, &D_800A37E8);
            func_80032854(i, 4, (u8 *)&SPAD->unkA8[i][idx], &D_800A37E8);
            return;
        }
        near = 0;
        if ((other->unk_40 >= other->unk_A1[0] - 3 && other->unk_40 < other->unk_A1[0])
            || (other->unk_40 >= other->unk_A1[1] - 3 && other->unk_40 < other->unk_A1[1])) {
            near = 1;
        }
        guard = 0;
        if ((other->unk_0E == 6 || other->unk_0E == 7 || other->unk_0C == 0x1D
             || other->unk_0C == 0xE)
            && (mask_a & (1 << idx))) {
            guard = 1;
        }
        if (guard == 0
            && (!(other->unk_6A == 2 || other->unk_6A == 0x1B || other->unk_6A == 0x28
                  || other->unk_6A == 0x26)
                || near == 0)) {
            bit = 1 << idx;
            temp3 = (mask_a & bit) != 0;
            if (temp3) {
                dx = SPAD->unk48[i == 0][0].x - other->unk_F4.x;
                dz = SPAD->unk48[i == 0][0].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk48[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - SPAD->unk48[i == 0][0].x;
                dz = SPAD->unk48[i == 0][1].z - SPAD->unk48[i == 0][0].z;
            } else if (mask_b & bit) {
                dx = SPAD->unk00[i == 0][2].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][2].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = (SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][2].x) / 4;
                dz = (SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][2].z) / 4;
            } else {
                dx = SPAD->unk00[i == 0][0].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][0].z - other->unk_F4.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - other->unk_F4.x;
                dz = SPAD->unk00[i == 0][1].z - other->unk_F4.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][0].x;
                dz = SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][0].z;
            }
            if (temp2 < temp1) {
                dx = -dx;
                dz = -dz;
            }
            diff = (other->unk_1D8 - ratan2(dx, dz)) & 0xFFF;
            if (diff >= 0x800) {
                diff = 0x1000 - diff;
            }
            if (diff < 0x400) {
                work = (ratan2(other->unk_114[alt].vx, other->unk_114[alt].vz) + 0x800
                     - self->unk_1D8) & 0xFFF;
                if (work >= 0x800) {
                    work = 0x1000 - work;
                }
                work = 0x400 - work;
                if (work < 0) {
                    work = 0;
                }
                dx = dx / 4 + (other->unk_114[temp3].vx * work / 2 >> 11);
                dz = dz / 4 + (other->unk_114[temp3].vz * work / 2 >> 11);
                if (other->unk_6A == 3 || other->unk_6A == 7 || other->unk_6A == 0xD
                    || other->unk_6A == 0x2C) {
                    self->unk_134.vx += dx / 2;
                    self->unk_134.vz += dz / 2;
                } else {
                    self->unk_134.vx += dx;
                    self->unk_134.vz += dz;
                }
                if ((self->unk_6A == 0x13 || self->unk_6A == 0x1B || self->unk_6A == 0x30)
                    && other->unk_6A == 0x15) {
                    if (self->unk_0E == 6 || self->unk_0E == 7 || other->unk_0E == 6
                        || other->unk_0E == 7) {
                        other->unk_286 = 5;
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
        ang = (self->unk_1C8.vy - other->unk_1C8.vy) & 0xFFF;
        if (ang >= 0x800) {
            ang = 0x1000 - ang;
        }
        alt = (mask_a & (1 << idx)) != 0;
        if (other->unk_AE == 0) {
            continue;
        }
        if (other->unk_8C != 0
            && (other->unk_40 < other->unk_A1[alt] || other->unk_40 > other->unk_A3[alt])) {
            continue;
        }
        flag = 0;
        if (!(other->unk_0E == 6 || other->unk_0E == 7)) {
            if ((other->unk_0C == 0x1D || other->unk_0C == 0xE) && alt != 0) {
                flag = 1;
            } else {
                func_800274BC(&other->unk_114[alt].vx, &D_800A37E8);
            }
        }
        if (D_800A3140 == 0) {
            strong = deep & (1 << idx);
        } else {
            strong = 1;
        }
        if (mask_b & (1 << idx) & ~mask_c) {
            strong = 0;
        }
        /* func_80027AD8's sixth argument is a record pointer on its pass-1 calls (func_80031B24) and,
         * on this pass-0 call, the 0/1 alternate-blade flag the target passes in that slot: its
         * `tbl == NULL ? 0xB : 0x19` picks the same reaction func_80029454 picks from +0x8C. */
        func_80027AD8(0, (u8 *)self, idx, ang, strong, (Tbl8008E194 *)alt, flag, 0);
        other->unk_AD = 0;
    }
}
