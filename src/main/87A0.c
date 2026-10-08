/* 12 game functions, among them pad_ResetState. .text 0x80017FA0 (ROM 0x87A0).
 * Start boundary: LEGACY (the splat 6CAC segment edge). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

extern s32 memcpy(s32 *, s32, s32);
extern s32 func_80054434(void);

extern void func_80018300(Func80017A44Output *);

typedef struct {
    s32 d0[3];     /* 0x00 delta to focus 0 (GTE input) */
    s32 d1[3];     /* 0x0C delta to focus 1 (GTE input) */
    s32 vel[3];    /* 0x18 velocity / GTE vector */
    s32 dpos[3];   /* 0x24 displacement added to every node */
    s32 sq[3];     /* 0x30 GTE squares */
    s32 pos[3];    /* 0x3C node position */
    s32 cpos[3];   /* 0x48 node position >> 5 */
    s32 unk54[3];  /* 0x54 */
    s32 nsph;      /* 0x60 ellipsoid count */
    s32 sph[3][6]; /* 0x64 ellipsoid foci 0 and 1 */
    s32 rad[3];    /* 0xAC ellipsoid bounds (sum of the two focus distances) */
    s32 ground;    /* 0xB8 ground height */
    s32 force[0][3]; /* 0xBC force table, indexed by the node's packed bytes */
} Scr1F800000;

#define SCR ((Scr1F800000 *)0x1F800000)

/* func_80017FA0 - copies the Unk80017FA0Rec's collision volumes (a0->unkC) into
 * scratchpad RAM (0x1F800000). unk00 is written scaled by 128 (the ground
 * word); count is the volume count, and each volume writes its two foci (three
 * words each, scaled by 4) at a 0x18 stride plus its bound from unk68. Nothing
 * happens when a0->unkC is null. The outer guard tests the live counter
 * (`i < ptr->count`) for the target's empty 8-byte leaf frame
 * (phantom-slot-frame-lever). */
