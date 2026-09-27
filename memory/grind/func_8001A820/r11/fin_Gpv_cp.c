extern u8 D_800A30F0[];
extern s32 D_800A30F4[];
typedef struct { s32 vx, vy, vz, pad; } CamVec;
/* Scratchpad ang area (0x1F800000) used by func_8001A820: the target yaw/roll
 * that D_800F6608's h12/h14 ease toward, the camera focus, the eye position
 * func_8001A538 computes, a fighter's head position, and the hit position,
 * surface normal and ang area func_80053614 is given (its 3rd/4th/5th
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
 * arg2/arg3 = the fighter records; caller func_8001E878). Resets the h30..h3C
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
void func_8001A820(s32 arg0, GameObj *arg1, s32 arg2, s32 arg3) {
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
    /* ang holds three values, all h10 (rot_x) quantities: the per-fighter
     * bisection angle (loop), the target angle max(pitch0, pitch1) clamped to
     * 0x80..0x1C0, and the final eased step. Ruling 11
     * (ordinary-c-judge-decidable.md); proof in
     * memory/grind/func_8001A820/ruling11.md. */
    s32 ang;
    s32 tgt;
    s32 step;
    s32 i, j;

    scr = (CamScratch *)0x1F800000;
    cam = &D_800F6608;
    cam->h30 = 0x64;
    cam->h32 = 0;
    cam->h34 = 0x64;
    cam->h38 = 0x64;
    cam->h3A = 0;
    cam->h3C = 0x64;
    dx = ((s32 *)arg1)[0] - ((s32 *)arg0)[0];
    dy = ((s32 *)arg1)[1] - ((s32 *)arg0)[1];
    dz = ((s32 *)arg1)[2] - ((s32 *)arg0)[2];
    if (D_800A3690 == 0) {
        scr->focus.vx = (((s32 *)arg0)[0] + ((s32 *)arg1)[0]) / 2;
        scr->focus.vy = (((s32 *)arg0)[1] + ((s32 *)arg1)[1]) / 2;
        scr->focus.vz = (((s32 *)arg0)[2] + ((s32 *)arg1)[2]) / 2;
    } else {
        scr->focus = *(CamVec *)arg0;
    }
    cam->w0 += (scr->focus.vx - cam->w0) / 4;
    cam->w4 += (scr->focus.vy - cam->w4) / 4;
    cam->w8 += (scr->focus.vz - cam->w8) / 4;

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
        dist = (u32)*(&D_8008D118 + dist_sq) >> 3;
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
            s32 tbl = *(&D_8008D118 + (dist_sq >> sh));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)sh >> 1));
        }
    }
    dist <<= shift;
    q = 0x2000000U / (dist + 0x4000) + 0x400;
    if (*(u16 *)(arg2 + 0x6A) == 0x13 || *(u16 *)(arg2 + 0x6A) == 0x1B || *(u16 *)(arg2 + 0x6A) == 0x30 ||
        *(u16 *)(arg3 + 0x6A) == 0x13 || *(u16 *)(arg3 + 0x6A) == 0x1B || *(u16 *)(arg3 + 0x6A) == 0x30) {
        q += 0x1000;
    }
    zoom = ((dist + q) << 7) / 100;
    if (dy < 0) {
        dy = -dy;
    }
    zoom += dy;
    if (*(u16 *)(arg2 + 0x6A) == 0xF || *(u16 *)(arg2 + 0x6A) == 0x1C || *(u16 *)(arg2 + 0x6A) == 0x1D ||
        *(u16 *)(arg2 + 0x6A) == 0x1E || *(u16 *)(arg2 + 0x6A) == 0x1F || *(u16 *)(arg2 + 0x6A) == 0x20 ||
        *(u16 *)(arg2 + 0x6A) == 0x21) {
        zoom = 0xBB8;
    }
    cam->w18 += (zoom - cam->w18) / 12;
    if (!(*(u16 *)(arg2 + 0x6A) == 0xF || *(u16 *)(arg2 + 0x6A) == 0x1C || *(u16 *)(arg2 + 0x6A) == 0x1D ||
          *(u16 *)(arg2 + 0x6A) == 0x1E || *(u16 *)(arg2 + 0x6A) == 0x1F || *(u16 *)(arg2 + 0x6A) == 0x20 ||
          *(u16 *)(arg2 + 0x6A) == 0x21) && cam->w18 < 0x1770) {
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

    if (*(u16 *)(arg2 + 0x6A) == 0x11) {
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

        ang = base_pitch;
        cam->h10 = ang;
        func_8001A538((s32 *)cam, (s32 *)&scr->eye);
        if (p != 0) {
            scr->head = *(CamVec *)(arg3 + 0xB8);
        } else {
            scr->head = *(CamVec *)(arg2 + 0xB8);
        }
        scr->head.vy -= 0xC8;
        if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, (s32 *)scr->nrm, (s32)&scr->unk60) &&
            scr->nrm[1] < -0x320) {
            func_8001A67C((s16 *)((u8 *)cam + 0x30 + p * 8), (s32 *)&scr->eye, scr->hit);
            if (D_800A30F0[p]) {
                D_800A30F4[p] += 0x20;
            } else {
                D_800A30F4[p] = 0x10;
                ang--;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x200) {
                D_800A30F4[p] = 0x200;
            }
            hi = ang + D_800A30F4[p] / 8;
            /* FAKE: cancellation pair (semantically-null pair family, owner ruling
             * 2026-08-18, no-new-park-categories.md), mechanism: global.c
             * allocno_compare priority -- the pair adds references to `hi`
             * (allocno_n_refs 13 -> 21, pri 11142 -> 23333 against ang's 13253), so
             * the bounds are allocated before `ang` and take $s0 and `ang` $s1, as
             * in the target; combine folds the pair to nothing (576/576 insns).
             * lever-exhaustion: memory/grind/func_8001A820/ruling11.md § `hi`/`lo`. */
            hi++;
            hi--;
            for (i = 0; i < 2; i++) {
                s32 d = (ang - hi) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = hi + d / 2;
                func_8001A538((s32 *)cam, (s32 *)&scr->eye);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, (s32 *)scr->nrm,
                                  (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    ang = cam->h10;
                } else {
                    hi = cam->h10;
                }
            }
            ang = hi;
            D_800A30F0[p] = 1;
        } else {
            if (D_800A30F0[p]) {
                D_800A30F4[p] = 0x10;
                ang++;
            } else {
                D_800A30F4[p] += 0x10;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x100) {
                D_800A30F4[p] = 0x100;
            }
            lo = ang - D_800A30F4[p] / 8;
            for (j = 0; j < 2; j++) {
                s32 d = (ang - lo) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = lo + d / 2;
                func_8001A538((s32 *)cam, (s32 *)&scr->eye);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->hit, (s32 *)scr->nrm,
                                  (s32)&scr->unk60) &&
                    scr->nrm[1] < -0x320) {
                    func_8001A67C((s16 *)((u8 *)cam + 0x30 + p * 8), (s32 *)&scr->eye, scr->hit);
                    lo = cam->h10;
                } else {
                    ang = cam->h10;
                }
            }
            D_800A30F0[p] = 0;
        }
        if (p != 0) {
            pitch1 = ang;
        } else {
            pitch0 = ang;
        }
    }
    tgt = (pitch0 < pitch1) ? pitch1 : pitch0;
    if (tgt < 0x80) {
        tgt = 0x80;
    }
    if (tgt > 0x1C0) {
        tgt = 0x1C0;
    }
    cam->h10 = base_pitch;
    step = math_SignExt12Div(tgt - base_pitch, 8);
    cam->h10 += step;
    ang++;
    ang--;
}
