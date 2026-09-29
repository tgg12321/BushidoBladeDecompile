/* func_800187F4 -- COMPLETED-INLINE-ASM-CANONICAL (manual lane, 2026-09-28).
 * Node-chain integrator. func_8001924C calls it for each 16-byte record (arg0;
 * +0xC enables collision) whose flag bit 0 is clear, with the record's descriptor
 * (arg1: +0 table of 8-byte anchor vectors, +4 s16 node count, +0xC the 64-byte
 * nodes). func_80018094 first sets up the GTE rotation/translation. Per node
 * (words 0-2 position, 3-5 velocity, 6 state, 7/8 force counts, 9-12 packed
 * force-table indices): state >= 0 springs the node toward its GTE-transformed
 * anchor (or snaps to it at 0) and ends there; state -0xFF..-1 first pulls the
 * position toward the anchor and then integrates like state < -0xFF: the indexed
 * scratchpad forces are added / subtracted, the node is pushed out of the ground and out of each
 * collision ellipsoid (inside when its distances to the two foci sum below the
 * bound; lengths via the D_8008D118 byte-LUT integer sqrt, with the GTE
 * leading-zero count above 0x400; the push applied on the GTE with GPF/GPL), and
 * the velocity is damped by 7/8 with 0x190 added to Y.
 * `sandbox func_800187F4 --disable all` = 0 (644/644).
 *
 * GTE ISLANDS: each island is one PsyQ Run-time Library 4.3 inline_o.h macro
 * (or gtemac.h gte_Lzc), written statement for statement as the header writes
 * it: the copy pinned in engine/gtemacro.py PINNED (silent-hill-decomp@a1f407cb
 * include/psyq/inline_o.h, sha256 76f28032...; gtemac.h 9fe028fd...), class route
 * inline-asm-policy.md § Owner ruling 2026-09-26. The seven gte_rtv0tr / gte_sqr0 /
 * gte_gpf0 / gte_gpl12 units carry the post-DMPSX command word in place of the
 * header's DMPSX placeholder, under § Per-function grant: func_800187F4 (owner
 * ruling 2026-09-28, Q29): 0x0000027f -> 0x4A480012, 0x00000f3f -> 0x4AA00428,
 * 0x000012ff -> 0x4B90003D, 0x0000133f -> 0x4BA8003E. */
