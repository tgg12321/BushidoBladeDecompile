/* 16 game functions: the collision grid walk (collision_WalkSegmentGrid) and
 * its per-cell tests, accessors and setters (collision_TestSegmentWithWork,
 * collision_GetHit, collision_TestCellKeepingNearestHitScalarProjection,
 * collision_TestCellKeepingNearestHitGteProjection, ...), then the
 * motion-sequence loader motion_StartPlayback and its wrappers
 * motion_StartPlaybackAtIndexedBasePlusOffset /
 * motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock. .text 0x80052D00 (ROM
 * 0x43500). Start boundary: SEAM (owner ruling 2026-10-17; Codex seam #72): the
 * statics D_800A33F0, g_collision_segment_work and g_collision_hit_tag
 * (0x800A33F0..0x800A33F8) are used only here and open the old 3AB48 .sbss
 * group in address order; calls run from here into 3AB48.c's GTE helpers
 * (gte_SumSquares3, gte_ReadIR1IR2Sra2,
 * gte_TransformPointDifferenceWithInheritedLastRow), back only once
 * (func_8004DDB4 -> gte_ColorGroupedNormalStreamWithSelectedMatrices); three
 * NOPs follow gte_ReadIR1IR2Sra2. A control cut inside this group (before
 * collision_TestCellKeepingNearestHitScalarProjection) does not byte-match: the
 * shared statics lose gp access. End: before motion_UpdatePlayback (seam #85,
 * moved there from motion_StartPlayback), so motion_StartPlayback's printf
 * string D_80015840 is this file's .rodata (0x80015840+0x1C) and 4510C.o's
 * starts with fighter_UpdateCpuNavigationChooseScriptsAndHeldInput's tables at
 * offset 0. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s32 D_800A33F0;
static Work_80053E9C *g_collision_segment_work;
static u16 g_collision_hit_tag;
static s32 D_800A33FC; /* not named by any code or data: size from the gap */

extern s32 collision_GetHit(s32 *, s16 *);

/* Walks the 32x32 grid of 2000-unit cells (origin -32000) along the XZ
 * segment from the start point (+0x8/+0x10) to the end point (+0x18/+0x20),
 * DDA style: +0x88/+0x8C are the current and end cells, +0x70/+0x74 the
 * major/minor extents (exchanged, with +0x60/+0x64 and +0x68/+0x6C, when
 * +0x74 > +0x70), +0x7C the 4.12 slope and
 * +0x90 the major-axis steps left. Each cell crossed goes to the per-cell
 * test at +0x5C (collision_TestCellKeepingNearestHitGteProjection or
 * collision_TestCellKeepingNearestHitScalarProjection) until one reports a hit;
 * collision_GetHit then reads the result back out. */
