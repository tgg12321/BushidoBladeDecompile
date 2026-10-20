/* 16 game functions: the collision grid walk (collision_WalkSegmentGrid) and
 * its per-cell tests, accessors and setters (collision_TestSegmentWithWork,
 * collision_GetHit, func_80053754, func_80053E9C, ...), then the
 * motion-sequence loader motion_StartPlayback and its wrappers func_80054884 /
 * func_800548DC. .text 0x80052D00 (ROM 0x43500). Start boundary: SEAM (owner
 * ruling 2026-10-17; Codex seam #72): the statics
 * D_800A33F0..g_collision_hit_tag are used only here and open the old 3AB48
 * .sbss group in address order; calls run from here into 3AB48.c's GTE helpers
 * (gte_SumSquares3, gte_ReadIR1IR2Sra2, func_80052C4C), back only once
 * (func_8004DDB4 -> func_80054440); three NOPs follow gte_ReadIR1IR2Sra2. A
 * control cut inside this group (before func_80053754) does not byte-match: the
 * shared statics lose gp access. End: before motion_UpdatePlayback (seam #85,
 * moved there from motion_StartPlayback), so motion_StartPlayback's printf
 * string D_80015840 is this file's .rodata (0x80015840+0x1C) and 4510C.o's
 * starts with func_80058580's tables at offset 0. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s32 D_800A33F0;
static Work_80053E9C *D_800A33F4;
static u16 g_collision_hit_tag;
static s32 D_800A33FC; /* not named by any code or data: size from the gap */

extern s32 collision_GetHit(s32 *, s16 *);

/* Walks the 32x32 grid of 2000-unit cells (origin -32000) along the XZ
 * segment from the start point (+0x8/+0x10) to the end point (+0x18/+0x20),
 * DDA style: +0x88/+0x8C are the current and end cells, +0x70/+0x74 the
 * major/minor extents (exchanged, with +0x60/+0x64 and +0x68/+0x6C, when
 * +0x74 > +0x70), +0x7C the 4.12 slope and
 * +0x90 the major-axis steps left. Each cell crossed goes to the per-cell
 * test at +0x5C (func_80053E9C or func_80053754) until one reports a hit;
 * collision_GetHit then reads the result back out. */
