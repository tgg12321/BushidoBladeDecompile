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
    s32 force[1][3]; /* 0xBC */
} RopeScratch;
#define SCR ((RopeScratch *)0x1F800000)
void func_800187F4(s32 arg0, s32 *arg1) {

    s32 *node;
    s32 *f;
    s32 i, j, n;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 shift, shift2;
    s32 byte;
    s32 t;
    s32 dist1, tot, pen;
    s32 lz[6];

    func_80018094((s32 *)arg0, arg1);
    count = ((s16 *)arg1)[2];
    node = (s32 *)arg1[3];

    for (i = 0; i < count; i++, node += 16) {
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
        n = node[7];
        bits = node[9];
        for (j = 0; j < n; j++) {
            f = SCR->force[bits & 0xFF];
            vx += f[0];
            vy += f[1];
            vz += f[2];
            if (j == 3) {
                bits = node[10];
            } else {
                bits >>= 8;
            }
        }
        n = node[8];
        bits2 = node[11];
        for (j = 0; j < n; j++) {
            f = SCR->force[bits2 & 0xFF];
            vx -= f[0];
            vy -= f[1];
            vz -= f[2];
            if (j == 3) {
                bits2 = node[12];
            } else {
                bits2 >>= 8;
            }
        }
        SCR->vel[0] = vx;
        SCR->vel[1] = vy;
        SCR->vel[2] = vz;
        if (*(s32 *)(arg0 + 0xC) != 0) {
            s32 dg;

            dg = SCR->pos[1] - SCR->ground;
            if (dg > 0) {
                if (dg > 0x3200) {
                    t = vy - 0x400;
                } else {
                    t = vy - dg / 8;
                }
                SCR->vel[1] = t;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (j = 0; j < SCR->nsph; j++) {
                s32 sq2, dist2;

                s32 dy0;
                s32 dx0;
                s32 dz0;
                s32 dy1;
                s32 dx1;
                s32 dz1;

                r = SCR->rad[j];
                dy0 = SCR->cpos[1] - SCR->sph[j][1];
                if (dy0 < -r || r < dy0) {
                    continue;
                }
                SCR->d0[1] = dy0;
                dx0 = SCR->cpos[0] - SCR->sph[j][0];
                if (dx0 < -r || r < dx0) {
                    continue;
                }
                SCR->d0[0] = dx0;
                dz0 = SCR->cpos[2] - SCR->sph[j][2];
                if (dz0 < -r || r < dz0) {
                    continue;
                }
                SCR->d0[2] = dz0;
                @gte_ldlvl(SCR->d0);
                @gte_sqr0();
                @gte_stlvnl(SCR->sq);
                dist1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                byte = dist1;
                if (dist1 < 0x400) {
                    dist1 = (&D_8008D118)[dist1] >> 3;
                } else {
                    @gte_Lzc(byte, &lz[0]);
                    shift = lz[0];
                    shift = 0x16 - (shift & ~1);
                    byte = (&D_8008D118)[dist1 >> shift];
                    dist1 = (byte << 16) >> (0x13 - (shift >> 1));
                }
                if (dist1 >= r) {
                    continue;
                }
                dy1 = SCR->cpos[1] - SCR->sph[j][4];
                if (dy1 < -r || r < dy1) {
                    continue;
                }
                SCR->d1[1] = dy1;
                dx1 = SCR->cpos[0] - SCR->sph[j][3];
                if (dx1 < -r || r < dx1) {
                    continue;
                }
                SCR->d1[0] = dx1;
                dz1 = SCR->cpos[2] - SCR->sph[j][5];
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
                    @gte_Lzc(sq2, &lz[1]);
                    shift2 = lz[1];
                    shift2 = 0x16 - (shift2 & ~1);
                    byte = (&D_8008D118)[sq2 >> shift2];
                    dist2 = (byte << 16) >> (0x13 - (shift2 >> 1));
                }
                tot = dist1 + dist2;
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
                if (dist1 != 0) {
                    dist1 = pen / dist1;
                }
                @gte_ldlvl(SCR->d0);
                @gte_lddp(dist1);
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