s32 collision_WalkSegmentGrid(s32 *hit_point, s16 *hit_normal) {
    s32 x_direction;
    s32 z_direction;
    s32 var_2;

    g_collision_segment_work->unk60 = g_collision_segment_work->unk8.vx + 32000;
    g_collision_segment_work->unk64 = g_collision_segment_work->unk8.vz + 32000;
    g_collision_segment_work->unk68 =
        g_collision_segment_work->unk18.vx + 32000;
    g_collision_segment_work->unk0 = 0x7FFFFFFF;
    g_collision_segment_work->unk6C =
        g_collision_segment_work->unk18.vz + 32000;
    g_collision_segment_work->unk70 =
        g_collision_segment_work->unk68 - g_collision_segment_work->unk60;
    g_collision_segment_work->unk88.x = g_collision_segment_work->unk60 / 2000;
    g_collision_segment_work->unk88.z = g_collision_segment_work->unk64 / 2000;
    g_collision_segment_work->unk8C.x = g_collision_segment_work->unk68 / 2000;
    g_collision_segment_work->unk8C.z = g_collision_segment_work->unk6C / 2000;
    g_collision_segment_work->unk74 =
        g_collision_segment_work->unk6C - g_collision_segment_work->unk64;
    if (g_collision_segment_work->unk88.x ==
            g_collision_segment_work->unk8C.x &&
        g_collision_segment_work->unk88.z ==
            g_collision_segment_work->unk8C.z) {
        if (g_collision_segment_work->unk8.vx ==
                g_collision_segment_work->unk18.vx &&
            g_collision_segment_work->unk8.vy ==
                g_collision_segment_work->unk18.vy &&
            g_collision_segment_work->unk8.vz ==
                g_collision_segment_work->unk18.vz) {
            return 0;
        }
        g_collision_segment_work->unk5C(g_collision_segment_work->unk88.x,
                                        g_collision_segment_work->unk88.z);
    } else {
        g_collision_segment_work->unk80 =
            g_collision_segment_work->unk88.x * 2000 + 1000;
        g_collision_segment_work->unk84 =
            g_collision_segment_work->unk88.z * 2000 + 1000;
        g_collision_segment_work->unk60 -= g_collision_segment_work->unk80;
        g_collision_segment_work->unk64 -= g_collision_segment_work->unk84;
        g_collision_segment_work->unk68 -= g_collision_segment_work->unk80;
        g_collision_segment_work->unk6C -= g_collision_segment_work->unk84;
        if (g_collision_segment_work->unk70 < 0) {
            x_direction = -1;
            g_collision_segment_work->unk70 = -g_collision_segment_work->unk70;
            g_collision_segment_work->unk60 = -g_collision_segment_work->unk60;
            g_collision_segment_work->unk68 = -g_collision_segment_work->unk68;
        } else {
            x_direction = 1;
        }
        if (g_collision_segment_work->unk74 < 0) {
            z_direction = -1;
            g_collision_segment_work->unk74 = -g_collision_segment_work->unk74;
            g_collision_segment_work->unk64 = -g_collision_segment_work->unk64;
            g_collision_segment_work->unk6C = -g_collision_segment_work->unk6C;
        } else {
            z_direction = 1;
        }
        if (g_collision_segment_work->unk74 > g_collision_segment_work->unk70) {
            var_2 = 1;
            g_collision_segment_work->unk80 = g_collision_segment_work->unk70;
            g_collision_segment_work->unk70 = g_collision_segment_work->unk74;
            g_collision_segment_work->unk74 = g_collision_segment_work->unk80;
            g_collision_segment_work->unk80 = g_collision_segment_work->unk60;
            g_collision_segment_work->unk60 = g_collision_segment_work->unk64;
            g_collision_segment_work->unk64 = g_collision_segment_work->unk80;
            g_collision_segment_work->unk80 = g_collision_segment_work->unk68;
            g_collision_segment_work->unk68 = g_collision_segment_work->unk6C;
            g_collision_segment_work->unk6C = g_collision_segment_work->unk80;
        } else {
            var_2 = 0;
        }
        g_collision_segment_work->unk68 += 1000;
        g_collision_segment_work->unk60 += 1000;
        g_collision_segment_work->unk64 += 1000;
        g_collision_segment_work->unk90 =
            g_collision_segment_work->unk68 / 2000 -
            g_collision_segment_work->unk60 / 2000 + 1;
        g_collision_segment_work->unk7C =
            (g_collision_segment_work->unk74 << 12) /
            g_collision_segment_work->unk70;
        g_collision_segment_work->unk64 -=
            (g_collision_segment_work->unk60 *
             g_collision_segment_work->unk7C) >>
            12;
        g_collision_segment_work->unk78 =
            (g_collision_segment_work->unk7C * 2000) >> 12;
        while (--g_collision_segment_work->unk90 != -1) {
            if ((g_collision_segment_work->unk80 =
                     g_collision_segment_work->unk5C(
                         g_collision_segment_work->unk88.x,
                         g_collision_segment_work->unk88.z)) != 0) {
                break;
            }
            g_collision_segment_work->unk64 %= 2000;
            g_collision_segment_work->unk64 += g_collision_segment_work->unk78;
            if (g_collision_segment_work->unk90 == 0) {
                break;
            }
            if (g_collision_segment_work->unk64 > 2000) {
                if (var_2) {
                    if (x_direction < 0) {
                        g_collision_segment_work->unk88.x--;
                    } else {
                        g_collision_segment_work->unk88.x++;
                    }
                } else {
                    if (z_direction < 0) {
                        g_collision_segment_work->unk88.z--;
                    } else {
                        g_collision_segment_work->unk88.z++;
                    }
                }
                if ((g_collision_segment_work->unk80 =
                         g_collision_segment_work->unk5C(
                             g_collision_segment_work->unk88.x,
                             g_collision_segment_work->unk88.z)) != 0) {
                    break;
                }
            }
            if (var_2) {
                if (z_direction < 0) {
                    g_collision_segment_work->unk88.z--;
                } else {
                    g_collision_segment_work->unk88.z++;
                }
            } else {
                if (x_direction < 0) {
                    g_collision_segment_work->unk88.x--;
                } else {
                    g_collision_segment_work->unk88.x++;
                }
            }
        }
        if (g_collision_segment_work->unk80 == 0 &&
            (g_collision_segment_work->unk88.x !=
                 g_collision_segment_work->unk8C.x ||
             g_collision_segment_work->unk88.z !=
                 g_collision_segment_work->unk8C.z)) {
            g_collision_segment_work->unk5C(g_collision_segment_work->unk8C.x,
                                            g_collision_segment_work->unk8C.z);
        }
    }
    return collision_GetHit(hit_point, hit_normal);
}