s32 collision_WalkSegmentGrid(s32 *arg0, s16 *arg1) {
    s32 var_0;
    s32 var_1;
    s32 var_2;

    D_800A33F4->unk60 = D_800A33F4->unk8.vx + 32000;
    D_800A33F4->unk64 = D_800A33F4->unk8.vz + 32000;
    D_800A33F4->unk68 = D_800A33F4->unk18.vx + 32000;
    D_800A33F4->unk0 = 0x7FFFFFFF;
    D_800A33F4->unk6C = D_800A33F4->unk18.vz + 32000;
    D_800A33F4->unk70 = D_800A33F4->unk68 - D_800A33F4->unk60;
    D_800A33F4->unk88.x = D_800A33F4->unk60 / 2000;
    D_800A33F4->unk88.z = D_800A33F4->unk64 / 2000;
    D_800A33F4->unk8C.x = D_800A33F4->unk68 / 2000;
    D_800A33F4->unk8C.z = D_800A33F4->unk6C / 2000;
    D_800A33F4->unk74 = D_800A33F4->unk6C - D_800A33F4->unk64;
    if (D_800A33F4->unk88.x == D_800A33F4->unk8C.x &&
        D_800A33F4->unk88.z == D_800A33F4->unk8C.z) {
        if (D_800A33F4->unk8.vx == D_800A33F4->unk18.vx &&
            D_800A33F4->unk8.vy == D_800A33F4->unk18.vy &&
            D_800A33F4->unk8.vz == D_800A33F4->unk18.vz) {
            return 0;
        }
        D_800A33F4->unk5C(D_800A33F4->unk88.x, D_800A33F4->unk88.z);
    } else {
        D_800A33F4->unk80 = D_800A33F4->unk88.x * 2000 + 1000;
        D_800A33F4->unk84 = D_800A33F4->unk88.z * 2000 + 1000;
        D_800A33F4->unk60 -= D_800A33F4->unk80;
        D_800A33F4->unk64 -= D_800A33F4->unk84;
        D_800A33F4->unk68 -= D_800A33F4->unk80;
        D_800A33F4->unk6C -= D_800A33F4->unk84;
        if (D_800A33F4->unk70 < 0) {
            var_0 = -1;
            D_800A33F4->unk70 = -D_800A33F4->unk70;
            D_800A33F4->unk60 = -D_800A33F4->unk60;
            D_800A33F4->unk68 = -D_800A33F4->unk68;
        } else {
            var_0 = 1;
        }
        if (D_800A33F4->unk74 < 0) {
            var_1 = -1;
            D_800A33F4->unk74 = -D_800A33F4->unk74;
            D_800A33F4->unk64 = -D_800A33F4->unk64;
            D_800A33F4->unk6C = -D_800A33F4->unk6C;
        } else {
            var_1 = 1;
        }
        if (D_800A33F4->unk74 > D_800A33F4->unk70) {
            var_2 = 1;
            D_800A33F4->unk80 = D_800A33F4->unk70;
            D_800A33F4->unk70 = D_800A33F4->unk74;
            D_800A33F4->unk74 = D_800A33F4->unk80;
            D_800A33F4->unk80 = D_800A33F4->unk60;
            D_800A33F4->unk60 = D_800A33F4->unk64;
            D_800A33F4->unk64 = D_800A33F4->unk80;
            D_800A33F4->unk80 = D_800A33F4->unk68;
            D_800A33F4->unk68 = D_800A33F4->unk6C;
            D_800A33F4->unk6C = D_800A33F4->unk80;
        } else {
            var_2 = 0;
        }
        D_800A33F4->unk68 += 1000;
        D_800A33F4->unk60 += 1000;
        D_800A33F4->unk64 += 1000;
        D_800A33F4->unk90 =
            D_800A33F4->unk68 / 2000 - D_800A33F4->unk60 / 2000 + 1;
        D_800A33F4->unk7C = (D_800A33F4->unk74 << 12) / D_800A33F4->unk70;
        D_800A33F4->unk64 -= (D_800A33F4->unk60 * D_800A33F4->unk7C) >> 12;
        D_800A33F4->unk78 = (D_800A33F4->unk7C * 2000) >> 12;
        while (--D_800A33F4->unk90 != -1) {
            if ((D_800A33F4->unk80 = D_800A33F4->unk5C(
                     D_800A33F4->unk88.x, D_800A33F4->unk88.z)) != 0) {
                break;
            }
            D_800A33F4->unk64 %= 2000;
            D_800A33F4->unk64 += D_800A33F4->unk78;
            if (D_800A33F4->unk90 == 0) {
                break;
            }
            if (D_800A33F4->unk64 > 2000) {
                if (var_2) {
                    if (var_0 < 0) {
                        D_800A33F4->unk88.x--;
                    } else {
                        D_800A33F4->unk88.x++;
                    }
                } else {
                    if (var_1 < 0) {
                        D_800A33F4->unk88.z--;
                    } else {
                        D_800A33F4->unk88.z++;
                    }
                }
                if ((D_800A33F4->unk80 = D_800A33F4->unk5C(
                         D_800A33F4->unk88.x, D_800A33F4->unk88.z)) != 0) {
                    break;
                }
            }
            if (var_2) {
                if (var_1 < 0) {
                    D_800A33F4->unk88.z--;
                } else {
                    D_800A33F4->unk88.z++;
                }
            } else {
                if (var_0 < 0) {
                    D_800A33F4->unk88.x--;
                } else {
                    D_800A33F4->unk88.x++;
                }
            }
        }
        if (D_800A33F4->unk80 == 0 &&
            (D_800A33F4->unk88.x != D_800A33F4->unk8C.x ||
             D_800A33F4->unk88.z != D_800A33F4->unk8C.z)) {
            D_800A33F4->unk5C(D_800A33F4->unk8C.x, D_800A33F4->unk8C.z);
        }
    }
    return collision_GetHit(arg0, arg1);
}

extern s32 collision_WalkSegmentGrid(s32 *, s16 *);
extern s32 func_80053754();
extern s32 func_80053E9C();
extern Work_80053E9C D_800EF9F8;

void func_80053304(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    if (gte_SumSquares3(D_800A33F4->unk18.vx - D_800A33F4->unk8.vx,
                        D_800A33F4->unk18.vy - D_800A33F4->unk8.vy,
                        D_800A33F4->unk18.vz - D_800A33F4->unk8.vz) <= 0x9C3F) {
        hi0 = D_800A33F4->unk18.vx;
        a = D_800A33F4->unk8.vx;
        hi1 = D_800A33F4->unk18.vy;
        b = D_800A33F4->unk8.vy;
        D_800A33F4->unk5C = func_80053754;
        hi2 = D_800A33F4->unk18.vz;
        D_800A33F4->unk8.vx = a - ((hi0 - a) << 1);
        D_800A33F4->unk8.vy = b - ((hi1 - b) << 1);
        c = D_800A33F4->unk8.vz;
        D_800A33F4->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        D_800A33F4->unk5C = func_80053E9C;
    }
    collision_WalkSegmentGrid(arg2, arg3);
}

