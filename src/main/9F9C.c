/* 93 game functions, among them math_SignExt12Div, math_FloorDiv2000, math_RotMatrixZYXAngles and
 * pad_ClearStateBits. .text 0x8001979C (ROM 0x9F9C). Start boundary: PHASE (a jump-table phase
 * change, docs/grind/rodata-align-2026-09-30.md site 1). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

/* Declarations from the file this TU was split from (code6cac.c). */

extern s32 memcpy(s32 *, s32, s32);
extern s32 func_80054434(void);
extern s32 rand();
extern void func_800325E0(s32, s32);

INCLUDE_RODATA("asm/rodata", D_800100A4);
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    u32 dst2;
    u32 out;
    s32 val;
    s32 nd;
    s32 neg2;
    u32 lo;
    u32 lo2;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        dst = base + i * 2;
        if (bits_left < 0xC) {
            /* FAKE: nd names the width subtraction so the constant's load site separates from its use site -- cse.c:7454 re-materialises the deleted early subu at the copy site. */
            nd = 0xC - bits_left;
            /* FAKE: hi carries its own shift amount before the value -- global.c allocno_compare (hi crosses the floor_log2 nrefs bucket and keeps $v1). */
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
            /* FAKE: new bits_left routed through val -- blocks cse.c:7454's cheapest-register rewrite via make_regs_eqv's last-use test (cse.c:856). */
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo;
            *(s16 *)(dst + 0xA) = (s16)hi;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
    } while (i < 0x3F);

    i = 0;
    do {
        dst2 = base + i * 2;
        if (bits_left < 2) {
            /* FAKE: nd - same construct as loop 1 (cse.c re-materialisation). */
            nd = 2 - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo2 = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo2;
            *(s16 *)(dst2 + 0x8E) = (s16)hi;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
    } while (i < 0x3F);

    /* FAKE: named constant holder for the fill value -- global.c find_reg conflict graph (a separate allocno for -2 is what puts the fill pointer in $v0). */
    neg2 = -2;
    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = neg2;
        i--;
        out -= 0x118;
    } while (i >= 0);
    /* FAKE: tail reuse of val ([[named-local-fake-exception]] constant-holder): this later SET of val blocks cse.c:7454's cheapest-register rewrite (make_regs_eqv last-use test, cse.c:856) of the in-loop `val = 0x20 - needed; bits_left = val;`, preserving the subu $v0,$t2,$a0 + move $a3,$v0 copy pair that exists in the target bytes in BOTH loops (asm/funcs/func_8001979C.s lines 28/58). A direct literal store here collapses both pairs to subu $a3,$t2,$a0 (75 insns vs the target's 77). */
    val = 0;
    *(s32 *)(base + 0x10C) = val;
}
extern s32 D_800A30EC;

/* Motion bitstream reader: `cur` holds the unread bits left-aligned, `bits`
   how many of them are valid, `ptr` the next stream word. GETBITS reads `nb`
   bits into `dst`; GETBITS_PRE ORs them onto `pre` (the implicit top bit of
   an escape/VLC code). */
#define GETBITS_PRE(dst, nb, pre)                               \
    {                                                           \
        u32 top = (pre);                                        \
        if (bits < (nb)) {                                      \
            s32 need = (nb) - bits;                             \
            u32 hi = cur >> (32 - bits);                        \
            s32 left = 32 - need;                               \
            cur = *ptr++;                                       \
            dst = top | ((hi << need) | (cur >> left));         \
            cur <<= need;                                       \
            bits = left;                                        \
        } else {                                                \
            dst = top | (cur >> (32 - (nb)));                   \
            cur <<= (nb);                                       \
            bits -= (nb);                                       \
        }                                                       \
    }

#define GETBITS(dst, nb)                                        \
    if (bits < (nb)) {                                          \
        s32 need = (nb) - bits;                                 \
        u32 hi = cur >> (32 - bits);                            \
        s32 left = 32 - need;                                   \
        cur = *ptr++;                                           \
        dst = (hi << need) | (cur >> left);                     \
        cur <<= need;                                           \
        bits = left;                                            \
    } else {                                                    \
        dst = cur >> (32 - (nb));                               \
        cur <<= (nb);                                           \
        bits -= (nb);                                           \
    }

#define COPY33(d, s)                                            \
    {                                                           \
        u32 *from = (u32 *)(s);                                 \
        u32 *to = (u32 *)(d);                                   \
        u32 k;                                                  \
        for (k = 0; k < 33; k++) {                              \
            *to++ = *from++;                                    \
        }                                                       \
    }

/* Decode motion frame `frame` of object `obj` into `out` (33 words). Frames are
   keyframes every 8 plus up to 7 delta sub-frames; the record at
   D_800F1B18[obj * 0x570] caches the last four decoded frames with the reader
   state after each, so a frame is either copied from the cache, continued from
   the cached previous frame, or decoded from its keyframe. `work` holds the
   current pose (+0x00), the per-channel rates (+0x84) and the channel codes
   (+0x108). */
void func_800198D0(s32 obj, s32 frame, MotionFrame *out, u16 *work) {
    u8 *rec;
    u8 *tbl;
    u8 *slot;
    u8 *prev;
    u32 *ptr;
    u32 cur;
    s32 bits;
    s32 sub;
    s32 key;
    s32 ctr;
    /* Ruling 11 (ordinary-c-judge-decidable.md): two values, both loop indices --
     * the keyframe channel loop's and the post-pass column loop's. */
    s32 idx;
    s32 ch;
    /* Ruling 11: two values, both loop indices -- the sub-frame loop's and the
     * post-pass row loop's. */
    s32 idx2;
    s32 off;
    s32 shift;
    u16 *p;
    u16 code;
    /* Ruling 11: eight values, each a bit field read by GETBITS -- the three
     * keyframe header words, the per-channel keyframe flag, the three
     * sub-frame header words and case 3's 4-bit low code. */
    u32 field;
    s16 x;

    sub = frame & 7;
    key = frame >> 3;
    rec = &D_800F1B18[obj * 0x570];
    tbl = (u8 *)*(u32 **)rec + 0x70;
    if (D_800A30EC == 0) {
        COPY33(work + 0x84, rec + 0x88);
    }
    ctr = *(s32 *)(rec + 0x10C);
    *(s32 *)(rec + 0x10C) = ctr + 1;
    slot = rec + (((ctr + 1) & 3) * 0x118 + 0x110);
    if (*(s32 *)slot == frame) {
        COPY33(out, slot + 4);
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    prev = rec + ((ctr & 3) * 0x118 + 0x110);
    if (*(s32 *)prev == frame) {
        COPY33(out, prev + 4);
        *(s32 *)(rec + 0x10C) -= 1;
        return;
    }
    if (*(s32 *)slot != frame - 1) {
        if (*(s32 *)prev == frame - 1) {
            slot = prev;
        } else {
            off = ((tbl[key * 3] << 16) | (tbl[key * 3 + 1] << 8) | tbl[key * 3 + 2]) + 0x380;
            ptr = *(u32 **)rec + (off >> 5);
            shift = off & 0x1F;
            bits = 32 - shift;
            cur = *ptr++ << shift;
            COPY33(work, rec + 4);
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[0] = field;
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[1] = field;
            GETBITS(field, 1);
            if (field) {
                GETBITS(field, 16);
            }
            work[2] = field;
            for (idx = 0; idx < 63; idx++) {
                GETBITS(field, 1);
                if (field) {
                    GETBITS(work[idx + 3], 12);
                }
            }
            idx2 = 0;
            goto decode;
        }
    }
    ptr = *(u32 **)(slot + 0x10C);
    bits = *(s32 *)(slot + 0x110);
    cur = *(u32 *)(slot + 0x114);
    COPY33(work, slot + 4);
    if (sub >= 2) {
        COPY33(work + 0x42, slot + 0x88);
    }
    idx2 = sub - 1;
decode:
    for (; idx2 < sub; idx2++) {
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[0] = field;
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[1] = field;
        GETBITS(field, 1);
        if (field) {
            GETBITS(field, 16);
        }
        work[2] = field;
        for (ch = 0; ch < 63; ch++) {
            /* Ruling 11: four values -- the channel's decoded delta, case 1's
             * magnitude, case 2's zero flag and case 3's magnitude. */
            s16 temp;

            code = work[ch + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1: {
                /* Ruling 11: two values, both bit counts -- the zero-run length and
                 * the suffix length (one less). */
                s32 nbits;

                nbits = 0;
                do {
                    u32 bit;

                    if (bits == 0) {
                        cur = *ptr++;
                        bits = 32;
                    }
                    bit = cur >> 31;
                    cur <<= 1;
                    bits--;
                    if (bit) {
                        break;
                    }
                    nbits++;
                } while (nbits < 12);
                if (nbits == 12) {
                    GETBITS_PRE(temp, 11, 0x800);
                } else if (nbits >= 2) {
                    nbits = nbits - 1;
                    GETBITS_PRE(temp, nbits, 1 << nbits);
                } else {
                    temp = nbits;
                }
                temp = (temp & 1) ? -(temp / 2) - 1 : temp / 2;
                break;
            }
            case 2: {
                GETBITS(temp, 1);
                if (temp) {
                    temp = 0;
                } else {
                    GETBITS(temp, 12);
                }
                break;
            }
            case 3: {
                GETBITS(temp, 1);
                if (temp) {
                    /* Ruling 11: two values, both bit counts -- the zero-run length
                     * and the suffix length (one less). */
                    s32 nbits2;

                    GETBITS(field, 4);
                    nbits2 = 0;
                    do {
                        u32 bit;

                        if (bits == 0) {
                            cur = *ptr++;
                            bits = 32;
                        }
                        bit = cur >> 31;
                        cur <<= 1;
                        bits--;
                        if (bit) {
                            break;
                        }
                        nbits2++;
                    } while (nbits2 < 8);
                    if (nbits2 == 8) {
                        GETBITS_PRE(temp, 7, 0x80);
                    } else if (nbits2 >= 2) {
                        nbits2 = nbits2 - 1;
                        GETBITS_PRE(temp, nbits2, 1 << nbits2);
                    } else {
                        temp = nbits2;
                    }
                    temp = ((temp << 3) | (field & 7)) + 1;
                    if (field & 8) {
                        temp = -temp;
                    }
                }
                break;
            }
            }
            if (idx2 == 0) {
                work[ch + 0x45] = temp;
                work[ch + 3] += temp;
            } else {
                work[ch + 3] += work[ch + 0x45] + temp;
                work[ch + 0x45] += temp;
            }
        }
    }
    p = &work[0x36];
    for (idx2 = 0; idx2 < 2; idx2++) {
        for (idx = 0; idx < 3; idx++) {
            x = *p;
            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);
            p++;
        }
        p += 3;
    }
    COPY33(out, work);
    if (sub < 7) {
        *(s32 *)slot = frame;
        COPY33(slot + 4, work);
        COPY33(slot + 0x88, work + 0x42);
        *(u32 **)(slot + 0x10C) = ptr;
        *(s32 *)(slot + 0x110) = bits;
        *(u32 *)(slot + 0x114) = cur;
    }
}
void func_8001A484(u16 *arg0) {
    s32 i;
    u16 *p;
    i = 0;
    p = arg0 + 2;
    do {
        i++;
        func_8003D52C((s32)&D_800100A4, arg0[0], p[-1], p[0]);
        p += 3;
        arg0 += 3;
    } while (i < 0x16);
}
s32 math_SignExt12Div(s32 arg0, s32 arg1) {
    s32 v = arg0 & 0xFFF;
    if (v >= 0x800) {
        v -= 0x1000;
    }
    return v / arg1;
}
void func_8001A538(Rec44 *arg0, s32 *arg1) {
    MATRIX m;
    m.m[0][0] = 0x1000;
    m.m[0][1] = 0;
    m.m[0][2] = 0;
    m.m[1][0] = 0;
    m.m[1][1] = 0x1000;
    m.m[1][2] = 0;
    m.m[2][0] = 0;
    m.m[2][1] = 0;
    m.m[2][2] = 0x1000;
    RotMatrixX(-arg0->h10, &m);
    RotMatrixY(-arg0->h12, &m);
    RotMatrixZ(-arg0->h14, &m);
    arg1[0] = arg0->unk_00.x - ((s32)(m.m[0][2] * arg0->w18) >> 12);
    arg1[1] = arg0->unk_00.y - ((s32)(m.m[1][2] * arg0->w18) >> 12);
    arg1[2] = arg0->unk_00.z - ((s32)(m.m[2][2] * arg0->w18) >> 12);
}
s32 math_FloorDiv2000(s32 arg0) {
    if (arg0 < 0) {
        return -((0x7CF - arg0) / 2000);
    }
    return arg0 / 2000;
}
void func_8001A67C(s16 *arg0, s32 *arg1, s32 *arg2) {
    s32 dx;
    s32 dz;
    u32 dist_sq;
    u32 log2_val;
    s32 sp_tmp;
    dx = arg1[0] - arg2[0];
    dz = arg1[2] - arg2[2];
    while ((((u32)(dx + 0x4000)) > 0x8000U) || (((u32)(dz + 0x4000)) > 0x8000U)) {
        dx = dx / 2;
        dz = dz / 2;
    }
    dist_sq = (dx * dx) + (dz * dz);
    if (dist_sq < 0x400U) {
        log2_val = ((u32)((u8)(g_sqrt_table_u8[dist_sq]))) >> 3;
    } else {
        u32 shift_a;
        u32 shift_b;
        /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
         * canonical inline asm (owner-authorized). The original is
         * hand asm: $t4 reused back-to-back for two unrelated values (no
         * compiler RA does this), 2 unfilled GTE delay nops, splat tags the
         * cop2 ops "handwritten instruction". */
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
            "nop\n"
            "nop\n"
            "addiu  $v0, $sp, 0x10\n"  /* &sp_tmp */
            "addu   $t4, $v0, $zero\n"
            "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
            : "=m"(sp_tmp)
            : "r"(dist_sq)
            : "$2", "$12");
        {
            s32 lw_v1 = sp_tmp;
            s32 li_v0 = -2;
            li_v0 = lw_v1 & li_v0;
            shift_a = 0x16 - li_v0;
        }
        shift_b = shift_a >> 1;
        log2_val = (((u32)((u8)(g_sqrt_table_u8[dist_sq >> shift_a]))) << 16) >> (0x13 - shift_b);
    }
    arg0[0] = (s16)math_FloorDiv2000(arg2[0] + ((dx << 10) / ((s32)log2_val)));
    arg0[2] = (s16)math_FloorDiv2000(arg2[2] + ((dz << 10) / ((s32)log2_val)));
}
void func_8001A820(Vec3i32 *arg0, Vec3i32 *arg1, Unk80101EC8Record *arg2, Unk80101EC8Record *arg3);
extern u8 D_800A30F0[];
extern s32 D_800A30F4[];
typedef Vec4i32 CamVec;
/* Scratchpad work area (0x1F800000) used by func_8001A820: the target yaw/roll
 * that D_800F6608's h12/h14 ease toward, the camera focus, the eye position
 * func_8001A538 computes, a fighter's head position, and the hit position,
 * surface normal and work area func_80053614 is given (its 3rd/4th/5th
 * arguments; its other callers pass a VECTOR hit and an s16[4] normal). */
typedef struct {
    s16 unk0;       /* 0x00 */
    s16 yaw;        /* 0x02 */
    s16 roll;       /* 0x04 */
    s16 unk6;       /* 0x06 */
    CamVec focus;   /* 0x08 */
    CamVec eye;     /* 0x18 */
    CamVec head;    /* 0x28 */
    s32 hit[8];     /* 0x38 */
    s16 nrm[4];     /* 0x58 */
    s32 unk60;      /* 0x60 */
} CamScratch;
/* Two-fighter camera for D_800F6608 (arg0/arg1 = the fighters' positions,
 * arg2/arg3 = the fighter records; caller func_8001E878). Resets the h30[][]
 * limits; eases the focus toward the fighters' midpoint (or arg0's position
 * when D_800A3690 is set); turns the fighters' separation into a zoom target
 * (distance through the D_8008D118 byte-LUT square root with the GTE
 * leading-zero count for large inputs, weapon-state adjustments, eased 1/12
 * and clamped) and a hysteresis flag passed to func_8003F1E4; eases h12/h14
 * toward the facing yaw and zero roll; then, per fighter, bisects h10 (rot_x)
 * so the fighter's head stays visible past the camera-collision normal test of
 * func_80053614, keeping a per-fighter step in D_800A30F4[] and a blocked flag
 * in D_800A30F0[]; finally eases h10 toward the larger of the two results,
 * clamped to 0x80..0x1C0.
 *
 * GTE island: PsyQ gte_Lzc(r1,r2), gtemac.h 4.3 :174-178, written out as the
 * six statements of its inline_o.h 4.3 expansion, character for character
 * against engine/gtemacro.py PINNED: gte_ldlzc(r1) :207-210, gte_nop() :1095-1097
 * twice, gte_stlzc(r2) :1074-1077 (the header's `($12)` verbatim). No other
 * asm; operand seats chosen by cc1. */
void func_8001A820(Vec3i32 *arg0, Vec3i32 *arg1, Unk80101EC8Record *arg2, Unk80101EC8Record *arg3) {
    s32 lzc_out;
    CamScratch *scr;
    Rec44 *cam;
    s32 dx, dy, dz;
    s32 x, y, z;
    s32 shift;
    u32 dist_sq;
    u32 dist;
    u32 q;
    s32 zoom;
    s32 pitch0, pitch1;
    s32 base_pitch;
    s32 p;
    /* work holds three values, all h10 (rot_x) quantities: the per-fighter
     * bisection angle (loop), the target angle max(pitch0, pitch1) clamped to
     * 0x80..0x1C0, and the final eased step. Ruling 11
     * (ordinary-c-judge-decidable.md). */
    s32 work;
    s32 i, j;

    scr = (CamScratch *)0x1F800000;
    cam = &D_800F6608;
    cam->h30[0][0] = 0x64;
    cam->h30[0][1] = 0;
    cam->h30[0][2] = 0x64;
    cam->h30[1][0] = 0x64;
    cam->h30[1][1] = 0;
    cam->h30[1][2] = 0x64;
    dx = arg1->x - arg0->x;
    dy = arg1->y - arg0->y;
    dz = arg1->z - arg0->z;
    if (D_800A3690 == 0) {
        scr->focus.vx = (arg0->x + arg1->x) / 2;
        scr->focus.vy = (arg0->y + arg1->y) / 2;
        scr->focus.vz = (arg0->z + arg1->z) / 2;
    } else {
        scr->focus = *(CamVec *)arg0;
    }
    cam->unk_00.x += (scr->focus.vx - cam->unk_00.x) / 4;
    cam->unk_00.y += (scr->focus.vy - cam->unk_00.y) / 4;
    cam->unk_00.z += (scr->focus.vz - cam->unk_00.z) / 4;

    x = dx;
    y = dy;
    z = dz;
    shift = 0;
    while ((u32)(x + 0x4000) > 0x8000U || (u32)(z + 0x4000) > 0x8000U) {
        x /= 2;
        y /= 2;
        z /= 2;
        shift++;
    }
    dist_sq = x * x + z * z + y * y;
    if (dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if ((s32)dist_sq >= 0) {
            /* gte_Lzc(dist_sq, &lzc_out): gtemac.h 4.3 :174-178 = inline_o.h 4.3
             * gte_ldlzc :207-210, gte_nop :1095-1097 (x2), gte_stlzc :1074-1077 */
            __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = lzc_out;
        }
        {
            s32 sh = 0x16 - (lzcr & ~1);
            s32 tbl = g_sqrt_table_u8[dist_sq >> sh];
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)sh >> 1));
        }
    }
    dist <<= shift;
    q = 0x2000000U / (dist + 0x4000) + 0x400;
    if (arg2->unk_6A == 0x13 || arg2->unk_6A == 0x1B || arg2->unk_6A == 0x30 ||
        arg3->unk_6A == 0x13 || arg3->unk_6A == 0x1B || arg3->unk_6A == 0x30) {
        q += 0x1000;
    }
    zoom = ((dist + q) << 7) / 100;
    if (dy < 0) {
        dy = -dy;
    }
    zoom += dy;
    if (arg2->unk_6A == 0xF || arg2->unk_6A == 0x1C || arg2->unk_6A == 0x1D ||
        arg2->unk_6A == 0x1E || arg2->unk_6A == 0x1F || arg2->unk_6A == 0x20 ||
        arg2->unk_6A == 0x21) {
        zoom = 0xBB8;
    }
    cam->w18 += (zoom - cam->w18) / 12;
    if (!(arg2->unk_6A == 0xF || arg2->unk_6A == 0x1C || arg2->unk_6A == 0x1D ||
          arg2->unk_6A == 0x1E || arg2->unk_6A == 0x1F || arg2->unk_6A == 0x20 ||
          arg2->unk_6A == 0x21) && cam->w18 < 0x1770) {
        cam->w18 = 0x1770;
    }
    if (cam->w18 > 100000) {
        cam->w18 = 100000;
    }
    if (cam->b1E) {
        cam->b1E = cam->w18 >= 0x558D;
    } else {
        cam->b1E = cam->w18 >= 0x55F1;
    }
    func_8003F1E4(cam->b1E);

    if (arg2->unk_6A == 0x11) {
        scr->yaw = cam->h12;
    } else {
        scr->yaw = (0x400 - ratan2(dx, dz)) & 0xFFF;
    }
    scr->roll = 0;
    cam->h12 += math_SignExt12Div(scr->yaw - cam->h12, 8);
    cam->h14 += math_SignExt12Div(scr->roll - cam->h14, 8);
    base_pitch = cam->h10;

    for (p = 0; p < 2; p++) {
        s32 hi, lo;

        work = base_pitch;
        cam->h10 = work;
        func_8001A538(cam, &scr->eye.vx);
        if (p != 0) {
            scr->head = arg3->unk_B8;
        } else {
            scr->head = arg2->unk_B8;
        }
        scr->head.vy -= 0xC8;
        if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, scr->nrm, (s32)&scr->unk60) &&
            scr->nrm[1] < -0x320) {
            func_8001A67C(cam->h30[p], (s32 *)&scr->eye, scr->hit);
            if (D_800A30F0[p]) {
                D_800A30F4[p] += 0x20;
            } else {
                D_800A30F4[p] = 0x10;
                work--;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x200) {
                D_800A30F4[p] = 0x200;
            }
            hi = work + D_800A30F4[p] / 8;
            /* FAKE: cancellation pair (F6 semantically-null pair,
             * no-new-park-categories.md) -- global.c allocno_compare priority:
             * the pair adds references to `hi` (allocno_n_refs 13 -> 21, pri
             * 11142 -> 23333 against work's 13253), so the bounds are allocated
             * before `work` and take $s0 and `work` $s1, as in the target;
             * combine folds the pair to nothing. */
            hi++;
            hi--;
            for (i = 0; i < 2; i++) {
                s32 d = (work - hi) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = hi + d / 2;
                func_8001A538(cam, &scr->eye.vx);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, scr->nrm,
                                  (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    work = cam->h10;
                } else {
                    hi = cam->h10;
                }
            }
            work = hi;
            D_800A30F0[p] = 1;
        } else {
            if (D_800A30F0[p]) {
                D_800A30F4[p] = 0x10;
                work++;
            } else {
                D_800A30F4[p] += 0x10;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x100) {
                D_800A30F4[p] = 0x100;
            }
            lo = work - D_800A30F4[p] / 8;
            for (j = 0; j < 2; j++) {
                s32 d = (work - lo) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = lo + d / 2;
                func_8001A538(cam, &scr->eye.vx);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, scr->nrm,
                                  (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    func_8001A67C(cam->h30[p], (s32 *)&scr->eye, scr->hit);
                    lo = cam->h10;
                } else {
                    work = cam->h10;
                }
            }
            D_800A30F0[p] = 0;
        }
        if (p != 0) {
            pitch1 = work;
        } else {
            pitch0 = work;
        }
    }
    work = (pitch0 < pitch1) ? pitch1 : pitch0;
    if (work < 0x80) {
        work = 0x80;
    }
    if (work > 0x1C0) {
        work = 0x1C0;
    }
    cam->h10 = base_pitch;
    work = math_SignExt12Div(work - base_pitch, 8);
    cam->h10 += work;
}
void func_8001B138(s32 *arg0) {
    D_800FF5C8 = 0;
    D_800FF5CC = 0;
    D_800FF5D0 = 0;
    D_800FF5D8 = 0;
    D_800FF5DA = 0;
    D_800FF5DC = 0;
    D_800FF5E0 = 0;
    if (D_800A38BA != 0 && D_800A3834 == 1) {
        if (*arg0 & 1) {
            D_800A37E0 = 1;
            if (*arg0 & 8) {
                D_800A3710 = D_800A3710 + 0x4CC;
            }
            if (*arg0 & 2) {
                D_800A3710 = D_800A3710 - 0x4CC;
            }
            if (D_800A3710 < -0x1C00) {
                D_800A3710 = -0x1C00;
            }
            if (D_800A3710 >= 0x7401) {
                D_800A3710 = 0x7400;
            }
            *arg0 = *arg0 & ~0xB;
        }
        {
            s32 v;
            v = D_800A3710;
            if (v < 0) {
                v = v + 0xF;
            }
            D_800FF5E0 = v >> 4;
        }
    }
    *arg0 = *arg0 & (s32)0xFFFEFFFE;
}
void func_8001B294(Unk80101EC8Record *a0, Unk80101EC8Record *a1) {    s32 v0;    D_800A36FA = 0;    D_800F6608.h30[0][0] = 0x64;    D_800F6608.h30[0][1] = 0;    D_800F6608.h30[0][2] = 0x64;    D_800F6608.h30[1][0] = 0x64;    D_800F6608.h30[1][1] = 0;    D_800F6608.h30[1][2] = 0x64;    func_8003F1E4(0);    D_800F6608.unk_00.x = (a0->unk_F4.x + a1->unk_F4.x) / 2;    D_800F6608.unk_00.y = (a0->unk_F4.y + a1->unk_F4.y) / 2;    {        s32 t1 = a0->unk_F4.z;        s32 t2 = a1->unk_F4.z;        D_800F6608.h10 = 0;        D_800F6608.unk_00.z = (t1 + t2) / 2;    }    {        s32 dx = a1->unk_F4.x - a0->unk_F4.x;        s32 dz = a1->unk_F4.z - a0->unk_F4.z;        v0 = ratan2(dx, dz);    }    D_800F6608.h12 = 0x400 - v0;    D_800F6608.h14 = 0;    D_800F6608.w18 = 0x1388;    D_800F6608.b1E = 0;}
void func_8001B3C0(Unk80101EC8Record *a0, Unk80101EC8Record *a1) {    D_800A36FA = 0;    D_800F5328.h30[0][0] = 0x64;    D_800F5328.h30[0][1] = 0;    D_800F5328.h30[0][2] = 0x64;    D_800F5328.h30[1][0] = 0x64;    D_800F5328.h30[1][1] = 0;    D_800F5328.h30[1][2] = 0x64;    func_8003F1E4(0);    if (D_800A36F6 != 0) {        a0 = a1;    }    D_800F5328.unk_00.x = a0->unk_180.x;    D_800F5328.unk_00.z = a0->unk_180.z;    {        s32 v = a0->unk_180.y;        D_800F5328.b40 = 0;        D_800F5328.unk_00.y = v;    }}
void func_8001B478(Unk80101EC8Record *obj) {
    Rec44 *s2 = &D_800F5328;
    s32 a2;
    s32 val;
    s32 far;

    func_8003F1E4(0);

    val = (obj->unk_198[0].y + obj->unk_198[1].y) / 2 - obj->unk_180.y;
    far = val >= 0x391;

    if (obj->unk_6A == 0x2A) {
        val = 0x200;
    } else {
        s2->unk_00.x = obj->unk_180.x;
        D_800F5328.unk_00.z = obj->unk_180.z;
        val = obj->unk_180.y;

        if (!far) {
            s32 v = -(obj->unk_1A * 950);
            if (v < 0) {
                v += 0xFFF;
            }
            val += v >> 12;
        }

        {
            s32 diff = val - s2->unk_00.y;
            if (diff < 0) {
                diff += 3;
            }
            a2 = s2->unk_00.y + (diff >> 2);
            s2->unk_00.y = a2;
        }

        if (obj->unk_6A == 0x2A) {
            val = 0x200;
        } else {
            val = (-obj->unk_1D8 - s2->h12) & 0xFFF;

            if (val >= 0x800) {
                val = 0x1000 - val;
            }
            if (val >= 0x400) {
                val = 0x400;
            }

            {
                s32 base_val = obj->other->unk_F4.y;
                s32 result = ratan2(base_val - a2, D_800A387C);
                val = (result * (0x400 - val)) >> 10;
            }
        }
    }

    {
        s16 old = s2->h10;
        s32 diff = val - old;
        if (diff < 0) {
            diff += 7;
        }
        s2->h10 = old + (diff >> 3);
    }
    s2->h14 = 0;

    {
        s16 counter;
        val = -obj->unk_1C8.vy;
        counter = D_800A36FC;

        if (counter != 0) {
            s16 old12 = s2->h12;
            s32 diff = val - old12;
            if (diff < 0) {
                diff += 3;
            }
            {
                s16 cnt = counter - 1;
                s2->h12 = old12 + (diff >> 2);
                D_800A36FC = cnt;
            }

            {
                s32 decay = s2->h1C * 3;
                if (decay < 0) {
                    decay += 3;
                }
                s2->h1C = decay >> 2;
            }
        } else {
            s2->h12 = val;
            s2->h1C = 0;
        }
    }
    s2->w18 = 0;
}

