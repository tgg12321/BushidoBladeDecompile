/* 16 game functions: the collision grid walk (collision_WalkSegmentGrid) and
 * its per-cell tests, accessors and setters (collision_TestSegmentWithWork,
 * collision_GetHit, collision_TestCellKeepingNearestHitScalarProjection,
 * collision_TestCellKeepingNearestHitGteProjection, ...), then the
 * motion-sequence loader motion_StartPlayback and its wrappers
 * motion_StartPlaybackAtIndexedBasePlusOffset /
 * motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock. .text 0x80052D00 (ROM
 * 0x43500). Start boundary: SEAM (owner ruling 2026-10-17; Codex seam #72): the
 * statics g_collision_grid, g_collision_segment_work and g_collision_hit_tag
 * (0x800A33F0..0x800A33F8) are used only here and open the old 3AB48 .sbss
 * group in address order; calls run from here into 3AB48.c's GTE helpers
 * (gte_SumSquares3, gte_ReadIR1IR2Sra2,
 * gte_TransformPointDifferenceWithInheritedLastRow), back only once
 * (func_8004DDB4 -> gte_ColorGroupedNormalStreamWithSelectedMatrices); three
 * NOPs follow gte_ReadIR1IR2Sra2. A control cut inside this group (before
 * collision_TestCellKeepingNearestHitScalarProjection) does not byte-match: the
 * shared statics lose gp access. End: before motion_UpdatePlayback (seam #85,
 * moved there from motion_StartPlayback), so motion_StartPlayback's printf
 * string g_motion_playback_arena_shortfall_msg is this file's .rodata
 * (0x80015840+0x1C) and 4510C.o's starts with
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput's tables at offset 0. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s32 g_collision_grid;
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
    s32 axes_swapped;

    g_collision_segment_work->unk60 =
        g_collision_segment_work->seg_start.vx + 32000;
    g_collision_segment_work->unk64 =
        g_collision_segment_work->seg_start.vz + 32000;
    g_collision_segment_work->unk68 =
        g_collision_segment_work->seg_end.vx + 32000;
    g_collision_segment_work->nearest_dist_sq = 0x7FFFFFFF;
    g_collision_segment_work->unk6C =
        g_collision_segment_work->seg_end.vz + 32000;
    g_collision_segment_work->unk70 =
        g_collision_segment_work->unk68 - g_collision_segment_work->unk60;
    g_collision_segment_work->walk_cell.x =
        g_collision_segment_work->unk60 / 2000;
    g_collision_segment_work->walk_cell.z =
        g_collision_segment_work->unk64 / 2000;
    g_collision_segment_work->end_cell.x =
        g_collision_segment_work->unk68 / 2000;
    g_collision_segment_work->end_cell.z =
        g_collision_segment_work->unk6C / 2000;
    g_collision_segment_work->unk74 =
        g_collision_segment_work->unk6C - g_collision_segment_work->unk64;
    if (g_collision_segment_work->walk_cell.x ==
            g_collision_segment_work->end_cell.x &&
        g_collision_segment_work->walk_cell.z ==
            g_collision_segment_work->end_cell.z) {
        if (g_collision_segment_work->seg_start.vx ==
                g_collision_segment_work->seg_end.vx &&
            g_collision_segment_work->seg_start.vy ==
                g_collision_segment_work->seg_end.vy &&
            g_collision_segment_work->seg_start.vz ==
                g_collision_segment_work->seg_end.vz) {
            return 0;
        }
        g_collision_segment_work->unk5C(g_collision_segment_work->walk_cell.x,
                                        g_collision_segment_work->walk_cell.z);
    } else {
        g_collision_segment_work->unk80 =
            g_collision_segment_work->walk_cell.x * 2000 + 1000;
        g_collision_segment_work->start_cell_center_z =
            g_collision_segment_work->walk_cell.z * 2000 + 1000;
        g_collision_segment_work->unk60 -= g_collision_segment_work->unk80;
        g_collision_segment_work->unk64 -=
            g_collision_segment_work->start_cell_center_z;
        g_collision_segment_work->unk68 -= g_collision_segment_work->unk80;
        g_collision_segment_work->unk6C -=
            g_collision_segment_work->start_cell_center_z;
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
            axes_swapped = 1;
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
            axes_swapped = 0;
        }
        g_collision_segment_work->unk68 += 1000;
        g_collision_segment_work->unk60 += 1000;
        g_collision_segment_work->unk64 += 1000;
        g_collision_segment_work->walk_steps_left =
            g_collision_segment_work->unk68 / 2000 -
            g_collision_segment_work->unk60 / 2000 + 1;
        g_collision_segment_work->walk_slope =
            (g_collision_segment_work->unk74 << 12) /
            g_collision_segment_work->unk70;
        g_collision_segment_work->unk64 -=
            (g_collision_segment_work->unk60 *
             g_collision_segment_work->walk_slope) >>
            12;
        g_collision_segment_work->walk_minor_step =
            (g_collision_segment_work->walk_slope * 2000) >> 12;
        while (--g_collision_segment_work->walk_steps_left != -1) {
            if ((g_collision_segment_work->unk80 =
                     g_collision_segment_work->unk5C(
                         g_collision_segment_work->walk_cell.x,
                         g_collision_segment_work->walk_cell.z)) != 0) {
                break;
            }
            g_collision_segment_work->unk64 %= 2000;
            g_collision_segment_work->unk64 +=
                g_collision_segment_work->walk_minor_step;
            if (g_collision_segment_work->walk_steps_left == 0) {
                break;
            }
            if (g_collision_segment_work->unk64 > 2000) {
                if (axes_swapped) {
                    if (x_direction < 0) {
                        g_collision_segment_work->walk_cell.x--;
                    } else {
                        g_collision_segment_work->walk_cell.x++;
                    }
                } else {
                    if (z_direction < 0) {
                        g_collision_segment_work->walk_cell.z--;
                    } else {
                        g_collision_segment_work->walk_cell.z++;
                    }
                }
                if ((g_collision_segment_work->unk80 =
                         g_collision_segment_work->unk5C(
                             g_collision_segment_work->walk_cell.x,
                             g_collision_segment_work->walk_cell.z)) != 0) {
                    break;
                }
            }
            if (axes_swapped) {
                if (z_direction < 0) {
                    g_collision_segment_work->walk_cell.z--;
                } else {
                    g_collision_segment_work->walk_cell.z++;
                }
            } else {
                if (x_direction < 0) {
                    g_collision_segment_work->walk_cell.x--;
                } else {
                    g_collision_segment_work->walk_cell.x++;
                }
            }
        }
        if (g_collision_segment_work->unk80 == 0 &&
            (g_collision_segment_work->walk_cell.x !=
                 g_collision_segment_work->end_cell.x ||
             g_collision_segment_work->walk_cell.z !=
                 g_collision_segment_work->end_cell.z)) {
            g_collision_segment_work->unk5C(
                g_collision_segment_work->end_cell.x,
                g_collision_segment_work->end_cell.z);
        }
    }
    return collision_GetHit(hit_point, hit_normal);
}

extern s32 collision_WalkSegmentGrid(s32 *, s16 *);
extern s32 collision_TestCellKeepingNearestHitScalarProjection();
extern s32 collision_TestCellKeepingNearestHitGteProjection();
extern Work_80053E9C g_collision_static_segment_work;

void collision_TestSegmentExtendingShortStart(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal) {
    s32 start_x, start_y, start_z;
    s32 end_x, end_y, end_z;
    g_collision_segment_work = &g_collision_static_segment_work;
    g_collision_static_segment_work.seg_start = *(VECTOR *)segment_start;
    g_collision_segment_work->seg_end = *(VECTOR *)segment_end;
    if (gte_SumSquares3(g_collision_segment_work->seg_end.vx -
                            g_collision_segment_work->seg_start.vx,
                        g_collision_segment_work->seg_end.vy -
                            g_collision_segment_work->seg_start.vy,
                        g_collision_segment_work->seg_end.vz -
                            g_collision_segment_work->seg_start.vz) <= 0x9C3F) {
        end_x = g_collision_segment_work->seg_end.vx;
        start_x = g_collision_segment_work->seg_start.vx;
        end_y = g_collision_segment_work->seg_end.vy;
        start_y = g_collision_segment_work->seg_start.vy;
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitScalarProjection;
        end_z = g_collision_segment_work->seg_end.vz;
        g_collision_segment_work->seg_start.vx =
            start_x - ((end_x - start_x) << 1);
        g_collision_segment_work->seg_start.vy =
            start_y - ((end_y - start_y) << 1);
        start_z = g_collision_segment_work->seg_start.vz;
        g_collision_segment_work->seg_start.vz =
            start_z - ((end_z - start_z) << 1);
    } else {
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitGteProjection;
    }
    collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegmentExtendingShortStartWithWork(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal,
    s32 segment_work_addr) {
    s32 start_x, start_y, start_z;
    s32 end_x, end_y, end_z;
    g_collision_segment_work = (Work_80053E9C *)segment_work_addr;
    g_collision_segment_work->seg_start = *(VECTOR *)segment_start;
    g_collision_segment_work->seg_end = *(VECTOR *)segment_end;
    if (gte_SumSquares3(g_collision_segment_work->seg_end.vx -
                            g_collision_segment_work->seg_start.vx,
                        g_collision_segment_work->seg_end.vy -
                            g_collision_segment_work->seg_start.vy,
                        g_collision_segment_work->seg_end.vz -
                            g_collision_segment_work->seg_start.vz) <= 0x9C3F) {
        end_x = g_collision_segment_work->seg_end.vx;
        start_x = g_collision_segment_work->seg_start.vx;
        end_y = g_collision_segment_work->seg_end.vy;
        start_y = g_collision_segment_work->seg_start.vy;
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitScalarProjection;
        end_z = g_collision_segment_work->seg_end.vz;
        g_collision_segment_work->seg_start.vx =
            start_x - ((end_x - start_x) << 1);
        g_collision_segment_work->seg_start.vy =
            start_y - ((end_y - start_y) << 1);
        start_z = g_collision_segment_work->seg_start.vz;
        g_collision_segment_work->seg_start.vz =
            start_z - ((end_z - start_z) << 1);
    } else {
        g_collision_segment_work->unk5C =
            collision_TestCellKeepingNearestHitGteProjection;
    }
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegment(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal) {
    g_collision_segment_work = &g_collision_static_segment_work;
    g_collision_static_segment_work.seg_start = *(VECTOR *)segment_start;
    g_collision_segment_work->seg_end = *(VECTOR *)segment_end;
    g_collision_segment_work->unk5C =
        collision_TestCellKeepingNearestHitGteProjection;
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_TestSegmentWithWork(
    s32 *segment_start, s32 *segment_end, s32 *hit_point, s16 *hit_normal,
    s32 segment_work_addr) {
    g_collision_segment_work = (Work_80053E9C *)segment_work_addr;
    g_collision_segment_work->seg_start = *(VECTOR *)segment_start;
    g_collision_segment_work->seg_end = *(VECTOR *)segment_end;
    g_collision_segment_work->unk5C =
        collision_TestCellKeepingNearestHitGteProjection;
    return collision_WalkSegmentGrid(hit_point, hit_normal);
}

s32 collision_GetHit(s32 *hit_point, s16 *hit_normal) {
    Work_80053E9C *p = g_collision_segment_work;
    s32 t;
    if (p->nearest_dist_sq != 0x7FFFFFFF) {
        t = (p->hit_cell_x * 0x7D0) - 0x7D00;
        hit_point[0] = p->hit_local_x + t;
        hit_point[1] = p->hit_local_y;
        t = (p->hit_cell_z * 0x7D0) - 0x7D00;
        hit_point[2] = p->hit_local_z + t;
        hit_normal[0] = p->hit_nx >> 2;
        hit_normal[1] = p->hit_ny >> 2;
        hit_normal[2] = p->hit_nz >> 2;
        g_collision_hit_tag = p->hit_tag;
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
    g_collision_segment_work->unkE0 =
        ((u16 *)g_collision_grid)[cell_z * 32 + cell_x];
    if (g_collision_segment_work->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(g_collision_grid + g_collision_segment_work->unkE0);
    x = cell_x * 2000 - 32000;
    z = cell_z * 2000 - 32000;
    g_collision_segment_work->local_start_x =
        g_collision_segment_work->seg_start.vx - x;
    g_collision_segment_work->local_start_y =
        g_collision_segment_work->seg_start.vy;
    g_collision_segment_work->local_start_z =
        g_collision_segment_work->seg_start.vz - z;
    g_collision_segment_work->local_end_x =
        g_collision_segment_work->seg_end.vx - x;
    g_collision_segment_work->local_end_y =
        g_collision_segment_work->seg_end.vy;
    g_collision_segment_work->local_end_z =
        g_collision_segment_work->seg_end.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        g_collision_segment_work->face_nx = *data;
        data++;
        g_collision_segment_work->face_ny = *data;
        data++;
        g_collision_segment_work->face_nz = *data;
        data++;
        g_collision_segment_work->face_plane_d = (u16)*data;
        data++;
        g_collision_segment_work->face_plane_d =
            (*data << 16) | g_collision_segment_work->face_plane_d;
        data++;
        g_collision_segment_work->unkE4 =
            g_collision_segment_work->face_nx *
                g_collision_segment_work->local_start_x +
            g_collision_segment_work->face_ny *
                g_collision_segment_work->local_start_y +
            g_collision_segment_work->face_nz *
                g_collision_segment_work->local_start_z +
            g_collision_segment_work->face_plane_d;
        g_collision_segment_work->unkE8 =
            g_collision_segment_work->face_nx *
                g_collision_segment_work->local_end_x +
            g_collision_segment_work->face_ny *
                g_collision_segment_work->local_end_y +
            g_collision_segment_work->face_nz *
                g_collision_segment_work->local_end_z +
            g_collision_segment_work->face_plane_d;

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
            g_collision_segment_work->plane_cross_x =
                (g_collision_segment_work->local_end_x -
                 g_collision_segment_work->local_start_x) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->plane_cross_y =
                (g_collision_segment_work->local_end_y -
                 g_collision_segment_work->local_start_y) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->plane_cross_z =
                (g_collision_segment_work->local_end_z -
                 g_collision_segment_work->local_start_z) *
                g_collision_segment_work->unkE4 /
                g_collision_segment_work->unkE0;
            g_collision_segment_work->plane_cross_x =
                (g_collision_segment_work->plane_cross_x < 0 ? -1 : 1) *
                ((g_collision_segment_work->plane_cross_x >= 0
                      ? g_collision_segment_work->plane_cross_x + 1
                      : -g_collision_segment_work->plane_cross_x + 1) >>
                 1);
            g_collision_segment_work->plane_cross_y =
                (g_collision_segment_work->plane_cross_y < 0 ? -1 : 1) *
                ((g_collision_segment_work->plane_cross_y >= 0
                      ? g_collision_segment_work->plane_cross_y + 1
                      : -g_collision_segment_work->plane_cross_y + 1) >>
                 1);
            g_collision_segment_work->plane_cross_z =
                (g_collision_segment_work->plane_cross_z < 0 ? -1 : 1) *
                ((g_collision_segment_work->plane_cross_z >= 0
                      ? g_collision_segment_work->plane_cross_z + 1
                      : -g_collision_segment_work->plane_cross_z + 1) >>
                 1);
            g_collision_segment_work->plane_cross_x +=
                g_collision_segment_work->local_start_x;
            g_collision_segment_work->plane_cross_y +=
                g_collision_segment_work->local_start_y;
            g_collision_segment_work->plane_cross_z +=
                g_collision_segment_work->local_start_z;
            g_collision_segment_work->unkE0 = *data;
            data++;
            g_collision_segment_work->unkE4 = *data;
            data++;
            g_collision_segment_work->unkE8 = *data;
            data++;
            g_collision_segment_work->proj_axis_x = *data;
            data++;
            g_collision_segment_work->proj_axis_y = *data;
            data++;
            g_collision_segment_work->proj_axis_z = *data;
            data++;
            g_collision_segment_work->proj_cross_u =
                ((g_collision_segment_work->plane_cross_x -
                  g_collision_segment_work->unkE0) *
                     g_collision_segment_work->proj_axis_x +
                 (g_collision_segment_work->plane_cross_y -
                  g_collision_segment_work->unkE4) *
                     g_collision_segment_work->proj_axis_y +
                 (g_collision_segment_work->plane_cross_z -
                  g_collision_segment_work->unkE8) *
                     g_collision_segment_work->proj_axis_z) >>
                14;
            g_collision_segment_work->proj_axis_x = *data;
            data++;
            g_collision_segment_work->proj_axis_y = *data;
            data++;
            g_collision_segment_work->proj_axis_z = *data;
            data++;
            g_collision_segment_work->proj_cross_v =
                ((g_collision_segment_work->plane_cross_x -
                  g_collision_segment_work->unkE0) *
                     g_collision_segment_work->proj_axis_x +
                 (g_collision_segment_work->plane_cross_y -
                  g_collision_segment_work->unkE4) *
                     g_collision_segment_work->proj_axis_y +
                 (g_collision_segment_work->plane_cross_z -
                  g_collision_segment_work->unkE8) *
                     g_collision_segment_work->proj_axis_z) >>
                14;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            g_collision_segment_work->face_tag = n >> 8;
            n &= 0xFF;
            g_collision_segment_work->edge_from_u = *data;
            data++;
            y = *data;
            data++;
            g_collision_segment_work->unkE0 = 1;
            g_collision_segment_work->edge_from_v = y;
            while (--n != -1) {
                g_collision_segment_work->edge_to_u = *data;
                data++;
                g_collision_segment_work->edge_to_v = *data;
                data++;
                if ((g_collision_segment_work->proj_cross_u -
                     g_collision_segment_work->edge_from_u) *
                            (g_collision_segment_work->edge_to_v -
                             g_collision_segment_work->edge_from_v) -
                        (g_collision_segment_work->proj_cross_v -
                         g_collision_segment_work->edge_from_v) *
                            (g_collision_segment_work->edge_to_u -
                             g_collision_segment_work->edge_from_u) >
                    0) {
                    g_collision_segment_work->unkE0 = 0;
                    break;
                }
                g_collision_segment_work->edge_from_u =
                    g_collision_segment_work->edge_to_u;
                g_collision_segment_work->edge_from_v =
                    g_collision_segment_work->edge_to_v;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (g_collision_segment_work->unkE0 != 0) {
                if ((g_collision_segment_work->unkE0 = gte_SumSquares3(
                         g_collision_segment_work->plane_cross_x -
                             g_collision_segment_work->local_start_x,
                         g_collision_segment_work->plane_cross_y -
                             g_collision_segment_work->local_start_y,
                         g_collision_segment_work->plane_cross_z -
                             g_collision_segment_work->local_start_z)) <
                    g_collision_segment_work->nearest_dist_sq) {
                    g_collision_segment_work->hit_cell_x = cell_x;
                    g_collision_segment_work->hit_cell_z = cell_z;
                    g_collision_segment_work->hit_local_x =
                        g_collision_segment_work->plane_cross_x;
                    g_collision_segment_work->hit_local_y =
                        g_collision_segment_work->plane_cross_y;
                    g_collision_segment_work->hit_local_z =
                        g_collision_segment_work->plane_cross_z;
                    g_collision_segment_work->hit_nx =
                        g_collision_segment_work->face_nx;
                    g_collision_segment_work->hit_ny =
                        g_collision_segment_work->face_ny;
                    g_collision_segment_work->hit_nz =
                        g_collision_segment_work->face_nz;
                    g_collision_segment_work->hit_plane_d =
                        g_collision_segment_work->face_plane_d;
                    g_collision_segment_work->nearest_dist_sq =
                        g_collision_segment_work->unkE0;
                    g_collision_segment_work->hit_tag =
                        g_collision_segment_work->face_tag;
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
    return g_collision_segment_work->nearest_dist_sq != 0x7FFFFFFF;
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
    g_collision_segment_work->unkE0 =
        ((u16 *)g_collision_grid)[cell_z * 32 + cell_x];
    if (g_collision_segment_work->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(g_collision_grid + g_collision_segment_work->unkE0);
    x = cell_x * 2000 - 32000;
    z = cell_z * 2000 - 32000;
    g_collision_segment_work->local_start_x =
        g_collision_segment_work->seg_start.vx - x;
    g_collision_segment_work->local_start_y =
        g_collision_segment_work->seg_start.vy;
    g_collision_segment_work->local_start_z =
        g_collision_segment_work->seg_start.vz - z;
    g_collision_segment_work->local_end_x =
        g_collision_segment_work->seg_end.vx - x;
    g_collision_segment_work->local_end_y =
        g_collision_segment_work->seg_end.vy;
    g_collision_segment_work->local_end_z =
        g_collision_segment_work->seg_end.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        g_collision_segment_work->face_nx = *data;
        data++;
        g_collision_segment_work->face_ny = *data;
        data++;
        g_collision_segment_work->face_nz = *data;
        data++;
        g_collision_segment_work->face_plane_d = (u16)*data;
        data++;
        g_collision_segment_work->face_plane_d =
            (*data << 16) | g_collision_segment_work->face_plane_d;
        data++;
        g_collision_segment_work->unkE4 =
            g_collision_segment_work->face_nx *
                g_collision_segment_work->local_start_x +
            g_collision_segment_work->face_ny *
                g_collision_segment_work->local_start_y +
            g_collision_segment_work->face_nz *
                g_collision_segment_work->local_start_z +
            g_collision_segment_work->face_plane_d;
        g_collision_segment_work->unkE8 =
            g_collision_segment_work->face_nx *
                g_collision_segment_work->local_end_x +
            g_collision_segment_work->face_ny *
                g_collision_segment_work->local_end_y +
            g_collision_segment_work->face_nz *
                g_collision_segment_work->local_end_z +
            g_collision_segment_work->face_plane_d;

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
            g_collision_segment_work->plane_cross_x =
                (g_collision_segment_work->local_end_x -
                 g_collision_segment_work->local_start_x) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->local_start_x;
            g_collision_segment_work->plane_cross_y =
                (g_collision_segment_work->local_end_y -
                 g_collision_segment_work->local_start_y) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->local_start_y;
            g_collision_segment_work->plane_cross_z =
                (g_collision_segment_work->local_end_z -
                 g_collision_segment_work->local_start_z) *
                    g_collision_segment_work->unkE4 /
                    g_collision_segment_work->unkE0 +
                g_collision_segment_work->local_start_z;
            gte_TransformPointDifferenceWithInheritedLastRow(
                (s32)data, g_collision_segment_work->plane_cross_x,
                g_collision_segment_work->plane_cross_y,
                g_collision_segment_work->plane_cross_z);
            data += 9;
            var_6 = (u16)*data;
            data++;
            n = var_6;
            g_collision_segment_work->face_tag = n >> 8;
            n &= 0xFF;
            g_collision_segment_work->edge_from_u = *data;
            data++;
            y = *data;
            data++;
            g_collision_segment_work->unkE0 = 1;
            g_collision_segment_work->edge_from_v = y;
            gte_ReadIR1IR2Sra2(&g_collision_segment_work->proj_cross_u,
                               &g_collision_segment_work->proj_cross_v);
            while (--n != -1) {
                g_collision_segment_work->edge_to_u = *data;
                data++;
                g_collision_segment_work->edge_to_v = *data;
                data++;
                if ((g_collision_segment_work->proj_cross_u -
                     g_collision_segment_work->edge_from_u) *
                            (g_collision_segment_work->edge_to_v -
                             g_collision_segment_work->edge_from_v) -
                        (g_collision_segment_work->proj_cross_v -
                         g_collision_segment_work->edge_from_v) *
                            (g_collision_segment_work->edge_to_u -
                             g_collision_segment_work->edge_from_u) >
                    0) {
                    g_collision_segment_work->unkE0 = 0;
                    break;
                }
                g_collision_segment_work->edge_from_u =
                    g_collision_segment_work->edge_to_u;
                g_collision_segment_work->edge_from_v =
                    g_collision_segment_work->edge_to_v;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (g_collision_segment_work->unkE0 != 0) {
                if ((g_collision_segment_work->unkE0 = gte_SumSquares3(
                         g_collision_segment_work->plane_cross_x -
                             g_collision_segment_work->local_start_x,
                         g_collision_segment_work->plane_cross_y -
                             g_collision_segment_work->local_start_y,
                         g_collision_segment_work->plane_cross_z -
                             g_collision_segment_work->local_start_z)) <
                    g_collision_segment_work->nearest_dist_sq) {
                    g_collision_segment_work->hit_cell_x = cell_x;
                    g_collision_segment_work->hit_cell_z = cell_z;
                    g_collision_segment_work->hit_local_x =
                        g_collision_segment_work->plane_cross_x;
                    g_collision_segment_work->hit_local_y =
                        g_collision_segment_work->plane_cross_y;
                    g_collision_segment_work->hit_local_z =
                        g_collision_segment_work->plane_cross_z;
                    g_collision_segment_work->hit_nx =
                        g_collision_segment_work->face_nx;
                    g_collision_segment_work->hit_ny =
                        g_collision_segment_work->face_ny;
                    g_collision_segment_work->hit_nz =
                        g_collision_segment_work->face_nz;
                    g_collision_segment_work->hit_plane_d =
                        g_collision_segment_work->face_plane_d;
                    g_collision_segment_work->nearest_dist_sq =
                        g_collision_segment_work->unkE0;
                    g_collision_segment_work->hit_tag =
                        g_collision_segment_work->face_tag;
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
    return g_collision_segment_work->nearest_dist_sq != 0x7FFFFFFF;
}

void collision_SetGrid(s32 grid_addr) { g_collision_grid = grid_addr; }

void collision_RebaseGrid(s32 rebase_delta) {
    g_collision_grid += rebase_delta;
}

s16 collision_GetHitTag(void) { return g_collision_hit_tag; }

INCLUDE_ASM("asm/funcs", gte_ColorGroupedNormalStreamWithSelectedMatrices);
INCLUDE_ASM("asm/funcs", func_800545F4);

const char g_motion_playback_arena_shortfall_msg[28] =
    "Destruction tiny model.\n";

s32 motion_StartPlayback(s32 playback_index, s32 start_pair, s32 a2, s32 a3,
                         s32 a4, s32 a5, s32 dest_addr) {
    Unk800EFAE8Rec *s = &g_motion_playback_record;
    s32 playback_file_id = playback_index + 0x131;
    s32 ret;
    s16 *start_position_cursor;
    Unk80045878Obj *slot_object;
    s32 n;

    if (dest_addr != 0) {
        ret = ndata_ReadChecked(playback_file_id, dest_addr);
        s->block_base = dest_addr;
    } else {
        if (ndata_GetArenaRoomAfter(playback_file_id) < 0) {
            resource_FreeNodeAssetBlock();
            printf(g_motion_playback_arena_shortfall_msg);
        }
        s->block_base = (s32)motion_LoadPlaybackBlock(playback_file_id);
        ret = 0;
    }
    s->control_word =
        *(s32 *)(((Unk800469C4Hdr *)s->block_base)->control_word_offset +
                 s->block_base);
    s->frame_count =
        *(u16 *)(((Unk800469C4Hdr *)s->block_base)->frame_count_offset +
                 s->block_base);
    s->current_frame = 0;
    start_position_cursor = fight_GetStartPositionPairTable();
    start_position_cursor += resource_GetLoadedSelector() * 24 + start_pair * 6;
    s->origin_x = *start_position_cursor++;
    s->origin_y = *start_position_cursor++;
    s->origin_z = *start_position_cursor++;
    s->rotation_x = 0;
    s->rotation_z = 0;
    s->unk44[0] = a2;
    s->unk44[1] = a3;
    s->unk48[0] = a4;
    s->unk48[1] = a5;
    s->rotation_y = (((s->control_word >> 8) & 0x7F) << 14) / 360;
    if (s->control_word >= 0) {
        s->unk44[0] = -1;
    }
    if (!(s->control_word & 0x40000000)) {
        s->unk44[1] = -1;
    }
    slot_object = slot_GetObject(0);
    if (slot_object != 0) {
        softbody_RequestRebuild(slot_object);
    }
    slot_object = slot_GetObject(1);
    if (slot_object != 0) {
        softbody_RequestRebuild(slot_object);
    }
    s->start_pair_index = start_pair;
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    if (s->control_word & 0x3F) {
        n = (s->control_word & 0x3F) - 1;
        if (dest_addr != 0) {
            dest_addr += ret;
            resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
                n, dest_addr);
        } else {
            gpu_ResetGraphMode1();
            resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
                n, (s32)D_800A3770);
        }
    }
    if (s->control_word & 0x8000) {
        resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
    }
    return ret;
}

extern s16 g_motion_playback_file_id_bases[];

void motion_StartPlaybackAtIndexedBasePlusOffset(
    s32 file_base_index, s32 file_id_offset, s32 start_pair, s32 a3, s32 a4,
    s32 a5, s32 a6, s32 dest_addr) {
    motion_StartPlayback(g_motion_playback_file_id_bases[file_base_index] +
                             file_id_offset - 0x131,
                         start_pair, a3, a4, a5, a6, dest_addr);
}

void motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock(void) {
    DrawSync(0);
    resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
    motion_FreePlaybackBlock();
}

/* .data 0x80099D54..0x80099D88, this file's whole .data (d_89C78.c's
 * g_node_entry_pair_local_transforms ends at 0x80099D54; 4510C.c's
 * g_cpu_profiles follows): per-entry base sequence ids (-1: none);
 * motion_StartPlaybackAtIndexedBasePlusOffset starts sequence
 * g_motion_playback_file_id_bases[file_base_index] + file_id_offset - 0x131.
 * The final 0 fills the 0x34 bytes to g_cpu_profiles. Defined after every
 * function, so no body sees a definition where it saw an extern. */
s16 g_motion_playback_file_id_bases[26] = {
    0x131, 0x151, 0x171, 0x192, 0x1AD, -1,    0x1CB, 0x1E6, 0x204,
    0x222, -1,    -1,    0x241, 0x264, 0x287, 0x2AA, -1,    0x2CD,
    0x2F0, 0x30F, 0x32E, 0x351, 0x374, -1,    -1,    0x0};