void func_80017FA0(Unk80017FA0Rec *a0) {
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
            s32 *ac_base = (s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 sp_inner = sp_off;
            /* FAKE: goto loop instead of do-while keeps loop.c from
             * strength-reducing the three scratchpad store addresses into one
             * biased base; as do-while: score 12 */
            inner: {
                s32 *dp = (s32 *)((u8 *)ptr + data_off);
                *(s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                data_off += 0x10;
                *(s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                j++;
                *(s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                sp_inner += 0xC;
            }
                if (j < 2) {
                    goto inner;
                }
                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr->count);
        }
    }

    SCR->nsph = a0->unkC->count;
end:;
}

/* func_80018094 -- loads the GTE rotation/translation from the quad's MATRIX
 * (arg0->unk4), runs func_80017FA0, writes that MATRIX's translation minus the
 * object's (arg1->matrix.t) to scratchpad, scales it by a factor derived from
 * its length (byte-LUT integer sqrt, GTE LZC above 0x400; 0x100 beyond 250000),
 * copies the MATRIX to arg1->matrix and runs func_80018300.
 * GTE: gte_SetRotMatrix, gte_SetTransMatrix, gte_Lzc (owner cop2 cluster
 * grant, cop2-addressing-preamble-cluster). */
typedef struct {
    s32 pad[9];
    s32 x, y, z;
} ScrV;

#define SCRV ((ScrV *)0x1F800000)

void func_80018094(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    /* FAKE: frame layout -- sp_tmp[0] is the live LZC output; the unwritten
     * tail sizes the 16-byte locals region of the 0x30 frame (as s32
     * sp_tmp[1]: score 8) (dead-vars-local-array, oversized-locals) */
    s32 sp_tmp[4];
    s32 dx, dy, dz;
    s32 sum_sq;
    s32 scale;
    u32 lut;

    /* gte_SetRotMatrix(r0) -- inline_c.h:297-310, in func_800203B4's granted
     * spelling (`move $12, %0` preamble, +$15 clobber); "memory" clobber
     * added */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(arg0->unk4) : "$12", "$13", "$14", "$15", "memory");
    /* gte_SetTransMatrix(r0) -- inline_c.h:360-369, in func_800203B4's granted
     * spelling (`move $12, %0` preamble, +$15 clobber); "memory" clobber
     * added */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(arg0->unk4) : "$12", "$13", "$14", "$15", "memory");

    func_80017FA0(arg0);

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
        {
            /* FAKE: do-while(0) raises the loop depth that weights register
             * allocation priority; without it: score 13
             * (do-while-zero-exception) */
            do {
                /* FAKE: the LZC island input staged through the still-dead
                 * `lut`, giving the target's `move $a0,$a1` in the beqz delay
                 * slot; island input sum_sq, no copy: score 10
                 * (staged-value-reused-variable) */
                lut = sum_sq;
                if (sum_sq < 0x400) {
                    /* FAKE: do-while(0) + goto exit puts a loop-end note
                     * before the LZC arm, stopping CSE from extending into it
                     * so the `lut = sum_sq;` copy survives; without it: score
                     * 10 (do-while-zero-exception) */
                    do {
                        sum_sq = (u8)(g_sqrt_table_u8[sum_sq]) >> 3;
                        goto lzc_done;
                    } while (0);
                }
                {
                    s32 shift_a, shift_b;
                    /* FAKE: do-while(0) raises the loop depth of `lut`'s
                     * references so it outranks sum_sq for $a0; without it:
                     * score 13 (do-while-zero-exception) */
                    /* gte_Lzc(r1,r2) -- gtemac.h:174-178 (gte_ldlzc
                     * inline_c.h:228-231, two gte_nop inline_c.h:1346-1347,
                     * gte_stlzc inline_c.h:1318-1322), with the original's
                     * `addu $t4,...,$zero` / `addiu $v0,$sp,0x10` addressing
                     * preamble (cop2-addressing-preamble-cluster). */
                    do {
                        __asm__ volatile(
                            "addu   $t4, %1, $zero\n"
                            "mtc2   $t4, $30\n"
                            "nop\n"
                            "nop\n"
                            "addiu  $v0, $sp, 0x10\n"
                            "addu   $t4, $v0, $zero\n"
                            "swc2   $31, 0($t4)\n"
                            : "=m"(sp_tmp[0])
                            : "r"(lut)
                            : "$2", "$12");
                        {
                            /* FAKE: the -2 mask held in a local that then takes
                             * the masked count (li -2 / and v0,v1,v0); as `0x16
                             * - (sp_tmp[0] & -2)` the load and the AND move to
                             * $a0: score 4 */
                            s32 tmp = -2;
                            tmp = sp_tmp[0] & tmp;
                            shift_a = 0x16 - tmp;
                        }
                        shift_b = shift_a >> 1;
                        lut = (u8)(g_sqrt_table_u8[sum_sq >> shift_a]);
                        sum_sq = ((s32)(lut << 16)) >> (0x13 - shift_b);
                    } while (0);
                }
            lzc_done:
                scale = ((sum_sq << 6) / 500) + 0xC0;
            } while (0);
        }
    }
    SCRV->x = (SCRV->x * scale) >> 1;
    SCRV->y = (SCRV->y * scale) >> 1;
    SCRV->z = (SCRV->z * scale) >> 1;

    arg1->matrix = *arg0->unk4;
    func_80018300(arg1);
}

/* func_80018300 -- distance-constraint pass over a chain of 64-byte nodes:
 * each link's node pair (packed indices, rest length dist) is bisected until
 * every axis delta is within 3x the rest length, then the delta >> 3 is scaled
 * on the GTE by ((len - rest) << 14) / len into the 0x1F8000BC output array
 * (len via the byte-LUT integer sqrt, GTE LZC above 0x400). The last link is
 * emitted after the loop without the bisection.
 * GTE (owner cop2 cluster grant, cop2-addressing-preamble-cluster): inline_o.h
 * gte_ldlvl, gte_sqr0, gte_stlvnl, gte_ldlzc, gte_stlzc, gte_lddp, gte_gpf12,
 * gte_stlvl, and two bare gte_nop (inline_o.h:3068) filling the post-loop LZC
 * result delay. Departures from the header text: each macro's statements are
 * joined into one __asm__ with its own clobbers ("$12"-"$15","memory"), `($12)`
 * is written `0($12)` (same encoding), and gte_sqr0 / gte_gpf12 carry the real
 * cop2 words (SQR 0x4AA00428, GPF 0x4B98003D) in place of the DMPSX
 * placeholders 0x00000f3f / 0x000012bf. */