s32 func_8005344C(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3, s32 arg4) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = (Work_80053E9C *)arg4;
    D_800A33F4->unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    if (gte_SumSquares3(D_800A33F4->unk18.vx - D_800A33F4->unk8.vx,
                        D_800A33F4->unk18.vy - D_800A33F4->unk8.vy,
                        D_800A33F4->unk18.vz - D_800A33F4->unk8.vz) <= 0x9C3F) {
        hi0 = D_800A33F4->unk18.vx;
        a = D_800A33F4->unk8.vx;
        hi1 = D_800A33F4->unk18.vy;
        b = D_800A33F4->unk8.vy;
        D_800A33F4->unk5C = func_80053754;
        hi2 = D_800A33F4->unk18.vz;
        D_800A33F4->unk8.vx = a - ((hi0 - a) << 1);
        D_800A33F4->unk8.vy = b - ((hi1 - b) << 1);
        c = D_800A33F4->unk8.vz;
        D_800A33F4->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        D_800A33F4->unk5C = func_80053E9C;
    }
    return collision_WalkSegmentGrid(arg2, arg3);
}

s32 collision_TestSegment(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3) {
    D_800A33F4 = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    D_800A33F4->unk5C = func_80053E9C;
    return collision_WalkSegmentGrid(arg2, arg3);
}

s32 collision_TestSegmentWithWork(
    s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3, s32 arg4) {
    D_800A33F4 = (Work_80053E9C *)arg4;
    D_800A33F4->unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    D_800A33F4->unk5C = func_80053E9C;
    return collision_WalkSegmentGrid(arg2, arg3);
}

s32 collision_GetHit(s32 *arg0, s16 *arg1) {
    Work_80053E9C *p = D_800A33F4;
    s32 t;
    if (p->unk0 != 0x7FFFFFFF) {
        t = (p->unk48 * 0x7D0) - 0x7D00;
        arg0[0] = p->unk38 + t;
        arg0[1] = p->unk3C;
        t = (p->unk4A * 0x7D0) - 0x7D00;
        arg0[2] = p->unk40 + t;
        arg1[0] = p->unk28 >> 2;
        arg1[1] = p->unk2C >> 2;
        arg1[2] = p->unk30 >> 2;
        g_collision_hit_tag = p->unk4;
        return 1;
    }
    return 0;
}

extern void func_80052C4C(s32, s32, s32, s32);
extern void gte_ReadIR1IR2Sra2(s32 *, s32 *);