extern s32 collision_WalkSegmentGrid(s32 *, s16 *);
extern s32 collision_TestCellKeepingNearestHitScalarProjection();
extern s32 collision_TestCellKeepingNearestHitGteProjection();
extern Work_80053E9C D_800EF9F8;

void collision_TestSegmentExtendingShortStart(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    g_collision_segment_work = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)segment_start;
    g_collision_segment_work->unk18 = *(VECTOR *)segment_end;
    if (gte_SumSquares3(g_collision_segment_work->unk18.vx -
                            g_collision_segment_work->unk8.vx,
                        g_collision_segment_work->unk18.vy -
                            g_collision_segment_work->unk8.vy,
                        g_collision_segment_work->unk18.vz -
                            g_collision_segment_work->unk8.vz) <= 0x9C3F) {
        hi0 = g_collision_segment_work->unk18.vx;
        a = g_collision_segment_work->unk8.vx;
        hi1 = g_collision_segment_work->unk18.vy;
        b = g_collision_segment_work->unk8.vy;
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitScalarProjection;
        hi2 = g_collision_segment_work->unk18.vz;
        g_collision_segment_work->unk8.vx = a - ((hi0 - a) << 1);
        g_collision_segment_work->unk8.vy = b - ((hi1 - b) << 1);
        c = g_collision_segment_work->unk8.vz;
        g_collision_segment_work->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitGteProjection;
    }
    collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegmentExtendingShortStartWithWork(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal,
    s32 segment_work_addr) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    g_collision_segment_work = (Work_80053E9C *)segment_work_addr;
    g_collision_segment_work->unk8 = *(VECTOR *)segment_start;
    g_collision_segment_work->unk18 = *(VECTOR *)segment_end;
    if (gte_SumSquares3(g_collision_segment_work->unk18.vx -
                            g_collision_segment_work->unk8.vx,
                        g_collision_segment_work->unk18.vy -
                            g_collision_segment_work->unk8.vy,
                        g_collision_segment_work->unk18.vz -
                            g_collision_segment_work->unk8.vz) <= 0x9C3F) {
        hi0 = g_collision_segment_work->unk18.vx;
        a = g_collision_segment_work->unk8.vx;
        hi1 = g_collision_segment_work->unk18.vy;
        b = g_collision_segment_work->unk8.vy;
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitScalarProjection;
        hi2 = g_collision_segment_work->unk18.vz;
        g_collision_segment_work->unk8.vx = a - ((hi0 - a) << 1);
        g_collision_segment_work->unk8.vy = b - ((hi1 - b) << 1);
        c = g_collision_segment_work->unk8.vz;
        g_collision_segment_work->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitGteProjection;
    }
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegment(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal) {
    g_collision_segment_work = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)segment_start;
    g_collision_segment_work->unk18 = *(VECTOR *)segment_end;
    g_collision_segment_work->unk5C =
        collision_TestCellKeepingNearestHitGteProjection;
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegmentWithWork(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal,
    s32 segment_work_addr) {
    g_collision_segment_work = (Work_80053E9C *)segment_work_addr;
    g_collision_segment_work->unk8 = *(VECTOR *)segment_start;
    g_collision_segment_work->unk18 = *(VECTOR *)segment_end;
    g_collision_segment_work->unk5C =
        collision_TestCellKeepingNearestHitGteProjection;
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_GetHit(s32 *hit_point, s16 *hit_normal) {
    Work_80053E9C *p = g_collision_segment_work;
    s32 t;
    if (p->unk0 != 0x7FFFFFFF) {
        t = (p->unk48 * 0x7D0) - 0x7D00;
        hit_point[0] = p->unk38 + t;
        hit_point[1] = p->unk3C;
        t = (p->unk4A * 0x7D0) - 0x7D00;
        hit_point[2] = p->unk40 + t;
        hit_normal[0] = p->unk28 >> 2;
        hit_normal[1] = p->unk2C >> 2;
        hit_normal[2] = p->unk30 >> 2;
        g_collision_hit_tag = p->unk4;
        return 1;
    }
    return 0;
}

extern void gte_TransformPointDifferenceWithInheritedLastRow(
    s32, s32, s32, s32);
extern void gte_ReadIR1IR2Sra2(s32 *, s32 *);

s32 collision_TestCellKeepingNearestHitScalarProjection(
    s32 cell_x, s32 cell_z) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 var_6;

    if (cell_x < 0 || cell_z < 0 || cell_x >= 32 || cell_z >= 32) {
        return 0;
    }
    g_collision_segment_work->unkE0 = ((u16 *)D_800A33F0)[cell_z * 32 + cell_x];
    if (g_collision_segment_work->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + g_collision_segment_work->unkE0);
    x = cell_x * 2000 - 32000;
    z = cell_z * 2000 - 32000;
    g_collision_segment_work->unk4C = g_collision_segment_work->unk8.vx - x;
    g_collision_segment_work->unk4E = g_collision_segment_work->unk8.vy;
    g_collision_segment_work->unk50 = g_collision_segment_work->unk8.vz - z;
    g_collision_segment_work->unk54 = g_collision_segment_work->unk18.vx - x;
    g_collision_segment_work->unk56 = g_collision_segment_work->unk18.vy;
    g_collision_segment_work->unk58 = g_collision_segment_work->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        g_collision_segment_work->unkD0 = *data;
        data++;
        g_collision_segment_work->unkD4 = *data;
        data++;
        g_collision_segment_work->unkD8 = *data;
        data++;
        g_collision_segment_work->unkDC = (u16)*data;
        data++;
        g_collision_segment_work->unkDC =
            (*data << 16) | g_collision_segment_work->unkDC;
        data++;
        g_collision_segment_work->unkE4 =
            g_collision_segment_work->unkD0 * g_collision_segment_work->unk4C +
            g_collision_segment_work->unkD4 * g_collision_segment_work->unk4E +
            g_collision_segment_work->unkD8 * g_collision_segment_work->unk50 +
            g_collision_segment_work->unkDC;
        g_collision_segment_work->unkE8 =
            g_collision_segment_work->unkD0 * g_collision_segment_work->unk54 +
            g_collision_segment_work->unkD4 * g_collision_segment_work->unk56 +
            g_collision_segment_work->unkD8 * g_collision_segment_work->unk58 +
            g_collision_segment_work->unkDC;

        g_collision_segment_work->unkE4 =
            (g_collision_segment_work->unkE4 < 0 ? -1 : 1) *
            ((g_collision_segment_work->unkE4 < 0
                  ? -g_collision_segment_work->unkE4
                  : g_collision_segment_work->unkE4) >>
             10);
        g_collision_segment_work->unkE8 =
            (g_collision_segment_work->unkE8 < 0 ? -1 : 1) *
            ((g_collision_segment_work->unkE8 < 0
                  ? -g_collision_segment_work->unkE8
                  : g_collision_segment_work->unkE8) >>
             10);

        if (g_collision_segment_work->unkE4 >= 0 &&
            g_collision_segment_work->unkE8 < 0) {
            g_collision_segment_work->unkE0 = g_collision_segment_work->unkE4 -
                                              g_collision_segment_work->unkE8;
            g_collision_segment_work->unkE4 *= 2;
            g_collision_segment_work->unkA8 =
                (g_collision_segment_work->unk54 -
                 g_collision_segment_work->unk4C) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->unkAC =
                (g_collision_segment_work->unk56 -
                 g_collision_segment_work->unk4E) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->unkB0 =
                (g_collision_segment_work->unk58 -
                 g_collision_segment_work->unk50) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->unkA8 =
                (g_collision_segment_work->unkA8 < 0 ? -1 : 1) *
                ((g_collision_segment_work->unkA8 >= 0
                      ? g_collision_segment_work->unkA8 + 1
                      : -g_collision_segment_work->unkA8 + 1) >>
                 1);
            g_collision_segment_work->unkAC =
                (g_collision_segment_work->unkAC < 0 ? -1 : 1) *
                ((g_collision_segment_work->unkAC >= 0
                      ? g_collision_segment_work->unkAC + 1
                      : -g_collision_segment_work->unkAC + 1) >>
                 1);
            g_collision_segment_work->unkB0 =
                (g_collision_segment_work->unkB0 < 0 ? -1 : 1) *
                ((g_collision_segment_work->unkB0 >= 0
                      ? g_collision_segment_work->unkB0 + 1
                      : -g_collision_segment_work->unkB0 + 1) >>
                 1);
            g_collision_segment_work->unkA8 += g_collision_segment_work->unk4C;
            g_collision_segment_work->unkAC += g_collision_segment_work->unk4E;
            g_collision_segment_work->unkB0 += g_collision_segment_work->unk50;
            g_collision_segment_work->unkE0 = *data;
            data++;
            g_collision_segment_work->unkE4 = *data;
            data++;
            g_collision_segment_work->unkE8 = *data;
            data++;
            g_collision_segment_work->unk9C = *data;
            data++;
            g_collision_segment_work->unkA0 = *data;
            data++;
            g_collision_segment_work->unkA4 = *data;
            data++;
            g_collision_segment_work->unkC4 =
                ((g_collision_segment_work->unkA8 -
                  g_collision_segment_work->unkE0) *
                     g_collision_segment_work->unk9C +
                 (g_collision_segment_work->unkAC -
                  g_collision_segment_work->unkE4) *
                     g_collision_segment_work->unkA0 +
                 (g_collision_segment_work->unkB0 -
                  g_collision_segment_work->unkE8) *
                     g_collision_segment_work->unkA4) >>
                14;
            g_collision_segment_work->unk9C = *data;
            data++;
            g_collision_segment_work->unkA0 = *data;
            data++;
            g_collision_segment_work->unkA4 = *data;
            data++;
            g_collision_segment_work->unkC8 =
                ((g_collision_segment_work->unkA8 -
                  g_collision_segment_work->unkE0) *
                     g_collision_segment_work->unk9C +
                 (g_collision_segment_work->unkAC -
                  g_collision_segment_work->unkE4) *
                     g_collision_segment_work->unkA0 +
                 (g_collision_segment_work->unkB0 -
                  g_collision_segment_work->unkE8) *
                     g_collision_segment_work->unkA4) >>
                14;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            g_collision_segment_work->unkCC = n >> 8;
            n &= 0xFF;
            g_collision_segment_work->unkB4 = *data;
            data++;
            y = *data;
            data++;
            g_collision_segment_work->unkE0 = 1;
            g_collision_segment_work->unkB8 = y;
            while (--n != -1) {
                g_collision_segment_work->unkBC = *data;
                data++;
                g_collision_segment_work->unkC0 = *data;
                data++;
                if ((g_collision_segment_work->unkC4 -
                     g_collision_segment_work->unkB4) *
                            (g_collision_segment_work->unkC0 -
                             g_collision_segment_work->unkB8) -
                        (g_collision_segment_work->unkC8 -
                         g_collision_segment_work->unkB8) *
                            (g_collision_segment_work->unkBC -
                             g_collision_segment_work->unkB4) >
                    0) {
                    g_collision_segment_work->unkE0 = 0;
                    break;
                }
                g_collision_segment_work->unkB4 =
                    g_collision_segment_work->unkBC;
                g_collision_segment_work->unkB8 =
                    g_collision_segment_work->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (g_collision_segment_work->unkE0 != 0) {
                if ((g_collision_segment_work->unkE0 = gte_SumSquares3(
                         g_collision_segment_work->unkA8 -
                             g_collision_segment_work->unk4C,
                         g_collision_segment_work->unkAC -
                             g_collision_segment_work->unk4E,
                         g_collision_segment_work->unkB0 -
                             g_collision_segment_work->unk50)) <
                    g_collision_segment_work->unk0) {
                    g_collision_segment_work->unk48 = cell_x;
                    g_collision_segment_work->unk4A = cell_z;
                    g_collision_segment_work->unk38 =
                        g_collision_segment_work->unkA8;
                    g_collision_segment_work->unk3C =
                        g_collision_segment_work->unkAC;
                    g_collision_segment_work->unk40 =
                        g_collision_segment_work->unkB0;
                    g_collision_segment_work->unk28 =
                        g_collision_segment_work->unkD0;
                    g_collision_segment_work->unk2C =
                        g_collision_segment_work->unkD4;
                    g_collision_segment_work->unk30 =
                        g_collision_segment_work->unkD8;
                    g_collision_segment_work->unk34 =
                        g_collision_segment_work->unkDC;
                    g_collision_segment_work->unk0 =
                        g_collision_segment_work->unkE0;
                    g_collision_segment_work->unk4 =
                        g_collision_segment_work->unkCC;
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
    return g_collision_segment_work->unk0 != 0x7FFFFFFF;
}

s32 collision_TestCellKeepingNearestHitGteProjection(s32 cell_x, s32 cell_z) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 var_6;

    if (cell_x < 0 || cell_z < 0 || cell_x >= 32 || cell_z >= 32) {
        return 0;
    }
    g_collision_segment_work->unkE0 = ((u16 *)D_800A33F0)[cell_z * 32 + cell_x];
    if (g_collision_segment_work->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + g_collision_segment_work->unkE0);
    x = cell_x * 2000 - 32000;
    z = cell_z * 2000 - 32000;
    g_collision_segment_work->unk4C = g_collision_segment_work->unk8.vx - x;
    g_collision_segment_work->unk4E = g_collision_segment_work->unk8.vy;
    g_collision_segment_work->unk50 = g_collision_segment_work->unk8.vz - z;
    g_collision_segment_work->unk54 = g_collision_segment_work->unk18.vx - x;
    g_collision_segment_work->unk56 = g_collision_segment_work->unk18.vy;
    g_collision_segment_work->unk58 = g_collision_segment_work->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        g_collision_segment_work->unkD0 = *data;
        data++;
        g_collision_segment_work->unkD4 = *data;
        data++;
        g_collision_segment_work->unkD8 = *data;
        data++;
        g_collision_segment_work->unkDC = (u16)*data;
        data++;
        g_collision_segment_work->unkDC =
            (*data << 16) | g_collision_segment_work->unkDC;
        data++;
        g_collision_segment_work->unkE4 =
            g_collision_segment_work->unkD0 * g_collision_segment_work->unk4C +
            g_collision_segment_work->unkD4 * g_collision_segment_work->unk4E +
            g_collision_segment_work->unkD8 * g_collision_segment_work->unk50 +
            g_collision_segment_work->unkDC;
        g_collision_segment_work->unkE8 =
            g_collision_segment_work->unkD0 * g_collision_segment_work->unk54 +
            g_collision_segment_work->unkD4 * g_collision_segment_work->unk56 +
            g_collision_segment_work->unkD8 * g_collision_segment_work->unk58 +
            g_collision_segment_work->unkDC;

        g_collision_segment_work->unkE4 =
            (g_collision_segment_work->unkE4 < 0 ? -1 : 1) *
            ((g_collision_segment_work->unkE4 < 0
                  ? -g_collision_segment_work->unkE4
                  : g_collision_segment_work->unkE4) >>
             14);
        g_collision_segment_work->unkE8 =
            (g_collision_segment_work->unkE8 < 0 ? -1 : 1) *
            ((g_collision_segment_work->unkE8 < 0
                  ? -g_collision_segment_work->unkE8
                  : g_collision_segment_work->unkE8) >>
             14);

        if (g_collision_segment_work->unkE4 >= 0 &&
            g_collision_segment_work->unkE8 < 0) {
            g_collision_segment_work->unkE0 = g_collision_segment_work->unkE4 -
                                              g_collision_segment_work->unkE8;
            g_collision_segment_work->unkA8 =
                (g_collision_segment_work->unk54 -
                 g_collision_segment_work->unk4C) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->unk4C;
            g_collision_segment_work->unkAC =
                (g_collision_segment_work->unk56 -
                 g_collision_segment_work->unk4E) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->unk4E;
            g_collision_segment_work->unkB0 =
                (g_collision_segment_work->unk58 -
                 g_collision_segment_work->unk50) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->unk50;
            gte_TransformPointDifferenceWithInheritedLastRow(
                (s32)data, g_collision_segment_work->unkA8,
                g_collision_segment_work->unkAC,
                g_collision_segment_work->unkB0);
            data += 9;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            g_collision_segment_work->unkCC = n >> 8;
            n &= 0xFF;
            g_collision_segment_work->unkB4 = *data;
            data++;
            y = *data;
            data++;
            g_collision_segment_work->unkE0 = 1;
            g_collision_segment_work->unkB8 = y;
            gte_ReadIR1IR2Sra2(&g_collision_segment_work->unkC4,
                               &g_collision_segment_work->unkC8);
            while (--n != -1) {
                g_collision_segment_work->unkBC = *data;
                data++;
                g_collision_segment_work->unkC0 = *data;
                data++;
                if ((g_collision_segment_work->unkC4 -
                     g_collision_segment_work->unkB4) *
                            (g_collision_segment_work->unkC0 -
                             g_collision_segment_work->unkB8) -
                        (g_collision_segment_work->unkC8 -
                         g_collision_segment_work->unkB8) *
                            (g_collision_segment_work->unkBC -
                             g_collision_segment_work->unkB4) >
                    0) {
                    g_collision_segment_work->unkE0 = 0;
                    break;
                }
                g_collision_segment_work->unkB4 =
                    g_collision_segment_work->unkBC;
                g_collision_segment_work->unkB8 =
                    g_collision_segment_work->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (g_collision_segment_work->unkE0 != 0) {
                if ((g_collision_segment_work->unkE0 = gte_SumSquares3(
                         g_collision_segment_work->unkA8 -
                             g_collision_segment_work->unk4C,
                         g_collision_segment_work->unkAC -
                             g_collision_segment_work->unk4E,
                         g_collision_segment_work->unkB0 -
                             g_collision_segment_work->unk50)) <
                    g_collision_segment_work->unk0) {
                    g_collision_segment_work->unk48 = cell_x;
                    g_collision_segment_work->unk4A = cell_z;
                    g_collision_segment_work->unk38 =
                        g_collision_segment_work->unkA8;
                    g_collision_segment_work->unk3C =
                        g_collision_segment_work->unkAC;
                    g_collision_segment_work->unk40 =
                        g_collision_segment_work->unkB0;
                    g_collision_segment_work->unk28 =
                        g_collision_segment_work->unkD0;
                    g_collision_segment_work->unk2C =
                        g_collision_segment_work->unkD4;
                    g_collision_segment_work->unk30 =
                        g_collision_segment_work->unkD8;
                    g_collision_segment_work->unk34 =
                        g_collision_segment_work->unkDC;
                    g_collision_segment_work->unk0 =
                        g_collision_segment_work->unkE0;
                    g_collision_segment_work->unk4 =
                        g_collision_segment_work->unkCC;
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
    return g_collision_segment_work->unk0 != 0x7FFFFFFF;
}

void collision_SetGrid(s32 a0) { D_800A33F0 = a0; }

void collision_RebaseGrid(s32 a0) { D_800A33F0 += a0; }

s16 collision_GetHitTag(void) { return g_collision_hit_tag; }

INCLUDE_ASM("asm/funcs", gte_ColorGroupedNormalStreamWithSelectedMatrices);
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
            resource_FreeNodeAssetBlock();
            printf(D_80015840);
        }
        s->unk2C = (s32)motion_LoadPlaybackBlock(id);
        ret = 0;
    }
    s->unk4 = *(s32 *)(((Unk800469C4Hdr *)s->unk2C)->unk4 + s->unk2C);
    s->unk2 = *(u16 *)(((Unk800469C4Hdr *)s->unk2C)->unk8 + s->unk2C);
    s->unk0 = 0;
    t = fight_GetStartPositionPairTable();
    t += resource_GetLoadedSelector() * 24 + a1 * 6;
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
    v = slot_GetObject(0);
    if (v != 0) {
        softbody_RequestRebuild(v);
    }
    v = slot_GetObject(1);
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
            resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
                n, a6);
        } else {
            gpu_ResetGraphMode1();
            resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
                n, (s32)D_800A3770);
        }
    }
    if (s->unk4 & 0x8000) {
        resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
    }
    return ret;
}

extern s16 D_80099D54[];

void motion_StartPlaybackAtIndexedBasePlusOffset(
    s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    motion_StartPlayback(D_80099D54[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}

void motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock(void) {
    DrawSync(0);
    resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
    motion_FreePlaybackBlock();
}

/* .data 0x80099D54..0x80099D88, this file's whole .data (d_89C78.c's
 * D_80099D3C ends at 0x80099D54; 4510C.c's g_cpu_profiles follows):
 * per-entry base sequence ids (-1: none);
 * motion_StartPlaybackAtIndexedBasePlusOffset starts sequence D_80099D54[a0] +
 * a1 - 0x131. The final 0 fills the 0x34 bytes to g_cpu_profiles. Defined after
 * every function, so no body sees a definition where it saw an extern. */
s16 D_80099D54[26] = {
    0x131, 0x151, 0x171, 0x192, 0x1AD, -1,    0x1CB, 0x1E6, 0x204,
    0x222, -1,    -1,    0x241, 0x264, 0x287, 0x2AA, -1,    0x2CD,
    0x2F0, 0x30F, 0x32E, 0x351, 0x374, -1,    -1,    0x0};
