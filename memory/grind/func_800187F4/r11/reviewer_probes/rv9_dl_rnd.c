typedef struct {
    s32 d0[3];      /* 0x00 */
    s32 d1[3];      /* 0x0C */
    s32 vel[3];     /* 0x18 */
    s32 dpos[3];    /* 0x24 */
    s32 sq[3];      /* 0x30 */
    s32 pos[3];     /* 0x3C */
    s32 cpos[3];    /* 0x48 */
    s32 unk54[3];   /* 0x54 */
    s32 nsph;       /* 0x60 */
    s32 sph[3][6];  /* 0x64 */
    s32 rad[3];     /* 0xAC */
    s32 ground;     /* 0xB8 */
    s32 force[0][3]; /* 0xBC */
} RopeScratch;
#define SCR ((RopeScratch *)0x1F800000)
void func_800187F4(s16 *arg0, s32 *arg1) {
    s32 *node;
    s32 i;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 vy_new;
    s32 tot, pen;
    s32 lz[6];

    func_80018094((s32 *)arg0, arg1);
    count = *(s16 *)((u8 *)arg1 + 4);
    node = (s32 *)arg1[3];

    for (i = 0; i < count; i++, node += 16) {
        s32 idx;
        s32 nforce;

        SCR->pos[0] = node[0];
        SCR->pos[1] = node[1];
        SCR->pos[2] = node[2];
        if (node[6] >= 0) {
            @gte_ldv0(arg1[0] + i * 8);
            @gte_rtv0tr();
            @gte_stlvnl(SCR->vel);
            if (node[6] > 0) {
                SCR->pos[0] += SCR->dpos[0];
                node[3] = node[3] / 2 + ((((SCR->vel[0] << 7) - SCR->pos[0]) * node[6]) >> 8);
                node[0] = SCR->pos[0] + node[3];
                SCR->pos[1] += SCR->dpos[1];
                node[4] = node[4] / 2 + ((((SCR->vel[1] << 7) - SCR->pos[1]) * node[6] + (0x100 - node[6]) * 25) >> 8);
                node[1] = SCR->pos[1] + node[4];
                SCR->pos[2] += SCR->dpos[2];
                node[5] = node[5] / 2 + ((((SCR->vel[2] << 7) - SCR->pos[2]) * node[6]) >> 8);
                node[2] = SCR->pos[2] + node[5];
            } else {
                node[0] = SCR->vel[0] << 7;
                node[1] = SCR->vel[1] << 7;
                node[2] = SCR->vel[2] << 7;
            }
            continue;
        }
        if (node[6] >= -0xFF) {
            @gte_ldv0(arg1[0] + i * 8);
            @gte_rtv0tr();
            @gte_stlvnl(SCR->vel);
            SCR->pos[0] -= (((SCR->vel[0] << 7) - SCR->pos[0]) * node[6]) >> 8;
            SCR->pos[1] -= (((SCR->vel[1] << 7) - SCR->pos[1]) * node[6]) >> 8;
            SCR->pos[2] -= (((SCR->vel[2] << 7) - SCR->pos[2]) * node[6]) >> 8;
        }
        vx = node[3];
        vy = node[4];
        vz = node[5];
        nforce = node[7];
        bits = node[9];
        for (idx = 0; idx < nforce; idx++) {
            s32 *f_add;

            f_add = SCR->force[bits & 0xFF];
            vx += f_add[0];
            vy += f_add[1];
            vz += f_add[2];
            if (idx == 3) {
                bits = node[10];
            } else {
                bits >>= 8;
            }
        }
        nforce = node[8];
        bits2 = node[11];
        for (idx = 0; idx < nforce; idx++) {
            s32 *f_sub;

            f_sub = SCR->force[bits2 & 0xFF];
            vx -= f_sub[0];
            vy -= f_sub[1];
            vz -= f_sub[2];
            if (idx == 3) {
                bits2 = node[12];
            } else {
                bits2 >>= 8;
            }
        }
        SCR->vel[0] = vx;
        SCR->vel[1] = vy;
        SCR->vel[2] = vz;
        if (*(s32 *)((u8 *)arg0 + 0xC) != 0) {
            s32 dg;

            dg = SCR->pos[1] - SCR->ground;
            if (dg > 0) {
                if (dg > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - ((dg < 0 ? dg + 7 : dg) >> 3);
                }
                SCR->vel[1] = vy_new;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (idx = 0; idx < SCR->nsph; idx++) {
                s32 dx0, dz0, dy1, dx1, dz1;
                s32 sq2, dist2;
                s32 temp;
                s32 work;
                s32 dy0;

                r = SCR->rad[idx];
                dy0 = SCR->cpos[1] - SCR->sph[idx][1];
                if (dy0 < -r || r < dy0) {
                    continue;
                }
                SCR->d0[1] = dy0;
                dx0 = SCR->cpos[0] - SCR->sph[idx][0];
                if (dx0 < -r || r < dx0) {
                    continue;
                }
                SCR->d0[0] = dx0;
                dz0 = SCR->cpos[2] - SCR->sph[idx][2];
                if (dz0 < -r || r < dz0) {
                    continue;
                }
                SCR->d0[2] = dz0;
                @gte_ldlvl(SCR->d0);
                @gte_sqr0();
                @gte_stlvnl(SCR->sq);
                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = work;
                if (work < 0x400) {
                    work = (&D_8008D118)[work] >> 3;
                } else {
                    s32 nbits;

                    @gte_Lzc(temp, &lz[0]);
                    nbits = lz[0];
                    nbits = 0x16 - (nbits & ~1);
                    temp = (&D_8008D118)[work >> nbits];
                    work = (temp << 16) >> (0x13 - (nbits >> 1));
                }
                if (work >= r) {
                    continue;
                }
                dy1 = SCR->cpos[1] - SCR->sph[idx][4];
                if (dy1 < -r || r < dy1) {
                    continue;
                }
                SCR->d1[1] = dy1;
                dx1 = SCR->cpos[0] - SCR->sph[idx][3];
                if (dx1 < -r || r < dx1) {
                    continue;
                }
                SCR->d1[0] = dx1;
                dz1 = SCR->cpos[2] - SCR->sph[idx][5];
                if (dz1 < -r || r < dz1) {
                    continue;
                }
                SCR->d1[2] = dz1;
                @gte_ldlvl(SCR->d1);
                @gte_sqr0();
                @gte_stlvnl(SCR->sq);
                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                if (sq2 < 0x400) {
                    dist2 = (&D_8008D118)[sq2] >> 3;
                } else {
                    s32 nbits2;

                    @gte_Lzc(sq2, &lz[1]);
                    nbits2 = lz[1];
                    nbits2 = 0x16 - (nbits2 & ~1);
                    temp = (&D_8008D118)[sq2 >> nbits2];
                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = work + dist2;
                if (tot >= r) {
                    continue;
                }
                @gte_lddp(1);
                @gte_ldlvl(SCR->vel);
                @gte_gpf0();
                pen = (r - tot) << 17;
                if (pen > 0x400000) {
                    pen = 0x400000;
                }
                if (work != 0) {
                    work = pen / work;
                }
                @gte_ldlvl(SCR->d0);
                @gte_lddp(work);
                @gte_gpl12();
                if (dist2 != 0) {
                    dist2 = pen / dist2;
                }
                @gte_ldlvl(SCR->d1);
                @gte_lddp(dist2);
                @gte_gpl12();
                @gte_stlvl(SCR->vel);
            }
        }
        node[3] = (SCR->vel[0] * 7) >> 3;
        node[0] = SCR->pos[0] + SCR->dpos[0] + node[3];
        node[4] = ((SCR->vel[1] * 7) >> 3) + 0x190;
        node[1] = SCR->pos[1] + SCR->dpos[1] + node[4];
        node[5] = (SCR->vel[2] * 7) >> 3;
        node[2] = SCR->pos[2] + SCR->dpos[2] + node[5];
    }
}