void func_8001B690(s32 arg0, s32 arg1) {
    if (D_800A38BA == 0) {
        return;
    }
    if (D_800A36F6 != arg0) {
        return;
    }
    arg1 &= 0xFFF;
    if (arg1 >= 0x800) {
        arg1 = 0x1000 - arg1;
    }
    if (arg1 >= 0x401) {
        D_800A36FC = 0x19;
        D_800F5328.h1C = 0x800;
    }
}
void func_8001B6F4(void) {
    func_80041688(0, 0);
    func_80041688(1, 0);
    D_800A36FA = 1;
    D_800F6608.b1F = 0;
    D_800F5328.b1F = 0;
    func_8003F1E4(0);
}
/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static u16 D_800A3310;

void func_8001B748(Rec44 *dst, Rec1C *a, Rec1C *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    Unk80101EC8Record *base = &D_80101EC8[D_800A3748];
    s32 zval;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 cur;
    s32 use_high;
    s32 v;
    s32 t;
    s32 dd;
    if (dst->b1F == 0) {
        dst->b1F = 1;
        dst->unk_00.x = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
        dst->unk_00.y = (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
        D_800A3310 = 0;
        zval = (frac * (a->h8)) + (inv_frac * (b->h8));
        dst->h12 = val;
        dst->h10 = 0x80;
        dst->h14 = 0;
        dst->w18 = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        dst->unk_00.z = zval >> 12;
        return;
    }
    {
        s32 sum = base->unk_198[0].y + base->unk_198[1].y;
        s32 avg = ((s32) (sum + (((u32) sum) >> 31))) >> 1;
        if ((avg - base->unk_180.y) < 0xC8) {
            D_800A3310 += 1;
        }
    }
    use_high = ((s16) D_800A3310) >= 0xB;
    cur = dst->unk_00.x;
    t = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
    dx = t - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    dst->unk_00.x = cur + (dx >> 4);
    cur = dst->unk_00.y;
    t = (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
    dy = t - cur;
    if (dy < 0) {
        dy += 0xF;
    }
    dst->unk_00.y = cur + (dy >> 4);
    cur = dst->unk_00.z;
    t = ((frac * (a->h8)) + (inv_frac * (b->h8))) >> 12;
    dz = t - cur;
    if (dz < 0) {
        dz += 0xF;
    }
    dst->unk_00.z = cur + (dz >> 4);
    if (use_high) {
        t = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        t = 0x80 - ((frac_s1 << 8) >> 12);
    }
    dst->h10 = dst->h10 + math_SignExt12Div(t - (s16)dst->h10, 0x10);
    v = math_SignExt12Div(val - (s16)dst->h12, 0x10);
    dst->h14 = 0;
    dst->h12 = dst->h12 + v;
    if (use_high) {
        t = ((frac_s1 * 0x7D0) + (inv_s1 * 0x2EE0)) >> 12;
    } else {
        t = ((frac_s1 * 0x1F4) + (inv_s1 * 0x2EE0)) >> 12;
    }
    cur = dst->w18;
    dd = t - cur;
    if (dd < 0) {
        dd += 0xF;
    }
    dst->w18 = cur + (dd >> 4);
    dst->h30[0][0] = 0x64;
    dst->h30[0][1] = 0;
    dst->h30[0][2] = 0x64;
    dst->h30[1][0] = 0x64;
    dst->h30[1][1] = 0;
    dst->h30[1][2] = 0x64;
}
void func_8001BAE4(s32 *arg0, s32 *arg1, s32 arg2) {
    s32 temp_a2;
    s32 var_s3;
    s32 var_v1;
    s32 var_v0;

    if (D_800A387C < 0x2711) {
        var_s3 = (arg2 / 2) + 0x800;
    } else {
        var_s3 = 0x1000;
    }
    temp_a2 = ratan2(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4),
                             *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8));
    var_v1 = arg2;
    if (arg2 < 0) {
        var_v1 = arg2 + 3;
    }
    var_v0 = Judge[(var_v1 >> 2) & 0xFFF] * 3;
    if (var_v0 < 0) {
        var_v0 += 3;
    }
    func_8001B748(&D_800F6608, arg0, arg1, (s32 *)arg2, var_s3, (0x500 - temp_a2) - (var_v0 >> 2));
}
void func_8001BBD8(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 temp_s0;
    temp_s0 = (D_800A387C < 0x2711) << 0xB;
    func_8001B748(&D_800F5328, arg0, arg1, arg2, temp_s0, -0x200 - ratan2(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4), *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8)));
}
void func_8001BC70(Unk80101EC8Record *arg0, s32 arg1) {
    func_8003F1E4(0);
    D_800F6608.unk_00 = arg0->unk_174;
    D_800F6608.h10 = 0x120;
    D_800F6608.h12 = arg1;
    D_800F6608.h14 = 0;
    D_800F6608.w18 = 0x1162;
}
void func_8001BCF0(u8 *arg0, s32 arg1) {
    typedef struct { s32 x, y, z; } Vec3;
    s32 diff = 0x1000 - arg1;

    func_8003F1E4(0);

    *(Vec3 *)&D_800F6608 = *(Vec3 *)(arg0 + 0xB8);

    D_800F6608.unk_00.y -= 0x44C;

    D_800F6608.h10 = 0x100 - (arg1 * 288) / 4096;

    {
        s32 div4 = arg1 / 4;
        s32 sum = arg1 * 3000 + diff * 8000;
        u16 lhu_val = *(u16 *)(arg0 + 0x1CA);
        s32 val;
        D_800F6608.w18 = sum >> 12;
        val = 0xB00 - div4;
        D_800F6608.h14 = 0;
        D_800F6608.h12 = val - lhu_val;
    }
}
void pad_ClearStateBits(PadState *arg0) {
    arg0->held = 0;
    arg0->pressed = 0;
    arg0->released = 0;
    arg0->unheld = -1;
}
void func_8001BE20(s32 arg0, PadState *arg1);
void func_8001BE20(s32 arg0, PadState *arg1) {
    s32 buf[4];
    s32 i;
    /* One local holds two values (Ruling 11, ordinary-c-judge-decidable.md):
     * (1) arg0 * 16, the bit offset of this player's half of the pad words;
     * (2) 0 or arg0 * 4, the bit offset of this player's colour-config nibble. */
    s32 temp;
    s32 out;

    D_80101EC8[arg0 == 0].unk_34E = D_800A38DC == 2 && D_800A389A == 0 && arg0 == 0 && ((g_pad_state.pressed >> 8) & 1);
    if (D_80101EC8[arg0].unk_06 != 0) {
        func_80055B60(arg0, arg1);
        return;
    }
    *arg1 = g_pad_state;
    temp = arg0 * 16;
    buf[0] = (g_pad_state.held >> temp) & 0xFFFF;
    buf[1] = (g_pad_state.pressed >> temp) & 0xFFFF;
    buf[2] = (g_pad_state.released >> temp) & 0xFFFF;
    buf[3] = (g_pad_state.unheld >> temp) & 0xFFFF;
    if (D_800A38DC == 6) {
        temp = 0;
    } else {
        temp = arg0 * 4;
    }
    for (i = 0; i < 4; i++) {
        if (D_800A38DC == 6 && arg0 != D_800A38A0) {
            out = buf[i] & ~0xF0;
            {
                u8 r = D_800A3912;
                if ((r & 1) && (buf[i] & 0x20) || (r & 2) && (buf[i] & 0x10) || (r & 4) && (buf[i] & 0x40) || (r & 8) && (buf[i] & 0x80)) {
                    out |= 0x20;
                }
            }
            {
                u8 g = D_800A3913;
                if ((g & 1) && (buf[i] & 0x20) || (g & 2) && (buf[i] & 0x10) || (g & 4) && (buf[i] & 0x40) || (g & 8) && (buf[i] & 0x80)) {
                    out |= 0x40;
                }
            }
            {
                u8 b = D_800A3914;
                if ((b & 1) && (buf[i] & 0x20) || (b & 2) && (buf[i] & 0x10) || (b & 4) && (buf[i] & 0x40) || (b & 8) && (buf[i] & 0x80)) {
                    out |= 0x80;
                }
            }
        } else {
            out = buf[i] & ~0xF0;
            {
                u8 r = D_80106A50.color[0] >> temp;
                if ((r & 1) && (buf[i] & 0x20) || (r & 2) && (buf[i] & 0x10) || (r & 4) && (buf[i] & 0x40) || (r & 8) && (buf[i] & 0x80)) {
                    out |= 0x20;
                }
            }
            {
                u8 g = D_80106A50.color[1] >> temp;
                if ((g & 1) && (buf[i] & 0x20) || (g & 2) && (buf[i] & 0x10) || (g & 4) && (buf[i] & 0x40) || (g & 8) && (buf[i] & 0x80)) {
                    out |= 0x40;
                }
            }
            {
                u8 b = D_80106A50.color[2] >> temp;
                if ((b & 1) && (buf[i] & 0x20) || (b & 2) && (buf[i] & 0x10) || (b & 4) && (buf[i] & 0x40) || (b & 8) && (buf[i] & 0x80)) {
                    out |= 0x80;
                }
            }
        }
        if (out & 0x200) {
            out = (out & ~0x200) | 0x8;
        }
        if (out & 0x400) {
            out = (out & ~0x400) | 0x20;
        }
        buf[i] = out;
    }
    arg1->held = buf[0];
    arg1->pressed = buf[1];
    arg1->released = buf[2];
    arg1->unheld = buf[3];
    if (D_800A38DC == 5 && (D_800A381E != 0 || D_800A3816 != 0 || D_800A37E1 != 0 || D_800A38B8 != 0 || D_800A3920 != 0 || D_800A36E8 != 0)) {
        pad_ClearStateBits(arg1);
    }
    if (arg0 == 1 && D_800A38DC != 6) {
        arg1->held = (arg1->held & 0xFFF) | ((arg1->held & 0x8000) >> 2) | ((arg1->held & 0x2000) << 2) | ((arg1->held & 0x4000) >> 2) | ((arg1->held & 0x1000) << 2);
        arg1->pressed = (arg1->pressed & 0xFFF) | ((arg1->pressed & 0x8000) >> 2) | ((arg1->pressed & 0x2000) << 2) | ((arg1->pressed & 0x4000) >> 2) | ((arg1->pressed & 0x1000) << 2);
        arg1->released = (arg1->released & 0xFFF) | ((arg1->released & 0x8000) >> 2) | ((arg1->released & 0x2000) << 2) | ((arg1->released & 0x4000) >> 2) | ((arg1->released & 0x1000) << 2);
        arg1->unheld = (arg1->unheld & 0xFFF) | ((arg1->unheld & 0x8000) >> 2) | ((arg1->unheld & 0x2000) << 2) | ((arg1->unheld & 0x4000) >> 2) | ((arg1->unheld & 0x1000) << 2);
    }
}
void func_8001C444(void) {
    D_80102778.unk_0[1] = 0x800;
    D_80102778.unk_0[0] = 0x800;
    D_80102778.unk_4[0] = 1;
    D_80102778.unk_4[1] = 0x10;
    D_80102778.unk_C = 0xC;
    D_80102778.unk_4[2] = 0;
    D_80102778.unk_4[3] = 0;
    D_80102778.unk_4[5] = 0;
    D_80102778.unk_4[4] = 0;
    D_80102778.unk_D = 4;
    D_80102778.unk_E = 0;
    D_80102778.unk_F = 0;
}
void func_8001C4C0(void) {
    u16 v = D_80101EC8[0].unk_6A;
    if (v == 0x32 || v == 0x11) {
        func_800218C8(0);
        {
            s32 v0 = func_80021974(0);
            D_80101EC8[0].unk_5E = 0;
            func_80021A98(0, v0, 0);
        }
    }
}
void func_8001C51C(void) {
    s32 v0;
    func_8001C4C0();
    if (D_800A38DC == 3 && D_800A3728 != 0) {
        func_80022580(1, (s8)D_80102778.unk_4[5], (s8)D_80102778.unk_4[1], (s8)D_80102778.unk_4[3], 0);
    } else {
        func_80022580(1, (s8)D_80102778.unk_4[5], (s8)D_80102778.unk_4[1], (s8)D_80102778.unk_4[3], 1);
    }
    func_80022F34();
    func_800218C8(1);
    v0 = func_80021974(1);
    D_80101EC8[1].unk_5E = 0;
    func_80021A98(1, v0, 0);
    D_800A382E = 0;
    D_800A3748 = -1;
    func_80030524();
    func_80030D04();
    func_8001B294(&D_80101EC8[0], &D_80101EC8[1]);
    func_800392C8();
    func_80021280(1);
}
/* Initialise record 0 of D_80101EC8 (Unk80101EC8Record). */
void func_8001C624(void) {
    Unk80101EC8Record *e = D_80101EC8;
    s32 local[3];
    s32 x, y, z;

    func_80021D10(0, &e->unk_D8.x, (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    e->unk_E8.x = 0;
    x = e->unk_D8.x;
    y = e->unk_D8.y;
    z = e->unk_D8.z;
    e->unk_E8.y = -0x384;
    e->unk_E8.z = 0;
    e->unk_F4.x = x;
    e->unk_F4.y = y - 0x384;
    e->unk_F4.z = z;
    e->unk_B8.vx = x;
    e->unk_B8.vy = y;
    e->unk_B8.vz = z;
    e->unk_C8 = e->unk_B8;
    e->unk_1F8 = e->unk_E8;
    e->unk_104.vx = 0;
    e->unk_104.vy = 0;
    e->unk_104.vz = 0;
    e->unk_24C = e->unk_104;
    /* FAKE: self-assigning round-trip through `local`, which is address-taken by
     * the func_80021D10 call above.  The target genuinely contains these
     * self-copy stores (pre-restructure-2026-10-03:asm/6CAC.s:5101-5119: lw $v0,0x10($sp) / sw $v0,0x10($sp),
     * lw $v1,0x18($sp) / sw $v1,0x18($sp)); this is the libgte setVector
     * comma-assign idiom, adjusting only the middle component. */
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    e->unk_114[0].vx = 0;
    e->unk_114[0].vy = 0;
    e->unk_114[0].vz = 0;
    e->unk_114[1].vx = 0;
    e->unk_114[1].vy = 0;
    e->unk_114[1].vz = 0;
    e->unk_134.vx = 0;
    e->unk_134.vy = 0;
    e->unk_134.vz = 0;
    e->unk_144 = 0;
    e->unk_14C = 0;
    e->unk_150 = 0;
    e->unk_152 = 0;
    e->unk_14E = 0;
    e->unk_148 = e->unk_B8.vy;
    func_8003FFE0(0);
}
void func_8001C820(void) {
    s16 *s0 = &D_80101EC8[0].unk_0A;
    s32 a0;
    if (D_8008D9EC[*s0] != 0) {
        if (D_800A37A0 == 1) return;
    }
    if (D_800A38DC != 0) return;
    if (D_800A3712 != 0) return;
    a0 = 0x56;
    if (D_800A3680 != D_800A3671) {
        if (rand(0x56) & 1) {
            a0 = 0x57;
        } else {
            a0 = 0x58;
        }
    }
    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));
}
void func_8001C8DC(void);

void func_8001C8DC(void) {
    u8 prev;
    s32 snd;

    if (D_80101EC8[0].unk_96 != 0 || D_80101EC8[1].unk_96 != 0) {
        D_800A382E++;
    } else if (D_800A38DC == 0 && D_800A385C != 0 && D_80101EC8[0].unk_B2 == 1) {
        func_8003B56C(3);
    }
    if (D_800A382E < 0x3D) goto end;

    switch (D_800A38DC) {
    case 0:
        if (D_80101EC8[0].unk_96 != 0) break;
        if (D_800A3680 != 0) {
            if (--D_800A3680 == 0) {
                func_8005B58C();
                if (D_800A37C6 != 0) {
                    D_800A37C6 = 0;
                    break;
                }
                if (D_800A3894 != 0) {
                    func_8003B484(D_800A3894 + 6);
                    if (D_800A3836 != 0xFF) {
                        func_8003B534(2);
                    } else {
                        func_8003B534(5);
                    }
                } else {
                    func_8003B5A4();
                }
            } else {
                func_8001C51C();
                func_8001C820();
            }
            goto end;
        }
        if (D_800A385C == 0) break;
        func_8003B56C(2);
        goto end;
    case 3:
        if (D_80101EC8[0].unk_96 != 0) break;
        if (func_80033DF4() == 0) goto end;
        D_800A390D = 1;
        gpu_ResetGraphMode1();
        func_80040510(1, D_800A38DE, 0);
        prev = D_800A38E0;
        switch (D_800A38E2) {
        case 0x1F:
            D_800A38E0 = 1;
            func_8004659C(1);
            break;
        case 0x33:
            D_800A38E0 = 2;
            func_8004659C(2);
            break;
        case 0x51:
            D_800A38E0 = 3;
            func_8004659C(3);
            break;
        }
        if (D_800A3728 = prev != D_800A38E0) {
            func_8001C624();
            snd = func_80021904(0);
            D_80101EC8[0].unk_5E = 0;
            func_80021A98(0, (MoveScript *)snd, 0);
        }
        func_8001C51C();
        if (D_800A384C < 4) {
            func_80041BF4(D_800A38EC, D_800A38ED, D_800A38EE);
        }
        goto end;
    case 2:
        {
            s16 t;
            u8 *p;
            if ((t = D_80101EC8[0].unk_96) == 0 || D_80101EC8[1].unk_96 == 0) {
                /* FAKE: second handle to D_800A37D2 (pointer-alias-fake-exception, owner Q63): the
                 * target sets the pair's base in its own register before the index (v0 base, v1
                 * index); the index written without p computes the index first and loses that
                 * seat. */
                p = &D_800A37D2;
                /* FAKE: indexes past D_800A37D2 into D_800A37D3 (owner Q63, this byte pair only):
                 * the target also reaches each byte by its own symbol, which no single array or
                 * struct gives. */
                p[t != 0]++;
            }
        }
        D_800A3670 = 1;
        D_800A38DF = func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);
        D_800A3834 = 0;
        goto end;
    case 4:
    case 6:
        {
            s16 t;
            u8 *p;
            if ((t = D_80101EC8[0].unk_96) == 0 || D_80101EC8[1].unk_96 == 0) {
                /* FAKE: second handle, as above (owner Q63). */
                p = &D_800A37D2;
                /* FAKE: indexes past D_800A37D2 into D_800A37D3, as above (owner Q63). */
                p[t != 0]++;
            }
        }
        break;
    case 5:
        goto end;
    }
    D_800A3834 = 4;
end:
    if (D_800A37D2 >= 100) {
        D_800A37D2 = 99;
    }
    if (D_800A37D3 >= 100) {
        D_800A37D3 = 99;
    }
}