void func_80018300(Func80017A44Output *arg0) {
    s32 *out;
    Func80017848Edge *data;
    Func80017A44Record *base;
    s32 count;
    s32 thresh;
    s32 radius;
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
    radius = data->dist;
    p1 = &base[thresh >> 16];
    p2 = &base[thresh & 0xFFFF];
    thresh = radius * 3;
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
            if (p2->index < 0) {
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
        /* gte_ldlvl(r1) -- inline_o.h:308 */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $9, 0($12)\n"
            "lwc2   $10, 4($12)\n"
            "lwc2   $11, 8($12)\n"
            : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
        /* gte_sqr0() -- inline_o.h:1749 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n"
            : : : "$12", "$13", "$14", "$15", "memory");
        data++;
        count--;
        thresh = data->ends.pair;
        /* gte_stlvnl(r1) -- inline_o.h:2422 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            : : "r"((s32 *)0x1F80000C) : "$12", "$13", "$14", "$15", "memory");
        sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
        if (sum < 0x400) {
            len = g_sqrt_table_u8[sum];
            p1 = &base[thresh >> 16];
            p2 = &base[thresh & 0xFFFF];
        } else {
            /* gte_ldlzc(r1) -- inline_o.h:645; the next link's node pointers
             * are computed in the LZC result delay, before gte_stlzc. */
            __asm__ volatile(
                "move   $12, %0\n"
                "mtc2   $12, $30\n"
                : : "r"(sum) : "$12", "$13", "$14", "$15", "memory");
            p1 = &base[thresh >> 16];
            p2 = &base[thresh & 0xFFFF];
            /* gte_stlzc(r1) -- inline_o.h:2999 */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $31, 0($12)\n"
                : : "r"(lz) : "$12", "$13", "$14", "$15", "memory");
            /* FAKE: `len` carries the LZC count and the table shift before the
             * root, and `sum` takes the table byte, keeping them in $v1 / $a0
             * as the target does; fresh shift / byte locals: score 22
             * (staged-value-reused-variable) */
            len = lz[0];
            len = 0x16 - (len & ~1);
            sum = g_sqrt_table_u8[sum >> len];
            len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
        }
        /* gte_ldlvl(r1) -- inline_o.h:308 */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $9, 0($12)\n"
            "lwc2   $10, 4($12)\n"
            "lwc2   $11, 8($12)\n"
            : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
        f = (s32)((len - radius) << 14) / (s32)len;
        radius = data->dist;
        thresh = radius * 3;
        nthresh = -thresh;
        /* gte_lddp(r1) -- inline_o.h:443 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $8\n"
            : : "r"(f) : "$12", "$13", "$14", "$15", "memory");
        /* gte_gpf12() -- inline_o.h:1875 */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4B98003D\n"
            : : : "$12", "$13", "$14", "$15", "memory");
        dx = p2->pos[0] - p1->pos[0];
        dy = p2->pos[1] - p1->pos[1];
        dz = p2->pos[2] - p1->pos[2];
        /* gte_stlvl(r1) -- inline_o.h:2403 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $9, 0($12)\n"
            "swc2   $10, 4($12)\n"
            "swc2   $11, 8($12)\n"
            : : "r"(out) : "$12", "$13", "$14", "$15", "memory");
        out += 3;
    }
    *(s32 *)0x1F800000 = dx >> 3;
    *(s32 *)0x1F800004 = dy >> 3;
    *(s32 *)0x1F800008 = dz >> 3;
    /* gte_ldlvl(r1) -- inline_o.h:308 */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "lwc2   $11, 8($12)\n"
        : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
    /* gte_sqr0() -- inline_o.h:1749 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4AA00428\n"
        : : : "$12", "$13", "$14", "$15", "memory");
    /* gte_stlvnl(r1) -- inline_o.h:2422 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        : : "r"((s32 *)0x1F80000C) : "$12", "$13", "$14", "$15", "memory");
    sum = *(s32 *)0x1F80000C + *(s32 *)0x1F800010 + *(s32 *)0x1F800014;
    if (sum < 0x400) {
        len = g_sqrt_table_u8[sum];
    } else {
        /* gte_ldlzc(r1) -- inline_o.h:645 */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            : : "r"(sum) : "$12", "$13", "$14", "$15", "memory");
        /* gte_nop() x2 -- inline_o.h:3068 */
        __asm__ volatile("nop" : : : "$12", "$13", "$14", "$15", "memory");
        __asm__ volatile("nop" : : : "$12", "$13", "$14", "$15", "memory");
        /* gte_stlzc(r1) -- inline_o.h:2999 */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            : : "r"(lz) : "$12", "$13", "$14", "$15", "memory");
        /* FAKE: same staged len/sum reuse as the loop's LZC arm above (fresh
         * shift / byte locals: score 3). */
        len = lz[0];
        len = 0x16 - (len & ~1);
        sum = g_sqrt_table_u8[sum >> len];
        len = (u32)(sum << 16) >> (0x10 - ((s32)len >> 1));
    }
    /* gte_ldlvl(r1) -- inline_o.h:308 */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "lwc2   $11, 8($12)\n"
        : : "r"((s32 *)0x1F800000) : "$12", "$13", "$14", "$15", "memory");
    f = (s32)((len - radius) << 14) / (s32)len;
    /* gte_lddp(r1) -- inline_o.h:443 */
    __asm__ volatile(
        "move   $12, %0\n"
        "mtc2   $12, $8\n"
        : : "r"(f) : "$12", "$13", "$14", "$15", "memory");
    /* gte_gpf12() -- inline_o.h:1875 */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4B98003D\n"
        : : : "$12", "$13", "$14", "$15", "memory");
    /* gte_stlvl(r1) -- inline_o.h:2403 */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $9, 0($12)\n"
        "swc2   $10, 4($12)\n"
        "swc2   $11, 8($12)\n"
        : : "r"(out) : "$12", "$13", "$14", "$15", "memory");
}

