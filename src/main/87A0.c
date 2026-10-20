/* 12 game functions, among them pad_ResetState. .text 0x80017FA0 (ROM 0x87A0).
 * Start boundary: LEGACY (the splat 6CAC segment edge). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include <psxsdk/inline_o.h>
#include <psxsdk/gtemac.h>

extern s32 memcpy(s32 *, s32, s32);
extern s32 collision_GetHitTag(void);

extern void softbody_SolveSprings(Func80017A44Output *);

typedef struct {
    s32 d0[3];       /* 0x00 delta to focus 0 (GTE input) */
    s32 d1[3];       /* 0x0C delta to focus 1 (GTE input) */
    s32 scr18[3];    /* 0x18 transformed anchor, then the velocity */
    s32 dpos[3];     /* 0x24 displacement added to every node */
    s32 sq[3];       /* 0x30 GTE squares */
    s32 pos[3];      /* 0x3C node position */
    s32 cpos[3];     /* 0x48 node position >> 5 */
    s32 unk54[3];    /* 0x54 */
    s32 scr60;       /* 0x60 ellipsoid count */
    s32 scr64[3][6]; /* 0x64 ellipsoid foci 0 and 1 */
    /* 0xAC ellipsoid bounds (sum of the two focus distances) */
    s32 scrAC[3];
    s32 ground;      /* 0xB8 ground height */
    s32 force[0][3]; /* 0xBC force table, indexed by the node's packed bytes */
} Scr1F800000;

#define SCR ((Scr1F800000 *)0x1F800000)

/* softbody_LoadColliders - copies the Unk80017FA0Rec's collision volumes
 * (a0->unkC) into scratchpad RAM (0x1F800000). unk00 is written scaled by 128
 * (the ground word); count is the volume count, and each volume writes its two
 * foci (three words each, scaled by 4) at a 0x18 stride plus its bound from
 * unk68. Nothing happens when a0->unkC is null. The outer guard tests the live
 * counter
 * (`i < ptr->count`) for the target's empty 8-byte leaf frame
 * (phantom-slot-frame-lever). */