void func_8001CD68(s16 *arg0) {
    s32 val = D_800A3858;

    if (val > 0x2BF1F) {
        *(s16 *)arg0 = 99;
        *((u8 *)arg0 + 2) = 59;
        *((u8 *)arg0 + 3) = 99;
        return;
    }
    {
        s32 minutes = val / 1800;
        s32 seconds = val / 30 - minutes * 60;
        *((u8 *)arg0 + 2) = seconds;
        {
            s32 centiseconds = (D_800A3858 % 30) * 100 / 30;
            *(s16 *)arg0 = minutes;
            *((u8 *)arg0 + 3) = centiseconds;
        }
    }
}
/* P1/P2 round scores and tiebreakers (per-file declarations: owner rulings Q21-Q25,
 * .claude/rules/no-new-park-categories.md aggregate-merge exception). Declared here
 * as [2] arrays because func_8001CE60 indexes them by player (D_800A38B0).
 * src/code6cac_b.c:128 declares the same bytes as single u8s for
 * func_800340A0. The mismatch is kept because no single declaration compiles
 * both files with a counting spelling (Q22/Q23 set-asides excluded). Under
 * scalars, func_8001CE60's player-indexed accesses are not produced (conditional
 * selects compile to separate direct accesses; a pointer pun is refused). Under
 * an aggregate, every measured counting spelling of func_800340A0 misses the
 * shipped code: constant subscripts put element 0 behind a base register, and
 * the index-variable and regrouped-condition spellings that avoid that miss its
 * round-result stores or compares (dummy-index and pointer-alias spellings that
 * match are refused/set aside, Q22/Q23). */