void func_800187F4(Unk80017FA0Rec *arg0, Func80017A44Output *arg1);
void func_80019310(Unk80017FA0Rec *arg0, Func80017A44Output *arg1);

/* func_800187F4 -- node-chain integrator, called by func_8001924C for each
 * Unk80017FA0Rec whose unk2 bit 0 is clear. A node with state >= 0 springs
 * toward (or snaps to) its GTE-transformed anchor; otherwise the indexed
 * scratchpad forces are applied, the node is pushed out of the ground and out
 * of each collision ellipsoid, and the velocity is damped by 7/8 with 0x190
 * added to Y. GTE: PsyQ 4.3 inline_o.h macros (and gtemac.h gte_Lzc), statement
 * for statement (inline-asm-policy class route, owner ruling 2026-09-26); the
 * gte_rtv0tr / gte_sqr0 / gte_gpf0 / gte_gpl12 units carry their command words
 * in place of the DMPSX placeholders (Q29): 0x0000027f -> 0x4A480012,
 * 0x00000f3f -> 0x4AA00428, 0x000012ff -> 0x4B90003D, 0x0000133f ->
 * 0x4BA8003E. */
void func_800187F4(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    Func80017A44Record *node;
    s32 i;
    s32 count;
    s32 vx, vy, vz;
    s32 bits, bits2;
    s32 r;
    s32 vy_new;
    s32 tot, pen;
    /* FAKE: frame layout -- lz[0] (first gte_stlzc) and lz[1] (second
     * gte_stlzc) are the live LZC outputs; the unwritten 16-byte tail gives the
     * 0x78 frame (as s32 lz[2]: frame 0x68, score 24) (dead-vars-local-array,
     * oversized-locals) */
    s32 lz[6];

    func_80018094(arg0, arg1);
    count = arg1->count;
    node = arg1->records;

    for (i = 0; i < count; i++, node++) {
        /* Ruling 11 (reused local): loop index of the add-force,
         * subtract-force and ellipsoid loops */
        s32 idx;
        /* Ruling 11 (reused local): force count, field_1C (added) then
         * field_20 (subtracted) */
        s32 nforce;

        SCR->pos[0] = node->pos[0];
        SCR->pos[1] = node->pos[1];
        SCR->pos[2] = node->pos[2];
        if (node->index >= 0) {
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(&arg1->points[i]):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455 (Q29 word) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A480012": : :"$12","$13","$14","$15","memory");
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            if (node->index > 0) {
                SCR->pos[0] += SCR->dpos[0];
                node->field_C =
                    node->field_C / 2 +
                    ((((SCR->vel[0] << 7) - SCR->pos[0]) * node->index) >> 8);
                node->pos[0] = SCR->pos[0] + node->field_C;
                SCR->pos[1] += SCR->dpos[1];
                node->field_10 =
                    node->field_10 / 2 +
                    ((((SCR->vel[1] << 7) - SCR->pos[1]) * node->index +
                      (0x100 - node->index) * 25) >>
                     8);
                node->pos[1] = SCR->pos[1] + node->field_10;
                SCR->pos[2] += SCR->dpos[2];
                node->field_14 =
                    node->field_14 / 2 +
                    ((((SCR->vel[2] << 7) - SCR->pos[2]) * node->index) >> 8);
                node->pos[2] = SCR->pos[2] + node->field_14;
            } else {
                node->pos[0] = SCR->vel[0] << 7;
                node->pos[1] = SCR->vel[1] << 7;
                node->pos[2] = SCR->vel[2] << 7;
            }
            continue;
        }
        if (node->index >= -0xFF) {
            /* gte_ldv0(r1) -- inline_o.h 4.3 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(&arg1->points[i]):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* gte_rtv0tr() -- inline_o.h 4.3 :451-455 (Q29 word) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A480012": : :"$12","$13","$14","$15","memory");
            /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            SCR->pos[0] -=
                (((SCR->vel[0] << 7) - SCR->pos[0]) * node->index) >> 8;
            SCR->pos[1] -=
                (((SCR->vel[1] << 7) - SCR->pos[1]) * node->index) >> 8;
            SCR->pos[2] -=
                (((SCR->vel[2] << 7) - SCR->pos[2]) * node->index) >> 8;
        }
        vx = node->field_C;
        vy = node->field_10;
        vz = node->field_14;
        nforce = node->field_1C;
        bits = *(s32 *)&node->field_24[0];
        for (idx = 0; idx < nforce; idx++) {
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
        nforce = node->field_20;
        bits2 = *(s32 *)&node->field_2C[0];
        for (idx = 0; idx < nforce; idx++) {
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
        SCR->vel[0] = vx;
        SCR->vel[1] = vy;
        SCR->vel[2] = vz;
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
                SCR->vel[1] = vy_new;
            }
            SCR->cpos[0] = SCR->pos[0] >> 5;
            SCR->cpos[1] = SCR->pos[1] >> 5;
            SCR->cpos[2] = SCR->pos[2] >> 5;
            for (idx = 0; idx < SCR->nsph; idx++) {
                s32 dx0, dz0, dy1, dx1, dz1;
                s32 sq2, dist2;
                /* Ruling 11 (reused local): the squared length copied for
                 * gte_Lzc (Q28), then the focus-0 and focus-1 table bytes */
                s32 temp;
                /* Ruling 11 (reused local): focus-0 squared distance, then the
                 * distance (scaled to its push factor below) */
                s32 work;

                r = SCR->rad[idx];
                delta = SCR->cpos[1] - SCR->sph[idx][1];
                if (delta < -r || r < delta) {
                    continue;
                }
                SCR->d0[1] = delta;
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
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d0):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_sqr0() -- inline_o.h 4.3 :646-650 (Q29 word) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4AA00428": : :"$12","$13","$14","$15","memory");
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = work;
                if (work < 0x400) {
                    work = g_sqrt_table_u8[work] >> 3;
                } else {
                    /* Ruling 11 (reused local): leading-zero count, then the
                     * table shift */
                    s32 nbits;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc,
                     * gte_nop twice, gte_stlzc */
                    __asm__ volatile ("move  $12,%0": :"r"(temp):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lz[0]):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    nbits = lz[0];
                    nbits = 0x16 - (nbits & ~1);
                    temp = g_sqrt_table_u8[work >> nbits];
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
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_sqr0() -- inline_o.h 4.3 :646-650 (Q29 word) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4AA00428": : :"$12","$13","$14","$15","memory");
                /* gte_stlvnl(r1) -- inline_o.h 4.3 :904-909 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                if (sq2 < 0x400) {
                    dist2 = g_sqrt_table_u8[sq2] >> 3;
                } else {
                    /* Ruling 11 (reused local): leading-zero count, then the
                     * table shift */
                    s32 nbits2;

                    /* gte_Lzc(r1,r2) -- gtemac.h 4.3 :174-178 = gte_ldlzc,
                     * gte_nop twice, gte_stlzc */
                    __asm__ volatile ("move  $12,%0": :"r"(sq2):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lz[1]):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    nbits2 = lz[1];
                    nbits2 = 0x16 - (nbits2 & ~1);
                    temp = g_sqrt_table_u8[sq2 >> nbits2];
                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = work + dist2;
                if (tot >= r) {
                    continue;
                }
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_gpf0() -- inline_o.h 4.3 :721-725 (Q29 word) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4B90003D": : :"$12","$13","$14","$15","memory");
                pen = (r - tot) << 17;
                if (pen > 0x400000) {
                    pen = 0x400000;
                }
                if (work != 0) {
                    work = pen / work;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d0):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(work):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_gpl12() -- inline_o.h 4.3 :726-730 (Q29 word) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4BA8003E": : :"$12","$13","$14","$15","memory");
                if (dist2 != 0) {
                    dist2 = pen / dist2;
                }
                /* gte_ldlvl(r1) -- inline_o.h 4.3 :104-109 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->d1):"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
                /* gte_lddp(r1) -- inline_o.h 4.3 :144-147 */
                __asm__ volatile ("move  $12,%0": :"r"(dist2):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory");
                /* gte_gpl12() -- inline_o.h 4.3 :726-730 (Q29 word) */
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile (".word 0x4BA8003E": : :"$12","$13","$14","$15","memory");
                /* gte_stlvl(r1) -- inline_o.h 4.3 :898-903 */
                __asm__ volatile ("move  $12,%0": :"r"(SCR->vel):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $9,($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $10,4($12)": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $11,8($12)": : :"$12","$13","$14","$15","memory");
            }
        }
        node->field_C = (SCR->vel[0] * 7) >> 3;
        node->pos[0] = SCR->pos[0] + SCR->dpos[0] + node->field_C;
        node->field_10 = ((SCR->vel[1] * 7) >> 3) + 0x190;
        node->pos[1] = SCR->pos[1] + SCR->dpos[1] + node->field_10;
        node->field_14 = (SCR->vel[2] * 7) >> 3;
        node->pos[2] = SCR->pos[2] + SCR->dpos[2] + node->field_14;
    }
}

void func_8001924C(Unk80017FA0Rec *arg0, s32 arg1) {
    s32 i = 0;
    Unk80017FA0Rec *s0;
    /* FAKE: pointer alias -- g_file_data_buf's address held in an integer
     * local. Referenced directly, its lui/addiu is scheduled after `move s0,a0`
     * (score 2); held as a u8 * the addu operands swap (score 2). */
    s32 buf;

    if (i < arg1) {
        buf = (s32)g_file_data_buf;
        s0 = arg0;
        do {
            if (s0->unk2 & 1) {
                s16 val = s0->unk0;
                func_80019310(s0, (Func80017A44Output *)(val * 52 + buf));
            } else {
                s16 val = s0->unk0;
                func_800187F4(s0, (Func80017A44Output *)(val * 52 + buf));
            }
            i++;
            s0++;
        } while (i < arg1);
    }
}

/* func_80019310 - GTE rotate-and-scale of an SVECTOR array into a 0x40-stride
 * VECTOR table, then a 32-byte MATRIX copy into the descriptor.
 * GTE (PsyQ inline_c.h): gte_SetRotMatrix inline_c.h:297-310,
 * gte_SetTransMatrix inline_c.h:360-369, gte_ldv0 inline_c.h:16-20, a
 * gte_rtv0-class MVMVA (0x4A480012), gte_stlvnl inline_c.h:1111-1117. The
 * islands use func_800203B4's `move $12, %0` macro-body spelling (owner grant,
 * widened cop2 materialize-then-copy anchor, which names func_80019310). Only
 * gte_stlvnl publishes a "memory" clobber (inline_c.h:1116); gte_SetRotMatrix,
 * gte_SetTransMatrix and gte_ldv0 publish only "$12","$13","$14" (gte_ldv0 no
 * clobber list at all), so on those three it is added, and truthful (the
 * islands read the MATRIX / SVECTOR and write out[]); on island 1 it makes GCC
 * re-read the MATRIX pointer before SetTransMatrix. */
void func_80019310(Unk80017FA0Rec *arg0, Func80017A44Output *arg1) {
    s32 out[3];
    s32 i;
    Func80017A44Record *dst;

    /* gte_SetRotMatrix(r0) -- inline_c.h:297-310; published clobbers
     * $12-$14 (inline_c.h:310); the `move $12, %0` preamble and $15 are
     * func_800203B4's granted spelling; "memory" clobber added */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(arg0->unk4) : "$12", "$13", "$14", "$15", "memory");
    /* gte_SetTransMatrix(r0) -- inline_c.h:360-369: translation words
     * 20/24/28 into cop2 control regs $5..$7; published clobbers
     * "$12","$13","$14" (inline_c.h:369), +$15 and "memory" clobbers added */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(arg0->unk4) : "$12", "$13", "$14", "$15", "memory");

    dst = arg1->records;
    for (i = 0; i < arg1->count; i++) {
        /* gte_ldv0(r0) -- inline_c.h:16-20: lwc2 VXY0/VZ0, plus the 2-cycle
         * GTE load delay; "$12" and "memory" clobbers added */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $0, 0($12)\n"
            "lwc2   $1, 4($12)\n"
            "nop\n"
            "nop\n"
            :: "r"(&arg1->points[i]) : "$12", "memory");
        /* gte_mvmva(sf,mx,v,cv,lm) -- inline_c.h:816-817 (body
         * gte_mvmva_core, inline_c.h:809-814), here gte_mvmva(1,0,0,0,0):
         * sf=1, mx=rotation, v=V0, cv=TR, lm=0, written as its encoded cop2
         * word (this assembler rejects the macro's literal encoding); the
         * macro's two nops are the tail of the gte_ldv0 island. */
        __asm__ volatile(".word 0x4A480012");
        /* gte_stlvnl(r0) -- inline_c.h:1111-1117: stores MAC1/MAC2/MAC3;
         * "memory" is its own published clobber (inline_c.h:1116); "$12" is
         * added with the preamble */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(out) : "$12", "memory");
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
    D_80106A50.color[0] = 0x11;
    D_80106A50.color[1] = 0x44;
    D_80106A50.color[2] = 0x88;
}