s32 func_80053754(s32 arg0, s32 arg1) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 var_6;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    D_800A33F4->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (D_800A33F4->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + D_800A33F4->unkE0);
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    D_800A33F4->unk4C = D_800A33F4->unk8.vx - x;
    D_800A33F4->unk4E = D_800A33F4->unk8.vy;
    D_800A33F4->unk50 = D_800A33F4->unk8.vz - z;
    D_800A33F4->unk54 = D_800A33F4->unk18.vx - x;
    D_800A33F4->unk56 = D_800A33F4->unk18.vy;
    D_800A33F4->unk58 = D_800A33F4->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        D_800A33F4->unkD0 = *data;
        data++;
        D_800A33F4->unkD4 = *data;
        data++;
        D_800A33F4->unkD8 = *data;
        data++;
        D_800A33F4->unkDC = (u16)*data;
        data++;
        D_800A33F4->unkDC = (*data << 16) | D_800A33F4->unkDC;
        data++;
        D_800A33F4->unkE4 =
            D_800A33F4->unkD0 * D_800A33F4->unk4C +
            D_800A33F4->unkD4 * D_800A33F4->unk4E +
            D_800A33F4->unkD8 * D_800A33F4->unk50 + D_800A33F4->unkDC;
        D_800A33F4->unkE8 =
            D_800A33F4->unkD0 * D_800A33F4->unk54 +
            D_800A33F4->unkD4 * D_800A33F4->unk56 +
            D_800A33F4->unkD8 * D_800A33F4->unk58 + D_800A33F4->unkDC;

        D_800A33F4->unkE4 =
            (D_800A33F4->unkE4 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE4 < 0 ? -D_800A33F4->unkE4 : D_800A33F4->unkE4) >>
             10);
        D_800A33F4->unkE8 =
            (D_800A33F4->unkE8 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE8 < 0 ? -D_800A33F4->unkE8 : D_800A33F4->unkE8) >>
             10);

        if (D_800A33F4->unkE4 >= 0 && D_800A33F4->unkE8 < 0) {
            D_800A33F4->unkE0 = D_800A33F4->unkE4 - D_800A33F4->unkE8;
            D_800A33F4->unkE4 *= 2;
            D_800A33F4->unkA8 = (D_800A33F4->unk54 - D_800A33F4->unk4C) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkAC = (D_800A33F4->unk56 - D_800A33F4->unk4E) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkB0 = (D_800A33F4->unk58 - D_800A33F4->unk50) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkA8 =
                (D_800A33F4->unkA8 < 0 ? -1 : 1) *
                ((D_800A33F4->unkA8 >= 0 ? D_800A33F4->unkA8 + 1
                                         : -D_800A33F4->unkA8 + 1) >>
                 1);
            D_800A33F4->unkAC =
                (D_800A33F4->unkAC < 0 ? -1 : 1) *
                ((D_800A33F4->unkAC >= 0 ? D_800A33F4->unkAC + 1
                                         : -D_800A33F4->unkAC + 1) >>
                 1);
            D_800A33F4->unkB0 =
                (D_800A33F4->unkB0 < 0 ? -1 : 1) *
                ((D_800A33F4->unkB0 >= 0 ? D_800A33F4->unkB0 + 1
                                         : -D_800A33F4->unkB0 + 1) >>
                 1);
            D_800A33F4->unkA8 += D_800A33F4->unk4C;
            D_800A33F4->unkAC += D_800A33F4->unk4E;
            D_800A33F4->unkB0 += D_800A33F4->unk50;
            D_800A33F4->unkE0 = *data;
            data++;
            D_800A33F4->unkE4 = *data;
            data++;
            D_800A33F4->unkE8 = *data;
            data++;
            D_800A33F4->unk9C = *data;
            data++;
            D_800A33F4->unkA0 = *data;
            data++;
            D_800A33F4->unkA4 = *data;
            data++;
            D_800A33F4->unkC4 =
                ((D_800A33F4->unkA8 - D_800A33F4->unkE0) * D_800A33F4->unk9C +
                 (D_800A33F4->unkAC - D_800A33F4->unkE4) * D_800A33F4->unkA0 +
                 (D_800A33F4->unkB0 - D_800A33F4->unkE8) * D_800A33F4->unkA4) >>
                14;
            D_800A33F4->unk9C = *data;
            data++;
            D_800A33F4->unkA0 = *data;
            data++;
            D_800A33F4->unkA4 = *data;
            data++;
            D_800A33F4->unkC8 =
                ((D_800A33F4->unkA8 - D_800A33F4->unkE0) * D_800A33F4->unk9C +
                 (D_800A33F4->unkAC - D_800A33F4->unkE4) * D_800A33F4->unkA0 +
                 (D_800A33F4->unkB0 - D_800A33F4->unkE8) * D_800A33F4->unkA4) >>
                14;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            D_800A33F4->unkCC = n >> 8;
            n &= 0xFF;
            D_800A33F4->unkB4 = *data;
            data++;
            y = *data;
            data++;
            D_800A33F4->unkE0 = 1;
            D_800A33F4->unkB8 = y;
            while (--n != -1) {
                D_800A33F4->unkBC = *data;
                data++;
                D_800A33F4->unkC0 = *data;
                data++;
                if ((D_800A33F4->unkC4 - D_800A33F4->unkB4) *
                            (D_800A33F4->unkC0 - D_800A33F4->unkB8) -
                        (D_800A33F4->unkC8 - D_800A33F4->unkB8) *
                            (D_800A33F4->unkBC - D_800A33F4->unkB4) >
                    0) {
                    D_800A33F4->unkE0 = 0;
                    break;
                }
                D_800A33F4->unkB4 = D_800A33F4->unkBC;
                D_800A33F4->unkB8 = D_800A33F4->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (D_800A33F4->unkE0 != 0) {
                if ((D_800A33F4->unkE0 = gte_SumSquares3(
                         D_800A33F4->unkA8 - D_800A33F4->unk4C,
                         D_800A33F4->unkAC - D_800A33F4->unk4E,
                         D_800A33F4->unkB0 - D_800A33F4->unk50)) <
                    D_800A33F4->unk0) {
                    D_800A33F4->unk48 = arg0;
                    D_800A33F4->unk4A = arg1;
                    D_800A33F4->unk38 = D_800A33F4->unkA8;
                    D_800A33F4->unk3C = D_800A33F4->unkAC;
                    D_800A33F4->unk40 = D_800A33F4->unkB0;
                    D_800A33F4->unk28 = D_800A33F4->unkD0;
                    D_800A33F4->unk2C = D_800A33F4->unkD4;
                    D_800A33F4->unk30 = D_800A33F4->unkD8;
                    D_800A33F4->unk34 = D_800A33F4->unkDC;
                    D_800A33F4->unk0 = D_800A33F4->unkE0;
                    D_800A33F4->unk4 = D_800A33F4->unkCC;
                }
            }
        } else {
            data += 9;
            n = *data;
            data++;
            n &= 0xFF;
            data += (n + 1) * 2;
        }
    }
    return D_800A33F4->unk0 != 0x7FFFFFFF;
}

