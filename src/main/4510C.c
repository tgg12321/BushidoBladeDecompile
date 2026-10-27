/* 20 game functions: motion-stream playback (motion_UpdatePlayback,
 * motion_RebasePlayback, ...) and the CPU fighter's control
 * (fighter_BuildCpuPad, fighter_GetNavCornerPoint,
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput, ...). .text 0x8005490C
 * (ROM 0x4510C). Start boundary: SEAM (owner ruling 2026-10-17; Codex seam #85,
 * placed before motion_UpdatePlayback rather than motion_StartPlayback so the
 * string D_80015840 belongs to its user's object): no static crosses the cut;
 * the .sdata D_800A3250..D_800A3263 belongs to these functions, and
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput's jump tables start this
 * file's .rodata at 0x8001585C, right after 43500.o's string; the only calls
 * back are to 43500.c's collision API collision_TestSegmentWithWork. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* 43500.c's collision API */
extern s32 collision_TestSegmentWithWork(
    s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3, s32 arg4);

extern s32 D_800A3250[2];

/* Per-frame playback of the record D_800EFAE8 that motion_StartPlayback fills.
 * On the first frame (unk0 == 0) it resolves the loaded data's offset table
 * (unk2C) into the camera stream (unk30), the per-object motion streams
 * (unk34[], dropped when they start with the "NULL" tag D_800A3250) and the
 * per-object nibble tables (unk3C[]). Every frame it places the camera (rotated
 * about y by unk1E, plus the offset unkC..unk14 motion_StartPlayback picks),
 * then decodes (motion_DecodeFrame) and places each slot_GetObject object's
 * data. The block holds those addresses as integers: typed as pointers, the
 * relocation sums in motion_RebasePlayback and motion_StartPlayback swap their
 * addu operands.
 */
s32 motion_UpdatePlayback(void) {
    Unk800EFAE8Rec *s = &D_800EFAE8;
    VECTOR vec;
    /* The 0x84 bytes motion_DecodeFrame decodes, as s16: three of its fields
     * load signed (lh), and the array goes to func_80040D48's s16 * parameter.
     */
    s16 decoded_motion_frame[0x42];
    s16 *v;
    s32 i;
    /* the slot_GetObject object being handled: objects 0 and 1 on the first
     * frame, object i in the loop, as SOTN's secret-room wall reuses
     * newEntity for each AllocEntity result, once and then in a loop (Q51) */
    /* SOTN: src/st/nz0/e_left_secret_room_wall.c:74 @db41b28 */
    Unk80045878Obj *slot_object;
    /* FAKE: one variable for the camera-rotated z of the camera and of object
     * i's offset: read in two blocks it is not tied to the subtraction and
     * takes $t0; one local per block: score 41 */
    s32 var_6;

    if (s->unk0 < 0) {
        return 0;
    }
    if (s->unk0 == 0) {
        s32 j;

        s->unk30 = ((Unk800469C4Hdr *)s->unk2C)->unkC + s->unk2C;
        s->unk34[0] = ((Unk800469C4Hdr *)s->unk2C)->unk10[0] + s->unk2C;
        s->unk34[1] = ((Unk800469C4Hdr *)s->unk2C)->unk10[1] + s->unk2C;
        bitstream_InitDeltaAccumulator(s->unk30, 0);
        for (j = 0; j < 2; j++) {
            if (*(s32 *)s->unk34[j] == D_800A3250[0]) {
                s->unk34[j] = 0;
            }
            if (s->unk34[j] != 0) {
                motion_InitStream(j, (u32 *)s->unk34[j]);
            }
        }
        s->unk3C[1] = 0;
        s->unk3C[0] = 0;
        if (s->unk4 & 0x80) {
            s->unk3C[0] = ((Unk800469C4Hdr *)s->unk2C)->unk18[0] + s->unk2C;
        }
        if (s->unk4 & 0x40) {
            s->unk3C[1] = ((Unk800469C4Hdr *)s->unk2C)->unk18[1] + s->unk2C;
        }
        slot_object = slot_GetObject(0);
        if (slot_object != 0) {
            softbody_RequestRebuild(slot_object);
        }
        slot_object = slot_GetObject(1);
        if (slot_object != 0) {
            softbody_RequestRebuild(slot_object);
        }
    }
    v = bitstream_AccumulateDeltas(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
        s32 sn = Judge[s->unk1E & 0xFFF];
        var_6 = (vec.vz * c - vec.vx * sn) >> 12;
        vec.vx = (vec.vz * sn + vec.vx * c) >> 12;
        vec.vz = var_6;
    }
    g_view_camera_root_node.work.t[0] = vec.vx + s->unkC;
    g_view_camera_root_node.work.t[1] = vec.vy + s->unk10;
    g_view_camera_root_node.work.t[2] = vec.vz + s->unk14;
    g_view_camera_root_node.xf.rot.vx = v[3];
    g_view_camera_root_node.xf.rot.vy = v[4] + 0x800;
    g_view_camera_root_node.xf.rot.vz = v[5];
    if (s->unk1E != 0) {
        MATRIX m;
        SVECTOR rot;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = s->unk1E;
        g_math_rot_matrix_funcs[0](&rot, &m);
        g_math_rot_matrix_funcs[0](
            &g_view_camera_root_node.xf.rot, &g_view_camera_root_node.work);
        MulMatrix2(&m, &g_view_camera_root_node.work);
        math_MatrixToAnglesYXZ(
            &g_view_camera_root_node.work, &g_view_camera_root_node.xf.rot);
        math_TransposeMatrixInPlace(&g_view_camera_root_node.work);
        g_view_camera_root_node.xf.mat = g_view_camera_root_node.work;
    } else {
        node_SetMatrixFromNegatedRot(&g_view_camera_root_node);
    }
    s->unk24[0] = -g_view_camera_root_node.xf.rot.vx;
    s->unk24[1] = -g_view_camera_root_node.xf.rot.vy;
    s->unk24[2] = -g_view_camera_root_node.xf.rot.vz;
    light_ResetPointLight();
    light_ApplyPointLightTargets();
    camera_CopyViewMatrixWithHalvedMiddleRow();
    gpu_ResetAndStampViewCenteredCellAttributes();
    calc_LightMatrix(&g_light_setups[0]);
    calc_LightMatrix(&g_light_setups[1]);
    calc_LightMatrix(&g_light_setups[4]);
    for (i = 0; i < 2; i++) {
        if (s->unk34[i] != 0) {
            s32 ang;
            slot_object = slot_GetObject(i);
            motion_DecodeFrame(i, s->unk0, (MotionFrame *)decoded_motion_frame,
                               (u16 *)0x1F800000);
            vec.vy = decoded_motion_frame[0];
            vec.vy *= slot_object->position_scale;
            vec.vy >>= 12;
            ang = decoded_motion_frame[1];
            vec.vx = (Judge[ang & 0xFFF] * decoded_motion_frame[2]) >> 12;
            vec.vz =
                (Judge[(ang + 0x400) & 0xFFF] * decoded_motion_frame[2]) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
                s32 sn = Judge[s->unk1E & 0xFFF];
                var_6 = (vec.vz * c - vec.vx * sn) >> 12;
                vec.vx = (vec.vz * sn + vec.vx * c) >> 12;
                vec.vz = var_6;
            }
            vec.vy += s->unk10;
            vec.vx += s->unkC;
            vec.vz += s->unk14;
            func_80040D48(
                i, 0, &vec.vx, &s->unk1C, decoded_motion_frame, s->unk10);
            if (s->unk44[i] >= 0) {
                node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
                    s->unk44[i], (i * 2) | 0x8000, 0, 0);
                node_QueueTableEntryNodePairUnderSlotNode(
                    s->unk44[i], i * 2, 0);
            }
            if (s->unk48[i] >= 0) {
                node_QueueTableEntryNodePairUnderSlotNode(
                    s->unk48[i], (i * 2) | 1, 1);
            }
            if (s->unk3C[i] != 0) {
                gpu_QueueSlotDrawMovesForSelectedSet(
                    i,
                    (((u32 *)s->unk3C[i])[s->unk0 / 8] >> ((s->unk0 % 8) * 4)) &
                        0xF);
            }
        }
    }
    gpu_RunGridDrawPassAndQueueFogMoves(10000);
    s->unk0++;
    if (s->unk0 >= s->unk2) {
        s->unk0 = -1;
    }
    return 1;
}

extern s32 motion_UpdatePlayback(void);

s32 motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush(void) {
    s32 s0;
    g_draw_queue_cursor = g_draw_queue;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = g_gpu_ot_ptr;
    D_800A378C = g_gpu_ot_ptr + 4;
    s0 = motion_UpdatePlayback();
    func_800444E0();
    return s0;
}

void motion_RebasePlayback(s32 a0) {
    s32 *p = &D_800EFAE8.unk2C;
    *p = a0 + *p;
    D_800EFAE8.unk30 = a0 + D_800EFAE8.unk30;
    if (D_800EFAE8.unk34[0]) {
        D_800EFAE8.unk34[0] = a0 + D_800EFAE8.unk34[0];
    }
    if (D_800EFAE8.unk34[1]) {
        D_800EFAE8.unk34[1] = a0 + D_800EFAE8.unk34[1];
    }
    if (D_800EFAE8.unk3C[0]) {
        D_800EFAE8.unk3C[0] = a0 + D_800EFAE8.unk3C[0];
    }
    if (D_800EFAE8.unk3C[1]) {
        D_800EFAE8.unk3C[1] = a0 + D_800EFAE8.unk3C[1];
    }
}

s16 *motion_GetPlaybackViewAngles(void) { return D_800EFAE8.unk24; }

s32 *node_GetRootNodeMatrixTranslation(void) {
    return g_view_camera_root_node.xf.mat.t;
}

void fighter_ClearCpuTally(s32 fighter_index) {
    s32 i;
    Unk80101EC8Record *p = &D_80101EC8[fighter_index];
    i = 0;
    do {
        p->unk_414[i][1] = 0;
        p->unk_414[i][0] = 0;
    } while ((++i) < 8);
}

void fighter_HalveCpuTally(s32 fighter_index) {
    s32 i;
    Unk80101EC8Record *p = &D_80101EC8[fighter_index];
    i = 0;
    do {
        p->unk_414[i][1] = p->unk_414[i][1] >> 1;
    } while (++i < 8);
}