typedef struct {
    s32 d0[3];      /* 0x00 delta to focus 0 (GTE input) */
    s32 d1[3];      /* 0x0C delta to focus 1 (GTE input) */
    s32 vel[3];     /* 0x18 velocity / GTE vector */
    s32 dpos[3];    /* 0x24 displacement added to every node */
    s32 sq[3];      /* 0x30 GTE squares */
    s32 pos[3];     /* 0x3C node position */
    s32 cpos[3];    /* 0x48 node position >> 5 */
    s32 unk54[3];   /* 0x54 */
    s32 nsph;       /* 0x60 ellipsoid count */
    s32 sph[3][6];  /* 0x64 ellipsoid foci 0 and 1 */
    s32 rad[3];     /* 0xAC ellipsoid bounds (sum of the two focus distances) */
    s32 ground;     /* 0xB8 ground height */
    s32 force[0][3]; /* 0xBC force table, indexed by the node's packed bytes */
} Scr1F800000;
#define SCR ((Scr1F800000 *)0x1F800000)
void func_800187F4(s16 *arg0, s32 *arg1) {
    s32 *node;
    s32 i;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 vy_new;
    s32 tot, pen;
    /* FAKE: n.b.! must be 17-24 bytes (s32 [5] and [6] are byte-identical).
     * OVERSIZED-LOCALS carve-out (.claude/rules/dead-vars-local-array.md, owner
     * ruling 2026-07-13), prong 2 (extend the LIVE locals object): lz[0] and lz[1]
     * are the two GTE leading-zero-count outputs, written by gte_stlzc and read
     * back. Frame from asm/funcs/func_800187F4.s alone: frame 0x78 = outgoing args
     * 0x10 + locals 0x40 + ten saves $s0-$s7/$fp/$ra at 0x50-0x74; the only locals
     * traffic is lz[0]/lz[1] at sp+0x10/0x14 and the count spill at sp+0x48.
     * Of the 0x40, 8 are the spill slot and 32 (0x28-0x47) are the four 8-byte
     * phantom slots of the combine orphan-USE loop-guard pseudos (the frame of the
     * lz[2] form: 0x68); the 16 bytes left are this object's.
     * Measured: lz[2]..lz[4] give frame 0x68, lz[5]/lz[6] 0x78, lz[7]/lz[8] 0x80.
     * lever-exhaustion: memory/grind/func_800187F4/evidence.md [s2] item 7 and
     * r11/proof.md section 7 (the phantom-slot producer census). */
    s32 lz[6];

    func_80018094((s32 *)arg0, arg1);
    count = *(s16 *)((u8 *)arg1 + 4);
    node = (s32 *)arg1[3];

    for (i = 0; i < count; i++, node += 16) {
        /* Ruling 11 (reused local, proof memory/grind/func_800187F4/r11/proof.md):
         * three values, all loop indices -- the add-force loop's, the
         * subtract-force loop's and the ellipsoid loop's. */
        s32 idx;
        /* Ruling 11 (proof r11/proof.md): two values, both force counts -- node
         * word 7 (forces added) and node word 8 (forces subtracted). */
        s32 nforce;

        SCR->pos[0] = node[0];
        SCR->pos[1] = node[1];
        SCR->pos[2] = node[2];
        if (node[6] >= 0) {
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            @gte_ldv0(arg1[0] + i * 8);
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455; post-DMPSX word 0x4A480012
             * for the header placeholder 0x0000027f (owner Q29) */
            @gte_rtv0tr();
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
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
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            @gte_ldv0(arg1[0] + i * 8);
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455; post-DMPSX word 0x4A480012
             * for the header placeholder 0x0000027f (owner Q29) */
            @gte_rtv0tr();
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
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
            s32 depth;

            depth = SCR->pos[1] - SCR->ground;
            if (depth > 0) {
                if (depth > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - depth / 8;
                }
                SCR->vel[1] = vy_new;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (idx = 0; idx < SCR->nsph; idx++) {
                s32 dy0, dx0, dz0, dy1, dx1, dz1;
                s32 sq2, dist2;
                /* Ruling 11 (proof r11/proof.md): three values -- a copy of the
                 * squared distance for the leading-zero-count macro (a value under
                 * (C)(3)'s GTE-macro input copy clause, owner ruling 2026-09-28
                 * Q28), then the focus-0 table byte, then the focus-1 table byte. */
                s32 temp;
                s32 sq1, dist1;

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
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                @gte_ldlvl(SCR->d0);
                /* gte_sqr0() -- inline_o.h 4.3 :646-650; post-DMPSX word 0x4AA00428
                 * for the header placeholder 0x00000f3f (owner Q29) */
                @gte_sqr0();
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                @gte_stlvnl(SCR->sq);
                sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = sq1;
                if (sq1 < 0x400) {
                    dist1 = (&D_8008D118)[sq1] >> 3;
                } else {
                    /* Ruling 11 (proof r11/proof.md): two values, both bit counts --
                     * the leading-zero count, then the table shift. */
                    s32 nbits;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc :207-210,
                     * gte_nop :1095-1097 twice, gte_stlzc :1074-1077 */
                    @gte_Lzc(temp, &lz[0]);
                    nbits = lz[0];
                    nbits = 0x16 - (nbits & ~1);
                    temp = (&D_8008D118)[sq1 >> nbits];
                    dist1 = (temp << 16) >> (0x13 - (nbits >> 1));
                }
                if (dist1 >= r) {
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
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                @gte_ldlvl(SCR->d1);
                /* gte_sqr0() -- inline_o.h 4.3 :646-650; post-DMPSX word 0x4AA00428
                 * for the header placeholder 0x00000f3f (owner Q29) */
                @gte_sqr0();
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                @gte_stlvnl(SCR->sq);
                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                if (sq2 < 0x400) {
                    dist2 = (&D_8008D118)[sq2] >> 3;
                } else {
                    /* Ruling 11 (proof r11/proof.md): two values, both bit counts --
                     * the leading-zero count, then the table shift. */
                    s32 nbits2;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc :207-210,
                     * gte_nop :1095-1097 twice, gte_stlzc :1074-1077 */
                    @gte_Lzc(sq2, &lz[1]);
                    nbits2 = lz[1];
                    nbits2 = 0x16 - (nbits2 & ~1);
                    temp = (&D_8008D118)[sq2 >> nbits2];
                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = dist1 + dist2;
                if (tot >= r) {
                    continue;
                }
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                @gte_lddp(1);
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                @gte_ldlvl(SCR->vel);
                /* gte_gpf0() -- inline_o.h 4.3 :721-725; post-DMPSX word 0x4B90003D
                 * for the header placeholder 0x000012ff (owner Q29) */
                @gte_gpf0();
                pen = (r - tot) << 17;
                if (pen > 0x400000) {
                    pen = 0x400000;
                }
                if (dist1 != 0) {
                    dist1 = pen / dist1;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                @gte_ldlvl(SCR->d0);
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                @gte_lddp(dist1);
                /* gte_gpl12() -- inline_o.h 4.3 :726-730; post-DMPSX word 0x4BA8003E
                 * for the header placeholder 0x0000133f (owner Q29) */
                @gte_gpl12();
                if (dist2 != 0) {
                    dist2 = pen / dist2;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                @gte_ldlvl(SCR->d1);
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                @gte_lddp(dist2);
                /* gte_gpl12() -- inline_o.h 4.3 :726-730; post-DMPSX word 0x4BA8003E
                 * for the header placeholder 0x0000133f (owner Q29) */
                @gte_gpl12();
                /* gte_stlvl(r1) -- inline_o.h 4.3 :898-903 */
                @gte_stlvl(SCR->vel);
            }
            /* FAKE: dead store; mechanism: reg_scan records it as depth's last
             * reference (regclass.c:1764 counts sets), so in cse the `depth / 8`
             * expansion's copy does not outlive depth and cse.c make_regs_eqv
             * (:840-857) keeps depth as the class head: the sign test reads depth
             * (target `bgez $v1`, copy in the delay slot) instead of the copy
             * (`bgez $v0`, 4 insns). flow deletes the store (no bytes). Lever
             * exhaustion: memory/grind/func_800187F4/r11/proof.md (depth). */
            depth = 0;
        }
        node[3] = (SCR->vel[0] * 7) >> 3;
        node[0] = SCR->pos[0] + SCR->dpos[0] + node[3];
        node[4] = ((SCR->vel[1] * 7) >> 3) + 0x190;
        node[1] = SCR->pos[1] + SCR->dpos[1] + node[4];
        node[5] = (SCR->vel[2] * 7) >> 3;
        node[2] = SCR->pos[2] + SCR->dpos[2] + node[5];
    }
}