s32 func_80053E9C(s32 arg0, s32 arg1) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 var_6;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    D_800A33F4->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (D_800A33F4->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + D_800A33F4->unkE0);
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    D_800A33F4->unk4C = D_800A33F4->unk8.vx - x;
    D_800A33F4->unk4E = D_800A33F4->unk8.vy;
    D_800A33F4->unk50 = D_800A33F4->unk8.vz - z;
    D_800A33F4->unk54 = D_800A33F4->unk18.vx - x;
    D_800A33F4->unk56 = D_800A33F4->unk18.vy;
    D_800A33F4->unk58 = D_800A33F4->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        D_800A33F4->unkD0 = *data;
        data++;
        D_800A33F4->unkD4 = *data;
        data++;
        D_800A33F4->unkD8 = *data;
        data++;
        D_800A33F4->unkDC = (u16)*data;
        data++;
        D_800A33F4->unkDC = (*data << 16) | D_800A33F4->unkDC;
        data++;
        D_800A33F4->unkE4 =
            D_800A33F4->unkD0 * D_800A33F4->unk4C +
            D_800A33F4->unkD4 * D_800A33F4->unk4E +
            D_800A33F4->unkD8 * D_800A33F4->unk50 + D_800A33F4->unkDC;
        D_800A33F4->unkE8 =
            D_800A33F4->unkD0 * D_800A33F4->unk54 +
            D_800A33F4->unkD4 * D_800A33F4->unk56 +
            D_800A33F4->unkD8 * D_800A33F4->unk58 + D_800A33F4->unkDC;

        D_800A33F4->unkE4 =
            (D_800A33F4->unkE4 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE4 < 0 ? -D_800A33F4->unkE4 : D_800A33F4->unkE4) >>
             14);
        D_800A33F4->unkE8 =
            (D_800A33F4->unkE8 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE8 < 0 ? -D_800A33F4->unkE8 : D_800A33F4->unkE8) >>
             14);

        if (D_800A33F4->unkE4 >= 0 && D_800A33F4->unkE8 < 0) {
            D_800A33F4->unkE0 = D_800A33F4->unkE4 - D_800A33F4->unkE8;
            D_800A33F4->unkA8 = (D_800A33F4->unk54 - D_800A33F4->unk4C) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk4C;
            D_800A33F4->unkAC = (D_800A33F4->unk56 - D_800A33F4->unk4E) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk4E;
            D_800A33F4->unkB0 = (D_800A33F4->unk58 - D_800A33F4->unk50) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk50;
            func_80052C4C((s32)data, D_800A33F4->unkA8, D_800A33F4->unkAC,
                          D_800A33F4->unkB0);
            data += 9;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            D_800A33F4->unkCC = n >> 8;
            n &= 0xFF;
            D_800A33F4->unkB4 = *data;
            data++;
            y = *data;
            data++;
            D_800A33F4->unkE0 = 1;
            D_800A33F4->unkB8 = y;
            gte_ReadIR1IR2Sra2(&D_800A33F4->unkC4, &D_800A33F4->unkC8);
            while (--n != -1) {
                D_800A33F4->unkBC = *data;
                data++;
                D_800A33F4->unkC0 = *data;
                data++;
                if ((D_800A33F4->unkC4 - D_800A33F4->unkB4) *
                            (D_800A33F4->unkC0 - D_800A33F4->unkB8) -
                        (D_800A33F4->unkC8 - D_800A33F4->unkB8) *
                            (D_800A33F4->unkBC - D_800A33F4->unkB4) >
                    0) {
                    D_800A33F4->unkE0 = 0;
                    break;
                }
                D_800A33F4->unkB4 = D_800A33F4->unkBC;
                D_800A33F4->unkB8 = D_800A33F4->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (D_800A33F4->unkE0 != 0) {
                if ((D_800A33F4->unkE0 = gte_SumSquares3(
                         D_800A33F4->unkA8 - D_800A33F4->unk4C,
                         D_800A33F4->unkAC - D_800A33F4->unk4E,
                         D_800A33F4->unkB0 - D_800A33F4->unk50)) <
                    D_800A33F4->unk0) {
                    D_800A33F4->unk48 = arg0;
                    D_800A33F4->unk4A = arg1;
                    D_800A33F4->unk38 = D_800A33F4->unkA8;
                    D_800A33F4->unk3C = D_800A33F4->unkAC;
                    D_800A33F4->unk40 = D_800A33F4->unkB0;
                    D_800A33F4->unk28 = D_800A33F4->unkD0;
                    D_800A33F4->unk2C = D_800A33F4->unkD4;
                    D_800A33F4->unk30 = D_800A33F4->unkD8;
                    D_800A33F4->unk34 = D_800A33F4->unkDC;
                    D_800A33F4->unk0 = D_800A33F4->unkE0;
                    D_800A33F4->unk4 = D_800A33F4->unkCC;
                }
            }
        } else {
            data += 9;
            n = *data;
            data++;
            n &= 0xFF;
            data += (n + 1) * 2;
        }
    }
    return D_800A33F4->unk0 != 0x7FFFFFFF;
}

