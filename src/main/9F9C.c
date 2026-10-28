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

const char D_800100A4[] = "%04x %04x %04x\n";
const char D_800100B4[] = "SOUND ID:%d\n";

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

void motion_InitStream(s32 decoder_index, u32 *ptr) {
    Unk800F1B18Rec *base;
    s32 i;
    u32 cur;
    s32 bits;

    bits = 0x20;
    base = &g_motion_decoders[decoder_index];
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

/* Decode entry `frame_index` of g_motion_decoders[decoder_index]'s stream into
   `out` (33 words). The table at stream + 0x70 holds a 24-bit offset per group
   of 8 entries (index frame_index >> 3), in bits from the table's start (0x380
   bits past the stream's); an entry is its group's start plus frame_index & 7
   delta steps. rec->ctr steps through the four slots; of the slot it steps to
   and the one before, one whose ->frame is frame_index is copied out (and the
   step is undone), one holding frame_index - 1 is continued from, and otherwise
   the entry is decoded from its group's start. When index_in_group is below 7,
   the entry
   (->frame, ->pose, ->rate) and the reader state after it are kept in that
   slot. `work` holds the pose (+0x00), the rates (+0x84) and the codes
   (+0x108, copied from rec->code when D_800A30EC is 0). */
void motion_DecodeFrame(
    s32 decoder_index, s32 frame_index, MotionFrame *out, u16 *work) {
    Unk800F1B18Rec *rec;
    u8 *tbl;
    Unk800F1B18Slot *slot;
    Unk800F1B18Slot *prev;
    u32 *ptr;
    u32 cur;
    s32 bits;
    s32 index_in_group;
    s32 group_index;
    s32 var_9;
    /* Ruling 11 (ordinary-c-judge-decidable): two values, both loop indices --
     * the group-start loop's over the 63 values and the post-pass column
     * loop's. */
    s32 idx;
    s32 channel_index;
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

    index_in_group = frame_index & 7;
    group_index = frame_index >> 3;
    rec = &g_motion_decoders[decoder_index];
    tbl = (u8 *)rec->stream + 0x70;
    if (D_800A30EC == 0) {
        COPY33(work + 0x84, rec->code);
    }
    var_9 = rec->ctr;
    rec->ctr = var_9 + 1;
    slot = &rec->slot[(var_9 + 1) & 3];
    if (slot->frame == frame_index) {
        COPY33(out, slot->pose);
        rec->ctr -= 1;
        return;
    }
    prev = &rec->slot[var_9 & 3];
    if (prev->frame == frame_index) {
        COPY33(out, prev->pose);
        rec->ctr -= 1;
        return;
    }
    if (slot->frame != frame_index - 1) {
        if (prev->frame == frame_index - 1) {
            slot = prev;
        } else {
            off = ((tbl[group_index * 3] << 16) |
                   (tbl[group_index * 3 + 1] << 8) | tbl[group_index * 3 + 2]) +
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
    if (index_in_group >= 2) {
        COPY33(work + 0x42, slot->rate);
    }
    idx2 = index_in_group - 1;
decode:
    for (; idx2 < index_in_group; idx2++) {
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
        for (channel_index = 0; channel_index < 63; channel_index++) {
            /* Ruling 11: four values -- the value's decoded delta, case 1's
             * magnitude, case 2's zero flag and case 3's magnitude. */
            s16 temp;

            code = work[channel_index + 0x87];
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
                work[channel_index + 0x45] = temp;
                work[channel_index + 3] += temp;
            } else {
                work[channel_index + 3] += work[channel_index + 0x45] + temp;
                work[channel_index + 0x45] += temp;
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
    if (index_in_group < 7) {
        slot->frame = frame_index;
        COPY33(slot->pose, work);
        COPY33(slot->rate, work + 0x42);
        slot->ptr = ptr;
        slot->bits = bits;
        slot->cur = cur;
    }
}

void font_PrintU16Triples(u16 *triple_cursor) {
    s32 i;
    u16 *p;
    i = 0;
    p = triple_cursor + 2;
    do {
        i++;
        font_Printf((u8 *)D_800100A4, triple_cursor[0], p[-1], p[0]);
        p += 3;
        triple_cursor += 3;
    } while (i < 0x16);
}

s32 math_SignExt12Div(s32 value_bits, s32 divisor) {
    s32 v = value_bits & 0xFFF;
    if (v >= 0x800) {
        v -= 0x1000;
    }
    return v / divisor;
}

void camera_CalcEye(Rec44 *view_record, s32 *eye_out) {
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
    RotMatrixX(-view_record->camera_angles.vx, &m);
    RotMatrixY(-view_record->camera_angles.vy, &m);
    RotMatrixZ(-view_record->camera_angles.vz, &m);
    eye_out[0] = view_record->focus_point.x -
                 ((s32)(m.m[0][2] * view_record->w18) >> 12);
    eye_out[1] = view_record->focus_point.y -
                 ((s32)(m.m[1][2] * view_record->w18) >> 12);
    eye_out[2] = view_record->focus_point.z -
                 ((s32)(m.m[2][2] * view_record->w18) >> 12);
}

s32 math_FloorDiv2000(s32 dividend) {
    if (dividend < 0) {
        return -((0x7CF - dividend) / 2000);
    }
    return dividend / 2000;
}

void math_StoreFloorDiv2000XZOfPointStepTowards(
    s16 *cell_out, s32 *toward_point, s32 *start_point) {
    s32 dx;
    s32 dz;
    u32 dist_sq;
    u32 distance;
    s32 sp_tmp;
    dx = toward_point[0] - start_point[0];
    dz = toward_point[2] - start_point[2];
    while (
        (((u32)(dx + 0x4000)) > 0x8000U) || (((u32)(dz + 0x4000)) > 0x8000U)) {
        dx = dx / 2;
        dz = dz / 2;
    }
    dist_sq = (dx * dx) + (dz * dz);
    if (dist_sq < 0x400U) {
        distance = ((u32)((u8)(g_sqrt_table_u8[dist_sq]))) >> 3;
    } else {
        u32 shift_a;
        u32 shift_b;
        gte_Lzc(dist_sq, &sp_tmp);
        {
            /* Ruling 4 split; one expression: score 3. */
            s32 lzc_even = -2;
            lzc_even &= sp_tmp;
            shift_a = 0x16 - lzc_even;
        }
        shift_b = shift_a >> 1;
        distance = (((u32)((u8)(g_sqrt_table_u8[dist_sq >> shift_a]))) << 16) >>
                   (0x13 - shift_b);
    }
    cell_out[0] =
        (s16)math_FloorDiv2000(start_point[0] + ((dx << 10) / ((s32)distance)));
    cell_out[2] =
        (s16)math_FloorDiv2000(start_point[2] + ((dz << 10) / ((s32)distance)));
}

void camera_UpdateTwoFighterView(
    Vec3i32 *arg0, Vec3i32 *arg1, Unk80101EC8Record *arg2,
    Unk80101EC8Record *arg3);
extern u8 g_camera_eye_blocked_flags[];
extern s32 g_camera_pitch_search_steps[];
typedef VECTOR CamVec;

/* Scratchpad work area (0x1F800000) used by camera_UpdateTwoFighterView: the
 * target yaw and vz angle that g_camera_default_view_record's camera_angles.vy
 * / vz ease toward, the camera focus, the eye position camera_CalcEye computes,
 * a fighter record's collision_pos less 0xC8 in vy (the collision segment's
 * start), and the hit position, surface normal and work area
 * collision_TestSegmentWithWork is given (its 3rd/4th/5th arguments; its other
 * callers pass a VECTOR hit and an s16[4] normal). */
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

/* Two-fighter camera for g_camera_default_view_record (first_camera_point/
 * second_camera_point = the fighters' positions, first_fighter/second_fighter =
 * the fighter records; caller fight_Update).
 * Eases the focus toward the fighters' midpoint (or first_camera_point's
 * position when g_fight_view_option is set); turns their separation (byte-LUT
 * square root, GTE leading-zero count for large inputs) into the value
 * g_camera_default_view_record.w18 eases 1/12 toward (0xBB8 in some
 * first_fighter->move_kind states), clamps w18, and sets a hysteresis flag for
 * func_8003F1E4; eases camera_angles.vy / vz toward the facing yaw and zero;
 * then per fighter bisects camera_angles.vx (rot_x) so the segment from its
 * collision_pos less 0xC8 in vy to the eye passes
 * collision_TestSegmentWithWork's camera-collision test (step in
 * g_camera_pitch_search_steps[], blocked flag in g_camera_eye_blocked_flags[]),
 * and eases camera_angles.vx toward the larger result, clamped to 0x80..0x1C0.
 * GTE: gte_Lzc (gte_ldlzc, gte_nop x2, gte_stlzc). */
void camera_UpdateTwoFighterView(
    Vec3i32 *first_camera_point, Vec3i32 *second_camera_point,
    Unk80101EC8Record *first_fighter, Unk80101EC8Record *second_fighter) {
    s32 lzc_out;
    CamScratch *scr;
    Rec44 *view_record;
    s32 dx, dy, dz;
    s32 x, y, z;
    s32 shift;
    u32 dist_sq;
    u32 dist;
    u32 q;
    s32 target_distance;
    s32 first_fighter_pitch, second_fighter_pitch;
    s32 start_pitch;
    s32 p;
    /* work holds three values, all camera_angles.vx (rot_x) quantities: the
     * per-fighter bisection angle (loop), the target angle
     * max(first_fighter_pitch, second_fighter_pitch) clamped to 0x80..0x1C0,
     * and the final eased step (Ruling 11, ordinary-c-judge-decidable). */
    s32 work;
    s32 i, j;

    scr = (CamScratch *)0x1F800000;
    view_record = &g_camera_default_view_record;
    view_record->h30[0][0] = 0x64;
    view_record->h30[0][1] = 0;
    view_record->h30[0][2] = 0x64;
    view_record->h30[1][0] = 0x64;
    view_record->h30[1][1] = 0;
    view_record->h30[1][2] = 0x64;
    dx = second_camera_point->x - first_camera_point->x;
    dy = second_camera_point->y - first_camera_point->y;
    dz = second_camera_point->z - first_camera_point->z;
    if (g_fight_view_option == 0) {
        scr->focus.vx = (first_camera_point->x + second_camera_point->x) / 2;
        scr->focus.vy = (first_camera_point->y + second_camera_point->y) / 2;
        scr->focus.vz = (first_camera_point->z + second_camera_point->z) / 2;
    } else {
        scr->focus = *(CamVec *)first_camera_point;
    }
    view_record->focus_point.x +=
        (scr->focus.vx - view_record->focus_point.x) / 4;
    view_record->focus_point.y +=
        (scr->focus.vy - view_record->focus_point.y) / 4;
    view_record->focus_point.z +=
        (scr->focus.vz - view_record->focus_point.z) / 4;

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
    if (first_fighter->move_kind == 0x13 || first_fighter->move_kind == 0x1B ||
        first_fighter->move_kind == 0x30 || second_fighter->move_kind == 0x13 ||
        second_fighter->move_kind == 0x1B ||
        second_fighter->move_kind == 0x30) {
        q += 0x1000;
    }
    target_distance = ((dist + q) << 7) / 100;
    if (dy < 0) {
        dy = -dy;
    }
    target_distance += dy;
    if (first_fighter->move_kind == 0xF || first_fighter->move_kind == 0x1C ||
        first_fighter->move_kind == 0x1D || first_fighter->move_kind == 0x1E ||
        first_fighter->move_kind == 0x1F || first_fighter->move_kind == 0x20 ||
        first_fighter->move_kind == 0x21) {
        target_distance = 0xBB8;
    }
    view_record->w18 += (target_distance - view_record->w18) / 12;
    if (!(first_fighter->move_kind == 0xF || first_fighter->move_kind == 0x1C ||
          first_fighter->move_kind == 0x1D ||
          first_fighter->move_kind == 0x1E ||
          first_fighter->move_kind == 0x1F ||
          first_fighter->move_kind == 0x20 ||
          first_fighter->move_kind == 0x21) &&
        view_record->w18 < 0x1770) {
        view_record->w18 = 0x1770;
    }
    if (view_record->w18 > 100000) {
        view_record->w18 = 100000;
    }
    if (view_record->b1E) {
        view_record->b1E = view_record->w18 >= 0x558D;
    } else {
        view_record->b1E = view_record->w18 >= 0x55F1;
    }
    func_8003F1E4(view_record->b1E);

    if (first_fighter->move_kind == 0x11) {
        scr->yaw = view_record->camera_angles.vy;
    } else {
        scr->yaw = (0x400 - ratan2(dx, dz)) & 0xFFF;
    }
    scr->scr04 = 0;
    view_record->camera_angles.vy +=
        math_SignExt12Div(scr->yaw - view_record->camera_angles.vy, 8);
    view_record->camera_angles.vz +=
        math_SignExt12Div(scr->scr04 - view_record->camera_angles.vz, 8);
    start_pitch = view_record->camera_angles.vx;

    for (p = 0; p < 2; p++) {
        /* The far end of either arm's bisection (above work when the eye is
         * blocked, below it when clear); a separate hi / lo per arm: score
         * 29. */
        s32 bound;

        work = start_pitch;
        view_record->camera_angles.vx = work;
        camera_CalcEye(view_record, &scr->eye.vx);
        if (p != 0) {
            scr->scr28 = second_fighter->collision_pos;
        } else {
            scr->scr28 = first_fighter->collision_pos;
        }
        scr->scr28.vy -= 0xC8;
        if (collision_TestSegmentWithWork(
                (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit, scr->nrm,
                (s32)&scr->unk60) &&
            scr->nrm[1] < -0x320) {
            math_StoreFloorDiv2000XZOfPointStepTowards(
                view_record->h30[p], (s32 *)&scr->eye, scr->hit);
            if (g_camera_eye_blocked_flags[p]) {
                g_camera_pitch_search_steps[p] += 0x20;
            } else {
                g_camera_pitch_search_steps[p] = 0x10;
                work--;
            }
            if (g_camera_pitch_search_steps[p] < 0x10) {
                g_camera_pitch_search_steps[p] = 0x10;
            } else if (g_camera_pitch_search_steps[p] > 0x200) {
                g_camera_pitch_search_steps[p] = 0x200;
            }
            bound = work + g_camera_pitch_search_steps[p] / 8;
            for (i = 0; i < 2; i++) {
                s32 d = (work - bound) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                view_record->camera_angles.vx = bound + d / 2;
                camera_CalcEye(view_record, &scr->eye.vx);
                if (collision_TestSegmentWithWork(
                        (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit,
                        scr->nrm, (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    work = view_record->camera_angles.vx;
                } else {
                    bound = view_record->camera_angles.vx;
                }
            }
            work = bound;
            g_camera_eye_blocked_flags[p] = 1;
        } else {
            if (g_camera_eye_blocked_flags[p]) {
                g_camera_pitch_search_steps[p] = 0x10;
                work++;
            } else {
                g_camera_pitch_search_steps[p] += 0x10;
            }
            if (g_camera_pitch_search_steps[p] < 0x10) {
                g_camera_pitch_search_steps[p] = 0x10;
            } else if (g_camera_pitch_search_steps[p] > 0x100) {
                g_camera_pitch_search_steps[p] = 0x100;
            }
            bound = work - g_camera_pitch_search_steps[p] / 8;
            for (j = 0; j < 2; j++) {
                s32 d = (work - bound) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                view_record->camera_angles.vx = bound + d / 2;
                camera_CalcEye(view_record, &scr->eye.vx);
                if (collision_TestSegmentWithWork(
                        (s32 *)&scr->scr28, (s32 *)&scr->eye, scr->hit,
                        scr->nrm, (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    math_StoreFloorDiv2000XZOfPointStepTowards(
                        view_record->h30[p], (s32 *)&scr->eye, scr->hit);
                    bound = view_record->camera_angles.vx;
                } else {
                    work = view_record->camera_angles.vx;
                }
            }
            g_camera_eye_blocked_flags[p] = 0;
        }
        if (p != 0) {
            second_fighter_pitch = work;
        } else {
            first_fighter_pitch = work;
        }
    }
    work = (first_fighter_pitch < second_fighter_pitch)
               ? second_fighter_pitch
               : first_fighter_pitch;
    if (work < 0x80) {
        work = 0x80;
    }
    if (work > 0x1C0) {
        work = 0x1C0;
    }
    view_record->camera_angles.vx = start_pitch;
    work = math_SignExt12Div(work - start_pitch, 8);
    view_record->camera_angles.vx += work;
}

void camera_UpdatePadOffset(s32 *held_bits) {
    g_camera_view_offset.unk_00.x = 0;
    g_camera_view_offset.unk_00.y = 0;
    g_camera_view_offset.unk_00.z = 0;
    g_camera_view_offset.unk_10.vx = 0;
    g_camera_view_offset.unk_10.vy = 0;
    g_camera_view_offset.unk_10.vz = 0;
    g_camera_view_offset.w18 = 0;
    if (g_fight_tracked_view_setting != 0 && g_main_loop_handler_index == 1) {
        if (*held_bits & 1) {
            D_800A37E0 = 1;
            if (*held_bits & 8) {
                g_camera_distance_offset = g_camera_distance_offset + 0x4CC;
            }
            if (*held_bits & 2) {
                g_camera_distance_offset = g_camera_distance_offset - 0x4CC;
            }
            if (g_camera_distance_offset < -0x1C00) {
                g_camera_distance_offset = -0x1C00;
            }
            if (g_camera_distance_offset >= 0x7401) {
                g_camera_distance_offset = 0x7400;
            }
            *held_bits = *held_bits & ~0xB;
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
    *held_bits = *held_bits & (s32)0xFFFEFFFE;
}

void camera_InitTwoFighterView(
    Unk80101EC8Record *first_fighter, Unk80101EC8Record *second_fighter) {
    g_camera_fight_view_mode = 0;
    g_camera_default_view_record.h30[0][0] = 0x64;
    g_camera_default_view_record.h30[0][1] = 0;
    g_camera_default_view_record.h30[0][2] = 0x64;
    g_camera_default_view_record.h30[1][0] = 0x64;
    g_camera_default_view_record.h30[1][1] = 0;
    g_camera_default_view_record.h30[1][2] = 0x64;
    func_8003F1E4(0);
    g_camera_default_view_record.focus_point.x =
        (first_fighter->world_pos.x + second_fighter->world_pos.x) / 2;
    g_camera_default_view_record.focus_point.y =
        (first_fighter->world_pos.y + second_fighter->world_pos.y) / 2;
    g_camera_default_view_record.focus_point.z =
        (first_fighter->world_pos.z + second_fighter->world_pos.z) / 2;
    g_camera_default_view_record.camera_angles.vx = 0;
    g_camera_default_view_record.camera_angles.vy =
        0x400 -
        ratan2(second_fighter->world_pos.x - first_fighter->world_pos.x,
               second_fighter->world_pos.z - first_fighter->world_pos.z);
    g_camera_default_view_record.camera_angles.vz = 0;
    g_camera_default_view_record.w18 = 0x1388;
    g_camera_default_view_record.b1E = 0;
}

void camera_InitTrackedFighterFocus(
    Unk80101EC8Record *a0, Unk80101EC8Record *a1) {
    g_camera_fight_view_mode = 0;
    g_camera_alternate_view_record.h30[0][0] = 0x64;
    g_camera_alternate_view_record.h30[0][1] = 0;
    g_camera_alternate_view_record.h30[0][2] = 0x64;
    g_camera_alternate_view_record.h30[1][0] = 0x64;
    g_camera_alternate_view_record.h30[1][1] = 0;
    g_camera_alternate_view_record.h30[1][2] = 0x64;
    func_8003F1E4(0);
    if (g_fight_tracked_fighter_index != 0) {
        a0 = a1;
    }
    g_camera_alternate_view_record.focus_point.x = a0->bone_hit_point_0.x;
    g_camera_alternate_view_record.focus_point.z = a0->bone_hit_point_0.z;
    {
        s32 v = a0->bone_hit_point_0.y;
        g_camera_alternate_view_record.b40 = 0;
        g_camera_alternate_view_record.focus_point.y = v;
    }
}

void camera_UpdateTrackedFighterView(Unk80101EC8Record *tracked_fighter) {
    /* Pointer read-modify-write on a global (pointer-alias-fake-exception):
     * the tracked camera target's height and rotation are read, eased toward
     * the fighter and stored back through cam, and h1C is read, decayed by
     * 3/4 and stored back; through the symbol each access re-forms the
     * address (score 34). */
    Rec44 *cam = &g_camera_alternate_view_record;
    s32 focus_y;
    s32 val;
    s32 var_3;

    func_8003F1E4(0);

    val = (tracked_fighter->ground_probe_points[0].y +
           tracked_fighter->ground_probe_points[1].y) /
              2 -
          tracked_fighter->bone_hit_point_0.y;
    var_3 = val >= 0x391;

    if (tracked_fighter->move_kind == 0x2A) {
        val = 0x200;
    } else {
        cam->focus_point.x = tracked_fighter->bone_hit_point_0.x;
        cam->focus_point.z = tracked_fighter->bone_hit_point_0.z;
        val = tracked_fighter->bone_hit_point_0.y;

        if (!var_3) {
            s32 v = -(tracked_fighter->size_scale * 950);
            if (v < 0) {
                v += 0xFFF;
            }
            val += v >> 12;
        }

        {
            s32 diff = val - cam->focus_point.y;
            if (diff < 0) {
                diff += 3;
            }
            focus_y = cam->focus_point.y + (diff >> 2);
            cam->focus_point.y = focus_y;
        }

        if (tracked_fighter->move_kind == 0x2A) {
            val = 0x200;
        } else {
            val = (-tracked_fighter->opponent_bearing - cam->camera_angles.vy) &
                  0xFFF;

            if (val >= 0x800) {
                val = 0x1000 - val;
            }
            if (val >= 0x400) {
                val = 0x400;
            }

            val = (ratan2(tracked_fighter->other->world_pos.y - focus_y,
                          g_fighter_horizontal_separation) *
                   (0x400 - val)) >>
                  10;
        }
    }

    {
        s16 old = cam->camera_angles.vx;
        s32 diff = val - old;
        if (diff < 0) {
            diff += 7;
        }
        cam->camera_angles.vx = old + (diff >> 3);
    }
    cam->camera_angles.vz = 0;

    {
        s16 counter;
        val = -tracked_fighter->orientation.vy;
        counter = g_camera_turn_easing_countdown;

        if (counter != 0) {
            s16 old12 = cam->camera_angles.vy;
            s32 diff = val - old12;
            if (diff < 0) {
                diff += 3;
            }
            {
                counter--;
                cam->camera_angles.vy = old12 + (diff >> 2);
                g_camera_turn_easing_countdown = counter;
            }

            {
                s32 var_12 = cam->h1C * 3;
                if (var_12 < 0) {
                    var_12 += 3;
                }
                cam->h1C = var_12 >> 2;
            }
        } else {
            cam->camera_angles.vy = val;
            cam->h1C = 0;
        }
    }
    cam->w18 = 0;
}

void camera_StartTurnEasingOnLargeHeadingChange(
    s32 fighter_index, s32 heading_change) {
    if (g_fight_tracked_view_setting == 0) {
        return;
    }
    if (g_fight_tracked_fighter_index != fighter_index) {
        return;
    }
    heading_change &= 0xFFF;
    if (heading_change >= 0x800) {
        heading_change = 0x1000 - heading_change;
    }
    if (heading_change >= 0x401) {
        g_camera_turn_easing_countdown = 0x19;
        g_camera_alternate_view_record.h1C = 0x800;
    }
}

void func_8001B6F4(void) {
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    g_camera_fight_view_mode = 1;
    g_camera_default_view_record.b1F = 0;
    g_camera_alternate_view_record.b1F = 0;
    func_8003F1E4(0);
}

/* Q65: this file's statics (.sbss), in address order. */
static u16 D_800A3310;

void camera_UpdateReplayView(
    Rec44 *dst, Rec1C *a, Rec1C *b, s32 frac_s1, s32 frac, s32 val) {
    s32 inv_frac = 0x1000 - frac;
    s32 inv_s1 = 0x1000 - frac_s1;
    Unk80101EC8Record *base = &D_80101EC8[g_fight_followup_fighter_index];
    s32 dx;
    s32 dy;
    s32 dz;
    /* cur / t are the current and target value of each eased step (x, y, z,
     * then the pitch target and the w18 step), reused component to component
     * the way SOTN's white-dragon segment update reuses var_s2 for posX
     * then posY (Q51) */
    /* SOTN: src/st/rare/e_white_dragon.c:78 @db41b28 */
    s32 cur;
    s32 var_8;
    s32 t;
    s32 distance_delta;
    if (dst->b1F == 0) {
        dst->b1F = 1;
        dst->focus_point.x = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
        dst->focus_point.y =
            (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
        D_800A3310 = 0;
        dst->focus_point.z = ((frac * a->h8) + (inv_frac * b->h8)) >> 12;
        dst->camera_angles.vy = val;
        dst->camera_angles.vx = 0x80;
        dst->camera_angles.vz = 0;
        dst->w18 = ((frac_s1 * 0x9C4) + (inv_s1 * 0x2710)) >> 12;
        return;
    }
    if ((base->ground_probe_points[0].y + base->ground_probe_points[1].y) / 2 -
            base->bone_hit_point_0.y <
        0xC8) {
        D_800A3310 += 1;
    }
    var_8 = ((s16)D_800A3310) >= 0xB;
    cur = dst->focus_point.x;
    t = ((frac * (a->h4)) + (inv_frac * (b->h4))) >> 12;
    dx = t - cur;
    if (dx < 0) {
        dx += 0xF;
    }
    dst->focus_point.x = cur + (dx >> 4);
    cur = dst->focus_point.y;
    t = (((frac * (a->h6)) + (inv_frac * (b->h6))) >> 12) - 0x12C;
    dy = t - cur;
    if (dy < 0) {
        dy += 0xF;
    }
    dst->focus_point.y = cur + (dy >> 4);
    cur = dst->focus_point.z;
    t = ((frac * (a->h8)) + (inv_frac * (b->h8))) >> 12;
    dz = t - cur;
    if (dz < 0) {
        dz += 0xF;
    }
    dst->focus_point.z = cur + (dz >> 4);
    if (var_8) {
        t = ((frac_s1 * 0x180) >> 12) + 0x80;
    } else {
        t = 0x80 - ((frac_s1 << 8) >> 12);
    }
    dst->camera_angles.vx = dst->camera_angles.vx +
                            math_SignExt12Div(t - dst->camera_angles.vx, 0x10);
    dst->camera_angles.vy +=
        math_SignExt12Div(val - dst->camera_angles.vy, 0x10);
    dst->camera_angles.vz = 0;
    if (var_8) {
        t = ((frac_s1 * 0x7D0) + (inv_s1 * 0x2EE0)) >> 12;
    } else {
        t = ((frac_s1 * 0x1F4) + (inv_s1 * 0x2EE0)) >> 12;
    }
    cur = dst->w18;
    distance_delta = t - cur;
    if (distance_delta < 0) {
        distance_delta += 0xF;
    }
    dst->w18 = cur + (distance_delta >> 4);
    dst->h30[0][0] = 0x64;
    dst->h30[0][1] = 0;
    dst->h30[0][2] = 0x64;
    dst->h30[1][0] = 0x64;
    dst->h30[1][1] = 0;
    dst->h30[1][2] = 0x64;
}

void camera_UpdateReplayViewWithSineYawOffset(
    Rec1C *followup_frame, Rec1C *other_frame, s32 playback_fraction) {
    s32 bearing_to_other;
    s32 var_s3;
    s32 var_v1;
    s32 var_v0;

    if (g_fighter_horizontal_separation < 0x2711) {
        var_s3 = (playback_fraction / 2) + 0x800;
    } else {
        var_s3 = 0x1000;
    }
    bearing_to_other = ratan2(other_frame->h4 - followup_frame->h4,
                              other_frame->h8 - followup_frame->h8);
    var_v1 = playback_fraction;
    if (playback_fraction < 0) {
        var_v1 = playback_fraction + 3;
    }
    var_v0 = Judge[(var_v1 >> 2) & 0xFFF] * 3;
    if (var_v0 < 0) {
        var_v0 += 3;
    }
    camera_UpdateReplayView(
        &g_camera_default_view_record, followup_frame, other_frame,
        playback_fraction, var_s3, (0x500 - bearing_to_other) - (var_v0 >> 2));
}

void camera_UpdateReplayViewWithConstantYawOffset(
    Rec1C *followup_frame, Rec1C *other_frame, s32 playback_fraction) {
    s32 temp_s0;
    temp_s0 = (g_fighter_horizontal_separation < 0x2711) << 0xB;
    camera_UpdateReplayView(
        &g_camera_alternate_view_record, followup_frame, other_frame,
        playback_fraction, temp_s0,
        -0x200 - ratan2(other_frame->h4 - followup_frame->h4,
                        other_frame->h8 - followup_frame->h8));
}

void camera_SetFighterFocusView(Unk80101EC8Record *fighter_record, s32 yaw) {
    func_8003F1E4(0);
    g_camera_default_view_record.focus_point =
        fighter_record->bone_hit_points_4_5_midpoint;
    g_camera_default_view_record.camera_angles.vx = 0x120;
    g_camera_default_view_record.camera_angles.vy = yaw;
    g_camera_default_view_record.camera_angles.vz = 0;
    g_camera_default_view_record.w18 = 0x1162;
}

void camera_SetInterpolatedFighterView(Unk80101EC8Record *arg0, s32 arg1) {
    s32 diff = 0x1000 - arg1;

    func_8003F1E4(0);

    /* FAKE: block copy of collision_pos's x / y / z (three lw, then three sw
       through one base); the three member assignments interleave the loads and
       stores (score 27). */
    g_camera_default_view_record.focus_point = *(Vec3i32 *)&arg0->collision_pos;

    g_camera_default_view_record.focus_point.y -= 0x44C;

    g_camera_default_view_record.camera_angles.vx = 0x100 - (arg1 * 288) / 4096;

    {
        /* FAKE: div4 taken first; written at its use the multiply chain is
         * rebuilt after the shift (score 17) (Q129). */
        s32 div4 = arg1 / 4;
        u16 rot_y = arg0->orientation.vy;
        s32 val;
        g_camera_default_view_record.w18 = (arg1 * 3000 + diff * 8000) >> 12;
        val = 0xB00 - div4;
        g_camera_default_view_record.camera_angles.vz = 0;
        g_camera_default_view_record.camera_angles.vy = val - rot_y;
    }
}

void pad_ClearStateBits(PadState *pad_state) {
    pad_state->held = 0;
    pad_state->pressed = 0;
    pad_state->released = 0;
    pad_state->unheld = -1;
}

void fighter_BuildCpuOrMappedPadState(s32 arg0, PadState *arg1);

void fighter_BuildCpuOrMappedPadState(s32 fighter_index, PadState *pad_out) {
    s32 pad_words[4];
    s32 i;
    /* One local holds two values (Ruling 11, ordinary-c-judge-decidable):
     * (1) fighter_index * 16, the bit offset of this player's half of the pad
     * words; (2) 0 or fighter_index * 4, the bit offset of this player's config
     * nibble.
     */
    s32 temp;
    s32 mapped_buttons;

    D_80101EC8[fighter_index == 0].unk_34E =
        g_fight_mode_selector == 2 && D_800A389A == 0 && fighter_index == 0 &&
        ((g_pad_state.pressed >> 8) & 1);
    if (D_80101EC8[fighter_index].cpu_controlled != 0) {
        fighter_BuildCpuPad(fighter_index, pad_out);
        return;
    }
    *pad_out = g_pad_state;
    temp = fighter_index * 16;
    pad_words[0] = (g_pad_state.held >> temp) & 0xFFFF;
    pad_words[1] = (g_pad_state.pressed >> temp) & 0xFFFF;
    pad_words[2] = (g_pad_state.released >> temp) & 0xFFFF;
    pad_words[3] = (g_pad_state.unheld >> temp) & 0xFFFF;
    if (g_fight_mode_selector == 6) {
        temp = 0;
    } else {
        temp = fighter_index * 4;
    }
    for (i = 0; i < 4; i++) {
        if (g_fight_mode_selector == 6 && fighter_index != g_comb_local_side) {
            mapped_buttons = pad_words[i] & ~0xF0;
            {
                u8 r = g_pad_remote_button_mapping_20;
                if ((r & 1) && (pad_words[i] & 0x20) ||
                    (r & 2) && (pad_words[i] & 0x10) ||
                    (r & 4) && (pad_words[i] & 0x40) ||
                    (r & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x20;
                }
            }
            {
                u8 g = g_pad_remote_button_mapping_40;
                if ((g & 1) && (pad_words[i] & 0x20) ||
                    (g & 2) && (pad_words[i] & 0x10) ||
                    (g & 4) && (pad_words[i] & 0x40) ||
                    (g & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x40;
                }
            }
            {
                u8 b = g_pad_remote_button_mapping_80;
                if ((b & 1) && (pad_words[i] & 0x20) ||
                    (b & 2) && (pad_words[i] & 0x10) ||
                    (b & 4) && (pad_words[i] & 0x40) ||
                    (b & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x80;
                }
            }
        } else {
            mapped_buttons = pad_words[i] & ~0xF0;
            {
                u8 r = g_saved_record.frec20[0] >> temp;
                if ((r & 1) && (pad_words[i] & 0x20) ||
                    (r & 2) && (pad_words[i] & 0x10) ||
                    (r & 4) && (pad_words[i] & 0x40) ||
                    (r & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x20;
                }
            }
            {
                u8 g = g_saved_record.frec20[1] >> temp;
                if ((g & 1) && (pad_words[i] & 0x20) ||
                    (g & 2) && (pad_words[i] & 0x10) ||
                    (g & 4) && (pad_words[i] & 0x40) ||
                    (g & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x40;
                }
            }
            {
                u8 b = g_saved_record.frec20[2] >> temp;
                if ((b & 1) && (pad_words[i] & 0x20) ||
                    (b & 2) && (pad_words[i] & 0x10) ||
                    (b & 4) && (pad_words[i] & 0x40) ||
                    (b & 8) && (pad_words[i] & 0x80)) {
                    mapped_buttons |= 0x80;
                }
            }
        }
        if (mapped_buttons & 0x200) {
            mapped_buttons = (mapped_buttons & ~0x200) | 0x8;
        }
        if (mapped_buttons & 0x400) {
            mapped_buttons = (mapped_buttons & ~0x400) | 0x20;
        }
        pad_words[i] = mapped_buttons;
    }
    pad_out->held = pad_words[0];
    pad_out->pressed = pad_words[1];
    pad_out->released = pad_words[2];
    pad_out->unheld = pad_words[3];
    if (g_fight_mode_selector == 5 &&
        (g_fight_time_up_phase_timer != 0 || g_fight_reinit_phase_timer != 0 ||
         D_800A37E1 != 0 || g_fight_mark_tally_phase_timer != 0 ||
         g_fight_point_award_phase_timer != 0 ||
         g_fight_match_result_timer != 0)) {
        pad_ClearStateBits(pad_out);
    }
    if (fighter_index == 1 && g_fight_mode_selector != 6) {
        pad_out->held =
            (pad_out->held & 0xFFF) | ((pad_out->held & 0x8000) >> 2) |
            ((pad_out->held & 0x2000) << 2) | ((pad_out->held & 0x4000) >> 2) |
            ((pad_out->held & 0x1000) << 2);
        pad_out->pressed =
            (pad_out->pressed & 0xFFF) | ((pad_out->pressed & 0x8000) >> 2) |
            ((pad_out->pressed & 0x2000) << 2) |
            ((pad_out->pressed & 0x4000) >> 2) |
            ((pad_out->pressed & 0x1000) << 2);
        pad_out->released =
            (pad_out->released & 0xFFF) | ((pad_out->released & 0x8000) >> 2) |
            ((pad_out->released & 0x2000) << 2) |
            ((pad_out->released & 0x4000) >> 2) |
            ((pad_out->released & 0x1000) << 2);
        pad_out->unheld =
            (pad_out->unheld & 0xFFF) | ((pad_out->unheld & 0x8000) >> 2) |
            ((pad_out->unheld & 0x2000) << 2) |
            ((pad_out->unheld & 0x4000) >> 2) |
            ((pad_out->unheld & 0x1000) << 2);
    }
}

void fight_InitSettings(void) {
    g_fight_config.cpu_base_levels[1] = 0x800;
    g_fight_config.cpu_base_levels[0] = 0x800;
    g_fight_config.unk_4[0] = 1;
    g_fight_config.unk_4[1] = 0x10;
    g_fight_config.pending_stage_index = 0xC;
    g_fight_config.unk_4[2] = 0;
    g_fight_config.unk_4[3] = 0;
    g_fight_config.unk_4[5] = 0;
    g_fight_config.unk_4[4] = 0;
    g_fight_config.pending_mode_selector = 4;
    g_fight_config.pending_view_setting = 0;
    g_fight_config.unk_F = 0;
}

void fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11(void) {
    u16 v = D_80101EC8[0].move_kind;
    if (v == 0x32 || v == 0x11) {
        motion_ResetVariantIndex(0);
        {
            MoveScript *default_script = motion_GetDefaultVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            fighter_StartMoveScript(0, default_script, 0);
        }
    }
}

void func_8001C51C(void) {
    MoveScript *default_script;
    fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11();
    if (g_fight_mode_selector == 3 &&
        g_fight_start_position_pair_changed != 0) {
        fighter_Init(
            1, (s8)g_fight_config.unk_4[5], (s8)g_fight_config.unk_4[1],
            (s8)g_fight_config.unk_4[3], 0);
    } else {
        fighter_Init(
            1, (s8)g_fight_config.unk_4[5], (s8)g_fight_config.unk_4[1],
            (s8)g_fight_config.unk_4[3], 1);
    }
    fighter_ConfigureCpuControl();
    motion_ResetVariantIndex(1);
    default_script = motion_GetDefaultVariantScript(1);
    D_80101EC8[1].move_is_common = 0;
    fighter_StartMoveScript(1, default_script, 0);
    g_fight_defeat_delay_frames = 0;
    g_fight_followup_fighter_index = -1;
    object_FreeAttachedObjects();
    object_ClearFixedKindRange();
    camera_InitTwoFighterView(&D_80101EC8[0], &D_80101EC8[1]);
    replay_Reset();
    fighter_AssignMotionSlotAndScanDataIdNibbles(1);
}

/* Initialise record 0 of D_80101EC8 (Unk80101EC8Record). */
void func_8001C624(void) {
    Unk80101EC8Record *fighter_record = D_80101EC8;
    VECTOR local;
    s32 x, y, z;

    fight_GetStartPosition(
        0, &fighter_record->base_pos.x, (s32)g_fight_start_position_pair_index);
    fight_GetStartPosition(
        1, &local.vx, (s32)g_fight_start_position_pair_index);
    fighter_record->root_offset.x = 0;
    x = fighter_record->base_pos.x;
    y = fighter_record->base_pos.y;
    z = fighter_record->base_pos.z;
    fighter_record->root_offset.y = -0x384;
    fighter_record->root_offset.z = 0;
    fighter_record->world_pos.x = x;
    fighter_record->world_pos.y = y - 0x384;
    fighter_record->world_pos.z = z;
    fighter_record->collision_pos.vx = x;
    fighter_record->collision_pos.vy = y;
    fighter_record->collision_pos.vz = z;
    fighter_record->prev_collision_pos = fighter_record->collision_pos;
    fighter_record->prev_root_offset = fighter_record->root_offset;
    fighter_record->base_velocity.vx = 0;
    fighter_record->base_velocity.vy = 0;
    fighter_record->base_velocity.vz = 0;
    fighter_record->unk_24C = fighter_record->base_velocity;
    /* Player 1's start position is raised by 0x384 as player 0's is, but not
     * read again here (the target performs these stores). */
    setVector(&local, local.vx, local.vy - 0x384, local.vz);
    fighter_record->weapon_point_delta[0].vx = 0;
    fighter_record->weapon_point_delta[0].vy = 0;
    fighter_record->weapon_point_delta[0].vz = 0;
    fighter_record->weapon_point_delta[1].vx = 0;
    fighter_record->weapon_point_delta[1].vy = 0;
    fighter_record->weapon_point_delta[1].vz = 0;
    fighter_record->decaying_xz_velocity.vx = 0;
    fighter_record->decaying_xz_velocity.vy = 0;
    fighter_record->decaying_xz_velocity.vz = 0;
    fighter_record->unk_144 = 0;
    fighter_record->remaining_turn = 0;
    fighter_record->unk_150 = 0;
    fighter_record->use_root_offset_yaw = 0;
    fighter_record->forward_move_rate = 0;
    fighter_record->ground_y = fighter_record->collision_pos.vy;
    softbody_RequestSnap(0);
}

void fight_QueuePositionalSoundWithRandomAlternate(void) {
    s16 *s0 = &D_80101EC8[0].character_data_index;
    s32 sound_index;
    if (g_fighter_group_flags[*s0] != 0) {
        if (g_fight_stage_command_count == 1)
            return;
    }
    if (g_fight_mode_selector != 0)
        return;
    if (D_800A3712 != 0)
        return;
    sound_index = 0x56;
    if (g_fight_second_fighter_defeats_left !=
        g_fight_second_fighter_initial_defeats_left) {
        if (rand() & 1) {
            sound_index = 0x57;
        } else {
            sound_index = 0x58;
        }
    }
    /* FAKE: player 1's world_pos reached from s0 (player 0's
       character_data_index) so the call reuses s0's register;
       &D_80101EC8[1].world_pos.x rematerialises the address (score 11). */
    snd_QueuePositionalSound(sound_index, (s32 *)((u8 *)s0 + 0x536));
}

void fight_UpdateDefeatDelayAndModeProgress(void);

void fight_UpdateDefeatDelayAndModeProgress(void) {
    u8 prev;
    MoveScript *current_script;

    if (D_80101EC8[0].is_defeated != 0 || D_80101EC8[1].is_defeated != 0) {
        g_fight_defeat_delay_frames++;
    } else if (g_fight_mode_selector == 0 && D_800A385C != 0 &&
               D_80101EC8[0].collision_hit_class == 1) {
        func_8003B56C(3);
    }
    if (g_fight_defeat_delay_frames < 0x3D)
        goto end;

    switch (g_fight_mode_selector) {
    case 0:
        if (D_80101EC8[0].is_defeated != 0)
            break;
        if (g_fight_second_fighter_defeats_left != 0) {
            if (--g_fight_second_fighter_defeats_left == 0) {
                snd_StopAllVoices();
                if (D_800A37C6 != 0) {
                    D_800A37C6 = 0;
                    break;
                }
                if (D_800A3894 != 0) {
                    fight_ConfigureFighter1FromOperandsAndLoad(D_800A3894 + 6);
                    if (g_fight_saved_first_fighter_selection != 0xFF) {
                        func_8003B534(2);
                    } else {
                        func_8003B534(5);
                    }
                } else {
                    fight_RunCommandStream();
                }
            } else {
                func_8001C51C();
                fight_QueuePositionalSoundWithRandomAlternate();
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
        if (fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags() == 0)
            goto end;
        g_main_present_skip_frames = 1;
        gpu_ResetGraphMode1();
        slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
            1, g_fight_step_row_object_id, 0);
        prev = g_fight_start_position_pair_index;
        switch (g_fight_step_table_next_row) {
        case 0x1F:
            g_fight_start_position_pair_index = 1;
            resource_SelectRetainedSubblockOrApplyHeaderSections(1);
            break;
        case 0x33:
            g_fight_start_position_pair_index = 2;
            resource_SelectRetainedSubblockOrApplyHeaderSections(2);
            break;
        case 0x51:
            g_fight_start_position_pair_index = 3;
            resource_SelectRetainedSubblockOrApplyHeaderSections(3);
            break;
        }
        if (g_fight_start_position_pair_changed =
                prev != g_fight_start_position_pair_index) {
            func_8001C624();
            current_script = motion_GetCurrentVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            fighter_StartMoveScript(0, current_script, 0);
        }
        func_8001C51C();
        if (g_fight_step_row_column < 4) {
            gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
                g_fight_step_row_tint_red, g_fight_step_row_tint_green,
                g_fight_step_row_tint_blue);
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
        g_fight_reinit_reuses_assets = 1;
        g_fight_nearest_start_pair_index = fight_FindNearestStartPair(
            &D_80101EC8[g_fight_followup_fighter_index].world_pos);
        g_main_loop_handler_index = 0;
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
    g_main_loop_handler_index = 4;
end:
    if (D_800A37D2 >= 100) {
        D_800A37D2 = 99;
    }
    if (D_800A37D3 >= 100) {
        D_800A37D3 = 99;
    }
}

void fight_SplitFrameCounterIntoMinutesSecondsHundredths(
    Unk8001CD68Rec *time_fields_out) {
    s32 timer_frames = g_fight_timer_frames;

    if (timer_frames > 0x2BF1F) {
        time_fields_out->unk_0 = 99;
        time_fields_out->unk_2 = 59;
        time_fields_out->unk_3 = 99;
        return;
    }
    {
        s32 minutes = timer_frames / 1800;
        s32 seconds = timer_frames / 30 - minutes * 60;
        time_fields_out->unk_2 = seconds;
        {
            s32 hundredths = (g_fight_timer_frames % 30) * 100 / 30;
            time_fields_out->unk_0 = minutes;
            time_fields_out->unk_3 = hundredths;
        }
    }
}

/* P1/P2 round scores and tiebreakers, declared here as [2] arrays because
 * fight_UpdateModeDisplaysAndMatchProgress indexes them by player
 * (g_fight_result_subject_index); fight_RecordMatchResult's declarations in
 * src/main/17AFC.c give the same bytes as single u8s (per-file declarations:
 * Q21-Q25, no-new-park-categories aggregate-merge exception). No single
 * declaration compiles both files' accesses with an admissible spelling. Under
 * scalars, fight_UpdateModeDisplaysAndMatchProgress's player-indexed accesses
 * are not produced (conditional selects compile to separate direct accesses; a
 * pointer pun is refused). Under an aggregate, every measured counting spelling
 * of fight_RecordMatchResult (Q22/Q23 set-asides excluded) misses: constant
 * subscripts put element 0 behind a base register, and the index-variable and
 * regrouped-condition spellings miss its match-result stores or compares; the
 * dummy-index and pointer-alias spellings that match are refused/set aside
 * (Q22/Q23). */
extern u8 D_800A3898[2];
extern u8 D_800A38AA[2];

void fight_UpdateModeDisplaysAndMatchProgress(void) {
    Unk8001CD68Rec time_fields;

    if (g_fight_mode_selector == 1) {
        g_prim_buf_cursor +=
            gpu_DrawDecrementedSingleSideCounter(
                g_fight_opponent_order_cursor, (s32)g_prim_buf_cursor, 1) /
            4;
    } else if (g_fight_mode_selector == 3) {
        if (D_80101EC8[0].is_defeated == 0 &&
            (D_80101EC8[1].is_defeated == 0 ||
             g_fight_step_table_next_row != 100)) {
            g_fight_timer_frames++;
            if (g_fight_timer_frames > 0x2BF20) {
                g_fight_timer_frames = 0x2BF20;
            }
        }
        fight_SplitFrameCounterIntoMinutesSecondsHundredths(&time_fields);
        g_prim_buf_cursor +=
            gpu_DrawTimeFieldsAndDecimalCounter(
                &time_fields, g_fight_step_table_next_row,
                (s32)g_prim_buf_cursor, 1) /
            4;
    } else if ((g_fight_mode_selector == 2 && D_800A389A == 1) ||
               g_fight_mode_selector == 4) {
        g_prim_buf_cursor +=
            gpu_DrawTwoDigitCounterWithOptionalSecondSide(
                D_800A37D2, D_800A37D3, (s32)g_prim_buf_cursor, 1) /
            4;
    } else if (g_fight_mode_selector == 5) {
        /* temp holds two values (Ruling 11, ordinary-c-judge-decidable): the
         * announcement length in frames (0x50 after a draw, 0x64 otherwise),
         * then the match clock's frames left for the on-screen timer. */
        s32 temp;

        if (g_fight_time_up_phase_timer != 0) {
            if (g_fight_time_up_phase_timer == 0x2D) {
                snd_QueueSound(0xA3, 0x7F, 0x7F);
            }
            if (++g_fight_time_up_phase_timer == 0x50) {
                g_fight_time_up_phase_timer = 0;
                fight_RecordMatchResult();
                g_fight_match_result_timer = 1;
            }
        } else if (g_fight_reinit_phase_timer != 0) {
            if (++g_fight_reinit_phase_timer == 0x46) {
                snd_QueueSound(0x9D, 0x7F, 0x7F);
            } else if (g_fight_reinit_phase_timer == 0x82) {
                g_fight_reinit_phase_timer = 0;
                g_fight_reinit_reuses_assets = 1;
                g_main_loop_handler_index = 0;
            }
        } else if (D_800A37E1 != 0) {
            g_prim_buf_cursor += gpu_DrawSelectedStretchedSpritePair(
                                     2, (s32)g_prim_buf_cursor, 1) /
                                 4;
            if (++D_800A37E1 == 0x3C) {
                D_800A37E1 = 0;
                if (g_fight_result_subject_index != 2) {
                    snd_QueueSound(0xA0, 0x7F, 0x7F);
                    D_800A38AA[g_fight_result_subject_index]++;
                    g_fight_mark_tally_phase_timer = 1;
                } else {
                    g_fight_reinit_reuses_assets = 1;
                    g_main_loop_handler_index = 0;
                }
            }
        } else if (g_fight_mark_tally_phase_timer != 0) {
            if (++g_fight_mark_tally_phase_timer == 0x3C) {
                g_fight_mark_tally_phase_timer = 0;
                if (D_800A38AA[g_fight_result_subject_index] == 2) {
                    D_800A3898[g_fight_result_subject_index == 0]++;
                    D_800A38AA[g_fight_result_subject_index] = 0;
                    g_fight_point_award_phase_timer = 0x5A;
                } else {
                    g_fight_reinit_reuses_assets = 1;
                    g_main_loop_handler_index = 0;
                }
            }
        } else if (g_fight_point_award_phase_timer != 0) {
            g_prim_buf_cursor += gpu_DrawSelectedStretchedSpritePair(
                                     1, (s32)g_prim_buf_cursor, 1) /
                                 4;
            if (++g_fight_point_award_phase_timer == 0x1E) {
                snd_QueueSound(0xA1, 0x7F, 0x7F);
            }
            if (g_fight_point_award_phase_timer == 0x69) {
                snd_QueueSound(0xA2, 0x7F, 0x7F);
            }
            if (g_fight_point_award_phase_timer == 0x5A ||
                g_fight_point_award_phase_timer == 0xB4) {
                g_fight_point_award_phase_timer = 0;
                if (D_800A3898[g_fight_result_subject_index == 0] ==
                    g_fight_win_target) {
                    fight_RecordMatchResult();
                    g_fight_match_result_timer = 1;
                } else {
                    g_fight_reinit_reuses_assets = 1;
                    g_main_loop_handler_index = 0;
                }
            }
        } else if (g_fight_sound_cue_countdown != 0) {
            if (--g_fight_sound_cue_countdown == 0) {
                snd_QueueSound(0x9C, 0x7F, 0x7F);
            }
        } else if (g_fight_match_result_timer != 0) {
            if (g_fight_match_results[g_fight_match_cursor - 1] == 2) {
                if (g_fight_match_result_timer == 1) {
                    snd_QueueSound(0xA4, 0x7F, 0x7F);
                }
                temp = 0x50;
            } else {
                if (g_fight_match_result_timer == 1) {
                    snd_QueueSound(0xA6, 0x7F, 0x7F);
                }
                if (g_fight_match_result_timer == 0x14) {
                    snd_QueueSound(
                        g_fighter_group_flags
                                [D_80101EC8[g_fight_match_results
                                                [g_fight_match_cursor - 1]]
                                     .character_data_index]
                            ? 0xA8
                            : 0xA7,
                        0x7F, 0x7F);
                }
                temp = 0x64;
            }
            if (++g_fight_match_result_timer == temp) {
                g_fight_match_result_timer = 0;
                fight_AdvanceMatch();
            }
        } else if (
            D_80101EC8[0].is_defeated != 0 && D_80101EC8[1].is_defeated != 0) {
            g_fight_reinit_phase_timer = 1;
        } else if (D_80101EC8[1].is_defeated != 0) {
            g_fight_result_subject_index = 1;
            g_fight_point_award_phase_timer = 1;
            D_800A3898[g_fight_result_subject_index ^ 1]++;
        } else if (D_80101EC8[0].is_defeated != 0) {
            g_fight_result_subject_index = 0;
            g_fight_point_award_phase_timer = 1;
            D_800A3898[g_fight_result_subject_index ^ 1]++;
        } else if ((u16)D_80101EC8->move_kind == 6 ||
                   (u16)D_80101EC8[1].move_kind == 6) {
            g_fight_reinit_phase_timer = 0x3C;
        } else if (D_80101EC8[0].collision_hit_tag == 2) {
            u16 id;

            snd_QueueSound(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[0].move_kind;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 ||
                id == 0x1A || id == 0x18) {
                g_fight_result_subject_index = 0;
            } else {
                g_fight_result_subject_index = 2;
            }
        } else if (D_80101EC8[1].collision_hit_tag == 2) {
            u16 id;

            snd_QueueSound(0x9F, 0x7F, 0x7F);
            D_800A37E1 = 1;
            id = D_80101EC8[1].move_kind;
            if (id == 0x13 || id == 0x1B || id == 0x30 || id == 0x19 ||
                id == 0x1A || id == 0x18) {
                g_fight_result_subject_index = 1;
            } else {
                g_fight_result_subject_index = 2;
            }
        } else if (g_fight_time_limit_seconds != 0) {
            g_fight_time_limit_elapsed_frames++;
            if (g_fight_time_limit_elapsed_frames >=
                g_fight_time_limit_seconds * 30) {
                snd_QueueSound(0x9E, 0x7F, 0x7F);
                g_fight_time_up_phase_timer = 1;
            }
        }
        if (g_fight_time_limit_seconds != 0) {
            temp = g_fight_time_limit_seconds * 30 -
                   g_fight_time_limit_elapsed_frames;
            time_fields.unk_2 = temp / 30;
            time_fields.unk_3 = temp % 30 * 100 / 30;
        }
        g_prim_buf_cursor +=
            gpu_DrawPairedMarkRowsAndTimeFields(
                &time_fields,
                D_800A3898[0] | (D_800A38AA[0] << 8) | (D_800A3898[1] << 4) |
                    (D_800A38AA[1] << 12),
                (s32)g_prim_buf_cursor, 1) /
            4;
    }
}

extern s8 D_800A30FC;
extern s8 D_800A30FD;
extern s32 D_800FF6A8;

void fight_ResetGraphAndReloadResourcesOnCachedKeyChange(void) {
    s32 load_addr = (s32)0x80190800;
    s32 copy_size;
    s32 *copy_dest;

    gpu_ResetGraphMode1();

    if (g_fight_stage_index != g_fight_loaded_stage_index ||
        D_8008E5A8[(s8)g_fight_config.unk_4[0]] != D_800A30FC ||
        D_8008E5A8[(s8)g_fight_config.unk_4[1]] != D_800A30FD) {
        motion_InvalidateMarkedNdataPackAndClearSlot1Id();
        resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
            g_fight_stage_index, load_addr);
        fight_ClearPositionRecordUsage();
        snd_CloseListedVabs();

        copy_size = snd_ReloadListedVabsFromFileAndInstallBillboards(
            load_addr, g_fight_stage_index,
            D_8008E5A8[(s8)g_fight_config.unk_4[0]],
            D_8008E5A8[(s8)g_fight_config.unk_4[1]]);

        g_fight_loaded_stage_index = g_fight_stage_index;
        D_800A30FC = D_8008E5A8[(s8)g_fight_config.unk_4[0]];
        D_800A30FD = D_8008E5A8[(s8)g_fight_config.unk_4[1]];

        if (copy_size >= 0x2519) {
            sys_Panic();
        }

        copy_dest = &D_800FF6A8;
        memcpy(copy_dest, load_addr, copy_size);
        snd_RebaseListedVabs((s32)copy_dest - load_addr);
    }
}

void snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVab9(void) {
    s32 load_addr = (s32)0x80190800;
    s32 copy_size;
    s32 *copy_dest;
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    snd_StopAllVoicesAndCloseSingleFileVab();
    copy_size = snd_CloseThenReadSingleFileAndLoadVab((s32)0x80190800);
    if (copy_size >= 0xE81) {
        sys_Panic();
    }
    copy_dest = &D_80104F38;
    memcpy(copy_dest, (s32)0x80190800, copy_size);
    snd_VabFakeOpen9((s32)copy_dest - load_addr);
}

void snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVabs8And4(void) {
    s32 load_addr = (s32)0x80190800;
    s32 copy_size;
    s32 *copy_dest;
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    snd_StopAllVoicesAndClosePairedFileVabs();
    copy_size =
        snd_ClosePairedFileVabsThenLoadFromConsecutiveFiles((s32)0x80190800);
    if (copy_size >= 0x1B19) {
        sys_Panic();
    }
    copy_dest = &D_80104F38;
    memcpy(copy_dest, (s32)0x80190800, copy_size);
    snd_VabFakeOpen8And4((s32)copy_dest - load_addr);
}

void snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode(void) {
    snd_ResetVoicesAndQueue();
    snd_StopVoicesAndReverb();
    if (g_fight_mode_selector == 5) {
        snd_StopAllVoicesAndCloseSingleFileVab();
    }
    if (g_fight_mode_selector == 3) {
        snd_StopAllVoicesAndClosePairedFileVabs();
    }
}

void snd_StartFightCdAudio(void) {
    snd_SerialMixOn();
    if (snd_GetSavedCdAudioSuppressFlag()) {
        return;
    }
    switch (g_fight_mode_selector) {
    case 0:
    case 1:
    default:
        break;
    case 4:
        cdrom_StartAudioEntry(
            (&g_snd_stage_cd_audio_entry_indices)[g_fight_stage_index]);
        break;
    case 3:
        if (g_fighter_group_flags[D_80101EC8[0].character_data_index] != 0) {
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

s32 snd_ShouldUpdateStageSoundSlots(void) {
    s32 v = g_fight_mode_selector;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return snd_GetSavedCdAudioSuppressFlag();
    }
    return 1;
}

void motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id(void) {
    ndata_LoadPack(
        g_fighter_group_flags[D_80101EC8[0].character_data_index] < 1,
        (s32)0x80190800);
    g_motion_fighter_loaded_data_ids[1] = 0xFFFF;
}

void fight_WaitForCounterThenClearAndEnableDisplay(void) {
    s32 i;

    if (g_local_ot_draw_mode != DISP_ACTIVE) {
        return;
    }
    comb_BeginWait();
    if (!(g_fight_stage_list_draw_cursor > g_fight_stage_command_count)) {
        do {
            comb_ExchangeEmptyPad();
            gpu_DrawDisplayModeOnLocalOtAndSync();
            VSync(2);
        } while (
            !(g_fight_stage_list_draw_cursor > g_fight_stage_command_count));
        i = 0;
        do {
            comb_ExchangeEmptyPad();
            i += 1;
            gpu_DrawDisplayModeOnLocalOtAndSync();
            VSync(2);
        } while (i < 15);
    }
    comb_EndWait();
    gpu_HideResetAndClear();
    gpu_SetDispMaskOn();
}

extern void motion_LoadFighterData(s32, s32, s32, s32);
extern void motion_InitStreams(void);
extern void camera_InitTwoFighterView(Unk80101EC8Record *, Unk80101EC8Record *);
extern void camera_InitTrackedFighterFocus(
    Unk80101EC8Record *, Unk80101EC8Record *);

void fight_Init(void) {
    s32 i;
    s32 addr;

    snd_ResetVoicesAndQueue();
    if (g_local_ot_draw_mode != DISP_ACTIVE) {
        gpu_HideResetAndClear();
        gpu_SetDispMaskOn();
    }
    gpu_DrawDisplayModeOnLocalOtAndSync();
    gpu_ResetGraphMode1();
    gpu_RestoreObjectClutOffsets();
    object_ResetPool();
    fight_ClearBallisticRecords();
    func_8003F218(g_fight_tracked_view_setting);
    SetGeomScreen(
        math_FovToScreenDist(g_fight_tracked_view_setting != 0 ? 0x50 : 0x2D));
    for (i = 0; i < 2; i++) {
        if (g_fight_mode_selector != 0) {
            slot_RequestTimGroupsAndMarkReloadOnChange(0, 0);
        }
        fighter_Init(
            i, (s8)g_fight_config.unk_4[4 + i], (s8)g_fight_config.unk_4[i],
            (s8)g_fight_config.unk_4[2 + i], 0);
        if (g_fight_tracked_view_setting != 0 &&
            g_fight_tracked_fighter_index == i) {
            gpu_MoveObjectClutsToFogBlock(i == 0);
        }
    }
    softbody_RequestSnap(0);
    softbody_RequestSnap(1);
    if (g_fight_reinit_reuses_assets == 0) {
        ResetEntrySelectionState();
        addr = (s32)0x80190800;
        motion_InvalidateMarkedNdataPackAndClearSlot1Id();
        for (i = 0; i < 2; i++) {
            if (g_fight_tracked_view_setting != 0 &&
                g_fight_tracked_fighter_index == i) {
                slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
                    i,
                    g_fight_selection_object_ids[(s8)g_fight_config.unk_4[i]],
                    addr);
                node_LoadSlotMappedAssetAndInitRetainedDrawNodes(i);
            } else if (g_fight_mode_selector == 3 && i == 1) {
                slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
                    1, g_fight_step_row_object_id, 0);
            } else {
                slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
                    i,
                    g_fight_selection_object_ids[(s8)g_fight_config.unk_4[i]],
                    addr);
            }
            resource_RequestEntryWithLinkedEntriesAndLatchCategory(
                D_80101EC8[i].weapon_entry_id);
            if (g_fight_mode_selector != 3 || i != 1) {
                if ((g_fight_mode_selector == 2 && D_800A389A == 0) ||
                    g_fight_mode_selector == 5) {
                    resource_SetPairSelectionSlotWithBoundChecks(
                        i, D_8008E6A4[D_80101EC8[i].character_data_index]
                                     [D_80101EC8[i].weapon_id]);
                } else {
                    resource_SetPairSelectionSlotWithBoundChecks(
                        i, D_8008E5CC[D_80101EC8[i].character_data_index]
                                     [D_80101EC8[i].weapon_id]);
                }
            }
            if (D_80101EC8[i].wielded_object_kind != -1) {
                resource_RequestEntryWithLinkedEntriesAndLatchCategory(
                    g_resource_kind_entry_ids[D_80101EC8[i]
                                                  .wielded_object_kind]);
                if (D_80101EC8[i].wielded_object_kind == 14) {
                    resource_RequestEntryWithLinkedEntriesAndLatchCategory(
                        g_resource_kind_entry_ids[14] + 3);
                }
            }
        }
        resource_RankRequestedEntriesAndReloadOnChange(addr);
        node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
        node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
        if (g_fight_mode_selector == 0 && D_800A3712 == 0) {
            gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
                g_fight_operand_tint_red, g_fight_operand_tint_green,
                g_fight_operand_tint_blue);
        } else if (g_fight_mode_selector == 3) {
            gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
                g_fight_step_row_tint_red, g_fight_step_row_tint_green,
                g_fight_step_row_tint_blue);
        } else if (g_fight_mode_selector == 2) {
            u8 *p = D_800A3100
                [g_fighter_group_flags[D_80101EC8[0].character_data_index]];
            if (D_800A389A == 0) {
                gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
                    p[0], p[1], p[2]);
            }
        }
        fight_ResetGraphAndReloadResourcesOnCachedKeyChange();
        if (g_fight_mode_selector == 5) {
            snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVab9();
        }
        if (g_fight_mode_selector == 3) {
            snd_ResetGraphForgetSlot1LoadCopyAndFakeOpenVabs8And4();
            motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id();
        }
        motion_LoadFighterData(
            g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[0]],
            (s8)g_fight_config.unk_4[2],
            g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[1]],
            (s8)g_fight_config.unk_4[3]);
    } else if (g_fight_mode_selector == 5) {
        g_fight_sound_cue_countdown = 1;
    }
    motion_InitStreams();
    fighter_AssignMotionSlotAndScanDataIdNibbles(0);
    fighter_AssignMotionSlotAndScanDataIdNibbles(1);
    fighter_ConfigureCpuControl();
    if (g_fight_mode_selector == 2 || g_fight_mode_selector == 5) {
        if (g_fight_reinit_reuses_assets != 0) {
            motion_ResetVariantIndex(0);
            {
                MoveScript *v = motion_GetDefaultVariantScript(0);
                D_80101EC8[0].move_is_common = 0;
                fighter_StartMoveScript(0, v, 0);
            }
            if (g_fight_mode_selector == 2 && D_800A389A == 0) {
                MoveScript *v = motion_GetCurrentVariantScript(1);
                D_80101EC8[1].move_is_common = 0;
                fighter_StartMoveScript(1, v, 0);
            } else {
                MoveScript *v;
                motion_ResetVariantIndex(1);
                v = motion_GetDefaultVariantScript(1);
                D_80101EC8[1].move_is_common = 0;
                fighter_StartMoveScript(1, v, 0);
            }
        } else {
            motion_ResetVariantIndex(0);
            motion_ResetVariantIndex(1);
            {
                MoveScript *v = motion_GetCommonScriptAtFighterBufferOffset(0);
                D_80101EC8[0].move_is_common = 1;
                fighter_StartMoveScript(0, v, 1);
            }
            {
                MoveScript *v = motion_GetCommonScriptAtFighterBufferOffset(1);
                D_80101EC8[1].move_is_common = 1;
                fighter_StartMoveScript(1, v, 1);
            }
        }
    } else {
        motion_ResetVariantIndex(0);
        motion_ResetVariantIndex(1);
        {
            MoveScript *v = motion_GetDefaultVariantScript(0);
            D_80101EC8[0].move_is_common = 0;
            fighter_StartMoveScript(0, v, 0);
        }
        {
            MoveScript *v = motion_GetDefaultVariantScript(1);
            D_80101EC8[1].move_is_common = 0;
            fighter_StartMoveScript(1, v, 0);
        }
    }
    g_fight_defeat_delay_frames = 0;
    g_fight_followup_fighter_index = -1;
    camera_InitTwoFighterView(&D_80101EC8[0], &D_80101EC8[1]);
    if (g_fight_tracked_view_setting != 0) {
        camera_InitTrackedFighterFocus(&D_80101EC8[0], &D_80101EC8[1]);
    }
    replay_Reset();
    eff_StopCommands();
    fight_WaitForCounterThenClearAndEnableDisplay();
    g_local_ot_draw_mode = DISP_DISABLED;
    g_gpu_jitter_burst_enabled = 0;
    eff_Init();
    g_fight_reinit_reuses_assets = 0;
    g_main_loop_handler_index = 1;
    fight_QueuePositionalSoundWithRandomAlternate();
    snd_StartFightCdAudio();
    snd_ResetStageSoundState();
    snd_SetReverb(g_fight_stage_index);
    if (g_fight_mode_selector == 6) {
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
    Rec44 *view_record;

    if (g_fight_tracked_view_setting != 0) {
        if (g_camera_fight_view_mode == 1) {
            if (D_80101EC8[0].is_defeated != 0 ||
                D_80101EC8[1].is_defeated != 0) {
                g_camera_fight_view_mode = 2;
            }
        }
        if (g_camera_fight_view_mode != 2) {
            if (D_80101EC8[0].move_kind == 0x11 ||
                D_80101EC8[1].move_kind == 0x11) {
                view_record = &g_camera_default_view_record;
                g_camera_fight_view_mode = 1;
            } else {
                view_record = &g_camera_alternate_view_record;
                g_camera_fight_view_mode = 0;
            }
        } else {
            view_record = &g_camera_default_view_record;
        }
        func_8003F218(g_camera_fight_view_mode < 1);

        SetGeomScreen(
            math_FovToScreenDist(g_camera_fight_view_mode == 0 ? 0x50 : 0x2D));

        if (g_camera_fight_view_mode == 0) {
            node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(
                g_fight_tracked_fighter_index, 1);
            node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(
                g_fight_tracked_fighter_index == 0, 0);
        } else {
            node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
            node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
        }
    } else {
        view_record = &g_camera_default_view_record;
    }

    if (g_main_loop_handler_index == 1) {
        local.focus_point.x =
            view_record->focus_point.x + g_camera_view_offset.unk_00.x;
        local.focus_point.y =
            view_record->focus_point.y + g_camera_view_offset.unk_00.y;
        local.focus_point.z =
            view_record->focus_point.z + g_camera_view_offset.unk_00.z;
        local.camera_angles.vx =
            view_record->camera_angles.vx + g_camera_view_offset.unk_10.vx;
        local.camera_angles.vy =
            view_record->camera_angles.vy + g_camera_view_offset.unk_10.vy;
        local.camera_angles.vz =
            view_record->camera_angles.vz + g_camera_view_offset.unk_10.vz;
        local.w18 = view_record->w18 + g_camera_view_offset.w18;
    } else {
        local = *view_record;
    }

    gpu_ResetDrawQueueAndPrepareViewAndLighting(
        &local.focus_point, &local.camera_angles, local.w18);
    {
        camera_CalcEye(&local, &view_record->w20);
        func_80061064(&local.camera_angles.vx, &view_record->w20);
    }
    gpu_SetCellForceBit(view_record->h30[0]);
    gpu_SetCellForceBit(view_record->h30[1]);
    g_listener_cam = view_record;
}

void camera_ApplyReplayView(s32 playback_fraction) {
    /* FAKE: frame layout: unwritten leading pad for the original frame's 8
     * untouched bytes (dead-vars-local-array carve-out; SOTN precedent:
     * volatile u32 pad[4]; // FAKE at st/sel/stream.c:80); without: score 21.
     */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *view_record;

    view_record = &g_camera_alternate_view_record;
    if ((u32)(playback_fraction - 0x555) >= 0x556U) {
        view_record = &g_camera_default_view_record;
    }

    local.focus_point.x =
        view_record->focus_point.x + g_camera_view_offset.unk_00.x;
    local.focus_point.y =
        view_record->focus_point.y + g_camera_view_offset.unk_00.y;
    local.focus_point.z =
        view_record->focus_point.z + g_camera_view_offset.unk_00.z;
    local.camera_angles.vx =
        view_record->camera_angles.vx + g_camera_view_offset.unk_10.vx;
    local.camera_angles.vy =
        view_record->camera_angles.vy + g_camera_view_offset.unk_10.vy;
    local.camera_angles.vz =
        view_record->camera_angles.vz + g_camera_view_offset.unk_10.vz;

    local.w18 = view_record->w18 + g_camera_view_offset.w18;
    gpu_ResetDrawQueueAndPrepareViewAndLighting(
        &local.focus_point, &local.camera_angles, local.w18);

    {
        camera_CalcEye(&local, &view_record->w20);
        func_80061064(&local.camera_angles.vx, &view_record->w20);
    }

    g_listener_cam = view_record;
}

void fight_QueueTrackedFighterNodesFromRecordFlagsAndInitOt(void) {
    s32 v = g_fight_tracked_fighter_index;
    Unk80101EC8Record *ptr = &D_80101EC8[v];
    s32 a1;
    if (ptr->frame_flags & 1) {
        a1 = ptr->weapon_id;
    } else {
        a1 = -1;
    }
    {
        u32 flags = ptr->frame_flags & 4;
        node_SetSlotRelativeMatrixQueueRetainedNodesAndLinkOrderingTable(
            g_camera_alternate_view_record.h1C, a1, flags > 0);
    }
}

void fight_Update(void) {
    PadState pad_state;

    D_800A3778 = camera_GetViewMatrixWithHalvedMiddleRow();
    camera_UpdateTwoFighterView(
        &D_80101EC8[0].camera_fighter_point,
        &D_80101EC8[1].camera_fighter_point, &D_80101EC8[0], &D_80101EC8[1]);
    if (g_fight_tracked_view_setting != 0) {
        camera_UpdateTrackedFighterView(
            &D_80101EC8[g_fight_tracked_fighter_index]);
    }
    camera_ApplyFightView();
    replay_BeginFrame();
    fight_UpdateSeparationAndPairedMoves();
    fighter_BuildCpuOrMappedPadState(0, &pad_state);
    fighter_Update(0, &pad_state);
    fighter_BuildCpuOrMappedPadState(1, &pad_state);
    fighter_Update(1, &pad_state);
    fight_UpdateFighterInteractionsAndPointCaches();
    object_UpdatePool();
    fight_UpdateBallisticRecords();
    replay_AdvanceFrame();
    if (g_fight_tracked_view_setting != 0 && g_camera_fight_view_mode == 0) {
        fight_QueueTrackedFighterNodesFromRecordFlagsAndInitOt();
    } else {
        gpu_SetViewOriginSegmentCellsGateBit(
            D_80101EC8[0].world_pos.x, D_80101EC8[0].world_pos.z);
        gpu_SetViewOriginSegmentCellsGateBit(
            D_80101EC8[1].world_pos.x, D_80101EC8[1].world_pos.z);
    }
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(
        (g_fight_view_option ^ 1) != 0);
    fight_UpdateModeDisplaysAndMatchProgress();
    snd_UpdateStageSounds();
    fight_UpdateDefeatDelayAndModeProgress();
}

void func_8001EA04(void) {
    u8 v;
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    eff_StopCommands();
    v = D_800A38D4;
    D_80101EC8[1].pose_blend_requests = 0;
    D_80101EC8[0].pose_blend_requests = 0;
    g_handler_frame_counter = 0;
    g_fight_choice_confirm_delay = 0;
    g_main_loop_handler_index = 0xD;
    g_fight_choice_menu_first_cursor = v < 1;
    g_fight_choice_menu_cursor = v < 1;
}

void fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange(
    void) {
    PadState pad_state;
    Unk8001CD68Rec time_fields;

    g_handler_frame_counter += 1;
    D_800A3778 = camera_GetViewMatrixWithHalvedMiddleRow();
    camera_SetFighterFocusView(&D_80101EC8[g_fight_followup_fighter_index == 0],
                               g_handler_frame_counter << 3);
    camera_ApplyFightView();
    replay_BeginFrame();
    fight_UpdateSeparationAndPairedMoves();
    pad_ClearStateBits(&pad_state);
    fighter_Update(0, &pad_state);
    fighter_Update(1, &pad_state);
    fight_UpdateFighterInteractionsAndPointCaches();
    object_UpdatePool();
    fight_UpdateBallisticRecords();
    replay_AdvanceFrame();
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(1);
    snd_UpdateStageSounds();
    if (g_fight_mode_selector == 3) {
        fight_SplitFrameCounterIntoMinutesSecondsHundredths(&time_fields);
        g_prim_buf_cursor +=
            gpu_DrawTimeFieldsAndDecimalCounter(
                &time_fields, g_fight_step_table_next_row,
                (s32)g_prim_buf_cursor, 1) /
            4;
    }
    if (g_fight_choice_confirm_delay == 0) {
        g_prim_buf_cursor +=
            gpu_DrawModeSizedPanelWithSelectedCellOpaque(
                1, g_fight_choice_menu_cursor, (s32)g_prim_buf_cursor, 0) /
            4;
        if ((g_pad_state.pressed & 0x10001000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_fight_choice_menu_cursor !=
                g_fight_choice_menu_first_cursor) {
                g_fight_choice_menu_cursor = g_fight_choice_menu_cursor - 1;
            } else {
                g_fight_choice_menu_cursor = 2;
            }
        } else if ((g_pad_state.pressed & 0x40004000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_fight_choice_menu_cursor == 2) {
                g_fight_choice_menu_cursor = g_fight_choice_menu_first_cursor;
            } else {
                g_fight_choice_menu_cursor = g_fight_choice_menu_cursor + 1;
            }
        }
        if ((g_pad_state.pressed & 0x400040) != 0) {
            snd_QueueSound(1, 0x7F, 0x7F);
            g_fight_choice_confirm_delay =
                (g_fight_choice_menu_cursor == 0) ? 1 : 0x3C;
            if (g_fight_choice_menu_cursor != 0)
                return;
            D_80101EC8[g_fight_followup_fighter_index == 0].draw_move_set = 0;
            if (g_fight_mode_selector == 3) {
                g_fight_timer_frames = g_fight_timer_frames + 0x384;
                if (g_fight_timer_frames > 0x2BF20) {
                    g_fight_timer_frames = 0x2BF20;
                }
            }
        }
        return;
    }
    if (g_fight_choice_menu_cursor == 0) {
        g_prim_buf_cursor +=
            gpu_DrawSelectedStretchedSpritePair(0, (s32)g_prim_buf_cursor, 1) /
            4;
    }
    g_fight_choice_confirm_delay = g_fight_choice_confirm_delay + 1;
    if (g_fight_choice_confirm_delay < 0x3C)
        return;
    if (g_fight_choice_menu_cursor == 0) {
        g_fight_reinit_reuses_assets = 1;
        g_fight_menu_restart_count = g_fight_menu_restart_count + 1;
        g_fight_nearest_start_pair_index = fight_FindNearestStartPair(
            &D_80101EC8[g_fight_followup_fighter_index].world_pos);
        if (D_80101EC8[1].cpu_controlled != 0) {
            fighter_HalveCpuTally(1);
        }
        g_main_loop_handler_index = 0;
        return;
    }
    if (g_fight_choice_menu_cursor == 1) {
        cdrom_PauseAndWait();
        snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
        D_800A31DA = 1;
        g_main_loop_handler_index = 8;
        return;
    }
    if (g_fight_choice_menu_cursor == 2) {
        cdrom_PauseAndWait();
        snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
        g_main_loop_handler_index = 8;
    }
}

void func_8001EEB4(void) {
    s8 idx = g_fight_followup_fighter_index;
    Unk80101EC8Record *entry = &D_80101EC8[idx];
    u16 a1 = entry->move_kind;

    if (a1 != 0xA && entry->airborne_until_landing == 0 && a1 != 0x17 &&
        a1 != 0x18 && entry->is_defeated == 0) {
        motion_ResetVariantIndex(g_fight_followup_fighter_index);
        {
            MoveScript *common_script =
                motion_GetCommonScriptAtFighterBufferIndexedOffset(
                    g_fight_followup_fighter_index,
                    entry->character_data_index);
            s32 idx2 = g_fight_followup_fighter_index;
            entry->move_is_common = 1;
            fighter_StartMoveScript(idx2, common_script, 1);
        }
        entry->unk_26C = 1;
    }

    eff_StopCommands();
    g_handler_frame_counter = 0;
    g_main_loop_handler_index = 0x11;
}

void fight_UpdateInterpolatedFighterViewAndSelectNextPhaseByModeOnTimeoutOrPadPress(
    void) {
    PadState pad_state;
    s16 next_handler_index;

    g_handler_frame_counter += 1;
    D_800A3778 = camera_GetViewMatrixWithHalvedMiddleRow();
    camera_SetInterpolatedFighterView(
        &D_80101EC8[g_fight_followup_fighter_index],
        (g_handler_frame_counter << 12) / 105);
    camera_ApplyFightView();
    replay_BeginFrame();
    fight_UpdateSeparationAndPairedMoves();
    pad_ClearStateBits(&pad_state);
    fighter_Update(0, &pad_state);
    fighter_Update(1, &pad_state);
    fight_UpdateFighterInteractionsAndPointCaches();
    object_UpdatePool();
    fight_UpdateBallisticRecords();
    replay_AdvanceFrame();
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(1);
    snd_UpdateStageSounds();

    if (D_80101EC8[g_fight_followup_fighter_index].is_defeated != 0 &&
        g_fight_mode_selector == 1) {
        g_handler_frame_counter = 0x69;
    }

    if (g_handler_frame_counter >= 0x69 || (g_pad_state.pressed & 0x400040)) {
        switch (g_fight_mode_selector) {
        case 4:
            next_handler_index = 0xC;
            break;
        case 1:
            if (g_fight_followup_fighter_index == 0) {
                snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
                g_local_ot_draw_mode = 2;
                fight_AdvanceOpponentOrder();
                return;
            }
            next_handler_index = 0xC;
            break;
        case 6:
            next_handler_index = 0xC;
            break;
        default:
            snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
            next_handler_index = 2;
            break;
        }
        g_main_loop_handler_index = next_handler_index;
    }
}

void motion_ApplyRecordedPoseAdjustments(
    Unk80101EC8Record *fighter_record, Rec1C *replay_frame,
    MotionFrame *first_frame, MotionFrame *second_frame) {
    s16 temp_v1;
    if (!(replay_frame->b18 & 0x80)) {
        motion_SetFixedChannelBlock(first_frame);
        motion_SetFixedChannelBlock(second_frame);
    }
    math_ComposeAxisRotationsAndRewriteAngles(
        &first_frame->unk_0C[0x15], replay_frame->b14 * 4,
        replay_frame->b15 * 4, 0);
    math_ComposeAxisRotationsAndRewriteAngles(
        &second_frame->unk_0C[0x15], replay_frame->b14 * 4,
        replay_frame->b15 * 4, 0);
    temp_v1 = fighter_record->character_roster_index;
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        first_frame->unk_0C[0x39] += replay_frame->b16 * 4;
        second_frame->unk_0C[0x39] += replay_frame->b16 * 4;
    }
    if ((u32)((u16)fighter_record->weapon_id - 6) < 2U) {
        first_frame->unk_0C[0x33] += replay_frame->b16 * 4;
        second_frame->unk_0C[0x33] += replay_frame->b16 * 4;
    }
}

/* Steers two bone-angle sets (a, b) toward obj's partner (obj->other):
 * while obj+0x6A is 0x15/0x25, a clamped heading (obj+0x1D8 - obj+0x1CA) and
 * a clamped elevation from ratan2(ground distance, height delta) are eased
 * 1/8 of the wrapped difference per call into obj+0x1E6/0x1E8 and applied
 * to both sets via math_ComposeAxisRotationsAndRewriteAngles (otherwise both
 * ease back toward 0; state 0x1F holds elevation at 0x100). Also sets or eases
 * a twist in obj+0x1EA (states 0x1D/0xE with obj+0x8C != 0, or obj+0xE in 6..7
 * with obj+0x6A == 2), and adds random jitter to both sets when obj+0x26E is
 * set and obj+0x96
 * == 0. The ground distance is the g_sqrt_table_u8 byte-LUT integer sqrt (the
 * math_PrepareSegmentCollisionFrame idiom), with one gte_Lzc per root. */
void motion_AdjustFighterPose(
    Unk80101EC8Record *obj, MotionFrame *a, MotionFrame *b) {
    s32 lzc_out;
    s32 lzc_out2;
    s32 target_bearing_offset;
    /* temp2: two values -- the clamped elevation target that obj+0x1E8 eases
     * toward (0x100 in state 0x1F, 0 outside states 0x15/0x25), then the
     * clamped twist target that obj+0x1EA eases toward (Ruling 11). */
    s32 temp2;
    /* FAKE: constant-holder (named-local-fake-exception): var_4 is 0 on every
     * path; the three arm writes reach both
     * math_ComposeAxisRotationsAndRewriteAngles calls unfolded and the pseudo
     * takes $s2 as in the target. The literal 0: score 46; one initializer:
     * score 43; one write after the join: score 46. */
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
        motion_SetFixedChannelBlock(a);
        motion_SetFixedChannelBlock(b);
    }
    if (obj->move_kind == 0x15 || obj->move_kind == 0x25) {
        if (obj->character_roster_index == 0x1F) {
            temp2 = 0x100;
            var_4 = 0;
            target_bearing_offset = 0;
        } else {
            s32 dist_sq;
            s32 dist;

            target_bearing_offset =
                (obj->opponent_bearing - obj->orientation.vy) & 0xFFF;
            if (target_bearing_offset >= 0x800) {
                target_bearing_offset -= 0x1000;
            }
            if (target_bearing_offset < -0x1FF) {
                target_bearing_offset = -0x1FF;
            } else if (target_bearing_offset >= 0x200) {
                target_bearing_offset = 0x1FF;
            }
            dx = obj->other->bone_hit_point_0.x - obj->bone_hit_point_0.x;
            dz = obj->other->bone_hit_point_0.z - obj->bone_hit_point_0.z;
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
            temp2 = 0x400 - ratan2(dist, obj->other->bone_hit_point_0.y -
                                             obj->bone_hit_point_0.y);
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
        target_bearing_offset = 0;
    }

    temp = (target_bearing_offset - obj->eased_pose_bearing_offset) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->eased_pose_bearing_offset = obj->eased_pose_bearing_offset + temp / 8;
    temp = (temp2 - obj->eased_pose_elevation_offset) & 0xFFF;
    if (temp >= 0x800) {
        temp -= 0x1000;
    }
    obj->eased_pose_elevation_offset =
        obj->eased_pose_elevation_offset + temp / 8;
    math_ComposeAxisRotationsAndRewriteAngles(
        &a->unk_0C[0x15], obj->eased_pose_bearing_offset,
        obj->eased_pose_elevation_offset, var_4);
    math_ComposeAxisRotationsAndRewriteAngles(
        &b->unk_0C[0x15], obj->eased_pose_bearing_offset,
        obj->eased_pose_elevation_offset, var_4);

    t = obj->character_roster_index;
    if ((t == 0x1D || t == 0xE) && obj->wielded_object_live_state != 0) {
        temp = (ratan2(g_fighter_horizontal_separation,
                       obj->other->world_pos.y - obj->world_pos.y) -
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
        obj->opponent_elevation_offset = temp;
        a->unk_0C[0x39] += temp;
        b->unk_0C[0x39] += temp;
    }

    if ((u32)((u16)obj->weapon_id - 6) < 2U && obj->move_kind == 2) {
        s32 dist_sq;
        s32 dist;

        if (obj->elevation_origin_from_weapon == 0) {
            obj->opponent_elevation_origin.x = obj->world_pos.x;
            obj->opponent_elevation_origin.y = obj->world_pos.y;
            obj->opponent_elevation_origin.z = obj->world_pos.z;
        }
        dx = obj->other->world_pos.x - obj->opponent_elevation_origin.x;
        dz = obj->other->world_pos.z - obj->opponent_elevation_origin.z;
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
        temp2 = (ratan2(dist, obj->other->world_pos.y -
                                  obj->opponent_elevation_origin.y) -
                 0x400) &
                0xFFF;
        if (temp2 >= 0x800) {
            temp2 -= 0x1000;
        }
        if (temp2 >= 0x200) {
            temp2 = 0x1FF;
        } else if (temp2 < -0x1FF) {
            temp2 = -0x1FF;
        }
        temp = (temp2 - obj->opponent_elevation_offset) & 0xFFF;
        if (temp >= 0x800) {
            temp -= 0x1000;
        }
        obj->opponent_elevation_offset =
            obj->opponent_elevation_offset + temp / 8;
        a->unk_0C[0x33] += obj->opponent_elevation_offset;
        b->unk_0C[0x33] += obj->opponent_elevation_offset;
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
    arg0->remaining_turn = arg1;
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
    a2 = arg0->base_move_frame_step;
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
    if (val < arg0->weapon_hit_start_frames[0]) {
        goto check_outer;
    }
    if (val > arg0->weapon_hit_end_frames[0]) {
        goto check_outer;
    }
    goto clamp;
check_outer:
    if (val < arg0->weapon_hit_start_frames[1]) {
        goto multpath_start;
    }
    if (val > arg0->weapon_hit_end_frames[1]) {
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

    a0 = arg0->transition_commands;
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

s32 fighter_PassesPairedMoveEntryGates(Unk80101EC8Record *arg0, s32 arg1) {
    if (g_fight_mode_selector == 2 || g_fight_mode_selector == 5 ||
        g_fight_mode_selector == 3) {
        return 0;
    }
    if (g_fight_mode_selector == 0 && D_800A385C != 0) {
        return 0;
    }
    if (arg0->other->character_roster_index == 0xD ||
        arg0->other->character_roster_index == 0x1C) {
        return 0;
    }
    if (arg0->character_data_index == 0xE &&
        (arg0->carried_object_count == 0 ||
         arg0->carried_object_kinds[0] != 0xA)) {
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

    if (g_fight_paired_move_start_pending != 0) {
        g_fight_paired_move_start_pending = 0;
        D_800A38E8 = 0xFF;
        if (D_80101EC8[0].is_defeated != 0) {
            return;
        }
        if (D_80101EC8[1].is_defeated != 0) {
            return;
        }
        fighter_StartMoveScript(
            g_fight_paired_move_initiator, g_fight_paired_move, D_800A381C);
        if (g_fight_paired_move_flags & 0x1000) {
            D_80101EC8[g_fight_paired_move_initiator == 0].borrow_other_motion =
                1;
        }
        fighter_StartMoveScript(g_fight_paired_move_initiator == 0,
                                g_fight_paired_move, D_800A381C);
        D_80101EC8[1].move_started = 2;
        D_80101EC8[0].move_started = 2;
        return;
    }
    if (g_fight_missing_support_fighter_index != 0xFF) {
        rec = &D_80101EC8[g_fight_missing_support_fighter_index];
        if (D_800A3769 != 0) {
            rec->pending_move_event = 1;
            rec->falling_backward = 0;
        } else {
            rec->pending_move_event = 0;
            rec->falling_backward = 1;
        }
        if (rec->is_defeated != 0) {
            rec->pending_move_event += 2;
        }
        rec->saved_collision_y = rec->collision_pos.vy;
        rec->other->pending_move_event = 1;
        rec->other->falling_backward = 0;
        rec->other->saved_collision_y = rec->other->collision_pos.vy;
        if (rec->other->is_defeated != 0) {
            rec->other->pending_move_event += 2;
        }
        g_fight_missing_support_fighter_index = 0xFF;
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
        if (fighter_PassesPairedMoveEntryGates(rec, data[3] & 0x80) == 0) {
            continue;
        }
        if (g_fighter_horizontal_separation < lo) {
            continue;
        }
        if (hi < g_fighter_horizontal_separation) {
            continue;
        }
        if (dz <= -100 || dz >= 100) {
            continue;
        }
        kind = rec->other->move_kind;
        if (kind != 0x15 && kind != 0x2C && kind != 0xE && kind != 0x19) {
            continue;
        }
        g_fight_paired_move_initiator = i;
        g_fight_paired_move_start_pending = 0;
        g_fight_missing_support_fighter_index = 0xFF;
        g_fight_paired_move_separation = data[2] * 20;
        D_800A38E8 = data[3] & 0x7F;
        var_3 = motion_ResolveMoveData(rec, ent->id, &rec->move_is_common);
        fighter_StartMoveScript(i, var_3, rec->move_is_common);
        rec->other->borrow_other_motion = 1;
        rec->other->move_is_common = rec->move_is_common;
        fighter_StartMoveScript(i == 0, var_3, rec->move_is_common);
        rec->move_started = 2;
        rec->other->move_started = 2;
        rec->other->variant_index = rec->other->default_variant_index;
        rec->other->unk_272 += 1;
        pos[0] = (rec->world_pos.x + rec->other->world_pos.x) / 2;
        pos[1] = (rec->world_pos.y + rec->other->world_pos.y) / 2;
        pos[2] = (rec->world_pos.z + rec->other->world_pos.z) / 2;
        fight_RecordReplayEventAndDispatchSoundOrEffect(i, 0x10, pos, 0);
        return;
    }
}

s32 fight_UpdateSeparationAndPairedMoves(void) {
    s32 s0 = g_fighter_horizontal_separation;
    s32 v0 = fighter_CalcHorizontalSeparation();
    s32 v = g_fight_mode_selector;
    g_fighter_horizontal_separation = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        fight_UpdatePairedMoves();
    }
    D_800A38A8 = 0;
}

void math_ComputeBallisticVelocityXZ(
    s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
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

/* Starts the rec->bone_event_counter frame counter, records bone index
 * g_fighter_bone_hit_record_templates[arg1].bone at rec->bone_event_bone_index,
 * and rotates the vector arg2 by that bone's matrix of
 * node_GetSlotMatrixPointerArray(rec->index) into rec->bone_event_local_vector.
 */
void fighter_StoreBoneLocalVectorAndArmEventTrigger(
    Unk80101EC8Record *rec, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    MATRIX *src;

    rec->bone_event_counter = 1;
    rec->bone_event_bone_index = g_fighter_bone_hit_record_templates[arg1].bone;
    src = ((MATRIX **)node_GetSlotMatrixPointerArray(
        rec->index))[rec->bone_event_bone_index];
    math_InvertMatrix3x3(src, mat);
    gte_SetRotMatrix(mat);
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    gte_ldlv0(vec);
    gte_rtv0();
    gte_stlvnl(&rec->bone_event_local_vector);
}

/* rec->bone_event_counter is a frame counter
 * fighter_StoreBoneLocalVectorAndArmEventTrigger starts at 1. On the tick where
 * (counter & 7) == 2, the vector fighter_StoreBoneLocalVectorAndArmEventTrigger
 * stored at rec->bone_event_local_vector is rotated by matrix
 * rec->bone_event_bone_index of node_GetSlotMatrixPointerArray(var_2) (GTE
 * MVMVA), scaled by (150 - counter) / 150, negated when its y exceeds 0x800,
 * and handed to fight_RecordReplayEventAndDispatchSoundOrEffect with scratchpad
 * point SPAD->bone_hit_points[var_2][rec->bone_event_bone_index]. The counter
 * is cleared on every call that finds it running (the original stores 0 under
 * `>= 150` and then again unconditionally). */
void fighter_DispatchArmedBoneVectorEventAndDisarm(Unk80101EC8Record *rec) {
    s32 mac[3];
    s16 out[3];
    s32 var_2;
    s32 mul;
    MATRIX **var_4;

    var_2 = rec->index;
    if (rec->bone_event_counter != 0) {
        rec->bone_event_counter += 1;
        if ((rec->bone_event_counter & 7) == 2) {
            var_4 = node_GetSlotMatrixPointerArray(var_2);
            gte_SetRotMatrix(var_4[rec->bone_event_bone_index]);
            gte_ldlv0(&rec->bone_event_local_vector);
            gte_rtv0();
            mul = ((0x96 - rec->bone_event_counter) << 12) / 150;
            gte_stlvnl(mac);
            out[0] = (mac[0] * mul) / 0x1000;
            out[1] = (mac[1] * mul) / 0x1000;
            out[2] = (mac[2] * mul) / 0x1000;
            if (out[1] > 0x800) {
                out[0] = -out[0];
                out[1] = -out[1];
                out[2] = -out[2];
            }
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                var_2, 4,
                &SPAD->bone_hit_points[var_2][rec->bone_event_bone_index].x,
                out);
        }
        if (rec->bone_event_counter >= 0x96) {
            rec->bone_event_counter = 0;
        }
        rec->bone_event_counter = 0;
    }
}

void fighter_ScaleBoneHitRecords(s32 arg0, s32 arg1) {
    BoneHitRec *src = g_fighter_bone_hit_record_templates;
    BoneHitRec *dst = g_fighter_bone_hit_records[arg0];
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
 * matrices (node_GetSlotMatrixPointerArray()): each of the 22 hit records'
 * offsets through its bone into arg1 (the callers pass
 * SPAD->bone_hit_points[i]); the point set D_8008D86C[weapon_id] (D_8008D774
 * when weapon_entry_id == 50) through bone 18 into arg2; when
 * wielded_object_live_state != 0, the pair D_8008D88C[wielded_object_kind]
 * through bone 19 into arg3. A point is the GTE MVMVA rotation (sf=1) of the
 * SVECTOR plus the matrix translation. Then: bone 11's y axis into
 * rec->bone_y_axis, arg1[0] into rec->bone_hit_point_0, the translations of
 * bones 17 and 14 (y raised by (size_scale * 71) >> 11) into
 * rec->ground_probe_points[], the floor height under each
 * (collision_TestSegmentWithWork down a segment from y - 100 to y + 2000; the
 * segment's lower end when it finds nothing) into rec->ground_probe_floor_ys[],
 * and the two bones' headings into unk_1BA / unk_1C2. */
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

    var_1 = node_GetSlotMatrixPointerArray(rec->index);
    var_5 = g_fighter_bone_hit_records[rec->index];
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
    if (rec->weapon_entry_id == 50) {
        v = D_8008D774;
    } else {
        v = D_8008D86C[rec->weapon_id];
    }
    o = arg2;
    for (i = 0; i < D_8008D864[rec->weapon_id]; i++, v++, o++) {
        gte_ApplyRotMatrix(v, o);
        o->x += m->t[0];
        o->y += m->t[1];
        o->z += m->t[2];
    }

    if (rec->wielded_object_live_state != 0) {
        m = var_1[19];
        gte_SetRotMatrix(m);
        v = D_8008D88C[rec->wielded_object_kind];
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
    gte_ldv0(&g_unit_y_vector);
    gte_rtv0();
    rec->bone_hit_point_0 = arg1[0];
    gte_stlvnl(&rec->bone_y_axis);

    pos = var_1[17]->t;
    rec->ground_probe_points[0].x = pos[0];
    rec->ground_probe_points[0].y = pos[1] + ((rec->size_scale * 71) >> 11);
    rec->ground_probe_points[0].z = pos[2];
    pos = var_1[14]->t;
    rec->ground_probe_points[1].x = pos[0];
    rec->ground_probe_points[1].y = pos[1] + ((rec->size_scale * 71) >> 11);
    rec->ground_probe_points[1].z = pos[2];
    for (i = 0; i < 2; i++) {
        var_0[0] = rec->ground_probe_points[i].x;
        var_0[1] = rec->ground_probe_points[i].y - 100;
        var_0[2] = rec->ground_probe_points[i].z;
        var_0[4] = rec->ground_probe_points[i].x;
        var_0[5] = rec->ground_probe_points[i].y + 2000;
        var_0[6] = rec->ground_probe_points[i].z;
        if (collision_TestSegmentWithWork(&var_0[0], &var_0[4], &var_0[8],
                                          (s16 *)&var_0[12], (s32)&var_0[14])) {
            rec->ground_probe_floor_ys[i] = var_0[9];
        } else {
            rec->ground_probe_floor_ys[i] = var_0[5];
        }
    }
    m = var_1[17];
    rec->unk_1BA = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
    m = var_1[14];
    rec->unk_1C2 = ratan2(m->m[0][2], m->m[2][2]) + 0x800;
}

/* g_motion_fighter_loaded_data_ids[i] is the fighter motion id loaded into slot
 * i; 0xFFFF in slot 1 marks an ndata pack there
 * (motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id). Both functions read slot
 * 1's id through `ids`, invalidate the pack if it holds one and store the slot
 * back (pointer read-modify-write, pointer-alias-fake-exception; the direct
 * g_motion_fighter_loaded_data_ids[1] form keeps the address live across
 * ndata_InvalidatePack: score 10 / 9). */
void motion_InvalidateMarkedNdataPackAndResetLoadCaches(void) {
    u16 *ids = g_motion_fighter_loaded_data_ids;

    if (ids[1] == 0xFFFF) {
        ndata_InvalidatePack();
    }
    g_motion_common_loaded = 0;
    ids[1] = 0;
    ids[0] = 0;
    g_motion_frame_set_ids[1] = 0xFF;
    g_motion_frame_set_ids[0] = 0xFF;
}

void motion_InvalidateMarkedNdataPackAndClearSlot1Id(void) {
    u16 *ids = g_motion_fighter_loaded_data_ids;

    if (ids[1] == 0xFFFF) {
        ndata_InvalidatePack();
    }
    ids[1] = 0;
}

void motion_InitBuffers(void) {
    g_motion_frame_buffers[0] = (MotionFrame *)0x80118800;
    g_motion_frame_buffers[1] = (MotionFrame *)0x8011C400;
    g_motion_common_buffer = (s32)0x80120000;
    g_motion_fighter_buffers[0] = (Tbl800A3860Entry *)0x80148800;
    g_motion_fighter_buffers[1] = (Tbl800A3860Entry *)0x80190800;
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
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

/* For i = 0, 1 with v = arg0, then arg2 (when g_fight_mode_selector is 1, 4 or
 * 6): when g_motion_frame_set_ids[i] differs from v, stores v there and calls
 * cdrom_StartReadAt(cdrom_GetFileIndex(1, 0), g_motion_frame_buffers[i], v * 7,
 * 7). Then sets D_80101EC8[0].motion_data_id to
 * g_fighter_motion_data_ids[arg0][arg1] | arg1
 * << 12 and D_80101EC8[1].motion_data_id to the same of arg2 and arg3 (a copy
 * of the first when g_fight_mode_selector is 3). It then fills var_0[0..1] from
 * g_motion_fighter_loaded_data_ids[0..1]: it returns when every id is already
 * there; equal ids give var_0[0] = id0; an id found in
 * g_motion_fighter_loaded_data_ids[0] stays and the other id goes to var_0[1];
 * id1 found only in g_motion_fighter_loaded_data_ids[1] gives var_0[0] = id0;
 * otherwise (also when id0 is only in g_motion_fighter_loaded_data_ids[1])
 * var_0[0] = id0 and var_0[1] = id1. For each nonzero var_0[i] it calls
 * cdrom_StartRead(cdrom_GetFileIndex(1, j + 2), g_motion_fighter_buffers[i]), j
 * the id's D_8008DCCC index, points g_motion_fighter_sections[i] into
 * g_motion_fighter_buffers[i] and stores the id in
 * g_motion_fighter_loaded_data_ids[i]. */
void motion_LoadFighterData(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* FAKE: frame layout: only var_0[0..1] are used; the target frame (0x140)
     * reserves 0x100 untouched bytes after them (N = 129..132; var_0[2] gives
     * the wrong frame); without it: score 20. */
    u16 var_0[130];
    s32 i;
    /* FAKE: one local for loop 1's arg0 / arg2 and loop 2's D_8008DCCC index;
     * separate locals seat the index in $a1, the target keeps both in $s0
     * (score 3) (Q129). */
    s32 j;

    if (g_motion_common_loaded == 0) {
        motion_LoadCommonData();
    }

    if (g_fight_mode_selector == 1 || g_fight_mode_selector == 4 ||
        g_fight_mode_selector == 6) {
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
        u16 id0 = g_fighter_motion_data_ids[arg0][arg1] | (arg1 << 12);
        u16 id1;

        if (g_fight_mode_selector == 3) {
            id1 = id0;
        } else {
            id1 = g_fighter_motion_data_ids[arg2][arg3] | (arg3 << 12);
        }
        D_80101EC8[0].motion_data_id = id0;
        D_80101EC8[1].motion_data_id = id1;

        if (id0 == id1) {
            if (g_motion_fighter_loaded_data_ids[0] == id1 ||
                g_motion_fighter_loaded_data_ids[1] == id1) {
                return;
            }
            var_0[0] = id0;
        } else if (g_motion_fighter_loaded_data_ids[0] == id0) {
            if (g_motion_fighter_loaded_data_ids[1] == id1) {
                return;
            }
            var_0[1] = id1;
        } else if (g_motion_fighter_loaded_data_ids[0] == id1) {
            if (g_motion_fighter_loaded_data_ids[1] == id0) {
                return;
            }
            var_0[1] = id0;
        } else if (g_motion_fighter_loaded_data_ids[1] == id1) {
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
            g_motion_fighter_loaded_data_ids[i] = var_0[i];
        }
    }
}

void motion_InitStreams(void) {
    motion_InitStream(0, g_motion_common_sections.unk_10);
    if (g_motion_fighter_loaded_data_ids[0]) {
        motion_InitStream(1, g_motion_fighter_sections[0].unk_10);
    }
    if (g_motion_fighter_loaded_data_ids[1]) {
        motion_InitStream(2, g_motion_fighter_sections[1].unk_10);
    }
}

/* The record's motion_data_id -> its slot in g_motion_fighter_loaded_data_ids
 * (motion_slot); for ids below 0x2000, the nibble positions of 4 and 5
 * (object_variant_index / unk_8E), copying unk_26C into object_variant_enabled
 * / unk_90 under conditions on g_fight_mode_selector. */
void fighter_AssignMotionSlotAndScanDataIdNibbles(s32 a0) {
    s32 i = 0;
    Unk80101EC8Record *rec = &D_80101EC8[a0];
    s32 id = rec->motion_data_id;
    u16 *p = g_motion_fighter_loaded_data_ids;

    for (; i < 2; i++, p++) {
        if (id == *p) {
            break;
        }
    }
    rec->motion_slot = i;
    rec->borrow_other_motion = 0;
    if ((u32)(rec->motion_data_id >> 12) < 2) {
        for (i = 0; i < 3; i++) {
            s32 nibble = (rec->motion_data_id >> (i << 2)) & 0xF;
            if (nibble == 4) {
                rec->object_variant_index = i;
                if (g_fight_mode_selector != 3 || a0 != 1 ||
                    g_fight_step_row_column == nibble) {
                    rec->object_variant_enabled = rec->unk_26C;
                }
            } else if (nibble == 5) {
                rec->unk_8E = i;
                if (g_fight_mode_selector != 0 || D_800A385C == 0 || a0 != 0) {
                    rec->unk_90 = rec->unk_26C;
                }
            }
        }
    }
}

void motion_AdvanceVariantIfAtNibble4Or5Position(Unk80101EC8Record *arg0) {
    s16 a1 = arg0->variant_index;
    if (a1 != arg0->object_variant_index) {
        if (a1 != arg0->unk_8E) {
            return;
        }
    }
    arg0->variant_index =
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

    rec->blend_into_move = 0;
    *out = 0;
    if ((u32)(id - 0x7FF5) < 11) {
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f66[id - 0x7FF5][rec->variant_index]];
    }
    switch (id) {
    case 0x7FF0:
        rec->blend_into_move = 1;
        rec->variant_index = rec->default_variant_index;
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f4E[rec->default_variant_index]];
    case 0x7FF1:
        rec->variant_index = (rec->variant_index + 1) %
                             g_motion_fighter_buffers[rec->motion_slot]->f14;
    case 0x7FF2:
    case 0x7FF4:
        if (id == 0x7FF4) {
            rec->blend_into_move = 1;
        }
        t = rec->variant_index;
        if ((t == rec->object_variant_index &&
             rec->object_variant_enabled == 0) ||
            (t == rec->unk_8E && rec->unk_90 == 0)) {
            rec->variant_index =
                (rec->variant_index + 1) %
                g_motion_fighter_buffers[rec->motion_slot]->f14;
        } else if (g_fight_mode_selector == 3 && rec->cpu_controlled != 0) {
            motion_AdvanceVariantIfAtNibble4Or5Position(rec);
        }
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f4E[rec->variant_index]];
    case 0x7FF3:
        t = (rec->variant_index + 1) %
            g_motion_fighter_buffers[rec->motion_slot]->f14;
        if ((t == rec->object_variant_index &&
             rec->object_variant_enabled == 0) ||
            (t == rec->unk_8E && rec->unk_90 == 0)) {
            t = (t + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
        } else if (g_fight_mode_selector == 3 && rec->cpu_controlled != 0 &&
                   (t == rec->object_variant_index || t == rec->unk_8E)) {
            t = (t + 1) % g_motion_fighter_buffers[rec->motion_slot]->f14;
        }
        return &g_motion_fighter_sections[rec->motion_slot]
                    .unk_00[g_motion_fighter_buffers[rec->motion_slot]
                                ->f54[rec->variant_index][t]];
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
    D_80101EC8[a0].variant_index = D_80101EC8[a0].default_variant_index;
}

MoveScript *motion_GetCurrentVariantScript(s32 a0) {
    s16 v1 = D_80101EC8[a0].motion_slot;
    s16 v0 = D_80101EC8[a0].variant_index;
    return (MoveScript *)&g_motion_fighter_sections[v1]
        .unk_00[g_motion_fighter_buffers[v1]->f4E[v0]];
}

MoveScript *motion_GetDefaultVariantScript(s32 a0) {
    s16 v1 = D_80101EC8[a0].motion_slot;
    s16 v0 = D_80101EC8[a0].default_variant_index;
    return (MoveScript *)&g_motion_fighter_sections[v1]
        .unk_00[g_motion_fighter_buffers[v1]->f4E[v0]];
}

MoveScript *motion_GetCommonScriptAtFighterBufferOffset(s32 a0) {
    return (MoveScript *)&g_motion_common_sections
        .unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f16];
}

MoveScript *motion_GetCommonScriptAtFighterBufferIndexedOffset(s32 a0, s32 a1) {
    return (MoveScript *)&g_motion_common_sections
        .unk_00[g_motion_fighter_buffers[D_80101EC8[a0].motion_slot]->f18[a1]];
}

void fighter_StartMoveScript(s32 arg0, MoveScript *arg1, s32 arg2) {
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
        u16 v1 = arg1->motion_bank_entry_index;
        s0->motion_entry_index = v1;
        if (arg2 != 0) {
            u16 *v0 = &g_motion_common_sections.unk_04[v1 * 2];
            s0->motion_entry = v0;
            s0->motion_metadata = g_motion_common_sections.unk_08 + v0[1];
        } else {
            u16 *v0 = &g_motion_fighter_sections[a3].unk_04[v1 * 2];
            s0->motion_entry = v0;
            s0->motion_metadata = g_motion_fighter_sections[a3].unk_08 + v0[1];
        }
    }
    {
        MoveScript *v0_50 = s0->cur_move;
        u16 old_kind = s0->move_kind;
        u8 *a0_58 = s0->motion_metadata;
        s0->cur_move_is_common = (u8)arg2;
        /* FAKE: empty do-while(0) -- its loop note puts a0_58 in $a0 and
         * v1_58 in $v1 (the two motion_metadata lbu bases); without it they
         * swap, score 2. */
        do {
        } while (0);
        s0->cur_move_slot = (u8)a3;
        {
            u8 a1_val = v0_50->first_move_frame;
            s0->prev_move_kind = old_kind;
            {
                u8 *v1_58 = s0->motion_metadata;
                s0->move_frame_frac = 0;
                s0->move_started = 1;
                s0->queued_move = 0;
                s0->move_frame_unchanged = 0;
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
                s0->attack_hit_armed = a0_flag;
            }
            fighter_ReadFrameMetadata(s0);
            {
                u16 state = s0->move_kind;

                if (state == 9) {
                    s0->use_root_offset_yaw = 1;
                    s0->root_offset_yaw = s0->orientation.vy;
                } else if (state == 2) {
                    if (s0->use_root_offset_yaw == 0 &&
                        s0->prev_move_kind != 0x13) {
                        s0->root_offset_yaw = s0->opponent_bearing;
                    }
                    s0->use_root_offset_yaw = 0;
                } else if (state == 0x19 || state == 0x1A) {
                    if (s0->use_root_offset_yaw == 0 ||
                        (state == 0x19 && s0->prev_move_kind == state)) {
                        s0->root_offset_yaw = s0->opponent_bearing;
                    }
                    s0->use_root_offset_yaw = 1;
                } else if (state == 0x11) {
                    s0->use_root_offset_yaw = 1;
                } else {
                    s0->use_root_offset_yaw = 0;
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
    temp_v0 = (s16 *)fight_GetStartPositionPairTable() +
              ((g_fight_stage_index * 0x18) + (arg2 * 6) + (arg0 * 3));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}

void fight_SearchRadialFlatSurfacePointOrUseStartPositionPair(
    s32 arg0, Vec3i32 *out, s32 *pos) {
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
     * fight_GetStartPositionPairTable's block
     * (g_fight_start_position_pair_index when g_fight_mode_selector is 3, else
     * the one of four nearest pos in x / z).
     */
    s32 temp;
    s32 angle;
    s32 dx;
    s32 dz;
    s32 best;
    s32 d;

    var_5 = fight_GetStartPositionPairTable();
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
            if (collision_TestSegmentExtendingShortStartWithWork(
                    &cur.x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) != 0) {
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
            if (collision_TestSegmentExtendingShortStartWithWork(
                    &cur.x, &var_2.x, &var_3.x, var_4, 0x1F8002B8) != 0 &&
                var_4[1] == -0x1000) {
                out->y = var_3.y;
                return;
            }
            cur.y = var_2.y;
        }
    }
    if (g_fight_mode_selector == 3) {
        temp = g_fight_start_position_pair_index;
    } else {
        best = 0x7FFFFFFF;
        var_5 += g_fight_stage_index * 24;
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

void fight_PickRandomStartPositionExcludingNearestAndFarthest(
    s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[4];
    s32 picks[2];
    s16 *base;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 j;

    base = (s16 *)fight_GetStartPositionPairTable() +
           (g_fight_stage_index * 3) * 8;
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

    p = fight_GetStartPositionPairTable();
    best_dist = 0x7FFFFFFF;
    x = arg0->x;
    z = arg0->z;
    p += g_fight_stage_index * 24;
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

s32 motion_SelectVariantNibblePositionMatchingPreferredPairInOpponentValue(
    Unk80101EC8Record *arg0) {
    u8 *p;
    s32 val;
    s32 i;
    s32 j;

    p = D_8008EB1C[g_fight_step_row_column];
    val = g_fighter_motion_data_ids[arg0->other->character_data_index]
                                   [arg0->other->weapon_id];
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 3; i++) {
            if (p[j] == ((val >> (i * 4)) & 0xF)) {
                return i;
            }
        }
    }
    return 0;
}

void fighter_ResetBoneGroupHitState(Unk80101EC8Record *arg0) {
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
    g_fight_missing_support_fighter_index = 0xFF;
    g_fight_paired_move_start_pending = 0;
    p->update_count = 0;
    p->other = (idx != 0) ? &D_80101EC8[0] : &D_80101EC8[1];
    p->index = idx;
    p->cpu_controlled = arg1;
    p->character_roster_index = arg2;
    p->character_data_index = g_fight_config_id_data_rows[arg2];
    p->weapon_id = arg3;

    if (g_fight_mode_selector == 5 ||
        (g_fight_mode_selector == 2 && D_800A389A == 0)) {
        p->weapon_entry_id = D_8008EB38[p->weapon_id];
    } else if (idx == 1 && g_fight_mode_selector == 3) {
        p->weapon_entry_id =
            D_8008EB28[p->weapon_id]
                      [g_fighter_group_flags[p->other->character_data_index] ==
                       0];
    } else if (g_fight_mode_selector == 0 && D_800A385C != 0 && idx == 0) {
        p->weapon_entry_id = 0x32;
    } else {
        p->weapon_entry_id =
            D_8008EB28[p->weapon_id]
                      [g_fighter_group_flags[p->character_data_index]];
    }

    if (idx == 1 && g_fight_mode_selector == 3) {
        p->size_scale = g_object_scale_factors[g_fight_step_row_object_id];
    } else {
        p->size_scale = g_object_scale_factors
            [g_fight_selection_object_ids[p->character_roster_index]];
    }
    p->base_move_frame_step =
        g_fighter_base_move_frame_steps[p->character_data_index][p->weapon_id];
    p->base_power =
        g_fighter_base_powers[p->character_data_index][p->weapon_id];
    p->power = p->base_power;
    p->cpu_base_level = 0x1000;

    if (g_fight_mode_selector == 3 && idx == 1) {
        p->default_variant_index =
            motion_SelectVariantNibblePositionMatchingPreferredPairInOpponentValue(
                p);
        var_2 = (g_fight_step_table_next_row - 1) / 10 + 1;
        p->base_move_frame_step = var_2 * 96 + ((var_2 == 9) ? 0xA00 : 0x800);
        if (var_2 == 9) {
            p->power = 0x1080;
        } else {
            p->power = var_2 * 64 + 0xC00;
        }
        if (var_2 == 9) {
            p->cpu_base_level = 0xE00;
        } else {
            p->cpu_base_level = var_2 * 80 + 0x150;
        }
    } else {
        p->default_variant_index =
            D_8008DD5C[p->character_data_index][p->weapon_id];
    }

    p->object_variant_index = -1;
    p->object_variant_enabled = 0;
    p->unk_8E = -1;
    p->unk_90 = 0;
    var_5 = (g_fight_reinit_reuses_assets != 0)
                ? g_fight_nearest_start_pair_index
                : 0;

    switch (g_fight_mode_selector) {
    case 0:
        if (arg4 != 0) {
            var_1 = p->other->base_pos;
            fight_PickRandomStartPositionExcludingNearestAndFarthest(
                idx, &p->base_pos.x, &var_1.x);
        } else {
            fight_GetStartPosition(idx, &p->base_pos.x, var_5);
            fight_GetStartPosition(idx == 0, &var_1.x, var_5);
        }
        break;
    case 2:
    case 3:
        if (arg4 != 0) {
            var_1 = p->other->base_pos;
            fight_SearchRadialFlatSurfacePointOrUseStartPositionPair(
                idx, &p->base_pos, &var_1.x);
        } else {
            fight_GetStartPosition(
                idx, &p->base_pos.x, g_fight_start_position_pair_index);
            fight_GetStartPosition(
                idx == 0, &var_1.x, g_fight_start_position_pair_index);
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
    p->prev_root_offset = p->root_offset;
    p->base_velocity.vx = 0;
    p->base_velocity.vy = 0;
    p->base_velocity.vz = 0;
    p->unk_24C = p->base_velocity;
    p->weapon_point_delta[0].vx = 0;
    p->weapon_point_delta[0].vy = 0;
    p->weapon_point_delta[0].vz = 0;
    p->weapon_point_delta[1].vx = 0;
    p->weapon_point_delta[1].vy = 0;
    p->weapon_point_delta[1].vz = 0;
    p->decaying_xz_velocity.vx = 0;
    p->decaying_xz_velocity.vy = 0;
    p->decaying_xz_velocity.vz = 0;
    p->unk_144 = 0;
    p->ground_y = p->collision_pos.vy;
    p->remaining_turn = 0;
    p->unk_150 = 0;
    p->use_root_offset_yaw = 0;
    p->forward_move_rate = 0;
    p->base_velocity_damping_x = 0xC00;
    p->base_velocity_damping_y = 0xF80;
    p->base_velocity_damping_z = 0xC00;
    p->decaying_xz_velocity_damping_x = 0xC00;
    p->decaying_xz_velocity_damping_y = 0xC00;
    p->decaying_xz_velocity_damping_z = 0xC00;
    p->eased_pose_elevation_offset = 0;
    p->eased_pose_bearing_offset = 0;
    p->opponent_elevation_offset = 0;
    p->elevation_origin_from_weapon = 0;
    p->camera_fighter_point = p->world_pos;
    p->bone_hit_points_4_5_midpoint = p->world_pos;
    p->bone_hit_point_0 = p->world_pos;
    p->bone_hit_points_1_3_centroid = p->world_pos;
    p->orientation.vx = 0;
    p->orientation.vy = 0;
    p->orientation.vz = 0;
    ang = ratan2(var_1.x - p->world_pos.x, var_1.z - p->world_pos.z);
    p->orientation.vy = p->opponent_bearing = ang;
    if (p->character_roster_index == 0x1F) {
        p->orientation.vy = ang + 0x800;
    }
    p->unk_1D0 = p->orientation;
    fighter_ScaleBoneHitRecords(idx, p->size_scale);
    if (g_fight_mode_selector != 0 || idx != 0 ||
        g_fight_stage_start_count < 2 || g_fight_reinit_reuses_assets != 0) {
        fighter_ResetBoneGroupHitState(p);
    }
    p->unk_274 = D_8008E3C0[p->character_data_index];
    for (i = 0; i < 4; i++) {
        p->unk_276[i] = D_8008E3F8[p->character_data_index][i];
        p->unk_27E[i] = D_8008E4D0[p->character_data_index][i];
    }
    p->unk_A0 = 8;
    p->queued_move = 0;
    p->pending_move_event = -1;
    p->pose_blend_requests = 0;
    p->downward_probe_hit = 0;
    p->airborne_until_landing = 0;
    p->is_defeated = 0;
    p->collision_hit_tag = 7;
    p->collision_hit_class = 0;
    p->reposition_count = 0;
    object_InitKindQueue(p);
    p->wielded_object_kind = p->carried_object_kinds[0];
    if (g_fight_mode_selector == 5 ||
        (g_fight_mode_selector == 2 && D_800A389A == 0)) {
        if (p->character_data_index == 1 || p->character_data_index == 3 ||
            p->character_data_index == 4 || p->character_data_index == 9 ||
            p->character_data_index == 0x11) {
            p->carried_object_kinds[0] = p->wielded_object_kind = 0x11;
        } else {
            p->wielded_object_kind = -1;
            p->carried_object_count = 0;
        }
    } else if (g_fight_mode_selector == 3 && idx == 1 &&
               g_fight_step_row_column != 4) {
        p->wielded_object_kind = -1;
        p->carried_object_count = 0;
    } else if (g_fight_mode_selector == 3 && idx == 1 &&
               g_fight_step_row_column == 4) {
        if (p->other->character_data_index == 1 ||
            p->other->character_data_index == 3 ||
            p->other->character_data_index == 4 ||
            p->other->character_data_index == 9 ||
            p->other->character_data_index == 0x11) {
            p->carried_object_kinds[0] = p->wielded_object_kind =
                p->other->wielded_object_kind;
            p->carried_object_count = 1;
        }
    } else {
        if (p->character_roster_index == 0x1D) {
            p->wielded_object_kind = 0x1F;
            p->weapon_ammo = 2;
        } else if (p->character_roster_index == 0xE) {
            p->wielded_object_kind = 0x1E;
            p->weapon_ammo = 2;
        } else {
            p->weapon_ammo = 0xA;
        }
        p->weapon_refills_left = 9;
    }
    if (g_fight_mode_selector == 5 || g_fight_mode_selector == 2 ||
        g_fight_mode_selector == 3 ||
        (g_fight_mode_selector == 0 && D_800A385C != 0)) {
        p->throw_charges_left = 0;
    } else {
        p->throw_charges_left = 2;
    }
    p->bone_event_counter = 0;
}

void fighter_ConfigureCpuControl(void) {
    s32 i;
    u16 *tbl;

    i = 0;
    tbl = g_fight_config.cpu_base_levels;
    do {
        Unk80101EC8Record *rec = &D_80101EC8[i];

        if (rec->cpu_controlled != 0) {

            switch (g_fight_mode_selector) {
            case 0:
                rec->cpu_base_level =
                    g_fight_config.operand_cpu_base_levels[i] << 4;
                break;
            case 1:
            case 2:
            default:
                rec->cpu_base_level = *tbl;
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

/* The masked state id is spelled as in its
 * copy-paste sibling fighter_UpdateGroundYByMoveKind (`s32 a0 =
 * arg0->move_kind; s32 v1 = a0 & 0xFFFF;`, same comparison cascade), which
 * keeps the target's `andi $v1,$a0,0xffff`: the raw value stays live for `mode
 * - 0x17`, so combine never folds the mask. A u16 on either side loses it. */
void fighter_ResolveMovement(
    Unk80101EC8Record *obj, VECTOR *pos1, VECTOR *pos2, s32 *arg3) {
    s32 *scratch = (s32 *)0x1F8001B0;
    s32 count = 0;
    do {
        if (((pos1->vx != pos2->vx) || (pos1->vy != pos2->vy)) ||
            (pos1->vz != pos2->vz)) {
            if (collision_TestSegmentExtendingShortStartWithWork(
                    &pos1->vx, &pos2->vx, scratch, (s16 *)(scratch + 4),
                    0x1F8002B8) == 0) {
                *pos1 = *pos2;
                break;
            }
            obj->collision_hit_tag = collision_GetHitTag();
            *pos1 = *(VECTOR *)scratch;
            math_ScaleVectorInDirectionFrame(
                arg3, (s16 *)(scratch + 4), arg3, -0x40, 0xE6);
            scratch[12] =
                pos1->vx + (*((s16 *)(((u8 *)scratch) + 0x10)) / 1024);
            scratch[13] =
                pos1->vy + (*((s16 *)(((u8 *)scratch) + 0x12)) / 1024);
            scratch[14] =
                pos1->vz + (*((s16 *)(((u8 *)scratch) + 0x14)) / 1024);
            pos2->vx += *((s16 *)(((u8 *)scratch) + 0x10)) / 1024;
            pos2->vy += *((s16 *)(((u8 *)scratch) + 0x12)) / 1024;
            pos2->vz += *((s16 *)(((u8 *)scratch) + 0x14)) / 1024;
            if (collision_TestSegmentExtendingShortStartWithWork(
                    &pos1->vx, scratch + 12, scratch, (s16 *)(scratch + 6),
                    0x1F8002B8) == 0) {
                *pos1 = *(VECTOR *)(scratch + 12);
                scratch[8] = pos2->vx - pos1->vx;
                scratch[9] = pos2->vy - pos1->vy;
                scratch[10] = pos2->vz - pos1->vz;
                math_ScaleVectorInDirectionFrame(
                    scratch + 8, (s16 *)(scratch + 4), scratch + 8, 0,
                    (obj->move_kind == 0x15) ? 0x80 : 0x100);
                if (*((s16 *)(((u8 *)scratch) + 0x12)) >= (-0x7FF)) {
                    s32 mode = obj->move_kind;
                    s32 m = mode & 0xFFFF;
                    if (((((m != 8) && (m != 0x22)) &&
                          (((u32)(mode - 0x17)) >= 2)) &&
                         (m != 0xA)) &&
                        (obj->airborne_until_landing == 0)) {
                        scratch[9] = 0;
                    }
                }
                pos2->vx = pos1->vx + scratch[8];
                pos2->vy = pos1->vy + scratch[9];
                count++;
                pos2->vz = pos1->vz + scratch[10];
            } else {
                break;
            }
        } else {
            break;
        }
    } while (count < 4);
}

s32 fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow(
    Unk80101EC8Record *arg0, s32 *arg1) {
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
        s32 px;
        s32 a1_val;

        px = arg0->collision_pos.vx;
        a1_val = D_8008EB40[a0_idx][a1_idx];

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

    arg0->horizontal_probe_normal = out1;

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

void fighter_UpdateInputSteeredForwardMotion(Unk80101EC8Record *arg0) {
    u16 kind = arg0->move_kind;

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        s32 a2;
        u32 bits = arg0->pad_input.held;
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            a2 = D_8008EB40[a0][a1];

            if (g_fight_tracked_view_setting != 0 &&
                arg0->cpu_controlled == 0) {
                fighter_SetTurnDelta(arg0, arg0->orientation.vy + a2 / 4);
            } else {
                fighter_SetTurnDelta(arg0, arg0->opponent_bearing + a2);
            }
        } else {
            if (g_fight_tracked_view_setting != 0 &&
                arg0->cpu_controlled == 0) {
                arg0->remaining_turn = 0;
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
                s32 sub_result = (u16)arg0->forward_move_rate - a2;
                s32 div16 = arg0->size_scale;
                s16 new_14e;
                s32 tbl_val;
                s32 mult_res;
                s32 limit;

                arg0->forward_move_rate = sub_result;
                if (div16 < 0) {
                    div16 += 15;
                }
                div16 >>= 4;
                new_14e = sub_result + div16;
                arg0->forward_move_rate = new_14e;

                tbl_val = D_800A310C[D_8008DA08[arg0->character_data_index]];
                /* FAKE: the second read of arg0->size_scale is staged through
                 * the dead `sub_result`, as above; reading in place: score
                 * 11. */
                sub_result = arg0->size_scale;
                mult_res = sub_result * tbl_val;
                limit = (mult_res << 4) >> 12;

                if (limit < (s16)new_14e) {
                    arg0->forward_move_rate = limit;
                } else if ((s16)new_14e < 0) {
                    arg0->forward_move_rate = 0;
                }

                {
                    s32 var_13 =
                        arg0->forward_move_rate * arg0->move_frame_step;
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
        if (arg0->forward_move_rate > 0) {
            if (kind != 0x22) {
                arg0->remaining_turn = 0;
            }
            arg0->forward_move_rate = 0;
        }
    }
}

void fighter_HandleMissingSupportWhileDescending(Unk80101EC8Record *arg0) {
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
    if (arg0->base_velocity.vy <= 0) {
        return;
    }
    kind = arg0->move_kind;
    if (kind == 0x17 || kind == 0x18 || kind == 0xA ||
        arg0->airborne_until_landing != 0 || kind == 1 || kind == 0x28) {
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
            arg0->falling_backward = 0;
        } else {
            arg0->pending_move_event = 1;
            arg0->falling_backward = 1;
        }
        arg0->other->pending_move_event = 2;
        arg0->saved_collision_y = arg0->collision_pos.vy;
    } else if (kind == 0x11) {
        D_800A3769 = s1 < 0x400;
        g_fight_missing_support_fighter_index = arg0->index;
    } else {
        if (s1 < 0x400) {
            arg0->pending_move_event = 0x12;
            arg0->falling_backward = 0;
        } else {
            arg0->pending_move_event = 0x11;
            arg0->falling_backward = 1;
        }
        arg0->saved_collision_y = arg0->collision_pos.vy;
    }
    arg0->base_velocity.vx += dx_delta;
    arg0->base_velocity.vz += dz_delta;
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
    arg0->pose_blend_requests += 1;
    v = arg0->pose_blend_requests;
    if (v == 1) {
        arg0->pose_blend_progress = arg1;
        arg0->pose_blend_base_shift.x = 0;
        arg0->pose_blend_base_shift.z = 0;
    }
    arg0->pose_blend_step = arg1;
    if (arg0->use_root_offset_yaw == 0) {
        arg0->use_root_offset_yaw = 1;
        arg0->root_offset_yaw = arg0->opponent_bearing;
    }
}

void func_80023D08(Unk80101EC8Record *arg0) {
    fighter_RequestPoseBlend(arg0, 0x200);
}

void fighter_CacheDownwardProbeHit(Unk80101EC8Record *arg0) {
    if (arg0->base_velocity.vy < 0) {
        arg0->downward_probe_hit = 0;
        return;
    }
    ((VECTOR *)0x1F8001E0)->vx = arg0->collision_pos.vx;
    ((VECTOR *)0x1F8001E0)->vy = arg0->collision_pos.vy + 0x1F4;
    ((VECTOR *)0x1F8001E0)->vz = arg0->collision_pos.vz;
    arg0->downward_probe_hit = collision_TestSegmentExtendingShortStartWithWork(
        &arg0->collision_pos.vx, (s32 *)0x1F8001E0, (s32 *)0x1F8001B0,
        (s16 *)0x1F8001C0, 0x1F8002B8);
}

s32 fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface(
    Unk80101EC8Record *arg0) {
    s32 result;
    ((VECTOR *)0x1F8001E0)->vx = arg0->collision_pos.vx;
    ((VECTOR *)0x1F8001E0)->vy = arg0->collision_pos.vy + 5;
    ((VECTOR *)0x1F8001E0)->vz = arg0->collision_pos.vz;
    if (collision_TestSegmentExtendingShortStartWithWork(
            &arg0->collision_pos.vx, (s32 *)0x1F8001E0, (s32 *)0x1F8001B0,
            (s16 *)0x1F8001C0, 0x1F8002B8) != 0) {
        result = *(s16 *)0x1F8001C2 < -0x800;
    } else {
        result = 0;
    }
    return result;
}

void fighter_UpdateGroundYByMoveKind(Unk80101EC8Record *arg0) {
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
            arg0->ground_y = s1[1];
        }
    } else {
        arg0->ground_y = arg0->collision_pos.vy;
    }
}

/* [character_data_index][min(unk_272, 3)]: 4.12 scale of base_power into power
 * (fighter_Update).
 */
extern s16 D_8008E0BC[27][4];
/* [g_fighter_group_flags[character_data_index]][weapon_id]: the move command
 * fighter_Update hands object_SpawnScatteredAt. */
extern u8 D_8008D90C[2][8];
extern void fighter_DispatchArmedBoneVectorEventAndDisarm(Unk80101EC8Record *);

/* Per-frame update of character `arg0`'s record from this frame's pad input:
 * converts the pad bits, advances the move frame, runs the move script's
 * command list (cur_move / queued_move, motion_ResolveMoveData /
 * fighter_StartMoveScript), decodes two MotionFrames with motion_DecodeFrame
 * (at move_frame and at move_frame + 1, capped at motion_metadata[1] - 1, each
 * offset by *motion_entry, and by motion_metadata[1] in some states; state
 * move_kind 0x33 instead copies g_motion_frame_buffers[arg0][move_frame] into
 * both); when pose_blend_requests is 0 it mixes their root offsets into
 * root_offset by the 4.12 weight move_frame_frac, otherwise the first one's (by
 * the weight pose_blend_progress) with saved_motion_frame's less
 * pose_blend_base_shift (by 0x1000 - pose_blend_progress), integrates the
 * velocities (base_velocity / decaying_xz_velocity) into the position, resolves
 * it (fighter_ResolveMovement), then refreshes the bone points, the flags
 * frame_flags, unk_AE and weapon_hit_phases, the damping and the per-frame side
 * effects. */
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
     * 1): the remaining_turn clamp limit, the
     * opponent_bearing / orientation.vy angle gap (folded to 0..0x800), the
     * remaining_turn step and the -1/0/1 stick side. The target keeps all four
     * in $a1; one local per value: score 25. */
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
    if (g_fight_tracked_view_setting != 0 &&
        (arg0 == 0 || (arg0 == 1 && rec->cpu_controlled == 0))) {
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
    if (g_fight_mode_selector == 2 || g_fight_mode_selector == 5 ||
        g_fight_mode_selector == 3 ||
        (g_fight_mode_selector == 0 && D_800A385C != 0)) {
        rec->pad_input.pressed &= ~0x100;
    }
    if (rec->update_count < 2) {
        rec->pad_input.held = rec->pad_input.pressed = rec->pad_input.released =
            0;
        rec->pad_input.unheld = 0xFFFF;
    }
    rec->power =
        (rec->base_power * D_8008E0BC[rec->character_data_index]
                                     [rec->unk_272 >= 4 ? 3 : rec->unk_272]) >>
        12;
    if (rec->move_started == 1) {
        rec->move_started = 0;
    }
    motion_UpdateFrameStep(rec);
    frac = rec->move_frame_frac + rec->move_frame_step;
    rec->move_frame_frac = frac;
    if (frac >= 0x1000) {
        rec->move_frame_unchanged = 0;
        rec->move_frame += frac >> 12;
        rec->move_frame_frac &= 0xFFF;
    } else {
        rec->move_frame_unchanged = 1;
    }
    if (rec->pending_move_event >= 0) {
        /* the move-script record of cur_move's event_move_table_id is a u16
         * move-id table indexed by event code (here pending_move_event; 0xD,
         * 0x17/0x18 below) */
        u16 *table;

        if (rec->move_kind == 0xA || rec->move_kind == 0x17 ||
            rec->move_kind == 0x18) {
            rec->airborne_until_landing = 1;
        }
        rec->pose_blend_requests = 0;
        table = motion_ResolveMoveData(
            rec, rec->cur_move->event_move_table_id, &rec->move_is_common);
        fighter_StartMoveScript(
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
            fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow(
                rec, &var_4) != 0) {
            u16 *table;

            fighter_SetTurnDelta(rec, var_4);
            rec->pose_blend_requests = 0;
            table = motion_ResolveMoveData(
                rec, rec->cur_move->event_move_table_id, &rec->move_is_common);
            fighter_StartMoveScript(
                arg0,
                motion_ResolveMoveData(rec, table[0xD], &rec->move_is_common),
                rec->move_is_common);
            rec->base_velocity.vx = 0;
            rec->base_velocity.vy = 0;
            rec->base_velocity.vz = 0;
        }
    }
    if (rec->cur_move->last_move_frame < rec->move_frame) {
        if (rec->queued_move != NULL) {
            rec->move_is_common = rec->queued_move_is_common;
            if (rec->queued_move_command & 0x1000) {
                rec->borrow_other_motion = 1;
            }
            fighter_StartMoveScript(
                arg0, rec->queued_move, rec->queued_move_is_common);
        } else {
            if ((rec->cur_move->transition_commands[0] & 0x2000) &&
                g_fight_paired_move_initiator != arg0) {
                if (rec->cur_move->transition_commands[0] & 0x1000) {
                    rec->borrow_other_motion = 1;
                }
                fighter_StartMoveScript(
                    arg0,
                    motion_ResolveMoveData(
                        rec, rec->cur_move->transition_commands[1],
                        &rec->move_is_common),
                    rec->move_is_common);
            } else {
                if (rec->cur_move->unk_09 & 0x20) {
                    rec->borrow_other_motion = 1;
                }
                fighter_StartMoveScript(
                    arg0,
                    motion_ResolveMoveData(rec, rec->cur_move->fallback_move_id,
                                           &rec->move_is_common),
                    rec->move_is_common);
            }
        }
    }
    if (rec->queued_move != NULL) {
        if (rec->cur_move->last_command_frame < rec->move_frame) {
            if (rec->move_kind == 0x11) {
                g_fight_paired_move_start_pending = 1;
                g_fight_paired_move = rec->queued_move;
                D_800A381C = rec->queued_move_is_common;
                g_fight_paired_move_flags = rec->queued_move_command;
            } else {
                rec->move_is_common = rec->queued_move_is_common;
                if (rec->queued_move_command & 0x1000) {
                    rec->borrow_other_motion = 1;
                }
                fighter_StartMoveScript(
                    arg0, rec->queued_move, rec->queued_move_is_common);
            }
        }
    } else if (rec->cur_move->last_command_frame >= rec->move_frame) {
        ent = rec->cur_move->transition_commands;
        while ((var_7 = ent[0]) != 0) {
            if (!(var_7 & 0x8000) || ((ent[2] | ((u32)ent[3] << 16)) &
                                      (1 << rec->character_data_index))) {
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
                if (rec->move_kind == 0x11 &&
                    g_fight_paired_move_initiator == arg0) {
                    var_8 = 0;
                }
                if ((var_7 & 0xF) == 9) {
                    var_7 = (var_7 & 0xFFF0) | 3;
                    if (rec->other->move_kind != 6 &&
                        rec->other->move_frame <
                            rec->other->opponent_transition_start_frame) {
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
                        rec->queued_move_command = var_7;
                        rec->borrow_other_motion = 0;
                        rec->queued_move_is_common = var_5;
                    } else if (rec->move_kind == 0x11) {
                        g_fight_paired_move_start_pending = 1;
                        g_fight_paired_move = move;
                        g_fight_paired_move_flags = var_7;
                        D_800A381C = var_5;
                    } else {
                        rec->move_is_common = var_5;
                        fighter_StartMoveScript(arg0, move, var_5);
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
    if (rec->move_kind == 0xA || rec->airborne_until_landing != 0) {
        if (fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface(
                rec) != 0) {
            if (rec->saved_collision_y + 0xFA0 < rec->collision_pos.vy) {
                u16 *table;

                table = motion_ResolveMoveData(
                    rec, rec->cur_move->event_move_table_id,
                    &rec->move_is_common);
                fighter_StartMoveScript(
                    arg0,
                    motion_ResolveMoveData(
                        rec, table[rec->falling_backward != 0 ? 0x17 : 0x18],
                        &rec->move_is_common),
                    rec->move_is_common);
            } else if (rec->airborne_until_landing == 0) {
                fighter_StartMoveScript(
                    arg0,
                    motion_ResolveMoveData(rec, rec->cur_move->fallback_move_id,
                                           &rec->move_is_common),
                    rec->move_is_common);
            }
            rec->airborne_until_landing = 0;
        }
    } else if (rec->move_started != 0 &&
               (rec->prev_move_kind == 0x17 || rec->prev_move_kind == 0x18) &&
               !(rec->move_kind == 0x17 || rec->move_kind == 0x18) &&
               rec->saved_collision_y + 0xFA0 < rec->collision_pos.vy) {
        u16 *table;

        table = motion_ResolveMoveData(
            rec, rec->cur_move->event_move_table_id, &rec->move_is_common);
        fighter_StartMoveScript(
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
                temp = (rec->motion_metadata[2] >> 4) * 0x88;
                if (rec->remaining_turn > temp) {
                    rec->remaining_turn = temp;
                } else if (rec->remaining_turn < -temp) {
                    rec->remaining_turn = -temp;
                }
            }
        }
    }
    if (rec->move_frame_unchanged == 0 && rec->move_kind == 9 &&
        rec->opponent_turn_frame == rec->move_frame) {
        fighter_SetTurnDelta(rec, rec->opponent_bearing);
    }
    if (rec->move_started != 0 && rec->prev_move_kind != 0x18 &&
        rec->move_kind == 0x18) {
        rec->base_velocity.vx +=
            (Judge[rec->orientation.vy & 0xFFF] *
             D_8008DA94[rec->character_data_index]) >>
            12;
        rec->base_velocity.vz +=
            (Judge[(rec->orientation.vy + 0x400) & 0xFFF] *
             D_8008DA94[rec->character_data_index]) >>
            12;
        rec->base_velocity.vy += D_8008DA50[rec->character_data_index];
        rec->saved_collision_y = rec->collision_pos.vy;
    }
    if (rec->move_started != 0 && rec->prev_move_kind != 0x17 &&
        rec->move_kind == 0x17) {
        rec->base_velocity.vy += D_8008DAD8[rec->character_data_index];
        rec->saved_collision_y = rec->collision_pos.vy;
    }
    if (rec->move_started != 0 && rec->move_kind == 0x28) {
        s32 dv[3];
        s32 var_30[3];

        temp = (rec->opponent_bearing - rec->orientation.vy) & 0xFFF;
        if (temp >= 0x800) {
            temp = 0x1000 - temp;
        }
        var_30[0] = rec->other->bone_hit_points_1_3_centroid.x -
                    ((Judge[rec->opponent_bearing & 0xFFF] * temp) >> 14);
        var_30[1] = rec->other->bone_hit_points_1_3_centroid.y;
        var_30[2] =
            rec->other->bone_hit_points_1_3_centroid.z -
            ((Judge[(rec->opponent_bearing + 0x400) & 0xFFF] * temp) >> 14);
        rec->base_velocity.vy -= 0xFA;
        math_ComputeBallisticVelocityXZ((s32 *)&rec->bone_hit_point_0, var_30,
                                        rec->base_velocity.vy, 0x1F, dv);
        rec->base_velocity.vx += dv[0];
        rec->base_velocity.vz += dv[2];
        rec->airborne_until_landing = 1;
        rec->saved_collision_y = rec->collision_pos.vy;
    }
    if (rec->move_kind == 0x17 || rec->move_kind == 0x18 ||
        rec->move_kind == 0x28 || rec->move_kind == 0xA) {
        if (rec->move_frame == rec->cur_move->last_move_frame &&
            rec->base_velocity.vy > 0) {
            rec->move_frame = rec->cur_move->last_move_frame - 1;
        }
    }
    if (rec->move_started != 0 && rec->move_kind == 0x23) {
        rec->collision_pos.vy -= 0x898;
        rec->base_pos.y -= 0x898;
        rec->prev_root_offset.y += 0x898;
    }
    {
        /* FAKE: named intermediate: move_kind is read into `state` before
         * the move_started test, as the target loads it; with direct reads
         * the load follows the branch and jump threading adds 2 insns (score
         * 3); a u16 `state` also misses (Q129). */
        s32 state = rec->move_kind;

        if (rec->move_started != 0 && state == 6) {
            rec->variant_index = rec->default_variant_index;
        }
    }
    temp = (rec->remaining_turn * rec->move_frame_step) / 24576;
    rec->orientation.vy += temp;
    rec->remaining_turn -= temp;
    if (rec->move_started != 0) {
        if (rec->blend_into_move != 0) {
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
    if (next >= rec->motion_metadata[1]) {
        next = rec->motion_metadata[1] - 1;
    }
    if (rec->cur_move->unk_09 & 0x40) {
        var_18 = rec->move_is_common == 0
                     ? D_80101EC8[g_fight_paired_move_initiator].motion_slot + 1
                     : 0;
        if (rec->move_kind == 0x11 && g_fight_paired_move_initiator != arg0) {
            var_19 = rec->motion_metadata[1];
        } else {
            var_19 = 0;
        }
    } else {
        var_18 = rec->move_is_common == 0 ? rec->motion_slot + 1 : 0;
        if (rec->move_kind == 0x15 &&
            g_fighter_group_flags[rec->character_data_index] != 0) {
            var_19 = rec->motion_metadata[1];
        } else {
            var_19 = 0;
        }
    }
    var_15 = var_19 + *rec->motion_entry + rec->move_frame;
    var_17 = var_19 + *rec->motion_entry + next;
    rec->blend_src_frame_code = (var_18 << 14) | var_15;
    rec->blend_dst_frame_code = (var_18 << 14) | var_17;
    if (rec->move_kind == 0x33) {
        var_0[1] = g_motion_frame_buffers[arg0][rec->move_frame];
        var_0[0] = var_0[1];
    } else {
        motion_DecodeFrame(var_18, var_15, &var_0[0], (u16 *)0x1F8001B0);
        motion_DecodeFrame(var_18, var_17, &var_0[1], (u16 *)0x1F8001B0);
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
            rec->saved_motion_frame.unk_06, rec->saved_motion_frame.unk_08,
            rec->saved_motion_frame.unk_0A, &m1);
        math_RotMatrixZYXAngles(
            var_0[0].unk_06, var_0[0].unk_08, var_0[0].unk_0A, &m2);
        old_heading = ratan2(m1.m[0][0], m1.m[2][0]);
        new_heading = ratan2(m2.m[0][0], m2.m[2][0]);
        var_34 = new_heading - old_heading;

        rec->orientation.vy += var_34;
        camera_StartTurnEasingOnLargeHeadingChange(arg0, var_34);
    }
    motion_AdjustFighterPose(rec, &var_0[0], &var_0[1]);
    if (rec->move_kind == 0x11) {
        rec->root_offset_yaw = rec->orientation.vy;
    }
    if (rec->use_root_offset_yaw != 0) {
        var_12 = rec->root_offset_yaw;
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
    v[0].x = (v[0].x * rec->size_scale) >> 12;
    v[0].z = (v[0].z * rec->size_scale) >> 12;
    v[1].x = (v[1].x * rec->size_scale) >> 12;
    v[1].z = (v[1].z * rec->size_scale) >> 12;
    if (rec->move_kind != 8) {
        v[0].y = (v[0].y * rec->size_scale) >> 12;
        v[1].y = (v[1].y * rec->size_scale) >> 12;
    }
    if (rec->pose_blend_requests != 0) {
        var_0[1] = var_0[0];
        ang[1] = ang[0];
        var_0[0] = rec->saved_motion_frame;
        ang[0] = rec->saved_frame_offset_yaw;
        var_14 = rec->pose_blend_progress;
        rec->blend_dst_frame_code = rec->blend_src_frame_code;
        rec->blend_src_frame_code = rec->saved_motion_frame_id;
    } else {
        var_14 = rec->move_frame_frac;
    }
    var_22 = 0x1000 - var_14;
    rec->pose_interp_weight = var_14;
    if (rec->pose_blend_requests != 0) {
        Vec3i32 var_35;

        if (rec->move_started != 0) {
            rec->base_pos.x += rec->prev_root_offset.x - v[0].x;
            rec->base_pos.z += rec->prev_root_offset.z - v[0].z;
            rec->pose_blend_base_shift.x += rec->prev_root_offset.x - v[0].x;
            rec->pose_blend_base_shift.z += rec->prev_root_offset.z - v[0].z;
        }
        ang_q = ang[0] + 0x400;
        var_35.x = (var_0[0].unk_04 *
                    Judge[(var_0[0].unk_02 - ang_q + 0x400) & 0xFFF]) >>
                   12;
        var_35.y = -var_0[0].unk_00;
        var_35.z =
            (var_0[0].unk_04 * Judge[(var_0[0].unk_02 - ang_q) & 0xFFF]) >> 12;
        var_35.x = (var_35.x * rec->size_scale) >> 12;
        var_35.y = (var_35.y * rec->size_scale) >> 12;
        var_35.z = (var_35.z * rec->size_scale) >> 12;
        var_35.x -= rec->pose_blend_base_shift.x;
        var_35.z -= rec->pose_blend_base_shift.z;
        rec->root_offset.x = (var_35.x * var_22 + v[0].x * var_14) >> 12;
        rec->root_offset.y = (var_35.y * var_22 + v[0].y * var_14) >> 12;
        rec->root_offset.z = (var_35.z * var_22 + v[0].z * var_14) >> 12;
    } else {
        rec->root_offset.x = (v[0].x * var_22 + v[1].x * var_14) >> 12;
        rec->root_offset.y = (v[0].y * var_22 + v[1].y * var_14) >> 12;
        rec->root_offset.z = (v[0].z * var_22 + v[1].z * var_14) >> 12;
    }
    if (rec->move_started != 0 && rec->pose_blend_requests == 0) {
        rec->base_pos.x += rec->prev_root_offset.x - rec->root_offset.x;
        rec->base_pos.z += rec->prev_root_offset.z - rec->root_offset.z;
        if (rec->move_kind == 0x23 || rec->move_kind == 0xA) {
            rec->base_pos.y += rec->prev_root_offset.y - rec->root_offset.y;
        }
    }
    fighter_UpdateInputSteeredForwardMotion(rec);
    fighter_HandleMissingSupportWhileDescending(rec);
    if (rec->move_kind != 8 && rec->move_kind != 0x22) {
        rec->base_velocity.vy += 0x15;
    }
    {
        s32 state = rec->move_kind;

        if (state == 8 || state == 0x22) {
            rec->decaying_xz_velocity.vx -=
                rec->horizontal_probe_normal.vx / 256;
            rec->decaying_xz_velocity.vz -=
                rec->horizontal_probe_normal.vz / 256;
        }
    }
    if (rec->move_started == 0 || rec->move_kind != 0x28) {
        rec->base_velocity.vx =
            (rec->base_velocity.vx * rec->base_velocity_damping_x) >> 12;
        rec->base_velocity.vy =
            (rec->base_velocity.vy * rec->base_velocity_damping_y) >> 12;
        rec->base_velocity.vz =
            (rec->base_velocity.vz * rec->base_velocity_damping_z) >> 12;
    }
    if (rec->base_velocity.vx >= -15 && rec->base_velocity.vx <= 15) {
        rec->base_velocity.vx /= 2;
    }
    if (rec->base_velocity.vy >= -15 && rec->base_velocity.vy <= 15) {
        rec->base_velocity.vy /= 2;
    }
    if (rec->base_velocity.vz >= -15 && rec->base_velocity.vz <= 15) {
        rec->base_velocity.vz /= 2;
    }
    rec->base_pos.x += rec->base_velocity.vx;
    rec->base_pos.y += rec->base_velocity.vy;
    rec->base_pos.z += rec->base_velocity.vz;
    if ((rec->move_kind == 7 || rec->move_kind == 0xD) && rec->unk_B4 == 0) {
        temp = (rec->pad_input.held & 0x1000)
                   ? 1
                   : -((rec->pad_input.held & 0x4000) != 0);
        if (temp != 0) {
            rec->decaying_xz_velocity.vx /= 2;
            rec->decaying_xz_velocity.vz /= 2;
            rec->decaying_xz_velocity.vx +=
                (Judge[rec->opponent_bearing & 0xFFF] -
                 Judge[(rec->opponent_bearing + 0x400) & 0xFFF] * temp * 3) /
                128;
            rec->decaying_xz_velocity.vz +=
                (Judge[(rec->opponent_bearing + 0x400) & 0xFFF] +
                 Judge[rec->opponent_bearing & 0xFFF] * temp * 3) /
                128;
        }
    }
    rec->decaying_xz_velocity.vx =
        (rec->decaying_xz_velocity.vx * rec->decaying_xz_velocity_damping_x) >>
        12;
    if (rec->decaying_xz_velocity.vx >= -3 &&
        rec->decaying_xz_velocity.vx <= 3) {
        rec->decaying_xz_velocity.vx = 0;
    }
    rec->decaying_xz_velocity.vy =
        (rec->decaying_xz_velocity.vy * rec->decaying_xz_velocity_damping_y) >>
        12;
    if (rec->decaying_xz_velocity.vy >= -3 &&
        rec->decaying_xz_velocity.vy <= 3) {
        rec->decaying_xz_velocity.vy = 0;
    }
    rec->decaying_xz_velocity.vz =
        (rec->decaying_xz_velocity.vz * rec->decaying_xz_velocity_damping_z) >>
        12;
    if (rec->decaying_xz_velocity.vz >= -3 &&
        rec->decaying_xz_velocity.vz <= 3) {
        rec->decaying_xz_velocity.vz = 0;
    }
    if (rec->move_kind == 4 || rec->move_kind == 0x14 ||
        rec->character_roster_index == 0x1F) {
        rec->decaying_xz_velocity.vx = 0;
        rec->decaying_xz_velocity.vy = 0;
        rec->decaying_xz_velocity.vz = 0;
    } else {
        rec->base_pos.x += rec->decaying_xz_velocity.vx;
        rec->base_pos.y += rec->decaying_xz_velocity.vy;
        rec->base_pos.z += rec->decaying_xz_velocity.vz;
    }
    pos.vx = rec->base_pos.x + rec->root_offset.x;
    pos.vy = rec->base_pos.y;
    if (rec->move_kind == 8 || rec->move_kind == 0x22) {
        pos.vy += rec->root_offset.y;
    }
    pos.vz = rec->base_pos.z + rec->root_offset.z;
    rec->prev_collision_pos = rec->collision_pos;
    fighter_ResolveMovement(
        rec, &rec->collision_pos, &pos, &rec->base_velocity.vx);
    if (rec->collision_hit_tag != 7 && rec->collision_hit_tag != 0) {
        rec->collision_hit_class = ((0x2A >> rec->collision_hit_tag) ^ 1) & 1;
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
    rec->prev_root_offset = rec->root_offset;
    rec->camera_fighter_point.x = rec->world_pos.x;
    rec->camera_fighter_point.y = rec->base_pos.y - 0x384;
    rec->camera_fighter_point.z = rec->world_pos.z;
    fighter_UpdateGroundYByMoveKind(rec);
    slot_BlendTwoPosesIntoNodeMatrices(
        arg0, (u8 *)&var_0[0], (u8 *)&var_0[1], rec->pose_interp_weight,
        (MATRIX *)0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, (s32 *)&rec->world_pos, (s16 *)&rec->orientation, 0,
                  rec->ground_y);
    scratchpad_Restore();
    if (rec->variant_index == rec->unk_8E &&
        (rec->move_kind == 0x15 || rec->move_kind == 0x19 ||
         rec->move_kind == 0x1A || rec->move_kind == 0x16 ||
         rec->move_kind == 0x30 || rec->move_kind == 0x13 ||
         rec->move_kind == 0x31)) {
        rec->weapon_live_state = 0;
    } else {
        rec->weapon_live_state = rec->weapon_live_state != 0 ? 1 : 2;
    }
    rec->frame_flags = 0;
    if (rec->character_roster_index != 0x1F && rec->is_defeated == 0 &&
        rec->weapon_live_state != 0) {
        rec->frame_flags = 1;
    }
    if ((g_fight_mode_selector != 3 || arg0 != 1 ||
         g_fight_step_row_column == 4) &&
        (rec->weapon_id == 0 || rec->weapon_id == 1) &&
        (g_fight_mode_selector != 2 || D_800A389A != 0) &&
        g_fight_mode_selector != 5) {
        rec->frame_flags |= 2;
    }
    if (rec->variant_index == rec->object_variant_index &&
        rec->object_variant_enabled != 0) {
        if (func_8002798C(rec) != 0) {
            rec->wielded_object_live_state =
                rec->wielded_object_live_state != 0 ? 1 : 2;
            rec->frame_flags |= 4;
        } else {
            rec->wielded_object_live_state = 0;
        }
    } else if (
        ((rec->character_roster_index == 0x1D ||
          rec->character_roster_index == 0xE) &&
         (rec->move_kind == 2 || rec->move_kind == 0x1B ||
          rec->move_kind == 0x28 || rec->move_kind == 0x26) &&
         rec->weapon_hit_start_frames[1] < 0xFF && rec->unk_26C != 0 &&
         rec->move_frame >= rec->frame_window_start &&
         rec->move_frame <= rec->frame_window_end) ||
        (rec->character_data_index == 0xE && rec->move_kind == 0x11 &&
         g_fight_paired_move_initiator == arg0 &&
         rec->move_frame >= rec->frame_window_start &&
         rec->frame_window_end >= rec->move_frame)) {
        rec->wielded_object_live_state =
            rec->wielded_object_live_state != 0 ? 1 : 2;
        rec->frame_flags |= 4;
    } else {
        rec->wielded_object_live_state = 0;
    }
    if (g_fight_mode_selector != 3 || arg0 != 1 ||
        g_fight_step_row_column == 4) {
        if (g_fight_mode_selector == 3 && arg0 == 1 &&
            g_fight_step_row_column == 4) {
            if (rec->other->character_data_index == 1 ||
                rec->other->character_data_index == 3 ||
                rec->other->character_data_index == 4 ||
                rec->other->character_data_index == 9 ||
                rec->other->character_data_index == 0x11) {
                rec->frame_flags |= 8;
            }
        } else if ((g_fight_mode_selector != 2 || D_800A389A != 0) &&
                   g_fight_mode_selector != 5) {
            if (rec->character_data_index == 1 ||
                rec->character_data_index == 3 ||
                rec->character_data_index == 4 ||
                rec->character_data_index == 9 ||
                rec->character_data_index == 0x11) {
                rec->frame_flags |= 8;
            }
        } else {
            goto skip_62;
        }
        if (rec->move_kind != 4 && rec->move_kind != 0x14 &&
            rec->weapon_live_state == 0) {
            rec->frame_flags |= 0x10;
        }
        if (rec->carried_object_count > 0 &&
            rec->carried_object_kinds[0] == rec->wielded_object_kind &&
            rec->wielded_object_live_state == 0) {
            rec->frame_flags |= 0x20;
        }
    }
skip_62:
    if ((g_fight_mode_selector == 2 && D_800A389A == 0) ||
        g_fight_mode_selector == 5) {
        if ((rec->weapon_id == 0 || rec->weapon_id == 1) &&
            rec->move_kind != 4 && rec->move_kind != 0x14 &&
            rec->weapon_live_state == 0) {
            rec->frame_flags = (rec->frame_flags | 2) & ~0x10;
        }
        if (rec->carried_object_count > 0 &&
            rec->carried_object_kinds[0] == rec->wielded_object_kind &&
            rec->wielded_object_live_state == 0) {
            rec->frame_flags = (rec->frame_flags | 8) & ~0x20;
            node_QueueTableEntryNodePairUnderSlotNode(
                g_resource_kind_entry_ids[rec->wielded_object_kind],
                (arg0 * 2) | 1, 0);
        }
    }
    if (rec->frame_flags & 1) {
        node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
            rec->weapon_entry_id, arg0 * 2 + 0x8000, 0, 0);
    }
    if (rec->frame_flags & 4) {
        node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
            g_resource_kind_entry_ids[rec->wielded_object_kind],
            arg0 * 2 + 0x8001, 0, 0);
    }
    if (rec->frame_flags & 2) {
        node_QueueTableEntryNodePairUnderSlotNode(
            rec->weapon_entry_id, arg0 * 2, (rec->frame_flags >> 4) & 1);
    }
    if (rec->frame_flags & 8) {
        node_QueueTableEntryNodePairUnderSlotNode(
            g_resource_kind_entry_ids[rec->wielded_object_kind], (arg0 * 2) | 1,
            (rec->frame_flags >> 5) & 1);
    }
    rec->saved_motion_frame = var_0[0];
    rec->saved_frame_offset_yaw = ang[0];
    rec->saved_motion_frame_id = rec->blend_src_frame_code;
    if (rec->pose_blend_requests != 0) {
        if (rec->pose_blend_requests >= 2) {
            rec->pose_blend_requests = 1;
        }
        rec->pose_blend_progress += rec->pose_blend_step;
        if (rec->pose_blend_progress >= 0x1000) {
            rec->pose_blend_requests = 0;
            rec->saved_motion_frame = var_0[1];
            rec->saved_frame_offset_yaw = ang[1];
            rec->saved_motion_frame_id = rec->blend_dst_frame_code;
            rec->prev_root_offset = v[1];
        }
    }
    rec->opponent_bearing = ratan2(rec->other->world_pos.x - rec->world_pos.x,
                                   rec->other->world_pos.z - rec->world_pos.z);
    rec->unk_24C = rec->base_velocity;
    fighter_CacheDownwardProbeHit(rec);
    fighter_UpdateBoneWorldData(
        rec, SPAD->bone_hit_points[arg0], SPAD->weapon_points[arg0],
        SPAD->second_weapon_points[arg0]);
    if (rec->update_count == 1) {
        rec->prev_weapon_points[0] = SPAD->weapon_points[arg0][0];
    }
    if (rec->wielded_object_live_state == 2) {
        rec->wielded_object_live_state = 1;
        rec->prev_second_weapon_points[0] = SPAD->second_weapon_points[arg0][0];
        rec->prev_second_weapon_points[1] = SPAD->second_weapon_points[arg0][1];
    }
    if (rec->weapon_live_state == 2) {
        rec->weapon_live_state = 1;
        rec->prev_second_weapon_points[0] = SPAD->second_weapon_points[arg0][0];
        rec->prev_second_weapon_points[1] = SPAD->second_weapon_points[arg0][1];
    }
    if ((rec->weapon_id == 6 || rec->weapon_id == 7) &&
        (rec->frame_flags & 1)) {
        rec->opponent_elevation_origin = SPAD->weapon_points[arg0][0];
        rec->elevation_origin_from_weapon = 1;
    } else {
        rec->elevation_origin_from_weapon = 0;
    }
    rec->weapon_point_delta[0].vx =
        SPAD->weapon_points[arg0][0].x - rec->prev_weapon_points[0].x;
    rec->weapon_point_delta[0].vy =
        SPAD->weapon_points[arg0][0].y - rec->prev_weapon_points[0].y;
    rec->weapon_point_delta[0].vz =
        SPAD->weapon_points[arg0][0].z - rec->prev_weapon_points[0].z;
    rec->weapon_point_delta[1].vx = SPAD->second_weapon_points[arg0][0].x -
                                    rec->prev_second_weapon_points[0].x;
    rec->weapon_point_delta[1].vy = SPAD->second_weapon_points[arg0][0].y -
                                    rec->prev_second_weapon_points[0].y;
    rec->weapon_point_delta[1].vz = SPAD->second_weapon_points[arg0][0].z -
                                    rec->prev_second_weapon_points[0].z;
    rec->bone_triangle_cross_y_positive =
        fighter_IsBoneTriangleCrossYPositive(rec);
    var_20 = 0;
    if ((rec->move_kind == 2 || rec->move_kind == 0x1B ||
         rec->move_kind == 0x28 || rec->move_kind == 0x26) &&
        rec->attack_hit_armed != 0) {
        if ((rec->move_frame >= rec->weapon_hit_start_frames[0] &&
             rec->move_frame <= rec->weapon_hit_end_frames[0]) ||
            (rec->move_frame >= rec->weapon_hit_start_frames[1] &&
             rec->move_frame <= rec->weapon_hit_end_frames[1])) {
            var_20 = 1;
        }
    }
    rec->unk_AE = var_20;
    rec->weapon_hit_phases[0] = 0;
    rec->weapon_hit_phases[1] = 0;
    if (rec->attack_hit_armed != 0) {
        if (rec->move_kind == 2 || rec->move_kind == 0x1B ||
            rec->move_kind == 0x28 || rec->move_kind == 0x26) {
            rec->weapon_hit_phases[0] += 1;
            rec->weapon_hit_phases[1] += 1;
            if (rec->move_frame < rec->weapon_hit_start_frames[0]) {
                rec->weapon_hit_phases[0] += 2;
            } else if (rec->move_frame <= rec->weapon_hit_end_frames[0]) {
                rec->weapon_hit_phases[0] += 4;
            }
            if (rec->move_frame < rec->weapon_hit_start_frames[1]) {
                rec->weapon_hit_phases[1] += 2;
            } else if (rec->move_frame <= rec->weapon_hit_end_frames[1]) {
                rec->weapon_hit_phases[1] += 4;
            }
        }
    }
    if (rec->move_kind == 6) {
        rec->decaying_xz_velocity_damping_x = 0x600;
        rec->decaying_xz_velocity_damping_y = 0x600;
        rec->decaying_xz_velocity_damping_z = 0x600;
    } else if (rec->move_kind == 8) {
        rec->decaying_xz_velocity_damping_x = 0x100;
        rec->decaying_xz_velocity_damping_y = 0x100;
        rec->decaying_xz_velocity_damping_z = 0x100;
    } else {
        rec->decaying_xz_velocity_damping_x = 0xC00;
        rec->decaying_xz_velocity_damping_y = 0xC00;
        rec->decaying_xz_velocity_damping_z = 0xC00;
    }
    if (rec->downward_probe_hit != 0) {
        rec->base_velocity_damping_x = 0xD00;
        rec->base_velocity_damping_y = 0xFC0;
        rec->base_velocity_damping_z = 0xD00;
    } else {
        rec->base_velocity_damping_x = 0xFC0;
        rec->base_velocity_damping_y = 0xFC0;
        rec->base_velocity_damping_z = 0xFC0;
    }
    if (rec->is_defeated == 0 &&
        ((rec->move_started != 0 && rec->prev_move_kind != 4 &&
          rec->prev_move_kind != 0x14 &&
          (rec->move_kind == 4 || rec->move_kind == 0x14)) ||
         (rec->move_kind == 0x11 && arg0 != g_fight_paired_move_initiator &&
          rec->paired_move_hit_frame == rec->move_frame))) {
        MATRIX **var_37 = node_GetSlotMatrixPointerArray(arg0);
        if (g_fight_mode_selector != 0 ||
            g_fighter_group_flags[D_80101EC8[0].character_data_index] == 0 ||
            g_fight_stage_command_count != 1 ||
            arg0 != g_fight_stage_command_count) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                arg0, 0x2E, &rec->world_pos.x, 0);
        }
        if (rec->character_roster_index != 0x1F) {
            object_SpawnScatteredAt(
                rec,
                D_8008D90C[g_fighter_group_flags[rec->character_data_index]]
                          [rec->weapon_id],
                (Vec3i32 *)var_37[0x12]->t);
        }
        rec->is_defeated = 1;
        if (g_fight_followup_fighter_index == -1) {
            g_fight_followup_fighter_index = arg0 == 0;
        }
    }
    if (rec->move_started != 0 && rec->character_roster_index == 0x1B &&
        rec->move_kind == 0xB && rec->throw_charges_left == 0) {
        rec->pending_move_event = 0x1F;
    }
    if (rec->move_frame_unchanged == 0) {
        if (g_fight_mode_selector != 5 && g_fight_mode_selector != 2 &&
            g_fight_mode_selector != 3 &&
            (g_fight_mode_selector != 0 || D_800A385C == 0)) {
            if (rec->move_kind == 0xB &&
                rec->move_frame == rec->release_frame) {
                if (rec->character_roster_index == 0x1B) {
                    if (rec->throw_charges_left != 0) {
                        rec->throw_charges_left--;
                    }
                } else if (rec->unk_26C != 0 && object_ThrowQueuedKind(rec) ==
                                                    rec->wielded_object_kind) {
                    rec->object_variant_enabled = 0;
                }
            }
        }
        if (rec->move_kind == 6) {
            if (object_PickUpNearby(rec) == rec->wielded_object_kind) {
                rec->object_variant_enabled = rec->unk_26C;
            }
        } else if (rec->unk_26C != 0 &&
                   (rec->move_kind == 0xC || rec->move_kind == 0x2A) &&
                   rec->move_frame == rec->object_pickup_frame &&
                   object_PickUpNearby(rec) == rec->wielded_object_kind) {
            rec->object_variant_enabled = 1;
        }
        if (rec->move_kind == 0x12 && rec->move_frame == rec->release_frame &&
            ((0x78 >> rec->collision_hit_tag) & 1) && rec->unk_26C != 0) {
            fight_CreateBallisticRecordAndDispatchEvent(
                rec, ((0x18 >> rec->collision_hit_tag) & 1) ? 1 : 2);
        }
        if (rec->move_kind == 0x10 &&
            rec->move_frame == rec->weapon_reload_frame && rec->unk_26C != 0 &&
            rec->weapon_refills_left != 0) {
            rec->weapon_ammo = 0xA;
            rec->weapon_refills_left--;
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                arg0, 0x31, &rec->world_pos.x, 0);
        }
        if (rec->move_frame_unchanged == 0 &&
            (rec->move_kind != 0x11 || arg0 == g_fight_paired_move_initiator)) {
            rec->frame_flags |= 0x40;
            fighter_ProcessFrameEvents(rec);
        }
    }
    if (rec->unk_26C != 0) {
        rec->frame_flags |= 0x80;
    }
    if (rec->is_defeated != 0) {
        if (g_main_loop_handler_index != 0xD) {
            rec->draw_move_set = 4;
        }
    } else {
        rec->draw_move_set = 0;
    }
    if (rec->character_roster_index == 0x1F) {
        rec->draw_move_set = 4;
    }
    gpu_QueueSlotDrawMovesForSelectedSet(arg0, rec->draw_move_set);
    fighter_DispatchArmedBoneVectorEventAndDisarm(rec);
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
u8 g_camera_eye_blocked_flags[4] = {0, 0, 0, 0};
s32 g_camera_pitch_search_steps[2] = {0, 0};
s8 D_800A30FC = -1;
s8 D_800A30FD = -1;
