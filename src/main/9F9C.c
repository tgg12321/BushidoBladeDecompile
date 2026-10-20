/* 93 game functions, among them math_SignExt12Div, math_FloorDiv2000,
 * math_RotMatrixZYXAngles and pad_ClearStateBits. .text 0x8001979C (ROM
 * 0x9F9C). Start boundary: PHASE (a jump-table phase change). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include <psxsdk/inline_o.h>
#include <psxsdk/gtemac.h>

extern s32 memcpy(s32 *, s32, s32);
extern s32 collision_GetHitTag(void);

INCLUDE_RODATA("asm/rodata", D_800100A4);

/* Motion bitstream reader: `cur` holds the unread bits left-aligned, `bits`
   how many of them are valid, `ptr` the next stream word. GETBITS reads `nb`
   bits into `dst`; GETBITS_PRE ORs them onto `pre` (the implicit top bit of
   an escape/VLC code). */
#define GETBITS_PRE(dst, nb, pre)                                              \
    {                                                                          \
        u32 top = (pre);                                                       \
        if (bits < (nb)) {                                                     \
            s32 need = (nb) - bits;                                            \
            u32 hi = cur >> (32 - bits);                                       \
            s32 left = 32 - need;                                              \
            cur = *ptr++;                                                      \
            dst = top | ((hi << need) | (cur >> left));                        \
            cur <<= need;                                                      \
            bits = left;                                                       \
        } else {                                                               \
            dst = top | (cur >> (32 - (nb)));                                  \
            cur <<= (nb);                                                      \
            bits -= (nb);                                                      \
        }                                                                      \
    }

#define GETBITS(dst, nb)                                                       \
    if (bits < (nb)) {                                                         \
        s32 need = (nb) - bits;                                                \
        u32 hi = cur >> (32 - bits);                                           \
        s32 left = 32 - need;                                                  \
        cur = *ptr++;                                                          \
        dst = (hi << need) | (cur >> left);                                    \
        cur <<= need;                                                          \
        bits = left;                                                           \
    } else {                                                                   \
        dst = cur >> (32 - (nb));                                              \
        cur <<= (nb);                                                          \
        bits -= (nb);                                                          \
    }

#define COPY33(d, s)                                                           \
    {                                                                          \
        u32 *from = (u32 *)(s);                                                \
        u32 *to = (u32 *)(d);                                                  \
        u32 k;                                                                 \
        for (k = 0; k < 33; k++) {                                             \
            *to++ = *from++;                                                   \
        }                                                                      \
    }

void func_8001979C(s32 arg0, u32 *ptr) {
    Unk800F1B18Rec *base;
    s32 i;
    u32 cur;
    s32 bits;

    bits = 0x20;
    base = &D_800F1B18[arg0];
    base->stream = ptr;
    cur = *ptr++;
    for (i = 0; i < 0x3F; i++) {
        GETBITS(base->pose[i + 3], 12);
    }
    for (i = 0; i < 0x3F; i++) {
        GETBITS(base->code[i + 3], 2);
    }
    for (i = 0; i < 4; i++) {
        base->slot[i].frame = -2;
    }
    base->ctr = 0;
}

extern s32 D_800A30EC;

/* Decode entry `arg1` of D_800F1B18[obj]'s stream into `out` (33 words). The
   table at stream + 0x70 holds a 24-bit offset per group of 8 entries (index
   arg1 >> 3), in bits from the table's start (0x380 bits past the stream's);
   an entry is its group's start plus arg1 & 7 delta steps. rec->ctr steps
   through the four slots; of the slot it steps to and the one before, one
   whose ->frame is arg1 is copied out (and the step is undone), one holding
   arg1 - 1 is continued from, and otherwise the entry is decoded from its
   group's start. When arg1 & 7 is below 7, the entry
   (->frame, ->pose, ->rate) and the reader state after it are kept in that
   slot. `work` holds the pose (+0x00), the rates (+0x84) and the codes
   (+0x108, copied from rec->code when D_800A30EC is 0). */