extern u8 D_800A3898[2];
extern u8 D_800A38AA[2];
void func_8001CE60(void) {
    u8 buf[4]; /* func_8001CD68's clock record: s16 minutes, u8 seconds, u8 centiseconds */

    if (D_800A38DC == 1) {
        D_800A38B4 += func_8005E51C(D_800A3783, D_800A38B4, 1) / 4 * 4;
    } else if (D_800A38DC == 3) {
        if (D_80101EC8[0].unk_96 == 0 && (D_80101EC8[1].unk_96 == 0 || D_800A38E2 != 100)) {
            D_800A3858++;
            if (D_800A3858 > 0x2BF20) {
                D_800A3858 = 0x2BF20;
            }
        }
        func_8001CD68((s16 *)buf);
        D_800A38B4 += func_8005D814((s16 *)buf, D_800A38E2, D_800A38B4, 1) / 4 * 4;
    } else if ((D_800A38DC == 2 && D_800A389A == 1) || D_800A38DC == 4) {
        D_800A38B4 += func_8005E098(D_800A37D2, D_800A37D3, D_800A38B4, 1) / 4 * 4;
    } else if (D_800A38DC == 5) {
        /* temp holds two values (ordinary-c-judge-decidable Ruling 11, with
         * its per-branch constants clause): the announcement length in
         * frames (0x50 after a draw, 0x64 otherwise), then the match clock's
         * frames left for the on-screen timer. */
        s32 temp;

        if (D_800A381E != 0) {
            if (D_800A381E == 0x2D) {
                func_8005C650(0xA3, 0x7F, 0x7F);
            }
            if (++D_800A381E == 0x50) {
                D_800A381E = 0;
                func_800340A0();
                D_800A36E8 = 1;
            }
        } else if (D_800A3816 != 0) {
            if (++D_800A3816 == 0x46) {
                func_8005C650(0x9D, 0x7F, 0x7F);
            } else if (D_800A3816 == 0x82) {
                D_800A3816 = 0;
                D_800A3670 = 1;
                D_800A3834 = 0;
            }
        } else if (D_800A37E1 != 0) {
            D_800A38B4 += func_8005FA98(2, D_800A38B4, 1) / 4 * 4;
            if (++D_800A37E1 == 0x3C) {
                D_800A37E1 = 0;
                if (D_800A38B0 != 2) {
                    func_8005C650(0xA0, 0x7F, 0x7F);
                    D_800A38AA[D_800A38B0]++;
                    D_800A38B8 = 1;
                } else {
                    D_800A3670 = 1;
                    D_800A3834 = 0;
                }
            }
        } else if (D_800A38B8 != 0) {
            if (++D_800A38B8 == 0x3C) {
                D_800A38B8 = 0;
                if (D_800A38AA[D_800A38B0] == 2) {
                    D_800A3898[D_800A38B0 == 0]++;
                    D_800A38AA[D_800A38B0] = 0;
                    D_800A3920 = 0x5A;
                } else {
                    D_800A3670 = 1;
                    D_800A3834 = 0;
                }
            }
        } else if (D_800A3920 != 0) {
            D_800A38B4 += func_8005FA98(1, D_800A38B4, 1) / 4 * 4;
            if (++D_800A3920 == 0x1E) {
                func_8005C650(0xA1, 0x7F, 0x7F);
            }
            if (D_800A3920 == 0x69) {
                func_8005C650(0xA2, 0x7F, 0x7F);
            }
            if (D_800A3920 == 0x5A || D_800A3920 == 0xB4) {
                D_800A3920 = 0;
                if (D_800A3898[D_800A38B0 == 0] == D_800A37F8) {
                    func_800340A0();
                    D_800A36E8 = 1;
                } else {
                    D_800A3670 = 1;
                    D_800A3834 = 0;
                }
            }
        } else if (D_800A391E != 0) {
            if (--D_800A391E == 0) {
                func_8005C650(0x9C, 0x7F, 0x7F);
            }
        } else if (D_800A36E8 != 0) {
            if (D_800A377C[D_800A3874 - 1] == 2) {
                if (D_800A36E8 == 1) {
                    func_8005C650(0xA4, 0x7F, 0x7F);
                }
                temp = 0x50;
            } else {
                if (D_800A36E8 == 1) {
                    func_8005C650(0xA6, 0x7F, 0x7F);
                }
                if (D_800A36E8 == 0x14) {
                    func_8005C650(D_8008D9EC[D_80101EC8[D_800A377C[D_800A3874 - 1]].unk_0A] ? 0xA8 : 0xA7, 0x7F, 0x7F);
                }
                temp = 0x64;
            }
            if (++D_800A36E8 == temp) {
                D_800A36E8 = 0;
                func_800342A0();
            }
        } else if (D_80101EC8[0].unk_96 != 0 && D_80101EC8[1].unk_96 != 0) {
            D_800A3816 = 1;
        } else if (D_80101EC8[1].unk_96 != 0) {
            D_800A38B0 = 1;
            D_800A3920 = 1;
            D_800A3898[D_800A38B0 ^ 1]++;
        } else if (D_80101EC8[0].unk_96 != 0) {
            D_800A38B0 = 0;
            D_800A3920 = 1;
            D_800A3898[D_800A38B0 ^ 1]++;
        } else if ((u16)D_80101EC8->unk_6A == 6 || (u16)D_80101EC8[1].unk_6A == 6) {
            D_800A3816 = 0x3C;
        } else if (D_80101EC8[0].unk_B1 == 2) {
            u16 id;

            func_8005C650(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[0].unk_6A;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 || id == 0x1A || id == 0x18) {
                D_800A38B0 = 0;
            } else {
                D_800A38B0 = 2;
            }
        } else if (D_80101EC8[1].unk_B1 == 2) {
            u16 id;

            func_8005C650(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[1].unk_6A;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 || id == 0x1A || id == 0x18) {
                D_800A38B0 = 1;
            } else {
                D_800A38B0 = 2;
            }
        } else if (D_800A36CC != 0) {
            D_800A38F4++;
            if (D_800A38F4 >= D_800A36CC * 30) {
                func_8005C650(0x9E, 0x7F, 0x7F);
                D_800A381E = 1;
            }
        }
        if (D_800A36CC != 0) {
            temp = D_800A36CC * 30 - D_800A38F4;
            buf[2] = temp / 30;
            buf[3] = temp % 30 * 100 / 30;
        }
        D_800A38B4 += func_8005F1C8(buf, D_800A3898[0] | (D_800A38AA[0] << 8) | (D_800A3898[1] << 4) | (D_800A38AA[1] << 12), D_800A38B4, 1) / 4 * 4;
    }
}
extern s8 D_800A30FC;
extern s8 D_800A30FD;
extern s32 D_800FF6A8;
void func_8001D790(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_ResetGraphMode1();

    if (D_800A36A4 != D_800A390E
        || D_8008E5A8[(s8)D_80102778.unk_4[0]] != D_800A30FC
        || D_8008E5A8[(s8)D_80102778.unk_4[1]] != D_800A30FD) {
        /* FAKE: block-local address cache for D_80102778.unk_4[0]. Every &-free spelling
         * re-materializes the symbol at both body reads instead of holding it in
         * a callee-save register across func_8005BA8C.
         * GCC keeps an address pseudo only for a pointer local dereferenced as a
         * plain scalar; no expression-level form produces one. Same construct for
         * this same global in func_8003B2C8/func_8003B328. */
        u8 *p = &D_80102778.unk_4[0];

        func_80020D38();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        snd_CloseListedVabs();

        s1 = func_8005BA8C(s2, D_800A36A4, D_8008E5A8[(s8)*p], D_8008E5A8[(s8)D_80102778.unk_4[1]]);

        D_800A390E = D_800A36A4;
        D_800A30FC = D_8008E5A8[(s8)*p];
        D_800A30FD = D_8008E5A8[(s8)D_80102778.unk_4[1]];

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
void func_8001D904(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_ResetGraphMode1();
    func_80020D38();
    func_8005B9C4();
    s1 = func_8005B9FC((s32)0x80190800);
    if (s1 >= 0xE81) {
        sys_Panic();
    }
    s0 = &MotDataBaseAddress;
    memcpy(s0, (s32)0x80190800, s1);
    snd_VabFakeOpen9((s32)s0 - s2);
}
void func_8001D998(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_ResetGraphMode1();
    func_80020D38();
    func_8005B868();
    s1 = func_8005B8B8((s32)0x80190800);
    if (s1 >= 0x1B19) {
        sys_Panic();
    }
    s0 = &MotDataBaseAddress;
    memcpy(s0, (s32)0x80190800, s1);
    snd_VabFakeOpen8And4((s32)s0 - s2);
}
void func_8001DA2C(void) {
    func_8005B5AC();
    func_8005BF3C();
    if (D_800A38DC == 5) {
        func_8005B9C4();
    }
    if (D_800A38DC == 3) {
        func_8005B868();
    }
}
void func_8001DA8C(void) {
    snd_SerialMixOn();
    if (file_GetFlag2()) {
        return;
    }
    switch (D_800A38DC) {
        case 0:
        case 1:
        default:
            break;
        case 4:
            func_80037110((&D_8008D518)[D_800A36A4]);
            break;
        case 3:
            if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
                func_80037110(9);
            } else {
                func_80037110(8);
            }
            break;
        case 2:
            if (D_800A389A != 0) {
                func_80037110(0xA);
            } else {
                func_80037110(0xB);
            }
            break;
        case 5:
            break;
    }
    func_800371E8(1);
}
s32 func_8001DB58(void) {
    s32 v = D_800A38DC;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return file_GetFlag2();
    }
    return 1;
}
void func_8001DB9C(void) {
    seq_Start(D_8008D9EC[D_80101EC8[0].unk_0A] < 1, (s32)0x80190800);
    D_800A38C4[1] = 0xFFFF;
}
void func_8001DBE4(void) {
    s32 i;

    if (g_disp_enable != DISP_ACTIVE) {
        return;
    }
    func_8003AA78();
    if (!(D_800A38F8 > D_800A37A0)) {
        do {
            func_8003AA48();
            func_800174F4();
            VSync(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            func_8003AA48();
            i += 1;
            func_800174F4();
            VSync(2);
        } while (i < 15);
    }
    func_8003AAB0();
    gpu_InitDisplay();
    gpu_SetDispMaskOn();
}
extern void func_80020E74(s32, s32, s32, s32);
extern void func_80021210(void);
extern void func_80021280(s32);
extern void func_80022F34(void);
extern void func_800218C8(s32);
extern s32 func_80021974(s32);
extern s32 func_80021904(s32);
extern s32 func_800219E4(s32);
extern void func_8001B294(Unk80101EC8Record *, Unk80101EC8Record *);
extern void func_8001B3C0(Unk80101EC8Record *, Unk80101EC8Record *);
void func_8001DCB0(void) {
    s32 i;
    s32 addr;

    func_8005B5AC();
    if (g_disp_enable != DISP_ACTIVE) {
        gpu_InitDisplay();
        gpu_SetDispMaskOn();
    }
    func_800174F4();
    gpu_ResetGraphMode1();
    func_8003E22C();
    func_8003043C();
    func_80032040();
    func_8003F218(D_800A38BA);
    SetGeomScreen(math_FovToScreenDist(D_800A38BA != 0 ? 0x50 : 0x2D));
    for (i = 0; i < 2; i++) {
        if (D_800A38DC != 0) {
            player_SetCharId(0, 0);
        }
        func_80022580(i, (s8)D_80102778.unk_4[4 + i], (s8)D_80102778.unk_4[i], (s8)D_80102778.unk_4[2 + i], 0);
        if (D_800A38BA != 0 && D_800A36F6 == i) {
            func_8003E164(i == 0);
        }
    }
    func_8003FFE0(0);
    func_8003FFE0(1);
    if (D_800A3670 == 0) {
        func_8004939C();
        addr = (s32)0x80190800;
        func_80020D38();
        for (i = 0; i < 2; i++) {
            if (D_800A38BA != 0 && D_800A36F6 == i) {
                func_80040510(i, D_8008D578[(s8)D_80102778.unk_4[i]], addr);
                func_80048AD0(i);
            } else if (D_800A38DC == 3 && i == 1) {
                func_80040510(1, D_800A38DE, 0);
            } else {
                func_80040510(i, D_8008D578[(s8)D_80102778.unk_4[i]], addr);
            }
            func_800493E4(D_80101EC8[i].unk_12);
            if (D_800A38DC != 3 || i != 1) {
                if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
                    func_800494D4(i, D_8008E6A4[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
                } else {
                    func_800494D4(i, D_8008E5CC[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
                }
            }
            if (D_80101EC8[i].unk_14 != -1) {
                func_800493E4(D_8008EB80[D_80101EC8[i].unk_14]);
                if (D_80101EC8[i].unk_14 == 14) {
                    func_800493E4(D_8008EB80[14] + 3);
                }
            }
        }
        func_80049584(addr);
        func_80041688(0, 0);
        func_80041688(1, 0);
        if (D_800A38DC == 0 && D_800A3712 == 0) {
            func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
        } else if (D_800A38DC == 3) {
            func_80041BF4(D_800A38EC, D_800A38ED, D_800A38EE);
        } else if (D_800A38DC == 2) {
            u8 *p = D_800A3100[D_8008D9EC[D_80101EC8[0].unk_0A]];
            if (D_800A389A == 0) {
                func_80041BF4(p[0], p[1], p[2]);
            }
        }
        func_8001D790();
        if (D_800A38DC == 5) {
            func_8001D904();
        }
        if (D_800A38DC == 3) {
            func_8001D998();
            func_8001DB9C();
        }
        func_80020E74(D_8008D538[(s8)D_80102778.unk_4[0]], (s8)D_80102778.unk_4[2],
                      D_8008D538[(s8)D_80102778.unk_4[1]], (s8)D_80102778.unk_4[3]);
    } else if (D_800A38DC == 5) {
        D_800A391E = 1;
    }
    func_80021210();
    func_80021280(0);
    func_80021280(1);
    func_80022F34();
    if (D_800A38DC == 2 || D_800A38DC == 5) {
        if (D_800A3670 != 0) {
            func_800218C8(0);
            {
                s32 v = func_80021974(0);
                D_80101EC8[0].unk_5E = 0;
                func_80021A98(0, (MoveScript *)v, 0);
            }
            if (D_800A38DC == 2 && D_800A389A == 0) {
                s32 v = func_80021904(1);
                D_80101EC8[1].unk_5E = 0;
                func_80021A98(1, (MoveScript *)v, 0);
            } else {
                s32 v;
                func_800218C8(1);
                v = func_80021974(1);
                D_80101EC8[1].unk_5E = 0;
                func_80021A98(1, (MoveScript *)v, 0);
            }
        } else {
            func_800218C8(0);
            func_800218C8(1);
            {
                s32 v = func_800219E4(0);
                D_80101EC8[0].unk_5E = 1;
                func_80021A98(0, (MoveScript *)v, 1);
            }
            {
                s32 v = func_800219E4(1);
                D_80101EC8[1].unk_5E = 1;
                func_80021A98(1, (MoveScript *)v, 1);
            }
        }
    } else {
        func_800218C8(0);
        func_800218C8(1);
        {
            s32 v = func_80021974(0);
            D_80101EC8[0].unk_5E = 0;
            func_80021A98(0, (MoveScript *)v, 0);
        }
        {
            s32 v = func_80021974(1);
            D_80101EC8[1].unk_5E = 0;
            func_80021A98(1, (MoveScript *)v, 0);
        }
    }
    D_800A382E = 0;
    D_800A3748 = -1;
    func_8001B294(&D_80101EC8[0], &D_80101EC8[1]);
    if (D_800A38BA != 0) {
        func_8001B3C0(&D_80101EC8[0], &D_80101EC8[1]);
    }
    func_800392C8();
    game_Cleanup();
    func_8001DBE4();
    g_disp_enable = DISP_DISABLED;
    g_disp_fade = 0;
    eff_Init();
    D_800A3670 = 0;
    D_800A3834 = 1;
    func_8001C820();
    func_8001DA8C();
    func_80033510();
    func_8005BE84(D_800A36A4);
    if (D_800A38DC == 6) {
        rng_SetSeed(D_800A3904);
    }
}

void func_8001E404(void) {
    /* FAKE: frame layout -- unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out): reconstructs the original frame's 8-byte allocated-but-untouched leading region (outgoing-args partition 24 vs 16); SOTN precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. Sanctioned for func_8001E404/func_8001E6E4/func_8003CF84 ONLY. */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *s2;

    if (D_800A38BA != 0) {
        s32 v3 = D_800A36FA;
        if (v3 == 1) {
            if (D_80101EC8[0].unk_96 != 0 || D_80101EC8[1].unk_96 != 0) {
                D_800A36FA = 2;
            }
        }
        if (D_800A36FA == 2) goto s2_default;
        if ((u16)D_80101EC8[0].unk_6A == 0x11 || (u16)D_80101EC8[1].unk_6A == 0x11) {
            s2 = &D_800F6608;
            D_800A36FA = 1;
        } else {
            s2 = &D_800F5328;
            D_800A36FA = 0;
        }
        goto done_s2;
    s2_default:
        s2 = &D_800F6608;
    done_s2:

        func_8003F218(D_800A36FA < 1);

        {
            s32 fov = 0x2D;
            if (D_800A36FA == 0) {
                fov = 0x50;
            }
            SetGeomScreen(math_FovToScreenDist(fov));
        }

        if (D_800A36FA == 0) {
            func_80041688(D_800A36F6, 1);
            func_80041688(D_800A36F6 == 0, 0);
        } else {
            func_80041688(0, 0);
            func_80041688(1, 0);
        }
        goto common_tail;
    }
    s2 = &D_800F6608;
common_tail:

    if (D_800A3834 == 1) {
        local.unk_00.x = s2->unk_00.x + D_800FF5C8;
        local.unk_00.y = s2->unk_00.y + D_800FF5CC;
        local.unk_00.z = s2->unk_00.z + D_800FF5D0;
        local.h10 = s2->h10 + (u16)D_800FF5D8;
        local.h12 = s2->h12 + (u16)D_800FF5DA;
        local.h14 = s2->h14 + (u16)D_800FF5DC;
        local.w18 = s2->w18 + D_800FF5E0;
    } else {
        local = *s2;
    }

    func_80046BF4(&local.unk_00.x, &local.h10, local.w18);
    {
        s32 *p20 = &s2->w20;
        func_8001A538(&local, p20);
        func_80061064(&local.h10, p20);
    }
    func_8003F3D4(s2->h30[0]);
    func_8003F3D4(s2->h30[1]);
    D_800A36B4 = (s32)s2;
}
void func_8001E6E4(s32 arg0) {
    /* FAKE: frame layout -- unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out): reconstructs the original frame's 8-byte allocated-but-untouched leading region (outgoing-args partition 24 vs 16); SOTN precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. Sanctioned for func_8001E404/func_8001E6E4/func_8003CF84 ONLY. */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *s2;

    s2 = &D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = &D_800F6608;
    }

    local.unk_00.x = s2->unk_00.x + D_800FF5C8;
    local.unk_00.y = s2->unk_00.y + D_800FF5CC;
    local.unk_00.z = s2->unk_00.z + D_800FF5D0;
    local.h10 = s2->h10 + (u16)D_800FF5D8;
    local.h12 = s2->h12 + (u16)D_800FF5DA;
    local.h14 = s2->h14 + (u16)D_800FF5DC;

    local.w18 = s2->w18 + D_800FF5E0;
    func_80046BF4(&local.unk_00.x, &local.h10, local.w18);

    {
        s32 *p20 = &s2->w20;
        func_8001A538(&local, p20);
        func_80061064(&local.h10, p20);
    }

    D_800A36B4 = (s32)s2;
}
void func_8001E800(void) {
    s32 v = D_800A36F6;
    Unk80101EC8Record *ptr = &D_80101EC8[v];
    s32 a1;
    if (ptr->unk_62 & 1) {
        a1 = ptr->unk_0E;
    } else {
        a1 = -1;
    }
    {
        u32 flags = ptr->unk_62 & 4;
        func_80048BA4(D_800F5328.h1C, a1, flags > 0);
    }
}
void func_8001E878(void) {
    PadState buf;
    s32 v0;
    v0 = (s32)camera_GetBoneData();
    D_800A3778 = v0;
    func_8001A820(&D_80101EC8[0].unk_168, &D_80101EC8[1].unk_168, &D_80101EC8[0], &D_80101EC8[1]);
    if (D_800A38BA != 0) {
        func_8001B478(&D_80101EC8[D_800A36F6]);
    }
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE20(0, &buf);
    func_80023F08(0, &buf);
    func_8001BE20(1, &buf);
    func_80023F08(1, &buf);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    if (D_800A38BA != 0 && D_800A36FA == 0) {
        func_8001E800();
    } else {
        func_8003E6A0(D_80101EC8[0].unk_F4.x, D_80101EC8[0].unk_F4.z);
        func_8003E6A0(D_80101EC8[1].unk_F4.x, D_80101EC8[1].unk_F4.z);
    }
    func_80046DA8((D_800A3690 ^ 1) != 0);
    func_8001CE60();
    func_800335D8();
    func_8001C8DC();
}
void func_8001EA04(void) {
    u8 v;
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    v = D_800A38D4;
    D_80101EC8[1].unk_31A = 0;
    D_80101EC8[0].unk_31A = 0;
    D_800A37B8 = 0;
    D_800A3929 = 0;
    D_800A3834 = 0xD;
    D_800A3804 = v < 1;
    D_800A3817 = v < 1;
}
void func_8001EA84(void) {
    PadState sp10;
    s16 buf[4];
    s32 ret;
    Unk80101EC8Record *base;

    D_800A37B8 += 1;
    D_800A3778 = (s32)camera_GetBoneData();
    base = &D_80101EC8[0];
    if (D_800A3748 == 0) {
        base++;
    }
    func_8001BC70(base, D_800A37B8 << 3);
    func_8001E404();
    func_80039320();
    func_8002006C();
    pad_ClearStateBits(&sp10);
    func_80023F08(0, &sp10);
    func_80023F08(1, &sp10);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    func_80046DA8(1);
    func_800335D8();
    if (D_800A38DC == 3) {
        func_8001CD68(buf);
        D_800A38B4 = D_800A38B4 + ((func_8005D814(buf, D_800A38E2, D_800A38B4, 1) / 4) * 4);
    }
    if (D_800A3929 == 0) {
        D_800A38B4 = D_800A38B4 + ((func_8005C8A8(1, D_800A3817, D_800A38B4, 0) / 4) * 4);
        if ((g_pad_state.pressed & 0x10001000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 != D_800A3804) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((g_pad_state.pressed & 0x40004000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = D_800A3804;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }
        if ((g_pad_state.pressed & 0x400040) != 0) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
            if (D_800A3817 != 0) return;
            D_80101EC8[D_800A3748 == 0].unk_B3 = 0;
            if (D_800A38DC == 3) {
                D_800A3858 = D_800A3858 + 0x384;
                if (D_800A3858 > 0x2BF20) {
                    D_800A3858 = 0x2BF20;
                }
            }
        }
        return;
    }
    if (D_800A3817 == 0) {
        ret = func_8005FA98(0, D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + ((ret / 4) * 4);
    }
    D_800A3929 = D_800A3929 + 1;
    if (((u8)D_800A3929) < 0x3C) return;
    if (D_800A3817 == 0) {
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF = func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);
        if (D_80101EC8[1].unk_06 != 0) {
            func_800550E8(1);
        }
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        func_800372C0();
        func_8001DA2C();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if (D_800A3817 == 2) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
void func_8001EEB4(void) {
    s8 idx = D_800A3748;
    Unk80101EC8Record *entry = &D_80101EC8[idx];
    u16 a1 = entry->unk_6A;

    if (a1 != 0xA && entry->unk_72 == 0 &&
        a1 != 0x17 && a1 != 0x18 && entry->unk_96 == 0) {
        func_800218C8(D_800A3748);
        {
            s32 ret = func_80021A3C(D_800A3748, entry->unk_0A);
            s32 idx2 = D_800A3748;
            entry->unk_5E = 1;
            func_80021A98(idx2, ret, 1);
        }
        entry->unk_26C = 1;
    }

    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x11;
}
void func_8001EFA0(void) {
    PadState sp10;
    s16 var_v0;

    D_800A37B8 += 1;
    D_800A3778 = (s32)camera_GetBoneData();
    func_8001BCF0((u8 *)&D_80101EC8[D_800A3748], (D_800A37B8 << 12) / 105);
    func_8001E404();
    func_80039320();
    func_8002006C();
    pad_ClearStateBits(&sp10);
    func_80023F08(0, &sp10);
    func_80023F08(1, &sp10);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    func_80046DA8(1);
    func_800335D8();

    if (D_80101EC8[D_800A3748].unk_96 != 0 && D_800A38DC == 1) {
        D_800A37B8 = 0x69;
    }

    if (D_800A37B8 >= 0x69 || (g_pad_state.pressed & 0x400040)) {
        switch (D_800A38DC) {
        case 4:
            var_v0 = 0xC;
            break;
        case 1:
            if (D_800A3748 == 0) {
                func_8001DA2C();
                g_disp_enable = 2;
                func_80033BC0();
                return;
            }
            var_v0 = 0xC;
            break;
        case 6:
            var_v0 = 0xC;
            break;
        default:
            func_8001DA2C();
            var_v0 = 2;
            break;
        }
        D_800A3834 = var_v0;
    }
}
void func_8001F1C4(Unk80101EC8Record *arg0, u8 *arg1, MotionFrame *arg2, MotionFrame *arg3) {
    s16 temp_v1;
    if (!(*(u8 *)(arg1 + 0x18) & 0x80)) {
        func_80027334(arg2);
        func_80027334(arg3);
    }
    func_8002F770(&arg2->unk_0C[0x15], *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    func_8002F770(&arg3->unk_0C[0x15], *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    temp_v1 = arg0->unk_0C;
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        arg2->unk_0C[0x39] += *(s8 *)(arg1 + 0x16) * 4;
        arg3->unk_0C[0x39] += *(s8 *)(arg1 + 0x16) * 4;
    }
    if ((u32)((u16)arg0->unk_0E - 6) < 2U) {
        arg2->unk_0C[0x33] += *(s8 *)(arg1 + 0x16) * 4;
        arg3->unk_0C[0x33] += *(s8 *)(arg1 + 0x16) * 4;
    }
}
/* Steers two bone-angle sets (a, b) toward obj's partner (obj->other):
 * while obj+0x6A is 0x15/0x25, a clamped heading (obj+0x1D8 - obj+0x1CA) and
 * a clamped elevation from ratan2(ground distance, height delta) are eased
 * 1/8 of the wrapped difference per call into obj+0x1E6/0x1E8 and applied
 * to both sets via func_8002F770 (otherwise both ease back toward 0; state
 * 0x1F holds elevation at 0x100). The ground distance is the D_8008D118
 * byte-LUT integer sqrt with the GTE leading-zero count for large inputs
 * (the func_8002E838 idiom; one PsyQ gte_Lzc per square root, written as
 * its inline_o.h statements like func_8001A820's). Also sets or
 * eases a twist in obj+0x1EA (states 0x1D/0xE with obj+0x8C != 0, and
 * obj+0xE in 6..7 with obj+0x6A == 2), and adds random jitter to both sets
 * when obj+0x26E is set and obj+0x96 == 0. The x target passed to
 * func_8002F770 is 0 in every state. */
void func_8001F2E4(Unk80101EC8Record *obj, MotionFrame *a, MotionFrame *b) {
    s32 lzc_out;
    s32 lzc_out2;
    s32 tgt_z;
    /* temp2: two values -- the clamped elevation target that obj+0x1E8 eases
     * toward (0x100 in state 0x1F, 0 outside states 0x15/0x25), then the
     * clamped twist target that obj+0x1EA eases toward (obj+0xE in 6..7).
     * Ruling 11, (E)(i) generic name. */
    s32 temp2;
    /* FAKE: constant-holder (named-local-fake-exception.md) -- tgt_x is 0 on
     * every path; cse works per extended basic block, so the three arm
     * writes reach both func_8002F770 calls through the join unfolded, and
     * global.c seats the pseudo, live across the first call, in $s2 as the
     * target does (`addu $s2,$zero,$zero` in the arms, `addu $a3,$s2,$zero`
     * before each call). The literal 0, one `= 0` initializer, or one write
     * after the join / before the calls do not match. */
    s32 tgt_x;
    /* dx / dz: two values each -- the partner-minus-obj x (z) offset of
     * obj+0x180 (0x188) for the elevation, then of the saved obj+0xF4 (0xFC)
     * position for the twist. Ruling 11, (E)(ii): every
     * write is `partner.x - obj.x` (`.z`). */
    s32 dx;
    s32 dz;
    /* temp: six values -- the wrapped obj+0x1E6 easing delta, the wrapped
     * obj+0x1E8 easing delta, the clamped twist stored to obj+0x1EA (states
     * 0x1D/0xE), the wrapped obj+0x1EA easing delta, and the two random
     * jitters (rng_Next() & 0x3F) - 0x20. Ruling 11 ((D)(2) by any named
     * pass, owner Q58), (E)(i) generic name. */
    s32 temp;
    s16 t;

    if (obj->unk_26C == 0) {
        func_80027334(a);
        func_80027334(b);
    }
    if (obj->unk_6A == 0x15 || obj->unk_6A == 0x25) {
        if (obj->unk_0C == 0x1F) {
            temp2 = 0x100;
            tgt_x = 0;
            tgt_z = 0;
        } else {
            s32 dist_sq;
            s32 dist;

            tgt_z = (obj->unk_1D8 - obj->unk_1C8.vy) & 0xFFF;
            if (tgt_z >= 0x800) {
                tgt_z -= 0x1000;
            }
            if (tgt_z < -0x1FF) {
                tgt_z = -0x1FF;
            } else if (tgt_z >= 0x200) {
                tgt_z = 0x1FF;
            }
            dx = obj->other->unk_180.x - obj->unk_180.x;
            dz = obj->other->unk_180.z - obj->unk_180.z;
            dist_sq = dx * dx + dz * dz;
            if ((u32)dist_sq < 0x400) {
                dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
            } else {
                s32 lzcr = 0;
                if (dist_sq >= 0) {
                    /* gte_Lzc(dist_sq, &lzc_out): gtemac.h 4.3 :174-178 = inline_o.h 4.3
                     * gte_ldlzc :207-210, gte_nop :1095-1097 (x2), gte_stlzc :1074-1077;
                     * LZCR slot sp+0x10 in the target. */
                    __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    lzcr = lzc_out;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            temp2 = 0x400 - ratan2(dist, obj->other->unk_180.y - obj->unk_180.y);
            if (temp2 < -0xFF) {
                temp2 = -0xFF;
            } else if (temp2 >= 0x100) {
                temp2 = 0xFF;
            }
            tgt_x = 0;
        }
    } else {
        tgt_x = 0;
        temp2 = 0;
        tgt_z = 0;
    }

    temp = (tgt_z - obj->unk_1E6) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->unk_1E6 = obj->unk_1E6 + temp / 8;
    temp = (temp2 - obj->unk_1E8) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->unk_1E8 = obj->unk_1E8 + temp / 8;
    func_8002F770(&a->unk_0C[0x15], obj->unk_1E6, obj->unk_1E8, tgt_x);
    func_8002F770(&b->unk_0C[0x15], obj->unk_1E6, obj->unk_1E8, tgt_x);

    t = obj->unk_0C;
    if ((t == 0x1D || t == 0xE) && obj->unk_8C != 0) {
        temp = (ratan2(D_800A387C, obj->other->unk_F4.y - obj->unk_F4.y) - 0x400) & 0xFFF;
        if (temp >= 0x800) {
            temp -= 0x1000;
        }
        if (temp >= 0x200) {
            temp = 0x1FF;
        } else if (temp < -0x1FF) {
            temp = -0x1FF;
        }
        obj->unk_1EA = temp;
        a->unk_0C[0x39] += temp;
        b->unk_0C[0x39] += temp;
    }

    if ((u32)((u16)obj->unk_0E - 6) < 2U && obj->unk_6A == 2) {
        s32 dist_sq;
        s32 dist;

        if (obj->unk_268 == 0) {
            obj->unk_25C.x = obj->unk_F4.x;
            obj->unk_25C.y = obj->unk_F4.y;
            obj->unk_25C.z = obj->unk_F4.z;
        }
        dx = obj->other->unk_F4.x - obj->unk_25C.x;
        dz = obj->other->unk_F4.z - obj->unk_25C.z;
        dist_sq = dx * dx + dz * dz;
        if ((u32)dist_sq < 0x400) {
            dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
        } else {
            s32 lzcr = 0;
            if (dist_sq >= 0) {
                /* gte_Lzc(dist_sq, &lzc_out2): gtemac.h 4.3 :174-178 = inline_o.h 4.3
                 * gte_ldlzc :207-210, gte_nop :1095-1097 (x2), gte_stlzc :1074-1077;
                 * LZCR slot sp+0x14 in the target. */
                __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&lzc_out2):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                lzcr = lzc_out2;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        temp2 = (ratan2(dist, obj->other->unk_F4.y - obj->unk_25C.y) - 0x400) & 0xFFF;
        if (temp2 >= 0x800) {
            temp2 -= 0x1000;
        }
        if (temp2 >= 0x200) {
            temp2 = 0x1FF;
        } else if (temp2 < -0x1FF) {
            temp2 = -0x1FF;
        }
        temp = (temp2 - obj->unk_1EA) & 0xFFF;
        if (temp >= 0x800) {
            temp -= 0x1000;
        }
        obj->unk_1EA = obj->unk_1EA + temp / 8;
        a->unk_0C[0x33] += obj->unk_1EA;
        b->unk_0C[0x33] += obj->unk_1EA;
    }

    if (obj->unk_26E != 0 && obj->unk_96 == 0) {
        temp = (rng_Next() & 0x3F) - 0x20;
        a->unk_0C[0] += temp;
        b->unk_0C[0] += temp;
        a->unk_0C[4] -= temp;
        b->unk_0C[4] -= temp;
        temp = (rng_Next() & 0x3F) - 0x20;
        a->unk_0C[9] += temp;
        b->unk_0C[9] += temp;
        a->unk_0C[0xD] -= temp;
        b->unk_0C[0xD] -= temp;
    }
}
void func_8001F860(Unk80101EC8Record *arg0, s32 arg1) {
    arg1 = (arg1 - arg0->unk_1C8.vy) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    arg0->unk_14C = arg1;
}
s32 func_8001F888(void) {
    s32 dx = D_80101EC8[1].unk_F4.x - D_80101EC8[0].unk_F4.x;
    s32 dy = D_80101EC8[1].unk_F4.z - D_80101EC8[0].unk_F4.z;
    s32 s0 = 0;
    while ((u32)(dx + 0x4000) > 0x8000 || (u32)(dy + 0x4000) > 0x8000) {
        s32 t;
        t = dx + ((u32)dx >> 31);
        dx = (s32)t >> 1;
        t = dy + ((u32)dy >> 31);
        dy = (s32)t >> 1;
        s0 += 1;
    }
    {
        s32 v0 = dx * dx;
        s32 v1 = dy * dy;
        s32 r = SquareRoot0(v0 + v1);
        return r << s0;
    }
}
/* func_8001F938 -- computes the s16 factor at arg0+0x44 from the state kind at
 * arg0+0x6A. Some kinds (and, for kinds 0x2/0x1B/0x28/0x26, an arg0+0x40 value
 * inside the 0xA1..0xA3 bounds or not outside the 0xA2..0xA4 bounds) force
 * 0x1000. Otherwise arg0+0x1C is scaled (Q12) by the table at arg0+0x27E indexed
 * by min(arg0+0x26E + arg0+0x272, 3); the range-checked kinds first also scale by
 * arg0+0x274 (when arg0+0x26C is 0) and by the table at arg0+0x276 indexed by the
 * damage count at arg0+0x270 clamped to 3.
 *
 * `short dmg` is a plain signed short local: the target's second `lhu` and its
 * `sll 16 ; sra 15` are GCC 2.7.2's own lowering of it (extendhisi2,
 * tools/gcc-2.7.2/config/mips/mips.md:2340), not a second view in the source. The
 * same declaration produces the target's 8-byte phantom stack frame
 * (asm/funcs/func_8001F938.s:11 and :117, an addiu pair with no stack accesses in
 * between): a signed short local assigned on more than one path and afterwards
 * used in a sign-extending context.
 *
 * The kind-split (`kind_full` raw for the `(u32)(kind_full - K) < 2U` range checks,
 * `kind = kind_full & 0xFFFFU` for the `==` set) mirrors the target's
 * `lhu $a1,0x6A ; andi $v1,$a1,0xFFFF`. A `(u16)kind_full` cast (`move` instead of
 * `andi`), a single `u16 kind` local, `kind_full` alone with no mask, or a second
 * `*(u16*)` read into a `u16` local do not reproduce it.
 */
void func_8001F938(Unk80101EC8Record *arg0)
{
    u32 kind_full;
    u32 kind;
    s32 val;
    s32 a2;
    s32 idx;
    s32 factor;
    kind_full = arg0->unk_6A;
    kind = kind_full & 0xFFFFU;
    a2 = arg0->unk_1C;
    if (kind == 0x11 || kind == 0xF ||
        ((u32)((s32)kind_full - 0x1C)) < 2U ||
        ((u32)((s32)kind_full - 0x1E)) < 2U ||
        ((u32)((s32)kind_full - 0x20)) < 2U ||
        kind == 0xE || kind == 0x2C || kind == 0xD ||
        kind == 0x7 || kind == 0x33 || kind == 0x14)
    {
        goto clamp;
    }
    if (kind == 0x2) { goto rangecheck; }
    if (kind == 0x1B) { goto rangecheck; }
    if (kind == 0x28) { goto rangecheck; }
    if (kind != 0x26) { goto defaultpath; }
rangecheck:
    val = arg0->unk_40;
    if (val < arg0->unk_A1[0]) { goto check_outer; }
    if (val > arg0->unk_A3[0]) { goto check_outer; }
    goto clamp;
check_outer:
    if (val < arg0->unk_A1[1]) { goto multpath_start; }
    if (val > arg0->unk_A3[1]) { goto multpath_start; }
clamp:
    arg0->unk_44 = 0x1000;
    return;
multpath_start:
    if (arg0->unk_26C == 0)
    {
        s32 f = arg0->unk_274;
        a2 = (a2 * f) >> 12;
    }
    {
        s16 dmg = arg0->unk_270;
        if (dmg >= 4) {
            dmg = 3;
        }
        idx = dmg;
    }
    factor = arg0->unk_276[idx];
    a2 = (a2 * factor) >> 12;
defaultpath:
    {
        s32 vv0 = arg0->unk_26E;
        s32 vv1 = arg0->unk_272;
        s32 sum = vv0 + vv1;
        s32 sum_or_3 = (sum < 4) ? sum : 3;
        idx = sum_or_3;
    }
    factor = arg0->unk_27E[idx];
    a2 = (a2 * factor) >> 12;
    arg0->unk_44 = a2;
}

typedef struct {
    u16 flags;
    u16 id;
    u8 b[4];
} StatusEvt;

StatusEvt *func_8001FAE4(MoveScript *arg0) {
    u16 v1;
    u16 *a0;

    a0 = arg0->unk_0A;
    v1 = *a0;
    while (v1 != 0) {
        if ((v1 & 0x4000) != 0) {
            return (StatusEvt *)a0;
        }
        if ((v1 & 0xC000) != 0) {
            a0 += 4;
        } else {
            a0 += 2;
        }
        v1 = *a0;
    }
    return 0;
}
s32 func_8001FB34(Unk80101EC8Record *arg0, s32 arg1) {
    s16 v1;
    s32 v0;
    v1 = D_800A38DC;
    if (v1 == 2) return 0;
    if (v1 == 5) return 0;
    if (v1 == 3) return 0;
    if (v1 != 0) goto check2;
    if (D_800A385C != 0) return 0;
check2:
    v1 = arg0->other->unk_0C;
    if (v1 == 0xD) return 0;
    if (v1 == 0x1C) return 0;
    v1 = arg0->unk_0A;
    if (v1 != 0xE) goto check3;
    if (arg0->unk_330 == 0) return 0;
    v1 = arg0->unk_332[0];
    if (v1 == 0xA) goto check3;
    return 0;
check3:
    v0 = 1;
    if (arg1 != 0) {
        v0 = arg0->unk_26C;
        v0 = (v0 != 0);
    }
    return v0;
}
void func_8001FBE8(void);


void func_8001FBE8(void) {
    Unk80101EC8Record *rec;
    StatusEvt *ent;
    u8 *data;
    MoveScript *snd;
    s32 lo;
    s32 hi;
    s32 dz;
    s32 i;
    u16 kind;
    s32 pos[3];

    if (D_800A376E != 0) {
        D_800A376E = 0;
        D_800A38E8 = 0xFF;
        if (D_80101EC8[0].unk_96 != 0) {
            return;
        }
        if (D_80101EC8[1].unk_96 != 0) {
            return;
        }
        func_80021A98(D_800A38AE, D_800A36D8, D_800A381C);
        if (D_800A36CA & 0x1000) {
            D_80101EC8[D_800A38AE == 0].unk_4C = 1;
        }
        func_80021A98(D_800A38AE == 0, D_800A36D8, D_800A381C);
        D_80101EC8[1].unk_7A = 2;
        D_80101EC8[0].unk_7A = 2;
        return;
    }
    if (D_800A3758 != 0xFF) {
        rec = &D_80101EC8[D_800A3758];
        if (D_800A3769 != 0) {
            rec->unk_286 = 1;
            rec->unk_94 = 0;
        } else {
            rec->unk_286 = 0;
            rec->unk_94 = 1;
        }
        if (rec->unk_96 != 0) {
            rec->unk_286 += 2;
        }
        rec->unk_74 = rec->unk_B8.vy;
        rec->other->unk_286 = 1;
        rec->other->unk_94 = 0;
        rec->other->unk_74 = rec->other->unk_B8.vy;
        if (rec->other->unk_96 != 0) {
            rec->other->unk_286 += 2;
        }
        D_800A3758 = 0xFF;
        return;
    }
    if (D_80101EC8[0].unk_286 != -1) {
        return;
    }
    if (D_80101EC8[1].unk_286 != -1) {
        return;
    }
    for (i = 0; i < 2; i++) {
        rec = &D_80101EC8[i];
        if (rec->unk_7A == 0) {
            continue;
        }
        ent = func_8001FAE4(rec->unk_50);
        if (ent == 0) {
            continue;
        }
        data = ent->b;
        lo = data[0] * 20;
        hi = data[1] * 20;
        dz = rec->unk_B8.vy - rec->other->unk_B8.vy;
        if (func_8001FB34(rec, data[3] & 0x80) == 0) {
            continue;
        }
        if (D_800A387C < lo) {
            continue;
        }
        if (hi < D_800A387C) {
            continue;
        }
        if (dz <= -100 || dz >= 100) {
            continue;
        }
        kind = rec->other->unk_6A;
        if (kind != 0x15 && kind != 0x2C && kind != 0xE && kind != 0x19) {
            continue;
        }
        D_800A38AE = i;
        D_800A376E = 0;
        D_800A3758 = 0xFF;
        D_800A371C = data[2] * 20;
        D_800A38E8 = data[3] & 0x7F;
        snd = func_80021424(rec, ent->id, &rec->unk_5E);
        func_80021A98(i, snd, rec->unk_5E);
        rec->other->unk_4C = 1;
        rec->other->unk_5E = rec->unk_5E;
        func_80021A98(i == 0, snd, rec->unk_5E);
        rec->unk_7A = 2;
        rec->other->unk_7A = 2;
        rec->other->unk_86 = rec->other->unk_84;
        rec->other->unk_272 += 1;
        pos[0] = (rec->unk_F4.x + rec->other->unk_F4.x) / 2;
        pos[1] = (rec->unk_F4.y + rec->other->unk_F4.y) / 2;
        pos[2] = (rec->unk_F4.z + rec->other->unk_F4.z) / 2;
        func_80032854(i, 0x10, pos, 0);
        return;
    }
}
s32 func_8002006C(void) {
    s32 s0 = D_800A387C;
    s32 v0 = func_8001F888();
    s32 v = D_800A38DC;
    D_800A387C = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        func_8001FBE8();
    }
    D_800A38A8 = 0;
}
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 disc;
    s32 dx;
    s32 dz;
    s32 dist;

    dx = arg1[0] - arg0[0];
    dz = arg1[2] - arg0[2];
    dist = SquareRoot0(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy;

        dy = arg1[1] - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 a0;

            disc = arg2 * arg2 + arg3 * dy2;

            if (disc >= 0) {
                s32 a2;
                disc = SquareRoot0(disc << 10);
                a2 = arg2 << 5;
                a0 = ((a2 + disc) * dist) / dy2 / 32;

                if (a0 < 0) {
                    /* FAKE: dead dy reused as the arm-2 (a2-disc) temp,
                     * mechanism: global.c set_preference/expand_preferences —
                     * disc dies at this subu so its {$v1} pref merges into dy
                     * and flows down the mult/divmodsi4 pref edges to the /32
                     * quotient, whose find_reg low-first override then takes
                     * $v1 (target); dy's own $v0 home matches the subu/mult.
                     * The natural spelling, a fresh named temp and reusing
                     * disc all miss (natural 6, fresh temp 6, reusing disc 8). */
                    dy = a2 - disc;
                    a0 = (dy * dist) / dy2 / 32;
                }
            } else {
                a0 = 300;
            }

            if (a0 >= 301) {
                a0 = 300;
            }

            arg4[0] = (a0 * dx) / dist;
            arg4[2] = (a0 * dz) / dist;
        }
    }
}
/* func_800203B4 — COMPLETED-INLINE-ASM-CANONICAL (owner grant, widened cop2
 * materialize-then-copy anchor; see inline_asm_canonical.txt). Starts the arg0+0x350
 * frame counter, records bone index D_8008D59C[arg1].bone at arg0+0x352, and rotates
 * the vector arg2 by that bone's matrix of game_GetPlayerData(arg0+4) into arg0+0x354.
 * Pure-C head + four PsyQ SDK GTE macro islands — gte_SetRotMatrix, gte_ldv0, cop2
 * MVMVA (.word 0x4A486012), gte_stlvnl — character-identical to the
 * func_8002FDB0-authorized spelling (src/code6cac_b.c, inline_asm_canonical.txt). The
 * 25-insn island surface is minimal (thinner islands do not match) and has no C form
 * (GCC 2.7.2's MIPS backend has zero cop2 mnemonics and no REG_ALLOC_ORDER path to the
 * SDK macros' $12-$15 seats). Load-bearing facts — do not tidy:
 *  - local DECLARATION ORDER (mat, vec, src) is byte-load-bearing;
 *  - STATEMENT ORDER is byte-load-bearing (vec[] stores stay below the func_8002EECC call);
 *  - `arg0 += 0x354;` mirrors the SDK call shape (re-association is byte-neutral). */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    MATRIX *src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = D_8008D59C[arg1].bone;
    src = ((MATRIX **)game_GetPlayerData(*(s16 *)(arg0 + 4)))[*(s16 *)(arg0 + 0x352)];
    func_8002EECC(src, mat);
    /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- loads the 5 packed
     * rotation-matrix words at r into cop2 control regs $0..$4.  The SDK
     * macro body hardcodes $12-$15 and copies the operand into $12. */
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
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    /* PsyQ libgte inline macro gte_ldv0(r) --- pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay carried
     * as explicit nops (maspsx does NOT supply them in the full-build
     * context: without them the link shifts by 8 bytes). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        :: "r"(vec) : "$12", "$13", "$14");
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    arg0 += 0x354;
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(arg0) : "$12");
}
/* func_800204C0 — rec->unk_350 is a frame counter func_800203B4 starts at 1. On the tick
 * where (counter & 7) == 2, the vector func_800203B4 stored at rec->unk_354 is rotated by
 * matrix rec->unk_352 of game_GetPlayerData(pid) (GTE MVMVA), scaled by (150 - counter) / 150,
 * negated when its y exceeds 0x800, and handed to func_80032854 together with scratchpad
 * point SPAD->unkA8[pid][rec->unk_352]. The counter is cleared on every call that finds it
 * running (the original stores 0 under `>= 150` and then again unconditionally).
 * GTE islands: PsyQ Run-time Library Release 4.3 inline_o.h statements, character for
 * character (engine/gtemacro.py PINNED). */
void func_800204C0(Unk80101EC8Record *rec) {
    s32 mac[3];
    s16 out[3];
    s32 pid;
    s32 mul;
    MATRIX **bones;

    pid = rec->index;
    if (rec->unk_350 != 0) {
        rec->unk_350 += 1;
        if ((rec->unk_350 & 7) == 2) {
            bones = game_GetPlayerData(pid);
            /* inline_o.h: gte_SetRotMatrix :272-284 */
            __asm__ volatile ("move  $12,%0": :"r"(bones[rec->unk_352]):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_ldlv0 :95-103, gte_rtv0 :426-430; gte_rtv0's command word is the
             * post-DMPSX word .word 0x4A486012 in place of the header's DMPSX placeholder
             * .word 0x0000013f (MVMVA sf=1 mx=rot v=V0 cv=none lm=0; per-function grant Q92, ded098133) */
            __asm__ volatile ("move  $12,%0": :"r"(&rec->unk_354):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
            mul = ((0x96 - rec->unk_350) << 12) / 150;
            /* inline_o.h: gte_stlvnl :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(mac):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            out[0] = (mac[0] * mul) / 0x1000;
            out[1] = (mac[1] * mul) / 0x1000;
            out[2] = (mac[2] * mul) / 0x1000;
            if (out[1] > 0x800) {
                out[0] = -out[0];
                out[1] = -out[1];
                out[2] = -out[2];
            }
            func_80032854(pid, 4, &SPAD->unkA8[pid][rec->unk_352].x, out);
        }
        if (rec->unk_350 >= 0x96) {
            rec->unk_350 = 0;
        }
        rec->unk_350 = 0;
    }
}

void func_800206B0(s32 arg0, s32 arg1) {
    BoneHitRec *src = D_8008D59C;
    BoneHitRec *dst = D_800F5F68[arg0];
    s32 i;

    for (i = 0; i < 22; i++, src++, dst++) {
        dst->unk_00 = src->unk_00;
        dst->bone = src->bone;
        dst->ofs.vx = (src->ofs.vx * arg1) >> 12;
        dst->ofs.vy = (src->ofs.vy * arg1) >> 12;
        dst->ofs.vz = (src->ofs.vz * arg1) >> 12;
        dst->unk_0C = (src->unk_0C * arg1) >> 12;
        dst->unk_0E = (src->unk_0E * arg1) >> 12;
        dst->unk_10 = (src->unk_10 * arg1) >> 12;
        dst->unk_12 = (src->unk_12 * arg1) >> 12;
    }
}
/* func_800207C8 — places character rec->index's points in world space with its bone
 * matrices (game_GetPlayerData()): each of the 22 hit records' offsets through its bone
 * into bone_out (SPAD->unkA8[ch]); the attachment point set D_8008D86C[unk_0E] (D_8008D774
 * when unk_12 == 50) through bone 18 into att_out; when unk_8C != 0, the pair
 * D_8008D88C[unk_14] through bone 19 into extra_out.  A point is the GTE MVMVA rotation
 * (sf=1) of the SVECTOR plus the matrix translation.  Then: bone 11's y axis into
 * rec->unk_1EC, bone_out[0] into rec->unk_180, the translations of bones 17 and 14 (y
 * raised by (unk_1A * 71) >> 11) into rec->unk_198[], the floor height under each
 * (func_80053614 down a probe from y - 100 to y + 2000; the probe's lower end when it
 * finds nothing) into rec->unk_1B0[], and the two bones' headings into unk_1BA / unk_1C2.
 * GTE islands: PsyQ Run-time Library Release 4.3 inline_o.h statements, character for
 * character (engine/gtemacro.py PINNED); each gte_rtv0 carries the post-DMPSX word
 * .word 0x4A486012 in place of the header's DMPSX placeholder .word 0x0000013f (MVMVA sf=1
 * mx=rot v=V0 cv=none lm=0; per-function grant Q93, fed205ca7). */
void func_800207C8(Unk80101EC8Record *rec, LeafPos *bone_out, LeafPos *att_out, LeafPos *extra_out) {
    /* the func_80053614 probe in scratchpad: from (words 0..2), to (4..6), hit (8..10),
     * normal (12..13), work area (14..) */
    s32 *probe = (s32 *)0x1F8002B8;
    MATRIX **bones;
    MATRIX *m;
    s32 *pos;
    SVec4i16 *v;
    BoneHitRec *hr;
    LeafPos *o;
    s32 i;

    bones = game_GetPlayerData(rec->index);
    hr = D_800F5F68[rec->index];
    o = bone_out;
    for (i = 0; i < 22; i++, hr++, o++) {
        m = bones[hr->bone];
        /* inline_o.h: gte_SetRotMatrix :272-284 */
        __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_ldv0 :16-20 */
        __asm__ volatile ("move  $12,%0": :"r"(&hr->ofs):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_rtv0 :426-430; .word 0x0000013f -> post-DMPSX .word 0x4A486012 (Q93, fed205ca7) */
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_stlvnl :904-909 */
        __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    m = bones[18];
    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    if (rec->unk_12 == 50) {
        v = D_8008D774;
    } else {
        v = D_8008D86C[rec->unk_0E];
    }
    o = att_out;
    for (i = 0; i < D_8008D864[rec->unk_0E]; i++, v++, o++) {
        /* inline_o.h: gte_ldv0 :16-20 */
        __asm__ volatile ("move  $12,%0": :"r"(v):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_rtv0 :426-430; .word 0x0000013f -> post-DMPSX .word 0x4A486012 (Q93, fed205ca7) */
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        /* inline_o.h: gte_stlvnl :904-909 */
        __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    if (rec->unk_8C != 0) {
        m = bones[19];
        /* inline_o.h: gte_SetRotMatrix :272-284 */
        __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
        v = D_8008D88C[rec->unk_14];
        o = extra_out;
        for (i = 0; i < 2; i++, v++, o++) {
            /* inline_o.h: gte_ldv0 :16-20 */
            __asm__ volatile ("move  $12,%0": :"r"(v):"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_rtv0 :426-430; .word 0x0000013f -> post-DMPSX .word 0x4A486012 (Q93, fed205ca7) */
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
            /* inline_o.h: gte_stlvnl :904-909 */
            __asm__ volatile ("move  $12,%0": :"r"(o):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
            o->x += m->t[0];
            o->y += m->t[1];
            o->z += m->t[2];
        }
    }

    m = bones[11];
    /* inline_o.h: gte_SetRotMatrix :272-284 */
    __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_ldv0 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&D_800A3138):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* inline_o.h: gte_rtv0 :426-430; .word 0x0000013f -> post-DMPSX .word 0x4A486012 (Q93, fed205ca7) */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    rec->unk_180 = bone_out[0];
    /* inline_o.h: gte_stlvnl :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&rec->unk_1EC):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    pos = bones[17]->t;
    rec->unk_198[0].x = pos[0];
    rec->unk_198[0].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[0].z = pos[2];
    pos = bones[14]->t;
    rec->unk_198[1].x = pos[0];
    rec->unk_198[1].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[1].z = pos[2];
    for (i = 0; i < 2; i++) {
        probe[0] = rec->unk_198[i].x;
        probe[1] = rec->unk_198[i].y - 100;
        probe[2] = rec->unk_198[i].z;
        probe[4] = rec->unk_198[i].x;
        probe[5] = rec->unk_198[i].y + 2000;
        probe[6] = rec->unk_198[i].z;
        if (func_80053614(&probe[0], &probe[4], &probe[8], (s16 *)&probe[12], (s32)&probe[14])) {
            rec->unk_1B0[i] = probe[9];
        } else {
            rec->unk_1B0[i] = probe[5];
        }
    }
    m = bones[17];
    rec->unk_1BA = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
    m = bones[14];
    rec->unk_1C2 = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
}
void func_80020CDC(void) {
    u16 *p = D_800A38C4; /* FAKE: direct D_800A38C4[1] puts the constant address in a pseudo that CSE keeps live across seq_Reset (+3 insns) */

    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    p[1] = 0;
    p[0] = 0;
    D_800A38C0[1] = 0xFF;
    D_800A38C0[0] = 0xFF;
}
void func_80020D38(void) {
    u16 *p = D_800A38C4; /* FAKE: direct D_800A38C4[1] puts the constant address in a pseudo that CSE keeps live across seq_Reset (+3 insns) */

    if (p[1] == 0xFFFF) {
        seq_Reset();
    }
    p[1] = 0;
}

void func_80020D70(void) {
    D_800A3888[0] = (MotionFrame *)0x80118800;
    D_800A3888[1] = (MotionFrame *)0x8011C400;
    D_800A3830 = (s32)0x80120000;
    D_800A3860[0] = (Tbl800A3860Entry *)0x80148800;
    D_800A3860[1] = (Tbl800A3860Entry *)0x80190800;
    func_80020CDC();
}
void func_80020DDC(void) {    s32 v0;    s32 v1;    s32 v2;    v0 = func_80036EA8(1, 1);    cdrom_StartRead(v0, D_800A3830);    game_FrameLoop();    v1 = D_800A3830;    D_80102760 = v1 + 0x14;    D_80102764 = v1 + *(s32 *)(v1 + 4);    D_80102768 = v1 + *(s32 *)(v1 + 8);    v2 = *(s32 *)(v1 + 0x10);    D_800A3880 = 1;    D_80102770 = v1 + v2;}

/* Loads the motion sets and models for the two characters about to fight:
 * slot i gets character chr0 / chr1 in costume costume0 / costume1. */
void func_80020E74(s32 chr0, s32 costume0, s32 chr1, s32 costume1) {
    u16 loads[130]; /* FAKE: frame layout -- only loads[0..1] are used; the target frame (0x140) reserves 0x100 untouched bytes after them (sp+0x14..0x113), N = 129..132 (loads[2] gives the wrong frame) */
    s32 i;
    s32 j; /* FAKE: one local for loop 1's character and loop 2's menuDat index; separate locals seat the index in $a1, the target keeps both in $s0 */

    if (D_800A3880 == 0) {
        func_80020DDC();
    }

    if (D_800A38DC == 1 || D_800A38DC == 4 || D_800A38DC == 6) {
        for (i = 0; i < 2; i++) {
            j = chr0;
            if (i != 0) {
                j = chr1;
            }
            if (D_800A38C0[i] != j) {
                D_800A38C0[i] = j;
                cdrom_StartReadAt(func_80036EA8(1, 0), (s32)D_800A3888[i], j * 7, 7);
                game_FrameLoop();
            }
        }
    }

    loads[1] = 0;
    loads[0] = 0;
    {
        u16 id0 = D_8008DB1C[chr0][costume0] | (costume0 << 12);
        u16 id1;

        if (D_800A38DC == 3) {
            id1 = id0;
        } else {
            id1 = D_8008DB1C[chr1][costume1] | (costume1 << 12);
        }
        D_80101EC8[0].unk_48 = id0;
        D_80101EC8[1].unk_48 = id1;

        if (id0 == id1) {
            if (D_800A38C4[0] == id1 || D_800A38C4[1] == id1) {
                return;
            }
            loads[0] = id0;
        } else if (D_800A38C4[0] == id0) {
            if (D_800A38C4[1] == id1) {
                return;
            }
            loads[1] = id1;
        } else if (D_800A38C4[0] == id1) {
            if (D_800A38C4[1] == id0) {
                return;
            }
            loads[1] = id0;
        } else if (D_800A38C4[1] == id1) {
            loads[0] = id0;
        } else {
            loads[0] = id0;
            loads[1] = id1;
        }
    }

    for (i = 0; i < 2; i++) {
        if (loads[i] != 0) {
            for (j = 0; menuDat[j].id != 0; j++) {
                if (menuDat[j].id == loads[i]) {
                    break;
                }
            }
            cdrom_StartRead(func_80036EA8(1, j + 2), (s32)D_800A3860[i]);
            game_FrameLoop();
            D_801027B0[i][0] = (s32)D_800A3860[i] + 0x6C + (D_800A3860[i]->unk_03 - 1) * 6;
            D_801027B0[i][1] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[0];
            D_801027B0[i][2] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[1];
            D_801027B0[i][3] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[2];
            D_801027B0[i][4] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[3];
            D_800A38C4[i] = loads[i];
        }
    }
}
void func_80021210(void) {
    func_8001979C(0, D_80102770);
    if (D_800A38C4[0]) {
        func_8001979C(1, D_801027B0[0][4]);
    }
    if (D_800A38C4[1]) {
        func_8001979C(2, D_801027B0[1][4]);
    }
}
/*
 * func_80021280: the record's model id (unk_48) -> its slot in D_800A38C4 (unk_4A); for ids below
 * 0x2000, the nibble positions of 4 and 5 (unk_88 / unk_8E), copying unk_26C into unk_8A / unk_90
 * under mode-dependent conditions.
 * The loop-tail duplication below is the construct accepted by an owner ruling on SOTN
 * evidence (docs/grind/sotn-evidence-2026-08-06.md). Every codegen-only local is
 * FAKE-labelled with what its natural spelling breaks.
 */
void func_80021280(s32 a0) {
    s32 a1 = 0;
    Unk80101EC8Record *a2 = &D_80101EC8[a0];
    s32 a3 = a2->unk_48;
    u16 *v1 = D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = a2->unk_48;
        a2->unk_4A = a1;
        a2->unk_4C = 0;

        if ((u32)(val >> 12) < 2) {
            u16 t1;   /* FAKE: copy of val (shifting val directly: score 7) */
            s32 t4;   /* FAKE: holds the constant 4 (a literal does not match) */
            s32 t3;   /* FAKE: holds the constant 3 (a literal does not match) */
            s32 t2;   /* FAKE: holds the constant 1 (a literal does not match) */
            u8 t0;    /* FAKE: D_800A384C read once before the loop (reading in place does not match) */
            s32 mode; /* FAKE: D_800A38DC read once before the loop (reading in place does not match) */
            s32 k;

            k = 0;
            t1 = val;
            t4 = 4;
            t3 = 3;
            t2 = 1;
            mode = D_800A38DC;
            t0 = D_800A384C;
        loop2_21280:
            {
                u16 nibble = (t1 >> (k << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                a2->unk_88 = k;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                a2->unk_8A = a2->unk_26C;
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                a2->unk_8E = k;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) {
                    /* FAKE: loop tail duplicated into this arm (jump2 cross-jump
                       re-merges it to identical bytes); with `goto next_21280` instead,
                       the counters and the record pointer swap $a1 / $a2 */
                    k++;
                    if (k < 3) goto loop2_21280;
                    return;
                }
            store5_21280:
                a2->unk_90 = a2->unk_26C;
            }
        next_21280:
            k++;
            if (k < 3) goto loop2_21280;
        }
    }
}
void func_800213A0(Unk80101EC8Record *arg0) {
    s16 a1 = arg0->unk_86;
    if (a1 != arg0->unk_88) {
        if (a1 != arg0->unk_8E) {
            return;
        }
    }
    arg0->unk_86 = (s16)((a1 + 1) % D_800A3860[arg0->unk_4A]->f14);
}
/* Rodata moved from asm/data/800.rodata_post.s (rodata-cleanup project,
 * docs/rodata-cleanup-project.md): the 66-string animation/asset
 * table referenced by func_80023F08. Defined here, ahead of func_80021424, so
 * it follows the 0x94 bytes of switch-jtbl rodata emitted by the functions
 * above (0x80010068..0x800100FC) and precedes func_80021424's own switch table
 * (0x80010414), matching the original rodata order. Bracket-sized [66][12] to
 * match the fixed 12-byte stride per name (8-char content + null + pad). */
const char D_800100FC[66][12] = {
    "WIN     ",
    "KARAMI_ED",
    "RUN_ED  ",
    "RUN_ST  ",
    "CHAKUTI ",
    "APPEAR  ",
    "KAISHAKU_ST",
    "HAJIKARE",
    "SERIEXIT",
    "SYAGAMI ",
    "HOM_ED  ",
    "HOM_AT  ",
    "HOM_ST  ",
    "ANOBORI ",
    "KAMAE_KA",
    "MOVE    ",
    "NOBORI_E",
    "NOBORI_S",
    "FURI2   ",
    "FURI1   ",
    "YURI2   ",
    "YURI1   ",
    "GOKAKU  ",
    "START   ",
    "ARUN    ",
    "STEP    ",
    "WALK    ",
    "LJUMP   ",
    "MJUMP   ",
    "SJUMP   ",
    "KAMAE   ",
    "DTH     ",
    "RUN     ",
    "SUNA    ",
    "KARAMI  ",
    "RELOAD  ",
    "SERI    ",
    "HAJI    ",
    "UKE     ",
    "SYASTEP ",
    "SUBWEP  ",
    "ORI     ",
    "OKIAGARI",
    "NOBORI  ",
    "KZRE    ",
    "KOROGARI",
    "KAISYAKU",
    "END_GAME",
    "DAM     ",
    "ATTACK  ",
    "NORMAL  ",
    "NULL    ",
    "Y123.BBM",
    "N123.BBM",
    "K123.BBM",
    "T123.BBM",
    "S234.BBM",
    "S125.BBM",
    "S124.BBM",
    "S123.BBM",
    "U235.BBM",
    "U135.BBM",
    "U134.BBM",
    "U125.BBM",
    "U124.BBM",
    "U123.BBM",
};
void *func_80021424(Unk80101EC8Record *rec, s32 id, s16 *out)
{
    s16 t;
    s32 ch;

    rec->unk_78 = 0;
    *out = 0;
    if ((u32)(id - 0x7FF5) < 11) {
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f66[id - 0x7FF5][rec->unk_86] * 2);
    }
    switch (id) {
    case 0x7FF0:
        rec->unk_78 = 1;
        rec->unk_86 = rec->unk_84;
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f4E[rec->unk_84] * 2);
    case 0x7FF1:
        rec->unk_86 = (rec->unk_86 + 1)
                    % D_800A3860[rec->unk_4A]->f14;
    case 0x7FF2:
    case 0x7FF4:
        if (id == 0x7FF4) {
            rec->unk_78 = 1;
        }
        t = rec->unk_86;
        if ((t == rec->unk_88 && rec->unk_8A == 0)
         || (t == rec->unk_8E && rec->unk_90 == 0)) {
            rec->unk_86 = (rec->unk_86 + 1)
                        % D_800A3860[rec->unk_4A]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0) {
            func_800213A0(rec);
        }
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f4E[rec->unk_86] * 2);
    case 0x7FF3:
        t = (rec->unk_86 + 1) % D_800A3860[rec->unk_4A]->f14;
        if ((t == rec->unk_88 && rec->unk_8A == 0)
         || (t == rec->unk_8E && rec->unk_90 == 0)) {
            t = (t + 1) % D_800A3860[rec->unk_4A]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0
                   && (t == rec->unk_88 || t == rec->unk_8E)) {
            t = (t + 1) % D_800A3860[rec->unk_4A]->f14;
        }
        return (void *)(D_801027B0[rec->unk_4A][0]
             + D_800A3860[rec->unk_4A]->f54[rec->unk_86][t] * 2);
    }
    if (id & 0x8000) {
        if (rec->unk_4C != 0) {
            ch = rec->other->unk_4A;
        } else {
            ch = rec->unk_4A;
        }
        return (void *)(D_801027B0[ch][0] + (id & 0x7FFF) * 2);
    }
    *out = 1;
    return (void *)(D_80102760 + id * 2);
}
void func_800218C8(s32 a0) {
    D_80101EC8[a0].unk_86 = D_80101EC8[a0].unk_84;
}
s32 func_80021904(s32 a0) {
    s16 v1 = D_80101EC8[a0].unk_4A;
    s16 v0 = D_80101EC8[a0].unk_86;
    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;
}
s32 func_80021974(s32 a0) {
    s16 v1 = D_80101EC8[a0].unk_4A;
    s16 v0 = D_80101EC8[a0].unk_84;
    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;
}
s32 func_800219E4(s32 a0) {
    return D_80102760 + D_800A3860[D_80101EC8[a0].unk_4A]->f16 * 2;
}
s32 func_80021A3C(s32 a0, s32 a1) {
    return D_80102760 + D_800A3860[D_80101EC8[a0].unk_4A]->f18[a1] * 2;
}
void func_80021A98(s32 arg0, MoveScript *arg1, s32 arg2) {
    Unk80101EC8Record *s0 = &D_80101EC8[arg0];
    s32 a3;
    if ((s0->unk_4C) != 0) {
        a3 = s0->other->unk_4A;
    } else {
        a3 = s0->unk_4A;
    }
    s0->unk_4C = 0;
    s0->unk_50 = arg1;
    {
        u16 v1 = arg1->unk_04;
        s0->unk_5C = v1;
        if (arg2 != 0) {
            u16 *v0 = (u16 *)(D_80102764 + (v1 * 4));
            s0->unk_54 = v0;
            s0->unk_58 = (u8 *)(D_80102768 + v0[1]);
        } else {
            u16 *v0 = (u16 *)(D_801027B0[a3][1] + (v1 * 4));
            s0->unk_54 = v0;
            s0->unk_58 = (u8 *)(D_801027B0[a3][2] + v0[1]);
        }
    }
    {
        MoveScript *v0_50 = s0->unk_50;
        u16 old_kind = s0->unk_6A;
        u8 *a0_58 = s0->unk_58;
        s0->unk_60 = (u8) arg2;
        /* FAKE: empty do-while(0) -- its loop note puts a0_58 in $a0 and
         * v1_58 in $v1 (the two unk_58 lbu bases); without it they swap,
         * score 2. */
        do { } while (0);
        s0->unk_61 = (u8) a3;
        {
            u8 a1_val = v0_50->unk_06;
            s0->unk_6C = old_kind;
            {
                u8 *v1_58 = s0->unk_58;
                s0->unk_42 = 0;
                s0->unk_7A = 1;
                s0->unk_7C = 0;
                s0->unk_46 = 0;
                s0->unk_40 = a1_val;
                /* FAKE: the do-while(0) wrap's loop-depth weighting seats a0_58
                 * in $a0 and a1_val in $a1 as in target; without it they swap, score 4. */
                do { s0->unk_6A = *a0_58; } while (0);
                s0->unk_6E = v1_58[2];
            }
        }
        {
            MoveScript *v0_50b = s0->unk_50;
            s32 kind = s0->unk_6A;
            s0->unk_70 = (v0_50b->unk_09) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) || (kind == 0x26)) {
                    a0_flag = 1;
                }
                s0->unk_AD = a0_flag;
            }
            func_800324D0(s0);
            {
                s32 kind2 = s0->unk_6A;
                /* FAKE: redundant mask of the u16 state code, reproduces the target's andi; without it score 6 */
                s32 v1k = kind2 & 0xFFFF;
                if (v1k == 9) {
                    s0->unk_152 = 1;
                    s0->unk_154 = s0->unk_1C8.vy;
                    goto end;
                }
                if (v1k == 2) {
                    if ((s0->unk_152) != 0) goto clear_152;
                    if ((s0->unk_6C) == 0x13) goto clear_152;
                    s0->unk_154 = s0->unk_1D8;
                    goto clear_152;
                }
                if (((u32) (kind2 - 0x19)) >= 2U) goto not_in_range;
                if ((s0->unk_152) == 0) goto set_154;
                if (v1k != 0x19) goto set_152;
                if ((s0->unk_6C) != v1k) goto set_152;
                set_154:
                s0->unk_154 = s0->unk_1D8;
                goto set_152;
                not_in_range:
                if (v1k != 0x11) goto clear_152;
                set_152:
                s0->unk_152 = 1;
                goto end;
                clear_152:
                s0->unk_152 = 0;
            }
            end:
            {
                u16 v1f = s0->unk_6A;
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) || (v1f == 0x26)) {
                    s0->unk_AF = ((s0->unk_B0) & 0xF) != 5;
                }
            }
        }
    }
}
void func_80021D10(s32 arg0, s32 *arg1, s32 arg2) {
    s16 *temp_v0;
    temp_v0 = (s16 *)stage_GetDataPtr() + ((D_800A36A4 * 0x18) + (arg2 * 6) + (arg0 * 3));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}
void func_80021DB0(s32 arg0, Vec3i32 *out, s32 *pos) {
    Vec3i32 base;
    Vec3i32 cur;
    Vec3i32 probe;
    Vec3i32 hit;
    s16 nrm[4];
    s16 *stage;
    s32 ofs;
    s32 phase;
    s32 i; /* the counter of both the probe loop and the start-record search, as SOTN reuses one
            * counter across consecutive loops (owner Q51): SOTN: src/dra/menu.c:138 @aa53500 */
    /* Ruling 11 (ordinary-c-judge-decidable.md): holds three values -- the ray-walk step (1..40), the
     * floor-climb step (1..40), then the chosen stage start-record index. */
    s32 temp;
    s32 angle;
    s32 dx;
    s32 dz;
    s32 best;
    s32 d;

    stage = stage_GetDataPtr();
    phase = rand();
    ofs = rand() & 7;
    base.x = pos[0];
    base.y = pos[1] - 500;
    base.z = pos[2];
    for (i = 0; i < 8; i++) {
        angle = (phase + ((ofs + i * 3) << 9)) & 0xFFF;
        dx = (Judge[(angle + 0x400) & 0xFFF] * 4000) / 4096;
        dz = (Judge[angle] * 4000) / 4096;
        probe.x = base.x + dx;
        probe.y = base.y;
        probe.z = base.z + dz;
        if (func_80053614(&base.x, &probe.x, &hit.x, nrm, 0x1F8002B8) != 0) {
            continue;
        }
        cur = base;
        for (temp = 1; temp < 41; temp++) {
            probe.y = base.y;
            probe.x = base.x + (dx * temp) / 40;
            probe.z = base.z + (dz * temp) / 40;
            if (func_8005344C(&cur.x, &probe.x, &hit.x, nrm, 0x1F8002B8) != 0) {
                break;
            }
            *out = probe;
            cur = *out;
        }
        probe = *out;
        probe.y += 4000;
        if (func_80053614(&out->x, &probe.x, &hit.x, nrm, 0x1F8002B8) == 0) {
            continue;
        }
        if (nrm[1] != -0x1000) {
            continue;
        }
        cur = *out;
        for (temp = 1; temp < 41; temp++) {
            probe.y = out->y + temp * 100;
            if (func_8005344C(&cur.x, &probe.x, &hit.x, nrm, 0x1F8002B8) != 0 && nrm[1] == -0x1000) {
                out->y = hit.y;
                return;
            }
            cur.y = probe.y;
        }
    }
    if (D_800A38DC == 3) {
        temp = D_800A38E0;
    } else {
        best = 0x7FFFFFFF;
        stage += D_800A36A4 * 24;
        for (i = 0, temp = 0; i < 4; i++) {
            dx = stage[i * 6 + 3] - pos[0];
            dz = stage[i * 6 + 5] - pos[2];
            d = dx * dx + dz * dz;
            if (d < best) {
                best = d;
                temp = i;
            }
        }
    }
    stage += temp * 6 + 3;
    out->x = stage[0];
    out->y = stage[1];
    out->z = stage[2];
}
void func_80022224(s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[6];
    s16 *base;
    s16 *p;
    s32 *d;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 *r;
    s32 *w;

    base = (s16 *)stage_GetDataPtr() + (D_800A36A4 * 3) * 8;
    i = 0;
    p = base;
    d = dists;
    do {
        dx = p[3] - arg2[0];
        dz = p[5] - arg2[2];
        *d = dx * dx + dz * dz;
        i++;
        d++;
        /* FAKE: split increment — two s16-triplets per record (rot + pos);
         * biv_count=2 stops loop.c reducing the p+6/p+10 address-givs
         * (single p += 6 always reduces: benefit 4 - add_cost*1 > 0);
         * combine re-merges the adds to one addiu. The two-increment class
         * is mechanism-proven as the original spelling; owner-sanctioned
         * per proven-spelling-class-reconstruction.md. */
        p += 3;
        p += 3;
    } while (i < 4);

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] < dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] > dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    r = dists;
    i = 0;
    {
        s32 stop = -1;
        w = r;
        for (; i < 4; i++) {
            if (*r != stop) {
                w[4] = i;
                w++;
            }
            r++;
        }
    }

    base += dists[4 + (rand() & 1)] * 6 + 3;
    arg1[0] = base[0];
    arg1[1] = base[1];
    arg1[2] = base[2];
}
s32 func_80022408(s32 *arg0) {
    s16 *p;
    s32 i;
    s32 best_dist;
    s32 best;
    s32 t1;
    s32 t2;
    s32 dx;
    s32 dz;
    s32 dist;
    p = stage_GetDataPtr();
    best_dist = 0x7FFFFFFF;
    i = 0;
    t1 = arg0[0];
    t2 = arg0[2];
    p = p + ((D_800A36A4 * 3) * 8);
loop:
    dx = ((p[0] + p[3]) / 2) - t1;
    dz = ((p[2] + p[5]) / 2) - t2;
    dist = dx * dx + dz * dz;
    if (dist < best_dist) {
        best_dist = dist;
        best = i;
    }
    i++;
    if (i < 4) {
        p += 6;
        goto loop;
    }
    return best;
}
s32 func_800224E0(Unk80101EC8Record *arg0) {
    u8 *p;
    u8 *end;
    s32 val;
    s32 i;

    p = D_8008EB1C[D_800A384C];
    end = p + 2;
    val = D_8008DB1C[arg0->other->unk_0A][arg0->other->unk_0E];
    do {
        for (i = 0; i < 3; i++) {
            if (*p == ((val >> (i * 4)) & 0xF)) {
                return i;
            }
        }
        p++;
    } while ((s32)p < (s32)end); /* FAKE: signed compare (slt); `p < end` gives sltu (score 1) */
    return 0;
}
void func_80022568(Unk80101EC8Record *arg0) {
    arg0->unk_26C = 1;
    arg0->unk_26E = 0;
    arg0->unk_270 = 0;
    arg0->unk_272 = 0;
}
void func_80022580(s32 idx, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    Unk80101EC8Record *p;
    Vec3i32 other;
    s32 level;
    s32 ang;
    s32 i;
    s32 slot;

    p = &D_80101EC8[idx];
    D_800A3758 = 0xFF;
    D_800A376E = 0;
    p->unk_3C = 0;
    p->other = (idx != 0) ? &D_80101EC8[0] : &D_80101EC8[1];
    p->index = idx;
    p->unk_06 = arg1;
    p->unk_0C = arg2;
    p->unk_0A = D_8008D538[arg2];
    p->unk_0E = arg3;

    if (D_800A38DC == 5 || (D_800A38DC == 2 && D_800A389A == 0)) {
        p->unk_12 = D_8008EB38[p->unk_0E];
    } else if (idx == 1 && D_800A38DC == 3) {
        p->unk_12 = D_8008EB28[p->unk_0E][D_8008D9EC[p->other->unk_0A] == 0];
    } else if (D_800A38DC == 0 && D_800A385C != 0 && idx == 0) {
        p->unk_12 = 0x32;
    } else {
        p->unk_12 = D_8008EB28[p->unk_0E][D_8008D9EC[p->unk_0A]];
    }

    if (idx == 1 && D_800A38DC == 3) {
        p->unk_1A = D_80094C68[D_800A38DE];
    } else {
        p->unk_1A = D_80094C68[D_8008D578[p->unk_0C]];
    }
    p->unk_1C = D_8008DE34[p->unk_0A][p->unk_0E];
    p->unk_1E = D_8008DF78[p->unk_0A][p->unk_0E];
    p->unk_20 = p->unk_1E;
    p->unk_08 = 0x1000;

    if (D_800A38DC == 3 && idx == 1) {
        p->unk_84 = func_800224E0(p);
        level = (D_800A38E2 - 1) / 10 + 1;
        p->unk_1C = level * 96 + ((level == 9) ? 0xA00 : 0x800);
        if (level == 9) {
            p->unk_20 = 0x1080;
        } else {
            p->unk_20 = level * 64 + 0xC00;
        }
        if (level == 9) {
            p->unk_08 = 0xE00;
        } else {
            p->unk_08 = level * 80 + 0x150;
        }
    } else {
        p->unk_84 = D_8008DD5C[p->unk_0A][p->unk_0E];
    }

    p->unk_88 = -1;
    p->unk_8A = 0;
    p->unk_8E = -1;
    p->unk_90 = 0;
    slot = (D_800A3670 != 0) ? D_800A38DF : 0;

    switch (D_800A38DC) {
    case 0:
        if (arg4 != 0) {
            other = p->other->unk_D8;
            func_80022224(idx, &p->unk_D8.x, &other.x);
        } else {
            /* FAKE: default's two func_80021D10 calls repeated in this arm
             * (duplicated-statement-into-arms; calls byte-identical, owner
             * Q47): cross-jump merges the copies (no extra jal). Mechanism:
             * block layout. With the copy here the dispatch tests mode == 0
             * first, places case 0's block after the range tests and reaches
             * case 2/3 through the target's `j` (asm/funcs/func_80022580.s
             * :250-251); every shared form lays the dispatch out in
             * another order. */
            func_80021D10(idx, &p->unk_D8.x, slot);
            func_80021D10(idx == 0, &other.x, slot);
        }
        break;
    case 2:
    case 3:
        if (arg4 != 0) {
            other = p->other->unk_D8;
            func_80021DB0(idx, &p->unk_D8, &other.x);
        } else {
            func_80021D10(idx, &p->unk_D8.x, D_800A38E0);
            func_80021D10(idx == 0, &other.x, D_800A38E0);
        }
        break;
    default:
        func_80021D10(idx, &p->unk_D8.x, slot);
        func_80021D10(idx == 0, &other.x, slot);
        break;
    }

    p->unk_E8.x = 0;
    p->unk_E8.y = -0x384;
    p->unk_E8.z = 0;
    p->unk_F4.x = p->unk_D8.x + p->unk_E8.x;
    p->unk_F4.y = p->unk_D8.y + p->unk_E8.y;
    p->unk_F4.z = p->unk_D8.z + p->unk_E8.z;
    other.x += p->unk_E8.x;
    other.y += p->unk_E8.y;
    other.z += p->unk_E8.z;
    p->unk_B8.vx = p->unk_F4.x;
    p->unk_B8.vy = p->unk_D8.y;
    p->unk_B8.vz = p->unk_F4.z;
    p->unk_C8 = p->unk_B8;
    p->unk_1F8 = p->unk_E8;
    p->unk_104.vx = 0;
    p->unk_104.vy = 0;
    p->unk_104.vz = 0;
    p->unk_24C = p->unk_104;
    p->unk_114[0].vx = 0;
    p->unk_114[0].vy = 0;
    p->unk_114[0].vz = 0;
    p->unk_114[1].vx = 0;
    p->unk_114[1].vy = 0;
    p->unk_114[1].vz = 0;
    p->unk_134.vx = 0;
    p->unk_134.vy = 0;
    p->unk_134.vz = 0;
    p->unk_144 = 0;
    p->unk_148 = p->unk_B8.vy;
    p->unk_14C = 0;
    p->unk_150 = 0;
    p->unk_152 = 0;
    p->unk_14E = 0;
    p->unk_156 = 0xC00;
    p->unk_158 = 0xF80;
    p->unk_15A = 0xC00;
    p->unk_15E = 0xC00;
    p->unk_160 = 0xC00;
    p->unk_162 = 0xC00;
    p->unk_1E8 = 0;
    p->unk_1E6 = 0;
    p->unk_1EA = 0;
    p->unk_268 = 0;
    p->unk_168 = p->unk_F4;
    p->unk_174 = p->unk_F4;
    p->unk_180 = p->unk_F4;
    p->unk_18C = p->unk_F4;
    p->unk_1C8.vx = 0;
    p->unk_1C8.vy = 0;
    p->unk_1C8.vz = 0;
    ang = ratan2(other.x - p->unk_F4.x, other.z - p->unk_F4.z);
    p->unk_1C8.vy = p->unk_1D8 = ang;
    if (p->unk_0C == 0x1F) {
        p->unk_1C8.vy = ang + 0x800;
    }
    p->unk_1D0 = p->unk_1C8;
    func_800206B0(idx, p->unk_1A);
    if (D_800A38DC != 0 || idx != 0 || D_800A3907 < 2 || D_800A3670 != 0) {
        func_80022568(p);
    }
    p->unk_274 = D_8008E3C0[p->unk_0A];
    for (i = 0; i < 4; i++) {
        p->unk_276[i] = D_8008E3F8[p->unk_0A][i];
        p->unk_27E[i] = D_8008E4D0[p->unk_0A][i];
    }
    p->unk_A0 = 8;
    p->unk_7C = 0;
    p->unk_286 = -1;
    p->unk_31A = 0;
    p->unk_1DC = 0;
    p->unk_72 = 0;
    p->unk_96 = 0;
    p->unk_B1 = 7;
    p->unk_B2 = 0;
    p->unk_34C = 0;
    func_8003047C(p);
    p->unk_14 = p->unk_332[0];
    if (D_800A38DC == 5 || (D_800A38DC == 2 && D_800A389A == 0)) {
        if (p->unk_0A == 1 || p->unk_0A == 3 || p->unk_0A == 4 || p->unk_0A == 9 || p->unk_0A == 0x11) {
            p->unk_332[0] = p->unk_14 = 0x11;
        } else {
            p->unk_14 = -1;
            p->unk_330 = 0;
        }
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C != 4) {
        p->unk_14 = -1;
        p->unk_330 = 0;
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C == 4) {
        if (p->other->unk_0A == 1 || p->other->unk_0A == 3 || p->other->unk_0A == 4 ||
            p->other->unk_0A == 9 || p->other->unk_0A == 0x11) {
            p->unk_332[0] = p->unk_14 = p->other->unk_14;
            p->unk_330 = 1;
        }
    } else {
        if (p->unk_0C == 0x1D) {
            p->unk_14 = 0x1F;
            p->unk_34A = 2;
        } else if (p->unk_0C == 0xE) {
            p->unk_14 = 0x1E;
            p->unk_34A = 2;
        } else {
            p->unk_34A = 0xA;
        }
        p->unk_34B = 9;
    }
    if (D_800A38DC == 5 || D_800A38DC == 2 || D_800A38DC == 3 || (D_800A38DC == 0 && D_800A385C != 0)) {
        p->unk_34D = 0;
    } else {
        p->unk_34D = 2;
    }
    p->unk_350 = 0;
}
void func_80022F34(void) {
    s32 i;
    u16 *tbl;

    i = 0;
    tbl = D_80102778.unk_0;
    do {
        Unk80101EC8Record *rec = &D_80101EC8[i];

        if (rec->unk_06 != 0) {
            s32 mode = D_800A38DC;

            switch (mode) {
                case 0:
                    rec->unk_08 = D_80102778.unk_A[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    rec->unk_08 = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = rec->unk_4A;
                s32 val1 = D_801027B0[idx1][3];
                {
                    s16 idx2 = rec->other->unk_4A;
                    func_80055138(i, (u16 *)val1, (u16 *)D_801027B0[idx2][3]);
                }
            }
        }

        tbl++;
        i++;
    } while (i < 2);
}

/* func_8002304C (tanren_CameraControl) - pure C, no FAKE constructs.
 *
 * The target's `andi $v1,$a0,0xffff` at 0x232F4 (rather than a plain copy)
 * comes from spelling the masked state id the way the original author spelled
 * it in the sibling function func_80023E40 later in this file, which
 * reads the identical field with the identical two-line idiom
 *     s32 a0 = arg0->unk_6A;
 *     s32 v1 = a0 & 0xFFFF;
 * and runs the identical comparison cascade (== 8, == 0x22,
 * (u32)(a0 - 0x17) < 2, == 0xA). The two functions are copy-paste siblings in
 * the original source; reconstructing the same idiom here is source fidelity,
 * not coercion. Both operands stay s32: `mode` is the raw widened load that
 * feeds `mode - 0x17`, `m` is the masked state id that feeds the three
 * equality tests.
 *
 * Why the mask survives to bytes (mechanism, read from the RTL dumps):
 * combine sees (insn A) reg74 = zero_extend:SI(mem:HI) and (insn B)
 * reg75 = and:SI(reg74, 65535). It cannot substitute A into B because reg74 is
 * still live afterwards (`mode - 0x17`), so the AND is never brought into a
 * combination where nonzero_bits() could prove it redundant, and the standalone
 * andsi3 insn reaches the assembler as `andi $v1,$a0,0xffff`. Typing either
 * side narrow (u16 mode, or u16 m) lets combine fold the truncate/extend pair
 * into a copy or delete it outright.
 */
void func_8002304C(Unk80101EC8Record *obj, Vec4i32 *pos1, Vec4i32 *pos2, s32 *arg3)
{
  s32 *scratch = (s32 *) 0x1F8001B0;
  s32 count = 0;
  s32 lim;
  s16 *scratch_d;
  s16 *scratch_c = (s16 *) 0x1F8001C0;
  lim = 0x1F8002B8;
  scratch_d = (s16 *) 0x1F8001D0;
  loop:
  if (((pos1->vx != pos2->vx) || (pos1->vy != pos2->vy)) || (pos1->vz != pos2->vz))
  {
    if (func_8005344C(&pos1->vx, &pos2->vx, scratch, scratch_c, lim) == 0)
    {
      *pos1 = *pos2;
      goto done;
    }
    obj->unk_B1 = func_80054434();
    *pos1 = *(Vec4i32 *)scratch;
    func_8002EBDC((s16 *) arg3, scratch_c, arg3, -0x40, 0xE6);
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      scratch[12] = pos1->vx + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      scratch[13] = pos1->vy + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      scratch[14] = pos1->vz + (vel / 1024);
    }
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      pos2->vx += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      pos2->vy += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      pos2->vz += vel / 1024;
    }
    if (func_8005344C(&pos1->vx, scratch + 12, scratch, (s16 *)(scratch + 6), lim) == 0)
    {
      *pos1 = *(Vec4i32 *)(scratch + 12);
      scratch[8] = pos2->vx - pos1->vx;
      scratch[9] = pos2->vy - pos1->vy;
      scratch[10] = pos2->vz - pos1->vz;
      func_8002EBDC(scratch_d, scratch_c, (s32 *) scratch_d, 0,
                    (obj->unk_6A == 0x15) ? 0x80 : 0x100);
      {
        s16 vel_y = *((s16 *) (((u8 *) scratch) + 0x12));
        if (vel_y >= (-0x7FF))
        {
          s32 mode = obj->unk_6A;
          s32 m = mode & 0xFFFF;
          if (((((m != 8) && (m != 0x22)) && (((u32) (mode - 0x17)) >= 2)) && (m != 0xA)) && (obj->unk_72 == 0))
          {
            scratch[9] = 0;
          }
        }
      }
      pos2->vx = pos1->vx + scratch[8];
      pos2->vy = pos1->vy + scratch[9];
      count++;
      pos2->vz = pos1->vz + scratch[10];
      if (count < 4)
      {
        goto loop;
      }
    }
  }

  done:
  ;
}
s32 func_800233AC(Unk80101EC8Record *arg0, s32 *arg1) {
    s32 pos[3];
    s32 off[3];
    SVec4i16 out1;
    s32 out2[4];
    u32 bits;
    s32 a1_idx;
    s32 a0_idx;

    bits = arg0->unk_24.held;
    a1_idx = (bits >> 14) & 1;
    if (!(bits & 0x1000)) {
        a1_idx++;
    }
    a0_idx = (bits >> 15) & 1;
    if (!(bits & 0x2000)) {
        a0_idx++;
    }

    {
        /* FAKE: pointer alias to D_8008EB40 (pointer-alias-fake-exception). The row
         * pointer local puts the table address in a register at its own statement,
         * ahead of the a0_idx * 6 row offset, as the target does. Written directly
         * (D_8008EB40[a0_idx]), expr.c:5245 expands the array base under EXPAND_SUM
         * as a bare constant term that is forced to a register only after the index
         * insns. Direct D_8008EB40[a0_idx], D_8008EB40[a0_idx][a1_idx] and a flat
         * D_8008EB40 + a0_idx * 3 each miss; dumps and measurements:
         * memory/grind/judge-decl-cleanup/eb40-pointer-alias.md */
        s16 (*tbl)[3] = D_8008EB40;
        s32 px;
        s16 *row;
        s32 a1_val;

        px = arg0->unk_B8.vx;
        row = tbl[a0_idx];
        a1_val = row[a1_idx];

        pos[0] = px;
        pos[1] = arg0->unk_B8.vy - 0x64;
        pos[2] = arg0->unk_B8.vz;

        {
            s32 angle = (arg0->unk_1D8 + a1_val) & 0xFFF;
            s16 jv = Judge[angle];

            off[0] = px + jv / 4;
            off[1] = pos[1];
        }

        {
            s32 angle2 = (arg0->unk_1D8 + a1_val + 0x400) & 0xFFF;
            s16 jv2 = Judge[angle2];

            off[2] = pos[2] + jv2 / 4;
        }
    }

    if (func_80053614(pos, off, out2, &out1.vx, (s32)0x1F8002B8) == 0) {
        return 0;
    }

    arg0->unk_98 = out1;

    {
        s32 fwd_angle = ratan2(out1.vx, out1.vz);
        s32 fwd_800;

        pos[0] = arg0->unk_B8.vx;
        pos[1] = arg0->unk_B8.vy - 0x898;
        pos[2] = arg0->unk_B8.vz;

        fwd_800 = fwd_angle + 0x800;

        {
            s32 angle3 = fwd_800 & 0xFFF;
            s16 jv3 = Judge[angle3];
            off[0] = pos[0] + jv3 / 8;
        }

        off[1] = pos[1];

        {
            s32 angle4 = (fwd_angle + 0xC00) & 0xFFF;
            s16 jv4 = Judge[angle4];
            off[2] = pos[2] + jv4 / 8;
        }

        if (func_80053614(pos, off, out2, &out1.vx, (s32)0x1F8002B8) != 0) {
            return 0;
        }

        pos[0] = off[0];
        pos[1] = off[1] + 0x190;
        pos[2] = off[2];

        if (func_80053614(off, pos, out2, &out1.vx, (s32)0x1F8002B8) == 0) {
            return 0;
        }

        *arg1 = fwd_800;
        return 1;
    }
}
void func_80023648(Unk80101EC8Record *arg0) {
    u16 kind = arg0->unk_6A;
    /* FAKE: pointer alias to D_8008EB40 (pointer-alias-fake-exception), same
     * mechanism as func_800233AC: the row pointer local puts the table address in
     * a register ahead of the a0 * 6 row offset, as the target does (direct
     * D_8008EB40[a0] lets expr.c:5245 keep the base a constant term until after
     * the index insns). Dumps and measurements:
     * memory/grind/judge-decl-cleanup/eb40-pointer-alias.md */
    s16 (*tbl)[3];

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        s32 a2;
        u32 bits = arg0->unk_24.held;
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;
            s16 *row;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            tbl = D_8008EB40;
            row = tbl[a0];
            /* FAKE: index-first element address `a1[row]` (identical value to
             * `row[a1]` - C defines E1[E2] as *(E1+E2), so this is the same load),
             * mechanism: GCC 2.7.2 RTL expansion emits the operands of the
             * commutative PLUS in source order, so index-first flips the addu
             * operand order and the element pointer lands in $a2 as target does. */
            a2 = a1[row];

            if (D_800A38BA != 0 && arg0->unk_06 == 0) {
                func_8001F860(arg0, arg0->unk_1C8.vy + a2 / 4);
            } else {
                func_8001F860(arg0, arg0->unk_1D8 + a2);
            }
        } else {
            if (D_800A38BA != 0 && arg0->unk_06 == 0) {
                arg0->unk_14C = 0;
            }
        }

        {
            /* FAKE: the clamped |arg0->unk_150| is staged through the
             * existing `a2` (its D_8008EB40 table-entry value is dead here - it
             * was consumed by the func_8001F860 call above and is never read
             * again), mechanism: GCC 2.7.2 global.c - a multiply-set pseudo is
             * ONE allocno spanning all of its live ranges, so global_alloc seats
             * every staged value in a single hard reg ($a2) exactly as target
             * does; separate locals form separate allocnos that find_reg seats in
             * $a2/$a0/$a1. */
            a2 = arg0->unk_150;
            if (a2 < 0) {
                a2 = -a2;
            }
            if (a2 >= 0x401) {
                a2 = 0x400;
            }

            {
                s32 sub_result = (u16)arg0->unk_14E - a2;
                s32 div16 = arg0->unk_1A;
                s16 new_14e;
                s32 tbl_val;
                s32 mult_res;
                s32 limit;

                arg0->unk_14E = sub_result;
                if (div16 < 0) {
                    div16 += 15;
                }
                div16 >>= 4;
                new_14e = sub_result + div16;
                arg0->unk_14E = new_14e;

                tbl_val = D_800A310C[D_8008DA08[arg0->unk_0A]];
                /* FAKE: the second read of arg0->unk_1A is staged through
                 * the existing `sub_result` (its 0x14E difference is dead here -
                 * consumed by the store above and by new_14e), mechanism: GCC
                 * 2.7.2 global.c multiply-set pseudo / single allocno as above. */
                sub_result = arg0->unk_1A;
                mult_res = sub_result * tbl_val;
                limit = (mult_res << 4) >> 12;

                if (limit < (s16)new_14e) {
                    arg0->unk_14E = limit;
                } else if ((s16)new_14e < 0) {
                    arg0->unk_14E = 0;
                }

                {
                    s32 speed_prod = arg0->unk_14E * arg0->unk_44;
                    s16 sin_val = Judge[(arg0->unk_1C8.vy & 0xFFF)];

                    /* FAKE: the >>12 speed is staged through the existing `a2`
                     * (its clamped-|0x150| value is dead here - consumed by
                     * sub_result above), mechanism: GCC 2.7.2 global.c
                     * multiply-set pseudo / single allocno as above. */
                    a2 = speed_prod >> 12;

                    arg0->unk_D8.x += (sin_val * a2) >> 16;

                    {
                        s16 cos_val = Judge[((arg0->unk_1C8.vy + 0x400) & 0xFFF)];
                        arg0->unk_D8.z += (cos_val * a2) >> 16;
                    }
                }
            }
        }
    } else {
        if (arg0->unk_14E > 0) {
            if (kind != 0x22) {
                arg0->unk_14C = 0;
            }
            arg0->unk_14E = 0;
        }
    }
}
void func_800238C4(Unk80101EC8Record *arg0)
{
    s32 src[4];
    s32 dst[4];
    s32 out[4];
    s16 offsets[4];
    s16 offsets2[4];
    s32 s1;
    s32 dx_delta;
    s32 dz_delta;
    s32 scratchpad;
    s32 ok;
    if (arg0->unk_104.vy <= 0) {
        return;
    }
    {
        s32 kind = arg0->unk_6A;
        if (((u32) (kind - 0x17)) < 2u) {
            return;
        }
        if ((kind & 0xFFFF) == 0xA) {
            return;
        }
        if (arg0->unk_72 != 0) {
            return;
        }
        if ((kind & 0xFFFF) == 1) {
            return;
        }
        if ((kind & 0xFFFF) == 0x28) {
            return;
        }
    }
    scratchpad = 0x1F8002B8;
    src[0] = arg0->unk_B8.vx;
    src[1] = arg0->unk_B8.vy - 0xC8;
    src[2] = arg0->unk_B8.vz;
    dst[0] = arg0->unk_B8.vx;
    dst[1] = arg0->unk_B8.vy + 0x514;
    dst[2] = arg0->unk_B8.vz;
    s1 = func_80053614(src, dst, out, offsets, scratchpad);
    ok = 1;
    if (s1 != 0)
    {
        if ((out[1] - src[1]) >= 0x191)
        {
            ok = arg0->unk_6A != 0x22;
            goto ok_check;
        }
        if (offsets[1] < (-0x7FF))
        {
            goto ok_zero;
        }
        dst[0] += offsets[0] / 8;
        dst[2] += offsets[2] / 8;
        ok = func_80053614(src, dst, out, offsets2, scratchpad) == 0;
        goto ok_check;
    }
    goto ok_check;
    ok_zero:
    ok = 0;
    ok_check:
    if (!ok) {
        return;
    }
    if (s1 == 0)
    {
        offsets[0] = arg0->unk_B8.vx - arg0->unk_C8.vx;
        offsets[2] = arg0->unk_B8.vz - arg0->unk_C8.vz;
        if (offsets[0] < -0x40) {
            offsets[0] = -0x40;
        } else if (offsets[0] > 0x40) {
            offsets[0] = 0x40;
        }
        if (offsets[2] < -0x40) {
            offsets[2] = -0x40;
        } else if (offsets[2] > 0x40) {
            offsets[2] = 0x40;
        }
        dx_delta = offsets[0];
        dz_delta = offsets[2];
    }
    else
    {
        dx_delta = offsets[0] / 64;
        dz_delta = offsets[2] / 64;
    }
    {
        s32 kind;
        s1 = (arg0->unk_1C8.vy - ratan2(offsets[0], offsets[2])) & 0xFFF;
        if (s1 >= 0x800) {
            s1 = 0x1000 - s1;
        }
        kind = arg0->unk_6A;
        if (((((kind & 0xFFFF) == 0xF) || (((u32) (kind - 0x1C)) < 2u)) || (((u32) (kind - 0x1E)) < 2u)) || (((u32) (kind - 0x20)) < 2u))
        {
            if (s1 < 0x400) {
                arg0->unk_286 = 0;
                arg0->unk_94 = 0;
            }
            else {
                arg0->unk_286 = 1;
                arg0->unk_94 = 1;
            }
            arg0->other->unk_286 = 2;
            /* FAKE: the common tail `0x74 = 0xBC` + its control transfer duplicated into
             * this arm instead of falling through to the shared copy below,
             * mechanism: local-alloc block_alloc's hand-rolled quantity sort
             * (tools/gcc-2.7.2/local-alloc.c:1539-1563). With only the two quantities of
             * `lw parent` (refs 2, span 4, pri 5000) and `li 2` (refs 2, span 2, pri 10000)
             * the next_qty==2 path ranks the constant first and hands it $v0; the
             * duplicated tail puts a third quantity (the 0xBC load, pri 10000) in the same
             * block, and the next_qty==3 path's third comparison restores the parent
             * pointer to qty_order[0], so it takes $v0 and the constant takes $v1 - the
             * target seating. jump2 cross-jump re-merges the two copies, so the emitted
             * function is unchanged at 219 instructions. */
            arg0->unk_74 = arg0->unk_B8.vy;
            goto skip_74;
        }
        else if ((kind & 0xFFFF) == 0x11)
        {
            D_800A3769 = s1 < 0x400;
            D_800A3758 = arg0->index;
            goto skip_74;
        }
        else if (s1 < 0x400)
        {
            arg0->unk_286 = 0x12;
            arg0->unk_94 = 0;
        }
        else
        {
            arg0->unk_286 = 0x11;
            arg0->unk_94 = 1;
        }
        arg0->unk_74 = arg0->unk_B8.vy;
    }
    skip_74:
    arg0->unk_104.vx += dx_delta;
    arg0->unk_104.vz += dz_delta;
}
void math_RotMatrixZYXAngles(s32 arg0, s32 arg1, s32 arg2, MATRIX *arg3) {
    arg3->m[0][0] = 0x1000;
    arg3->m[0][1] = 0;
    arg3->m[0][2] = 0;
    arg3->m[1][0] = 0;
    arg3->m[1][1] = 0x1000;
    arg3->m[1][2] = 0;
    arg3->m[2][0] = 0;
    arg3->m[2][1] = 0;
    arg3->m[2][2] = 0x1000;
    RotMatrixX(arg0, arg3);
    RotMatrixY(arg1, arg3);
    RotMatrixZ(arg2, arg3);
}
void func_80023CB4(Unk80101EC8Record *arg0, s16 arg1) {
    s16 v;
    arg0->unk_31A += 1;
    v = arg0->unk_31A;
    if (v == 1) {
        arg0->unk_318 = arg1;
        arg0->unk_320.x = 0;
        arg0->unk_320.z = 0;
    }
    arg0->unk_31C = arg1;
    if (arg0->unk_152 == 0) {
        arg0->unk_152 = 1;
        arg0->unk_154 = arg0->unk_1D8;
    }
}
void func_80023D08(Unk80101EC8Record *arg0) {
    func_80023CB4(arg0, 0x200);
}
void func_80023D28(Unk80101EC8Record *arg0) {
    if (arg0->unk_104.vy < 0) {
        arg0->unk_1DC = 0;
        return;
    }
    ((Vec4i32 *)0x1F8001E0)->vx = arg0->unk_B8.vx;
    ((Vec4i32 *)0x1F8001E0)->vy = arg0->unk_B8.vy + 0x1F4;
    ((Vec4i32 *)0x1F8001E0)->vz = arg0->unk_B8.vz;
    arg0->unk_1DC = func_8005344C(&arg0->unk_B8.vx, (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s16 *)0x1F8001C0, 0x1F8002B8);
}
s32 func_80023DB8(Unk80101EC8Record *arg0) {
    s32 result;
    ((Vec4i32 *)0x1F8001E0)->vx = arg0->unk_B8.vx;
    ((Vec4i32 *)0x1F8001E0)->vy = arg0->unk_B8.vy + 5;
    ((Vec4i32 *)0x1F8001E0)->vz = arg0->unk_B8.vz;
    if (func_8005344C(&arg0->unk_B8.vx, (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s16 *)0x1F8001C0, 0x1F8002B8) != 0) {
        result = *(s16 *)0x1F8001C2 < -0x800;
    } else {
        result = 0;
    }
    return result;
}
void func_80023E40(Unk80101EC8Record *arg0) {
    s32 *s1 = (s32 *)0x1F8001B0;
    s32 a0;
    s32 v1;
    a0 = arg0->unk_6A;
    v1 = a0 & 0xFFFF;
    if (v1 == 8) goto done;
    if (v1 == 0x22) goto done;
    if ((u32)(a0 - 0x17) < 2 || v1 == 0x28 || v1 == 0xA) {
        s32 v0;
        s32 *v3 = (s32 *)0x1F8002B8;
        s1[0xC] = arg0->unk_B8.vx;
        s1[0xD] = arg0->unk_B8.vy + 0x1F40;
        s1[0xE] = arg0->unk_B8.vz;
        v0 = func_80053614(&arg0->unk_B8.vx, &s1[0xC], s1, (s16 *)&s1[4], (s32)v3);
        if (v0 == 0) goto done;
        arg0->unk_148 = s1[1];
        goto done;
    }
    arg0->unk_148 = arg0->unk_B8.vy;
done:;
}
/* [unk_0A][min(unk_272, 3)]: 4.12 scale of unk_1E into unk_20 (func_80023F08). */
extern s16 D_8008E0BC[27][4];
/* [D_8008D9EC[unk_0A]][unk_0E]: the move command func_80023F08 hands
 * func_80030A2C (ex cpu_set_move_command_and_dir, RESET sweep 2026-10-03). */
extern u8 D_8008D90C[28][8];
extern void func_800204C0(Unk80101EC8Record *);

/* Per-frame update of character `arg0`'s record from this frame's pad input:
 * converts the pad bits, advances the move frame, runs the move script's
 * command list (unk_50 / unk_7C, func_80021424 / func_80021A98), decodes the
 * current and next motion frames and blends their root offsets into unk_E8,
 * integrates the velocities (unk_104 / unk_134) into the position, resolves
 * it (func_8002304C), then refreshes the bone points, hit flags (unk_62,
 * unk_AE, unk_288) and damping and the per-frame side effects. */
void func_80023F08(s32 arg0, PadState *pad) {
    MotionFrame pose[2];
    Vec3i32 v[2];
    Vec4i32 pos;
    s16 ang[2];
    s32 dir;
    s16 alt;
    Unk80101EC8Record *rec;
    u16 cmd;
    u16 keys;
    u16 *ent;
    MoveScript *move;
    /* FAKE: one local reused for four values (no-new-park-categories.md
     * entry 1): the unk_14C clamp limit, the unk_1D8 / unk_1C8.vy angle gap
     * (folded to 0..0x800), the unk_14C turn step and the -1/0/1 stick side.
     * The target keeps all four in $a1; a local per value, or any partial
     * split, misses (global.c find_reg gives the split values other
     * registers). */
    s32 temp;
    s32 face;
    s32 face90;
    s32 blend;
    s32 cur_frame;
    s32 next;
    s32 first;
    s32 next_frame;
    s32 motion;
    s32 extra;
    s32 hit;
    s16 frac;
    s32 rest;
    s32 perp;

    rec = &D_80101EC8[arg0];
    rec->unk_3C++;
    rec->unk_24 = *pad;
    if (D_800A38BA != 0 && (arg0 == 0 || (arg0 == 1 && rec->unk_06 == 0))) {
        rec->unk_24.held = (rec->unk_24.held & 0xFFF) | ((rec->unk_24.held & 0x8000) >> 3) | ((rec->unk_24.held & 0x7000) << 1);
        rec->unk_24.pressed = (rec->unk_24.pressed & 0xFFF) | ((rec->unk_24.pressed & 0x8000) >> 3) | ((rec->unk_24.pressed & 0x7000) << 1);
        rec->unk_24.released = (rec->unk_24.released & 0xFFF) | ((rec->unk_24.released & 0x8000) >> 3) | ((rec->unk_24.released & 0x7000) << 1);
        rec->unk_24.unheld = (rec->unk_24.unheld & 0xFFF) | ((rec->unk_24.unheld & 0x8000) >> 3) | ((rec->unk_24.unheld & 0x7000) << 1);
    }
    if (D_800A38DC == 2 || D_800A38DC == 5 || D_800A38DC == 3 || (D_800A38DC == 0 && D_800A385C != 0)) {
        rec->unk_24.pressed &= ~0x100;
    }
    if (rec->unk_3C < 2) {
        rec->unk_24.held = rec->unk_24.pressed = rec->unk_24.released = 0;
        rec->unk_24.unheld = 0xFFFF;
    }
    rec->unk_20 = (rec->unk_1E * D_8008E0BC[rec->unk_0A][rec->unk_272 >= 4 ? 3 : rec->unk_272]) >> 12;
    if (rec->unk_7A == 1) {
        rec->unk_7A = 0;
    }
    func_8001F938(rec);
    frac = rec->unk_42 + rec->unk_44;
    rec->unk_42 = frac;
    if (frac >= 0x1000) {
        rec->unk_46 = 0;
        rec->unk_40 += frac >> 12;
        rec->unk_42 &= 0xFFF;
    } else {
        rec->unk_46 = 1;
    }
    if (rec->unk_286 >= 0) {
        /* the move-script record of unk_50's follow-up id is a u16 move-id
         * table indexed by event code (here unk_286; 0xD, 0x17/0x18 below) */
        u16 *table;

        if (rec->unk_6A == 0xA || rec->unk_6A == 0x17 || rec->unk_6A == 0x18) {
            rec->unk_72 = 1;
        }
        rec->unk_31A = 0;
        table = func_80021424(rec, rec->unk_50->unk_00, &rec->unk_5E);
        func_80021A98(arg0, func_80021424(rec, table[rec->unk_286], &rec->unk_5E), rec->unk_5E);
        rec->unk_286 = -1;
    }
    if ((rec->unk_24.held & 8) && (rec->unk_24.held & 0xF000)) {
        if ((rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x13 || rec->unk_6A == 0x30 || rec->unk_6A == 0x31) && func_800233AC(rec, &dir) != 0) {
            u16 *table;

            func_8001F860(rec, dir);
            rec->unk_31A = 0;
            table = func_80021424(rec, rec->unk_50->unk_00, &rec->unk_5E);
            func_80021A98(arg0, func_80021424(rec, table[0xD], &rec->unk_5E), rec->unk_5E);
            rec->unk_104.vx = 0;
            rec->unk_104.vy = 0;
            rec->unk_104.vz = 0;
        }
    }
    if (rec->unk_50->unk_07 < rec->unk_40) {
        if (rec->unk_7C != NULL) {
            rec->unk_5E = rec->unk_80;
            if (rec->unk_82 & 0x1000) {
                rec->unk_4C = 1;
            }
            func_80021A98(arg0, rec->unk_7C, rec->unk_80);
        } else {
            if ((rec->unk_50->unk_0A[0] & 0x2000) && D_800A38AE != arg0) {
                if (rec->unk_50->unk_0A[0] & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424(rec, rec->unk_50->unk_0A[1], &rec->unk_5E), rec->unk_5E);
            } else {
                if (rec->unk_50->unk_09 & 0x20) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424(rec, rec->unk_50->unk_02, &rec->unk_5E), rec->unk_5E);
            }
        }
    }
    if (rec->unk_7C != NULL) {
        if (rec->unk_50->unk_08 < rec->unk_40) {
            if (rec->unk_6A == 0x11) {
                D_800A376E = 1;
                D_800A36D8 = rec->unk_7C;
                D_800A381C = rec->unk_80;
                D_800A36CA = rec->unk_82;
            } else {
                rec->unk_5E = rec->unk_80;
                if (rec->unk_82 & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, rec->unk_7C, rec->unk_80);
            }
        }
    } else if (rec->unk_50->unk_08 >= rec->unk_40) {
        ent = rec->unk_50->unk_0A;
        while ((cmd = ent[0]) != 0) {
            /* FAKE (owner ruling Q90, .claude/rules/ordinary-c-judge-decidable.md):
             * the entry's class mask (one bit per character class unk_0A; bit set =
             * the entry applies) is a u32 local only for the int->u32 conversion.
             * With an int mask, fold-const.c:4437 rewrites (mask & (1 << cls)) != 0
             * into ((mask >> cls) & 1) != 0 (srav / andi); converted, the
             * rewrite does not apply and the target's li 1 / sllv / and stays
             * (0x800244E0..E8). */
            u32 mask;

            if (!(cmd & 0x8000) || ((mask = ent[2] | (ent[3] << 16)) & (1 << rec->unk_0A))) {
                switch (cmd & 0x30) {
                case 0x00:
                    keys = rec->unk_24.held;
                    break;
                case 0x10:
                    keys = rec->unk_24.pressed;
                    break;
                case 0x20:
                    keys = rec->unk_24.unheld;
                    break;
                case 0x30:
                    keys = rec->unk_24.released;
                    break;
                }
                if (cmd & 0x2000) {
                    keys = 0;
                }
                if (rec->unk_6A == 0x11 && D_800A38AE == arg0) {
                    keys = 0;
                }
                if ((cmd & 0xF) == 9) {
                    cmd = (cmd & 0xFFF0) | 3;
                    if (rec->other->unk_6A != 6 && rec->other->unk_40 < rec->other->unk_AB) {
                        keys = 0;
                    }
                } else if ((cmd & 0xF) == 0xA) {
                    cmd = (cmd & 0xFFF0) | 5;
                    if (rec->unk_26C == 0) {
                        keys = 0;
                    }
                }
                if ((keys >> (cmd & 0xF)) & 1) {
                    rec->unk_B0 = cmd;
                    if (cmd & 0x1000) {
                        rec->unk_4C = 1;
                    }
                    move = func_80021424(rec, ent[1], &alt);
                    if (rec->unk_50->unk_09 & 0x80) {
                        rec->unk_7C = move;
                        rec->unk_82 = cmd;
                        rec->unk_4C = 0;
                        rec->unk_80 = alt;
                    } else if (rec->unk_6A == 0x11) {
                        D_800A376E = 1;
                        D_800A36D8 = move;
                        D_800A36CA = cmd;
                        D_800A381C = alt;
                    } else {
                        rec->unk_5E = alt;
                        func_80021A98(arg0, move, alt);
                    }
                    break;
                }
            }
            if (cmd & 0xC000) {
                ent += 4;
            } else {
                ent += 2;
            }
        }
    }
    if (rec->unk_6A == 0xA || rec->unk_72 != 0) {
        if (func_80023DB8(rec) != 0) {
            if (rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
                u16 *table;

                table = func_80021424(rec, rec->unk_50->unk_00, &rec->unk_5E);
                func_80021A98(arg0, func_80021424(rec, table[rec->unk_94 != 0 ? 0x17 : 0x18], &rec->unk_5E), rec->unk_5E);
            } else if (rec->unk_72 == 0) {
                func_80021A98(arg0, func_80021424(rec, rec->unk_50->unk_02, &rec->unk_5E), rec->unk_5E);
            }
            rec->unk_72 = 0;
        }
    } else if (rec->unk_7A != 0 && (rec->unk_6C == 0x17 || rec->unk_6C == 0x18) && !(rec->unk_6A == 0x17 || rec->unk_6A == 0x18) && rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
        u16 *table;

        table = func_80021424(rec, rec->unk_50->unk_00, &rec->unk_5E);
        func_80021A98(arg0, func_80021424(rec, table[0x18], &rec->unk_5E), rec->unk_5E);
    }
    if (rec->unk_70 == 1) {
        func_8001F860(rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26 || rec->unk_6A == 0x24) {
            func_8001F860(rec, rec->unk_1D8);
            if (rec->unk_6A == 2 || rec->unk_6A == 0x24) {
                temp = (rec->unk_58[2] >> 4) * 0x88;
                if (rec->unk_14C > temp) {
                    rec->unk_14C = temp;
                } else if (rec->unk_14C < -temp) {
                    rec->unk_14C = -temp;
                }
            }
        }
    }
    if (rec->unk_46 == 0 && rec->unk_6A == 9 && rec->unk_A9 == rec->unk_40) {
        func_8001F860(rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x18 && rec->unk_6A == 0x18) {
        rec->unk_104.vx += (Judge[rec->unk_1C8.vy & 0xFFF] * D_8008DA94[rec->unk_0A]) >> 12;
        rec->unk_104.vz += (Judge[(rec->unk_1C8.vy + 0x400) & 0xFFF] * D_8008DA94[rec->unk_0A]) >> 12;
        rec->unk_104.vy += D_8008DA50[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x17 && rec->unk_6A == 0x17) {
        rec->unk_104.vy += D_8008DAD8[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x28) {
        s32 dv[3];
        s32 tgt[3];

        temp = (rec->unk_1D8 - rec->unk_1C8.vy) & 0xFFF;
        if (temp >= 0x800) {
            temp = 0x1000 - temp;
        }
        tgt[0] = rec->other->unk_18C.x - ((Judge[rec->unk_1D8 & 0xFFF] * temp) >> 14);
        tgt[1] = rec->other->unk_18C.y;
        tgt[2] = rec->other->unk_18C.z - ((Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * temp) >> 14);
        rec->unk_104.vy -= 0xFA;
        func_800200DC((s32 *)&rec->unk_180, tgt, rec->unk_104.vy, 0x1F, dv);
        rec->unk_104.vx += dv[0];
        rec->unk_104.vz += dv[2];
        rec->unk_72 = 1;
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_6A == 0x17 || rec->unk_6A == 0x18 || rec->unk_6A == 0x28 || rec->unk_6A == 0xA) {
        if (rec->unk_40 == rec->unk_50->unk_07 && rec->unk_104.vy > 0) {
            rec->unk_40 = rec->unk_50->unk_07 - 1;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x23) {
        rec->unk_B8.vy -= 0x898;
        rec->unk_D8.y -= 0x898;
        rec->unk_1F8.y += 0x898;
    }
    {
        /* FAKE: named intermediate (no-new-park-categories.md entry 6): unk_6A
         * is read into `state` before the unk_7A test, as the target loads it
         * (lhu 0x6A ahead of the beqz at 0x80024C14). Spelled with two direct
         * reads, the 6A load follows the branch and jump.c thread_jumps sends
         * the 0x23 test's unk_7A == 0 jump past this test (+2 insns); a u16
         * `state` also misses. */
        s32 state = rec->unk_6A;

        if (rec->unk_7A != 0 && state == 6) {
            rec->unk_86 = rec->unk_84;
        }
    }
    temp = (rec->unk_14C * rec->unk_44) / 24576;
    rec->unk_1C8.vy += temp;
    rec->unk_14C -= temp;
    if (rec->unk_7A != 0) {
        if (rec->unk_78 != 0) {
            func_80023D08(rec);
        } else if (rec->unk_6A == 0x1C) {
            func_80023CB4(rec, 0x400);
        } else {
            if (rec->unk_6C == 0x1C || rec->unk_6A == 0x11 || rec->unk_6C == 0x11 || rec->unk_6A == 0x15 || rec->unk_6A == 0xA || rec->unk_6A == 0x2D) {
                func_80023D08(rec);
            } else {
                switch ((rec->unk_6C << 8) | rec->unk_6A) {
                case 0x1525:
                case 0x1502:
                case 0x1322:
                case 0x1519:
                case 0x3022:
                case 0x1913:
                case 0x3122:
                    func_80023D08(rec);
                    break;
                }
            }
        }
    }
    next = rec->unk_40 + 1;
    if (next >= rec->unk_58[1]) {
        next = rec->unk_58[1] - 1;
    }
    if (rec->unk_50->unk_09 & 0x40) {
        motion = rec->unk_5E == 0 ? D_80101EC8[D_800A38AE].unk_4A + 1 : 0;
        if (rec->unk_6A == 0x11 && D_800A38AE != arg0) {
            extra = rec->unk_58[1];
        } else {
            extra = 0;
        }
    } else {
        motion = rec->unk_5E == 0 ? rec->unk_4A + 1 : 0;
        if (rec->unk_6A == 0x15 && D_8008D9EC[rec->unk_0A] != 0) {
            extra = rec->unk_58[1];
        } else {
            extra = 0;
        }
    }
    first = extra + *rec->unk_54;
    cur_frame = first + rec->unk_40;
    next_frame = first + next;
    rec->unk_64 = (motion << 14) | cur_frame;
    rec->unk_66 = (motion << 14) | next_frame;
    if (rec->unk_6A == 0x33) {
        pose[1] = D_800A3888[arg0][rec->unk_40];
        pose[0] = pose[1];
    } else {
        func_800198D0(motion, cur_frame, &pose[0], (u16 *)0x1F8001B0);
        func_800198D0(motion, next_frame, &pose[1], (u16 *)0x1F8001B0);
    }
    if (rec->unk_7A != 0 && (rec->unk_6A == 6 || rec->unk_6A == 0x14) && rec->unk_6C == 6) {
        MATRIX m1;
        MATRIX m2;
        s32 twist;

        math_RotMatrixZYXAngles(rec->unk_290.unk_06, rec->unk_290.unk_08, rec->unk_290.unk_0A, &m1);
        math_RotMatrixZYXAngles(pose[0].unk_06, pose[0].unk_08, pose[0].unk_0A, &m2);
        /* new frame's heading minus the previous frame's.  FAKE: spelled
         * -old + new so the old heading is computed first, as the target
         * does; new - old computes the new heading first (score 7). */
        twist = -ratan2(m1.m[0][0], m1.m[2][0]) + ratan2(m2.m[0][0], m2.m[2][0]);

        rec->unk_1C8.vy += twist;
        func_8001B690(arg0, twist);
    }
    func_8001F2E4(rec, &pose[0], &pose[1]);
    if (rec->unk_6A == 0x11) {
        rec->unk_154 = rec->unk_1C8.vy;
    }
    if (rec->unk_152 != 0) {
        face = rec->unk_154;
    } else {
        face = rec->unk_1C8.vy;
    }
    ang[0] = ang[1] = face;
    face90 = face + 0x400;
    v[0].x = (pose[0].unk_04 * Judge[(pose[0].unk_02 - face90 + 0x400) & 0xFFF]) >> 12;
    v[0].y = -pose[0].unk_00;
    v[0].z = (pose[0].unk_04 * Judge[(pose[0].unk_02 - face90) & 0xFFF]) >> 12;
    v[1].x = (pose[1].unk_04 * Judge[(pose[1].unk_02 - face90 + 0x400) & 0xFFF]) >> 12;
    v[1].y = -pose[1].unk_00;
    v[1].z = (pose[1].unk_04 * Judge[(pose[1].unk_02 - face90) & 0xFFF]) >> 12;
    v[0].x = (v[0].x * rec->unk_1A) >> 12;
    v[0].z = (v[0].z * rec->unk_1A) >> 12;
    v[1].x = (v[1].x * rec->unk_1A) >> 12;
    v[1].z = (v[1].z * rec->unk_1A) >> 12;
    if (rec->unk_6A != 8) {
        v[0].y = (v[0].y * rec->unk_1A) >> 12;
        v[1].y = (v[1].y * rec->unk_1A) >> 12;
    }
    if (rec->unk_31A != 0) {
        pose[1] = pose[0];
        ang[1] = ang[0];
        pose[0] = rec->unk_290;
        ang[0] = rec->unk_314;
        blend = rec->unk_318;
        rec->unk_66 = rec->unk_64;
        rec->unk_64 = rec->unk_316;
    } else {
        blend = rec->unk_42;
    }
    rest = 0x1000 - blend;
    rec->unk_68 = blend;
    if (rec->unk_31A != 0) {
        Vec3i32 vc;

        if (rec->unk_7A != 0) {
            rec->unk_D8.x += rec->unk_1F8.x - v[0].x;
            rec->unk_D8.z += rec->unk_1F8.z - v[0].z;
            rec->unk_320.x += rec->unk_1F8.x - v[0].x;
            rec->unk_320.z += rec->unk_1F8.z - v[0].z;
        }
        perp = ang[0] + 0x400;
        vc.x = (pose[0].unk_04 * Judge[(pose[0].unk_02 - perp + 0x400) & 0xFFF]) >> 12;
        vc.y = -pose[0].unk_00;
        vc.z = (pose[0].unk_04 * Judge[(pose[0].unk_02 - perp) & 0xFFF]) >> 12;
        vc.x = (vc.x * rec->unk_1A) >> 12;
        vc.y = (vc.y * rec->unk_1A) >> 12;
        vc.z = (vc.z * rec->unk_1A) >> 12;
        vc.x -= rec->unk_320.x;
        vc.z -= rec->unk_320.z;
        rec->unk_E8.x = (vc.x * rest + v[0].x * blend) >> 12;
        rec->unk_E8.y = (vc.y * rest + v[0].y * blend) >> 12;
        rec->unk_E8.z = (vc.z * rest + v[0].z * blend) >> 12;
    } else {
        rec->unk_E8.x = (v[0].x * rest + v[1].x * blend) >> 12;
        rec->unk_E8.y = (v[0].y * rest + v[1].y * blend) >> 12;
        rec->unk_E8.z = (v[0].z * rest + v[1].z * blend) >> 12;
    }
    if (rec->unk_7A != 0 && rec->unk_31A == 0) {
        rec->unk_D8.x += rec->unk_1F8.x - rec->unk_E8.x;
        rec->unk_D8.z += rec->unk_1F8.z - rec->unk_E8.z;
        if (rec->unk_6A == 0x23 || rec->unk_6A == 0xA) {
            rec->unk_D8.y += rec->unk_1F8.y - rec->unk_E8.y;
        }
    }
    func_80023648(rec);
    func_800238C4(rec);
    if (rec->unk_6A != 8 && rec->unk_6A != 0x22) {
        rec->unk_104.vy += 0x15;
    }
    {
        /* FAKE: named intermediate (no-new-park-categories.md entry 6): with
         * `state`, cse.c keeps this test's own li 8 / li 0x22 (the target
         * re-materialises them at 0x80025780 / 0x80025788); with direct rec->unk_6A
         * reads cse substitutes the previous test's constant pseudos. */
        s32 state = rec->unk_6A;

        if (state == 8 || state == 0x22) {
            rec->unk_134.vx -= rec->unk_98.vx / 256;
            rec->unk_134.vz -= rec->unk_98.vz / 256;
        }
    }
    if (rec->unk_7A == 0 || rec->unk_6A != 0x28) {
        rec->unk_104.vx = (rec->unk_104.vx * rec->unk_156) >> 12;
        rec->unk_104.vy = (rec->unk_104.vy * rec->unk_158) >> 12;
        rec->unk_104.vz = (rec->unk_104.vz * rec->unk_15A) >> 12;
    }
    if (rec->unk_104.vx >= -15 && rec->unk_104.vx <= 15) {
        rec->unk_104.vx /= 2;
    }
    if (rec->unk_104.vy >= -15 && rec->unk_104.vy <= 15) {
        rec->unk_104.vy /= 2;
    }
    if (rec->unk_104.vz >= -15 && rec->unk_104.vz <= 15) {
        rec->unk_104.vz /= 2;
    }
    rec->unk_D8.x += rec->unk_104.vx;
    rec->unk_D8.y += rec->unk_104.vy;
    rec->unk_D8.z += rec->unk_104.vz;
    if ((rec->unk_6A == 7 || rec->unk_6A == 0xD) && rec->unk_B4 == 0) {
        temp = (rec->unk_24.held & 0x1000) ? 1 : -((rec->unk_24.held & 0x4000) != 0);
        if (temp != 0) {
            rec->unk_134.vx /= 2;
            rec->unk_134.vz /= 2;
            rec->unk_134.vx += (Judge[rec->unk_1D8 & 0xFFF] - Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * temp * 3) / 128;
            rec->unk_134.vz += (Judge[(rec->unk_1D8 + 0x400) & 0xFFF] + Judge[rec->unk_1D8 & 0xFFF] * temp * 3) / 128;
        }
    }
    rec->unk_134.vx = (rec->unk_134.vx * rec->unk_15E) >> 12;
    if (rec->unk_134.vx >= -3 && rec->unk_134.vx <= 3) {
        rec->unk_134.vx = 0;
    }
    rec->unk_134.vy = (rec->unk_134.vy * rec->unk_160) >> 12;
    if (rec->unk_134.vy >= -3 && rec->unk_134.vy <= 3) {
        rec->unk_134.vy = 0;
    }
    rec->unk_134.vz = (rec->unk_134.vz * rec->unk_162) >> 12;
    if (rec->unk_134.vz >= -3 && rec->unk_134.vz <= 3) {
        rec->unk_134.vz = 0;
    }
    if (rec->unk_6A == 4 || rec->unk_6A == 0x14 || rec->unk_0C == 0x1F) {
        rec->unk_134.vx = 0;
        rec->unk_134.vy = 0;
        rec->unk_134.vz = 0;
    } else {
        rec->unk_D8.x += rec->unk_134.vx;
        rec->unk_D8.y += rec->unk_134.vy;
        rec->unk_D8.z += rec->unk_134.vz;
    }
    pos.vx = rec->unk_D8.x + rec->unk_E8.x;
    pos.vy = rec->unk_D8.y;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        pos.vy += rec->unk_E8.y;
    }
    pos.vz = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_C8 = rec->unk_B8;
    func_8002304C(rec, &rec->unk_B8, &pos, &rec->unk_104.vx);
    if (rec->unk_B1 != 7 && rec->unk_B1 != 0) {
        rec->unk_B2 = ((0x2A >> rec->unk_B1) ^ 1) & 1;
    }
    rec->unk_D8.x = rec->unk_B8.vx - rec->unk_E8.x;
    rec->unk_D8.y = rec->unk_B8.vy;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        rec->unk_D8.y -= rec->unk_E8.y;
    }
    rec->unk_D8.z = rec->unk_B8.vz - rec->unk_E8.z;
    rec->unk_F4.x = rec->unk_D8.x + rec->unk_E8.x;
    rec->unk_F4.y = rec->unk_D8.y + rec->unk_E8.y;
    rec->unk_F4.z = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_1F8 = rec->unk_E8;
    rec->unk_168.x = rec->unk_F4.x;
    rec->unk_168.y = rec->unk_D8.y - 0x384;
    rec->unk_168.z = rec->unk_F4.z;
    func_80023E40(rec);
    func_80041188(arg0, (u8 *)&pose[0], (u8 *)&pose[1], rec->unk_68, (MATRIX *)0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, (s32 *)&rec->unk_F4, (s16 *)&rec->unk_1C8, 0, rec->unk_148);
    scratchpad_Restore();
    if (rec->unk_86 == rec->unk_8E && (rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x16 || rec->unk_6A == 0x30 || rec->unk_6A == 0x13 || rec->unk_6A == 0x31)) {
        rec->unk_92 = 0;
    } else {
        rec->unk_92 = rec->unk_92 != 0 ? 1 : 2;
    }
    rec->unk_62 = 0;
    if (rec->unk_0C != 0x1F && rec->unk_96 == 0 && rec->unk_92 != 0) {
        rec->unk_62 = 1;
    }
    if ((D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) && (rec->unk_0E == 0 || rec->unk_0E == 1) && (D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
        rec->unk_62 |= 2;
    }
    if (rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) {
        if (func_8002798C(rec) == 0) {
            goto clear_8c;
        }
        goto set_8c;
    }
    /* FAKE (gotos): the goto-free spelling
     * (cond ? func_8002798C(rec) != 0 : (A || B)) does not match.  FAKE: the
     * second range test is written unk_A6 >= unk_40; spelled like the first
     * (unk_40 <= unk_A6) fold-const factors the two identical tests out of
     * the || */
    if (((rec->unk_0C == 0x1D || rec->unk_0C == 0xE) && (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_A1[1] < 0xFF && rec->unk_26C != 0 && rec->unk_40 >= rec->unk_A5 && rec->unk_40 <= rec->unk_A6)
        || (rec->unk_0A == 0xE && rec->unk_6A == 0x11 && D_800A38AE == arg0 && rec->unk_40 >= rec->unk_A5 && rec->unk_A6 >= rec->unk_40)) {
    set_8c:
        rec->unk_8C = rec->unk_8C != 0 ? 1 : 2;
        rec->unk_62 |= 4;
    } else {
    clear_8c:
        rec->unk_8C = 0;
    }
    if (D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) {
        if (D_800A38DC == 3 && arg0 == 1 && D_800A384C == 4) {
            if (rec->other->unk_0A == 1 || rec->other->unk_0A == 3 || rec->other->unk_0A == 4 || rec->other->unk_0A == 9 || rec->other->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else if ((D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
            if (rec->unk_0A == 1 || rec->unk_0A == 3 || rec->unk_0A == 4 || rec->unk_0A == 9 || rec->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else {
            goto skip_62;
        }
        if (rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 |= 0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
            rec->unk_62 |= 0x20;
        }
    }
skip_62:
    if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
        if ((rec->unk_0E == 0 || rec->unk_0E == 1) && rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 = (rec->unk_62 | 2) & ~0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
            rec->unk_62 = (rec->unk_62 | 8) & ~0x20;
            func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, 0);
        }
    }
    if (rec->unk_62 & 1) {
        func_80049718(rec->unk_12, arg0 * 2 + 0x8000, 0, 0);
    }
    if (rec->unk_62 & 4) {
        func_80049718(D_8008EB80[rec->unk_14], arg0 * 2 + 0x8001, 0, 0);
    }
    if (rec->unk_62 & 2) {
        func_80049A2C(rec->unk_12, arg0 * 2, (rec->unk_62 >> 4) & 1);
    }
    if (rec->unk_62 & 8) {
        func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, (rec->unk_62 >> 5) & 1);
    }
    rec->unk_290 = pose[0];
    rec->unk_314 = ang[0];
    rec->unk_316 = rec->unk_64;
    if (rec->unk_31A != 0) {
        if (rec->unk_31A >= 2) {
            rec->unk_31A = 1;
        }
        rec->unk_318 += rec->unk_31C;
        if (rec->unk_318 >= 0x1000) {
            rec->unk_31A = 0;
            rec->unk_290 = pose[1];
            rec->unk_314 = ang[1];
            rec->unk_316 = rec->unk_66;
            rec->unk_1F8 = v[1];
        }
    }
    rec->unk_1D8 = ratan2(rec->other->unk_F4.x - rec->unk_F4.x, rec->other->unk_F4.z - rec->unk_F4.z);
    rec->unk_24C = rec->unk_104;
    func_80023D28(rec);
    func_800207C8(rec, SPAD->unkA8[arg0], SPAD->unk00[arg0], SPAD->unk48[arg0]);
    if (rec->unk_3C == 1) {
        rec->unk_210[0] = SPAD->unk00[arg0][0];
    }
    if (rec->unk_8C == 2) {
        rec->unk_8C = 1;
        rec->unk_234[0] = SPAD->unk48[arg0][0];
        rec->unk_234[1] = SPAD->unk48[arg0][1];
    }
    if (rec->unk_92 == 2) {
        rec->unk_92 = 1;
        rec->unk_234[0] = SPAD->unk48[arg0][0];
        rec->unk_234[1] = SPAD->unk48[arg0][1];
    }
    if ((rec->unk_0E == 6 || rec->unk_0E == 7) && (rec->unk_62 & 1)) {
        rec->unk_25C = SPAD->unk00[arg0][0];
        rec->unk_268 = 1;
    } else {
        rec->unk_268 = 0;
    }
    rec->unk_114[0].vx = SPAD->unk00[arg0][0].x - rec->unk_210[0].x;
    rec->unk_114[0].vy = SPAD->unk00[arg0][0].y - rec->unk_210[0].y;
    rec->unk_114[0].vz = SPAD->unk00[arg0][0].z - rec->unk_210[0].z;
    rec->unk_114[1].vx = SPAD->unk48[arg0][0].x - rec->unk_234[0].x;
    rec->unk_114[1].vy = SPAD->unk48[arg0][0].y - rec->unk_234[0].y;
    rec->unk_114[1].vz = SPAD->unk48[arg0][0].z - rec->unk_234[0].z;
    rec->unk_1DA = func_8002FDB0(rec);
    hit = 0;
    if ((rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_AD != 0) {
        if ((rec->unk_40 >= rec->unk_A1[0] && rec->unk_40 <= rec->unk_A3[0]) || (rec->unk_40 >= rec->unk_A1[1] && rec->unk_40 <= rec->unk_A3[1])) {
            hit = 1;
        }
    }
    rec->unk_AE = hit;
    rec->unk_288[0] = 0;
    rec->unk_288[1] = 0;
    if (rec->unk_AD != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) {
            rec->unk_288[0] += 1;
            rec->unk_288[1] += 1;
            if (rec->unk_40 < rec->unk_A1[0]) {
                rec->unk_288[0] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[0]) {
                rec->unk_288[0] += 4;
            }
            if (rec->unk_40 < rec->unk_A1[1]) {
                rec->unk_288[1] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[1]) {
                rec->unk_288[1] += 4;
            }
        }
    }
    if (rec->unk_6A == 6) {
        rec->unk_15E = 0x600;
        rec->unk_160 = 0x600;
        rec->unk_162 = 0x600;
    } else if (rec->unk_6A == 8) {
        rec->unk_15E = 0x100;
        rec->unk_160 = 0x100;
        rec->unk_162 = 0x100;
    } else {
        rec->unk_15E = 0xC00;
        rec->unk_160 = 0xC00;
        rec->unk_162 = 0xC00;
    }
    if (rec->unk_1DC != 0) {
        rec->unk_156 = 0xD00;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xD00;
    } else {
        rec->unk_156 = 0xFC0;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xFC0;
    }
    if (rec->unk_96 == 0
        && ((rec->unk_7A != 0 && rec->unk_6C != 4 && rec->unk_6C != 0x14 && (rec->unk_6A == 4 || rec->unk_6A == 0x14))
            || (rec->unk_6A == 0x11 && arg0 != D_800A38AE && rec->unk_AA == rec->unk_40))) {
        MATRIX **bones = game_GetPlayerData(arg0);
        if (D_800A38DC != 0 || D_8008D9EC[D_80101EC8[0].unk_0A] == 0 || D_800A37A0 != 1 || arg0 != D_800A37A0) {
            func_80032854(arg0, 0x2E, &rec->unk_F4.x, 0);
        }
        if (rec->unk_0C != 0x1F) {
            func_80030A2C(rec, D_8008D90C[D_8008D9EC[rec->unk_0A]][rec->unk_0E], (Vec3i32 *)bones[0x12]->t);
        }
        rec->unk_96 = 1;
        if (D_800A3748 == -1) {
            D_800A3748 = arg0 == 0;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_0C == 0x1B && rec->unk_6A == 0xB && rec->unk_34D == 0) {
        rec->unk_286 = 0x1F;
    }
    if (rec->unk_46 == 0) {
        if (D_800A38DC != 5 && D_800A38DC != 2 && D_800A38DC != 3 && (D_800A38DC != 0 || D_800A385C == 0)) {
            if (rec->unk_6A == 0xB && rec->unk_40 == rec->unk_A7) {
                if (rec->unk_0C == 0x1B) {
                    if (rec->unk_34D != 0) {
                        rec->unk_34D--;
                    }
                } else if (rec->unk_26C != 0 && func_800307D0(rec) == rec->unk_14) {
                    rec->unk_8A = 0;
                }
            }
        }
        if (rec->unk_6A == 6) {
            if (func_80030BA8(rec) == rec->unk_14) {
                rec->unk_8A = rec->unk_26C;
            }
        } else if (rec->unk_26C != 0 && (rec->unk_6A == 0xC || rec->unk_6A == 0x2A) && rec->unk_40 == rec->unk_A8 && func_80030BA8(rec) == rec->unk_14) {
            rec->unk_8A = 1;
        }
        if (rec->unk_6A == 0x12 && rec->unk_40 == rec->unk_A7 && ((0x78 >> rec->unk_B1) & 1) && rec->unk_26C != 0) {
            func_80032064(rec, ((0x18 >> rec->unk_B1) & 1) ? 1 : 2);
        }
        if (rec->unk_6A == 0x10 && rec->unk_40 == rec->unk_AC && rec->unk_26C != 0 && rec->unk_34B != 0) {
            rec->unk_34A = 0xA;
            rec->unk_34B--;
            func_80032854(arg0, 0x31, &rec->unk_F4.x, 0);
        }
        if (rec->unk_46 == 0 && (rec->unk_6A != 0x11 || arg0 == D_800A38AE)) {
            rec->unk_62 |= 0x40;
            func_8003339C(rec);
        }
    }
    if (rec->unk_26C != 0) {
        rec->unk_62 |= 0x80;
    }
    if (rec->unk_96 != 0) {
        if (D_800A3834 != 0xD) {
            rec->unk_B3 = 4;
        }
    } else {
        rec->unk_B3 = 0;
    }
    if (rec->unk_0C == 0x1F) {
        rec->unk_B3 = 4;
    }
    func_80040304(arg0, rec->unk_B3);
    func_800204C0(rec);
    if (rec->unk_7A == 2) {
        rec->unk_7A = 0;
    }
    func_80039680(rec);
}

/* Tail word after func_80021424's five-entry compiler-generated switch table. */
const u32 D_80010428[1] = { 0x00000000 };

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A30EC = 0;
u8 D_800A30F0[4] = { 0, 0, 0, 0 };
s32 D_800A30F4[2] = { 0, 0 };
s8 D_800A30FC = -1;
s8 D_800A30FD = -1;