void collision_SetGrid(s32 a0) { D_800A33F0 = a0; }

void collision_RebaseGrid(s32 a0) { D_800A33F0 += a0; }

s16 collision_GetHitTag(void) { return g_collision_hit_tag; }

INCLUDE_ASM("asm/funcs", func_80054440);
INCLUDE_ASM("asm/funcs", func_800545F4);

const char D_80015840[28] = "Destruction tiny model.\n";

s32 motion_StartPlayback(
    s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6) {
    Unk800EFAE8Rec *s = &D_800EFAE8;
    s32 id = a0 + 0x131;
    s32 ret;
    s16 *t;
    Unk80045878Obj *v;
    s32 n;

    if (a6 != 0) {
        ret = ndata_ReadChecked(id, a6);
        s->unk2C = a6;
    } else {
        if (ndata_GetArenaRoomAfter(id) < 0) {
            func_80046914();
            printf(D_80015840);
        }
        s->unk2C = (s32)func_800469C4(id);
        ret = 0;
    }
    s->unk4 = *(s32 *)(((Unk800469C4Hdr *)s->unk2C)->unk4 + s->unk2C);
    s->unk2 = *(u16 *)(((Unk800469C4Hdr *)s->unk2C)->unk8 + s->unk2C);
    s->unk0 = 0;
    t = func_80046F14();
    t += func_80046798() * 24 + a1 * 6;
    s->unkC = *t++;
    s->unk10 = *t++;
    s->unk14 = *t++;
    s->unk1C = 0;
    s->unk20 = 0;
    s->unk44[0] = a2;
    s->unk44[1] = a3;
    s->unk48[0] = a4;
    s->unk48[1] = a5;
    s->unk1E = (((s->unk4 >> 8) & 0x7F) << 14) / 360;
    if (s->unk4 >= 0) {
        s->unk44[0] = -1;
    }
    if (!(s->unk4 & 0x40000000)) {
        s->unk44[1] = -1;
    }
    v = func_8004153C(0);
    if (v != 0) {
        softbody_RequestRebuild(v);
    }
    v = func_8004153C(1);
    if (v != 0) {
        softbody_RequestRebuild(v);
    }
    s->unk8 = a1;
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    if (s->unk4 & 0x3F) {
        n = (s->unk4 & 0x3F) - 1;
        if (a6 != 0) {
            a6 += ret;
            func_80046EDC(n, a6);
        } else {
            gpu_ResetGraphMode1();
            func_80046EDC(n, (s32)D_800A3770);
        }
    }
    if (s->unk4 & 0x8000) {
        func_8004659C(-1);
    }
    return ret;
}

extern s16 D_80099D54[];

void func_80054884(
    s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    motion_StartPlayback(D_80099D54[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}

void func_800548DC(void) {
    DrawSync(0);
    func_8004659C(-1);
    func_80046A60();
}