s32 func_80019488(void) {
    return (D_80106A50.color[0] & 0xF) | ((D_80106A50.color[1] & 0xF) << 4) |
           ((D_80106A50.color[2] & 0xF) << 8);
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

void func_80019568(s32 arg0) {
    PadState pad;
    s32 pkts[4];
    s32 i;
    s32 held;
    s32 old_held;

    held = 0;
    i = 0;
    pkts[0] = g_pad_buf[0][0];
    pkts[1] = g_pad_buf[0][1];
    pkts[2] = g_pad_buf[1][0];
    pkts[3] = g_pad_buf[1][1];
    do {
        /* FAKE: rec holds the packet's address for the loop body; indexing the
         * packet bytes at each read adds a second induction (score 18). */
        u8 *rec = (u8 *)pkts + i * 8;
        s32 valid = 0;
        s32 bits;

        if (rec[0] == 0) {
            s32 type_m1;

            pad.type[i] = rec[1] >> 4;
            valid = 1;
            /* FAKE: `pad.valid[i] = valid;` duplicated into both arms keeps
             * loop.c from hoisting the `1`, which fills the lhu delay slot; a
             * single store after the join: score 21 (family:
             * duplicated-statement-into-arms, rule
             * duplicated-statement-into-arms) */
            pad.valid[i] = valid;
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
            pad.valid[i] = valid;
            bits = 0;
        }

        held = ((u32)held >> 16) | (bits << 16);
        i++;
    } while (i < 2);

    pad.held = held;
    func_8001B138(&pad.held);

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

    func_8003A728(&pad);

    i = 0;
    do {
        g_pad_state.type[i] = pad.type[i];
        g_pad_state.valid[i] = pad.valid[i];
        i++;
    } while (i < 2);

    old_held = g_pad_state.held;
    g_pad_state.held = pad.held;
    g_pad_state.pressed = pad.held & ~old_held;
    g_pad_state.unheld = ~pad.held;
    g_pad_state.released = ~pad.held & old_held;
}