void softbody_LoadColliders(Unk80017FA0Rec *a0) {
    /* FAKE: temp takes the block pointer for the null test and ptr a copy after
     * it (lw v0 / move t1,v0); tested as ptr itself the load goes straight to
     * t1 and the move drops: score 3 */
    Unk8003F6D8Coll *temp;
    Unk8003F6D8Coll *ptr;

    temp = a0->unkC;
    if (temp == 0) {
        goto end;
    }
    ptr = temp;

    SCR->ground = ptr->unk00 << 7;

    {
        s32 i = 0;
        if (i < ptr->count) {
            s32 *p68 = (s32 *)ptr;
            s32 *var_4 = (s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 var_8 = sp_off;
            /* FAKE: goto loop instead of do-while keeps loop.c from
             * strength-reducing the three scratchpad store addresses into one
             * biased base; as do-while: score 12 */
            inner: {
                s32 *var_9 = (s32 *)((u8 *)ptr + data_off);
                *(s32 *)(0x1F800064 + var_8) = var_9[2] << 2;
                data_off += 0x10;
                *(s32 *)(0x1F800068 + var_8) = var_9[3] << 2;
                j++;
                *(s32 *)(0x1F80006C + var_8) = var_9[4] << 2;
                var_8 += 0xC;
            }
                if (j < 2) {
                    goto inner;
                }
                var_4[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                var_4++;
            } while (i < ptr->count);
        }
    }

    SCR->scr60 = a0->unkC->count;
end:;
}

/* softbody_PrepareStep -- loads the GTE rotation/translation from arg0's MATRIX
 * (arg0->unk4), runs softbody_LoadColliders, writes that MATRIX's translation
 * minus the object's (arg1->matrix.t) to scratchpad, scales it by a factor
 * derived from its length (byte-LUT integer sqrt, GTE LZC above 0x400; 0x100
 * when the squared length exceeds 250000), copies the MATRIX to arg1->matrix
 * and runs softbody_SolveSprings. */
typedef struct {
    s32 pad[9];
    s32 x, y, z;
} ScrV;

#define SCRV ((ScrV *)0x1F800000)

void softbody_PrepareStep(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    /* FAKE: frame layout -- sp_tmp[0] is the live LZC output; the unwritten
     * tail sizes the 16-byte locals region of the 0x30 frame (as s32
     * sp_tmp[1]: score 8) (dead-vars-local-array, oversized-locals) */
    s32 sp_tmp[4];
    s32 dx, dy, dz;
    s32 sum_sq;
    s32 scale;
    u32 lut;

    gte_SetRotMatrix(arg0->unk4);
    gte_SetTransMatrix(arg0->unk4);

    softbody_LoadColliders(arg0);

    dx = arg0->unk4->t[0] - arg1->matrix.t[0];
    SCRV->x = dx;
    dy = arg0->unk4->t[1] - arg1->matrix.t[1];
    SCRV->y = dy;
    dz = arg0->unk4->t[2] - arg1->matrix.t[2];
    SCRV->z = dz;

    sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

    if (sum_sq > 250000) {
        scale = 0x100;
    } else if (sum_sq < 0) {
        scale = 0;
    } else {
        /* FAKE: the LZC input staged through the still-dead `lut`, giving the
         * target's `move $a0,$a1` in the beqz delay slot; input sum_sq, no
         * copy: score 10 (staged-value-reused-variable) */
        lut = sum_sq;
        if (sum_sq < 0x400) {
            sum_sq = (u8)(g_sqrt_table_u8[sum_sq]) >> 3;
        } else {
            s32 shift_a, shift_b;

            gte_Lzc(lut, &sp_tmp[0]);
            {
                /* FAKE: the -2 mask held in a local that then takes the masked
                 * count (li -2 / and v0,v1,v0); as `0x16 - (sp_tmp[0] & -2)`
                 * the load and the AND move to $a0: score 3 */
                s32 tmp = -2;
                tmp = sp_tmp[0] & tmp;
                shift_a = 0x16 - tmp;
            }
            shift_b = shift_a >> 1;
            lut = (u8)(g_sqrt_table_u8[sum_sq >> shift_a]);
            sum_sq = ((s32)(lut << 16)) >> (0x13 - shift_b);
        }
        scale = ((sum_sq << 6) / 500) + 0xC0;
    }
    SCRV->x = (SCRV->x * scale) >> 1;
    SCRV->y = (SCRV->y * scale) >> 1;
    SCRV->z = (SCRV->z * scale) >> 1;

    arg1->matrix = *arg0->unk4;
    softbody_SolveSprings(arg1);
}

/* softbody_SolveSprings -- the spring-force pass over the body's edges (the
 * springs between the nodes of each mesh face group, Func80017848Edge; 64-byte
 * nodes): each edge's node pair (packed indices, rest length dist) is bisected
 * until every axis delta is within 3x the rest length, then the delta >> 3 is
 * scaled on the GTE by ((len - rest) << 14) / len into the 0x1F8000BC output
 * array (len via the byte-LUT integer sqrt, GTE LZC above 0x400). The last
 * edge is emitted after the loop without the bisection. */
void softbody_SolveSprings(Func80017A44Output *arg0) {
    s32 *out;
    Func80017848Edge *data;
    Func80017A44Record *base;
    s32 count;
    s32 thresh;
    s32 var_5;
    s32 nthresh;
    Func80017A44Record *p1, *p2;
    s32 dx, dy, dz;
    u32 sum;
    u32 len;
    /* FAKE: frame layout -- lz[0] is the live LZC output (gte_stlzc); the
     * unwritten tail sizes the 24-byte locals region of the 0x28 frame (as s32
     * lz[1]: score 8) (dead-vars-local-array, oversized-locals) */
    s32 lz[6];
    s32 f;

    out = (s32 *)0x1F8000BC;
    count = arg0->edge_count - 1;
    data = arg0->edges;
    base = arg0->records;
    /* FAKE: the packed node-index word staged through the still-dead
     * `thresh` so it shares thresh's register ($t1); its own local takes $a1:
     * score 5 (staged-value-reused-variable) */
    thresh = data->ends.pair;
    var_5 = data->dist;
    p1 = &base[thresh >> 16];
    p2 = &base[thresh & 0xFFFF];
    thresh = var_5 * 3;
    nthresh = -thresh;
    dx = p2->pos[0] - p1->pos[0];
    dy = p2->pos[1] - p1->pos[1];
    dz = p2->pos[2] - p1->pos[2];
    while (count > 0) {
        while (dx < nthresh || thresh < dx || dy < nthresh || thresh < dy ||
               dz < nthresh || thresh < dz) {
            dx = (p2->pos[0] + p1->pos[0]) / 2;
            dy = (p2->pos[1] + p1->pos[1]) / 2;
            dz = (p2->pos[2] + p1->pos[2]) / 2;
            if (p2->field_18 < 0) {
                p2->pos[0] = dx;
                p2->pos[1] = dy;
                p2->pos[2] = dz;
                p2->field_C /= 4;
                p2->field_10 /= 4;
                p2->field_14 /= 4;
            } else {
                p1->pos[0] = dx;
                p1->pos[1] = dy;
                p1->pos[2] = dz;
                p1->field_C /= 4;
                p1->field_10 /= 4;
                p1->field_14 /= 4;
            }
            dx = p2->pos[0] - p1->pos[0];
            dy = p2->pos[1] - p1->pos[1];
            dz = p2->pos[2] - p1->pos[2];
        }
        *(s32 *)0x1F800000 = dx >> 3;
        *(s32 *)0x1F800004 = dy >> 3;
        *(s32 *)0x1F800008 = dz >> 3;
        gte_ldlvl((s32 *)0x1F800000);
        gte_sqr0();
        data++;
        count--;
        thresh = data->ends.pair;
        gte_stlvnl((s32 *)0x1F80000C);
        sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
        if (sum < 0x400) {
            len = g_sqrt_table_u8[sum];
            p1 = &base[thresh >> 16];
            p2 = &base[thresh & 0xFFFF];
        } else {
            gte_ldlzc(sum);
            /* the next link's node pointers fill the LZC result delay */
            p1 = &base[thresh >> 16];
            p2 = &base[thresh & 0xFFFF];
            gte_stlzc(lz);
            /* FAKE: `len` carries the LZC count and the table shift before the
             * root, and `sum` takes the table byte, keeping them in $v1 / $a0
             * as the target does; fresh shift / byte locals: score 22
             * (staged-value-reused-variable) */
            len = lz[0];
            len = 0x16 - (len & ~1);
            sum = g_sqrt_table_u8[sum >> len];
            len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
        }
        gte_ldlvl((s32 *)0x1F800000);
        f = (s32)((len - var_5) << 14) / (s32)len;
        var_5 = data->dist;
        thresh = var_5 * 3;
        nthresh = -thresh;
        gte_lddp(f);
        gte_gpf12();
        dx = p2->pos[0] - p1->pos[0];
        dy = p2->pos[1] - p1->pos[1];
        dz = p2->pos[2] - p1->pos[2];
        gte_stlvl(out);
        out += 3;
    }
    *(s32 *)0x1F800000 = dx >> 3;
    *(s32 *)0x1F800004 = dy >> 3;
    *(s32 *)0x1F800008 = dz >> 3;
    gte_Square0((s32 *)0x1F800000, (s32 *)0x1F80000C);
    sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
    if (sum < 0x400) {
        len = g_sqrt_table_u8[sum];
    } else {
        gte_Lzc(sum, lz);
        /* FAKE: same staged len/sum reuse as the loop's LZC arm above (fresh
         * shift / byte locals: score 3). */
        len = lz[0];
        len = 0x16 - (len & ~1);
        sum = g_sqrt_table_u8[sum >> len];
        len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
    }
    gte_ldlvl((s32 *)0x1F800000);
    f = (s32)((len - var_5) << 14) / (s32)len;
    gte_lddp(f);
    gte_gpf12();
    gte_stlvl(out);
}

void softbody_Step(Unk80017FA0Rec *arg0, Func80017A44Output *arg1);
void softbody_SnapToPoints(Unk80017FA0Rec *arg0, Func80017A44Output *arg1);

/* softbody_Step -- one integration step of the body's nodes, called by
 * softbody_UpdateList for each Unk80017FA0Rec whose unk2 bit 0 is clear. A node
 * with state >= 0 springs toward (or snaps to) its GTE-transformed anchor;
 * otherwise the indexed scratchpad forces are applied, the node is pushed out
 * of the ground and out of each collision ellipsoid, and the velocity is damped
 * by 7/8 with 0x190 added to Y. */
void softbody_Step(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    Func80017A44Record *node;
    s32 i;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 vy_new;
    s32 tot, var_11;
    /* FAKE: frame layout -- lz[0] (first gte_stlzc) and lz[1] (second
     * gte_stlzc) are the live LZC outputs; the unwritten 16-byte tail gives the
     * 0x78 frame (as s32 lz[2]: frame 0x68, score 24) (dead-vars-local-array,
     * oversized-locals) */
    s32 lz[6];

    softbody_PrepareStep(arg0, arg1);
    count = arg1->count;
    node = arg1->records;

    for (i = 0; i < count; i++, node++) {
        /* Ruling 11 (reused local): loop index of the add-force,
         * subtract-force and ellipsoid loops */
        s32 idx;
        /* Ruling 11 (reused local): force count, field_1C (added) then
         * field_20 (subtracted) */
        s32 var_14;

        SCR->pos[0] = node->pos[0];
        SCR->pos[1] = node->pos[1];
        SCR->pos[2] = node->pos[2];
        if (node->field_18 >= 0) {
            gte_ldv0(&arg1->points[i]);
            gte_rtv0tr();
            gte_stlvnl(SCR->scr18);
            if (node->field_18 > 0) {
                SCR->pos[0] += SCR->dpos[0];
                node->field_C =
                    node->field_C / 2 +
                    ((((SCR->scr18[0] << 7) - SCR->pos[0]) * node->field_18) >>
                     8);
                node->pos[0] = SCR->pos[0] + node->field_C;
                SCR->pos[1] += SCR->dpos[1];
                node->field_10 =
                    node->field_10 / 2 +
                    ((((SCR->scr18[1] << 7) - SCR->pos[1]) * node->field_18 +
                      (0x100 - node->field_18) * 25) >>
                     8);
                node->pos[1] = SCR->pos[1] + node->field_10;
                SCR->pos[2] += SCR->dpos[2];
                node->field_14 =
                    node->field_14 / 2 +
                    ((((SCR->scr18[2] << 7) - SCR->pos[2]) * node->field_18) >>
                     8);
                node->pos[2] = SCR->pos[2] + node->field_14;
            } else {
                node->pos[0] = SCR->scr18[0] << 7;
                node->pos[1] = SCR->scr18[1] << 7;
                node->pos[2] = SCR->scr18[2] << 7;
            }
            continue;
        }
        if (node->field_18 >= -0xFF) {
            gte_ldv0(&arg1->points[i]);
            gte_rtv0tr();
            gte_stlvnl(SCR->scr18);
            SCR->pos[0] -=
                (((SCR->scr18[0] << 7) - SCR->pos[0]) * node->field_18) >> 8;
            SCR->pos[1] -=
                (((SCR->scr18[1] << 7) - SCR->pos[1]) * node->field_18) >> 8;
            SCR->pos[2] -=
                (((SCR->scr18[2] << 7) - SCR->pos[2]) * node->field_18) >> 8;
        }
        vx = node->field_C;
        vy = node->field_10;
        vz = node->field_14;
        var_14 = node->field_1C;
        bits = *(s32 *)&node->field_24[0];
        for (idx = 0; idx < var_14; idx++) {
            s32 *f_add;

            f_add = SCR->force[bits & 0xFF];
            vx += f_add[0];
            vy += f_add[1];
            vz += f_add[2];
            if (idx == 3) {
                bits = *(s32 *)&node->field_24[4];
            } else {
                bits >>= 8;
            }
        }
        var_14 = node->field_20;
        bits2 = *(s32 *)&node->field_2C[0];
        for (idx = 0; idx < var_14; idx++) {
            s32 *f_sub;

            f_sub = SCR->force[bits2 & 0xFF];
            vx -= f_sub[0];
            vy -= f_sub[1];
            vz -= f_sub[2];
            if (idx == 3) {
                bits2 = *(s32 *)&node->field_2C[4];
            } else {
                bits2 >>= 8;
            }
        }
        SCR->scr18[0] = vx;
        SCR->scr18[1] = vy;
        SCR->scr18[2] = vz;
        if (arg0->unkC != 0) {
            /* Ruling 11 (reused local): Y delta, depth below the ground then
             * the delta to focus 0 */
            s32 delta;

            delta = SCR->pos[1] - SCR->ground;
            if (delta > 0) {
                if (delta > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - delta / 8;
                }
                SCR->scr18[1] = vy_new;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (idx = 0; idx < SCR->scr60; idx++) {
                s32 dx0, dz0, dy1, dx1, dz1;
                s32 sq2, dist2;
                /* Ruling 11 (reused local): the squared length copied for
                 * gte_Lzc (Q28), then the focus-0 and focus-1 table bytes */
                s32 temp;
                /* Ruling 11 (reused local): focus-0 squared distance, then the
                 * distance (scaled to its push factor below) */
                s32 work;

                r = SCR->scrAC[idx];
                delta = SCR->cpos[1] - SCR->scr64[idx][1];
                if (delta < -r || r < delta) {
                    continue;
                }
                SCR->d0[1] = delta;
                dx0 = SCR->cpos[0] - SCR->scr64[idx][0];
                if (dx0 < -r || r < dx0) {
                    continue;
                }
                SCR->d0[0] = dx0;
                dz0 = SCR->cpos[2] - SCR->scr64[idx][2];
                if (dz0 < -r || r < dz0) {
                    continue;
                }
                SCR->d0[2] = dz0;
                gte_Square0(SCR->d0, SCR->sq);
                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = work;
                if (work < 0x400) {
                    work = g_sqrt_table_u8[work] >> 3;
                } else {
                    /* Ruling 11 (reused local): leading-zero count, then the
                     * table shift */
                    s32 nbits;

                    gte_Lzc(temp, &lz[0]);
                    nbits = lz[0];
                    nbits = 0x16 - (nbits & ~1);
                    temp = g_sqrt_table_u8[work >> nbits];
                    work = (temp << 16) >> (0x13 - (nbits >> 1));
                }
                if (work >= r) {
                    continue;
                }
                dy1 = SCR->cpos[1] - SCR->scr64[idx][4];
                if (dy1 < -r || r < dy1) {
                    continue;
                }
                SCR->d1[1] = dy1;
                dx1 = SCR->cpos[0] - SCR->scr64[idx][3];
                if (dx1 < -r || r < dx1) {
                    continue;
                }
                SCR->d1[0] = dx1;
                dz1 = SCR->cpos[2] - SCR->scr64[idx][5];
                if (dz1 < -r || r < dz1) {
                    continue;
                }
                SCR->d1[2] = dz1;
                gte_Square0(SCR->d1, SCR->sq);
                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                if (sq2 < 0x400) {
                    dist2 = g_sqrt_table_u8[sq2] >> 3;
                } else {
                    /* Ruling 11 (reused local): leading-zero count, then the
                     * table shift */
                    s32 nbits2;

                    gte_Lzc(sq2, &lz[1]);
                    nbits2 = lz[1];
                    nbits2 = 0x16 - (nbits2 & ~1);
                    temp = g_sqrt_table_u8[sq2 >> nbits2];
                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = work + dist2;
                if (tot >= r) {
                    continue;
                }
                gte_lddp(1);
                gte_ldlvl(SCR->scr18);
                gte_gpf0();
                var_11 = (r - tot) << 17;
                if (var_11 > 0x400000) {
                    var_11 = 0x400000;
                }
                if (work != 0) {
                    work = var_11 / work;
                }
                gte_ldlvl(SCR->d0);
                gte_lddp(work);
                gte_gpl12();
                if (dist2 != 0) {
                    dist2 = var_11 / dist2;
                }
                gte_ldlvl(SCR->d1);
                gte_lddp(dist2);
                gte_gpl12();
                gte_stlvl(SCR->scr18);
            }
        }
        node->field_C = (SCR->scr18[0] * 7) >> 3;
        node->pos[0] = SCR->pos[0] + SCR->dpos[0] + node->field_C;
        node->field_10 = ((SCR->scr18[1] * 7) >> 3) + 0x190;
        node->pos[1] = SCR->pos[1] + SCR->dpos[1] + node->field_10;
        node->field_14 = (SCR->scr18[2] * 7) >> 3;
        node->pos[2] = SCR->pos[2] + SCR->dpos[2] + node->field_14;
    }
}

void softbody_UpdateList(Unk80017FA0Rec *arg0, s32 arg1) {
    s32 i = 0;
    Unk80017FA0Rec *s0;
    /* FAKE: pointer alias -- g_softbody_slots's address held in an integer
     * local. Referenced directly, its lui/addiu is scheduled after `move s0,a0`
     * (score 2); held as a u8 * the addu operands swap (score 2). */
    s32 buf;

    if (i < arg1) {
        buf = (s32)g_softbody_slots;
        s0 = arg0;
        do {
            if (s0->unk2 & 1) {
                s16 val = s0->unk0;
                softbody_SnapToPoints(
                    s0, (Func80017A44Output *)(val * 52 + buf));
            } else {
                s16 val = s0->unk0;
                softbody_Step(s0, (Func80017A44Output *)(val * 52 + buf));
            }
            i++;
            s0++;
        } while (i < arg1);
    }
}

/* softbody_SnapToPoints - GTE rotate-translate of the body's SVECTOR points (by
 * arg0's MATRIX) into its 0x40-stride node table, scaled << 7, with each node's
 * velocity zeroed; then copies that MATRIX (32 bytes) into arg1->matrix. */
void softbody_SnapToPoints(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    s32 out[3];
    s32 i;
    Func80017A44Record *dst;

    gte_SetRotMatrix(arg0->unk4);
    gte_SetTransMatrix(arg0->unk4);

    dst = arg1->records;
    for (i = 0; i < arg1->count; i++) {
        gte_ldv0(&arg1->points[i]);
        gte_rtv0tr();
        gte_stlvnl(out);
        dst->pos[0] = out[0] << 7;
        dst->pos[1] = out[1] << 7;
        dst->pos[2] = out[2] << 7;
        dst->field_C = 0;
        dst->field_10 = 0;
        dst->field_14 = 0;
        dst++;
    }
    arg1->matrix = *arg0->unk4;
}

void func_8001945C(void) {
    D_80106A50.frec20[0] = 0x11;
    D_80106A50.frec20[1] = 0x44;
    D_80106A50.frec20[2] = 0x88;
}

s32 func_80019488(void) {
    return (D_80106A50.frec20[0] & 0xF) | ((D_80106A50.frec20[1] & 0xF) << 4) |
           ((D_80106A50.frec20[2] & 0xF) << 8);
}

void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}

void pad_ResetState(void) {
    g_pad_state.type[0] = 4;
    g_pad_state.type[1] = 4;
    g_pad_state.held = 0;
    g_pad_state.pressed = 0;
    g_pad_state.released = 0;
    g_pad_state.unheld = -1;
}

void pad_ResetStateMarkValid(void) {
    pad_ResetState();
    g_pad_state.valid[0] = 1;
    g_pad_state.valid[1] = 1;
}

void pad_Update(s32 arg0) {
    PadState pad;
    s32 var_1[4];
    s32 i;
    s32 var_3;
    s32 var_4;

    var_3 = 0;
    i = 0;
    var_1[0] = g_pad_buf[0][0];
    var_1[1] = g_pad_buf[0][1];
    var_1[2] = g_pad_buf[1][0];
    var_1[3] = g_pad_buf[1][1];
    do {
        /* FAKE: rec holds (u8 *)var_1 + i * 8 for the loop body; indexing
         * from var_1 at each read adds a second induction (score 18). */
        u8 *rec = (u8 *)var_1 + i * 8;
        s32 bits;

        if (rec[0] == 0) {
            s32 type_m1;

            pad.type[i] = rec[1] >> 4;
            pad.valid[i] = 1;
            type_m1 = (s16)((u16)pad.type[i] - 1);

            switch (type_m1) {
            case 4:
            case 6:
                pad.type[i] = 4;
            case 1:
            case 2:
            case 3:
                bits = ~((rec[2] << 8) | rec[3]);
                break;
            case 0:
            case 5:
            case 7:
            default:
                bits = 0;
                break;
            }
        } else {
            pad.type[i] = 4;
            pad.valid[i] = 0;
            bits = 0;
        }

        var_3 = ((u32)var_3 >> 16) | (bits << 16);
        i++;
    } while (i < 2);

    pad.held = var_3;
    camera_UpdatePadOffset(&pad.held);

    if (D_800A3834 == 1 && arg0 == 0) {

        switch (D_800A38DC) {
        case 4:
        case 5:
            if (g_pad_state.valid[1] == 0) {
                pad.held |= 0x08000800;
            }
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            if (g_pad_state.valid[0] == 0) {
                pad.held |= 0x08000800;
            }
            break;
        }
    }

    comb_ExchangePad(&pad);

    i = 0;
    do {
        g_pad_state.type[i] = pad.type[i];
        g_pad_state.valid[i] = pad.valid[i];
        i++;
    } while (i < 2);

    var_4 = g_pad_state.held;
    g_pad_state.held = pad.held;
    g_pad_state.pressed = pad.held & ~var_4;
    g_pad_state.unheld = ~pad.held;
    g_pad_state.released = ~pad.held & var_4;
}