void fighter_InitCpuControl(s32 fighter_index, u16 *arg1, u16 *arg2) {
    Unk80101EC8Record *p = &D_80101EC8[fighter_index];
    Unk8009A8C8Entry *src;
    u8 *var_2;
    u8 base;
    /* idx counts two loops: the eight bytes cleared at 0x444, then the two
     * players (0 = this record, 1 = the opponent's) (Ruling 11,
     * ordinary-c-judge-decidable). */
    s32 idx;
    s32 var_5;
    Unk80101EC8Record *rec;
    u16 *cursor;
    u16 *list;
    s32 var_9;
    s32 bit;
    s32 lo, hi1, hi2;
    /* temp holds six values in turn, each read before the next write: case 2's
     * level D_800A37D2 / 5; case 2's practice level D_800A37D2 / 3 (0 once it
     * reaches 3); case 3's row in D_8009A9B4; a move entry's byte-assembled
     * character mask; the entry's stat bytes e[1] and e[2] (Ruling 11,
     * ordinary-c-judge-decidable). */
    s32 temp;
    u32 var_15;
    s32 lo_val, hi1_val, hi2_val;
    Unk80101EC8Record *other_record;

    p->cpu_profile = p->character_data_index;
    p->unk_438 = p->unk_08;
    switch (g_fight_mode_selector) {
    case 1:
        p->unk_438 = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (p->unk_438 > 0xD00) {
            p->unk_438 = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            fighter_ClearCpuTally(p->index);
        }
        if (g_cpu_profiles[p->cpu_profile].flags & 0x300) {
            src = &D_8009A8C8[p->unk_86][g_fight_stage_command_count - 1];
            p->unk_424 = src->unk0;
            p->unk_3F6 = src->unk1;
        }
        break;
    case 2:
        if (D_800A389A) {
            temp = D_800A37D2 / 5;
            p->unk_438 = temp * 0x180 + 0x280;
            if (p->unk_438 > 0x1000) {
                p->unk_438 = 0x1000;
            }
            if (D_800A37D2 % 5 == 0) {
                fighter_ClearCpuTally(p->index);
            }
        } else {
            temp = D_800A37D2 / 3;
            if (temp >= 3) {
                D_800A37D2 = 0;
                temp = 0;
            }
            p->cpu_profile = 0x19;
            p->base_move_frame_step = (temp + 2) << 10;
            p->unk_438 = 0;
            p->unk_424 = 0;
            p->unk_3F6 = 0x3C - temp * 15;
        }
        break;
    case 3:
        p->cpu_profile = D_8008E778[g_fight_step_table_next_row - 1][0] + 0x1B;
        base = g_fight_step_table_next_row / 10;
        p->unk_438 = base * 16 + 0x80;
        if (g_cpu_profiles[p->cpu_profile].flags & 0x3000) {
            p->unk_438 = base * 16 + 0x180;
        }
        if (g_cpu_profiles[p->cpu_profile].flags & 0x4000) {
            p->unk_438 += 0x200;
        }
        if ((g_fight_step_table_next_row - 1) % 10 == 0) {
            fighter_ClearCpuTally(p->index);
        }
        temp = g_fight_step_table_next_row / 10 * 2;
        if (g_fight_step_table_next_row % 10 == 0) {
            temp--;
        }
        var_2 = D_8009A9B4[temp];
        p->unk_424 = var_2[0];
        p->unk_3F6 = var_2[1];
        break;
    }
    if (fighter_GetSavedCpuBehaviorAdjustFlag() && g_fight_mode_selector != 3) {
        p->unk_438 = p->unk_438 * 11 >> 4;
    }
    if (!(g_cpu_profiles[p->cpu_profile].flags & 0xFF00)) {
        p->unk_424 = 0x11 - (p->unk_438 >> 8);
    }
    p->unk_39A = 0x8000 / p->base_move_frame_step;
    p->unk_3BD = 0x10 - (p->unk_438 >> 8);
    if (g_cpu_profiles[p->cpu_profile].flags & 0x100) {
        g_cpu_profiles[p->cpu_profile].unk3 = (rand() & 3) + 1;
    }
    for (idx = 0; idx < sizeof(p->cpu_probes); idx++) {
        p->cpu_probes[idx] = 0;
    }
    p->unk_3A4 = arg1;
    for (idx = 0; idx < 2; idx++) {
        if (idx) {
            rec = p->other;
            cursor = arg2;
            list = arg2;
            var_9 = rec->character_data_index;
        } else {
            rec = p;
            cursor = arg1;
            list = arg1;
            var_9 = p->cpu_profile;
        }
        rec->scale_distance_offset = (rec->size_scale - 0x1000) * 225 >> 11;
        for (var_5 = 0; var_5 < 3; var_5++) {
            bit = 1 << var_9;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (idx == 0) {
                p->unk_3A8[var_5] = cursor;
            }
            for (; *cursor != 0; cursor++) {
                u8 *e = (u8 *)list + *cursor;
                if (e[4] == 0x40) {
                    temp = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(temp & bit)) {
                        continue;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    temp = e[1];
                    if (temp < lo) {
                        lo = temp;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    temp = e[2];
                    if (hi1 < temp) {
                        hi1 = temp;
                    }
                    var_15 = e[0] & 7;
                    if (hi2 < temp && (e[3] & 0xF) * 4 < 0x10 &&
                        (var_15 < 2 || var_15 == 7)) {
                        hi2 = temp;
                    }
                }
            }
            if (rec->weapon_id >= 6) {
                lo_val = 0;
                hi2_val = 0x7530;
                hi1_val = 0x7530;
            } else {
                /* FAKE: the shared base (rec's scale_distance_offset + 100) is
                 * staged through hi2_val, completed below; a separate base
                 * local is tied to the lh result (target: lh v0; addiu
                 * v1,v0,100); a separate base local: score 6; the base inline
                 * in all three sums: 6-23 (Q129) */
                hi2_val = rec->scale_distance_offset + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
            }
            cursor++;
            rec->unk_3F8[var_5] = lo_val;
            rec->unk_3FE[var_5] = hi1_val;
            rec->unk_404[var_5] = hi2_val;
        }
    }
    other_record = p->other;
    p->last_chosen_script_list_index = -1;
    p->last_chosen_script_index = -1;
    p->unk_428 = -1;
    p->unk_425 = 0;
    p->unk_426 = 0;
    p->cpu_pad_script_cursor = 0;
    p->last_cpu_pad.released = 0;
    p->last_cpu_pad.pressed = 0;
    p->last_cpu_pad.held = 0;
    other_record->unk_440 = 0;
    p->unk_440 = 0;
    other_record->unk_441 = 0;
    p->unk_441 = 0;
    other_record->opponent_angle_from_orientation_abs = 0;
    other_record->opponent_angle_from_orientation = 0;
    p->opponent_angle_from_orientation_abs = 0;
    p->opponent_angle_from_orientation = 0;
    p->cpu_route.count = 0;
    p->unk_39D = 0;
    p->unk_3F5 = 0;
    p->unk_3F4 = 0;
    p->unk_3F3 = 0;
    p->unk_3F2 = 0;
    p->unk_3EE = 0;
    p->unk_3F0 = 0;
    p->cpu_pad_build_count = 0;
    p->unk_39C = 0;
    p->unk_394 = 0;
    p->unk_398 = 0;
    p->unk_3C4 = 0;
    p->unk_3C2 = 0;
    p->unk_3C1 = 0;
    p->unk_3C0 = 0;
    p->unk_440 = 0;
    p->unk_430 = 0;
    p->last_cpu_pad.unheld = -1;
}

s32 fighter_StepCpuPadScript(Unk80101EC8Record *arg0) {
    u8 *p;
    u8 script_delay_left;
    s32 t;
    s32 mask;
    s32 dx, dy, dz;

    script_delay_left = arg0->cpu_pad_script_delay_left;
    p = arg0->cpu_pad_script_cursor;
    if (script_delay_left != 0) {
        if (arg0->unk_46 == 0) {
            arg0->cpu_pad_script_delay_left = script_delay_left - 1;
        }
        goto ret_3c8;
    }
    if (arg0->unk_3BC != 0) {
        goto check_loop;
    }
    {
        u8 idx = arg0->cpu_profile;
        if (idx == 22)
            goto check_loop;
        if ((g_cpu_profiles[idx].flags & 0xBF00) != 0)
            goto check_loop;
        {
            s32 angle_in_range;
            if (arg0->unk_430 & 0x200) {
                angle_in_range =
                    (arg0->opponent_angle_from_orientation_abs < 0x801);
            } else {
                angle_in_range =
                    (arg0->opponent_angle_from_orientation_abs < 0x401);
            }
            if (angle_in_range == 0)
                goto reset_ret_neg1;
        }
        if (arg0->unk_430 & 0x800)
            goto reset_ret_neg1;
        if ((u32)(arg0->unk_425 - 1) < 2U)
            goto reset_ret_neg1;
        if (arg0->unk_442 != 0)
            goto reset_ret_neg1;
        {
            Unk80101EC8Record *other_record = arg0->other;
            if (other_record->move_kind == 0x2D)
                goto reset_ret_neg1;
            dx = other_record->world_pos.x - arg0->chosen_script_start_x;
            dy = other_record->world_pos.z - arg0->chosen_script_start_z;
            dz = arg0->chosen_script_radius;
            if ((dz * dz) >= ((dx * dx) + (dy * dy)))
                goto loop;
        }
    }
    goto reset_ret_neg1;
check_loop:
    if (arg0->unk_3BC != 1)
        goto loop;
    if (arg0->unk_430 & 0x800) {
        goto loop;
    }
reset_ret_neg1:
    arg0->cpu_pad_script_cursor = 0;
    return -1;
sentinel_reset:
    arg0->cpu_pad_script_cursor = 0;
    goto ret_3c8;
loop:
    while (1) {
        t = *p;
        p += 1;
        if (t & 0x80) {
            if (t == 0x80)
                goto sentinel_reset;
            arg0->cpu_pad_script_cursor = p;
            arg0->cpu_pad_script_delay_left = (t & 0x7F) - 1;
            goto ret_3c8;
        }
        mask = 1 << (t & 0xF);
        if (t & 0x10) {
            arg0->cpu_pad_script_held &= ~mask;
        } else {
            arg0->cpu_pad_script_held |= mask;
        }
    }
ret_3c8:
    return arg0->cpu_pad_script_held;
}

void fighter_StartCpuPadScript(Unk80101EC8Record *a0, u8 *a1, s32 a2, s32 a3) {
    a0->cpu_pad_script_cursor = a1;
    a0->unk_3BC = a2;
    a0->cpu_pad_script_delay_left = a3;
    a0->cpu_pad_script_held = 0;
    a0->unk_3CC = -1;
}

/* Four pad-bit numbers; fighter_BuildCpuPad copies the table whole (align 1:
   lwl/lwr) and indexes the copy by Unk80101EC8Record.unk_441. */
typedef struct {
    u8 bit[4];
} PadBitTable;

extern PadBitTable D_800A3258;
extern u8 D_8009A088[];
extern s32 fighter_UpdateCpuNavigationChooseScriptsAndHeldInput(
    Unk80101EC8Record *);
extern void fighter_UpdateCpuProbes(Unk80101EC8Record *);
extern s32 fighter_CalcOpponentDistanceThresholdFromTables(Unk80101EC8Record *);

void fighter_BuildCpuPad(s32 fighter_index, PadState *pad_out) {
    Unk80101EC8Record *rec;
    Unk80101EC8Record *var_1;
    Unk80101EC8Record *var_2;
    PadState pad;
    PadBitTable bits;
    s32 lo, hi;
    /* work holds two values: g_fighter_horizontal_separation - unk_43E, then
     * its sign (work >> 31) (Ruling 11, ordinary-c-judge-decidable). */
    s32 work;
    /* temp holds six values: the ground_y distance, a slot count minus one
     * (clamped at 0), the slot increment (4 or 8, then plus the old count,
     * clamped at 0xFF), the 0x20/0x60 flag, the wrapped ratan2() angle
     * difference and the poll result of
     * fighter_StepCpuPadScript/fighter_UpdateCpuNavigationChooseScriptsAndHeldInput
     * (Ruling 11; per-branch constants: Q20). */
    s32 temp;
    /* temp2 holds three values: the unk7 * 25 >> 3 limit, the
     * fighter_CalcOpponentDistanceThresholdFromTables() result and the
     * SquareRoot0() distance (Ruling 11). */
    s32 temp2;
    /* temp3 holds two values: the least slot count seen (starting at 0x100) and
     * the fighter_CalcOpponentDistanceThresholdFromTables() result plus 800
     * (Ruling 11). */
    s32 temp3;
    /* the counter of each of the four loops, reused loop to loop the way
       SOTN's AddToInventory reuses i for its two loops (Q51) */
    s32 i; /* SOTN: src/dra/5D5BC.c:173 @aa53500 */

    rec = &D_80101EC8[fighter_index];
    rec->unk_3CC = 0;
    if (rec->cpu_pad_build_count & 1) {
        var_2 = rec->other;
        var_1 = rec;
    } else {
        var_1 = rec->other;
        var_2 = rec;
    }
    var_1->unk_441 = var_1->motion_metadata[2] & 0xF;
    var_1->opponent_angle_from_orientation =
        (ratan2(var_2->world_pos.x - var_1->world_pos.x,
                var_2->world_pos.z - var_1->world_pos.z) -
         var_1->orientation.vy) &
        0xFFF;
    if (var_1->opponent_angle_from_orientation > 0x800) {
        var_1->opponent_angle_from_orientation -= 0x1000;
    }
    var_1->opponent_angle_from_orientation_abs =
        var_1->opponent_angle_from_orientation < 0
            ? -var_1->opponent_angle_from_orientation
            : var_1->opponent_angle_from_orientation;
    if (var_1->move_kind == 0x15) {
        var_1->unk_440 = var_1->unk_441;
    }

    rec->unk_430 =
        (rec->unk_430 & 0x40060) |
        ((rand() & 0xFFF) < (rec->unk_438 >> 3) &&
         rec->cpu_pad_build_count >= 0x3D) |
        ((g_cpu_profiles[rec->cpu_profile].flags & 0xFF00)
             ? (rec->unk_3F6 < rec->unk_3F5) << 2
             : ((rand() & 0xFFF) < (rec->unk_438 >> 3) &&
                rec->cpu_pad_build_count >= 0x3D)
                   << 2) |
        (((rand() & 0xFFF) < rec->unk_438 ||
          rec->cpu_pad_build_count < (rec->unk_438 >> 3))
         << 3) |
        (((rand() & 0xFFF) < (rec->unk_438 >> 1) ||
          rec->cpu_pad_build_count < (rec->unk_438 >> 4))
         << 4) |
        ((rec->other->move_kind == 2 || rec->other->move_kind == 0x1B ||
          rec->other->move_kind == 0x28 || rec->other->move_kind == 0x26 ||
          (rec->move_kind == 0x11 &&
           rec->cur_move->last_command_frame != rec->motion_metadata[1] - 1))
         << 7) |
        ((rec->move_kind == 0x13 || rec->move_kind == 0x1B ||
          rec->move_kind == 0x30)
         << 8) |
        ((rec->other->move_kind == 0x13 || rec->other->move_kind == 0x1B ||
          rec->other->move_kind == 0x30)
         << 9) |
        ((rec->move_kind == 6 || rec->move_kind == 4 || rec->move_kind == 0x14)
         << 10) |
        ((rec->other->move_kind == 6 || rec->other->move_kind == 4 ||
          rec->other->move_kind == 0x14)
         << 11) |
        (rec->move_kind == 0x15 ? 0x1000 : 0) |
        (rec->other->move_kind == 0x15 ? 0x2000 : 0) |
        (rec->move_kind == 0x19 ? 0x4000 : 0) |
        (rec->other->move_kind == 0x19 ? 0x8000 : 0) |
        (rec->move_kind == 0x1A ? 0x10000 : 0);
    rec->cpu_pad_build_count++;

    if (rec->move_kind != 2 && rec->move_kind != 0x1B &&
        rec->move_kind != 0x28 && rec->move_kind != 0x26 &&
        rec->move_kind != 0x2C && rec->move_kind != 3 && rec->move_kind != 7) {
        if (rec->unk_3F5 != 0xFF) {
            rec->unk_3F5++;
        }
    } else {
        rec->unk_3F5 = 0;
    }

    if (((D_8009A088[rec->other->weapon_id] >> rec->other->unk_440) & 1) &&
        rec->other->opponent_angle_from_orientation_abs < (rec->unk_438 >> 4) &&
        rec->weapon_id < 6 && (rec->unk_430 & 0x2000) &&
        !(g_cpu_profiles[rec->cpu_profile].flags & 0xBF00)) {
        rec->unk_430 |= 0x20000;
    } else {
        rec->unk_430 &= ~0x20000;
    }

    rec->unk_43E = rec->unk_3F8[rec->unk_86] +
                   (((rec->unk_404[rec->unk_86] - rec->unk_3F8[rec->unk_86]) *
                     g_cpu_profiles[rec->cpu_profile].unk5) >>
                    8);
    temp2 = (g_cpu_profiles[rec->cpu_profile].unk7 * 25u) >> 3;
    work = g_fighter_horizontal_separation - rec->unk_43E;
    if (temp2 < (work >= 0 ? work : -work)) {
        work >>= 31;
        if (work != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work ? -1 : 1;
    } else {
        rec->unk_3F0 = 0;
    }

    temp = rec->other->ground_y - rec->ground_y;
    if (temp < -1000) {
        rec->unk_442 = 1;
    } else if (temp > 1000) {
        rec->unk_442 = 2;
    } else {
        rec->unk_442 = 0;
    }
    if (rec->unk_430 & 0x15500) {
        fighter_UpdateCpuProbes(rec);
    }
    if ((rec->move_frame == 0 &&
         (rec->move_kind == 3 || rec->move_kind == 0x2C)) ||
        (rec->other->move_frame == 0 &&
         (rec->other->move_kind == 0xD || rec->other->move_kind == 0x2C))) {
        if (rec->unk_3F4 != 0xFF) {
            rec->unk_3F4++;
        }
    }

    if (rec->unk_430 & 0x80) {
        lo = rec->other->weapon_hit_start_frames[0] != 0xFF
                 ? rec->other->weapon_hit_start_frames[0]
                 : rec->other->weapon_hit_start_frames[1];
        hi = rec->other->weapon_hit_end_frames[0] != 0xFF
                 ? rec->other->weapon_hit_end_frames[0]
                 : rec->other->weapon_hit_end_frames[1];
    }
    if (!(rec->unk_430 & 0x80) ||
        (rec->move_kind != 0x11
             ? (hi < rec->other->move_frame || lo - rec->other->move_frame >= 9)
             : rec->cur_move->last_command_frame < rec->move_frame)) {
        if (rec->unk_428 != -1) {
            if (rec->unk_424 != 0 &&
                (fighter_GetSavedCpuBehaviorAdjustFlag() == 0 ||
                 g_fight_mode_selector == 3)) {
                s32 var_12;
                s32 var_13;

                temp3 = 0x100;
                i = 0;
                var_12 = -1;
                var_13 = -1;

                for (; i < 8; i++) {
                    if (rec->unk_414[i][0] == rec->unk_428) {
                        var_13 = i;
                    } else {
                        temp = rec->unk_414[i][1] - 1;
                        if (rec->unk_414[i][1] < temp3) {
                            var_12 = i;
                            temp3 = rec->unk_414[i][1];
                        }
                        if (temp < 0) {
                            temp = 0;
                        }
                        rec->unk_414[i][1] = temp;
                    }
                }
                temp = rec->move_kind == 0x11 ? 8 : 4;
                if (var_13 == -1) {
                    rec->unk_414[var_12][0] = rec->unk_428;
                    rec->unk_414[var_12][1] = temp;
                } else {
                    temp += rec->unk_414[var_13][1];
                    if (temp > 0xFF) {
                        temp = 0xFF;
                    }
                    rec->unk_414[var_13][1] = temp;
                }
            }
            rec->unk_428 = -1;
            rec->unk_426 = 0;
            rec->unk_430 &= ~0x60;
        }
    } else {
        if (rec->unk_428 != rec->other->motion_entry_index) {
            if (rec->move_kind == 0x11) {
                rec->unk_428 = 0xFF;
            } else {
                rec->unk_428 = (rec->other->prev_move_kind != 0xE &&
                                rec->other->prev_move_kind != 0x2C)
                                   ? rec->other->motion_entry_index
                                   : 0xFE;
                rec->unk_427 = lo;
                temp2 = fighter_CalcOpponentDistanceThresholdFromTables(rec);
                temp3 = temp2 + 800;
                if (rec->unk_442 != 0 ||
                    ((rec->unk_430 & 0x100) &&
                     rec->opponent_angle_from_orientation_abs > 0x400)) {
                    if (temp2 / 2 >= g_fighter_horizontal_separation) {
                        rec->unk_426 = 1;
                    } else if (temp2 >= g_fighter_horizontal_separation) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                } else {
                    if (temp2 >= g_fighter_horizontal_separation) {
                        rec->unk_426 = 1;
                    } else if (temp3 >= g_fighter_horizontal_separation) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                }
                rec->opponent_snapshot_x = rec->other->world_pos.x;
                rec->opponent_snapshot_z = rec->other->world_pos.z;
                rec->opponent_snapshot_radius = temp3;
            }
            for (i = 0; i < 8; i++) {
                if (rec->unk_414[i][0] == rec->unk_428 &&
                    rec->unk_414[i][1] != 0 &&
                    rec->unk_414[i][1] >= rec->unk_424) {
                    temp = 0x20;
                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {
                        temp = 0x60;
                    }
                    rec->unk_430 |= temp;
                    break;
                }
            }
        }
        if (rec->unk_430 & 0x20) {
            rec->unk_430 |= 0x18;
        }
    }

    if (rec->other->move_kind == 0x12 &&
        rec->other->opponent_angle_from_orientation_abs < 0x80 &&
        g_fighter_horizontal_separation < 0x1194) {
        rec->unk_425 = 1;
    } else {
        rec->unk_425 = 0;
        for (i = 0; i < 12; i++) {
            Obj80106A78 *obj = &g_object_pool[i];

            temp2 = SquareRoot0((rec->world_pos.x - obj->pos.x) *
                                    (rec->world_pos.x - obj->pos.x) +
                                (rec->world_pos.z - obj->pos.z) *
                                    (rec->world_pos.z - obj->pos.z));
            if (obj->kind != -1 && obj->unk_04 != 0 &&
                obj->owner != rec->index) {
                temp = (ratan2(rec->world_pos.x - obj->pos.x,
                               rec->world_pos.z - obj->pos.z) -
                        ratan2(obj->pos.x - obj->prev_pos.x,
                               obj->pos.z - obj->prev_pos.z)) &
                       0xFFF;
                if (temp > 0x800) {
                    temp -= 0x1000;
                }
                if ((temp < 0 ? -temp : temp) < 0x80) {
                    if ((rec->collision_pos.vy - obj->pos.y >= 0)
                            ? (rec->collision_pos.vy - obj->pos.y < 2000)
                            : (obj->pos.y - rec->collision_pos.vy < 2000)) {
                        rec->unk_425 = temp2 < 3000 ? 2 : 1;
                    }
                }
            }
        }
    }

    if ((rec->unk_430 & 1) &&
        !(g_cpu_profiles[rec->cpu_profile].flags & 0xFF00) &&
        rec->unk_426 != 1 && rec->unk_426 != 2 && rec->unk_425 != 1 &&
        rec->unk_425 != 2 &&
        (((rec->unk_430 & 0x80) &&
          ((rec->other->queued_move == 0 && hi < rec->other->move_frame) ||
           lo - rec->other->move_frame >= 9)) ||
         rec->other->move_kind == 0x10 || rec->other->move_kind == 3 ||
         rec->other->move_kind == 7 || rec->other->move_kind == 0x2C ||
         rec->other->move_kind == 0x24 ||
         (!(g_cpu_profiles[rec->cpu_profile].flags & 0x20) &&
          (((rec->unk_430 & 0x80) && rec->unk_426 == 3) ||
           (rec->other->move_kind == 0x2A && rec->other->unk_26C == 0) ||
           (rec->other->move_kind == 0x12 &&
            (!((0x78 >> rec->other->collision_hit_tag) & 1) ||
             rec->other->unk_26C == 0)) ||
           (rec->other->move_kind == 0xB &&
            ((rec->other->carried_object_count == 0 && rec->unk_425 != 1 &&
              rec->unk_425 != 2) ||
             rec->other->unk_26C == 0)))) ||
         ((g_cpu_profiles[rec->cpu_profile].flags & 0x20) &&
          (rec->other->move_kind == 0x25 || rec->other->move_kind == 9 ||
           rec->other->move_kind == 0x16 || rec->other->move_kind == 0x17 ||
           rec->other->move_kind == 0xA ||
           (rec->other->move_kind == 0x22 && rec->unk_442 == 0) ||
           rec->other->opponent_angle_from_orientation_abs > 0x400)))) {
        rec->unk_430 |= 2;
    } else {
        rec->unk_430 &= ~2;
    }

    if (g_cpu_profiles[rec->cpu_profile].flags & 0x8000) {
        rec->unk_430 &= ~0x78;
        if (rec->move_kind != 0x25 && (g_pad_state.held & 0x100)) {
            rec->unk_430 |= 0x40000;
            rec->unk_3F2 = 0;
        }
    }
    if (rec->weapon_id >= 6) {
        rec->unk_430 |= 0x78;
    }

    i = 0;
    do {
        if (rec->cpu_pad_script_cursor != 0) {
            temp = fighter_StepCpuPadScript(rec);
        } else {
            temp = fighter_UpdateCpuNavigationChooseScriptsAndHeldInput(rec);
        }
        i++;
    } while (temp == -1 && i < 4);
    if (temp != -1) {
        pad.held = temp;
    } else {
        pad.held = 0;
    }
    if (pad.held & 0x660) {
        bits = D_800A3258;
        if (rec->move_kind == 0x19) {
            pad.held |= 1 << bits.bit[rec->unk_441];
        } else if (rec->move_kind == 0x13) {
            pad.held |= 4;
        }
    }
    pad.held &= 0xFFFF;
    pad.pressed = pad.held & ~rec->last_cpu_pad.held;
    pad.unheld = ~pad.held & 0xFFFF;
    pad.released = ~pad.held & rec->last_cpu_pad.held;
    pad.type[fighter_index] = 4;
    rec->last_cpu_pad = pad;
    *pad_out = rec->last_cpu_pad;
}

/* 8 {distance, direction} byte pairs (fighter_UpdateCpuProbes) */
extern u8 g_fighter_cpu_probes[8][2];

void fighter_UpdateCpuProbes(Unk80101EC8Record *arg0) {
    s32 segment_start[4];
    s32 segment_end[4];
    s32 var_2[4];
    s32 var_3[4];
    s16 work[4];
    s32 start;
    s32 i;

    start = (arg0->cpu_pad_build_count & 3) * 2;
    for (i = start; i < start + 2; i++) {
        Unk80101EC8Record *obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;

        obj = arg0;
        flags = g_fighter_cpu_probes[i][1] << 8;
        if ((flags & 0x1000) != 0) {
            obj = arg0->other;
        }

        if (arg0->move_kind == 0x13 || arg0->move_kind == 6) {
            flags += obj->orientation.vy;
        } else {
            flags += ratan2(
                g_camera_default_view_record.unk_00.x - obj->world_pos.x,
                g_camera_default_view_record.unk_00.z - obj->world_pos.z);
        }

        sin_p = &Judge[flags & 0xFFF];
        scale = g_fighter_cpu_probes[i][0] << 8;
        x = obj->collision_pos.vx + ((scale * *sin_p) >> 12);
        cos_p = &Judge[(flags + 0x400) & 0xFFF];
        z = obj->collision_pos.vz + ((scale * *cos_p) >> 12);
        segment_start[0] = obj->collision_pos.vx;
        segment_start[1] = obj->collision_pos.vy - 0x320;
        segment_start[2] = obj->collision_pos.vz;
        segment_end[0] = x;
        segment_end[1] = obj->collision_pos.vy - 0x320;
        segment_end[2] = z;

        flags = collision_TestSegmentWithWork(
            segment_start, segment_end, var_2, work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        segment_start[0] = x;
        segment_start[1] = obj->collision_pos.vy - 0x834;
        segment_start[2] = z;
        segment_end[0] = x;
        segment_end[1] = obj->collision_pos.vy + 0x1004;
        segment_end[2] = z;

        flags |= collision_TestSegmentWithWork(
                     segment_start, segment_end, var_3, work, 0x1F8002B8)
                 << 1;
        flags += 1;
        if (flags == 3 && var_3[1] - obj->collision_pos.vy < 5) {
            flags = 0;
        } else if (flags == 4) {
            s32 dx = var_2[0] - obj->collision_pos.vx;
            s32 dz = var_2[2] - obj->collision_pos.vz;
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = obj->collision_pos.vy;
                if ((y - var_3[1] >= 0 ? y - var_3[1] : var_3[1] - y) >=
                    0x3E9) {
                    flags = 5;
                }
            }
        }
        arg0->cpu_probes[i] = flags;
    }
}

#undef sp18
#undef sp1C
#undef sp20
#undef sp28
#undef sp2C
#undef sp30
#undef sp38
#undef sp3C
#undef sp40
#undef sp48
#undef sp4C
#undef sp50
#undef sp58
#undef sp5C
#undef sp60
#undef sp68
#undef sp70
#undef sp78
/* Three byte tables (8, 8 and 16 bytes). D_8009A838 is read signed (lb) here
 * and in fighter_UpdateCpuNavigationChooseScriptsAndHeldInput; the other two
 * unsigned (lbu). */
extern u8 D_8009A830[];
extern s8 D_8009A838[];
extern u8 D_8009A840[];

/* fighter_CalcOpponentDistanceThresholdFromTables: per-record table lookup on
 * arg0->other: its row (motion_metadata[3] * 40), plus a D_8009A830 /
 * D_8009A838 (by arg0->move_is_common) or D_8009A840 (when
 * weapon_hit_end_frames[0] is 0xFF) term, plus its scale_distance_offset and
 * 300. */
s32 fighter_CalcOpponentDistanceThresholdFromTables(Unk80101EC8Record *arg0) {
    Unk80101EC8Record *a2 = arg0->other;
    s32 base = a2->motion_metadata[3] * 40;

    base += a2->weapon_hit_end_frames[0] != 0xFF
                ? (arg0->move_is_common == 0 ? D_8009A830[a2->weapon_id] * 2
                                             : D_8009A838[a2->weapon_id] * 8)
                : D_8009A840[a2->unk_14] * 2;
    return base + arg0->other->scale_distance_offset + 0x12C;
}

extern s32 D_8009AA50[];

s32 fighter_BuildCpuRoutePadWithSurfaceProbe(
    Unk80101EC8Record *arg0, s32 route_target_x, s32 route_target_z, s32 arg3) {
    s32 sp10;
    s32 focus_bearing;
    s32 angle_sector;
    s32 sector_index;
    s32 var_v0;

    focus_bearing =
        ratan2(g_camera_default_view_record.unk_00.x - arg0->world_pos.x,
               g_camera_default_view_record.unk_00.z - arg0->world_pos.z);
    var_v0 = focus_bearing - ratan2(route_target_x - arg0->world_pos.x,
                                    route_target_z - arg0->world_pos.z);
    var_v0 -= 0x100;
    angle_sector = (s32)var_v0 >> 9;
    sector_index = angle_sector & 7;
    sp10 = sector_index;
    if ((arg3 == 0) && !(angle_sector & 1)) {
        if (arg0->cpu_pad_build_count & 0x10) {
            sp10 = sector_index + 1;
            if (sp10 >= 8) {
                sp10 = 0;
            }
        } else {
            sp10 = sector_index - 1;
            if (sp10 < 0) {
                sp10 = 7;
            }
        }
    }
    var_v0 = D_8009AA50[sp10 & 7];
    if (arg3 == 1) {
        var_v0 |= 4;
    }
    if (fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow(
            arg0, &sp10) != 0) {
        var_v0 |= 8;
    }
    return var_v0;
}

typedef struct {
    s32 x, y, z, w;
} Vec4_571C0;

s32 fighter_BuildCpuOrbitRoute(Unk80101EC8Record *obj) {
    Vec4_571C0 var_0;
    Vec4_571C0 top;
    Vec4_571C0 left;
    Vec4_571C0 var_3;
    s32 var_4[4];
    s16 work[4];
    s32 ret;
    s8 var_7;
    /* temp holds two values: the count of probes accepted at angle
     * opponent_bearing - ang (var_7 counts those at opponent_bearing + ang),
     * then which count the waypoint loop takes (0: temp's, stepping by -ang; 1:
     * var_7's, by +ang; per-branch constants, Q20) (Ruling 11,
     * ordinary-c-judge-decidable). */
    s8 temp;
    u8 var_9;
    u8 var_10;
    s32 ang;
    s32 var_12;
    s32 a;
    Unk80101EC8Record *p;
    s32 dx;
    s32 dz;
    s32 x;
    s32 z;

    temp = 0;
    var_7 = 0;
    var_10 = 1;
    var_9 = 1;
    ret = 0;
    left.x = obj->collision_pos.vx;
    left.y = obj->collision_pos.vy - 5;
    var_12 = g_fighter_horizontal_separation + 800;
    left.z = obj->collision_pos.vz;
    var_3 = left;
    for (ang = 0x200; ang <= 0x800; ang += 0x200) {
        if (var_9) {
            p = obj->other;
            a = p->opponent_bearing + ang;
            var_9 = 0;
            dx = var_12 * Judge[a & 0xFFF];
            dz = var_12 * Judge[(a + 0x400) & 0xFFF];
            x = p->collision_pos.vx + (dx >> 12);
            z = p->collision_pos.vz + (dz >> 12);
            var_0.x = x;
            var_0.y = obj->collision_pos.vy - 5;
            var_0.z = z;
            top.x = x;
            top.y = obj->collision_pos.vy + 5;
            top.z = z;
            if (collision_TestSegmentWithWork(
                    &var_0.x, &top.x, var_4, work, 0x1F8002B8) != 0) {
                var_9 = collision_TestSegmentWithWork(
                            &left.x, &var_0.x, var_4, work, 0x1F8002B8) == 0;
            }
            if (var_9) {
                left = var_0;
                var_7++;
            }
        }
        if (var_10) {
            p = obj->other;
            a = p->opponent_bearing - ang;
            var_10 = 0;
            dx = var_12 * Judge[a & 0xFFF];
            dz = var_12 * Judge[(a + 0x400) & 0xFFF];
            x = p->collision_pos.vx + (dx >> 12);
            z = p->collision_pos.vz + (dz >> 12);
            var_0.x = x;
            var_0.y = obj->collision_pos.vy - 5;
            var_0.z = z;
            top.x = x;
            top.y = obj->collision_pos.vy + 5;
            top.z = z;
            if (collision_TestSegmentWithWork(
                    &var_0.x, &top.x, var_4, work, 0x1F8002B8) != 0) {
                var_10 = collision_TestSegmentWithWork(
                             &var_3.x, &var_0.x, var_4, work, 0x1F8002B8) == 0;
            }
            if (var_10) {
                var_3 = var_0;
                temp++;
            }
        }
    }
    if (var_7 != 0 || temp != 0) {
        if (var_7 == temp) {
            if (rand() & 1) {
                var_7 = 0;
            } else {
                temp = 0;
            }
        }
        if (var_7 < temp) {
            var_7 = temp;
            temp = 0;
        } else {
            temp = 1;
        }
        ret = var_7--;
        for (ang = 0x200; var_7 >= 0; var_7--, ang += 0x200) {
            s32 base = obj->other->opponent_bearing;
            if (temp != 0) {
                a = base + ang;
            } else {
                a = base - ang;
            }
            obj->cpu_route.node[var_7].x =
                obj->other->collision_pos.vx +
                ((g_fighter_horizontal_separation * Judge[a & 0xFFF]) >> 12);
            obj->cpu_route.node[var_7].z =
                obj->other->collision_pos.vz +
                ((g_fighter_horizontal_separation *
                  Judge[(a + 0x400) & 0xFFF]) >>
                 12);
            obj->cpu_route.node[var_7].kind = 2;
        }
        obj->unk_398 = 0;
        obj->unk_3A0 = obj->cpu_route.node[0].x;
        obj->unk_3A2 = obj->cpu_route.node[0].z;
        obj->unk_39E = obj->cpu_route.node[0].kind;
    }
    return ret;
}

s32 math_IntersectSegments(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                           s32 arg5, s32 arg6, s32 arg7, s32 *arg8, s32 *arg9) {
    s32 var_0;
    s32 x1;
    s32 dx1;
    s32 dy1;
    s32 dx2;
    s32 dy2;
    s32 var_6;
    s32 var_7;
    s32 var_8;
    s32 y;

    x1 = arg0;
    x1 >>= 3;
    arg2 >>= 3;
    arg1 >>= 3;
    arg3 >>= 3;
    dx1 = arg2 - x1;
    dy1 = arg3 - arg1;
    arg4 >>= 3;
    arg6 >>= 3;
    arg5 >>= 3;
    arg7 >>= 3;
    dx2 = arg6 - arg4;
    dy2 = arg7 - arg5;
    if ((arg4 == arg6) && (arg1 == arg3)) {
        *arg8 = arg6;
        *arg9 = arg3;
    } else if ((x1 == arg2) && (arg5 == arg7)) {
        *arg8 = arg2;
        *arg9 = arg7;
    } else if ((arg4 == arg6) && (x1 != arg2)) {
        *arg8 = arg6;
        *arg9 = arg1 + (dy1 * (arg6 - x1)) / dx1;
    } else if ((x1 == arg2) && (arg4 != arg6)) {
        *arg8 = x1;
        *arg9 = arg5 + (dy2 * (x1 - arg4)) / dx2;
    } else if ((arg1 == arg3) && (arg5 != arg7)) {
        *arg9 = arg3;
        *arg8 = arg4 + (dx2 * (arg3 - arg5)) / dy2;
    } else if ((arg5 == arg7) && (arg1 != arg3)) {
        *arg9 = arg5;
        *arg8 = x1 + (dx1 * (arg5 - arg1)) / dy1;
    } else {
        if (arg2 == x1) {
            return 0;
        }
        if (arg6 == arg4) {
            return 0;
        }
        var_0 = (dy1 << 7) / dx1;
        var_6 = (dy2 << 7) / dx2;
        if (var_0 == var_6) {
            return 0;
        }
        var_7 = (((arg1 * arg2) - (arg3 * x1)) << 7) / dx1;
        var_8 = (((((arg5 * arg6) - (arg7 * arg4)) << 7) / dx2) - var_7) /
                (var_0 - var_6);
        *arg8 = var_8;
        *arg9 = ((var_8 * var_0) + var_7) >> 7;
    }
    if (!((((*arg8 - x1) >= -50) || ((*arg8 - arg2) >= -50)) &&
          (((x1 - *arg8) >= -50) || ((arg2 - *arg8) >= -50)) &&
          ((y = *arg9, ((arg1 - y) >= -50)) || ((arg3 - y) >= -50)) &&
          (((y - arg1) >= -50) || ((y - arg3) >= -50)) &&
          (((*arg8 - arg4) >= -50) || ((*arg8 - arg6) >= -50)) &&
          (((arg4 - *arg8) >= -50) || ((arg6 - *arg8) >= -50)) &&
          (((arg5 - y) >= -50) || ((arg7 - y) >= -50)) &&
          (((y - arg5) >= -50) || ((y - arg7) >= -50)))) {
        return 0;
    }
    *arg8 = *arg8 << 3;
    *arg9 = *arg9 << 3;
    return 1;
}

extern s32 math_IntersectSegments(
    s32, s32, s32, s32, s32, s32, s32, s32, s32 *, s32 *);

s32 fighter_FindNearestNavCrossing(
    Unk80101EC8Record *arg0, NavPolySet *arg1, s32 arg2, s32 arg3) {
    s32 sp28;
    s32 sp2C;
    s32 best;
    s16 i;
    s16 j;
    s16 k;
    s16 n;
    NavPoly *poly;
    s32 dx;
    s32 dy;
    s32 d;

    best = 100000;
    for (i = 0; i < arg1->npolys; i++) {
        poly = &arg1->polys[i];
        n = poly->nvtx;
        if (poly->flags & 0x80) {
            n = poly->nvtx - 1;
        }
        for (j = 0; j < n; j++) {
            k = j + 1;
            if (!(k < poly->nvtx)) {
                k = 0;
            }
            if (math_IntersectSegments(
                    arg0->world_pos.x, arg0->world_pos.z, arg2, arg3,
                    poly->vtx[j][0], poly->vtx[j][1], poly->vtx[k][0],
                    poly->vtx[k][1], &sp28, &sp2C) != 0) {
                dx = sp28 - arg0->world_pos.x;
                dy = sp2C - arg0->world_pos.z;
                d = SquareRoot0(dx * dx + dy * dy);
                if (d < best) {
                    best = d;
                    arg0->cpu_route.poly = i;
                    arg0->cpu_route.vtx = j;
                }
            }
        }
    }
    return best;
}

/* Per-vertex neighbour-angle midpoint: computes the bisector direction
 * at vertex arg1 of polygon arg0 and writes the offset point into *arg2 /
 * *arg3. */
void fighter_GetNavCornerPoint(NavPoly *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s16 prev;
    s16 next;
    s16 cx, cy;
    s32 ang_prev, ang_next, scale;
    s16 ang_mid;

    prev = arg1 - 1;
    cx = arg0->vtx[arg1][0];
    cy = arg0->vtx[arg1][1];
    if (prev < 0) {
        prev = arg0->nvtx - 1;
    }
    next = arg1 + 1;
    if (next >= arg0->nvtx) {
        next = 0;
    }
    ang_prev = ratan2(arg0->vtx[prev][0] - cx, arg0->vtx[prev][1] - cy) & 0xFFF;
    ang_next = ratan2(arg0->vtx[next][0] - cx, arg0->vtx[next][1] - cy) & 0xFFF;
    if (ang_next < ang_prev) {
        s32 antipode = ang_prev + 0x800;
        ang_mid = antipode - (ang_prev - ang_next) / 2;
    } else {
        ang_mid = (ang_next - ang_prev) / 2 + ang_prev;
    }
    scale = arg0->margin * 40;
    *arg2 = cx + ((scale * Judge[ang_mid & 0xFFF]) >> 12);
    *arg3 = cy + ((scale * Judge[(ang_mid + 0x400) & 0xFFF]) >> 12);
}

/* Walks the vertex ring of arg0's polygon arg1->polys[arg0->cpu_route.poly]
 * both ways from vertex cpu_route.vtx, each way starting at arg0->world_pos.
 * While math_IntersectSegments reports that the segment from the way's current
 * point to (arg2, arg3) crosses a polygon edge, the way steps to the next
 * vertex's fighter_GetNavCornerPoint point, appends it (up to 8) to its
 * CpuRoute and adds the step's SquareRoot0 length to its sum; a clear segment
 * adds that segment's length and ends the way. The way with the smaller sum is
 * appended, last point first, to arg0->cpu_route. */
void fighter_AppendNavDetour(
    Unk80101EC8Record *arg0, NavPolySet *arg1, s32 arg2, s32 arg3) {
    CpuRoute var_0[2];
    s16 ofs0_x;
    s16 ofs0_z;
    s16 ofs1_x;
    s16 ofs1_z;
    s32 var_5;
    s32 var_6;
    s16 var_7;
    s16 var_8;
    s16 var_9;
    s16 var_10;
    u8 var_11;
    s8 var_12;
    s8 var_13;
    s16 iter;
    s16 var_15;
    s32 var_16;
    s32 var_17;
    u8 var_18;
    u8 var_19;
    u8 var_20;
    /* the counter of the edge scan and the route copy, reused loop to loop
     * the way SOTN's DebugCaptureScreen reuses its i for its file-search loop
     * and its row countdown (Q51) */
    s16 i; /* SOTN: src/dra/42398.c:75 @aa53500 */
    s16 next;
    s16 ax;
    s16 az;
    s16 bx;
    s16 bz;
    NavPoly *poly;
    /* route holds three values: &var_0[0] in the first corner block, &var_0[1]
     * in the second corner block, and the cheaper of the two for the copy loop
     * (Ruling 11, reused-local-necessity). */
    CpuRoute *route;

    var_11 = 1;
    var_18 = 1;
    poly = &arg1->polys[arg0->cpu_route.poly];
    var_7 = var_9 = arg0->world_pos.x;
    var_8 = var_10 = arg0->world_pos.z;
    var_0[1].count = 0;
    var_0[0].count = 0;
    var_17 = 0;
    var_16 = 0;
    var_12 = arg0->cpu_route.vtx;
    var_13 = var_12 + 1;
    if (!(var_13 < poly->nvtx)) {
        if (poly->flags & 0x80) {
            var_18 = 0;
        } else {
            var_13 = 0;
        }
    }
    var_15 = poly->nvtx;
    if (poly->flags & 0x80) {
        var_15--;
    }
    for (iter = 0; iter < var_15; iter++) {
        /* var_29 holds four values, each the address of an x/z pair: the
         * edge's start (i) and end (next) in the edge scan, then the corners
         * at var_12 and var_13 (Ruling 11,
         * reused-local-necessity). */
        s16 *var_29;
        /* node holds two values: the waypoint appended to &var_0[0], then
         * the one appended to &var_0[1] (Ruling 11, reused-local-necessity).
         */
        CpuWaypoint *node;

        var_20 = 0;
        var_19 = 0;
        for (i = 0; i < var_15; i++) {
            next = i + 1;
            if (!(next < poly->nvtx)) {
                next = 0;
            }
            var_29 = poly->vtx[i];
            ax = var_29[0];
            az = var_29[1];
            fighter_GetNavCornerPoint(poly, i, &ofs0_x, &ofs0_z);
            var_29 = poly->vtx[next];
            bx = var_29[0];
            bz = var_29[1];
            fighter_GetNavCornerPoint(poly, next, &ofs1_x, &ofs1_z);
            if (var_11 && !var_19) {
                if ((var_7 != ofs0_x || var_8 != ofs0_z) &&
                    (var_7 != ofs1_x || var_8 != ofs1_z)) {
                    if (math_IntersectSegments(var_7, var_8, arg2, arg3, ax, az,
                                               bx, bz, &var_5, &var_6)) {
                        var_19 = 1;
                    }
                }
            }
            if (var_18 && !var_20) {
                if ((var_9 != ofs0_x || var_10 != ofs0_z) &&
                    (var_9 != ofs1_x || var_10 != ofs1_z)) {
                    if (math_IntersectSegments(var_9, var_10, arg2, arg3, ax,
                                               az, bx, bz, &var_5, &var_6)) {
                        var_20 = 1;
                    }
                }
            }
            if (var_19 && var_20) {
                break;
            }
        }
        if (var_11) {
            if (var_19) {
                s32 c;

                var_29 = poly->vtx[var_12];
                var_16 +=
                    SquareRoot0((var_29[0] - var_7) * (var_29[0] - var_7) +
                                (var_29[1] - var_8) * (var_29[1] - var_8));
                fighter_GetNavCornerPoint(poly, var_12, &var_7, &var_8);
                route = &var_0[0];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    var_11 = 0;
                }
                node = &route->node[c];
                node->x = var_7;
                node->z = var_8;
                node->kind = poly->kind;
                if (--var_12 < 0) {
                    if (poly->flags & 0x80) {
                        var_11 = 0;
                        var_16 = 100000;
                    } else {
                        var_12 = poly->nvtx - 1;
                    }
                }
            } else {
                var_16 += SquareRoot0((arg2 - var_7) * (arg2 - var_7) +
                                      (arg3 - var_8) * (arg3 - var_8));
                var_11 = 0;
            }
        }
        if (var_18) {
            if (var_20) {
                s32 c;

                var_29 = poly->vtx[var_13];
                var_17 +=
                    SquareRoot0((var_29[0] - var_9) * (var_29[0] - var_9) +
                                (var_29[1] - var_10) * (var_29[1] - var_10));
                fighter_GetNavCornerPoint(poly, var_13, &var_9, &var_10);
                route = &var_0[1];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    var_18 = 0;
                }
                node = &route->node[c];
                node->x = var_9;
                node->z = var_10;
                node->kind = poly->kind;
                if (!(++var_13 < poly->nvtx)) {
                    if (poly->flags & 0x80) {
                        var_18 = 0;
                        var_17 = 100000;
                    } else {
                        var_13 = 0;
                    }
                }
            } else {
                var_17 += SquareRoot0((arg2 - var_9) * (arg2 - var_9) +
                                      (arg3 - var_10) * (arg3 - var_10));
                var_18 = 0;
            }
        }
        if (!var_11 && !var_18) {
            break;
        }
    }
    if (var_16 < var_17) {
        route = &var_0[0];
    } else {
        route = &var_0[1];
    }
    for (i = route->count - 1; i >= 0; i--) {
        arg0->cpu_route.node[arg0->cpu_route.count] = route->node[i];
        arg0->cpu_route.count++;
    }
}