void func_800198D0(s32 obj, s32 arg1, MotionFrame *out, u16 *work) {
    Unk800F1B18Rec *rec;
    u8 *tbl;
    Unk800F1B18Slot *slot;
    Unk800F1B18Slot *prev;
    u32 *ptr;
    u32 cur;
    s32 bits;
    s32 var_7;
    s32 var_8;
    s32 var_9;
    /* Ruling 11 (ordinary-c-judge-decidable): two values, both loop indices --
     * the group-start loop's over the 63 values and the post-pass column
     * loop's. */
    s32 idx;
    s32 var_11;
    /* Ruling 11: two values, both loop indices -- the delta-step loop's and
     * the post-pass row loop's. */
    s32 idx2;
    s32 off;
    s32 shift;
    u16 *p;
    u16 code;
    /* Ruling 11: eight values, each read by GETBITS -- the three group-start
     * header words, the group start's per-value flag, the three delta-step
     * header words and case 3's 4-bit value (3 low bits and a sign bit). */
    u32 field;
    s16 x;

    var_7 = arg1 & 7;
    var_8 = arg1 >> 3;
    rec = &D_800F1B18[obj];
    tbl = (u8 *)rec->stream + 0x70;
    if (D_800A30EC == 0) {
        COPY33(work + 0x84, rec->code);
    }
    var_9 = rec->ctr;
    rec->ctr = var_9 + 1;
    slot = &rec->slot[(var_9 + 1) & 3];
    if (slot->frame == arg1) {
        COPY33(out, slot->pose);
        rec->ctr -= 1;
        return;
    }
    prev = &rec->slot[var_9 & 3];
    if (prev->frame == arg1) {
        COPY33(out, prev->pose);
        rec->ctr -= 1;
        return;
    }
    if (slot->frame != arg1 - 1) {
        if (prev->frame == arg1 - 1) {
            slot = prev;
        } else {
            off = ((tbl[var_8 * 3] << 16) | (tbl[var_8 * 3 + 1] << 8) |
                   tbl[var_8 * 3 + 2]) +
                  0x380;
            ptr = rec->stream + (off >> 5);
            shift = off & 0x1F;
            bits = 32 - shift;
            cur = *ptr++ << shift;
            COPY33(work, rec->pose);
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
    ptr = slot->ptr;
    bits = slot->bits;
    cur = slot->cur;
    COPY33(work, slot->pose);
    if (var_7 >= 2) {
        COPY33(work + 0x42, slot->rate);
    }
    idx2 = var_7 - 1;
decode:
    for (; idx2 < var_7; idx2++) {
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
        for (var_11 = 0; var_11 < 63; var_11++) {
            /* Ruling 11: four values -- the value's decoded delta, case 1's
             * magnitude, case 2's zero flag and case 3's magnitude. */
            s16 temp;

            code = work[var_11 + 0x87];
            if (code == 0) {
                continue;
            }
            switch (code) {
            case 1: {
                /* Ruling 11: two values, both bit counts -- the zero-run length
                 * and the suffix length (one less). */
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
                    /* Ruling 11: two values, both bit counts -- the zero-run
                     * length and the suffix length (one less). */
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
                work[var_11 + 0x45] = temp;
                work[var_11 + 3] += temp;
            } else {
                work[var_11 + 3] += work[var_11 + 0x45] + temp;
                work[var_11 + 0x45] += temp;
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
    if (var_7 < 7) {
        slot->frame = arg1;
        COPY33(slot->pose, work);
        COPY33(slot->rate, work + 0x42);
        slot->ptr = ptr;
        slot->bits = bits;
        slot->cur = cur;
    }
}

void font_PrintU16Triples(u16 *arg0) {
    s32 i;
    u16 *p;
    i = 0;
    p = arg0 + 2;
    do {
        i++;
        font_Printf(D_800100A4, arg0[0], p[-1], p[0]);
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

void camera_CalcEye(Rec44 *arg0, s32 *arg1) {
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
    RotMatrixX(-arg0->unk_10.vx, &m);
    RotMatrixY(-arg0->unk_10.vy, &m);
    RotMatrixZ(-arg0->unk_10.vz, &m);
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
    u32 var_3;
    s32 sp_tmp;
    dx = arg1[0] - arg2[0];
    dz = arg1[2] - arg2[2];
    while (
        (((u32)(dx + 0x4000)) > 0x8000U) || (((u32)(dz + 0x4000)) > 0x8000U)) {
        dx = dx / 2;
        dz = dz / 2;
    }
    dist_sq = (dx * dx) + (dz * dz);
    if (dist_sq < 0x400U) {
        var_3 = ((u32)((u8)(g_sqrt_table_u8[dist_sq]))) >> 3;
    } else {
        u32 shift_a;
        u32 shift_b;
        gte_Lzc(dist_sq, &sp_tmp);
        {
            /* FAKE: the -2 mask held in a local that then takes the masked
             * count; as `0x16 - (sp_tmp & -2)`: score 3 */
            s32 var_8 = -2;
            var_8 = sp_tmp & var_8;
            shift_a = 0x16 - var_8;
        }
        shift_b = shift_a >> 1;
        var_3 = (((u32)((u8)(g_sqrt_table_u8[dist_sq >> shift_a]))) << 16) >>
                (0x13 - shift_b);
    }
    arg0[0] = (s16)math_FloorDiv2000(arg2[0] + ((dx << 10) / ((s32)var_3)));
    arg0[2] = (s16)math_FloorDiv2000(arg2[2] + ((dz << 10) / ((s32)var_3)));
}

void camera_UpdateTwoFighterView(
    Vec3i32 *arg0, Vec3i32 *arg1, Unk80101EC8Record *arg2,
    Unk80101EC8Record *arg3);
extern u8 D_800A30F0[];
extern s32 D_800A30F4[];
typedef VECTOR CamVec;

/* Scratchpad work area (0x1F800000) used by camera_UpdateTwoFighterView: the
 * target yaw and vz angle that D_800F6608's unk_10.vy / vz ease toward, the
 * camera focus, the eye position camera_CalcEye computes, a fighter record's
 * collision_pos less 0xC8 in vy (the collision segment's start), and the hit
 * position, surface normal and work area collision_TestSegmentWithWork is given
 * (its 3rd/4th/5th arguments; its other callers pass a VECTOR hit and an s16[4]
 * normal). */
typedef struct {
    s16 unk0;     /* 0x00 */
    s16 yaw;      /* 0x02 */
    s16 scr04;    /* 0x04 */
    s16 unk6;     /* 0x06 */
    CamVec focus; /* 0x08 */
    CamVec eye;   /* 0x18 */
    CamVec scr28; /* 0x28 */
    s32 hit[8];   /* 0x38 */
    s16 nrm[4];   /* 0x58 */
    s32 unk60;    /* 0x60 */
} CamScratch;

/* Two-fighter camera for D_800F6608 (arg0/arg1 = the fighters' positions,
 * arg2/arg3 = the fighter records; caller fight_Update). Eases the focus
 * toward the fighters' midpoint (or arg0's position when D_800A3690 is set);
 * turns their separation (byte-LUT square root, GTE leading-zero count for
 * large inputs) into the value D_800F6608.w18 eases 1/12 toward (0xBB8 in
 * some arg2->move_kind states), clamps w18,
 * and sets a hysteresis flag for func_8003F1E4; eases unk_10.vy / vz toward
 * the facing yaw and zero; then per fighter bisects unk_10.vx (rot_x) so
 * the segment from its collision_pos less 0xC8 in vy to the eye passes
 * collision_TestSegmentWithWork's camera-collision test (step in D_800A30F4[],
 * blocked flag in D_800A30F0[]), and eases unk_10.vx toward the larger result,
 * clamped to 0x80..0x1C0. GTE: gte_Lzc (gte_ldlzc, gte_nop x2, gte_stlzc). */
void camera_UpdateTwoFighterView(
    Vec3i32 *arg0, Vec3i32 *arg1, Unk80101EC8Record *arg2,
    Unk80101EC8Record *arg3) {
    s32 lzc_out;
    CamScratch *scr;
    Rec44 *var_2;
    s32 dx, dy, dz;
    s32 x, y, z;
    s32 shift;
    u32 dist_sq;
    u32 dist;
    u32 q;
    s32 var_13;
    s32 var_14, var_15;
    s32 var_16;
    s32 p;
    /* work holds three values, all unk_10.vx (rot_x) quantities: the
     * per-fighter bisection angle (loop), the target angle max(var_14, var_15)
     * clamped to 0x80..0x1C0, and the final eased step (Ruling 11,
     * ordinary-c-judge-decidable). */
    s32 work;
    s32 i, j;

    scr = (CamScratch *)0x1F800000;
    var_2 = &D_800F6608;
    var_2->h30[0][0] = 0x64;
    var_2->h30[0][1] = 0;
    var_2->h30[0][2] = 0x64;
    var_2->h30[1][0] = 0x64;
    var_2->h30[1][1] = 0;
    var_2->h30[1][2] = 0x64;
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
    var_2->unk_00.x += (scr->focus.vx - var_2->unk_00.x) / 4;
    var_2->unk_00.y += (scr->focus.vy - var_2->unk_00.y) / 4;
    var_2->unk_00.z += (scr->focus.vz - var_2->unk_00.z) / 4;

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
            gte_Lzc(dist_sq, &lzc_out);
            lzcr = lzc_out;
        }
        {
            s32 var_22 = 0x16 - (lzcr & ~1);
            dist = (u32)(g_sqrt_table_u8[dist_sq >> var_22] << 16) >>
                   (0x13 - ((u32)var_22 >> 1));
        }
    }
    dist <<= shift;
    q = 0x2000000U / (dist + 0x4000) + 0x400;
    if (arg2->move_kind == 0x13 || arg2->move_kind == 0x1B ||
        arg2->move_kind == 0x30 || arg3->move_kind == 0x13 ||
        arg3->move_kind == 0x1B || arg3->move_kind == 0x30) {
        q += 0x1000;
    }
    var_13 = ((dist + q) << 7) / 100;
    if (dy < 0) {
        dy = -dy;
    }
    var_13 += dy;
    if (arg2->move_kind == 0xF || arg2->move_kind == 0x1C ||
        arg2->move_kind == 0x1D || arg2->move_kind == 0x1E ||
        arg2->move_kind == 0x1F || arg2->move_kind == 0x20 ||
        arg2->move_kind == 0x21) {
        var_13 = 0xBB8;
    }
    var_2->w18 += (var_13 - var_2->w18) / 12;
    if (!(arg2->move_kind == 0xF || arg2->move_kind == 0x1C ||
          arg2->move_kind == 0x1D || arg2->move_kind == 0x1E ||
          arg2->move_kind == 0x1F || arg2->move_kind == 0x20 ||
          arg2->move_kind == 0x21) &&
        var_2->w18 < 0x1770) {
        var_2->w18 = 0x1770;
    }
    if (var_2->w18 > 100000) {
        var_2->w18 = 100000;
    }
    if (var_2->b1E) {
        var_2->b1E = var_2->w18 >= 0x558D;
    } else {
        var_2->b1E = var_2->w18 >= 0x55F1;
    }
    func_8003F1E4(var_2->b1E);

    if (arg2->move_kind == 0x11) {
        scr->yaw = var_2->unk_10.vy;
    } else {
        scr->yaw = (0x400 - ratan2(dx, dz)) & 0xFFF;
    }
    scr->scr04 = 0;
    var_2->unk_10.vy += math_SignExt12Div(scr->yaw - var_2->unk_10.vy, 8);
    var_2->unk_10.vz += math_SignExt12Div(scr->scr04 - var_2->unk_10.vz, 8);
    var_16 = var_2->unk_10.vx;

    for (p = 0; p < 2; p++) {
        s32 hi, lo;

        work = var_16;
        var_2->unk_10.vx = work;
        camera_CalcEye(var_2, &scr->eye.vx);
        if (p != 0) {
            scr->scr28 = arg3->collision_pos;
        } else {
            scr->scr28 = arg2->collision_pos;
        }
        scr->scr28.vy -= 0xC8;
        if (collision_TestSegmentWithWork(
                (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit, scr->nrm,
                (s32)&scr->unk60) &&
            scr->nrm[1] < -0x320) {
            func_8001A67C(var_2->h30[p], (s32 *)&scr->eye, scr->hit);
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
            /* FAKE: cancellation pair (F6, no-new-park-categories): the extra
             * references to `hi` raise its allocation priority above work's, so
             * the bounds take $s0 and `work` $s1, as in the target; combine
             * folds the pair to nothing. Without: score 29. */
            hi++;
            hi--;
            for (i = 0; i < 2; i++) {
                s32 d = (work - hi) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                var_2->unk_10.vx = hi + d / 2;
                camera_CalcEye(var_2, &scr->eye.vx);
                if (collision_TestSegmentWithWork(
                        (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit,
                        scr->nrm, (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    work = var_2->unk_10.vx;
                } else {
                    hi = var_2->unk_10.vx;
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
                var_2->unk_10.vx = lo + d / 2;
                camera_CalcEye(var_2, &scr->eye.vx);
                if (collision_TestSegmentWithWork(
                        (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit,
                        scr->nrm, (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    func_8001A67C(var_2->h30[p], (s32 *)&scr->eye, scr->hit);
                    lo = var_2->unk_10.vx;
                } else {
                    work = var_2->unk_10.vx;
                }
            }
            D_800A30F0[p] = 0;
        }
        if (p != 0) {
            var_15 = work;
        } else {
            var_14 = work;
        }
    }
    work = (var_14 < var_15) ? var_15 : var_14;
    if (work < 0x80) {
        work = 0x80;
    }
    if (work > 0x1C0) {
        work = 0x1C0;
    }
    var_2->unk_10.vx = var_16;
    work = math_SignExt12Div(work - var_16, 8);
    var_2->unk_10.vx += work;
}

void camera_UpdatePadOffset(s32 *arg0) {
    g_camera_view_offset.unk_00.x = 0;
    g_camera_view_offset.unk_00.y = 0;
    g_camera_view_offset.unk_00.z = 0;
    g_camera_view_offset.unk_10.vx = 0;
    g_camera_view_offset.unk_10.vy = 0;
    g_camera_view_offset.unk_10.vz = 0;
    g_camera_view_offset.w18 = 0;
    if (D_800A38BA != 0 && D_800A3834 == 1) {
        if (*arg0 & 1) {
            D_800A37E0 = 1;
            if (*arg0 & 8) {
                g_camera_distance_offset = g_camera_distance_offset + 0x4CC;
            }
            if (*arg0 & 2) {
                g_camera_distance_offset = g_camera_distance_offset - 0x4CC;
            }
            if (g_camera_distance_offset < -0x1C00) {
                g_camera_distance_offset = -0x1C00;
            }
            if (g_camera_distance_offset >= 0x7401) {
                g_camera_distance_offset = 0x7400;
            }
            *arg0 = *arg0 & ~0xB;
        }
        {
            s32 v;
            v = g_camera_distance_offset;
            if (v < 0) {
                v = v + 0xF;
            }
            g_camera_view_offset.w18 = v >> 4;
        }
    }
    *arg0 = *arg0 & (s32)0xFFFEFFFE;
}

void camera_InitTwoFighterView(Unk80101EC8Record *a0, Unk80101EC8Record *a1) {
    g_camera_fight_view_mode = 0;
    D_800F6608.h30[0][0] = 0x64;
    D_800F6608.h30[0][1] = 0;
    D_800F6608.h30[0][2] = 0x64;
    D_800F6608.h30[1][0] = 0x64;
    D_800F6608.h30[1][1] = 0;
    D_800F6608.h30[1][2] = 0x64;
    func_8003F1E4(0);
    D_800F6608.unk_00.x = (a0->world_pos.x + a1->world_pos.x) / 2;
    D_800F6608.unk_00.y = (a0->world_pos.y + a1->world_pos.y) / 2;
    D_800F6608.unk_00.z = (a0->world_pos.z + a1->world_pos.z) / 2;
    D_800F6608.unk_10.vx = 0;
    D_800F6608.unk_10.vy = 0x400 - ratan2(a1->world_pos.x - a0->world_pos.x,
                                          a1->world_pos.z - a0->world_pos.z);
    D_800F6608.unk_10.vz = 0;
    D_800F6608.w18 = 0x1388;
    D_800F6608.b1E = 0;
}

void camera_InitTrackedFighterFocus(
    Unk80101EC8Record *a0, Unk80101EC8Record *a1) {
    g_camera_fight_view_mode = 0;
    D_800F5328.h30[0][0] = 0x64;
    D_800F5328.h30[0][1] = 0;
    D_800F5328.h30[0][2] = 0x64;
    D_800F5328.h30[1][0] = 0x64;
    D_800F5328.h30[1][1] = 0;
    D_800F5328.h30[1][2] = 0x64;
    func_8003F1E4(0);
    if (D_800A36F6 != 0) {
        a0 = a1;
    }
    D_800F5328.unk_00.x = a0->unk_180.x;
    D_800F5328.unk_00.z = a0->unk_180.z;
    {
        s32 v = a0->unk_180.y;
        D_800F5328.b40 = 0;
        D_800F5328.unk_00.y = v;
    }
}

void camera_UpdateTrackedFighterView(Unk80101EC8Record *obj) {
    /* FAKE: s2 holds &D_800F5328 in a saved register; through the symbol each
     * access re-forms the address (score 34). */
    Rec44 *s2 = &D_800F5328;
    s32 a2;
    s32 val;
    s32 var_3;

    func_8003F1E4(0);

    val = (obj->unk_198[0].y + obj->unk_198[1].y) / 2 - obj->unk_180.y;
    var_3 = val >= 0x391;

    if (obj->move_kind == 0x2A) {
        val = 0x200;
    } else {
        s2->unk_00.x = obj->unk_180.x;
        s2->unk_00.z = obj->unk_180.z;
        val = obj->unk_180.y;

        if (!var_3) {
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

        if (obj->move_kind == 0x2A) {
            val = 0x200;
        } else {
            val = (-obj->opponent_bearing - s2->unk_10.vy) & 0xFFF;

            if (val >= 0x800) {
                val = 0x1000 - val;
            }
            if (val >= 0x400) {
                val = 0x400;
            }

            val = (ratan2(obj->other->world_pos.y - a2, D_800A387C) *
                   (0x400 - val)) >>
                  10;
        }
    }

    {
        s16 old = s2->unk_10.vx;
        s32 diff = val - old;
        if (diff < 0) {
            diff += 7;
        }
        s2->unk_10.vx = old + (diff >> 3);
    }
    s2->unk_10.vz = 0;

    {
        s16 counter;
        val = -obj->orientation.vy;
        counter = D_800A36FC;

        if (counter != 0) {
            s16 old12 = s2->unk_10.vy;
            s32 diff = val - old12;
            if (diff < 0) {
                diff += 3;
            }
            {
                counter--;
                s2->unk_10.vy = old12 + (diff >> 2);
                D_800A36FC = counter;
            }

            {
                s32 var_12 = s2->h1C * 3;
                if (var_12 < 0) {
                    var_12 += 3;
                }
                s2->h1C = var_12 >> 2;
            }
        } else {
            s2->unk_10.vy = val;
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
    g_camera_fight_view_mode = 1;
    D_800F6608.b1F = 0;
    D_800F5328.b1F = 0;
    func_8003F1E4(0);
}

/* Q65: this file's statics (.sbss), in address order. */
static u16 D_800A3310;

void camera_UpdateReplayView(
    Rec44 *dst, Rec1C *a, Rec1C *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    Unk80101EC8Record *base = &D_80101EC8[D_800A3748];
    s32 dx;
    s32 dy;
    s32 dz;
    /* FAKE: cur / t are reused for the x, y and z steps, the pitch target and
     * the w18 step; one local per role re-seats the products and moves the
     * loads (score 23). */
    s32 cur;
    s32 var_8;
    s32 t;
    s32 var_11;
    if (dst->b1F == 0) {
        dst->b1F = 1;
        dst->unk_00.x = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
        dst->unk_00.y =
            (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
        D_800A3310 = 0;
        dst->unk_00.z = ((frac * a->h8) + (inv_frac * b->h8)) >> 12;
        dst->unk_10.vy = val;
        dst->unk_10.vx = 0x80;
        dst->unk_10.vz = 0;
        dst->w18 = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        return;
    }
    if ((base->unk_198[0].y + base->unk_198[1].y) / 2 - base->unk_180.y <
        0xC8) {
        D_800A3310 += 1;
    }
    var_8 = ((s16)D_800A3310) >= 0xB;
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
    if (var_8) {
        t = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        t = 0x80 - ((frac_s1 << 8) >> 12);
    }
    dst->unk_10.vx =
        dst->unk_10.vx + math_SignExt12Div(t - dst->unk_10.vx, 0x10);
    dst->unk_10.vy += math_SignExt12Div(val - dst->unk_10.vy, 0x10);
    dst->unk_10.vz = 0;
    if (var_8) {
        t = ((frac_s1 * 0x7D0) + (inv_s1 * 0x2EE0)) >> 12;
    } else {
        t = ((frac_s1 * 0x1F4) + (inv_s1 * 0x2EE0)) >> 12;
    }
    cur = dst->w18;
    var_11 = t - cur;
    if (var_11 < 0) {
        var_11 += 0xF;
    }
    dst->w18 = cur + (var_11 >> 4);
    dst->h30[0][0] = 0x64;
    dst->h30[0][1] = 0;
    dst->h30[0][2] = 0x64;
    dst->h30[1][0] = 0x64;
    dst->h30[1][1] = 0;
    dst->h30[1][2] = 0x64;
}

void func_8001BAE4(Rec1C *arg0, Rec1C *arg1, s32 arg2) {
    s32 temp_a2;
    s32 var_s3;
    s32 var_v1;
    s32 var_v0;

    if (D_800A387C < 0x2711) {
        var_s3 = (arg2 / 2) + 0x800;
    } else {
        var_s3 = 0x1000;
    }
    temp_a2 = ratan2(arg1->h4 - arg0->h4, arg1->h8 - arg0->h8);
    var_v1 = arg2;
    if (arg2 < 0) {
        var_v1 = arg2 + 3;
    }
    var_v0 = Judge[(var_v1 >> 2) & 0xFFF] * 3;
    if (var_v0 < 0) {
        var_v0 += 3;
    }
    camera_UpdateReplayView(&D_800F6608, arg0, arg1, arg2, var_s3,
                            (0x500 - temp_a2) - (var_v0 >> 2));
}

void func_8001BBD8(Rec1C *arg0, Rec1C *arg1, s32 arg2) {
    s32 temp_s0;
    temp_s0 = (D_800A387C < 0x2711) << 0xB;
    camera_UpdateReplayView(
        &D_800F5328, arg0, arg1, arg2, temp_s0,
        -0x200 - ratan2(arg1->h4 - arg0->h4, arg1->h8 - arg0->h8));
}

void camera_SetFighterFocusView(Unk80101EC8Record *arg0, s32 arg1) {
    func_8003F1E4(0);
    D_800F6608.unk_00 = arg0->unk_174;
    D_800F6608.unk_10.vx = 0x120;
    D_800F6608.unk_10.vy = arg1;
    D_800F6608.unk_10.vz = 0;
    D_800F6608.w18 = 0x1162;
}

void camera_SetInterpolatedFighterView(Unk80101EC8Record *arg0, s32 arg1) {
    s32 diff = 0x1000 - arg1;

    func_8003F1E4(0);

    /* FAKE: block copy of collision_pos's x / y / z (three lw, then three sw
       through one base); the three member assignments interleave the loads and
       stores (score 27). */
    D_800F6608.unk_00 = *(Vec3i32 *)&arg0->collision_pos;

    D_800F6608.unk_00.y -= 0x44C;

    D_800F6608.unk_10.vx = 0x100 - (arg1 * 288) / 4096;

    {
        s32 div4 = arg1 / 4;
        u16 rot_y = arg0->orientation.vy;
        s32 val;
        D_800F6608.w18 = (arg1 * 3000 + diff * 8000) >> 12;
        val = 0xB00 - div4;
        D_800F6608.unk_10.vz = 0;
        D_800F6608.unk_10.vy = val - rot_y;
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
    /* One local holds two values (Ruling 11, ordinary-c-judge-decidable):
     * (1) arg0 * 16, the bit offset of this player's half of the pad words;
     * (2) 0 or arg0 * 4, the bit offset of this player's config nibble.
     */
    s32 temp;
    s32 out;

    D_80101EC8[arg0 == 0].unk_34E =
        D_800A38DC == 2 && D_800A389A == 0 && arg0 == 0 &&
        ((g_pad_state.pressed >> 8) & 1);
    if (D_80101EC8[arg0].unk_06 != 0) {
        fighter_BuildCpuPad(arg0, arg1);
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
                if ((r & 1) && (buf[i] & 0x20) || (r & 2) && (buf[i] & 0x10) ||
                    (r & 4) && (buf[i] & 0x40) || (r & 8) && (buf[i] & 0x80)) {
                    out |= 0x20;
                }
            }
            {
                u8 g = D_800A3913;
                if ((g & 1) && (buf[i] & 0x20) || (g & 2) && (buf[i] & 0x10) ||
                    (g & 4) && (buf[i] & 0x40) || (g & 8) && (buf[i] & 0x80)) {
                    out |= 0x40;
                }
            }
            {
                u8 b = D_800A3914;
                if ((b & 1) && (buf[i] & 0x20) || (b & 2) && (buf[i] & 0x10) ||
                    (b & 4) && (buf[i] & 0x40) || (b & 8) && (buf[i] & 0x80)) {
                    out |= 0x80;
                }
            }
        } else {
            out = buf[i] & ~0xF0;
            {
                u8 r = D_80106A50.frec20[0] >> temp;
                if ((r & 1) && (buf[i] & 0x20) || (r & 2) && (buf[i] & 0x10) ||
                    (r & 4) && (buf[i] & 0x40) || (r & 8) && (buf[i] & 0x80)) {
                    out |= 0x20;
                }
            }
            {
                u8 g = D_80106A50.frec20[1] >> temp;
                if ((g & 1) && (buf[i] & 0x20) || (g & 2) && (buf[i] & 0x10) ||
                    (g & 4) && (buf[i] & 0x40) || (g & 8) && (buf[i] & 0x80)) {
                    out |= 0x40;
                }
            }
            {
                u8 b = D_80106A50.frec20[2] >> temp;
                if ((b & 1) && (buf[i] & 0x20) || (b & 2) && (buf[i] & 0x10) ||
                    (b & 4) && (buf[i] & 0x40) || (b & 8) && (buf[i] & 0x80)) {
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
    if (D_800A38DC == 5 &&
        (D_800A381E != 0 || D_800A3816 != 0 || D_800A37E1 != 0 ||
         D_800A38B8 != 0 || D_800A3920 != 0 || D_800A36E8 != 0)) {
        pad_ClearStateBits(arg1);
    }
    if (arg0 == 1 && D_800A38DC != 6) {
        arg1->held =
            (arg1->held & 0xFFF) | ((arg1->held & 0x8000) >> 2) |
            ((arg1->held & 0x2000) << 2) | ((arg1->held & 0x4000) >> 2) |
            ((arg1->held & 0x1000) << 2);
        arg1->pressed =
            (arg1->pressed & 0xFFF) | ((arg1->pressed & 0x8000) >> 2) |
            ((arg1->pressed & 0x2000) << 2) | ((arg1->pressed & 0x4000) >> 2) |
            ((arg1->pressed & 0x1000) << 2);
        arg1->released =
            (arg1->released & 0xFFF) | ((arg1->released & 0x8000) >> 2) |
            ((arg1->released & 0x2000) << 2) |
            ((arg1->released & 0x4000) >> 2) | ((arg1->released & 0x1000) << 2);
        arg1->unheld =
            (arg1->unheld & 0xFFF) | ((arg1->unheld & 0x8000) >> 2) |
            ((arg1->unheld & 0x2000) << 2) | ((arg1->unheld & 0x4000) >> 2) |
            ((arg1->unheld & 0x1000) << 2);
    }
}

void fight_InitSettings(void) {
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
    u16 v = D_80101EC8[0].move_kind;
    if (v == 0x32 || v == 0x11) {
        motion_ResetVariantIndex(0);
        {
            MoveScript *v0 = motion_GetDefaultVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            func_80021A98(0, v0, 0);
        }
    }
}

void func_8001C51C(void) {
    MoveScript *v0;
    func_8001C4C0();
    if (D_800A38DC == 3 && D_800A3728 != 0) {
        fighter_Init(1, (s8)D_80102778.unk_4[5], (s8)D_80102778.unk_4[1],
                     (s8)D_80102778.unk_4[3], 0);
    } else {
        fighter_Init(1, (s8)D_80102778.unk_4[5], (s8)D_80102778.unk_4[1],
                     (s8)D_80102778.unk_4[3], 1);
    }
    func_80022F34();
    motion_ResetVariantIndex(1);
    v0 = motion_GetDefaultVariantScript(1);
    D_80101EC8[1].move_is_common = 0;
    func_80021A98(1, v0, 0);
    D_800A382E = 0;
    D_800A3748 = -1;
    object_FreeAttachedObjects();
    func_80030D04();
    camera_InitTwoFighterView(&D_80101EC8[0], &D_80101EC8[1]);
    replay_Reset();
    func_80021280(1);
}

/* Initialise record 0 of D_80101EC8 (Unk80101EC8Record). */
void func_8001C624(void) {
    Unk80101EC8Record *e = D_80101EC8;
    VECTOR local;
    s32 x, y, z;

    fight_GetStartPosition(0, &e->base_pos.x, (s32)D_800A38E0);
    fight_GetStartPosition(1, &local.vx, (s32)D_800A38E0);
    e->root_offset.x = 0;
    x = e->base_pos.x;
    y = e->base_pos.y;
    z = e->base_pos.z;
    e->root_offset.y = -0x384;
    e->root_offset.z = 0;
    e->world_pos.x = x;
    e->world_pos.y = y - 0x384;
    e->world_pos.z = z;
    e->collision_pos.vx = x;
    e->collision_pos.vy = y;
    e->collision_pos.vz = z;
    e->prev_collision_pos = e->collision_pos;
    e->unk_1F8 = e->root_offset;
    e->unk_104.vx = 0;
    e->unk_104.vy = 0;
    e->unk_104.vz = 0;
    e->unk_24C = e->unk_104;
    /* FAKE: player 1's start position is read into local and raised by 0x384
     * as player 0's is, but never used (the target keeps these dead stores,
     * written with libgpu's setVector); without it: score 22. */
    setVector(&local, local.vx, local.vy - 0x384, local.vz);
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
    e->unk_148 = e->collision_pos.vy;
    softbody_RequestSnap(0);
}

void func_8001C820(void) {
    s16 *s0 = &D_80101EC8[0].unk_0A;
    s32 a0;
    if (D_8008D9EC[*s0] != 0) {
        if (D_800A37A0 == 1)
            return;
    }
    if (D_800A38DC != 0)
        return;
    if (D_800A3712 != 0)
        return;
    a0 = 0x56;
    if (D_800A3680 != D_800A3671) {
        if (rand() & 1) {
            a0 = 0x57;
        } else {
            a0 = 0x58;
        }
    }
    /* FAKE: player 1's world_pos reached from s0 (player 0's unk_0A) so the
       call reuses s0's register; &D_80101EC8[1].world_pos.x rematerialises the
       address (score 11). */
    snd_QueuePositionalSound(a0, (s32 *)((u8 *)s0 + 0x536));
}

void func_8001C8DC(void);

void func_8001C8DC(void) {
    u8 prev;
    MoveScript *var_1;

    if (D_80101EC8[0].is_defeated != 0 || D_80101EC8[1].is_defeated != 0) {
        D_800A382E++;
    } else if (
        D_800A38DC == 0 && D_800A385C != 0 && D_80101EC8[0].unk_B2 == 1) {
        func_8003B56C(3);
    }
    if (D_800A382E < 0x3D)
        goto end;

    switch (D_800A38DC) {
    case 0:
        if (D_80101EC8[0].is_defeated != 0)
            break;
        if (D_800A3680 != 0) {
            if (--D_800A3680 == 0) {
                snd_StopAllVoices();
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
        if (D_800A385C == 0)
            break;
        func_8003B56C(2);
        goto end;
    case 3:
        if (D_80101EC8[0].is_defeated != 0)
            break;
        if (func_80033DF4() == 0)
            goto end;
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
            var_1 = motion_GetCurrentVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            func_80021A98(0, var_1, 0);
        }
        func_8001C51C();
        if (D_800A384C < 4) {
            func_80041BF4(D_800A38EC, D_800A38ED, D_800A38EE);
        }
        goto end;
    case 2: {
        s16 t;
        u8 *p;
        if ((t = D_80101EC8[0].is_defeated) == 0 ||
            D_80101EC8[1].is_defeated == 0) {
            /* FAKE: second handle to D_800A37D2 (pointer-alias-fake-exception,
             * Q63): the target sets the pair's base in its own register before
             * the index; indexing &D_800A37D2 at both sites: score 17; each
             * byte by its own symbol: score 27. */
            p = &D_800A37D2;
            /* FAKE: indexes past D_800A37D2 into D_800A37D3 (owner Q63, this
             * byte pair only): the target also reaches each byte by its own
             * symbol, which no single array or struct gives; each byte by its
             * own symbol here: score 13. */
            p[t != 0]++;
        }
    }
        D_800A3670 = 1;
        D_800A38DF =
            fight_FindNearestStartPair(&D_80101EC8[D_800A3748].world_pos);
        D_800A3834 = 0;
        goto end;
    case 4:
    case 6: {
        s16 t;
        u8 *p;
        if ((t = D_80101EC8[0].is_defeated) == 0 ||
            D_80101EC8[1].is_defeated == 0) {
            /* FAKE: second handle, as above (owner Q63); indexing &D_800A37D2
             * here: score 8. */
            p = &D_800A37D2;
            /* FAKE: indexes past D_800A37D2 into D_800A37D3, as above (owner
             * Q63); each byte by its own symbol here: score 14. */
            p[t != 0]++;
        }
    } break;
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

void func_8001CD68(Unk8001CD68Rec *arg0) {
    s32 val = D_800A3858;

    if (val > 0x2BF1F) {
        arg0->unk_0 = 99;
        arg0->unk_2 = 59;
        arg0->unk_3 = 99;
        return;
    }
    {
        s32 var_1 = val / 1800;
        s32 var_2 = val / 30 - var_1 * 60;
        arg0->unk_2 = var_2;
        {
            s32 var_3 = (D_800A3858 % 30) * 100 / 30;
            arg0->unk_0 = var_1;
            arg0->unk_3 = var_3;
        }
    }
}

/* P1/P2 round scores and tiebreakers, declared here as [2] arrays because
 * func_8001CE60 indexes them by player (D_800A38B0); fight_RecordMatchResult's
 * declarations in src/main/17AFC.c give the same bytes as single u8s
 * (per-file declarations: Q21-Q25, no-new-park-categories aggregate-merge
 * exception). No single declaration compiles both files' accesses with an
 * admissible spelling. Under scalars, func_8001CE60's player-indexed accesses
 * are not produced (conditional selects compile to separate direct accesses;
 * a pointer pun is refused). Under an aggregate, every measured counting
 * spelling of fight_RecordMatchResult (Q22/Q23 set-asides excluded) misses:
 * constant subscripts put element 0 behind a base register, and the
 * index-variable and regrouped-condition spellings miss its match-result stores
 * or compares; the dummy-index and pointer-alias spellings that match are
 * refused/set aside (Q22/Q23). */
extern u8 D_800A3898[2];
extern u8 D_800A38AA[2];

void func_8001CE60(void) {
    Unk8001CD68Rec buf;

    if (D_800A38DC == 1) {
        g_prim_buf_cursor +=
            func_8005E51C(D_800A3783, (s32)g_prim_buf_cursor, 1) / 4;
    } else if (D_800A38DC == 3) {
        if (D_80101EC8[0].is_defeated == 0 &&
            (D_80101EC8[1].is_defeated == 0 || D_800A38E2 != 100)) {
            D_800A3858++;
            if (D_800A3858 > 0x2BF20) {
                D_800A3858 = 0x2BF20;
            }
        }
        func_8001CD68(&buf);
        g_prim_buf_cursor +=
            func_8005D814(&buf, D_800A38E2, (s32)g_prim_buf_cursor, 1) / 4;
    } else if ((D_800A38DC == 2 && D_800A389A == 1) || D_800A38DC == 4) {
        g_prim_buf_cursor +=
            func_8005E098(D_800A37D2, D_800A37D3, (s32)g_prim_buf_cursor, 1) /
            4;
    } else if (D_800A38DC == 5) {
        /* temp holds two values (Ruling 11, ordinary-c-judge-decidable): the
         * announcement length in frames (0x50 after a draw, 0x64 otherwise),
         * then the match clock's frames left for the on-screen timer. */
        s32 temp;

        if (D_800A381E != 0) {
            if (D_800A381E == 0x2D) {
                snd_QueueSound(0xA3, 0x7F, 0x7F);
            }
            if (++D_800A381E == 0x50) {
                D_800A381E = 0;
                fight_RecordMatchResult();
                D_800A36E8 = 1;
            }
        } else if (D_800A3816 != 0) {
            if (++D_800A3816 == 0x46) {
                snd_QueueSound(0x9D, 0x7F, 0x7F);
            } else if (D_800A3816 == 0x82) {
                D_800A3816 = 0;
                D_800A3670 = 1;
                D_800A3834 = 0;
            }
        } else if (D_800A37E1 != 0) {
            g_prim_buf_cursor +=
                func_8005FA98(2, (s32)g_prim_buf_cursor, 1) / 4;
            if (++D_800A37E1 == 0x3C) {
                D_800A37E1 = 0;
                if (D_800A38B0 != 2) {
                    snd_QueueSound(0xA0, 0x7F, 0x7F);
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
            g_prim_buf_cursor +=
                func_8005FA98(1, (s32)g_prim_buf_cursor, 1) / 4;
            if (++D_800A3920 == 0x1E) {
                snd_QueueSound(0xA1, 0x7F, 0x7F);
            }
            if (D_800A3920 == 0x69) {
                snd_QueueSound(0xA2, 0x7F, 0x7F);
            }
            if (D_800A3920 == 0x5A || D_800A3920 == 0xB4) {
                D_800A3920 = 0;
                if (D_800A3898[D_800A38B0 == 0] == g_fight_win_target) {
                    fight_RecordMatchResult();
                    D_800A36E8 = 1;
                } else {
                    D_800A3670 = 1;
                    D_800A3834 = 0;
                }
            }
        } else if (D_800A391E != 0) {
            if (--D_800A391E == 0) {
                snd_QueueSound(0x9C, 0x7F, 0x7F);
            }
        } else if (D_800A36E8 != 0) {
            if (g_fight_match_results[g_fight_match_cursor - 1] == 2) {
                if (D_800A36E8 == 1) {
                    snd_QueueSound(0xA4, 0x7F, 0x7F);
                }
                temp = 0x50;
            } else {
                if (D_800A36E8 == 1) {
                    snd_QueueSound(0xA6, 0x7F, 0x7F);
                }
                if (D_800A36E8 == 0x14) {
                    snd_QueueSound(
                        D_8008D9EC[D_80101EC8[g_fight_match_results
                                                  [g_fight_match_cursor - 1]]
                                       .unk_0A]
                            ? 0xA8
                            : 0xA7,
                        0x7F, 0x7F);
                }
                temp = 0x64;
            }
            if (++D_800A36E8 == temp) {
                D_800A36E8 = 0;
                fight_AdvanceMatch();
            }
        } else if (
            D_80101EC8[0].is_defeated != 0 && D_80101EC8[1].is_defeated != 0) {
            D_800A3816 = 1;
        } else if (D_80101EC8[1].is_defeated != 0) {
            D_800A38B0 = 1;
            D_800A3920 = 1;
            D_800A3898[D_800A38B0 ^ 1]++;
        } else if (D_80101EC8[0].is_defeated != 0) {
            D_800A38B0 = 0;
            D_800A3920 = 1;
            D_800A3898[D_800A38B0 ^ 1]++;
        } else if ((u16)D_80101EC8->move_kind == 6 ||
                   (u16)D_80101EC8[1].move_kind == 6) {
            D_800A3816 = 0x3C;
        } else if (D_80101EC8[0].unk_B1 == 2) {
            u16 id;

            snd_QueueSound(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[0].move_kind;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 ||
                id == 0x1A || id == 0x18) {
                D_800A38B0 = 0;
            } else {
                D_800A38B0 = 2;
            }
        } else if (D_80101EC8[1].unk_B1 == 2) {
            u16 id;

            snd_QueueSound(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[1].move_kind;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 ||
                id == 0x1A || id == 0x18) {
                D_800A38B0 = 1;
            } else {
                D_800A38B0 = 2;
            }
        } else if (D_800A36CC != 0) {
            D_800A38F4++;
            if (D_800A38F4 >= D_800A36CC * 30) {
                snd_QueueSound(0x9E, 0x7F, 0x7F);
                D_800A381E = 1;
            }
        }
        if (D_800A36CC != 0) {
            temp = D_800A36CC * 30 - D_800A38F4;
            buf.unk_2 = temp / 30;
            buf.unk_3 = temp % 30 * 100 / 30;
        }
        g_prim_buf_cursor +=
            func_8005F1C8(&buf,
                          D_800A3898[0] | (D_800A38AA[0] << 8) |
                              (D_800A3898[1] << 4) | (D_800A38AA[1] << 12),
                          (s32)g_prim_buf_cursor, 1) /
            4;
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

    if (D_800A36A4 != D_800A390E ||
        D_8008E5A8[(s8)D_80102778.unk_4[0]] != D_800A30FC ||
        D_8008E5A8[(s8)D_80102778.unk_4[1]] != D_800A30FD) {
        func_80020D38();
        func_80046EDC(D_800A36A4, s2);
        func_8002906C();
        snd_CloseListedVabs();

        s1 = func_8005BA8C(s2, D_800A36A4, D_8008E5A8[(s8)D_80102778.unk_4[0]],
                           D_8008E5A8[(s8)D_80102778.unk_4[1]]);

        D_800A390E = D_800A36A4;
        D_800A30FC = D_8008E5A8[(s8)D_80102778.unk_4[0]];
        D_800A30FD = D_8008E5A8[(s8)D_80102778.unk_4[1]];

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        memcpy(s0, s2, s1);
        snd_RebaseListedVabs((s32)s0 - s2);
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
    s0 = &D_80104F38;
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
    s0 = &D_80104F38;
    memcpy(s0, (s32)0x80190800, s1);
    snd_VabFakeOpen8And4((s32)s0 - s2);
}

void func_8001DA2C(void) {
    snd_ResetVoicesAndQueue();
    snd_StopVoicesAndReverb();
    if (D_800A38DC == 5) {
        func_8005B9C4();
    }
    if (D_800A38DC == 3) {
        func_8005B868();
    }
}

void snd_StartFightCdAudio(void) {
    snd_SerialMixOn();
    if (func_800167D4()) {
        return;
    }
    switch (D_800A38DC) {
    case 0:
    case 1:
    default:
        break;
    case 4:
        cdrom_StartAudioEntry((&D_8008D518)[D_800A36A4]);
        break;
    case 3:
        if (D_8008D9EC[D_80101EC8[0].unk_0A] != 0) {
            cdrom_StartAudioEntry(9);
        } else {
            cdrom_StartAudioEntry(8);
        }
        break;
    case 2:
        if (D_800A389A != 0) {
            cdrom_StartAudioEntry(0xA);
        } else {
            cdrom_StartAudioEntry(0xB);
        }
        break;
    case 5:
        break;
    }
    cdrom_SetAudioRepeat(1);
}

s32 func_8001DB58(void) {
    s32 v = D_800A38DC;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return func_800167D4();
    }
    return 1;
}

void func_8001DB9C(void) {
    ndata_LoadPack(D_8008D9EC[D_80101EC8[0].unk_0A] < 1, (s32)0x80190800);
    D_800A38C4[1] = 0xFFFF;
}

void func_8001DBE4(void) {
    s32 i;

    if (D_800A3768 != DISP_ACTIVE) {
        return;
    }
    comb_BeginWait();
    if (!(D_800A38F8 > D_800A37A0)) {
        do {
            comb_ExchangeEmptyPad();
            func_800174F4();
            VSync(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            comb_ExchangeEmptyPad();
            i += 1;
            func_800174F4();
            VSync(2);
        } while (i < 15);
    }
    comb_EndWait();
    gpu_HideResetAndClear();
    gpu_SetDispMaskOn();
}

extern void motion_LoadFighterData(s32, s32, s32, s32);
extern void func_80021210(void);
extern void camera_InitTwoFighterView(Unk80101EC8Record *, Unk80101EC8Record *);
extern void camera_InitTrackedFighterFocus(
    Unk80101EC8Record *, Unk80101EC8Record *);

void fight_Init(void) {
    s32 i;
    s32 addr;

    snd_ResetVoicesAndQueue();
    if (D_800A3768 != DISP_ACTIVE) {
        gpu_HideResetAndClear();
        gpu_SetDispMaskOn();
    }
    func_800174F4();
    gpu_ResetGraphMode1();
    gpu_RestoreObjectClutOffsets();
    object_ResetPool();
    func_80032040();
    func_8003F218(D_800A38BA);
    SetGeomScreen(math_FovToScreenDist(D_800A38BA != 0 ? 0x50 : 0x2D));
    for (i = 0; i < 2; i++) {
        if (D_800A38DC != 0) {
            func_80041604(0, 0);
        }
        fighter_Init(i, (s8)D_80102778.unk_4[4 + i], (s8)D_80102778.unk_4[i],
                     (s8)D_80102778.unk_4[2 + i], 0);
        if (D_800A38BA != 0 && D_800A36F6 == i) {
            gpu_MoveObjectClutsToFogBlock(i == 0);
        }
    }
    softbody_RequestSnap(0);
    softbody_RequestSnap(1);
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
                    func_800494D4(
                        i,
                        D_8008E6A4[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
                } else {
                    func_800494D4(
                        i,
                        D_8008E5CC[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
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
        motion_LoadFighterData(
            D_8008D538[(s8)D_80102778.unk_4[0]], (s8)D_80102778.unk_4[2],
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
            motion_ResetVariantIndex(0);
            {
                MoveScript *v = motion_GetDefaultVariantScript(0);
                D_80101EC8[0].move_is_common = 0;
                func_80021A98(0, v, 0);
            }
            if (D_800A38DC == 2 && D_800A389A == 0) {
                MoveScript *v = motion_GetCurrentVariantScript(1);
                D_80101EC8[1].move_is_common = 0;
                func_80021A98(1, v, 0);
            } else {
                MoveScript *v;
                motion_ResetVariantIndex(1);
                v = motion_GetDefaultVariantScript(1);
                D_80101EC8[1].move_is_common = 0;
                func_80021A98(1, v, 0);
            }
        } else {
            motion_ResetVariantIndex(0);
            motion_ResetVariantIndex(1);
            {
                MoveScript *v = func_800219E4(0);
                D_80101EC8[0].move_is_common = 1;
                func_80021A98(0, v, 1);
            }
            {
                MoveScript *v = func_800219E4(1);
                D_80101EC8[1].move_is_common = 1;
                func_80021A98(1, v, 1);
            }
        }
    } else {
        motion_ResetVariantIndex(0);
        motion_ResetVariantIndex(1);
        {
            MoveScript *v = motion_GetDefaultVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            func_80021A98(0, v, 0);
        }
        {
            MoveScript *v = motion_GetDefaultVariantScript(1);
            D_80101EC8[1].move_is_common = 0;
            func_80021A98(1, v, 0);
        }
    }
    D_800A382E = 0;
    D_800A3748 = -1;
    camera_InitTwoFighterView(&D_80101EC8[0], &D_80101EC8[1]);
    if (D_800A38BA != 0) {
        camera_InitTrackedFighterFocus(&D_80101EC8[0], &D_80101EC8[1]);
    }
    replay_Reset();
    eff_StopCommands();
    func_8001DBE4();
    D_800A3768 = DISP_DISABLED;
    D_800A36A8 = 0;
    eff_Init();
    D_800A3670 = 0;
    D_800A3834 = 1;
    func_8001C820();
    snd_StartFightCdAudio();
    snd_ResetStageSoundState();
    snd_SetReverb(D_800A36A4);
    if (D_800A38DC == 6) {
        rng_SetSeed(D_800A3904);
    }
}

void camera_ApplyFightView(void) {
    /* FAKE: frame layout: unwritten leading pad for the original frame's 8
     * untouched bytes (dead-vars-local-array carve-out; SOTN precedent:
     * volatile u32 pad[4]; // FAKE at st/sel/stream.c:80); without: score 23.
     */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *s2;

    if (D_800A38BA != 0) {
        if (g_camera_fight_view_mode == 1) {
            if (D_80101EC8[0].is_defeated != 0 ||
                D_80101EC8[1].is_defeated != 0) {
                g_camera_fight_view_mode = 2;
            }
        }
        if (g_camera_fight_view_mode != 2) {
            if (D_80101EC8[0].move_kind == 0x11 ||
                D_80101EC8[1].move_kind == 0x11) {
                s2 = &D_800F6608;
                g_camera_fight_view_mode = 1;
            } else {
                s2 = &D_800F5328;
                g_camera_fight_view_mode = 0;
            }
        } else {
            s2 = &D_800F6608;
        }
        func_8003F218(g_camera_fight_view_mode < 1);

        SetGeomScreen(
            math_FovToScreenDist(g_camera_fight_view_mode == 0 ? 0x50 : 0x2D));

        if (g_camera_fight_view_mode == 0) {
            func_80041688(D_800A36F6, 1);
            func_80041688(D_800A36F6 == 0, 0);
        } else {
            func_80041688(0, 0);
            func_80041688(1, 0);
        }
    } else {
        s2 = &D_800F6608;
    }

    if (D_800A3834 == 1) {
        local.unk_00.x = s2->unk_00.x + g_camera_view_offset.unk_00.x;
        local.unk_00.y = s2->unk_00.y + g_camera_view_offset.unk_00.y;
        local.unk_00.z = s2->unk_00.z + g_camera_view_offset.unk_00.z;
        local.unk_10.vx = s2->unk_10.vx + g_camera_view_offset.unk_10.vx;
        local.unk_10.vy = s2->unk_10.vy + g_camera_view_offset.unk_10.vy;
        local.unk_10.vz = s2->unk_10.vz + g_camera_view_offset.unk_10.vz;
        local.w18 = s2->w18 + g_camera_view_offset.w18;
    } else {
        local = *s2;
    }

    func_80046BF4(&local.unk_00, &local.unk_10, local.w18);
    {
        camera_CalcEye(&local, &s2->w20);
        func_80061064(&local.unk_10.vx, &s2->w20);
    }
    func_8003F3D4(s2->h30[0]);
    func_8003F3D4(s2->h30[1]);
    g_listener_cam = s2;
}

void camera_ApplyReplayView(s32 arg0) {
    /* FAKE: frame layout: unwritten leading pad for the original frame's 8
     * untouched bytes (dead-vars-local-array carve-out; SOTN precedent:
     * volatile u32 pad[4]; // FAKE at st/sel/stream.c:80); without: score 21.
     */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *s2;

    s2 = &D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = &D_800F6608;
    }

    local.unk_00.x = s2->unk_00.x + g_camera_view_offset.unk_00.x;
    local.unk_00.y = s2->unk_00.y + g_camera_view_offset.unk_00.y;
    local.unk_00.z = s2->unk_00.z + g_camera_view_offset.unk_00.z;
    local.unk_10.vx = s2->unk_10.vx + g_camera_view_offset.unk_10.vx;
    local.unk_10.vy = s2->unk_10.vy + g_camera_view_offset.unk_10.vy;
    local.unk_10.vz = s2->unk_10.vz + g_camera_view_offset.unk_10.vz;

    local.w18 = s2->w18 + g_camera_view_offset.w18;
    func_80046BF4(&local.unk_00, &local.unk_10, local.w18);

    {
        camera_CalcEye(&local, &s2->w20);
        func_80061064(&local.unk_10.vx, &s2->w20);
    }

    g_listener_cam = s2;
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

void fight_Update(void) {
    PadState buf;

    D_800A3778 = func_800472B0();
    camera_UpdateTwoFighterView(&D_80101EC8[0].unk_168, &D_80101EC8[1].unk_168,
                                &D_80101EC8[0], &D_80101EC8[1]);
    if (D_800A38BA != 0) {
        camera_UpdateTrackedFighterView(&D_80101EC8[D_800A36F6]);
    }
    camera_ApplyFightView();
    replay_BeginFrame();
    func_8002006C();
    func_8001BE20(0, &buf);
    fighter_Update(0, &buf);
    func_8001BE20(1, &buf);
    fighter_Update(1, &buf);
    func_8002C61C();
    object_UpdatePool();
    func_800321E8();
    replay_AdvanceFrame();
    if (D_800A38BA != 0 && g_camera_fight_view_mode == 0) {
        func_8001E800();
    } else {
        func_8003E6A0(D_80101EC8[0].world_pos.x, D_80101EC8[0].world_pos.z);
        func_8003E6A0(D_80101EC8[1].world_pos.x, D_80101EC8[1].world_pos.z);
    }
    func_80046DA8((D_800A3690 ^ 1) != 0);
    func_8001CE60();
    snd_UpdateStageSounds();
    func_8001C8DC();
}

void func_8001EA04(void) {
    u8 v;
    func_80041688(0, 0);
    func_80041688(1, 0);
    eff_StopCommands();
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
    Unk8001CD68Rec buf;

    D_800A37B8 += 1;
    D_800A3778 = func_800472B0();
    camera_SetFighterFocusView(&D_80101EC8[D_800A3748 == 0], D_800A37B8 << 3);
    camera_ApplyFightView();
    replay_BeginFrame();
    func_8002006C();
    pad_ClearStateBits(&sp10);
    fighter_Update(0, &sp10);
    fighter_Update(1, &sp10);
    func_8002C61C();
    object_UpdatePool();
    func_800321E8();
    replay_AdvanceFrame();
    func_80046DA8(1);
    snd_UpdateStageSounds();
    if (D_800A38DC == 3) {
        func_8001CD68(&buf);
        g_prim_buf_cursor +=
            func_8005D814(&buf, D_800A38E2, (s32)g_prim_buf_cursor, 1) / 4;
    }
    if (D_800A3929 == 0) {
        g_prim_buf_cursor +=
            func_8005C8A8(1, D_800A3817, (s32)g_prim_buf_cursor, 0) / 4;
        if ((g_pad_state.pressed & 0x10001000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3817 != D_800A3804) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((g_pad_state.pressed & 0x40004000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = D_800A3804;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }
        if ((g_pad_state.pressed & 0x400040) != 0) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
            if (D_800A3817 != 0)
                return;
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
        g_prim_buf_cursor += func_8005FA98(0, (s32)g_prim_buf_cursor, 1) / 4;
    }
    D_800A3929 = D_800A3929 + 1;
    if (D_800A3929 < 0x3C)
        return;
    if (D_800A3817 == 0) {
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF =
            fight_FindNearestStartPair(&D_80101EC8[D_800A3748].world_pos);
        if (D_80101EC8[1].unk_06 != 0) {
            fighter_HalveCpuTally(1);
        }
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        cdrom_PauseAndWait();
        func_8001DA2C();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if (D_800A3817 == 2) {
        cdrom_PauseAndWait();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}

void func_8001EEB4(void) {
    s8 idx = D_800A3748;
    Unk80101EC8Record *entry = &D_80101EC8[idx];
    u16 a1 = entry->move_kind;

    if (a1 != 0xA && entry->unk_72 == 0 && a1 != 0x17 && a1 != 0x18 &&
        entry->is_defeated == 0) {
        motion_ResetVariantIndex(D_800A3748);
        {
            MoveScript *ret = func_80021A3C(D_800A3748, entry->unk_0A);
            s32 idx2 = D_800A3748;
            entry->move_is_common = 1;
            func_80021A98(idx2, ret, 1);
        }
        entry->unk_26C = 1;
    }

    eff_StopCommands();
    D_800A37B8 = 0;
    D_800A3834 = 0x11;
}

void func_8001EFA0(void) {
    PadState sp10;
    s16 var_v0;

    D_800A37B8 += 1;
    D_800A3778 = func_800472B0();
    camera_SetInterpolatedFighterView(
        &D_80101EC8[D_800A3748], (D_800A37B8 << 12) / 105);
    camera_ApplyFightView();
    replay_BeginFrame();
    func_8002006C();
    pad_ClearStateBits(&sp10);
    fighter_Update(0, &sp10);
    fighter_Update(1, &sp10);
    func_8002C61C();
    object_UpdatePool();
    func_800321E8();
    replay_AdvanceFrame();
    func_80046DA8(1);
    snd_UpdateStageSounds();

    if (D_80101EC8[D_800A3748].is_defeated != 0 && D_800A38DC == 1) {
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
                D_800A3768 = 2;
                fight_AdvanceOpponentOrder();
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

void motion_ApplyRecordedPoseAdjustments(Unk80101EC8Record *arg0, Rec1C *arg1,
                                         MotionFrame *arg2, MotionFrame *arg3) {
    s16 temp_v1;
    if (!(arg1->b18 & 0x80)) {
        func_80027334(arg2);
        func_80027334(arg3);
    }
    func_8002F770(&arg2->unk_0C[0x15], arg1->b14 * 4, arg1->b15 * 4, 0);
    func_8002F770(&arg3->unk_0C[0x15], arg1->b14 * 4, arg1->b15 * 4, 0);
    temp_v1 = arg0->unk_0C;
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        arg2->unk_0C[0x39] += arg1->b16 * 4;
        arg3->unk_0C[0x39] += arg1->b16 * 4;
    }
    if ((u32)((u16)arg0->unk_0E - 6) < 2U) {
        arg2->unk_0C[0x33] += arg1->b16 * 4;
        arg3->unk_0C[0x33] += arg1->b16 * 4;
    }
}

/* Steers two bone-angle sets (a, b) toward obj's partner (obj->other):
 * while obj+0x6A is 0x15/0x25, a clamped heading (obj+0x1D8 - obj+0x1CA) and
 * a clamped elevation from ratan2(ground distance, height delta) are eased
 * 1/8 of the wrapped difference per call into obj+0x1E6/0x1E8 and applied
 * to both sets via func_8002F770 (otherwise both ease back toward 0; state
 * 0x1F holds elevation at 0x100). Also sets or eases a twist in obj+0x1EA
 * (states 0x1D/0xE with obj+0x8C != 0, or obj+0xE in 6..7 with obj+0x6A ==
 * 2), and adds random jitter to both sets when obj+0x26E is set and obj+0x96
 * == 0. The ground distance is the g_sqrt_table_u8 byte-LUT integer sqrt (the
 * math_PrepareSegmentCollisionFrame idiom), with one gte_Lzc per root. */
void motion_AdjustFighterPose(
    Unk80101EC8Record *obj, MotionFrame *a, MotionFrame *b) {
    s32 lzc_out;
    s32 lzc_out2;
    s32 var_2;
    /* temp2: two values -- the clamped elevation target that obj+0x1E8 eases
     * toward (0x100 in state 0x1F, 0 outside states 0x15/0x25), then the
     * clamped twist target that obj+0x1EA eases toward (Ruling 11). */
    s32 temp2;
    /* FAKE: constant-holder (named-local-fake-exception): var_4 is 0 on every
     * path; the three arm writes reach both func_8002F770 calls unfolded and
     * the pseudo takes $s2 as in the target. The literal 0: score 46; one
     * initializer: score 43; one write after the join: score 46. */
    s32 var_4;
    /* dx / dz: two values each -- the partner-minus-obj x (z) offset of
     * obj+0x180 (0x188) for the elevation, then of the saved obj+0xF4 (0xFC)
     * position for the twist (Ruling 11). */
    s32 dx;
    s32 dz;
    /* temp: six values -- the wrapped obj+0x1E6 easing delta, the wrapped
     * obj+0x1E8 easing delta, the clamped twist stored to obj+0x1EA (states
     * 0x1D/0xE), the wrapped obj+0x1EA easing delta, and the two random
     * jitters (rng_Next() & 0x3F) - 0x20 (Ruling 11; owner Q58). */
    s32 temp;
    s16 t;

    if (obj->unk_26C == 0) {
        func_80027334(a);
        func_80027334(b);
    }
    if (obj->move_kind == 0x15 || obj->move_kind == 0x25) {
        if (obj->unk_0C == 0x1F) {
            temp2 = 0x100;
            var_4 = 0;
            var_2 = 0;
        } else {
            s32 dist_sq;
            s32 dist;

            var_2 = (obj->opponent_bearing - obj->orientation.vy) & 0xFFF;
            if (var_2 >= 0x800) {
                var_2 -= 0x1000;
            }
            if (var_2 < -0x1FF) {
                var_2 = -0x1FF;
            } else if (var_2 >= 0x200) {
                var_2 = 0x1FF;
            }
            dx = obj->other->unk_180.x - obj->unk_180.x;
            dz = obj->other->unk_180.z - obj->unk_180.z;
            dist_sq = dx * dx + dz * dz;
            if ((u32)dist_sq < 0x400) {
                dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
            } else {
                s32 lzcr = 0;
                if (dist_sq >= 0) {
                    gte_Lzc(dist_sq, &lzc_out);
                    lzcr = lzc_out;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            temp2 =
                0x400 - ratan2(dist, obj->other->unk_180.y - obj->unk_180.y);
            if (temp2 < -0xFF) {
                temp2 = -0xFF;
            } else if (temp2 >= 0x100) {
                temp2 = 0xFF;
            }
            var_4 = 0;
        }
    } else {
        var_4 = 0;
        temp2 = 0;
        var_2 = 0;
    }

    temp = (var_2 - obj->unk_1E6) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->unk_1E6 = obj->unk_1E6 + temp / 8;
    temp = (temp2 - obj->unk_1E8) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->unk_1E8 = obj->unk_1E8 + temp / 8;
    func_8002F770(&a->unk_0C[0x15], obj->unk_1E6, obj->unk_1E8, var_4);
    func_8002F770(&b->unk_0C[0x15], obj->unk_1E6, obj->unk_1E8, var_4);

    t = obj->unk_0C;
    if ((t == 0x1D || t == 0xE) && obj->unk_8C != 0) {
        temp = (ratan2(D_800A387C, obj->other->world_pos.y - obj->world_pos.y) -
                0x400) &
               0xFFF;
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

    if ((u32)((u16)obj->unk_0E - 6) < 2U && obj->move_kind == 2) {
        s32 dist_sq;
        s32 dist;

        if (obj->unk_268 == 0) {
            obj->unk_25C.x = obj->world_pos.x;
            obj->unk_25C.y = obj->world_pos.y;
            obj->unk_25C.z = obj->world_pos.z;
        }
        dx = obj->other->world_pos.x - obj->unk_25C.x;
        dz = obj->other->world_pos.z - obj->unk_25C.z;
        dist_sq = dx * dx + dz * dz;
        if ((u32)dist_sq < 0x400) {
            dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
        } else {
            s32 lzcr = 0;
            if (dist_sq >= 0) {
                gte_Lzc(dist_sq, &lzc_out2);
                lzcr = lzc_out2;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        temp2 =
            (ratan2(dist, obj->other->world_pos.y - obj->unk_25C.y) - 0x400) &
            0xFFF;
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

    if (obj->unk_26E != 0 && obj->is_defeated == 0) {
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

void fighter_SetTurnDelta(Unk80101EC8Record *arg0, s32 arg1) {
    arg1 = (arg1 - arg0->orientation.vy) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    arg0->unk_14C = arg1;
}

s32 fighter_CalcHorizontalSeparation(void) {
    s32 dx = D_80101EC8[1].world_pos.x - D_80101EC8[0].world_pos.x;
    s32 dy = D_80101EC8[1].world_pos.z - D_80101EC8[0].world_pos.z;
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

/* Computes the s16 factor at arg0+0x44 from the state kind at arg0+0x6A.
 * Some kinds (and, for kinds 0x2/0x1B/0x28/0x26, an arg0+0x40 value inside
 * the 0xA1..0xA3 bounds or not outside the 0xA2..0xA4 bounds) force 0x1000.
 * Otherwise arg0+0x1C is scaled (Q12) by the table at arg0+0x27E indexed by
 * min(arg0+0x26E + arg0+0x272, 3); the range-checked kinds first also scale
 * by arg0+0x274 (when arg0+0x26C is 0) and by the table at arg0+0x276 indexed
 * by arg0+0x270 clamped to 3.
 *
 * `short var_7` is a plain signed short local: GCC's lowering of it gives the
 * second `lhu`, the `sll 16 ; sra 15` and the 8-byte stack frame. `var_0`
 * / `kind = var_0 & 0xFFFFU` mirror the target's
 * `lhu $a1,0x6A ; andi $v1,$a1,0xFFFF`. */
void motion_UpdateFrameStep(Unk80101EC8Record *arg0) {
    u32 var_0;
    u32 kind;
    s32 val;
    s32 a2;
    s32 idx;
    s32 factor;
    var_0 = arg0->move_kind;
    kind = var_0 & 0xFFFFU;
    a2 = arg0->unk_1C;
    if (kind == 0x11 || kind == 0xF || ((u32)((s32)var_0 - 0x1C)) < 2U ||
        ((u32)((s32)var_0 - 0x1E)) < 2U || ((u32)((s32)var_0 - 0x20)) < 2U ||
        kind == 0xE || kind == 0x2C || kind == 0xD || kind == 0x7 ||
        kind == 0x33 || kind == 0x14) {
        goto clamp;
    }
    if (kind == 0x2) {
        goto rangecheck;
    }
    if (kind == 0x1B) {
        goto rangecheck;
    }
    if (kind == 0x28) {
        goto rangecheck;
    }
    if (kind != 0x26) {
        goto defaultpath;
    }
rangecheck:
    val = arg0->move_frame;
    if (val < arg0->unk_A1[0]) {
        goto check_outer;
    }
    if (val > arg0->unk_A3[0]) {
        goto check_outer;
    }
    goto clamp;
check_outer:
    if (val < arg0->unk_A1[1]) {
        goto multpath_start;
    }
    if (val > arg0->unk_A3[1]) {
        goto multpath_start;
    }
clamp:
    arg0->move_frame_step = 0x1000;
    return;
multpath_start:
    if (arg0->unk_26C == 0) {
        s32 f = arg0->unk_274;
        a2 = (a2 * f) >> 12;
    }
    {
        s16 var_7 = arg0->unk_270;
        if (var_7 >= 4) {
            var_7 = 3;
        }
        idx = var_7;
    }
    factor = arg0->unk_276[idx];
    a2 = (a2 * factor) >> 12;
defaultpath: {
    s32 var_8 = arg0->unk_26E;
    s32 var_9 = arg0->unk_272;
    s32 sum = var_8 + var_9;
    s32 sum_or_3 = (sum < 4) ? sum : 3;
    idx = sum_or_3;
}
    factor = arg0->unk_27E[idx];
    a2 = (a2 * factor) >> 12;
    arg0->move_frame_step = a2;
}

typedef struct {
    u16 flags;
    u16 id;
    u8 b[4];
} Unk8001FAE4Entry;

Unk8001FAE4Entry *motion_FindPairedMoveEntry(MoveScript *arg0) {
    u16 v1;
    u16 *a0;

    a0 = arg0->unk_0A;
    v1 = *a0;
    while (v1 != 0) {
        if ((v1 & 0x4000) != 0) {
            return (Unk8001FAE4Entry *)a0;
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
    if (D_800A38DC == 2 || D_800A38DC == 5 || D_800A38DC == 3) {
        return 0;
    }
    if (D_800A38DC == 0 && D_800A385C != 0) {
        return 0;
    }
    if (arg0->other->unk_0C == 0xD || arg0->other->unk_0C == 0x1C) {
        return 0;
    }
    if (arg0->unk_0A == 0xE &&
        (arg0->unk_330 == 0 || arg0->unk_332[0] != 0xA)) {
        return 0;
    }
    if (arg1 != 0) {
        return arg0->unk_26C != 0;
    }
    return 1;
}

void fight_UpdatePairedMoves(void);

void fight_UpdatePairedMoves(void) {
    Unk80101EC8Record *rec;
    Unk8001FAE4Entry *ent;
    u8 *data;
    MoveScript *var_3;
    s32 lo;
    s32 hi;
    s32 dz;
    s32 i;
    u16 kind;
    s32 pos[3];

    if (D_800A376E != 0) {
        D_800A376E = 0;
        D_800A38E8 = 0xFF;
        if (D_80101EC8[0].is_defeated != 0) {
            return;
        }
        if (D_80101EC8[1].is_defeated != 0) {
            return;
        }
        func_80021A98(D_800A38AE, D_800A36D8, D_800A381C);
        if (D_800A36CA & 0x1000) {
            D_80101EC8[D_800A38AE == 0].borrow_other_motion = 1;
        }
        func_80021A98(D_800A38AE == 0, D_800A36D8, D_800A381C);
        D_80101EC8[1].move_started = 2;
        D_80101EC8[0].move_started = 2;
        return;
    }
    if (D_800A3758 != 0xFF) {
        rec = &D_80101EC8[D_800A3758];
        if (D_800A3769 != 0) {
            rec->pending_move_event = 1;
            rec->unk_94 = 0;
        } else {
            rec->pending_move_event = 0;
            rec->unk_94 = 1;
        }
        if (rec->is_defeated != 0) {
            rec->pending_move_event += 2;
        }
        rec->unk_74 = rec->collision_pos.vy;
        rec->other->pending_move_event = 1;
        rec->other->unk_94 = 0;
        rec->other->unk_74 = rec->other->collision_pos.vy;
        if (rec->other->is_defeated != 0) {
            rec->other->pending_move_event += 2;
        }
        D_800A3758 = 0xFF;
        return;
    }
    if (D_80101EC8[0].pending_move_event != -1) {
        return;
    }
    if (D_80101EC8[1].pending_move_event != -1) {
        return;
    }
    for (i = 0; i < 2; i++) {
        rec = &D_80101EC8[i];
        if (rec->move_started == 0) {
            continue;
        }
        ent = motion_FindPairedMoveEntry(rec->cur_move);
        if (ent == 0) {
            continue;
        }
        data = ent->b;
        lo = data[0] * 20;
        hi = data[1] * 20;
        dz = rec->collision_pos.vy - rec->other->collision_pos.vy;
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
        kind = rec->other->move_kind;
        if (kind != 0x15 && kind != 0x2C && kind != 0xE && kind != 0x19) {
            continue;
        }
        D_800A38AE = i;
        D_800A376E = 0;
        D_800A3758 = 0xFF;
        D_800A371C = data[2] * 20;
        D_800A38E8 = data[3] & 0x7F;
        var_3 = motion_ResolveMoveData(rec, ent->id, &rec->move_is_common);
        func_80021A98(i, var_3, rec->move_is_common);
        rec->other->borrow_other_motion = 1;
        rec->other->move_is_common = rec->move_is_common;
        func_80021A98(i == 0, var_3, rec->move_is_common);
        rec->move_started = 2;
        rec->other->move_started = 2;
        rec->other->unk_86 = rec->other->unk_84;
        rec->other->unk_272 += 1;
        pos[0] = (rec->world_pos.x + rec->other->world_pos.x) / 2;
        pos[1] = (rec->world_pos.y + rec->other->world_pos.y) / 2;
        pos[2] = (rec->world_pos.z + rec->other->world_pos.z) / 2;
        func_80032854(i, 0x10, pos, 0);
        return;
    }
}

s32 func_8002006C(void) {
    s32 s0 = D_800A387C;
    s32 v0 = fighter_CalcHorizontalSeparation();
    s32 v = D_800A38DC;
    D_800A387C = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        fight_UpdatePairedMoves();
    }
    D_800A38A8 = 0;
}

void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 var_0;
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

            var_0 = arg2 * arg2 + arg3 * dy2;

            if (var_0 >= 0) {
                s32 a2;
                var_0 = SquareRoot0(var_0 << 10);
                a2 = arg2 << 5;
                a0 = ((a2 + var_0) * dist) / dy2 / 32;

                if (a0 < 0) {
                    /* FAKE: dead dy reused as the arm-2 (a2-var_0) temp:
                     * var_0's register preference flows through dy to the /32
                     * quotient, which takes $v1 as in the target. The
                     * natural spelling or a fresh temp: score 6; reusing
                     * var_0: 8. */
                    dy = a2 - var_0;
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

/* Starts the rec->unk_350 frame counter, records bone index
 * D_8008D59C[arg1].bone at rec->unk_352, and rotates the vector arg2 by that
 * bone's matrix of func_80046DEC(rec->index) into rec->unk_354. */
void func_800203B4(Unk80101EC8Record *rec, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    MATRIX *src;

    rec->unk_350 = 1;
    rec->unk_352 = D_8008D59C[arg1].bone;
    src = ((MATRIX **)func_80046DEC(rec->index))[rec->unk_352];
    math_InvertMatrix3x3(src, mat);
    gte_SetRotMatrix(mat);
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    gte_ldlv0(vec);
    gte_rtv0();
    gte_stlvnl(&rec->unk_354);
}

/* rec->unk_350 is a frame counter func_800203B4 starts at 1. On the tick
 * where (counter & 7) == 2, the vector func_800203B4 stored at rec->unk_354
 * is rotated by matrix rec->unk_352 of func_80046DEC(var_2) (GTE MVMVA),
 * scaled by (150 - counter) / 150, negated when its y exceeds 0x800, and
 * handed to func_80032854 with scratchpad point
 * SPAD->unkA8[var_2][rec->unk_352]. The counter is cleared on every call that
 * finds it running (the original stores 0 under `>= 150` and then again
 * unconditionally). */
void func_800204C0(Unk80101EC8Record *rec) {
    s32 mac[3];
    s16 out[3];
    s32 var_2;
    s32 mul;
    MATRIX **var_4;

    var_2 = rec->index;
    if (rec->unk_350 != 0) {
        rec->unk_350 += 1;
        if ((rec->unk_350 & 7) == 2) {
            var_4 = func_80046DEC(var_2);
            gte_SetRotMatrix(var_4[rec->unk_352]);
            gte_ldlv0(&rec->unk_354);
            gte_rtv0();
            mul = ((0x96 - rec->unk_350) << 12) / 150;
            gte_stlvnl(mac);
            out[0] = (mac[0] * mul) / 0x1000;
            out[1] = (mac[1] * mul) / 0x1000;
            out[2] = (mac[2] * mul) / 0x1000;
            if (out[1] > 0x800) {
                out[0] = -out[0];
                out[1] = -out[1];
                out[2] = -out[2];
            }
            func_80032854(var_2, 4, &SPAD->unkA8[var_2][rec->unk_352].x, out);
        }
        if (rec->unk_350 >= 0x96) {
            rec->unk_350 = 0;
        }
        rec->unk_350 = 0;
    }
}

void fighter_ScaleBoneHitRecords(s32 arg0, s32 arg1) {
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

/* Places character rec->index's points in world space with its bone
 * matrices (func_80046DEC()): each of the 22 hit records' offsets through
 * its bone into arg1 (the callers pass SPAD->unkA8[i]); the point set
 * D_8008D86C[unk_0E] (D_8008D774 when unk_12 == 50) through bone 18 into
 * arg2; when unk_8C != 0, the pair D_8008D88C[unk_14] through bone 19 into
 * arg3. A point is the GTE MVMVA rotation (sf=1) of the SVECTOR plus the
 * matrix translation. Then: bone 11's y axis into rec->unk_1EC, arg1[0]
 * into rec->unk_180, the translations of bones 17 and 14 (y raised by (unk_1A *
 * 71) >> 11) into rec->unk_198[], the floor height under each
 * (collision_TestSegmentWithWork down a segment from y - 100 to y + 2000; the
 * segment's lower end when it finds nothing) into rec->unk_1B0[], and the two
 * bones' headings into unk_1BA / unk_1C2. */
void fighter_UpdateBoneWorldData(Unk80101EC8Record *rec, Unk80107850Rec *arg1,
                                 Unk80107850Rec *arg2, Unk80107850Rec *arg3) {
    /* collision_TestSegmentWithWork's arguments in scratchpad: from (words
     * 0..2), to (4..6), hit (8..10), normal (12..13), work area (14..) */
    s32 *var_0 = (s32 *)0x1F8002B8;
    MATRIX **var_1;
    MATRIX *m;
    s32 *pos;
    SVec4i16 *v;
    BoneHitRec *var_5;
    Unk80107850Rec *o;
    s32 i;

    var_1 = func_80046DEC(rec->index);
    var_5 = D_800F5F68[rec->index];
    o = arg1;
    for (i = 0; i < 22; i++, var_5++, o++) {
        m = var_1[var_5->bone];
        gte_ApplyMatrix(m, &var_5->ofs, o);
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    m = var_1[18];
    gte_SetRotMatrix(m);
    if (rec->unk_12 == 50) {
        v = D_8008D774;
    } else {
        v = D_8008D86C[rec->unk_0E];
    }
    o = arg2;
    for (i = 0; i < D_8008D864[rec->unk_0E]; i++, v++, o++) {
        gte_ApplyRotMatrix(v, o);
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    if (rec->unk_8C != 0) {
        m = var_1[19];
        gte_SetRotMatrix(m);
        v = D_8008D88C[rec->unk_14];
        o = arg3;
        for (i = 0; i < 2; i++, v++, o++) {
            gte_ApplyRotMatrix(v, o);
            o->x += m->t[0];
            o->y += m->t[1];
            o->z += m->t[2];
        }
    }

    m = var_1[11];
    gte_SetRotMatrix(m);
    gte_ldv0(&D_800A3138);
    gte_rtv0();
    rec->unk_180 = arg1[0];
    gte_stlvnl(&rec->unk_1EC);

    pos = var_1[17]->t;
    rec->unk_198[0].x = pos[0];
    rec->unk_198[0].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[0].z = pos[2];
    pos = var_1[14]->t;
    rec->unk_198[1].x = pos[0];
    rec->unk_198[1].y = pos[1] + ((rec->unk_1A * 71) >> 11);
    rec->unk_198[1].z = pos[2];
    for (i = 0; i < 2; i++) {
        var_0[0] = rec->unk_198[i].x;
        var_0[1] = rec->unk_198[i].y - 100;
        var_0[2] = rec->unk_198[i].z;
        var_0[4] = rec->unk_198[i].x;
        var_0[5] = rec->unk_198[i].y + 2000;
        var_0[6] = rec->unk_198[i].z;
        if (collision_TestSegmentWithWork(&var_0[0], &var_0[4], &var_0[8],
                                          (s16 *)&var_0[12], (s32)&var_0[14])) {
            rec->unk_1B0[i] = var_0[9];
        } else {
            rec->unk_1B0[i] = var_0[5];
        }
    }
    m = var_1[17];
    rec->unk_1BA = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
    m = var_1[14];
    rec->unk_1C2 = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
}

void func_80020CDC(void) {
    /* FAKE: direct D_800A38C4[1] puts the constant address in a pseudo that CSE
     * keeps live across ndata_InvalidatePack (+3 insns) */
    u16 *p = D_800A38C4;

    if (p[1] == 0xFFFF) {
        ndata_InvalidatePack();
    }
    g_motion_common_loaded = 0;
    p[1] = 0;
    p[0] = 0;
    g_motion_frame_set_ids[1] = 0xFF;
    g_motion_frame_set_ids[0] = 0xFF;
}

void func_80020D38(void) {
    /* FAKE: direct D_800A38C4[1] puts the constant address in a pseudo that CSE
     * keeps live across ndata_InvalidatePack (+3 insns) */
    u16 *p = D_800A38C4;

    if (p[1] == 0xFFFF) {
        ndata_InvalidatePack();
    }
    p[1] = 0;
}

void motion_InitBuffers(void) {
    g_motion_frame_buffers[0] = (MotionFrame *)0x80118800;
    g_motion_frame_buffers[1] = (MotionFrame *)0x8011C400;
    g_motion_common_buffer = (s32)0x80120000;
    g_motion_fighter_buffers[0] = (Tbl800A3860Entry *)0x80148800;
    g_motion_fighter_buffers[1] = (Tbl800A3860Entry *)0x80190800;
    func_80020CDC();
}

void motion_LoadCommonData(void) {
    s32 v1;
    cdrom_StartRead(cdrom_GetFileIndex(1, 1), g_motion_common_buffer);
    cdrom_WaitIdle();
    v1 = g_motion_common_buffer;
    g_motion_common_sections.unk_00 = (u16 *)(v1 + 0x14);
    g_motion_common_sections.unk_04 = (u16 *)(v1 + *(s32 *)(v1 + 4));
    g_motion_common_sections.unk_08 = (u8 *)(v1 + *(s32 *)(v1 + 8));
    g_motion_common_sections.unk_10 = (u32 *)(v1 + *(s32 *)(v1 + 0x10));
    g_motion_common_loaded = 1;
}

/* For i = 0, 1 with v = arg0, then arg2 (when D_800A38DC is 1, 4 or 6): when
 * g_motion_frame_set_ids[i] differs from v, stores v there and calls
 * cdrom_StartReadAt(cdrom_GetFileIndex(1, 0), g_motion_frame_buffers[i], v * 7,
 * 7). Then sets D_80101EC8[0].unk_48 to D_8008DB1C[arg0][arg1] | arg1 << 12 and
 * D_80101EC8[1].unk_48 to the same of arg2 and arg3 (a copy of the first
 * when D_800A38DC is 3). It then fills var_0[0..1] from D_800A38C4[0..1]: it
 * returns when every id is already there; equal ids give var_0[0] = id0; an
 * id found in D_800A38C4[0] stays and the other id goes to var_0[1]; id1
 * found only in D_800A38C4[1] gives var_0[0] = id0; otherwise (also when id0
 * is only in D_800A38C4[1]) var_0[0] = id0 and var_0[1] = id1. For each
 * nonzero var_0[i] it calls cdrom_StartRead(cdrom_GetFileIndex(1, j + 2),
 * g_motion_fighter_buffers[i]), j the id's D_8008DCCC index, points
 * g_motion_fighter_sections[i] into g_motion_fighter_buffers[i] and stores the
 * id in D_800A38C4[i]. */
void motion_LoadFighterData(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* FAKE: frame layout: only var_0[0..1] are used; the target frame (0x140)
     * reserves 0x100 untouched bytes after them (N = 129..132; var_0[2] gives
     * the wrong frame); without it: score 20. */
    u16 var_0[130];
    s32 i;
    /* FAKE: one local for loop 1's arg0 / arg2 and loop 2's D_8008DCCC index;
     * separate locals seat the index in $a1, the target keeps both in $s0
     * (score 3). */
    s32 j;

    if (g_motion_common_loaded == 0) {
        motion_LoadCommonData();
    }

    if (D_800A38DC == 1 || D_800A38DC == 4 || D_800A38DC == 6) {
        for (i = 0; i < 2; i++) {
            j = arg0;
            if (i != 0) {
                j = arg2;
            }
            if (g_motion_frame_set_ids[i] != j) {
                g_motion_frame_set_ids[i] = j;
                cdrom_StartReadAt(cdrom_GetFileIndex(1, 0),
                                  (s32)g_motion_frame_buffers[i], j * 7, 7);
                cdrom_WaitIdle();
            }
        }
    }

    var_0[1] = 0;
    var_0[0] = 0;
    {
        u16 id0 = D_8008DB1C[arg0][arg1] | (arg1 << 12);
        u16 id1;

        if (D_800A38DC == 3) {
            id1 = id0;
        } else {
            id1 = D_8008DB1C[arg2][arg3] | (arg3 << 12);
        }
        D_80101EC8[0].unk_48 = id0;
        D_80101EC8[1].unk_48 = id1;

        if (id0 == id1) {
            if (D_800A38C4[0] == id1 || D_800A38C4[1] == id1) {
                return;
            }
            var_0[0] = id0;
        } else if (D_800A38C4[0] == id0) {
            if (D_800A38C4[1] == id1) {
                return;
            }
            var_0[1] = id1;
        } else if (D_800A38C4[0] == id1) {
            if (D_800A38C4[1] == id0) {
                return;
            }
            var_0[1] = id0;
        } else if (D_800A38C4[1] == id1) {
            var_0[0] = id0;
        } else {
            var_0[0] = id0;
            var_0[1] = id1;
        }
    }

    for (i = 0; i < 2; i++) {
        if (var_0[i] != 0) {
            for (j = 0; D_8008DCCC[j].id != 0; j++) {
                if (D_8008DCCC[j].id == var_0[i]) {
                    break;
                }
            }
            cdrom_StartRead(
                cdrom_GetFileIndex(1, j + 2), (s32)g_motion_fighter_buffers[i]);
            cdrom_WaitIdle();
            g_motion_fighter_sections[i].unk_00 =
                (u16 *)((s32)g_motion_fighter_buffers[i] + 0x6C +
                        (g_motion_fighter_buffers[i]->unk_03 - 1) * 6);
            g_motion_fighter_sections[i].unk_04 =
                (u16 *)((s32)g_motion_fighter_buffers[i] +
                        g_motion_fighter_buffers[i]->unk_04[0]);
            g_motion_fighter_sections[i].unk_08 =
                (u8 *)((s32)g_motion_fighter_buffers[i] +
                       g_motion_fighter_buffers[i]->unk_04[1]);
            g_motion_fighter_sections[i].unk_0C =
                (u16 *)((s32)g_motion_fighter_buffers[i] +
                        g_motion_fighter_buffers[i]->unk_04[2]);
            g_motion_fighter_sections[i].unk_10 =
                (u32 *)((s32)g_motion_fighter_buffers[i] +
                        g_motion_fighter_buffers[i]->unk_04[3]);
            D_800A38C4[i] = var_0[i];
        }
    }
}

void func_80021210(void) {
    func_8001979C(0, g_motion_common_sections.unk_10);
    if (D_800A38C4[0]) {
        func_8001979C(1, g_motion_fighter_sections[0].unk_10);
    }
    if (D_800A38C4[1]) {
        func_8001979C(2, g_motion_fighter_sections[1].unk_10);
    }
}

/* The record's model id (unk_48) -> its slot in D_800A38C4 (motion_slot); for
 * ids below 0x2000, the nibble positions of 4 and 5 (unk_88 / unk_8E),
 * copying unk_26C into unk_8A / unk_90 under conditions on D_800A38DC. The
 * loop-tail duplication is accepted by an owner ruling on SOTN evidence; every
 * codegen-only local is FAKE-labelled with what its natural spelling breaks. */
void func_80021280(s32 a0) {
    s32 a1 = 0;
    Unk80101EC8Record *a2 = &D_80101EC8[a0];
    s32 a3 = a2->unk_48;
    u16 *v1 = D_800A38C4;

loop1_21280:
    if (a3 == *v1)
        goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2)
        goto loop1_21280;
done1_21280:

{
    u16 val = a2->unk_48;
    a2->motion_slot = a1;
    a2->borrow_other_motion = 0;

    if ((u32)(val >> 12) < 2) {
        u16 t1; /* FAKE: copy of val (shifting val directly: score 7) */
        /* FAKE: holds the constant 4 (a literal: score 16) */
        s32 t4;
        /* FAKE: holds the constant 3 (a literal: score 20) */
        s32 t3;
        /* FAKE: holds the constant 1 (a literal: score 20) */
        s32 t2;
        u8 t0;
        s32 mode;
        s32 k;

        k = 0;
        t1 = val;
        t4 = 4;
        t3 = 3;
        t2 = 1;
        mode = D_800A38DC;
        t0 = D_800A384C;
    loop2_21280: {
        u16 nibble = (t1 >> (k << 2)) & 0xF;
        if (nibble != t4)
            goto not4_21280;
        a2->unk_88 = k;
        if (mode != t3)
            goto store4_21280;
        if (a0 != t2)
            goto store4_21280;
        if (t0 != nibble)
            goto next_21280;
    store4_21280:
        a2->unk_8A = a2->unk_26C;
        goto next_21280;
    not4_21280:
        if (nibble != 5)
            goto next_21280;
        a2->unk_8E = k;
        if (mode != 0)
            goto store5_21280;
        if (D_800A385C == 0)
            goto store5_21280;
        if (a0 == 0) {
            /* FAKE: loop tail duplicated into this arm (jump2 cross-jump
               re-merges it to identical bytes); with `goto next_21280` instead,
               the counters and the record pointer swap $a1 / $a2: score 19 */
            k++;
            if (k < 3)
                goto loop2_21280;
            return;
        }
    store5_21280:
        a2->unk_90 = a2->unk_26C;
    }
    next_21280:
        k++;
        if (k < 3)
            goto loop2_21280;
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
    arg0->unk_86 =
        (s16)((a1 + 1) % g_motion_fighter_buffers[arg0->motion_slot]->f14);
}

/* The 66-name animation/asset table (12-byte stride: 8 chars + null + pad).
 * No code reads it; a data word at 0x8008D9E8 points at its first name.
 * Defined here, between the switch tables of the functions above and
 * motion_ResolveMoveData's own, as in the original rodata order. */
const char D_800100FC[66][12] = {
    "WIN     ",    "KARAMI_ED", "RUN_ED  ", "RUN_ST  ", "CHAKUTI ", "APPEAR  ",
    "KAISHAKU_ST", "HAJIKARE",  "SERIEXIT", "SYAGAMI ", "HOM_ED  ", "HOM_AT  ",
    "HOM_ST  ",    "ANOBORI ",  "KAMAE_KA", "MOVE    ", "NOBORI_E", "NOBORI_S",
    "FURI2   ",    "FURI1   ",  "YURI2   ", "YURI1   ", "GOKAKU  ", "START   ",
    "ARUN    ",    "STEP    ",  "WALK    ", "LJUMP   ", "MJUMP   ", "SJUMP   ",
    "KAMAE   ",    "DTH     ",  "RUN     ", "SUNA    ", "KARAMI  ", "RELOAD  ",
    "SERI    ",    "HAJI    ",  "UKE     ", "SYASTEP ", "SUBWEP  ", "ORI     ",
    "OKIAGARI",    "NOBORI  ",  "KZRE    ", "KOROGARI", "KAISYAKU", "END_GAME",
    "DAM     ",    "ATTACK  ",  "NORMAL  ", "NULL    ", "Y123.BBM", "N123.BBM",
    "K123.BBM",    "T123.BBM",  "S234.BBM", "S125.BBM", "S124.BBM", "S123.BBM",
    "U235.BBM",    "U135.BBM",  "U134.BBM", "U125.BBM", "U124.BBM", "U123.BBM",
};

void *motion_ResolveMoveData(Unk80101EC8Record *rec, s32 id, s16 *out) {
    s16 t;
    s32 var_1;

    rec->unk_78 = 0;
    *out = 0;
    if ((u32)(id - 0x7FF5) < 11) {
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f66[id - 0x7FF5][rec->unk_86]];
    }
    switch (id) {
    case 0x7FF0:
        rec->unk_78 = 1;
        rec->unk_86 = rec->unk_84;
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f4E[rec->unk_84]];
    case 0x7FF1:
        rec->unk_86 =
            (rec->unk_86 + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
    case 0x7FF2:
    case 0x7FF4:
        if (id == 0x7FF4) {
            rec->unk_78 = 1;
        }
        t = rec->unk_86;
        if ((t == rec->unk_88 && rec->unk_8A == 0) ||
            (t == rec->unk_8E && rec->unk_90 == 0)) {
            rec->unk_86 = (rec->unk_86 + 1) %
                          g_motion_fighter_buffers[rec->motion_slot]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0) {
            func_800213A0(rec);
        }
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f4E[rec->unk_86]];
    case 0x7FF3:
        t = (rec->unk_86 + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
        if ((t == rec->unk_88 && rec->unk_8A == 0) ||
            (t == rec->unk_8E && rec->unk_90 == 0)) {
            t = (t + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
        } else if (D_800A38DC == 3 && rec->unk_06 != 0 &&
                   (t == rec->unk_88 || t == rec->unk_8E)) {
            t = (t + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
        }
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f54[rec->unk_86][t]];
    }
    if (id & 0x8000) {
        if (rec->borrow_other_motion != 0) {
            var_1 = rec->other->motion_slot;
        } else {
            var_1 = rec->motion_slot;
        }
        return &g_motion_fighter_sections[var_1].unk_00[id & 0x7FFF];
    }
    *out = 1;
    return &g_motion_common_sections.unk_00[id];
}

void motion_ResetVariantIndex(s32 a0) {
    D_80101EC8[a0].unk_86 = D_80101EC8[a0].unk_84;
}

MoveScript *motion_GetCurrentVariantScript(s32 a0) {
    s16 v1 = D_80101EC8[a0].motion_slot;
    s16 v0 = D_80101EC8[a0].unk_86;
    return (MoveScript *)&g_motion_fighter_sections[v1]
        .unk_00[g_motion_fighter_buffers[v1]->f4E[v0]];
}

MoveScript *motion_GetDefaultVariantScript(s32 a0) {
    s16 v1 = D_80101EC8[a0].motion_slot;
    s16 v0 = D_80101EC8[a0].unk_84;
    return (MoveScript *)&g_motion_fighter_sections[v1]
        .unk_00[g_motion_fighter_buffers[v1]->f4E[v0]];
}

MoveScript *func_800219E4(s32 a0) {
    return (MoveScript *)&g_motion_common_sections
        .unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f16];
}

MoveScript *func_80021A3C(s32 a0, s32 a1) {
    return (MoveScript *)&g_motion_common_sections
        .unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f18[a1]];
}

void func_80021A98(s32 arg0, MoveScript *arg1, s32 arg2) {
    Unk80101EC8Record *s0 = &D_80101EC8[arg0];
    s32 a3;
    if ((s0->borrow_other_motion) != 0) {
        a3 = s0->other->motion_slot;
    } else {
        a3 = s0->motion_slot;
    }
    s0->borrow_other_motion = 0;
    s0->cur_move = arg1;
    {
        u16 v1 = arg1->unk_04;
        s0->unk_5C = v1;
        if (arg2 != 0) {
            u16 *v0 = &g_motion_common_sections.unk_04[v1 * 2];
            s0->unk_54 = v0;
            s0->unk_58 = g_motion_common_sections.unk_08 + v0[1];
        } else {
            u16 *v0 = &g_motion_fighter_sections[a3].unk_04[v1 * 2];
            s0->unk_54 = v0;
            s0->unk_58 = g_motion_fighter_sections[a3].unk_08 + v0[1];
        }
    }
    {
        MoveScript *v0_50 = s0->cur_move;
        u16 old_kind = s0->move_kind;
        u8 *a0_58 = s0->unk_58;
        s0->cur_move_is_common = (u8)arg2;
        /* FAKE: empty do-while(0) -- its loop note puts a0_58 in $a0 and
         * v1_58 in $v1 (the two unk_58 lbu bases); without it they swap,
         * score 2. */
        do {
        } while (0);
        s0->cur_move_slot = (u8)a3;
        {
            u8 a1_val = v0_50->unk_06;
            s0->prev_move_kind = old_kind;
            {
                u8 *v1_58 = s0->unk_58;
                s0->move_frame_frac = 0;
                s0->move_started = 1;
                s0->queued_move = 0;
                s0->unk_46 = 0;
                s0->move_frame = a1_val;
                /* FAKE: the do-while(0) wrap's loop-depth weighting seats a0_58
                 * in $a0 and a1_val in $a1 as in target; without it they swap,
                 * score 4. */
                do {
                    s0->move_kind = *a0_58;
                } while (0);
                s0->unk_6E = v1_58[2];
            }
        }
        {
            MoveScript *v0_50b = s0->cur_move;
            s32 kind = s0->move_kind;
            s0->unk_70 = (v0_50b->unk_09) & 3;
            {
                s32 a0_flag = 0;
                if ((((kind == 2) || (kind == 0x1B)) || (kind == 0x28)) ||
                    (kind == 0x26)) {
                    a0_flag = 1;
                }
                s0->unk_AD = a0_flag;
            }
            fighter_ReadFrameMetadata(s0);
            {
                u16 state = s0->move_kind;

                if (state == 9) {
                    s0->unk_152 = 1;
                    s0->unk_154 = s0->orientation.vy;
                } else if (state == 2) {
                    if (s0->unk_152 == 0 && s0->prev_move_kind != 0x13) {
                        s0->unk_154 = s0->opponent_bearing;
                    }
                    s0->unk_152 = 0;
                } else if (state == 0x19 || state == 0x1A) {
                    if (s0->unk_152 == 0 ||
                        (state == 0x19 && s0->prev_move_kind == state)) {
                        s0->unk_154 = s0->opponent_bearing;
                    }
                    s0->unk_152 = 1;
                } else if (state == 0x11) {
                    s0->unk_152 = 1;
                } else {
                    s0->unk_152 = 0;
                }
            }
            {
                u16 v1f = s0->move_kind;
                if ((((v1f == 2) || (v1f == 0x1B)) || (v1f == 0x28)) ||
                    (v1f == 0x26)) {
                    s0->unk_AF = ((s0->unk_B0) & 0xF) != 5;
                }
            }
        }
    }
}

void fight_GetStartPosition(s32 arg0, s32 *arg1, s32 arg2) {
    s16 *temp_v0;
    temp_v0 = (s16 *)func_80046F14() +
              ((D_800A36A4 * 0x18) + (arg2 * 6) + (arg0 * 3));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}

void func_80021DB0(s32 arg0, Vec3i32 *out, s32 *pos) {
    Vec3i32 base;
    Vec3i32 cur;
    Vec3i32 var_2;
    Vec3i32 var_3;
    s16 var_4[4];
    s16 *var_5;
    s32 ofs;
    s32 var_7;
    /* the counter of both the 8-direction loop and the record search, as SOTN
     * reuses one counter across consecutive loops (Q51):
     * SOTN: src/dra/menu.c:138 @aa53500 */
    s32 i;
    /* Ruling 11 (ordinary-c-judge-decidable): holds three values -- the
     * horizontal step (1..40, dx / dz scaled by temp / 40), the vertical
     * step (1..40, temp * 100 in y), then the index of a record of
     * func_80046F14's block (D_800A38E0 when D_800A38DC is 3, else the one of
     * four nearest pos in x / z). */
    s32 temp;
    s32 angle;
    s32 dx;
    s32 dz;
    s32 best;
    s32 d;

    var_5 = func_80046F14();
    var_7 = rand();
    ofs = rand() & 7;
    base.x = pos[0];
    base.y = pos[1] - 500;
    base.z = pos[2];
    for (i = 0; i < 8; i++) {
        angle = (var_7 + ((ofs + i * 3) << 9)) & 0xFFF;
        dx = (Judge[(angle + 0x400) & 0xFFF] * 4000) / 4096;
        dz = (Judge[angle] * 4000) / 4096;
        var_2.x = base.x + dx;
        var_2.y = base.y;
        var_2.z = base.z + dz;
        if (collision_TestSegmentWithWork(
                &base.x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) != 0) {
            continue;
        }
        cur = base;
        for (temp = 1; temp < 41; temp++) {
            var_2.y = base.y;
            var_2.x = base.x + (dx * temp) / 40;
            var_2.z = base.z + (dz * temp) / 40;
            if (func_8005344C(&cur.x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) !=
                0) {
                break;
            }
            *out = var_2;
            cur = *out;
        }
        var_2 = *out;
        var_2.y += 4000;
        if (collision_TestSegmentWithWork(
                &out->x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) == 0) {
            continue;
        }
        if (var_4[1] != -0x1000) {
            continue;
        }
        cur = *out;
        for (temp = 1; temp < 41; temp++) {
            var_2.y = out->y + temp * 100;
            if (func_8005344C(&cur.x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) !=
                    0 &&
                var_4[1] == -0x1000) {
                out->y = var_3.y;
                return;
            }
            cur.y = var_2.y;
        }
    }
    if (D_800A38DC == 3) {
        temp = D_800A38E0;
    } else {
        best = 0x7FFFFFFF;
        var_5 += D_800A36A4 * 24;
        for (i = 0, temp = 0; i < 4; i++) {
            dx = var_5[i * 6 + 3] - pos[0];
            dz = var_5[i * 6 + 5] - pos[2];
            d = dx * dx + dz * dz;
            if (d < best) {
                best = d;
                temp = i;
            }
        }
    }
    var_5 += temp * 6 + 3;
    out->x = var_5[0];
    out->y = var_5[1];
    out->z = var_5[2];
}

void func_80022224(s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[4];
    s32 picks[2];
    s16 *base;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 j;

    base = (s16 *)func_80046F14() + (D_800A36A4 * 3) * 8;
    for (i = 0; i < 4; i++) {
        dx = base[i * 6 + 3] - arg2[0];
        dz = base[i * 6 + 5] - arg2[2];
        dists[i] = dx * dx + dz * dz;
    }

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

    for (i = 0, j = 0; i < 4; i++) {
        if (dists[i] != -1) {
            picks[j++] = i;
        }
    }

    base += picks[rand() & 1] * 6 + 3;
    arg1[0] = base[0];
    arg1[1] = base[1];
    arg1[2] = base[2];
}

s32 fight_FindNearestStartPair(Vec3i32 *arg0) {
    s16 *p;
    s32 i;
    s32 best_dist;
    s32 best;
    s32 x;
    s32 z;
    s32 dx;
    s32 dz;
    s32 dist;

    p = func_80046F14();
    best_dist = 0x7FFFFFFF;
    x = arg0->x;
    z = arg0->z;
    p += D_800A36A4 * 24;
    for (i = 0; i < 4; i++) {
        dx = (p[i * 6 + 0] + p[i * 6 + 3]) / 2 - x;
        dz = (p[i * 6 + 2] + p[i * 6 + 5]) / 2 - z;
        dist = dx * dx + dz * dz;
        if (dist < best_dist) {
            best_dist = dist;
            best = i;
        }
    }
    return best;
}

s32 func_800224E0(Unk80101EC8Record *arg0) {
    u8 *p;
    s32 val;
    s32 i;
    s32 j;

    p = D_8008EB1C[D_800A384C];
    val = D_8008DB1C[arg0->other->unk_0A][arg0->other->unk_0E];
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 3; i++) {
            if (p[j] == ((val >> (i * 4)) & 0xF)) {
                return i;
            }
        }
    }
    return 0;
}

void func_80022568(Unk80101EC8Record *arg0) {
    arg0->unk_26C = 1;
    arg0->unk_26E = 0;
    arg0->unk_270 = 0;
    arg0->unk_272 = 0;
}

void fighter_Init(s32 idx, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    Unk80101EC8Record *p;
    Vec3i32 var_1;
    s32 var_2;
    s32 ang;
    s32 i;
    s32 var_5;

    p = &D_80101EC8[idx];
    D_800A3758 = 0xFF;
    D_800A376E = 0;
    p->update_count = 0;
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
        var_2 = (D_800A38E2 - 1) / 10 + 1;
        p->unk_1C = var_2 * 96 + ((var_2 == 9) ? 0xA00 : 0x800);
        if (var_2 == 9) {
            p->unk_20 = 0x1080;
        } else {
            p->unk_20 = var_2 * 64 + 0xC00;
        }
        if (var_2 == 9) {
            p->unk_08 = 0xE00;
        } else {
            p->unk_08 = var_2 * 80 + 0x150;
        }
    } else {
        p->unk_84 = D_8008DD5C[p->unk_0A][p->unk_0E];
    }

    p->unk_88 = -1;
    p->unk_8A = 0;
    p->unk_8E = -1;
    p->unk_90 = 0;
    var_5 = (D_800A3670 != 0) ? D_800A38DF : 0;

    switch (D_800A38DC) {
    case 0:
        if (arg4 != 0) {
            var_1 = p->other->base_pos;
            func_80022224(idx, &p->base_pos.x, &var_1.x);
        } else {
            /* FAKE: default's two fight_GetStartPosition calls repeated in this
             * arm (duplicated-statement-into-arms, Q47); cross-jump merges the
             * copies. With the copy, the dispatch tests mode == 0 first and
             * reaches case 2/3 through the target's `j`; every shared form
             * lays the dispatch out in another order. */
            fight_GetStartPosition(idx, &p->base_pos.x, var_5);
            fight_GetStartPosition(idx == 0, &var_1.x, var_5);
        }
        break;
    case 2:
    case 3:
        if (arg4 != 0) {
            var_1 = p->other->base_pos;
            func_80021DB0(idx, &p->base_pos, &var_1.x);
        } else {
            fight_GetStartPosition(idx, &p->base_pos.x, D_800A38E0);
            fight_GetStartPosition(idx == 0, &var_1.x, D_800A38E0);
        }
        break;
    default:
        fight_GetStartPosition(idx, &p->base_pos.x, var_5);
        fight_GetStartPosition(idx == 0, &var_1.x, var_5);
        break;
    }

    p->root_offset.x = 0;
    p->root_offset.y = -0x384;
    p->root_offset.z = 0;
    p->world_pos.x = p->base_pos.x + p->root_offset.x;
    p->world_pos.y = p->base_pos.y + p->root_offset.y;
    p->world_pos.z = p->base_pos.z + p->root_offset.z;
    var_1.x += p->root_offset.x;
    var_1.y += p->root_offset.y;
    var_1.z += p->root_offset.z;
    p->collision_pos.vx = p->world_pos.x;
    p->collision_pos.vy = p->base_pos.y;
    p->collision_pos.vz = p->world_pos.z;
    p->prev_collision_pos = p->collision_pos;
    p->unk_1F8 = p->root_offset;
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
    p->unk_148 = p->collision_pos.vy;
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
    p->unk_168 = p->world_pos;
    p->unk_174 = p->world_pos;
    p->unk_180 = p->world_pos;
    p->unk_18C = p->world_pos;
    p->orientation.vx = 0;
    p->orientation.vy = 0;
    p->orientation.vz = 0;
    ang = ratan2(var_1.x - p->world_pos.x, var_1.z - p->world_pos.z);
    p->orientation.vy = p->opponent_bearing = ang;
    if (p->unk_0C == 0x1F) {
        p->orientation.vy = ang + 0x800;
    }
    p->unk_1D0 = p->orientation;
    fighter_ScaleBoneHitRecords(idx, p->unk_1A);
    if (D_800A38DC != 0 || idx != 0 || D_800A3907 < 2 || D_800A3670 != 0) {
        func_80022568(p);
    }
    p->unk_274 = D_8008E3C0[p->unk_0A];
    for (i = 0; i < 4; i++) {
        p->unk_276[i] = D_8008E3F8[p->unk_0A][i];
        p->unk_27E[i] = D_8008E4D0[p->unk_0A][i];
    }
    p->unk_A0 = 8;
    p->queued_move = 0;
    p->pending_move_event = -1;
    p->unk_31A = 0;
    p->unk_1DC = 0;
    p->unk_72 = 0;
    p->is_defeated = 0;
    p->unk_B1 = 7;
    p->unk_B2 = 0;
    p->unk_34C = 0;
    object_InitKindQueue(p);
    p->unk_14 = p->unk_332[0];
    if (D_800A38DC == 5 || (D_800A38DC == 2 && D_800A389A == 0)) {
        if (p->unk_0A == 1 || p->unk_0A == 3 || p->unk_0A == 4 ||
            p->unk_0A == 9 || p->unk_0A == 0x11) {
            p->unk_332[0] = p->unk_14 = 0x11;
        } else {
            p->unk_14 = -1;
            p->unk_330 = 0;
        }
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C != 4) {
        p->unk_14 = -1;
        p->unk_330 = 0;
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C == 4) {
        if (p->other->unk_0A == 1 || p->other->unk_0A == 3 ||
            p->other->unk_0A == 4 || p->other->unk_0A == 9 ||
            p->other->unk_0A == 0x11) {
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
    if (D_800A38DC == 5 || D_800A38DC == 2 || D_800A38DC == 3 ||
        (D_800A38DC == 0 && D_800A385C != 0)) {
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

            switch (D_800A38DC) {
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
                {
                    fighter_InitCpuControl(
                        i, g_motion_fighter_sections[rec->motion_slot].unk_0C,
                        g_motion_fighter_sections[rec->other->motion_slot]
                            .unk_0C);
                }
            }
        }

        tbl++;
        i++;
    } while (i < 2);
}

/* Its scratchpad address holders are
 * a labelled FAKE.
 * The masked state id is spelled as in its
 * copy-paste sibling func_80023E40 (`s32 a0 = arg0->move_kind; s32 v1 = a0 &
 * 0xFFFF;`, same comparison cascade), which keeps the target's
 * `andi $v1,$a0,0xffff`: the raw value stays live for `mode - 0x17`, so
 * combine never folds the mask. A u16 on either side loses it. */
void fighter_ResolveMovement(
    Unk80101EC8Record *obj, VECTOR *pos1, VECTOR *pos2, s32 *arg3) {
    /* FAKE: the scratchpad addresses scratch_c (0x1F8001C0) / scratch_d
     * (0x1F8001D0) and work (0x1F8002B8) are held in locals; as literals they
     * are re-formed at each use (scratch_c: score 29; scratch_d: 17; work: 13).
     */
    s32 *scratch = (s32 *)0x1F8001B0;
    s32 count = 0;
    s32 work;
    s16 *scratch_d;
    s16 *scratch_c = (s16 *)0x1F8001C0;
    work = 0x1F8002B8;
    scratch_d = (s16 *)0x1F8001D0;
loop:
    if (((pos1->vx != pos2->vx) || (pos1->vy != pos2->vy)) ||
        (pos1->vz != pos2->vz)) {
        if (func_8005344C(&pos1->vx, &pos2->vx, scratch, scratch_c, work) ==
            0) {
            *pos1 = *pos2;
            goto done;
        }
        obj->unk_B1 = collision_GetHitTag();
        *pos1 = *(VECTOR *)scratch;
        math_ScaleVectorInDirectionFrame(arg3, scratch_c, arg3, -0x40, 0xE6);
        scratch[12] = pos1->vx + (*((s16 *)(((u8 *)scratch) + 0x10)) / 1024);
        scratch[13] = pos1->vy + (*((s16 *)(((u8 *)scratch) + 0x12)) / 1024);
        scratch[14] = pos1->vz + (*((s16 *)(((u8 *)scratch) + 0x14)) / 1024);
        pos2->vx += *((s16 *)(((u8 *)scratch) + 0x10)) / 1024;
        pos2->vy += *((s16 *)(((u8 *)scratch) + 0x12)) / 1024;
        pos2->vz += *((s16 *)(((u8 *)scratch) + 0x14)) / 1024;
        if (func_8005344C(&pos1->vx, scratch + 12, scratch,
                          (s16 *)(scratch + 6), work) == 0) {
            *pos1 = *(VECTOR *)(scratch + 12);
            scratch[8] = pos2->vx - pos1->vx;
            scratch[9] = pos2->vy - pos1->vy;
            scratch[10] = pos2->vz - pos1->vz;
            math_ScaleVectorInDirectionFrame(
                (s32 *)scratch_d, scratch_c, (s32 *)scratch_d, 0,
                (obj->move_kind == 0x15) ? 0x80 : 0x100);
            if (*((s16 *)(((u8 *)scratch) + 0x12)) >= (-0x7FF)) {
                s32 mode = obj->move_kind;
                s32 m = mode & 0xFFFF;
                if (((((m != 8) && (m != 0x22)) &&
                      (((u32)(mode - 0x17)) >= 2)) &&
                     (m != 0xA)) &&
                    (obj->unk_72 == 0)) {
                    scratch[9] = 0;
                }
            }
            pos2->vx = pos1->vx + scratch[8];
            pos2->vy = pos1->vy + scratch[9];
            count++;
            pos2->vz = pos1->vz + scratch[10];
            if (count < 4) {
                goto loop;
            }
        }
    }

done:;
}

s32 func_800233AC(Unk80101EC8Record *arg0, s32 *arg1) {
    s32 pos[3];
    s32 off[3];
    SVec4i16 out1;
    s32 out2[4];
    u32 bits;
    s32 a1_idx;
    s32 a0_idx;

    bits = arg0->pad_input.held;
    a1_idx = (bits >> 14) & 1;
    if (!(bits & 0x1000)) {
        a1_idx++;
    }
    a0_idx = (bits >> 15) & 1;
    if (!(bits & 0x2000)) {
        a0_idx++;
    }

    {
        /* FAKE: pointer alias to D_8008EB40 (pointer-alias-fake-exception): the
         * row pointer puts the table address in a register ahead of the
         * a0_idx * 6 row offset, as the target does; written directly as
         * D_8008EB40[a0_idx], EXPAND_SUM keeps the base a constant term until
         * after the index insns (direct D_8008EB40[a0_idx]: score 13).
         * D_8008EB40[a0_idx][a1_idx] and D_8008EB40 + a0_idx * 3 also miss. */
        s16(*tbl)[3] = D_8008EB40;
        s32 px;
        s16 *row;
        s32 a1_val;

        px = arg0->collision_pos.vx;
        row = tbl[a0_idx];
        a1_val = row[a1_idx];

        pos[0] = px;
        pos[1] = arg0->collision_pos.vy - 0x64;
        pos[2] = arg0->collision_pos.vz;

        {
            s32 angle = (arg0->opponent_bearing + a1_val) & 0xFFF;
            s16 var_12 = Judge[angle];

            off[0] = px + var_12 / 4;
            off[1] = pos[1];
        }

        {
            s32 angle2 = (arg0->opponent_bearing + a1_val + 0x400) & 0xFFF;
            s16 var_14 = Judge[angle2];

            off[2] = pos[2] + var_14 / 4;
        }
    }

    if (collision_TestSegmentWithWork(
            pos, off, out2, &out1.vx, (s32)0x1F8002B8) == 0) {
        return 0;
    }

    arg0->unk_98 = out1;

    {
        s32 var_15 = ratan2(out1.vx, out1.vz);
        s32 var_16;

        pos[0] = arg0->collision_pos.vx;
        pos[1] = arg0->collision_pos.vy - 0x898;
        pos[2] = arg0->collision_pos.vz;

        var_16 = var_15 + 0x800;

        {
            s32 angle3 = var_16 & 0xFFF;
            s16 var_18 = Judge[angle3];
            off[0] = pos[0] + var_18 / 8;
        }

        off[1] = pos[1];

        {
            s32 angle4 = (var_15 + 0xC00) & 0xFFF;
            s16 var_20 = Judge[angle4];
            off[2] = pos[2] + var_20 / 8;
        }

        if (collision_TestSegmentWithWork(
                pos, off, out2, &out1.vx, (s32)0x1F8002B8) != 0) {
            return 0;
        }

        pos[0] = off[0];
        pos[1] = off[1] + 0x190;
        pos[2] = off[2];

        if (collision_TestSegmentWithWork(
                off, pos, out2, &out1.vx, (s32)0x1F8002B8) == 0) {
            return 0;
        }

        *arg1 = var_16;
        return 1;
    }
}

void func_80023648(Unk80101EC8Record *arg0) {
    u16 kind = arg0->move_kind;
    /* FAKE: pointer alias to D_8008EB40 (pointer-alias-fake-exception), as
     * in func_800233AC: the table
     * address is in a register ahead of the a0 * 6 row offset; direct
     * D_8008EB40[a0]: score 14. */
    s16(*tbl)[3];

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        s32 a2;
        u32 bits = arg0->pad_input.held;
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
            a2 = row[a1];

            if (D_800A38BA != 0 && arg0->unk_06 == 0) {
                fighter_SetTurnDelta(arg0, arg0->orientation.vy + a2 / 4);
            } else {
                fighter_SetTurnDelta(arg0, arg0->opponent_bearing + a2);
            }
        } else {
            if (D_800A38BA != 0 && arg0->unk_06 == 0) {
                arg0->unk_14C = 0;
            }
        }

        {
            /* FAKE: the clamped |arg0->unk_150| is staged through the existing
             * `a2` (its table-entry value is dead here): one multiply-set
             * pseudo keeps every staged value in $a2, as in the target;
             * separate locals land in $a2/$a0/$a1 (a fresh local here:
             * score 15). */
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
                 * the dead `sub_result`, as above; reading in place: score
                 * 11. */
                sub_result = arg0->unk_1A;
                mult_res = sub_result * tbl_val;
                limit = (mult_res << 4) >> 12;

                if (limit < (s16)new_14e) {
                    arg0->unk_14E = limit;
                } else if ((s16)new_14e < 0) {
                    arg0->unk_14E = 0;
                }

                {
                    s32 var_13 = arg0->unk_14E * arg0->move_frame_step;
                    s16 sin_val = Judge[(arg0->orientation.vy & 0xFFF)];

                    /* FAKE: the >>12 speed is staged through the dead `a2`, as
                     * above; a fresh local or the shift inline: score 30. */
                    a2 = var_13 >> 12;

                    arg0->base_pos.x += (sin_val * a2) >> 16;

                    {
                        s16 cos_val =
                            Judge[((arg0->orientation.vy + 0x400) & 0xFFF)];
                        arg0->base_pos.z += (cos_val * a2) >> 16;
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

void func_800238C4(Unk80101EC8Record *arg0) {
    s32 src[4];
    s32 dst[4];
    s32 out[4];
    s16 offsets[4];
    s16 offsets2[4];
    s32 s1;
    s32 dx_delta;
    s32 dz_delta;
    s32 scratchpad;
    s32 var_9;
    u16 kind;
    if (arg0->unk_104.vy <= 0) {
        return;
    }
    kind = arg0->move_kind;
    if (kind == 0x17 || kind == 0x18 || kind == 0xA || arg0->unk_72 != 0 ||
        kind == 1 || kind == 0x28) {
        return;
    }
    scratchpad = 0x1F8002B8;
    src[0] = arg0->collision_pos.vx;
    src[1] = arg0->collision_pos.vy - 0xC8;
    src[2] = arg0->collision_pos.vz;
    dst[0] = arg0->collision_pos.vx;
    dst[1] = arg0->collision_pos.vy + 0x514;
    dst[2] = arg0->collision_pos.vz;
    s1 = collision_TestSegmentWithWork(src, dst, out, offsets, scratchpad);
    var_9 = 1;
    if (s1 != 0) {
        if (out[1] - src[1] > 0x190) {
            var_9 = arg0->move_kind != 0x22;
        } else if (offsets[1] >= -0x7FF) {
            dst[0] += offsets[0] / 8;
            dst[2] += offsets[2] / 8;
            var_9 = collision_TestSegmentWithWork(
                        src, dst, out, offsets2, scratchpad) == 0;
        } else {
            var_9 = 0;
        }
    }
    if (!var_9) {
        return;
    }
    if (s1 == 0) {
        offsets[0] = arg0->collision_pos.vx - arg0->prev_collision_pos.vx;
        offsets[2] = arg0->collision_pos.vz - arg0->prev_collision_pos.vz;
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
    } else {
        dx_delta = offsets[0] / 64;
        dz_delta = offsets[2] / 64;
    }
    s1 = (arg0->orientation.vy - ratan2(offsets[0], offsets[2])) & 0xFFF;
    if (s1 >= 0x800) {
        s1 = 0x1000 - s1;
    }
    kind = arg0->move_kind;
    if (kind == 0xF || kind == 0x1C || kind == 0x1D || kind == 0x1E ||
        kind == 0x1F || kind == 0x20 || kind == 0x21) {
        if (s1 < 0x400) {
            arg0->pending_move_event = 0;
            arg0->unk_94 = 0;
        } else {
            arg0->pending_move_event = 1;
            arg0->unk_94 = 1;
        }
        arg0->other->pending_move_event = 2;
        arg0->unk_74 = arg0->collision_pos.vy;
    } else if (kind == 0x11) {
        D_800A3769 = s1 < 0x400;
        D_800A3758 = arg0->index;
    } else {
        if (s1 < 0x400) {
            arg0->pending_move_event = 0x12;
            arg0->unk_94 = 0;
        } else {
            arg0->pending_move_event = 0x11;
            arg0->unk_94 = 1;
        }
        arg0->unk_74 = arg0->collision_pos.vy;
    }
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

void fighter_RequestPoseBlend(Unk80101EC8Record *arg0, s16 arg1) {
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
        arg0->unk_154 = arg0->opponent_bearing;
    }
}

void func_80023D08(Unk80101EC8Record *arg0) {
    fighter_RequestPoseBlend(arg0, 0x200);
}

void func_80023D28(Unk80101EC8Record *arg0) {
    if (arg0->unk_104.vy < 0) {
        arg0->unk_1DC = 0;
        return;
    }
    ((VECTOR *)0x1F8001E0)->vx = arg0->collision_pos.vx;
    ((VECTOR *)0x1F8001E0)->vy = arg0->collision_pos.vy + 0x1F4;
    ((VECTOR *)0x1F8001E0)->vz = arg0->collision_pos.vz;
    arg0->unk_1DC =
        func_8005344C(&arg0->collision_pos.vx, (s32 *)0x1F8001E0,
                      (s32 *)0x1F8001B0, (s16 *)0x1F8001C0, 0x1F8002B8);
}

s32 func_80023DB8(Unk80101EC8Record *arg0) {
    s32 result;
    ((VECTOR *)0x1F8001E0)->vx = arg0->collision_pos.vx;
    ((VECTOR *)0x1F8001E0)->vy = arg0->collision_pos.vy + 5;
    ((VECTOR *)0x1F8001E0)->vz = arg0->collision_pos.vz;
    if (func_8005344C(&arg0->collision_pos.vx, (s32 *)0x1F8001E0,
                      (s32 *)0x1F8001B0, (s16 *)0x1F8001C0, 0x1F8002B8) != 0) {
        result = *(s16 *)0x1F8001C2 < -0x800;
    } else {
        result = 0;
    }
    return result;
}

void func_80023E40(Unk80101EC8Record *arg0) {
    s32 *s1 = (s32 *)0x1F8001B0;
    u16 state = arg0->move_kind;

    if (state == 8 || state == 0x22) {
        return;
    }
    if (state == 0x17 || state == 0x18 || state == 0x28 || state == 0xA) {
        s1[0xC] = arg0->collision_pos.vx;
        s1[0xD] = arg0->collision_pos.vy + 0x1F40;
        s1[0xE] = arg0->collision_pos.vz;
        if (collision_TestSegmentWithWork(&arg0->collision_pos.vx, &s1[0xC], s1,
                                          (s16 *)&s1[4], 0x1F8002B8) != 0) {
            arg0->unk_148 = s1[1];
        }
    } else {
        arg0->unk_148 = arg0->collision_pos.vy;
    }
}

/* [unk_0A][min(unk_272, 3)]: 4.12 scale of unk_1E into unk_20 (fighter_Update).
 */
extern s16 D_8008E0BC[27][4];
/* [D_8008D9EC[unk_0A]][unk_0E]: the move command fighter_Update hands
 * object_SpawnScatteredAt. */
extern u8 D_8008D90C[28][8];
extern void func_800204C0(Unk80101EC8Record *);

/* Per-frame update of character `arg0`'s record from this frame's pad input:
 * converts the pad bits, advances the move frame, runs the move script's
 * command list (cur_move / queued_move, motion_ResolveMoveData /
 * func_80021A98), decodes two MotionFrames with func_800198D0 (at move_frame
 * and at move_frame + 1, capped at unk_58[1] - 1, each offset by *unk_54, and
 * by unk_58[1] in some states; state move_kind 0x33 instead copies
 * g_motion_frame_buffers[arg0][move_frame] into both); when unk_31A is 0 it
 * mixes their root offsets into root_offset by the 4.12 weight move_frame_frac,
 * otherwise the first one's (by the weight unk_318) with unk_290's less unk_320
 * (by 0x1000 - unk_318), integrates the velocities (unk_104 / unk_134) into the
 * position, resolves it (fighter_ResolveMovement), then refreshes the bone
 * points, the flags unk_62, unk_AE and unk_288, the damping and the per-frame
 * side effects. */
void fighter_Update(s32 arg0, PadState *pad) {
    MotionFrame var_0[2];
    Vec3i32 v[2];
    VECTOR pos;
    s16 ang[2];
    s32 var_4;
    s16 var_5;
    Unk80101EC8Record *rec;
    u16 var_7;
    u16 var_8;
    u16 *ent;
    MoveScript *move;
    /* FAKE: one local reused for four values (no-new-park-categories entry
     * 1): the unk_14C clamp limit, the
     * opponent_bearing / orientation.vy angle gap (folded to 0..0x800), the
     * unk_14C turn step and the -1/0/1 stick side. The target keeps all four in
     * $a1; one local per value: score 25. */
    s32 temp;
    s32 var_12;
    s32 var_13;
    s32 var_14;
    s32 var_15;
    s32 next;
    s32 var_17;
    s32 var_18;
    s32 var_19;
    s32 var_20;
    s16 frac;
    s32 var_22;
    s32 ang_q;

    rec = &D_80101EC8[arg0];
    rec->update_count++;
    rec->pad_input = *pad;
    if (D_800A38BA != 0 && (arg0 == 0 || (arg0 == 1 && rec->unk_06 == 0))) {
        rec->pad_input.held = (rec->pad_input.held & 0xFFF) |
                              ((rec->pad_input.held & 0x8000) >> 3) |
                              ((rec->pad_input.held & 0x7000) << 1);
        rec->pad_input.pressed =
            (rec->pad_input.pressed & 0xFFF) |
            ((rec->pad_input.pressed & 0x8000) >> 3) |
            ((rec->pad_input.pressed & 0x7000) << 1);
        rec->pad_input.released =
            (rec->pad_input.released & 0xFFF) |
            ((rec->pad_input.released & 0x8000) >> 3) |
            ((rec->pad_input.released & 0x7000) << 1);
        rec->pad_input.unheld = (rec->pad_input.unheld & 0xFFF) |
                                ((rec->pad_input.unheld & 0x8000) >> 3) |
                                ((rec->pad_input.unheld & 0x7000) << 1);
    }
    if (D_800A38DC == 2 || D_800A38DC == 5 || D_800A38DC == 3 ||
        (D_800A38DC == 0 && D_800A385C != 0)) {
        rec->pad_input.pressed &= ~0x100;
    }
    if (rec->update_count < 2) {
        rec->pad_input.held = rec->pad_input.pressed = rec->pad_input.released =
            0;
        rec->pad_input.unheld = 0xFFFF;
    }
    rec->unk_20 =
        (rec->unk_1E *
         D_8008E0BC[rec->unk_0A][rec->unk_272 >= 4 ? 3 : rec->unk_272]) >>
        12;
    if (rec->move_started == 1) {
        rec->move_started = 0;
    }
    motion_UpdateFrameStep(rec);
    frac = rec->move_frame_frac + rec->move_frame_step;
    rec->move_frame_frac = frac;
    if (frac >= 0x1000) {
        rec->unk_46 = 0;
        rec->move_frame += frac >> 12;
        rec->move_frame_frac &= 0xFFF;
    } else {
        rec->unk_46 = 1;
    }
    if (rec->pending_move_event >= 0) {
        /* the move-script record of cur_move's follow-up id is a u16 move-id
         * table indexed by event code (here pending_move_event; 0xD, 0x17/0x18
         * below) */
        u16 *table;

        if (rec->move_kind == 0xA || rec->move_kind == 0x17 ||
            rec->move_kind == 0x18) {
            rec->unk_72 = 1;
        }
        rec->unk_31A = 0;
        table = motion_ResolveMoveData(
            rec, rec->cur_move->unk_00, &rec->move_is_common);
        func_80021A98(
            arg0,
            motion_ResolveMoveData(
                rec, table[rec->pending_move_event], &rec->move_is_common),
            rec->move_is_common);
        rec->pending_move_event = -1;
    }
    if ((rec->pad_input.held & 8) && (rec->pad_input.held & 0xF000)) {
        if ((rec->move_kind == 0x15 || rec->move_kind == 0x19 ||
             rec->move_kind == 0x1A || rec->move_kind == 0x13 ||
             rec->move_kind == 0x30 || rec->move_kind == 0x31) &&
            func_800233AC(rec, &var_4) != 0) {
            u16 *table;

            fighter_SetTurnDelta(rec, var_4);
            rec->unk_31A = 0;
            table = motion_ResolveMoveData(
                rec, rec->cur_move->unk_00, &rec->move_is_common);
            func_80021A98(
                arg0,
                motion_ResolveMoveData(rec, table[0xD], &rec->move_is_common),
                rec->move_is_common);
            rec->unk_104.vx = 0;
            rec->unk_104.vy = 0;
            rec->unk_104.vz = 0;
        }
    }
    if (rec->cur_move->unk_07 < rec->move_frame) {
        if (rec->queued_move != NULL) {
            rec->move_is_common = rec->queued_move_is_common;
            if (rec->unk_82 & 0x1000) {
                rec->borrow_other_motion = 1;
            }
            func_80021A98(arg0, rec->queued_move, rec->queued_move_is_common);
        } else {
            if ((rec->cur_move->unk_0A[0] & 0x2000) && D_800A38AE != arg0) {
                if (rec->cur_move->unk_0A[0] & 0x1000) {
                    rec->borrow_other_motion = 1;
                }
                func_80021A98(
                    arg0,
                    motion_ResolveMoveData(
                        rec, rec->cur_move->unk_0A[1], &rec->move_is_common),
                    rec->move_is_common);
            } else {
                if (rec->cur_move->unk_09 & 0x20) {
                    rec->borrow_other_motion = 1;
                }
                func_80021A98(arg0,
                              motion_ResolveMoveData(rec, rec->cur_move->unk_02,
                                                     &rec->move_is_common),
                              rec->move_is_common);
            }
        }
    }
    if (rec->queued_move != NULL) {
        if (rec->cur_move->unk_08 < rec->move_frame) {
            if (rec->move_kind == 0x11) {
                D_800A376E = 1;
                D_800A36D8 = rec->queued_move;
                D_800A381C = rec->queued_move_is_common;
                D_800A36CA = rec->unk_82;
            } else {
                rec->move_is_common = rec->queued_move_is_common;
                if (rec->unk_82 & 0x1000) {
                    rec->borrow_other_motion = 1;
                }
                func_80021A98(
                    arg0, rec->queued_move, rec->queued_move_is_common);
            }
        }
    } else if (rec->cur_move->unk_08 >= rec->move_frame) {
        ent = rec->cur_move->unk_0A;
        while ((var_7 = ent[0]) != 0) {
            if (!(var_7 & 0x8000) ||
                ((ent[2] | ((u32)ent[3] << 16)) & (1 << rec->unk_0A))) {
                switch (var_7 & 0x30) {
                case 0x00:
                    var_8 = rec->pad_input.held;
                    break;
                case 0x10:
                    var_8 = rec->pad_input.pressed;
                    break;
                case 0x20:
                    var_8 = rec->pad_input.unheld;
                    break;
                case 0x30:
                    var_8 = rec->pad_input.released;
                    break;
                }
                if (var_7 & 0x2000) {
                    var_8 = 0;
                }
                if (rec->move_kind == 0x11 && D_800A38AE == arg0) {
                    var_8 = 0;
                }
                if ((var_7 & 0xF) == 9) {
                    var_7 = (var_7 & 0xFFF0) | 3;
                    if (rec->other->move_kind != 6 &&
                        rec->other->move_frame < rec->other->unk_AB) {
                        var_8 = 0;
                    }
                } else if ((var_7 & 0xF) == 0xA) {
                    var_7 = (var_7 & 0xFFF0) | 5;
                    if (rec->unk_26C == 0) {
                        var_8 = 0;
                    }
                }
                if ((var_8 >> (var_7 & 0xF)) & 1) {
                    rec->unk_B0 = var_7;
                    if (var_7 & 0x1000) {
                        rec->borrow_other_motion = 1;
                    }
                    move = motion_ResolveMoveData(rec, ent[1], &var_5);
                    if (rec->cur_move->unk_09 & 0x80) {
                        rec->queued_move = move;
                        rec->unk_82 = var_7;
                        rec->borrow_other_motion = 0;
                        rec->queued_move_is_common = var_5;
                    } else if (rec->move_kind == 0x11) {
                        D_800A376E = 1;
                        D_800A36D8 = move;
                        D_800A36CA = var_7;
                        D_800A381C = var_5;
                    } else {
                        rec->move_is_common = var_5;
                        func_80021A98(arg0, move, var_5);
                    }
                    break;
                }
            }
            if (var_7 & 0xC000) {
                ent += 4;
            } else {
                ent += 2;
            }
        }
    }
    if (rec->move_kind == 0xA || rec->unk_72 != 0) {
        if (func_80023DB8(rec) != 0) {
            if (rec->unk_74 + 0xFA0 < rec->collision_pos.vy) {
                u16 *table;

                table = motion_ResolveMoveData(
                    rec, rec->cur_move->unk_00, &rec->move_is_common);
                func_80021A98(arg0,
                              motion_ResolveMoveData(
                                  rec, table[rec->unk_94 != 0 ? 0x17 : 0x18],
                                  &rec->move_is_common),
                              rec->move_is_common);
            } else if (rec->unk_72 == 0) {
                func_80021A98(arg0,
                              motion_ResolveMoveData(rec, rec->cur_move->unk_02,
                                                     &rec->move_is_common),
                              rec->move_is_common);
            }
            rec->unk_72 = 0;
        }
    } else if (rec->move_started != 0 &&
               (rec->prev_move_kind == 0x17 || rec->prev_move_kind == 0x18) &&
               !(rec->move_kind == 0x17 || rec->move_kind == 0x18) &&
               rec->unk_74 + 0xFA0 < rec->collision_pos.vy) {
        u16 *table;

        table = motion_ResolveMoveData(
            rec, rec->cur_move->unk_00, &rec->move_is_common);
        func_80021A98(
            arg0,
            motion_ResolveMoveData(rec, table[0x18], &rec->move_is_common),
            rec->move_is_common);
    }
    if (rec->unk_70 == 1) {
        fighter_SetTurnDelta(rec, rec->opponent_bearing);
    }
    if (rec->move_started != 0) {
        if (rec->move_kind == 2 || rec->move_kind == 0x1B ||
            rec->move_kind == 0x28 || rec->move_kind == 0x26 ||
            rec->move_kind == 0x24) {
            fighter_SetTurnDelta(rec, rec->opponent_bearing);
            if (rec->move_kind == 2 || rec->move_kind == 0x24) {
                temp = (rec->unk_58[2] >> 4) * 0x88;
                if (rec->unk_14C > temp) {
                    rec->unk_14C = temp;
                } else if (rec->unk_14C < -temp) {
                    rec->unk_14C = -temp;
                }
            }
        }
    }
    if (rec->unk_46 == 0 && rec->move_kind == 9 &&
        rec->unk_A9 == rec->move_frame) {
        fighter_SetTurnDelta(rec, rec->opponent_bearing);
    }
    if (rec->move_started != 0 && rec->prev_move_kind != 0x18 &&
        rec->move_kind == 0x18) {
        rec->unk_104.vx +=
            (Judge[rec->orientation.vy & 0xFFF] * D_8008DA94[rec->unk_0A]) >>
            12;
        rec->unk_104.vz += (Judge[(rec->orientation.vy + 0x400) & 0xFFF] *
                            D_8008DA94[rec->unk_0A]) >>
                           12;
        rec->unk_104.vy += D_8008DA50[rec->unk_0A];
        rec->unk_74 = rec->collision_pos.vy;
    }
    if (rec->move_started != 0 && rec->prev_move_kind != 0x17 &&
        rec->move_kind == 0x17) {
        rec->unk_104.vy += D_8008DAD8[rec->unk_0A];
        rec->unk_74 = rec->collision_pos.vy;
    }
    if (rec->move_started != 0 && rec->move_kind == 0x28) {
        s32 dv[3];
        s32 var_30[3];

        temp = (rec->opponent_bearing - rec->orientation.vy) & 0xFFF;
        if (temp >= 0x800) {
            temp = 0x1000 - temp;
        }
        var_30[0] = rec->other->unk_18C.x -
                    ((Judge[rec->opponent_bearing & 0xFFF] * temp) >> 14);
        var_30[1] = rec->other->unk_18C.y;
        var_30[2] =
            rec->other->unk_18C.z -
            ((Judge[(rec->opponent_bearing + 0x400) & 0xFFF] * temp) >> 14);
        rec->unk_104.vy -= 0xFA;
        func_800200DC((s32 *)&rec->unk_180, var_30, rec->unk_104.vy, 0x1F, dv);
        rec->unk_104.vx += dv[0];
        rec->unk_104.vz += dv[2];
        rec->unk_72 = 1;
        rec->unk_74 = rec->collision_pos.vy;
    }
    if (rec->move_kind == 0x17 || rec->move_kind == 0x18 ||
        rec->move_kind == 0x28 || rec->move_kind == 0xA) {
        if (rec->move_frame == rec->cur_move->unk_07 && rec->unk_104.vy > 0) {
            rec->move_frame = rec->cur_move->unk_07 - 1;
        }
    }
    if (rec->move_started != 0 && rec->move_kind == 0x23) {
        rec->collision_pos.vy -= 0x898;
        rec->base_pos.y -= 0x898;
        rec->unk_1F8.y += 0x898;
    }
    {
        s32 state = rec->move_kind;

        if (rec->move_started != 0 && state == 6) {
            rec->unk_86 = rec->unk_84;
        }
    }
    temp = (rec->unk_14C * rec->move_frame_step) / 24576;
    rec->orientation.vy += temp;
    rec->unk_14C -= temp;
    if (rec->move_started != 0) {
        if (rec->unk_78 != 0) {
            func_80023D08(rec);
        } else if (rec->move_kind == 0x1C) {
            fighter_RequestPoseBlend(rec, 0x400);
        } else {
            if (rec->prev_move_kind == 0x1C || rec->move_kind == 0x11 ||
                rec->prev_move_kind == 0x11 || rec->move_kind == 0x15 ||
                rec->move_kind == 0xA || rec->move_kind == 0x2D) {
                func_80023D08(rec);
            } else {
                switch ((rec->prev_move_kind << 8) | rec->move_kind) {
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
    next = rec->move_frame + 1;
    if (next >= rec->unk_58[1]) {
        next = rec->unk_58[1] - 1;
    }
    if (rec->cur_move->unk_09 & 0x40) {
        var_18 = rec->move_is_common == 0
                     ? D_80101EC8[D_800A38AE].motion_slot + 1
                     : 0;
        if (rec->move_kind == 0x11 && D_800A38AE != arg0) {
            var_19 = rec->unk_58[1];
        } else {
            var_19 = 0;
        }
    } else {
        var_18 = rec->move_is_common == 0 ? rec->motion_slot + 1 : 0;
        if (rec->move_kind == 0x15 && D_8008D9EC[rec->unk_0A] != 0) {
            var_19 = rec->unk_58[1];
        } else {
            var_19 = 0;
        }
    }
    var_15 = var_19 + *rec->unk_54 + rec->move_frame;
    var_17 = var_19 + *rec->unk_54 + next;
    rec->unk_64 = (var_18 << 14) | var_15;
    rec->unk_66 = (var_18 << 14) | var_17;
    if (rec->move_kind == 0x33) {
        var_0[1] = g_motion_frame_buffers[arg0][rec->move_frame];
        var_0[0] = var_0[1];
    } else {
        func_800198D0(var_18, var_15, &var_0[0], (u16 *)0x1F8001B0);
        func_800198D0(var_18, var_17, &var_0[1], (u16 *)0x1F8001B0);
    }
    if (rec->move_started != 0 &&
        (rec->move_kind == 6 || rec->move_kind == 0x14) &&
        rec->prev_move_kind == 6) {
        MATRIX m1;
        MATRIX m2;
        s32 old_heading;
        s32 new_heading;
        s32 var_34;

        math_RotMatrixZYXAngles(
            rec->unk_290.unk_06, rec->unk_290.unk_08, rec->unk_290.unk_0A, &m1);
        math_RotMatrixZYXAngles(
            var_0[0].unk_06, var_0[0].unk_08, var_0[0].unk_0A, &m2);
        old_heading = ratan2(m1.m[0][0], m1.m[2][0]);
        new_heading = ratan2(m2.m[0][0], m2.m[2][0]);
        var_34 = new_heading - old_heading;

        rec->orientation.vy += var_34;
        func_8001B690(arg0, var_34);
    }
    motion_AdjustFighterPose(rec, &var_0[0], &var_0[1]);
    if (rec->move_kind == 0x11) {
        rec->unk_154 = rec->orientation.vy;
    }
    if (rec->unk_152 != 0) {
        var_12 = rec->unk_154;
    } else {
        var_12 = rec->orientation.vy;
    }
    ang[0] = ang[1] = var_12;
    var_13 = var_12 + 0x400;
    v[0].x =
        (var_0[0].unk_04 * Judge[(var_0[0].unk_02 - var_13 + 0x400) & 0xFFF]) >>
        12;
    v[0].y = -var_0[0].unk_00;
    v[0].z =
        (var_0[0].unk_04 * Judge[(var_0[0].unk_02 - var_13) & 0xFFF]) >> 12;
    v[1].x =
        (var_0[1].unk_04 * Judge[(var_0[1].unk_02 - var_13 + 0x400) & 0xFFF]) >>
        12;
    v[1].y = -var_0[1].unk_00;
    v[1].z =
        (var_0[1].unk_04 * Judge[(var_0[1].unk_02 - var_13) & 0xFFF]) >> 12;
    v[0].x = (v[0].x * rec->unk_1A) >> 12;
    v[0].z = (v[0].z * rec->unk_1A) >> 12;
    v[1].x = (v[1].x * rec->unk_1A) >> 12;
    v[1].z = (v[1].z * rec->unk_1A) >> 12;
    if (rec->move_kind != 8) {
        v[0].y = (v[0].y * rec->unk_1A) >> 12;
        v[1].y = (v[1].y * rec->unk_1A) >> 12;
    }
    if (rec->unk_31A != 0) {
        var_0[1] = var_0[0];
        ang[1] = ang[0];
        var_0[0] = rec->unk_290;
        ang[0] = rec->unk_314;
        var_14 = rec->unk_318;
        rec->unk_66 = rec->unk_64;
        rec->unk_64 = rec->unk_316;
    } else {
        var_14 = rec->move_frame_frac;
    }
    var_22 = 0x1000 - var_14;
    rec->unk_68 = var_14;
    if (rec->unk_31A != 0) {
        Vec3i32 var_35;

        if (rec->move_started != 0) {
            rec->base_pos.x += rec->unk_1F8.x - v[0].x;
            rec->base_pos.z += rec->unk_1F8.z - v[0].z;
            rec->unk_320.x += rec->unk_1F8.x - v[0].x;
            rec->unk_320.z += rec->unk_1F8.z - v[0].z;
        }
        ang_q = ang[0] + 0x400;
        var_35.x = (var_0[0].unk_04 *
                    Judge[(var_0[0].unk_02 - ang_q + 0x400) & 0xFFF]) >>
                   12;
        var_35.y = -var_0[0].unk_00;
        var_35.z =
            (var_0[0].unk_04 * Judge[(var_0[0].unk_02 - ang_q) & 0xFFF]) >> 12;
        var_35.x = (var_35.x * rec->unk_1A) >> 12;
        var_35.y = (var_35.y * rec->unk_1A) >> 12;
        var_35.z = (var_35.z * rec->unk_1A) >> 12;
        var_35.x -= rec->unk_320.x;
        var_35.z -= rec->unk_320.z;
        rec->root_offset.x = (var_35.x * var_22 + v[0].x * var_14) >> 12;
        rec->root_offset.y = (var_35.y * var_22 + v[0].y * var_14) >> 12;
        rec->root_offset.z = (var_35.z * var_22 + v[0].z * var_14) >> 12;
    } else {
        rec->root_offset.x = (v[0].x * var_22 + v[1].x * var_14) >> 12;
        rec->root_offset.y = (v[0].y * var_22 + v[1].y * var_14) >> 12;
        rec->root_offset.z = (v[0].z * var_22 + v[1].z * var_14) >> 12;
    }
    if (rec->move_started != 0 && rec->unk_31A == 0) {
        rec->base_pos.x += rec->unk_1F8.x - rec->root_offset.x;
        rec->base_pos.z += rec->unk_1F8.z - rec->root_offset.z;
        if (rec->move_kind == 0x23 || rec->move_kind == 0xA) {
            rec->base_pos.y += rec->unk_1F8.y - rec->root_offset.y;
        }
    }
    func_80023648(rec);
    func_800238C4(rec);
    if (rec->move_kind != 8 && rec->move_kind != 0x22) {
        rec->unk_104.vy += 0x15;
    }
    {
        s32 state = rec->move_kind;

        if (state == 8 || state == 0x22) {
            rec->unk_134.vx -= rec->unk_98.vx / 256;
            rec->unk_134.vz -= rec->unk_98.vz / 256;
        }
    }
    if (rec->move_started == 0 || rec->move_kind != 0x28) {
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
    rec->base_pos.x += rec->unk_104.vx;
    rec->base_pos.y += rec->unk_104.vy;
    rec->base_pos.z += rec->unk_104.vz;
    if ((rec->move_kind == 7 || rec->move_kind == 0xD) && rec->unk_B4 == 0) {
        temp = (rec->pad_input.held & 0x1000)
                   ? 1
                   : -((rec->pad_input.held & 0x4000) != 0);
        if (temp != 0) {
            rec->unk_134.vx /= 2;
            rec->unk_134.vz /= 2;
            rec->unk_134.vx +=
                (Judge[rec->opponent_bearing & 0xFFF] -
                 Judge[(rec->opponent_bearing + 0x400) & 0xFFF] * temp * 3) /
                128;
            rec->unk_134.vz +=
                (Judge[(rec->opponent_bearing + 0x400) & 0xFFF] +
                 Judge[rec->opponent_bearing & 0xFFF] * temp * 3) /
                128;
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
    if (rec->move_kind == 4 || rec->move_kind == 0x14 || rec->unk_0C == 0x1F) {
        rec->unk_134.vx = 0;
        rec->unk_134.vy = 0;
        rec->unk_134.vz = 0;
    } else {
        rec->base_pos.x += rec->unk_134.vx;
        rec->base_pos.y += rec->unk_134.vy;
        rec->base_pos.z += rec->unk_134.vz;
    }
    pos.vx = rec->base_pos.x + rec->root_offset.x;
    pos.vy = rec->base_pos.y;
    if (rec->move_kind == 8 || rec->move_kind == 0x22) {
        pos.vy += rec->root_offset.y;
    }
    pos.vz = rec->base_pos.z + rec->root_offset.z;
    rec->prev_collision_pos = rec->collision_pos;
    fighter_ResolveMovement(rec, &rec->collision_pos, &pos, &rec->unk_104.vx);
    if (rec->unk_B1 != 7 && rec->unk_B1 != 0) {
        rec->unk_B2 = ((0x2A >> rec->unk_B1) ^ 1) & 1;
    }
    rec->base_pos.x = rec->collision_pos.vx - rec->root_offset.x;
    rec->base_pos.y = rec->collision_pos.vy;
    if (rec->move_kind == 8 || rec->move_kind == 0x22) {
        rec->base_pos.y -= rec->root_offset.y;
    }
    rec->base_pos.z = rec->collision_pos.vz - rec->root_offset.z;
    rec->world_pos.x = rec->base_pos.x + rec->root_offset.x;
    rec->world_pos.y = rec->base_pos.y + rec->root_offset.y;
    rec->world_pos.z = rec->base_pos.z + rec->root_offset.z;
    rec->unk_1F8 = rec->root_offset;
    rec->unk_168.x = rec->world_pos.x;
    rec->unk_168.y = rec->base_pos.y - 0x384;
    rec->unk_168.z = rec->world_pos.z;
    func_80023E40(rec);
    func_80041188(arg0, (u8 *)&var_0[0], (u8 *)&var_0[1], rec->unk_68,
                  (MATRIX *)0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, (s32 *)&rec->world_pos, (s16 *)&rec->orientation, 0,
                  rec->unk_148);
    scratchpad_Restore();
    if (rec->unk_86 == rec->unk_8E &&
        (rec->move_kind == 0x15 || rec->move_kind == 0x19 ||
         rec->move_kind == 0x1A || rec->move_kind == 0x16 ||
         rec->move_kind == 0x30 || rec->move_kind == 0x13 ||
         rec->move_kind == 0x31)) {
        rec->unk_92 = 0;
    } else {
        rec->unk_92 = rec->unk_92 != 0 ? 1 : 2;
    }
    rec->unk_62 = 0;
    if (rec->unk_0C != 0x1F && rec->is_defeated == 0 && rec->unk_92 != 0) {
        rec->unk_62 = 1;
    }
    if ((D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) &&
        (rec->unk_0E == 0 || rec->unk_0E == 1) &&
        (D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
        rec->unk_62 |= 2;
    }
    if (rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) {
        if (func_8002798C(rec) != 0) {
            rec->unk_8C = rec->unk_8C != 0 ? 1 : 2;
            rec->unk_62 |= 4;
        } else {
            rec->unk_8C = 0;
        }
    } else if (
        ((rec->unk_0C == 0x1D || rec->unk_0C == 0xE) &&
         (rec->move_kind == 2 || rec->move_kind == 0x1B ||
          rec->move_kind == 0x28 || rec->move_kind == 0x26) &&
         rec->unk_A1[1] < 0xFF && rec->unk_26C != 0 &&
         rec->move_frame >= rec->unk_A5 && rec->move_frame <= rec->unk_A6) ||
        /* FAKE: this second range test is written unk_A6 >= move_frame;
         * spelled like the first, fold-const factors the two identical
         * tests out of the || (score 11). */
        (rec->unk_0A == 0xE && rec->move_kind == 0x11 && D_800A38AE == arg0 &&
         rec->move_frame >= rec->unk_A5 && rec->unk_A6 >= rec->move_frame)) {
        rec->unk_8C = rec->unk_8C != 0 ? 1 : 2;
        rec->unk_62 |= 4;
    } else {
        rec->unk_8C = 0;
    }
    if (D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) {
        if (D_800A38DC == 3 && arg0 == 1 && D_800A384C == 4) {
            if (rec->other->unk_0A == 1 || rec->other->unk_0A == 3 ||
                rec->other->unk_0A == 4 || rec->other->unk_0A == 9 ||
                rec->other->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else if ((D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
            if (rec->unk_0A == 1 || rec->unk_0A == 3 || rec->unk_0A == 4 ||
                rec->unk_0A == 9 || rec->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else {
            goto skip_62;
        }
        if (rec->move_kind != 4 && rec->move_kind != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 |= 0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 &&
            rec->unk_8C == 0) {
            rec->unk_62 |= 0x20;
        }
    }
skip_62:
    if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
        if ((rec->unk_0E == 0 || rec->unk_0E == 1) && rec->move_kind != 4 &&
            rec->move_kind != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 = (rec->unk_62 | 2) & ~0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 &&
            rec->unk_8C == 0) {
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
        func_80049A2C(
            D_8008EB80[rec->unk_14], (arg0 * 2) | 1, (rec->unk_62 >> 5) & 1);
    }
    rec->unk_290 = var_0[0];
    rec->unk_314 = ang[0];
    rec->unk_316 = rec->unk_64;
    if (rec->unk_31A != 0) {
        if (rec->unk_31A >= 2) {
            rec->unk_31A = 1;
        }
        rec->unk_318 += rec->unk_31C;
        if (rec->unk_318 >= 0x1000) {
            rec->unk_31A = 0;
            rec->unk_290 = var_0[1];
            rec->unk_314 = ang[1];
            rec->unk_316 = rec->unk_66;
            rec->unk_1F8 = v[1];
        }
    }
    rec->opponent_bearing = ratan2(rec->other->world_pos.x - rec->world_pos.x,
                                   rec->other->world_pos.z - rec->world_pos.z);
    rec->unk_24C = rec->unk_104;
    func_80023D28(rec);
    fighter_UpdateBoneWorldData(
        rec, SPAD->unkA8[arg0], SPAD->unk00[arg0], SPAD->unk48[arg0]);
    if (rec->update_count == 1) {
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
    var_20 = 0;
    if ((rec->move_kind == 2 || rec->move_kind == 0x1B ||
         rec->move_kind == 0x28 || rec->move_kind == 0x26) &&
        rec->unk_AD != 0) {
        if ((rec->move_frame >= rec->unk_A1[0] &&
             rec->move_frame <= rec->unk_A3[0]) ||
            (rec->move_frame >= rec->unk_A1[1] &&
             rec->move_frame <= rec->unk_A3[1])) {
            var_20 = 1;
        }
    }
    rec->unk_AE = var_20;
    rec->unk_288[0] = 0;
    rec->unk_288[1] = 0;
    if (rec->unk_AD != 0) {
        if (rec->move_kind == 2 || rec->move_kind == 0x1B ||
            rec->move_kind == 0x28 || rec->move_kind == 0x26) {
            rec->unk_288[0] += 1;
            rec->unk_288[1] += 1;
            if (rec->move_frame < rec->unk_A1[0]) {
                rec->unk_288[0] += 2;
            } else if (rec->move_frame <= rec->unk_A3[0]) {
                rec->unk_288[0] += 4;
            }
            if (rec->move_frame < rec->unk_A1[1]) {
                rec->unk_288[1] += 2;
            } else if (rec->move_frame <= rec->unk_A3[1]) {
                rec->unk_288[1] += 4;
            }
        }
    }
    if (rec->move_kind == 6) {
        rec->unk_15E = 0x600;
        rec->unk_160 = 0x600;
        rec->unk_162 = 0x600;
    } else if (rec->move_kind == 8) {
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
    if (rec->is_defeated == 0 &&
        ((rec->move_started != 0 && rec->prev_move_kind != 4 &&
          rec->prev_move_kind != 0x14 &&
          (rec->move_kind == 4 || rec->move_kind == 0x14)) ||
         (rec->move_kind == 0x11 && arg0 != D_800A38AE &&
          rec->unk_AA == rec->move_frame))) {
        MATRIX **var_37 = func_80046DEC(arg0);
        if (D_800A38DC != 0 || D_8008D9EC[D_80101EC8[0].unk_0A] == 0 ||
            D_800A37A0 != 1 || arg0 != D_800A37A0) {
            func_80032854(arg0, 0x2E, &rec->world_pos.x, 0);
        }
        if (rec->unk_0C != 0x1F) {
            object_SpawnScatteredAt(
                rec, D_8008D90C[D_8008D9EC[rec->unk_0A]][rec->unk_0E],
                (Vec3i32 *)var_37[0x12]->t);
        }
        rec->is_defeated = 1;
        if (D_800A3748 == -1) {
            D_800A3748 = arg0 == 0;
        }
    }
    if (rec->move_started != 0 && rec->unk_0C == 0x1B &&
        rec->move_kind == 0xB && rec->unk_34D == 0) {
        rec->pending_move_event = 0x1F;
    }
    if (rec->unk_46 == 0) {
        if (D_800A38DC != 5 && D_800A38DC != 2 && D_800A38DC != 3 &&
            (D_800A38DC != 0 || D_800A385C == 0)) {
            if (rec->move_kind == 0xB && rec->move_frame == rec->unk_A7) {
                if (rec->unk_0C == 0x1B) {
                    if (rec->unk_34D != 0) {
                        rec->unk_34D--;
                    }
                } else if (rec->unk_26C != 0 &&
                           object_ThrowQueuedKind(rec) == rec->unk_14) {
                    rec->unk_8A = 0;
                }
            }
        }
        if (rec->move_kind == 6) {
            if (object_PickUpNearby(rec) == rec->unk_14) {
                rec->unk_8A = rec->unk_26C;
            }
        } else if (rec->unk_26C != 0 &&
                   (rec->move_kind == 0xC || rec->move_kind == 0x2A) &&
                   rec->move_frame == rec->unk_A8 &&
                   object_PickUpNearby(rec) == rec->unk_14) {
            rec->unk_8A = 1;
        }
        if (rec->move_kind == 0x12 && rec->move_frame == rec->unk_A7 &&
            ((0x78 >> rec->unk_B1) & 1) && rec->unk_26C != 0) {
            func_80032064(rec, ((0x18 >> rec->unk_B1) & 1) ? 1 : 2);
        }
        if (rec->move_kind == 0x10 && rec->move_frame == rec->unk_AC &&
            rec->unk_26C != 0 && rec->unk_34B != 0) {
            rec->unk_34A = 0xA;
            rec->unk_34B--;
            func_80032854(arg0, 0x31, &rec->world_pos.x, 0);
        }
        if (rec->unk_46 == 0 &&
            (rec->move_kind != 0x11 || arg0 == D_800A38AE)) {
            rec->unk_62 |= 0x40;
            fighter_ProcessFrameEvents(rec);
        }
    }
    if (rec->unk_26C != 0) {
        rec->unk_62 |= 0x80;
    }
    if (rec->is_defeated != 0) {
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
    if (rec->move_started == 2) {
        rec->move_started = 0;
    }
    replay_RecordFighter(rec);
}

/* Tail word after motion_ResolveMoveData's five-entry compiler-generated switch
 * table.
 */
const u32 D_80010428[1] = {0x00000000};

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A30EC = 0;
u8 D_800A30F0[4] = {0, 0, 0, 0};
s32 D_800A30F4[2] = {0, 0};
s8 D_800A30FC = -1;
s8 D_800A30FD = -1;
