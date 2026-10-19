/* candidate.c -- func_800288C8 (src/main/17AFC.c:885-1049)
 * Session 1 (recon): body already on `main` measures sandbox --disable all
 * == 0 (465/465) and the full build's SHA1 == oracle. This file banks the
 * exact body found in src/ at session start so the next session has a known
 * byte-exact reference even if src/ drifts. Apply verbatim over
 * src/main/17AFC.c's func_800288C8 if it ever reverts.
 *
 * GTE: gte_Lzc (gte_ldlzc, gte_nop x2, gte_stlzc), inline_o.h class
 * (inline-asm-policy, owner ruling 2026-09-26). Everything else is ordinary C
 * except the three FAKE-annotated locals below (dist, tbl, shift). */
void func_800288C8(void) {
    s32 lzc_out;
    s32 var_1;
    s32 i;
    s32 j;
    s32 c;
    s32 var_5;
    s32 var_6;
    s32 var_7;
    s32 var_8;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 var_12;
    /* FAKE: two values -- the squared distance, then its square root (clamped
     * to 1); a separate squared-distance local reorders the copy and the r*r
     * test (score 7) */
    s32 dist;
    /* FAKE: two values -- the gte_Lzc input copy of the squared distance,
     * then the table byte; a fresh copy local (or dist itself) takes $v1, not
     * the target's $a0: score 13 (Q28) */
    s32 tbl;
    s32 var_15;
    s32 var_16;
    s32 flag1;
    s32 flag2;
    s32 var_19;

    if (D_80101EC8[0].unk_6A == 0x28 || D_80101EC8[1].unk_6A == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        SPAD->unk78[c][0].x = SPAD->unkA8[c][1].x;
        SPAD->unk78[c][0].y = SPAD->unkA8[c][1].y;
        SPAD->unk78[c][0].z = SPAD->unkA8[c][1].z;
        SPAD->unk78[c][1].x = (SPAD->unkA8[c][15].x + SPAD->unkA8[c][19].x) / 2;
        SPAD->unk78[c][1].y = (SPAD->unkA8[c][15].y + SPAD->unkA8[c][19].y) / 2;
        SPAD->unk78[c][1].z = (SPAD->unkA8[c][15].z + SPAD->unkA8[c][19].z) / 2;
    }
    var_1 = 0;
    var_5 = var_6 = var_7 = var_8 = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = SPAD->unk78[0][i].x - SPAD->unk78[1][j].x;
            dy = SPAD->unk78[0][i].y - SPAD->unk78[1][j].y;
            dz = SPAD->unk78[0][i].z - SPAD->unk78[1][j].z;
            var_12 = D_800A3144[i] + D_800A3144[j];
            if (dx > var_12 || dx < -var_12 || dy > var_12 || dy < -var_12 ||
                dz > var_12 || dz < -var_12) {
                continue;
            }
            dist = dx * dx + dy * dy + dz * dz;
            if (var_12 * var_12 < dist) {
                continue;
            }
            var_1++;
            tbl = dist;
            if ((u32)dist < 0x400) {
                dist = (u32)g_sqrt_table_u8[dist] >> 3;
            } else {
                /* gte_Lzc(tbl, &lzc_out) -- gtemac.h :174-178: gte_ldlzc,
                 * gte_nop x2, gte_stlzc */
                __asm__ volatile ("move  $12,%0": :"r"(tbl):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                {
                    /* FAKE: two values -- the leading-zero count, then the
                     * table shift; as one expression `shift = 0x16 - (lzc_out &
                     * ~1)` the count lands in $a0 (target: $v1 / $v0) */
                    s32 shift;
                    shift = lzc_out;
                    shift = 0x16 - (shift & ~1);
                    tbl = g_sqrt_table_u8[(u32)dist >> shift];
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            var_15 = var_12 - dist;
            if (var_15 > 0x80) {
                var_15 = 0x80;
            }
            if (dist == 0) {
                dist = 1;
            }
            var_16 = (D_80101EC8[1].unk_20 - D_80101EC8[0].unk_20) / 4;
            var_5 += (dx * (var_15 * (0x400 + var_16)) / dist) >> 10;
            var_6 += (dz * (var_15 * (0x400 + var_16)) / dist) >> 10;
            var_7 -= (dx * (var_15 * (0x400 - var_16)) / dist) >> 10;
            var_8 -= (dz * (var_15 * (0x400 - var_16)) / dist) >> 10;
        }
    }
    if (var_1 == 0) {
        return;
    }
    flag1 = 0;
    if (D_80101EC8[0].unk_6A == 0x13 || D_80101EC8[0].unk_6A == 0x1B ||
        D_80101EC8[0].unk_6A == 0x30) {
        if (D_80101EC8[0].unk_14E > 0x9C4) {
            flag1 = 1;
        }
    }
    flag2 = 0;
    if (D_80101EC8[1].unk_6A == 0x13 || D_80101EC8[1].unk_6A == 0x1B ||
        D_80101EC8[1].unk_6A == 0x30) {
        if (D_80101EC8[1].unk_14E > 0x9C4) {
            flag2 = 1;
        }
    }
    var_19 = (D_80101EC8[0].unk_1C8.vy - D_80101EC8[1].unk_1C8.vy) & 0xFFF;
    if (var_19 >= 0x800) {
        var_19 = 0x1000 - var_19;
    }
    if (flag1) {
        if (flag2) {
            if (var_19 > 0x600) {
                func_80032854(0, 0x21, &D_80101EC8[0].unk_F4.x, 0);
                D_80101EC8[1].unk_286 = 0xF;
                D_80101EC8[0].unk_286 = 0xF;
                func_80027A58(&D_80101EC8[0]);
                func_80027A58(&D_80101EC8[1]);
                var_1 = 0;
            }
        } else {
            if (var_19 < 0x400 && D_80101EC8[1].unk_6A == 0x15) {
                D_80101EC8[1].unk_286 = 0x10;
                func_80027A58(&D_80101EC8[1]);
            }
            if (var_19 > 0x600 && D_80101EC8[1].unk_6A == 0x15) {
                if (D_80101EC8[0].unk_0E == 6 || D_80101EC8[0].unk_0E == 7 ||
                    D_80101EC8[1].unk_0E == 6 || D_80101EC8[1].unk_0E == 7) {
                    func_80032854(1, 0x21, &D_80101EC8[1].unk_F4.x, 0);
                    func_80032854(1, 0x2D, &D_80101EC8[1].unk_F4.x, 0);
                    D_80101EC8[1].unk_286 = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (flag2) {
        if (var_19 < 0x400 && D_80101EC8[0].unk_6A == 0x15) {
            D_80101EC8[0].unk_286 = 0x10;
            func_80027A58(&D_80101EC8[0]);
        }
        if (var_19 > 0x600 && D_80101EC8[0].unk_6A == 0x15) {
            if (D_80101EC8[0].unk_0E == 6 || D_80101EC8[0].unk_0E == 7 ||
                D_80101EC8[1].unk_0E == 6 || D_80101EC8[1].unk_0E == 7) {
                func_80032854(0, 0x21, &D_80101EC8[0].unk_F4.x, 0);
                func_80032854(0, 0x2D, &D_80101EC8[0].unk_F4.x, 0);
                D_80101EC8[0].unk_286 = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (var_1 != 0) {
        D_80101EC8[0].unk_134.vx += var_5;
        D_80101EC8[0].unk_134.vz += var_6;
        D_80101EC8[1].unk_134.vx += var_7;
        D_80101EC8[1].unk_134.vz += var_8;
    }
}