extern u8 D_8009A870[];
extern u8 D_8009A874[];
extern u8 D_8009A878[];
extern u8 D_8009A880[];
extern u8 D_8009A888[];
extern u8 D_8009A890[];
extern u8 D_8009A898[];
extern u8 D_8009A89C[];
extern u8 D_8009A8A4[];
extern u8 D_8009A8AC[];
extern u8 D_8009A8B4[];
extern u8 D_8009A8C0[];
extern u8 D_8009A850[8][4];
extern NavPolySet D_8009A658[];
extern u16 D_8009A928[][23];
extern u8 D_8009A9DC[][3];
extern s32 D_8009A9F0[][8];
extern u8 D_800A325C[4];
extern u8 D_800A3260[4];
extern void fighter_AppendNavDetour(
    Unk80101EC8Record *, NavPolySet *, s32, s32);

#define CPU_SQ(x) ((x) * (x))

s32 fighter_UpdateCpuNavigationChooseScriptsAndHeldInput(Unk80101EC8Record *p) {
    s32 wx;
    u8 *var_1;
    s32 wz;
    s8 var_3;
    s16 var_4;
    s32 var_5;
    s32 var_6;
    u8 *var_7;
    u8 *var_8;
    u8 *var_9;
    u8 *var_10;
    u8 mode;
    /* work1 holds nine values in turn, each read before the next write: the
     * cpu_probes[5] == 0 flag of the state-0x15 script choice; the stage
     * distance base (100000, or D_8009A838[stage] * 8) of the D_8009A850 scan;
     * cpu_probes[6] for the lim chain; cpu_probes[6] again for the waypoint
     * script; the x of waypoint 1; the cpu_probes[1] == 0 flag of the 0x394
     * choice; case 2's D_8009A9F0 pattern word, shifted in place; a script
     * entry's low distance bound (e[1] * 40, then adjusted); the state-0x15
     * script's near bound (the opponent's unk_3F8 entry, or its unk_404 entry +
     * 300) (Ruling 11, reused-local-necessity). */
    s32 work1;
    /* work2 holds seven values in turn, each read before the next write: the
     * cpu_probes[1] == 0 flag of the state-0x15 script choice; a D_8009A850
     * entry's distance; the pace byte cpu_probes[0]; the z of waypoint 1; the
     * cpu_probes[5] == 0 flag of the 0x394 choice; the best weighted random
     * value so far (Q75 constant start + copy; compared as an s16, `sll; sra
     * 16`, Q82); case 2's nibble count, counted down (Ruling 11,
     * reused-local-necessity). */
    s32 work2;
    /* work3 holds thirteen values in turn, each read before the next write: the
     * script side bit (opponent unk_AF & 1, possibly inverted); cpu_probes[3]
     * == 0; the opponent_angle_from_orientation, wrapped to +-0x800; the
     * state-0x11 threshold (0x1000 - (stance sum << 8)), scaled by unk_438 >>
     * 12; the forced-scan flag of the D_8009A850 scan; the "longer than lim"
     * flag; the bearing to the next waypoint, wrapped; the 0x394 choice's coin
     * bit, stepped per try; the 0x394 slot (unk_394, or a D_800A325C /
     * D_800A3260 entry); case 3's column in D_8009A928; the entry-type mask; a
     * script entry's character mask; the entry's accept flag. Read-before-write
     * kept from the original (Q74): the 0x394 slot switch below reads work3
     * with no write on the path unk_39C == 1, opponent state neither 0x19 nor
     * 0x1A (target 0x80059D18 -> 0x80059D6C -> 0x80059DB0; $s3 read by the
     * sltiu at 0x80059D70 and the sll at 0x80059DB4), switching on whatever
     * earlier work left there (that read is excluded from the value grouping
     * above) (Ruling 11, reused-local-necessity). */
    s32 work3;
    /* work4 holds three values in turn, each read before the next write: the
     * D_8009A850 scan index; a copy of var_53 taken in the walk branch
     * (Q34), counted down; the script-list entry index (Ruling 11,
     * reused-local-necessity). */
    s32 work4;
    s32 var_16;
    s32 hi;
    s16 var_18;
    s32 var_19;
    s32 var_20;
    s32 var_21;
    s32 var_22;
    s32 var_23;
    s16 var_24;
    NavPolySet *var_25;
    s32 tx, tz;
    s32 r;
    s32 var_29;
    u16 *list;
    u8 *e;
    u8 *var_32;
    /* FAKE: second handle to var_32's value, see `q = var_32;` (read through
     * var_32: score 58; e[4]/e[8..5]: 18, also with wrapper removed) */
    u8 *q;
    u16 off;
    s16 var_35;
    u8 var_36;
    s8 var_37;
    s32 var_38;
    s32 cnt;
    s32 var_40;
    s32 var_41;
    s32 lim;
    s8 var_43;
    s32 var_44;

    if (p->other->move_kind == 4 || p->other->move_kind == 0x14) {
        return 0;
    }
    if (p->cpu_profile != 0x16 &&
        ((p->unk_430 & 0x15100) ||
         (p->move_kind == 0xD && (p->unk_426 == 4 || p->unk_425 == 4)))) {
        var_7 = 0;
        if ((p->unk_426 == 1 && p->other->move_frame + 1 >= p->unk_427) ||
            p->unk_425 == 2) {
            if (p->unk_430 & 8) {
                u8 *tbl[2];
                s32 f;
                tbl[0] = D_8009A874;
                tbl[1] = D_8009A870;
                work3 = p->other->unk_AF & 1;
                f = p->unk_430;
                if (!(((f & 0x20) ||
                       ((f & 0x10) && p->unk_3F3 % ((p->unk_438 >> 8) + 2) !=
                                          (p->unk_438 >> 8) + 1)) &&
                      (!(g_cpu_profiles[p->cpu_profile].flags & 0xFF00) ||
                       (f & 0x40))) ||
                    (fighter_GetSavedCpuBehaviorAdjustFlag() &&
                     g_fight_mode_selector != 3)) {
                    work3 = !work3;
                }
                var_7 = tbl[work3];
                mode = 3;
            }
            if (p->move_kind == 0xD) {
                var_7 = 0;
                p->unk_3CC = 0;
                p->unk_426 = 4;
                p->unk_425 = 4;
                p->unk_3F3++;
            }
        } else if (p->move_kind == 0x15) {
            mode = 4;
            work3 = p->cpu_probes[3] == 0;
            work1 = p->cpu_probes[5] == 0;
            work2 = p->cpu_probes[1] == 0;
            if (p->unk_426 == 2) {
                if (p->opponent_snapshot_radius * p->opponent_snapshot_radius <
                    CPU_SQ(p->opponent_snapshot_x - p->world_pos.x) +
                        CPU_SQ(p->opponent_snapshot_z - p->world_pos.z)) {
                    p->unk_3CC = 0;
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                } else if (work3 && (p->unk_430 & 8) &&
                           (!(g_cpu_profiles[p->cpu_profile].flags & 0x8F00) ||
                            ((g_cpu_profiles[p->cpu_profile].flags & 0x300) &&
                             g_fight_stage_command_count >= 6))) {
                    var_7 = D_8009A890;
                }
            }
            if (p->unk_425 == 1 ||
                (p->unk_426 == 1 && p->other->unk_441 == 2 &&
                 p->unk_427 - p->other->move_frame >= 6) ||
                (p->unk_426 == 2 && var_7 == 0)) {
                if ((p->unk_430 & 8) &&
                    (!(g_cpu_profiles[p->cpu_profile].flags & 0xBF00) ||
                     ((g_cpu_profiles[p->cpu_profile].flags & 0x300) &&
                      g_fight_stage_command_count >= 7))) {
                    if (p->other->opponent_angle_from_orientation > 0) {
                        if (work1) {
                            var_7 = D_8009A880;
                        } else if (work2) {
                            var_7 = D_8009A878;
                        }
                    } else {
                        if (work2) {
                            var_7 = D_8009A878;
                        } else if (work1) {
                            var_7 = D_8009A880;
                        }
                    }
                    if (var_7 == 0) {
                        p->unk_426 = 1;
                        p->unk_425 = 2;
                        return -1;
                    }
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                }
            }
        }
        if (var_7 != 0) {
            fighter_StartCpuPadScript(p, var_7, mode, 0);
            return p->unk_3CC;
        }
    }

    if ((p->unk_430 & 0x400) && p->unk_441 != 2) {
        if (g_cpu_profiles[p->cpu_profile].flags & 0x8000) {
            p->unk_3CC = 0x2000;
        } else if (p->unk_430 & 8) {
            work3 = p->opponent_angle_from_orientation;
            if (p->unk_441 == 0) {
                work3 = (work3 + 0x800) & 0xFFF;
                if (work3 > 0x800) {
                    work3 -= 0x1000;
                }
            }
            if (!(g_cpu_profiles[p->cpu_profile].flags & 0xF00)) {
                if (work3 < 0) {
                    if (p->cpu_probes[1] == 0 || p->cpu_probes[1] == 3) {
                        p->unk_3CC = 0x4000;
                    }
                } else {
                    if (p->cpu_probes[5] == 0 || p->cpu_probes[5] == 3) {
                        p->unk_3CC = 0x1000;
                    }
                }
            }
            if (!(p->unk_425 == 1 || p->unk_425 == 2) && p->unk_426 != 2 &&
                (p->unk_3CC == 0 || p->unk_442 != 0 ||
                 p->other->move_kind == 2 || p->other->move_kind == 0x29 ||
                 p->other->move_kind == 0x13 || p->other->move_kind == 6 ||
                 p->other->unk_404[p->other->unk_86] <
                     g_fighter_horizontal_separation)) {
                if (p->opponent_angle_from_orientation_abs < 0x400 &&
                    p->cpu_probes[3] == 0) {
                    p->unk_3CC = 0x8000;
                } else {
                    p->unk_3CC = 0x2000;
                }
            }
        }
    } else {
        u16 state;

        state = p->move_kind;
        if (state == 0xF || state == 0x1C || state == 0x1D || state == 0x1E ||
            state == 0x1F || state == 0x20 || state == 0x21) {
            if (p->move_kind == 0x1D &&
                (rand() & 0xFF) < g_cpu_profiles[p->cpu_profile].unk4 &&
                p->cpu_probes[3] == 0) {
                p->unk_3CC = 0x8000;
            } else {
                var_20 = 0x80;
                if (p->cpu_pad_build_count % (0x12 - (p->unk_438 >> 8)) == 0) {
                    var_20 = 0x20;
                }
                p->unk_3CC = var_20;
                if (p->cpu_probes[3] != 0) {
                    if (p->cpu_probes[2] != 0) {
                        p->unk_3CC = var_20 | 0x1000;
                    } else if (p->cpu_probes[4] != 0) {
                        p->unk_3CC = var_20 | 0x4000;
                    }
                } else if (p->cpu_probes[7] == 0) {
                    p->unk_3CC = (p->cpu_profile & 1) ? var_20 | 0x1000
                                                      : var_20 | 0x4000;
                }
            }
        } else if (state == 0x11 && p->index != g_fight_paired_move_initiator &&
                   p->move_frame == p->cur_move->last_command_frame - 1) {
            {
                s32 a, b, c;
                a = p->unk_26E;
                b = p->unk_270;
                c = p->unk_272;
                work3 =
                    0x1000 - (((p->unk_26C == 0 ? a + 4 + b : a + b) + c) << 8);
                work3 = (p->unk_438 * work3) >> 12;
                if ((rand() & 0xFFF) < work3) {
                    p->unk_3CC = 0x20;
                } else if (p->unk_430 & 0x20) {
                    p->unk_3CC = 0x20;
                }
            }
        } else {
            if ((p->ground_y - p->collision_pos.vy >= 0
                     ? p->ground_y - p->collision_pos.vy
                     : p->collision_pos.vy - p->ground_y) < 200 &&
                !(g_cpu_profiles[p->cpu_profile].flags & 0x8C00) &&
                ((!fighter_GetSavedCpuBehaviorAdjustFlag() &&
                  g_fight_mode_selector != 3) ||
                 g_fight_mode_selector == 3)) {
                work3 = 0;
                if ((p->unk_426 == 1 || p->unk_425 == 2) && (p->unk_430 & 8)) {
                    work3 = 1;
                }
                var_41 = 0;
                if ((rand() & 0xFFF) < (p->unk_438 >> 3)) {
                    var_41 = p->cpu_pad_build_count > 0x3C;
                }
                if (p->weapon_id >= 6) {
                    work1 = 100000;
                } else {
                    work1 = D_8009A838[p->weapon_id] * 8;
                }
                for (work4 = 0;
                     work4 < sizeof(D_8009A850) / sizeof(D_8009A850[0]);
                     work4++) {
                    if (!(D_8009A850[work4][3] & 1) ||
                        p->move_frame >= p->cur_move->last_command_frame - 2 ||
                        work3) {
                        work2 = D_8009A850[work4][2] * 16 + work1 +
                                p->scale_distance_offset;
                        if ((g_fighter_horizontal_separation < work2 &&
                             (!(D_8009A850[work4][3] & 8) ||
                              p->opponent_angle_from_orientation_abs < 0x100) &&
                             (var_41 || (D_8009A850[work4][3] & 4))) ||
                            work3) {
                            if (D_8009A850[work4][0] == p->move_kind &&
                                ((var_36 = D_8009A850[work4][1]) == 0xFF ||
                                 var_36 == p->other->move_kind)) {
                                if (D_8009A850[work4][3] & 2) {
                                    var_21 = 0x40;
                                    if (p->cpu_pad_build_count & 1) {
                                        var_21 = 0x20;
                                    }
                                    p->unk_3CC = var_21;
                                } else if (
                                    (g_cpu_profiles[p->cpu_profile].flags &
                                     0x20) &&
                                    !(p->unk_440 == 3 || p->unk_440 == 4)) {
                                    p->unk_3CC =
                                        (p->cpu_pad_build_count & 1) * 8;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 0x4108) == 8) {
        u16 state;

        state = p->move_kind;
        if (state == 3 || state == 0x2C || state == 7 ||
            (p->unk_426 == 1 || p->unk_426 == 2) ||
            (p->unk_425 == 1 || p->unk_425 == 2)) {
            p->unk_39D = 0;
            p->cpu_route.count = 0;
            p->unk_398 = 0;
            return 0;
        }
    }

    if (p->unk_430 & 0x5100) {
        var_25 = &D_8009A658[g_fight_stage_index];
        if (p->unk_39D == 0) {
            tx = p->other->world_pos.x;
            tz = p->other->world_pos.z;
            var_38 = 1;
        } else {
            tx = p->unk_3A0;
            tz = p->unk_3A2;
            var_38 = p->unk_39E;
        }
        /* FAKE: the trailing `&& var_38 == 1` repeats the first test
         * (redundant condition, no-new-park-categories entry 16; owner ruling
         * Q131). The target's $s2 re-test after the || chain is real (without
         * it the chain is two instructions shorter), but this C spelling is
         * our reconstruction */
        if (!(var_38 == 1 &&
              (p->other->move_kind == 0xA || p->cpu_profile == 0xA ||
               (p->weapon_id >= 6 && p->weapon_ammo == 0)) &&
              var_38 == 1)) {
            if (!(p->cpu_pad_build_count & 7)) {
                p->nearest_nav_crossing_distance =
                    fighter_FindNearestNavCrossing(p, var_25, tx, tz);
            }
            if (var_38 == 1) {
                work1 = p->cpu_probes[6];
                if (!(p->unk_430 & 0x800) ||
                    g_fighter_horizontal_separation < 4000) {
                    if (p->unk_442 == 2 && !(work1 == 1 || work1 == 2)) {
                        lim = p->other->unk_3F8[p->other->unk_86];
                    } else if ((p->unk_442 == 1 || p->unk_442 == 2) ||
                               p->unk_442 == 3) {
                        if (!(g_cpu_profiles[p->cpu_profile].flags & 0x4000)) {
                            lim = p->other->unk_3FE[p->other->unk_86];
                        } else {
                            lim = p->other->unk_404[p->other->unk_86];
                        }
                    } else if (p->nearest_nav_crossing_distance != 100000) {
                        if (p->unk_430 & 0x200) {
                            lim = p->other->unk_404[p->other->unk_86];
                        } else {
                            lim = p->unk_404[p->unk_86] +
                                  p->other->unk_404[p->other->unk_86];
                        }
                    } else {
                        lim = p->unk_3FE[p->unk_86] +
                              p->other->unk_3FE[p->other->unk_86];
                        if (p->unk_430 & 0x200) {
                            if (p->other->opponent_angle_from_orientation_abs >
                                0x600) {
                                lim = p->other->unk_3F8[p->other->unk_86];
                            } else if (
                                p->other->opponent_angle_from_orientation_abs >
                                0x300) {
                                lim = p->other->unk_404[p->other->unk_86];
                            } else if (
                                p->other->opponent_angle_from_orientation_abs >
                                0x100) {
                                lim = p->other->unk_3FE[p->other->unk_86];
                            }
                        }
                    }
                } else {
                    lim = 2000;
                }
            } else {
                lim = 2000;
            }
            if (var_38 == 1) {
                if (p->cpu_route.count < 2 &&
                    (p->nearest_nav_crossing_distance != 100000 ||
                     g_fighter_horizontal_separation >= lim)) {
                    goto record;
                }
            } else if (p->cpu_route.count == 0) {
                if (p->nearest_nav_crossing_distance != 100000 ||
                    CPU_SQ(p->world_pos.x - tx) + CPU_SQ(p->world_pos.z - tz) >
                        0x15F8F) {
                record:
                    p->cpu_route.node[0].x = tx;
                    p->cpu_route.node[0].z = tz;
                    p->cpu_route.node[0].kind = var_38;
                    p->cpu_route.count = 1;
                    if (p->nearest_nav_crossing_distance != 100000 &&
                        !(p->cpu_pad_build_count & 7)) {
                        fighter_AppendNavDetour(p, var_25, tx, tz);
                    }
                }
            }
            if (p->cpu_route.count != 0) {
                if (p->cpu_route.node[0].kind == 1
                        ? (p->nearest_nav_crossing_distance == 100000 &&
                           g_fighter_horizontal_separation < lim)
                        : SquareRoot0(CPU_SQ(p->world_pos.x - tx) +
                                      CPU_SQ(p->world_pos.z - tz)) < 2000) {
                    p->cpu_route.count = 0;
                    p->unk_39D = 0;
                    goto after_nav;
                }
                {
                    s32 dist;
                    s32 var_53;
                    work1 = p->cpu_probes[6];
                    var_53 = p->cpu_route.count - 1;
                    work2 = p->cpu_probes[0];
                    wx = p->cpu_route.node[var_53].x;
                    wz = p->cpu_route.node[var_53].z;
                    if (!(work1 == 1 || work1 == 2)) {
                        var_8 = 0;
                        if (p->cpu_route.node[var_53].kind == 1) {
                            if (work2 == 3 &&
                                g_fighter_horizontal_separation <
                                    p->other->unk_404[p->other->unk_86]) {
                                goto pick2;
                            }
                            /* FAKE: the inner test contradicts the outer one,
                             * so pick2's body runs only by the goto above
                             * (contradictory condition, no-new-park-categories
                             * entry 16; owner ruling Q131). The target's two
                             * compares are real, and failing the inner compare
                             * still reaches the var_8 call test, but this C
                             * spelling is our reconstruction; without the
                             * work2 == 5 arm: score 20 */
                            if (work2 == 5 &&
                                p->other->unk_404[p->other->unk_86] <
                                    g_fighter_horizontal_separation) {
                                if (g_fighter_horizontal_separation <
                                    p->other->unk_404[p->other->unk_86]) {
                                pick2:
                                    if (p->move_kind == 0x13) {
                                        var_8 = D_8009A8A4;
                                    } else if (p->unk_440 != 4) {
                                        var_8 = D_8009A89C;
                                    }
                                }
                                if (var_8 != 0) {
                                    fighter_StartCpuPadScript(p, var_8, 4, 0);
                                }
                            }
                        }
                    }
                    if (p->unk_3CC != 0) {
                        return p->unk_3CC;
                    }
                    if (var_53 == 0) {
                        if (p->cpu_route.node[0].kind == 1) {
                            dist = SquareRoot0(CPU_SQ(p->world_pos.x - tx) +
                                               CPU_SQ(p->world_pos.z - tz));
                        } else {
                            dist = SquareRoot0(CPU_SQ(p->world_pos.x - wx) +
                                               CPU_SQ(p->world_pos.z - wz));
                        }
                    } else {
                        work4 = var_53;
                        dist = SquareRoot0(CPU_SQ(wx - p->world_pos.x) +
                                           CPU_SQ(wz - p->world_pos.z));
                        while (work4 >= 2) {
                            dist += SquareRoot0(
                                CPU_SQ(p->cpu_route.node[work4].x -
                                       p->cpu_route.node[work4 - 1].x) +
                                CPU_SQ(p->cpu_route.node[work4].z -
                                       p->cpu_route.node[work4 - 1].z));
                            work4--;
                        }
                        {
                            work1 = p->cpu_route.node[1].x;
                            work2 = p->cpu_route.node[1].z;
                            if (p->cpu_route.node[0].kind == 1) {
                                dist += SquareRoot0(
                                    CPU_SQ(work1 - tx) + CPU_SQ(work2 - tz));
                            } else {
                                dist += SquareRoot0(
                                    CPU_SQ(work1 - p->cpu_route.node[0].x) +
                                    CPU_SQ(work2 - p->cpu_route.node[0].z));
                            }
                        }
                    }
                    work3 = lim < dist;
                    p->unk_3CC = fighter_BuildCpuRoutePadWithSurfaceProbe(
                        p, wx, wz, work3);
                    var_19 = 300;
                    var_22 = p->unk_3CC & 4;
                    if (var_22) {
                        var_19 = 1000;
                    }
                    if (CPU_SQ(p->cpu_route.node[p->cpu_route.count - 1].x -
                               p->world_pos.x) +
                            CPU_SQ(p->cpu_route.node[p->cpu_route.count - 1].z -
                                   p->world_pos.z) <
                        var_19 * (var_22 ? 1000 : 300)) {
                        if (--p->cpu_route.count != 0) {
                            work3 =
                                (ratan2(
                                     p->cpu_route.node[p->cpu_route.count - 1]
                                             .x -
                                         p->world_pos.x,
                                     p->cpu_route.node[p->cpu_route.count - 1]
                                             .z -
                                         p->world_pos.z) -
                                 p->orientation.vy) &
                                0xFFF;
                            if (work3 > 0x800) {
                                work3 -= 0x1000;
                            }
                            if ((work3 < 0 ? -work3 : work3) > 0x300) {
                                return 0;
                            }
                        }
                    }
                }
            }
        after_nav:
            if (p->unk_3CC != 0) {
                return p->unk_3CC;
            }
        }

        if ((!(p->unk_430 & 0x80) &&
             g_fighter_horizontal_separation <
                 p->other->unk_404[p->other->unk_86] &&
             ((p->cpu_probes[3] >= 2 &&
               (p->cpu_probes[5] >= 2 || p->cpu_probes[1] >= 2)) ||
              p->cpu_probes[3] == 1 || p->cpu_probes[4] == 1 ||
              p->cpu_probes[2] == 1)) ||
            ((g_cpu_profiles[p->cpu_profile].flags & 0x4000 ||
              p->cpu_profile == 0x18 || p->cpu_profile == 0x1A) &&
             g_fighter_horizontal_separation <
                 p->other->unk_404[p->other->unk_86] &&
             p->other->opponent_angle_from_orientation_abs < 0x10) ||
            (p->weapon_id >= 7 &&
             g_fighter_horizontal_separation <
                 p->other->unk_404[p->other->unk_86] &&
             p->other->opponent_angle_from_orientation_abs < 0x10 &&
             p->weapon_ammo != 0)) {
            if (!(g_cpu_profiles[p->cpu_profile].flags & 0x8F00)) {
                if ((p->cpu_route.count = fighter_BuildCpuOrbitRoute(p)) != 0) {
                    p->unk_39D = 2;
                    return -1;
                }
            }
            if (!(g_cpu_profiles[p->cpu_profile].flags & 0x8C00)) {
                cnt = 0;
                work3 = g_cpu_profiles[p->cpu_profile].pick_weight[4] <
                        (rand() & 0xFF);
                var_29 = -1;
                work1 = p->cpu_probes[1] == 0;
                work2 = p->cpu_probes[5] == 0;
                for (; cnt < 2; cnt++, work3++) {
                    if (work3 & 1) {
                        if (p->other->opponent_angle_from_orientation < 0) {
                            if (work1) {
                                var_29 = 6;
                            } else if (work2) {
                                var_29 = 7;
                            }
                        } else {
                            if (work2) {
                                var_29 = 7;
                            } else if (work1) {
                                var_29 = 6;
                            }
                        }
                        if (var_29 != -1) {
                            break;
                        }
                    } else if (p->unk_440 != 4 &&
                               g_fighter_horizontal_separation > 3000 &&
                               g_fighter_horizontal_separation < 5000) {
                        var_29 = 8;
                        break;
                    }
                }
                if (var_29 != -1) {
                    p->unk_394 = var_29;
                    p->unk_39C = 0;
                    p->unk_398 = p->unk_39A;
                }
            }
        }

        if (p->unk_398 >= p->unk_39A) {
            var_9 = 0;
            if (p->unk_39C != 1) {
                work3 = p->unk_394;
            } else if (p->other->move_kind == 0x19) {
                u8 var_54[4];
                __builtin_memcpy(var_54, D_800A325C, 4);
                work3 = var_54[p->other->unk_441];
            } else if (p->other->move_kind == 0x1A) {
                u8 var_55[4];
                __builtin_memcpy(var_55, D_800A3260, 4);
                work3 = var_55[p->other->unk_441];
            }
            switch (work3) {
            case 0:
                if (p->cpu_probes[3] == 0) {
                    p->unk_3CC = 0x8000;
                }
                break;
            case 1:
                if (p->cpu_probes[0] == 0) {
                    p->unk_3CC = 0x2000;
                }
                break;
            case 2:
                if (p->cpu_probes[1] == 0) {
                    p->unk_3CC = 0x4000;
                }
                break;
            case 3:
                if (p->cpu_probes[5] == 0) {
                    p->unk_3CC = 0x1000;
                }
                break;
            case 4:
                if (p->cpu_probes[3] == 0 &&
                    g_fighter_horizontal_separation < p->unk_3FE[p->unk_86]) {
                    var_9 = D_8009A890;
                }
                break;
            case 5:
                if (p->cpu_probes[0] == 0 &&
                    p->unk_3F8[p->unk_86] < g_fighter_horizontal_separation) {
                    var_9 = D_8009A888;
                }
                break;
            case 6:
                if (p->cpu_probes[1] == 0 &&
                    g_fighter_horizontal_separation < p->unk_3FE[p->unk_86]) {
                    var_9 = D_8009A878;
                }
                break;
            case 7:
                if (p->cpu_probes[5] == 0 &&
                    g_fighter_horizontal_separation < p->unk_3FE[p->unk_86]) {
                    var_9 = D_8009A880;
                }
                break;
            case 8:
                if (!(p->cpu_probes[6] == 1 || p->cpu_probes[6] == 2) &&
                    g_fighter_horizontal_separation > 3000 &&
                    g_fighter_horizontal_separation < 5000 &&
                    p->opponent_angle_from_orientation_abs < 0x200) {
                    var_9 = D_8009A89C;
                }
                break;
            }
            if ((p->unk_426 == 1 || p->unk_426 == 2) ||
                (p->unk_425 == 1 || p->unk_425 == 2)) {
                if (p->unk_3CC != 0) {
                    p->unk_398 = p->unk_39A + 1;
                } else {
                    p->unk_398 = 0;
                    var_9 = 0;
                }
            } else if ((p->unk_430 & 0x800) || p->unk_442 != 0 ||
                       (p->unk_3CC == 0x2000 &&
                        g_fighter_horizontal_separation <
                            p->other->unk_3F8[p->other->unk_86])) {
                var_9 = 0;
                p->unk_398 = 0;
                p->unk_3CC = 0;
            }
            if (var_9 != 0) {
                p->unk_398 = 0;
                if (p->unk_430 & 0xA000) {
                    fighter_StartCpuPadScript(p, var_9, 4, 0);
                }
            }
        } else if (p->unk_398 == 0) {
            s32 var_56 = g_cpu_profiles[p->cpu_profile].unk6;
            r = rand() & 0xFF;
            if (p->unk_440 == 4 ? r < (var_56 >> 2) : r < var_56) {
                work2 = -1;
                var_43 = -1;
                var_16 = 0;
            pick_loop: {
                var_44 = ((rand() & 0xFFF) *
                          g_cpu_profiles[p->cpu_profile].pick_weight[var_16]) >>
                         12;
                if (var_44 != 0) {
                    var_37 = 0;
                    switch (var_16) {
                    case 0:
                    case 2:
                        if (rand() & 1) {
                            var_37 = 1;
                        } else if (p->cpu_probes[3] != 0) {
                            goto pick_next;
                        }
                        var_23 = p->unk_3F0;
                        if (g_cpu_profiles[p->cpu_profile].unk7 <
                            (var_23 >= 0 ? var_23 : -var_23)) {
                            var_44 += 0x80;
                            var_37 = var_23 > 0;
                        }
                        if (p->weapon_id >= 6 && p->weapon_ammo == 0 &&
                            var_16 == 2) {
                            var_44 += 0x100;
                            var_37 = 0;
                        }
                        break;
                    case 1:
                    case 3:
                        if (rand() & 1) {
                            if (p->cpu_probes[5] != 0) {
                                goto pick_next;
                            }
                            var_37 = 1;
                        } else if (p->cpu_probes[1] != 0) {
                            goto pick_next;
                        }
                        if (p->unk_430 & 0x20000) {
                            var_44 += 0x80;
                            var_37 = p->opponent_angle_from_orientation > 0;
                            if (*(var_37 ? &p->cpu_probes[5]
                                         : &p->cpu_probes[1]) != 0) {
                                var_37 ^= 1;
                            }
                        }
                        break;
                    case 4:
                        if ((p->cpu_probes[6] == 1 || p->cpu_probes[6] == 2) ||
                            !(g_fighter_horizontal_separation >= 3000 &&
                              g_fighter_horizontal_separation <= 5000) ||
                            p->opponent_angle_from_orientation_abs >= 0x201 ||
                            p->unk_440 == 4 || p->other->move_kind == 0x18 ||
                            p->other->move_kind == 0x2A) {
                            goto pick_next;
                        }
                        break;
                    }
                    /* Q82: work2 (the Q75 value) is compared as an s16, as
                     * the target does (sll 16; sra 16; slt). No cast,
                     * an s16 best or var_44 local, or work2 as s16 all differ.
                     */
                    if ((s16)work2 < var_44) {
                        var_43 = var_16;
                        work2 = var_44;
                        var_3 = var_37;
                    }
                }
            }
            pick_next:
                if (++var_16 < 7) {
                    goto pick_loop;
                }
                if (var_43 != -1) {
                    p->unk_394 = 0;
                    p->unk_39C = 0;
                    p->unk_398 =
                        (((((rand() & 0xFFF) *
                            g_cpu_profiles[p->cpu_profile].unk6) >>
                           12) +
                          0x17)
                         << 12) /
                        p->base_move_frame_step;
                    switch (var_43) {
                    case 0:
                        p->unk_394 = var_3 != 0;
                        break;
                    case 1:
                        if (var_3) {
                            p->unk_394 = 3;
                        } else {
                            p->unk_394 = 2;
                        }
                        break;
                    case 2:
                        if (var_3) {
                            p->unk_394 = 5;
                        } else {
                            p->unk_394 = 4;
                        }
                        break;
                    case 3:
                        if (var_3) {
                            p->unk_394 = 7;
                        } else {
                            p->unk_394 = 6;
                        }
                        break;
                    case 4:
                        p->unk_394 = 8;
                        break;
                    case 5:
                        p->unk_39C = 1;
                        break;
                    }
                }
            }
        }
        if (p->unk_398 != 0) {
            p->unk_398--;
        }
    }

    if (p->unk_3CC != 0 || p->unk_398 != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 6) && !(p->unk_430 & 0x800)) {
        u16 state;

        state = p->move_kind;
        if (state == 0x15 || (state == 0x19 && p->unk_441 >= 2)) {
            if ((p->cpu_pad_build_count & 1) || p->weapon_id >= 6) {
                if (p->unk_442 == 0 && p->cpu_probes[3] != 1 &&
                    (p->weapon_id < 6 || p->weapon_ammo != 0)) {
                    var_35 = -1;
                    if (p->unk_3F2 % ((p->unk_438 >> 8) + 2) ==
                        (p->unk_438 >> 8) + 1) {
                        var_6 = p->unk_438 >> 1;
                    } else {
                        var_6 = p->unk_438;
                    }
                    list = p->unk_3A8[p->unk_86];
                    off = *list;
                    work4 = 0;
                    while (off != 0) {
                        /* work5 holds three values in turn, each read before
                         * the next write: case 2's pattern-word top bits
                         * (work1 >> 27); the offset
                         * ((0x1000 - var_6) * 625 >> 10) - 400; a copy of
                         * var_18 for the var_18 < 5 and var_18 == 5 / 6 tests
                         * (Q34) (Ruling 11, reused-local-necessity). */
                        s32 work5;
                        /* FAKE: opaque 1 (no-new-park-categories entry 2,
                         * loop-rotation-two-shift): with a
                         * literal the mask tests `(x & (1 << n)) == 0` fold to
                         * srav; andi, while the target shifts the 1 held in
                         * $fp, set once before the loop and shared with case
                         * 2's mask shifts. `1U << n`, a u32 mask local and a
                         * function-scope `var_59` all differ */
                        s32 var_59 = 1;
                        var_32 = off + (u8 *)p->unk_3A4;
                        e = var_32;
                        var_32 += 4;
                        /* FAKE: pass-through pointer alias
                         * (pointer-alias-fake-exception; SOTN precedent
                         * below): the target copies var_32's value into its
                         * own register and reads the 0x40 character-mask header
                         * through it while var_32 stays in $s6; read through
                         * var_32 the allocation order shifts: score 58.
                         * e[4]/e[8..5]: 18; same with wrapper removed;
                         * e + 4 cursor initialization: 24 (e reads), 30
                         * (var_32 reads); cursor = e + 9: 19 (e reads) /
                         * 25 (var_32 reads); mask inline or in a u32
                         * local, wrapper also removed: 21 (e reads) / 61
                         * (var_32 reads) */
                        /* SOTN: src/st/no0/e_stone_rose.c:611 @aa53500 */
                        q = var_32;
                        if (g_cpu_profiles[p->cpu_profile].flags & 0xFF00) {
                            switch (g_fight_mode_selector) {
                            case 3:
                                if (g_fight_step_table_next_row < 0x5B) {
                                    work3 =
                                        g_fight_step_table_next_row / 10 * 2;
                                    if (g_fight_step_table_next_row % 10 == 0) {
                                        work3--;
                                    }
                                } else if (g_fight_step_table_next_row < 0x5E) {
                                    work3 = 0x12;
                                } else if (g_fight_step_table_next_row < 0x60) {
                                    work3 = 0x13;
                                } else if (g_fight_step_table_next_row < 0x62) {
                                    work3 = 0x14;
                                } else if (g_fight_step_table_next_row < 0x64) {
                                    work3 = 0x15;
                                } else {
                                    work3 = 0x16;
                                }
                                work3 = D_8009A928[p->unk_440][work3];
                                break;
                            case 2:
                                work1 = D_8009A9F0[D_8009A9DC[p->weapon_id]
                                                             [p->unk_440]]
                                                  [D_800A3788];
                                work3 = 0;
                                work5 = work1 >> 27;
                                work2 = work1 & 0xF;
                                if (work2 != 0) {
                                    if (work5) {
                                        while (work2 > 0) {
                                            work1 >>= 4;
                                            work3 |=
                                                var_59 << ((work1 & 0xF) - 1);
                                            work2--;
                                        }
                                    } else {
                                        work1 >>=
                                            (p->unk_3F2 / 3 % work2) * 4 + 4;
                                        work3 = var_59 << ((work1 & 0xF) - 1);
                                    }
                                }
                                break;
                            default:
                                work3 =
                                    D_8009A8C8[p->unk_440]
                                              [g_fight_stage_command_count - 1]
                                                  .mask;
                                break;
                            }
                            if ((e[3] >> 4) == 0 ||
                                !(work3 & (var_59 << ((e[3] >> 4) - 1)))) {
                                goto next;
                            }
                        }
                        if (!(g_cpu_profiles[p->cpu_profile].flags & 0x80) &&
                            p->last_chosen_script_list_index == p->unk_86 &&
                            p->last_chosen_script_index == work4) {
                            goto next;
                        }
                        if (q[0] == 0x40) {
                            work3 = q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1];
                            if (!(work3 & (var_59 << p->cpu_profile))) {
                                goto next;
                            }
                            var_32 += 5;
                        }
                        if ((e[0] & 0x80) && p->unk_26C == 0) {
                            goto next;
                        }
                        work1 = e[1] * 40;
                        hi = e[2] * 40;
                        /* FAKE: do-while(0): its loop depth weights var_18's
                         * references so var_18 is allocated ahead of var_32
                         * (var_18 $s5, var_32 $s6, as in the target);
                         * unwrapped: score 10 (do-while-zero-exception);
                         * before work1 = e[1] * 40 or as if subject: 10;
                         * before 0x80 test: 20; loop-scope var_18, var_32,
                         * e separately or together: 10; no-alias cluster
                         * through e / var_32: 18 / 58 */
                        do {
                            var_18 = e[0] & 7;
                        } while (0);
                        work3 = 0;
                        if (var_18 == 0) {
                            if (work1 < g_fighter_horizontal_separation &&
                                g_fighter_horizontal_separation < hi) {
                                if (p->other->move_kind == 0x15 ||
                                    p->other->move_kind == 0x2C ||
                                    p->other->move_kind == 0xE ||
                                    p->other->move_kind == 0x19) {
                                    work3 = 1;
                                }
                            }
                        } else {
                            if ((g_cpu_profiles[p->cpu_profile].flags &
                                 0xFC00) ||
                                p->weapon_id >= 6) {
                                work1 = var_18 < 5 ? 100000 : 0;
                                hi = 100000;
                            } else {
                                work5 = (((0x1000 - var_6) * 625) >> 10) - 400;
                                work1 += work5 + p->scale_distance_offset;
                                hi += work5 + p->scale_distance_offset;
                            }
                            work5 = var_18;
                            if (work5 < 5) {
                                if (g_fighter_horizontal_separation < work1 &&
                                    !(p->unk_430 & 0x20000)) {
                                    if ((p->unk_430 & 0x200)
                                            ? p->opponent_angle_from_orientation_abs <
                                                  0x800
                                            : p->opponent_angle_from_orientation_abs <
                                                  0x400) {
                                        work3 = 1;
                                    } else if (p->weapon_id >= 6) {
                                        work3 = 1;
                                    }
                                }
                            } else if (
                                work1 < g_fighter_horizontal_separation &&
                                g_fighter_horizontal_separation < hi &&
                                p->opponent_angle_from_orientation_abs <
                                    0x200 - ((p->unk_438 * 0x100) >> 12) &&
                                /* Q76: unk_438 / 16 as the 4.12 multiply by
                                 * 0x100 (1/16); `>> 4` merges the shift into
                                 * the sign extension (lhu; sll 16; sra 20), the
                                 * target has lh; sra 4. */
                                (p->unk_430 & 0x280) != 0x280) {
                                switch (work5) {
                                case 5:
                                    if ((0x78 >> p->collision_hit_tag) & 1) {
                                        work3 = 1;
                                    }
                                    break;
                                case 6:
                                    if (p->cpu_profile == 0x15 ||
                                        p->carried_object_count != 0) {
                                        work3 = 1;
                                    }
                                    break;
                                default:
                                    work3 = 1;
                                    break;
                                }
                            }
                        }
                        if (work3) {
                            var_24 =
                                ((rand() & 0xFFF) *
                                 g_cpu_profiles[p->cpu_profile].unkF[var_18]) >>
                                12;
                            if (var_24 != 0 && var_35 < var_24) {
                                var_35 = var_24;
                                var_4 = work4;
                                var_1 = var_32;
                                var_5 = hi;
                            }
                        }
                    next:
                        list++;
                        off = *list;
                        work4++;
                    }
                    if (var_35 != -1) {
                        fighter_StartCpuPadScript(p, var_1, 0, 0);
                        p->last_chosen_script_index = var_4;
                        p->last_chosen_script_list_index = p->unk_86;
                        p->chosen_script_start_x = p->world_pos.x;
                        p->chosen_script_start_z = p->world_pos.z;
                        p->unk_3F2++;
                        p->chosen_script_radius = var_5;
                    }
                }
            } else if (
                state == 0x15 && p->unk_26C != 0 && p->unk_440 != 4 &&
                /* Q76: as above, 4.12 factor 0x100; target lh; sra 4 at
                   0x8005ACCC. */
                p->opponent_angle_from_orientation_abs <
                    0x200 - ((p->unk_438 * 0x100) >> 12) &&
                !(g_fight_mode_selector == 2 || g_fight_mode_selector == 3)) {
                var_10 = 0;
                if ((rand() & 0xFF) <
                        (g_cpu_profiles[p->cpu_profile].unkF[5] >> 2) &&
                    ((0x78 >> p->collision_hit_tag) & 1) && p->unk_442 == 0 &&
                    p->other->unk_404[p->other->unk_86] <
                        g_fighter_horizontal_separation &&
                    g_fighter_horizontal_separation < 4500) {
                    var_10 = D_8009A8C0;
                } else {
                    s32 var_60;

                    if (p->cpu_profile == 0x15) {
                        var_60 = p->unk_34D;
                        work1 = p->other->unk_3F8[p->other->unk_86];
                    } else {
                        work1 = p->other->unk_404[p->other->unk_86] + 300;
                        if (g_cpu_profiles[p->cpu_profile].flags & 0x300) {
                            var_60 = 0;
                            if (g_fight_stage_command_count >= 6) {
                                var_60 = p->weapon_ammo;
                            }
                        } else {
                            var_60 = p->carried_object_count;
                        }
                    }
                    var_40 = 0;
                    if ((rand() & 0xFF) <
                            (g_cpu_profiles[p->cpu_profile].unkF[6] >> 2) &&
                        var_60 != 0 &&
                        work1 < g_fighter_horizontal_separation &&
                        p->nearest_nav_crossing_distance == 100000 &&
                        p->unk_442 == 0 && (p->unk_430 & 0xA002) &&
                        (p->cpu_profile != 0x15 ||
                         g_fighter_horizontal_separation < 3000) &&
                        (p->unk_8A == 0 || var_60 >= 2)) {
                        var_40 = 1;
                    }
                    if (var_40) {
                        if ((g_cpu_profiles[p->cpu_profile].flags & 0x10) &&
                            var_60 >= 2 && (rand() & 1)) {
                            var_10 = D_8009A8B4;
                        } else {
                            var_10 = D_8009A8AC;
                        }
                    }
                }
                if (var_10 != 0) {
                    fighter_StartCpuPadScript(p, var_10, 2, 0);
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if (p->move_kind == 0x15) {
        if (p->opponent_angle_from_orientation_abs > 0x100 &&
            !(p->prev_move_kind == 0x19 || p->prev_move_kind == 0x1A) &&
            p->weapon_id < 7 && p->cpu_profile != 0x16) {
            p->unk_39C = 0;
            p->unk_398 = 0x17000 / p->base_move_frame_step;
            if (p->cpu_probes[5] == 0) {
                p->unk_394 = 3;
            } else if (p->cpu_probes[1] == 0) {
                p->unk_394 = 2;
            } else if (p->cpu_probes[0] == 0) {
                p->unk_394 = 1;
            } else {
                p->unk_394 = 0;
            }
        } else if (p->weapon_id >= 6) {
            if (p->weapon_ammo == 0 && p->weapon_refills_left != 0 &&
                p->unk_26C != 0 &&
                p->other->unk_404[p->other->unk_86] <
                    g_fighter_horizontal_separation) {
                p->unk_3CC = 0x80;
            }
        } else if ((p->unk_430 & 0x800) &&
                   (g_cpu_profiles[p->cpu_profile].flags & 1) &&
                   g_fighter_horizontal_separation < 4000 && p->unk_440 != 4 &&
                   (p->unk_442 == 0 || p->unk_442 == 2)) {
            fighter_StartCpuPadScript(p, D_8009A898, 1, p->unk_3BD);
            p->unk_3F2++;
        } else if ((p->unk_430 & 0xA801) || p->other->move_kind == 0x18 ||
                   p->other->move_kind == 0x25 || p->other->move_kind == 8 ||
                   p->other->move_kind == 0xA ||
                   (p->other->move_kind == 0x1A && p->unk_441 == 1)) {
            if (g_cpu_profiles[p->cpu_profile].unk3 != 0 &&
                ((!(g_cpu_profiles[p->cpu_profile].flags & 0xFF00) &&
                  p->other->unk_404[p->other->unk_86] <
                      g_fighter_horizontal_separation &&
                  p->unk_3F4 >= g_cpu_profiles[p->cpu_profile].unk3) ||
                 ((g_cpu_profiles[p->cpu_profile].flags & 0x100) &&
                  p->other->unk_3F8[p->other->unk_86] <
                      g_fighter_horizontal_separation &&
                  p->unk_3F4 >= g_cpu_profiles[p->cpu_profile].unk3 &&
                  p->unk_440 != 2) ||
                 ((g_cpu_profiles[p->cpu_profile].flags & 0x7C00) &&
                  p->other->unk_3F8[p->other->unk_86] <
                      g_fighter_horizontal_separation &&
                  p->unk_3F4 >= g_cpu_profiles[p->cpu_profile].unk3 &&
                  g_fight_step_table_next_row >= 0x5B) ||
                 (p->unk_430 & 0x40000))) {
                p->unk_3F4 = 0;
                p->unk_3CC = 0x80;
                p->unk_430 &= ~0x40000;
            }
        }
    }
    return p->unk_3CC;
}

#undef CPU_SQ

/* .data 0x80099D88..0x8009A088, this file's .data (43500.c's D_80099D54
 * ends at 0x80099D88; the data-only d_8A588.c's D_8009A088 follows): the 32
 * CPU profiles, indexed by cpu_profile.
 * Defined after every function, so no body sees a definition where it saw
 * an extern. */
Unk80099D88Rec g_cpu_profiles[32] = {
    {0x0000,
     0x0,
     0x4,
     0x0,
     0x40,
     0x4,
     0x80,
     {0x80, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0},
     {0xFF, 0xC0, 0x80, 0x40, 0x40, 0x40, 0x40, 0x40},
     0x0},
    {0x0007,
     0x3,
     0x4,
     0x20,
     0x80,
     0x8,
     0x40,
     {0x80, 0x80, 0xFF, 0xFF, 0x60, 0x80, 0x0},
     {0x40, 0xC0, 0xC0, 0x80, 0x40, 0x80, 0x40, 0x80},
     0x0},
    {0x0012,
     0x2,
     0x3,
     0xFF,
     0xC0,
     0x10,
     0xFF,
     {0x20, 0xFF, 0x0, 0x0, 0x40, 0x0, 0x0},
     {0x40, 0x80, 0x80, 0x80, 0x80, 0x40, 0xC0, 0xC0},
     0x0},
    {0x0000,
     0x2,
     0x2,
     0x20,
     0x80,
     0x6,
     0xFF,
     {0x80, 0xFF, 0x0, 0x0, 0x0, 0x80, 0x0},
     {0xC0, 0xC0, 0x80, 0x40, 0x40, 0x40, 0x40, 0x80},
     0x0},
    {0x0007,
     0x1,
     0x2,
     0xFF,
     0xC0,
     0x10,
     0x40,
     {0xFF, 0x0, 0xFF, 0xFF, 0x60, 0x0, 0x0},
     {0x40, 0x40, 0xC0, 0x80, 0x80, 0xC0, 0xC0, 0xC0},
     0x0},
    {0x0005,
     0x1,
     0x3,
     0xFF,
     0xFF,
     0x20,
     0x20,
     {0xFF, 0xFF, 0x0, 0xFF, 0x0, 0x0, 0x0},
     {0x0, 0xFF, 0x80, 0x40, 0x0, 0x0, 0x0, 0x0},
     0x0},
    {0x00B7,
     0x1,
     0x2,
     0x0,
     0x40,
     0x10,
     0x40,
     {0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0x0},
     {0xC0, 0x40, 0x80, 0xC0, 0x80, 0xC0, 0xC0, 0x40},
     0x0},
    {0x0000,
     0x0,
     0x3,
     0x0,
     0x40,
     0x4,
     0xFF,
     {0xFF, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0},
     {0xFF, 0xC0, 0x0, 0x40, 0x40, 0x40, 0x40, 0x80},
     0x0},
    {0x0053,
     0x0,
     0x2,
     0xFF,
     0xC0,
     0x10,
     0x80,
     {0xFF, 0xFF, 0x80, 0x80, 0x80, 0xFF, 0x0},
     {0xFF, 0x40, 0xC0, 0xC0, 0x40, 0x80, 0x80, 0xC0},
     0x0},
    {0x0083,
     0x3,
     0x3,
     0xFF,
     0x80,
     0x8,
     0xFF,
     {0x20, 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0},
     {0x40, 0x80, 0xC0, 0x40, 0x40, 0x80, 0x80, 0x80},
     0x0},
    {0x0002,
     0x2,
     0x3,
     0x20,
     0x0,
     0x8,
     0xFF,
     {0x80, 0x80, 0x80, 0x80, 0x0, 0xFF, 0x0},
     {0xC0, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0xC0},
     0x0},
    {0x01C1,
     0x9,
     0xFF,
     0x0,
     0x80,
     0x20,
     0x30,
     {0x0, 0x0, 0xFF, 0x80, 0x0, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x40, 0xFF},
     0x0},
    {0x0011,
     0x2,
     0x4,
     0x0,
     0x80,
     0x10,
     0xFF,
     {0x20, 0xFF, 0x0, 0x0, 0x0, 0x80, 0x0},
     {0xC0, 0x80, 0xC0, 0x40, 0x40, 0x40, 0x40, 0x40},
     0x0},
    {0x0001,
     0x4,
     0x3,
     0xFF,
     0x80,
     0x8,
     0x80,
     {0x80, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0},
     {0xC0, 0xC0, 0x80, 0x40, 0x40, 0x80, 0x40, 0x80},
     0x0},
    {0x0087,
     0x0,
     0x2,
     0xFF,
     0xC0,
     0x18,
     0x40,
     {0xFF, 0x80, 0x80, 0x80, 0x40, 0x0, 0x0},
     {0xC0, 0x40, 0x40, 0xC0, 0x80, 0xC0, 0xC0, 0xC0},
     0x0},
    {0x0003,
     0x2,
     0x4,
     0x0,
     0x40,
     0x8,
     0x80,
     {0xFF, 0x80, 0x0, 0x0, 0x0, 0xFF, 0x0},
     {0xFF, 0x40, 0xC0, 0x80, 0x40, 0x40, 0x40, 0x80},
     0x0},
    {0x0027,
     0x0,
     0x0,
     0x0,
     0xFF,
     0x40,
     0x0,
     {0x20, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0x0},
     {0x0, 0x40, 0x80, 0xFF, 0x0, 0xFF, 0x0, 0x0},
     0x0},
    {0x0002,
     0x2,
     0x3,
     0x20,
     0x80,
     0x10,
     0xFF,
     {0x20, 0xFF, 0x0, 0x0, 0x20, 0x80, 0x0},
     {0x40, 0x80, 0xC0, 0x80, 0x40, 0x40, 0x40, 0xC0},
     0x0},
    {0x0027,
     0x1,
     0x2,
     0xFF,
     0xC0,
     0x18,
     0x80,
     {0x80, 0x80, 0x0, 0x0, 0x20, 0x80, 0x0},
     {0x40, 0xC0, 0x80, 0x40, 0x40, 0xC0, 0xC0, 0xC0},
     0x0},
    {0x0013,
     0x4,
     0x3,
     0x20,
     0x80,
     0x18,
     0xFF,
     {0x20, 0xFF, 0x80, 0x80, 0x20, 0x0, 0x0},
     {0x40, 0x40, 0x80, 0xC0, 0x40, 0x80, 0x80, 0x80},
     0x0},
    {0x00E7,
     0x0,
     0x1,
     0x20,
     0xC0,
     0x20,
     0x30,
     {0x0, 0x0, 0x80, 0xFF, 0x80, 0x0, 0x0},
     {0xC0, 0x80, 0xC0, 0x40, 0x40, 0xC0, 0xC0, 0xC0},
     0x0},
    {0x0001,
     0x0,
     0x2,
     0x0,
     0x40,
     0x4,
     0xFF,
     {0x20, 0xFF, 0x80, 0x80, 0x20, 0xFF, 0x0},
     {0xFF, 0xC0, 0xC0, 0x40, 0x40, 0x80, 0xC0, 0x40},
     0x0},
    {0x0001,
     0x0,
     0x4,
     0x0,
     0x40,
     0x2,
     0x80,
     {0x80, 0x80, 0x80, 0x80, 0x0, 0x80, 0x0},
     {0x80, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x40, 0x40},
     0x0},
    {0x02C1,
     0x9,
     0x0,
     0x0,
     0x80,
     0x20,
     0x30,
     {0x80, 0x40, 0x0, 0x0, 0xFF, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x30, 0xFF},
     0x0},
    {0x0000,
     0x9,
     0x1,
     0xFF,
     0x80,
     0x30,
     0x10,
     {0x0, 0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
     0x0},
    {0x8080,
     0x9,
     0xFF,
     0x0,
     0x80,
     0x0,
     0x0,
     {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0xFF},
     0x0},
    {0x0000,
     0x9,
     0x1,
     0xFF,
     0x80,
     0x30,
     0x10,
     {0x0, 0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
     0x0},
    {0x0481,
     0x9,
     0x1,
     0x0,
     0x80,
     0x18,
     0x80,
     {0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF},
     0x0},
    {0x0881,
     0x9,
     0x1,
     0x0,
     0x80,
     0x10,
     0xFF,
     {0xFF, 0x0, 0x80, 0x0, 0x40, 0x0, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF},
     0x0},
    {0x1081,
     0x9,
     0x1,
     0xFF,
     0x80,
     0x8,
     0x40,
     {0xFF, 0xFF, 0x0, 0x0, 0x40, 0x80, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF},
     0x0},
    {0x2081,
     0x9,
     0x1,
     0x0,
     0x80,
     0x10,
     0xFF,
     {0x80, 0xFF, 0x0, 0x80, 0x0, 0x80, 0x0},
     {0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0xFF},
     0x0},
    {0x4083,
     0x9,
     0x1,
     0xFF,
     0x80,
     0x30,
     0x10,
     {0x0, 0x0, 0xFF, 0xFF, 0xFF, 0x0, 0x0},
     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
     0x0},
};

/* This file's initialized small data (.sdata), in address order (Q65). */
/* "NULL" tag motion_UpdatePlayback compares the motion streams against
 * (gp-relative): size from the blob label */
s32 D_800A3250[2] = {0x4c4c554e, 0};
PadBitTable D_800A3258 = {{0xd, 0xf, 0xc, 0xe}};
u8 D_800A325C[4] = {1, 0, 2, 3};
u8 D_800A3260[4] = {5, 4, 6, 7};
