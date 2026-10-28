/* 80 game functions, among them math_LerpAngle. .text 0x800272FC (ROM 0x17AFC).
 * Start boundary: PHASE (rodata-align site 2). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"
#include <psxsdk/inline_o.h>
#include <psxsdk/gtemac.h>

/* P1/P2 round scores and tiebreakers, declared here as single u8s (per-file
 * declarations: Q21-Q25, no-new-park-categories aggregate-merge exception):
 * every counting aggregate spelling of fight_RecordMatchResult misses the
 * shipped code (constant subscripts put element 0 behind a base register; the
 * index-variable and regrouped-condition spellings miss its match-result
 * stores or compares); dummy-index and pointer-alias spellings are refused
 * (Q22/Q23). src/main/9F9C.c declares the same bytes as D_800A3898[2] /
 * D_800A38AA[2] for fight_UpdateModeDisplaysAndMatchProgress, whose
 * player-indexed accesses single bytes do not produce; no single declaration
 * compiles both files with a counting spelling. */
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A38AA;
extern u8 D_800A38AB;

/* replay_RecordEvent (28708.c: u8, u8, s32 *, u16 *) and
 * eff_StartRecycledYawedParticlesWithTextureSelection (51268.c: s32 *, s32,
 * s32) are called without a prototype: the u8 prototype narrows
 * fight_RecordReplayEventAndDispatchSoundOrEffect's arguments, and
 * eff_StartRecycledYawedParticlesWithTextureSelection is called with two of its
 * three arguments. */
extern void replay_RecordEvent();
extern void eff_StartRecycledYawedParticlesWithTextureSelection();

const char D_80010478[] = "ILLEGAL GUN MOTION : %d\n";
const char D_80010494[] = "";

s32 math_ClassifyAbsMagnitude(s32 a0) {
    s32 v1;
    if (a0 < 0) {
        a0 = -a0;
    }
    if (a0 < 0xA01) {
        if (a0 < 0x600) {
            v1 = 1;
        } else {
            v1 = 2;
        }
    } else {
        v1 = 0;
    }
    return v1;
}

void motion_SetFixedChannelBlock(MotionFrame *motion_frame) {
    motion_frame->unk_0C[0x18] = 0x3F5;
    motion_frame->unk_0C[0x19] = 0x2B6;
    motion_frame->unk_0C[0x1A] = 0x77A;
    motion_frame->unk_0C[0x1B] = 0xBEE;
    motion_frame->unk_0C[0x1C] = 0x8C;
    motion_frame->unk_0C[0x1D] = 0x227;
    motion_frame->unk_0C[0x21] = 0x8A0;
    motion_frame->unk_0C[0x22] = 0xF0E;
    motion_frame->unk_0C[0x1E] = 0;
    motion_frame->unk_0C[0x1F] = 0;
    motion_frame->unk_0C[0x20] = 0;
    motion_frame->unk_0C[0x23] = 0xBCD;
}

/* fighter_AccumulateBoneGroupFlagWhenGameModeZero ORs limb bits into
 * `*(&g_fight_bone_group_tim_requests + a0)`; its only caller passes a0 = 0
 * (fighter_ResolveBoneHit), so D_800A376B (written 0 elsewhere, never read) is
 * not reached from here. FAKE (Q97): two u8 scalars; as u8[2],
 * fight_SaveFighter0SelectionAndStateSubstituteStreamSelectionClearState /
 * fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields
 * were reported at 12 / 16 (not banked). */
void fighter_AccumulateBoneGroupFlagWhenGameModeZero(s32 a0, s32 a1) {
    if (g_fight_mode_selector != 0) {
        return;
    }
    switch (a1) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        *(&g_fight_bone_group_tim_requests + a0) |= 0x10; /* FAKE: Q97 */
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        *(&g_fight_bone_group_tim_requests + a0) |= 0x01; /* FAKE: Q97 */
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        *(&g_fight_bone_group_tim_requests + a0) |= 0x02; /* FAKE: Q97 */
        break;
    case 14:
    case 15:
    case 16:
    case 17:
        *(&g_fight_bone_group_tim_requests + a0) |= 0x04; /* FAKE: Q97 */
        break;
    case 18:
    case 19:
    case 20:
    case 21:
        *(&g_fight_bone_group_tim_requests + a0) |= 0x08; /* FAKE: Q97 */
        break;
    }
}

void fighter_AccumulateOrClearByBoneGroup(
    Unk80101EC8Record *a0, s32 a1, s16 a2) {
    s16 v1;
    v1 = g_fight_mode_selector;
    if (v1 == 5) {
        return;
    }
    if ((u32)a1 >= 0x16) {
        return;
    }
    switch (a1) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        a0->unk_272 += a2;
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        a0->unk_26C = 0;
        a0->unk_90 = 0;
        a0->object_variant_enabled = 0;
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        a0->unk_270 += a2;
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        a0->unk_26E += a2;
        break;
    }
}

void math_NegNormalize3D(s32 *input_vector, s16 *output_vector) {
    u32 dist_sq = (u32)((input_vector[0] * input_vector[0]) +
                        (input_vector[1] * input_vector[1]) +
                        (input_vector[2] * input_vector[2]));
    u32 vector_length;
    if (dist_sq < 0x400) {
        vector_length = ((u32)(g_sqrt_table_u8[dist_sq])) >> 3;
    } else {
        s32 sp_tmp;
        gte_Lzc(dist_sq, &sp_tmp);
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = dist_sq >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            vector_length = (hi << 16) >> (0x13 - v1_m);
        }
    }
    output_vector[0] = (s16)(((-input_vector[0]) << 12) / ((s32)vector_length));
    output_vector[1] = (s16)(((-input_vector[1]) << 12) / ((s32)vector_length));
    output_vector[2] = (s16)(((-input_vector[2]) << 12) / ((s32)vector_length));
}

void fighter_RelocateAndStartEventMove(Unk80101EC8Record *arg0) {
    VECTOR var_0;
    VECTOR var_1;
    s32 idx;
    s16 *tbl;
    s32 var_4;
    u8 cnt;
    u16 *r1;
    void *r2;
    s32 vx;
    s32 vz;

    idx = arg0->index;
    tbl = fight_GetStartPositionPairTable();
    cnt = arg0->reposition_count;
    if (cnt < 0x40) {
        arg0->reposition_count = cnt + 1;
    }
    var_4 = 0x3C - ((arg0->reposition_count - 1) * 4);
    if (var_4 < 10) {
        var_4 = 10;
    }
    if (g_fight_stage_index == 3) {
        vx = 0x2EE0;
        if (arg0->other->world_pos.x >= 0x3E9) {
            vx = -0x2710;
        }
        var_1.vx = vx;
        vz = 0x1770;
        if (arg0->other->world_pos.z > 0) {
            vz = -0x1770;
        }
        var_1.vz = vz;
        var_0.vx = (var_1.vx * var_4 + arg0->world_pos.x * (100 - var_4)) / 100;
        var_0.vz = (var_1.vz * var_4 + arg0->world_pos.z * (100 - var_4)) / 100;
    } else {
        tbl += (g_fight_stage_index * 12 + idx * 3);
        var_0.vx = tbl[0];
        var_0.vz = tbl[2];
    }
    var_0.vx -= arg0->world_pos.x;
    var_0.vz -= arg0->world_pos.z;
    arg0->world_pos.x += var_0.vx;
    arg0->world_pos.z += var_0.vz;
    arg0->base_pos.x += var_0.vx;
    arg0->base_pos.z += var_0.vz;
    arg0->collision_pos.vx += var_0.vx;
    arg0->collision_pos.vz += var_0.vz;
    arg0->base_velocity.vx = 0;
    arg0->base_velocity.vy = 0;
    arg0->base_velocity.vz = 0;
    arg0->decaying_xz_velocity.vx = 0;
    arg0->decaying_xz_velocity.vy = 0;
    arg0->decaying_xz_velocity.vz = 0;
    r1 = motion_ResolveMoveData(
        arg0, arg0->cur_move->event_move_table_id, &arg0->move_is_common);
    r2 = motion_ResolveMoveData(arg0, r1[0x1D], &arg0->move_is_common);
    fighter_StartMoveScript(idx, r2, arg0->move_is_common);
    fight_RecordReplayEventAndDispatchSoundOrEffect(
        arg0->index, 0x30, &arg0->world_pos.x, 0);
}

void fighter_DispatchBoneHitSoundEvent(
    s32 a0, Unk80101EC8Record *ptr, s32 arg2, Tbl8008E194 *a3, s32 *stack_a2,
    s32 stack_v1) {
    Unk80101EC8Record *chk_obj;

    if (a0 == 1) {
        return;
    }

    if (stack_v1 != 0) {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            ptr->other->index, 0x2B, stack_a2, (s16 *)0);
        return;
    }

    chk_obj = ptr->other;
    {
        s16 f86 = chk_obj->variant_index;
        s16 f8E = chk_obj->unk_8E;

        if (f86 == f8E) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                chk_obj->index, 0x28, stack_a2, (s16 *)0);
            return;
        }

        {
            s16 f88 = chk_obj->object_variant_index;
            if (f86 == f88 && a3 != 0) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    chk_obj->index, 0x27, stack_a2, (s16 *)0);
                return;
            }
        }
    }

    if (arg2 == 0) {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            ptr->other->index, 0x22, stack_a2, (s16 *)0);
        return;
    }

    if (arg2 < 6) {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            ptr->other->index, 0x23, stack_a2, (s16 *)0);
        return;
    }

    fight_RecordReplayEventAndDispatchSoundOrEffect(
        ptr->other->index, 0x24, stack_a2, (s16 *)0);
}

s32 func_8002798C(Unk80101EC8Record *a0) {
    s32 ret = 0;

    if (a0->move_kind == 6 || a0->move_kind == 0x25 || a0->move_kind == 0x33 ||
        a0->move_kind == 4 || a0->move_kind == 0x14 ||
        a0->move_frame < a0->frame_window_start ||
        a0->frame_window_end < a0->move_frame) {
        if (a0->object_variant_index != -1) {
            u16 state = a0->move_kind;
            if (state == 0xF || state == 0x1C || state == 0x1D ||
                state == 0x1E || state == 0x1F || state == 0x20 ||
                state == 0x21 || state == 0x2B) {
                ret = 1;
            }
        }
    } else {
        ret = 1;
    }
    return ret;
}

void fighter_TryDropQueuedHeadAtBone(Unk80101EC8Record *a0) {
    s16 v1 = a0->variant_index;
    if (v1 == a0->object_variant_index) {
        if (a0->object_variant_enabled) {
            if (func_8002798C(a0)) {
                MATRIX **v0 = node_GetSlotMatrixPointerArray(a0->index);
                object_DropQueuedHeadAt(a0, (Vec3i32 *)v0[19]->t);
                a0->object_variant_enabled = 0;
                a0->variant_index = a0->default_variant_index;
            }
        }
    }
}

extern u8 D_8008EB74[3][2][2];

/* the tail fighter_ResolveBoneHit's cases 0, 1-3 and 4-5 share */
static inline s32 fighter_ResolveBoneHitTail(
    s32 arg0, Tbl8008E194 *var_0, Unk80101EC8Record *arg1, s32 var_4,
    s32 *scr) {
    if (arg0 == 1 && var_0->unkD == 2) {
        arg1->unk_272++;
    }
    if (g_fight_mode_selector != 5) {
        arg1->unk_272++;
    }
    arg1->pending_move_event = 5;
    fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
    fighter_TryDropQueuedHeadAtBone(arg1);
    return 0;
}

s32 fighter_ResolveBoneHit(
    s32 arg0, Unk80101EC8Record *arg1, s32 arg2, s32 thresh, s32 flag,
    Tbl8008E194 *rec, s32 arg6, s32 *out) {
    /* FAKE: var_0, var_1: copies of the stack-passed parameter `rec` (an
     * unmodified stack parameter gets half the register priority); rec used
     * directly: score 78 (Ruling 12) */
    Tbl8008E194 *var_0; /* the record, read field by field */
    /* the record, passed on to fighter_DispatchBoneHitSoundEvent */
    Tbl8008E194 *var_1;
    s16 *vec;
    s32 *scr;
    s32 var_4;
    Unk80101EC8Record *var_5;
    s32 dot;
    s32 var_7;
    u16 var_8;
    s32 diff;
    s32 var_10;
    u32 sign;
    s32 var_12;
    s32 code;

    /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached by the
     * first's address, passed to fighter_StoreBoneLocalVectorAndArmEventTrigger
     * and read as vec[0..2] (Q96); as s16[3] or {x,y,z}: score 2 */
    vec = &D_800A37E8;
    var_4 = arg1->index;
    var_5 = arg1->other;
    var_0 = rec;
    var_1 = rec;
    scr = &SPAD->unkA8[var_4][arg2].x;
    *out = 0;
    if (arg1->character_roster_index == 0x1C) {
        dot = (arg1->bone_y_axis.x * D_800A37E8 +
               arg1->bone_y_axis.y * D_800A37EA +
               arg1->bone_y_axis.z * D_800A37EC) >>
              12;
        dot &= 0x1FFF;
        if (dot >= 0x1000) {
            dot -= 0x2000;
        }
        var_7 = dot < 0x400;
        if (!((0xE >> arg2) & 1) || var_7) {
            if (arg1->is_defeated == 0) {
                arg1->pending_move_event = 0x1E;
            }
            if (arg0 == 0 && var_5->pending_move_event == -1) {
                var_5->pending_move_event = var_0 == NULL ? 0xB : 0x19;
            }
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                var_4, 0x12, scr, 0);
            return 0;
        }
    }
    if (arg1->character_roster_index == 0xD && arg1->is_defeated == 0 &&
        arg1->move_kind != 0x2E) {
        fighter_RelocateAndStartEventMove(arg1);
        return 2;
    }
    var_8 = arg1->move_kind;
    if (var_8 == 4 || var_8 == 0x14) {
        fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
        fighter_DispatchBoneHitSoundEvent(arg0, arg1, arg2, var_1, scr, arg6);
        return 1;
    }
    if (arg1->character_roster_index == 0x1F) {
        fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
        fighter_DispatchBoneHitSoundEvent(arg0, arg1, arg2, var_1, scr, arg6);
        arg1->pending_move_event = 7;
        return 1;
    }
    if ((var_8 == 2 || var_8 == 0x1B || var_8 == 0x28 || var_8 == 0x26) &&
        arg1->move_frame < arg1->unk_A0) {
        if (arg0 == 0) {
            diff = var_5->power - arg1->power;
            var_10 = math_ClassifyAbsMagnitude(diff);
            sign = (u32)diff >> 31;
            var_12 = arg1->unk_AF == var_5->unk_AF;
            arg1->unk_B4 = var_5->unk_B4 = var_12;
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                var_4, var_12 ? 0xF : 2, scr, 0);
            code = arg1->pending_move_event = D_8008EB74[var_10][sign][var_12];
            if (code == 0) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4 == 0, 0x25, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x25, scr, 0);
            } else if (code == 1) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4 == 0, 0x26, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x25, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x2D, scr, 0);
            } else if (code == 2) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4 == 0, 0x26, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x26, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x2D, scr, 0);
                fighter_TryDropQueuedHeadAtBone(arg1);
            } else {
                return 0;
            }
            if (arg1->pending_move_event != 2) {
                return 0;
            }
            if (vec[1] * vec[1] < vec[0] * vec[0] + vec[2] * vec[2]) {
                fighter_SetTurnDelta(arg1, ratan2(vec[0], vec[2]));
                arg1->decaying_xz_velocity.vx -= vec[0] / 16;
                arg1->decaying_xz_velocity.vz -= vec[2] / 16;
            }
            return 0;
        }
        if (arg0 == 1) {
            if (var_0->unkC == 0) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 2, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x25, scr, 0);
                arg1->pending_move_event = 0;
                *out = arg0;
                return 0;
            }
            if (var_0->unkC == 1) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 2, scr, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    var_4, 0x25, scr, 0);
                arg1->pending_move_event = 1;
                fighter_SetTurnDelta(arg1, ratan2(vec[0], vec[2]));
                arg1->decaying_xz_velocity.vx -= vec[0] / 16;
                arg1->decaying_xz_velocity.vz -= vec[2] / 16;
                *out = arg0;
                return 0;
            }
        }
    }
    if (g_fight_mode_selector == 0 && var_4 == 0) {
        fighter_AccumulateBoneGroupFlagWhenGameModeZero(0, arg2);
    }
    fighter_DispatchBoneHitSoundEvent(arg0, arg1, arg2, var_1, scr, arg6);
    var_8 = arg1->move_kind;
    if (var_8 == 6 || var_8 == 9) {
        if (flag) {
            arg1->pending_move_event =
                arg1->bone_triangle_cross_y_positive ? 6 : 7;
            fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
            return 1;
        }
        arg1->pending_move_event = arg1->bone_triangle_cross_y_positive ? 3 : 4;
        fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
        fighter_AccumulateOrClearByBoneGroup(arg1, arg2, 1);
        return 0;
    }
    if (arg0 == 1 && var_0->unk0 == 4) {
        s16 kind = arg1->character_data_index;
        if (kind == 2 || kind == 4 || kind == 0xE || kind == 0xF) {
            fighter_AccumulateOrClearByBoneGroup(
                arg1, arg2, var_0->unkD == 2 ? 2 : 1);
            arg1->pending_move_event = 0x1B;
            fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
            return 0;
        }
    }
    switch (arg2) {
    case 0:
        if (flag) {
            arg1->pending_move_event = thresh > 0x400 ? 6 : 9;
            fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
            fighter_TryDropQueuedHeadAtBone(arg1);
            return 1;
        }
        return fighter_ResolveBoneHitTail(arg0, var_0, arg1, var_4, scr);
    case 1:
    case 2:
    case 3:
        if (flag) {
            arg1->pending_move_event = thresh > 0x400 ? 7 : 9;
            fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
            fighter_TryDropQueuedHeadAtBone(arg1);
            return 1;
        }
        return fighter_ResolveBoneHitTail(arg0, var_0, arg1, var_4, scr);
    case 4:
    case 5:
        if (!flag) {
            return fighter_ResolveBoneHitTail(arg0, var_0, arg1, var_4, scr);
        }
        arg1->pending_move_event = thresh > 0x400 ? 8 : 9;
        fighter_StoreBoneLocalVectorAndArmEventTrigger(arg1, arg2, vec);
        fighter_TryDropQueuedHeadAtBone(arg1);
        return 1;
    case 6:
    case 7:
    case 8:
    case 9:
        if (g_fight_mode_selector != 5) {
            arg1->unk_26C = 0;
        }
        arg1->pending_move_event = 4;
        arg1->unk_90 = 0;
        fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
        fighter_TryDropQueuedHeadAtBone(arg1);
        arg1->object_variant_enabled = 0;
        return 0;
    case 10:
    case 11:
    case 12:
    case 13:
        if (g_fight_mode_selector != 5) {
            arg1->unk_270++;
        }
        arg1->pending_move_event = 5;
        fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
        fighter_TryDropQueuedHeadAtBone(arg1);
        return 0;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        if (g_fight_mode_selector != 5) {
            arg1->unk_26E++;
        }
        arg1->pending_move_event = 3;
        fight_RecordReplayEventAndDispatchSoundOrEffect(var_4, 3, scr, 0);
        fighter_TryDropQueuedHeadAtBone(arg1);
        return 0;
    }
}

s32 fighter_ResolveWeaponContact(Unk80101EC8Record *arg0, s32 *arg1) {
    Unk80101EC8Record *temp_s4;
    u16 state;
    s32 var_s1;
    s16 var_v0;
    s32 ret;

    temp_s4 = arg0->other;
    ret = 1;
    state = arg0->move_kind;
    if (state == 4) {
        goto ret_one;
    }
    if (state == 0x14) {
        return ret;
    }
    {
        u16 temp_v0 = temp_s4->move_kind;
        if (temp_v0 == 4) {
            goto ret_one;
        }
        if (temp_v0 == 0x14) {
            goto ret_one;
        }
        {
            s32 d_val;
            s32 temp_a1_2;
            s32 temp_s5;

            if (state != 0x13) {
                if (state != 0x19 && state != 0x1A && state != 2 &&
                    state != 0x26 && state != 0x1B && state != 0x15 &&
                    state != 0x25 && state != 0x2C && state != 0xC) {
                ret_one:
                    return 1;
                }
            }
            var_s1 = 0;
            d_val = g_fight_weapon_contact_result;
            temp_a1_2 = (d_val >> arg0->index) & 1;
            temp_s5 = (d_val >> temp_s4->index) & 1;
            if (arg0->wielded_object_live_state != 0) {
                var_s1 = temp_a1_2 == 0;
            }
            if (var_s1 != 0) {
                s16 temp_v1_2 = arg0->character_roster_index;
                if (temp_v1_2 != 0x1D) {
                    if (temp_v1_2 != 0xE) {
                        goto block_20;
                    }
                    return ret;
                }
                goto block_49;
            }
        block_20: {
            s16 temp_v1_3 = temp_s4->weapon_hit_phases[temp_s5];
            if (temp_v1_3 == 0) {
                if (arg0->weapon_hit_phases[temp_a1_2] > 0) {
                    var_v0 = 0x19;
                    if (var_s1 == 0) {
                    set_0xB:
                        var_v0 = 0xB;
                    }
                do_store_calls:
                    arg0->pending_move_event = var_v0;
                do_calls:
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        arg0->index, 1, arg1, (s16 *)0);
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        arg0->index, 0x25, arg1, (s16 *)0);
                    return ret;
                }
                goto block_49;
            }
            {
                s16 temp_v0_3 = arg0->weapon_hit_phases[temp_a1_2];
                s16 var_v0_2;
                if (temp_v0_3 == temp_v1_3) {
                    /* FAKE: do-while(0) loop depth weights these refs in
                     * register allocation; without it score 12
                     * (do-while-zero-exception) */
                    do {
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            arg0->index, 1, arg1, (s16 *)0);
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            arg0->index, 0x25, arg1, (s16 *)0);
                    } while (0);
                    if (arg0->weapon_hit_phases[temp_a1_2] == 5) {
                        if (arg0->weapon_id == 6 || arg0->weapon_id == 7 ||
                            temp_s4->weapon_id == 6 ||
                            temp_s4->weapon_id == 7) {
                            if (var_s1 != 0) {
                                goto sel19;
                            }
                            var_v0_2 = 0xB;
                            goto block_48;
                        sel19:
                            /* FAKE: store duplicated into this arm instead of
                             * sharing block_48's copy; cross-jumping picks
                             * which copy survives; sharing it scores 2 */
                            arg0->pending_move_event = 0x19;
                            goto block_49;
                        }
                        D_800A38A8 = 1;
                        D_800A3876 = -1;
                        goto block_49;
                    }
                    {
                        /* FAKE: named intermediate for the selected constant
                         * keeps the two constants in the target's order (a
                         * plain ternary into var_v0_2 reorders them: score
                         * 2) (Q129). */
                        s32 next_286 = (var_s1 == 0) ? 0xB : 0x19;
                        var_v0_2 = next_286;
                    }
                    goto block_48;
                }
                if (temp_v1_3 < temp_v0_3) {
                    s16 var_v0_4 = 0x19;
                    if (var_s1 == 0) {
                        goto set_0xB;
                    }
                    /* FAKE: store duplicated into the `<` arm instead of
                     * sharing do_store_calls's copy (cross-jump tail merge);
                     * sharing it scores 4 */
                    arg0->pending_move_event = var_v0_4;
                    goto do_calls;
                }
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    arg0->index, 0x26, arg1, (s16 *)0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    arg0->index, 0x2D, arg1, (s16 *)0);
                var_v0_2 = 0x1A;
                if (var_s1 == 0) {
                    s32 temp_v1_4 = -arg0->orientation.vy;
                    s32 temp_v1_5 =
                        (s32)(Judge[(temp_v1_4 + 0x400) & 0xFFF] *
                                  temp_s4->weapon_point_delta[temp_s5].vx +
                              Judge[temp_v1_4 & 0xFFF] *
                                  temp_s4->weapon_point_delta[temp_s5].vz) >>
                        0xC;
                    s32 temp_a0_2 = temp_s4->weapon_point_delta[temp_s5].vy;
                    s32 var_a1 = temp_a0_2;
                    s32 var_v0_3;
                    if (temp_a0_2 < 0) {
                        var_a1 = -temp_a0_2;
                    }
                    var_v0_3 = temp_v1_5;
                    if (temp_v1_5 < 0) {
                        var_v0_3 = -temp_v1_5;
                    }
                    if (var_v0_3 < var_a1) {
                        var_v0_2 = 0x14;
                        if (temp_a0_2 > 0) {
                            var_v0_2 = 0x13;
                        }
                    } else {
                        var_v0_2 = 0x15;
                        if (temp_v1_5 <= 0) {
                            var_v0_2 = 0x16;
                        }
                    }
                }
            block_48:
                arg0->pending_move_event = var_v0_2;
            }
        }
        block_49:
            return ret;
        }
    }
}

void fighter_TriggerOpponentResponseOnMoveFrame(void) {
    s32 i;
    Unk80101EC8Record *base;
    u16 state;

    for (i = 0; i < 2; i++) {
        base = &D_80101EC8[i];
        if (base->character_roster_index != 0x1B ||
            base->move_frame_unchanged != 0 || base->move_kind != 0xB ||
            base->move_frame != base->release_frame) {
            continue;
        }
        state = base->other->move_kind;
        if (state == 0x26 || state == 0x1B || state == 2 || state == 0x15 ||
            state == 0x24 || state == 0x25 || state == 8 || state == 0x22 ||
            state == 0x23 || state == 0 || state == 0x10 || state == 0x13 ||
            state == 0x30 || state == 0x31 || state == 0x1A || state == 0xB ||
            state == 0x12 || state == 0x2A || state == 0xC || state == 0x19) {
            if (g_fighter_horizontal_separation < D_800A3134) {
                base->other->pending_move_event = 0x1C;
                fighter_TryDropQueuedHeadAtBone(base->other);
                if (base->other->move_kind == 0x25) {
                    base->other->variant_index =
                        base->other->default_variant_index;
                }
            }
        }
    }
}

extern s32 D_800A3144[2];

/* math_SqrtLut -- integer square root of x >= 0 by the byte LUT (>> 3 below
 * 0x400; above it the GTE LZC picks an even shift, x is replaced in place by
 * the table byte of x >> shift, and the byte is scaled back up). */
static inline s32 math_SqrtLut(s32 x) {
    s32 lzc_out;

    if ((u32)x < 0x400) {
        return (u32)g_sqrt_table_u8[x] >> 3;
    } else {
        /* Ruling 11 (reused local), as in src/main/87A0.c: leading-zero
         * count, then the table shift computed from it in place; used to
         * index g_sqrt_table_u8 and derive the final shift. FAKE: the copy
         * of lzc_out, then overwritten; the one-expression form
         * `var_20 = 0x16 - (lzc_out & ~1)`: score 4 (Q129) */
        s32 var_20;

        gte_Lzc(x, &lzc_out);
        var_20 = lzc_out;
        var_20 = 0x16 - (var_20 & ~1);
        /* x: the radicand, then its square-root table byte, looked up in
         * place as SOTN's shop takes `itemId = D_us_801814D4[itemId]` (Q51);
         * a fresh byte local: score 12 */
        /* SOTN: src/st/lib/e_shop.c:3168 @db41b28 */
        x = g_sqrt_table_u8[(u32)x >> var_20];
        return (u32)(x << 16) >> (0x13 - ((u32)var_20 >> 1));
    }
}

/* Body-overlap push between the two characters (D_80101EC8[0] and [1]),
 * skipped while either's move_kind is 0x28. Each character's two points (joint
 * 1, and the midpoint of joints 15 and 19) are tested pairwise with the radii
 * D_800A3144[0..1]; an overlap (at most 0x80) pushes the characters apart along
 * x/z, scaled by the distance. After an overlap, the move_kind states and the
 * orientation.vy difference (folded to 0..0x800) pick the reaction
 * (pending_move_event, D_800A38A8 / D_800A3876). The push is added to
 * unk_134.vx / .vz. */
void fighter_ResolveBodyOverlap(void) {
    s32 var_1;
    s32 i;
    s32 j;
    s32 c;
    s32 var_5;
    s32 var_6;
    s32 var_7;
    s32 var_8;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 var_12;
    /* the squared distance, then its square root in place (clamped to 1),
     * as SOTN's cloaked knight takes `distance = SquareRoot0(distance)`
     * (Q51) */
    /* SOTN: src/st/nz1/e_cloaked_knight.c:83 @db41b28 */
    s32 var_13;
    s32 var_15;
    s32 var_16;
    s32 flag1;
    s32 flag2;
    s32 var_19;

    if (D_80101EC8[0].move_kind == 0x28 || D_80101EC8[1].move_kind == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        SPAD->unk78[c][0].x = SPAD->unkA8[c][1].x;
        SPAD->unk78[c][0].y = SPAD->unkA8[c][1].y;
        SPAD->unk78[c][0].z = SPAD->unkA8[c][1].z;
        SPAD->unk78[c][1].x = (SPAD->unkA8[c][15].x + SPAD->unkA8[c][19].x) / 2;
        SPAD->unk78[c][1].y = (SPAD->unkA8[c][15].y + SPAD->unkA8[c][19].y) / 2;
        SPAD->unk78[c][1].z = (SPAD->unkA8[c][15].z + SPAD->unkA8[c][19].z) / 2;
    }
    var_1 = 0;
    var_5 = var_6 = var_7 = var_8 = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = SPAD->unk78[0][i].x - SPAD->unk78[1][j].x;
            dy = SPAD->unk78[0][i].y - SPAD->unk78[1][j].y;
            dz = SPAD->unk78[0][i].z - SPAD->unk78[1][j].z;
            var_12 = D_800A3144[i] + D_800A3144[j];
            if (dx > var_12 || dx < -var_12 || dy > var_12 || dy < -var_12 ||
                dz > var_12 || dz < -var_12) {
                continue;
            }
            var_13 = dx * dx + dy * dy + dz * dz;
            if (var_12 * var_12 < var_13) {
                continue;
            }
            var_1++;
            var_13 = math_SqrtLut(var_13);
            var_15 = var_12 - var_13;
            if (var_15 > 0x80) {
                var_15 = 0x80;
            }
            if (var_13 == 0) {
                var_13 = 1;
            }
            var_16 = (D_80101EC8[1].power - D_80101EC8[0].power) / 4;
            var_5 += (dx * (var_15 * (0x400 + var_16)) / var_13) >> 10;
            var_6 += (dz * (var_15 * (0x400 + var_16)) / var_13) >> 10;
            var_7 -= (dx * (var_15 * (0x400 - var_16)) / var_13) >> 10;
            var_8 -= (dz * (var_15 * (0x400 - var_16)) / var_13) >> 10;
        }
    }
    if (var_1 == 0) {
        return;
    }
    flag1 = 0;
    if (D_80101EC8[0].move_kind == 0x13 || D_80101EC8[0].move_kind == 0x1B ||
        D_80101EC8[0].move_kind == 0x30) {
        if (D_80101EC8[0].forward_move_rate > 0x9C4) {
            flag1 = 1;
        }
    }
    flag2 = 0;
    if (D_80101EC8[1].move_kind == 0x13 || D_80101EC8[1].move_kind == 0x1B ||
        D_80101EC8[1].move_kind == 0x30) {
        if (D_80101EC8[1].forward_move_rate > 0x9C4) {
            flag2 = 1;
        }
    }
    var_19 =
        (D_80101EC8[0].orientation.vy - D_80101EC8[1].orientation.vy) & 0xFFF;
    if (var_19 >= 0x800) {
        var_19 = 0x1000 - var_19;
    }
    if (flag1) {
        if (flag2) {
            if (var_19 > 0x600) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    0, 0x21, &D_80101EC8[0].world_pos.x, 0);
                D_80101EC8[1].pending_move_event = 0xF;
                D_80101EC8[0].pending_move_event = 0xF;
                fighter_TryDropQueuedHeadAtBone(&D_80101EC8[0]);
                fighter_TryDropQueuedHeadAtBone(&D_80101EC8[1]);
                var_1 = 0;
            }
        } else {
            if (var_19 < 0x400 && D_80101EC8[1].move_kind == 0x15) {
                D_80101EC8[1].pending_move_event = 0x10;
                fighter_TryDropQueuedHeadAtBone(&D_80101EC8[1]);
            }
            if (var_19 > 0x600 && D_80101EC8[1].move_kind == 0x15) {
                if (D_80101EC8[0].weapon_id == 6 ||
                    D_80101EC8[0].weapon_id == 7 ||
                    D_80101EC8[1].weapon_id == 6 ||
                    D_80101EC8[1].weapon_id == 7) {
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        1, 0x21, &D_80101EC8[1].world_pos.x, 0);
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        1, 0x2D, &D_80101EC8[1].world_pos.x, 0);
                    D_80101EC8[1].pending_move_event = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (flag2) {
        if (var_19 < 0x400 && D_80101EC8[0].move_kind == 0x15) {
            D_80101EC8[0].pending_move_event = 0x10;
            fighter_TryDropQueuedHeadAtBone(&D_80101EC8[0]);
        }
        if (var_19 > 0x600 && D_80101EC8[0].move_kind == 0x15) {
            if (D_80101EC8[0].weapon_id == 6 || D_80101EC8[0].weapon_id == 7 ||
                D_80101EC8[1].weapon_id == 6 || D_80101EC8[1].weapon_id == 7) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    0, 0x21, &D_80101EC8[0].world_pos.x, 0);
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    0, 0x2D, &D_80101EC8[0].world_pos.x, 0);
                D_80101EC8[0].pending_move_event = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (var_1 != 0) {
        D_80101EC8[0].decaying_xz_velocity.vx += var_5;
        D_80101EC8[0].decaying_xz_velocity.vz += var_6;
        D_80101EC8[1].decaying_xz_velocity.vx += var_7;
        D_80101EC8[1].decaying_xz_velocity.vz += var_8;
    }
}

/* One 16-byte entry of the list fight_GetPositionRecordList returns (terminated
 * by a zero `type`); fight_ClearPositionRecordUsage clears every entry's
 * `used`. */
typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} PosRec;

void fight_ClearPositionRecordUsage(void) {
    PosRec *ptr = (PosRec *)fight_GetPositionRecordList();
    while (ptr->type != 0) {
        ptr->used = 0;
        ptr++;
    }
}

s32 math_PointInTriangleXZ(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3);
s32 math_TriangleNormalYawWithPositiveYHalfTurn(
    VECTOR *a0, VECTOR *a1, VECTOR *a2);
void snd_QueuePositionSequence(Unk80107850Rec *arg0);

/* Tests the unused PosRec entries inside grid `idx`'s x/z bounds against the
 * grid's two triangles (points 0,1,2 and 1,2,3) with math_PointInTriangleXZ.
 * `tbl` holds two 2x2 grids of points (entry idx * 4 + row * 2 + col); the
 * bounds go to the scratch record at 0x1F8002B8. A type-2 hit restarts on grid
 * 1 when `flag` is set and idx is 0; otherwise it calls
 * node_StartFrameAnimatedEntryAndInitPairedNode / snd_QueuePositionSequence,
 * marks the entry used and returns 0. Any other hit, or a miss with `flag` set
 * and idx not 0, stores the average of unk78.y and the top y in unk100[0].y and
 * returns
 * 1. Returns 0 when the list runs out. */
s32 fight_ProcessPositionRecordsInQuad(s32 idx, s32 flag, Unk80107850Rec *tbl) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    PosRec *rec;
    Unk80107850Rec *a;
    Unk80107850Rec *b;
    Unk80107850Rec *c;
    s32 i;
    s32 n;
    /* two values: the grid row i / 2, then the list index passed to
     * node_StartFrameAnimatedEntryAndInitPairedNode (Ruling 11) */
    s32 temp;
    /* two values: the grid column i & 1, then the triangle number
     * (Ruling 11) */
    s32 temp2;

    scr->bbox_max = tbl[idx * 4];
    scr->unk78 = scr->bbox_max;

    for (i = 1; i < 4; i++) {
        temp = i / 2;
        temp2 = i & 1;
        n = idx * 4 + temp * 2 + temp2;
        if (tbl[n].x < scr->unk78.x) {
            scr->unk78.x = tbl[n].x;
        } else if (scr->bbox_max.x < tbl[n].x) {
            scr->bbox_max.x = tbl[n].x;
        }
        if (tbl[n].z < scr->unk78.z) {
            scr->unk78.z = tbl[n].z;
        } else if (scr->bbox_max.z < tbl[n].z) {
            scr->bbox_max.z = tbl[n].z;
        }
        if (tbl[n].y > scr->bbox_max.y) {
            scr->bbox_max.y = tbl[n].y;
        }
    }

    rec = (PosRec *)fight_GetPositionRecordList();
    for (temp = 0; rec->type != 0; temp++, rec++) {
        if (rec->used != 0)
            continue;
        if (rec->y > scr->bbox_max.y)
            continue;
        if (rec->x < scr->unk78.x)
            continue;
        if (scr->bbox_max.x < rec->x)
            continue;
        if (rec->z < scr->unk78.z)
            continue;
        if (scr->bbox_max.z < rec->z)
            continue;

        scr->unk100[0].x = rec->x;
        scr->unk100[0].z = rec->z;
        for (temp2 = 0; temp2 < 2; temp2++) {
            if (temp2 == 0) {
                a = &tbl[idx * 4];
                b = &tbl[idx * 4 + 1];
                c = &tbl[idx * 4 + 2];
            } else {
                a = &tbl[idx * 4 + 1];
                b = &tbl[idx * 4 + 2];
                c = &tbl[idx * 4 + 3];
            }
            if (math_PointInTriangleXZ(
                    (s32 *)a, (s32 *)b, (s32 *)c, &scr->unk100[0].x) != 0) {
                if (rec->type != 2)
                    goto hit;
                if (flag != 0 && idx == 0) {
                    idx = 1;
                    temp2 = -1;
                    continue;
                }
                node_StartFrameAnimatedEntryAndInitPairedNode(
                    temp, math_TriangleNormalYawWithPositiveYHalfTurn(
                              (VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                snd_QueuePositionSequence(a);
                rec->used = 1;
                return 0;
            }
        }
        if (flag == 0 || idx == 0)
            continue;
    hit:
        scr->unk100[0].y = (scr->unk78.y + scr->bbox_max.y) / 2;
        return 1;
    }
    return 0;
}

/* 1 when the box unk78 (min) / bbox_max (max) and the box other_bbox_min (min)
 * / other_bbox_max (max) overlap on all three axes. */
static inline s32 box_overlap(Unk1F8002B8Rec *scr) {
    return scr->unk78.x <= scr->other_bbox_max.x &&
           scr->bbox_max.x >= scr->other_bbox_min.x &&
           scr->unk78.y <= scr->other_bbox_max.y &&
           scr->bbox_max.y >= scr->other_bbox_min.y &&
           scr->unk78.z <= scr->other_bbox_max.z &&
           scr->bbox_max.z >= scr->other_bbox_min.z;
}

/* Both are defined further down this file. */
extern s32 math_TryPrepareTriangleNormalFrame(Unk1F8002B8Rec *obj);
extern s32 func_8002DE20(Unk1F8002B8Rec *obj, Unk80107850Rec *p0,
                         Unk80107850Rec *p1, Unk80107850Rec *p2);

/* Blade contact test between the two D_80101EC8 records (called by
 * fight_UpdateFighterInteractionsAndPointCaches; the result goes to
 * g_fight_weapon_contact_result). Returns -1 unless both records' +0x3C are at
 * least 4. Borrows the first 16 SPAD->unkA8 points as two 2x2 grids per record
 * (fight_ProcessPositionRecordsInQuad's layout). A record whose +0xE is not 6
 * or 7, in +0x6A state 2, 0x1B, 0x28 or 0x26, whose grid
 * fight_ProcessPositionRecordsInQuad hits plays effects 1, 0x26 and 0x2D and
 * sets +0x286 to 0x19 or 0xB; the hit counts only when +0x40 lies within
 * +0xA1..+0xA3 (+0xA2..+0xA4 for grid 1, which is tested only with +0x8C set).
 * Then each record-0 triangle math_TryPrepareTriangleNormalFrame accepts is
 * tested against each record-1 triangle (bounding boxes, then func_8002DE20),
 * the rejected ones again with the roles swapped. A hit returns (record-1 grid
 * << 1) | record-0 grid, else -1; the points are restored. */
s32 fighter_ProcessWeaponContacts(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    Unk80107850Rec *var_1 = SPAD->unkA8[0];
    Unk80107850Rec saved[16];
    s32 count[2];
    /* i, j, n, k: loop counters, each shared by several loops */
    s32 i;
    s32 j;
    s32 n;
    u32 mask;
    s32 k;
    s32 *p;
    /* rec: the record pointer of the first record loop, then of the
     * second (Ruling 11, ordinary-c-judge-decidable) */
    Unk80101EC8Record *rec;

    if (D_80101EC8[0].update_count < 4) {
        return -1;
    }
    if (D_80101EC8[1].update_count < 4) {
        return -1;
    }
    for (k = 0; k < 16; k++) {
        saved[k] = SPAD->unkA8[0][k];
    }

    for (i = 0; i < 2; i++) {
        rec = &D_80101EC8[i];
        if (rec->weapon_id == 6 || rec->weapon_id == 7) {
            continue;
        }
        if (!(rec->move_kind == 2 || rec->move_kind == 0x1B ||
              rec->move_kind == 0x28 || rec->move_kind == 0x26)) {
            continue;
        }
        if (rec->weapon_id == 4 || rec->weapon_id == 5) {
            var_1[0] = SPAD->unk00[i][0];
            var_1[1] = SPAD->unk00[i][2];
            var_1[2] = rec->prev_weapon_points[0];
            var_1[3] = rec->prev_weapon_points[2];
            var_1[4] = SPAD->unk00[i][0];
            var_1[5] = SPAD->unk00[i][1];
            var_1[6] = rec->prev_weapon_points[0];
            var_1[7] = rec->prev_weapon_points[1];
        } else {
            var_1[0] = SPAD->unk00[i][0];
            var_1[1] = SPAD->unk00[i][1];
            var_1[2] = rec->prev_weapon_points[0];
            var_1[3] = rec->prev_weapon_points[1];
            if (rec->wielded_object_live_state != 0) {
                var_1[4] = SPAD->unk48[i][0];
                var_1[5] = SPAD->unk48[i][1];
                var_1[6] = rec->prev_second_weapon_points[0];
                var_1[7] = rec->prev_second_weapon_points[1];
            }
        }
        if (rec->move_frame >= rec->weapon_hit_start_frames[0] &&
            rec->move_frame <= rec->weapon_hit_end_frames[0] &&
            fight_ProcessPositionRecordsInQuad(
                0, rec->weapon_id == 4 || rec->weapon_id == 5, var_1) != 0) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 1, &scr->unk100[0].x, 0);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 0x26, &scr->unk100[0].x, 0);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 0x2D, &scr->unk100[0].x, 0);
            rec->pending_move_event =
                rec->wielded_object_live_state != 0 ? 0x19 : 0xB;
            rec->attack_hit_armed = 0;
        } else if (rec->wielded_object_live_state != 0 &&
                   rec->move_frame >= rec->weapon_hit_start_frames[1] &&
                   rec->move_frame <= rec->weapon_hit_end_frames[1] &&
                   fight_ProcessPositionRecordsInQuad(1, 0, var_1) != 0) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 1, &scr->unk100[0].x, 0);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 0x26, &scr->unk100[0].x, 0);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 0x2D, &scr->unk100[0].x, 0);
            rec->pending_move_event = 0xB;
            rec->attack_hit_armed = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        Unk80107850Rec *dst = &var_1[i * 8];
        rec = &D_80101EC8[i];
        dst[0] = SPAD->unk00[i][0];
        dst[1] = SPAD->unk00[i][1];
        dst[2] = rec->prev_weapon_points[0];
        dst[3] = rec->prev_weapon_points[1];
        count[i] = 2;
        if (rec->is_defeated != 0 || rec->weapon_live_state == 0 ||
            rec->character_roster_index == 0x1F) {
            dst[0].y = 100000;
            dst[1].y = 100000;
            dst[2].y = 100000;
            dst[3].y = 100000;
        }
        if (rec->wielded_object_live_state != 0) {
            dst[4] = SPAD->unk48[i][0];
            dst[5] = SPAD->unk48[i][1];
            dst[6] = rec->prev_second_weapon_points[0];
            dst[7] = rec->prev_second_weapon_points[1];
            count[i] += 2;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            p = (s32 *)((u8 *)var_1 + (i * 0x60 + j * 0x18));
            p[0] >>= 1;
            p[1] >>= 1;
            p[2] >>= 1;
            p[3] >>= 1;
            p[4] >>= 1;
            p[5] >>= 1;
        }
    }

    mask = 0;
    for (j = 0; j < count[0]; j++) {
        switch (j) {
        case 0:
            scr->shape_points[0] = &var_1[2];
            scr->shape_points[1] = &var_1[3];
            scr->shape_points[2] = &var_1[1];
            break;
        case 1:
            scr->shape_points[0] = &var_1[0];
            scr->shape_points[1] = &var_1[1];
            scr->shape_points[2] = &var_1[2];
            break;
        case 2:
            scr->shape_points[0] = &var_1[6];
            scr->shape_points[1] = &var_1[7];
            scr->shape_points[2] = &var_1[5];
            break;
        case 3:
            scr->shape_points[0] = &var_1[4];
            scr->shape_points[1] = &var_1[5];
            scr->shape_points[2] = &var_1[6];
            break;
        }
        if (math_TryPrepareTriangleNormalFrame(scr) == 0) {
            mask |= 1 << j;
            continue;
        }
        scr->bbox_max = *scr->shape_points[0];
        scr->unk78 = scr->bbox_max;
        for (k = 1; k < 3; k++) {
            if (scr->shape_points[k]->x < scr->unk78.x) {
                scr->unk78.x = scr->shape_points[k]->x;
            } else if (scr->bbox_max.x < scr->shape_points[k]->x) {
                scr->bbox_max.x = scr->shape_points[k]->x;
            }
            if (scr->shape_points[k]->y < scr->unk78.y) {
                scr->unk78.y = scr->shape_points[k]->y;
            } else if (scr->bbox_max.y < scr->shape_points[k]->y) {
                scr->bbox_max.y = scr->shape_points[k]->y;
            }
            if (scr->shape_points[k]->z < scr->unk78.z) {
                scr->unk78.z = scr->shape_points[k]->z;
            } else if (scr->bbox_max.z < scr->shape_points[k]->z) {
                scr->bbox_max.z = scr->shape_points[k]->z;
            }
        }
        for (n = 0; n < count[1]; n++) {
            switch (n) {
            case 0:
                scr->other_shape_points[0] = &var_1[10];
                scr->other_shape_points[1] = &var_1[11];
                scr->other_shape_points[2] = &var_1[9];
                break;
            case 1:
                scr->other_shape_points[0] = &var_1[8];
                scr->other_shape_points[1] = &var_1[9];
                scr->other_shape_points[2] = &var_1[10];
                break;
            case 2:
                scr->other_shape_points[0] = &var_1[14];
                scr->other_shape_points[1] = &var_1[15];
                scr->other_shape_points[2] = &var_1[13];
                break;
            case 3:
                scr->other_shape_points[0] = &var_1[12];
                scr->other_shape_points[1] = &var_1[13];
                scr->other_shape_points[2] = &var_1[14];
                break;
            }
            scr->other_bbox_max = *scr->other_shape_points[0];
            scr->other_bbox_min = scr->other_bbox_max;
            for (k = 1; k < 3; k++) {
                if (scr->other_shape_points[k]->x < scr->other_bbox_min.x) {
                    scr->other_bbox_min.x = scr->other_shape_points[k]->x;
                } else if (
                    scr->other_bbox_max.x < scr->other_shape_points[k]->x) {
                    scr->other_bbox_max.x = scr->other_shape_points[k]->x;
                }
                if (scr->other_shape_points[k]->y < scr->other_bbox_min.y) {
                    scr->other_bbox_min.y = scr->other_shape_points[k]->y;
                } else if (
                    scr->other_bbox_max.y < scr->other_shape_points[k]->y) {
                    scr->other_bbox_max.y = scr->other_shape_points[k]->y;
                }
                if (scr->other_shape_points[k]->z < scr->other_bbox_min.z) {
                    scr->other_bbox_min.z = scr->other_shape_points[k]->z;
                } else if (
                    scr->other_bbox_max.z < scr->other_shape_points[k]->z) {
                    scr->other_bbox_max.z = scr->other_shape_points[k]->z;
                }
            }
            if (box_overlap(scr) &&
                func_8002DE20(
                    scr, scr->other_shape_points[0], scr->other_shape_points[1],
                    scr->other_shape_points[2]) != 0) {
                for (k = 0; k < 16; k++) {
                    SPAD->unkA8[0][k] = saved[k];
                }
                return ((n >> 1) * 2) | (j >> 1);
            }
        }
    }
    if (mask == 0) {
        for (k = 0; k < 16; k++) {
            SPAD->unkA8[0][k] = saved[k];
        }
        return -1;
    }

    for (j = 0; j < count[1]; j++) {
        switch (j) {
        case 0:
            scr->shape_points[0] = &var_1[10];
            scr->shape_points[1] = &var_1[11];
            scr->shape_points[2] = &var_1[9];
            break;
        case 1:
            scr->shape_points[0] = &var_1[8];
            scr->shape_points[1] = &var_1[9];
            scr->shape_points[2] = &var_1[10];
            break;
        case 2:
            scr->shape_points[0] = &var_1[14];
            scr->shape_points[1] = &var_1[15];
            scr->shape_points[2] = &var_1[13];
            break;
        case 3:
            scr->shape_points[0] = &var_1[12];
            scr->shape_points[1] = &var_1[13];
            scr->shape_points[2] = &var_1[14];
            break;
        }
        if (math_TryPrepareTriangleNormalFrame(scr) == 0) {
            continue;
        }
        scr->bbox_max = *scr->shape_points[0];
        scr->unk78 = scr->bbox_max;
        for (k = 1; k < 3; k++) {
            if (scr->shape_points[k]->x < scr->unk78.x) {
                scr->unk78.x = scr->shape_points[k]->x;
            } else if (scr->bbox_max.x < scr->shape_points[k]->x) {
                scr->bbox_max.x = scr->shape_points[k]->x;
            }
            if (scr->shape_points[k]->y < scr->unk78.y) {
                scr->unk78.y = scr->shape_points[k]->y;
            } else if (scr->bbox_max.y < scr->shape_points[k]->y) {
                scr->bbox_max.y = scr->shape_points[k]->y;
            }
            if (scr->shape_points[k]->z < scr->unk78.z) {
                scr->unk78.z = scr->shape_points[k]->z;
            } else if (scr->bbox_max.z < scr->shape_points[k]->z) {
                scr->bbox_max.z = scr->shape_points[k]->z;
            }
        }
        for (n = 0; n < count[0]; n++) {
            if (!(mask & (1 << n))) {
                continue;
            }
            switch (n) {
            case 0:
                scr->other_shape_points[0] = &var_1[2];
                scr->other_shape_points[1] = &var_1[3];
                scr->other_shape_points[2] = &var_1[1];
                break;
            case 1:
                scr->other_shape_points[0] = &var_1[0];
                scr->other_shape_points[1] = &var_1[1];
                scr->other_shape_points[2] = &var_1[2];
                break;
            case 2:
                scr->other_shape_points[0] = &var_1[6];
                scr->other_shape_points[1] = &var_1[7];
                scr->other_shape_points[2] = &var_1[5];
                break;
            case 3:
                scr->other_shape_points[0] = &var_1[4];
                scr->other_shape_points[1] = &var_1[5];
                scr->other_shape_points[2] = &var_1[6];
                break;
            }
            scr->other_bbox_max = *scr->other_shape_points[0];
            scr->other_bbox_min = scr->other_bbox_max;
            for (k = 1; k < 3; k++) {
                if (scr->other_shape_points[k]->x < scr->other_bbox_min.x) {
                    scr->other_bbox_min.x = scr->other_shape_points[k]->x;
                } else if (
                    scr->other_bbox_max.x < scr->other_shape_points[k]->x) {
                    scr->other_bbox_max.x = scr->other_shape_points[k]->x;
                }
                if (scr->other_shape_points[k]->y < scr->other_bbox_min.y) {
                    scr->other_bbox_min.y = scr->other_shape_points[k]->y;
                } else if (
                    scr->other_bbox_max.y < scr->other_shape_points[k]->y) {
                    scr->other_bbox_max.y = scr->other_shape_points[k]->y;
                }
                if (scr->other_shape_points[k]->z < scr->other_bbox_min.z) {
                    scr->other_bbox_min.z = scr->other_shape_points[k]->z;
                } else if (
                    scr->other_bbox_max.z < scr->other_shape_points[k]->z) {
                    scr->other_bbox_max.z = scr->other_shape_points[k]->z;
                }
            }
            if (box_overlap(scr) &&
                func_8002DE20(
                    scr, scr->other_shape_points[0], scr->other_shape_points[1],
                    scr->other_shape_points[2]) != 0) {
                for (k = 0; k < 16; k++) {
                    SPAD->unkA8[0][k] = saved[k];
                }
                return ((j >> 1) * 2) | (n >> 1);
            }
        }
    }
    for (k = 0; k < 16; k++) {
        SPAD->unkA8[0][k] = saved[k];
    }
    return -1;
}

/* Sweeps a character's weapon segment (base -> tip, in the scratch record at
 * 0x1F8002B8) against its limb spheres. Takes the segment's pitch and yaw (a
 * negative square prints "ILLEGAL GUN MOTION" and returns) and unit direction
 * q, and pushes the tip out by q*4. If the yaw difference to obj->other's
 * opponent_bearing is under 0x400, each of character `id`'s 22 records
 * (g_fighter_bone_hit_records; records 6..9 only when obj+0x26C is set) is
 * tested: a math_TestPreparedSegmentSphere test with unk_0C / unk_0E sets bit i
 * in *arg1 and, only when the record's first halfword is nonzero, a second test
 * with unk_10 / unk_12 sets bit i in *arg2. Then collision_TestSegmentWithWork
 * casts the segment against the stage; when it returns nonzero and
 * collision_GetHitTag() != 7: if *arg1 is nonzero, the function skips to the
 * end when the cast point is no nearer the base than obj+0xF4 and otherwise
 * clears *arg1 and *arg2; then, unless `arg3`, effect 0xA plays at the cast
 * point. D_800A37E8.. receive -q. GTE: gte_Lzc (gte_ldlzc, gte_nop x2,
 * gte_stlzc) per square root, inline_o.h class (inline-asm-policy, owner ruling
 * 2026-09-26). Everything else is ordinary C, except the five Ruling 11 locals
 * (dx, dy, dz, temp, temp2) and the do-while(0), each annotated below. */
extern void math_PrepareSegmentCollisionFrame(Unk1F8002B8Rec *scr);
extern s32 math_TestPreparedSegmentSphere(
    Unk1F8002B8Rec *scr, Unk80107850Rec *pos, s32 threshold, s32 r_sq);
extern s32 collision_GetHitTag(void);

void fighter_CastWeaponSegment(
    Unk80101EC8Record *obj, u32 *arg1, u32 *arg2, s32 arg3) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 id = obj->index;
    Unk80101EC8Record *var_2 = obj->other;
    s32 work[64];
    s32 sp_tmp;
    s32 sp_tmp2;
    /* dx/dy/dz: a point's offset from the segment base -- the segment itself,
     * then the cast point (unk100[0]) and obj+0xF4 (Ruling 11) */
    s32 dx;
    s32 dy;
    s32 dz;
    s32 diff;
    /* temp: the squared horizontal length, then the segment length
     * (Ruling 11) */
    s32 temp;
    s32 hlen;
    /* FAKE: two values -- the first gte_Lzc input copy of `temp`, then the
     * g_sqrt_table_u8 byte; calling gte_Lzc(temp, ...) directly: score 2
     * (Ruling 11, Q28) */
    s32 temp2;
    s32 len_sq;
    s32 var_14;
    s32 qx;
    s32 qy;
    s32 qz;
    Unk80107850Rec *p;
    BoneHitRec *rec;
    s32 i;

    scr->shape_points[0] = &scr->point_slots.unk00[0];
    scr->shape_points[1] = &scr->point_slots.unk00[1];
    scr->unkC8 = scr->point_slots.unk00[1];
    dx = scr->shape_points[1]->x - scr->shape_points[0]->x;
    dy = scr->shape_points[1]->y - scr->shape_points[0]->y;
    dz = scr->shape_points[1]->z - scr->shape_points[0]->z;
    diff = (var_2->opponent_bearing - ratan2(dx, dz)) & 0xFFF;
    if (diff >= 0x800) {
        diff = 0x1000 - diff;
    }
    temp = dx * dx + dz * dz - dy * dy;
    if (temp < 0) {
        printf(D_80010478, temp);
        return;
    }
    temp2 = temp;
    if ((u32)temp < 0x400) {
        hlen = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        gte_Lzc(temp2, &sp_tmp);
        {
            s32 lz = ~1;
            s32 shift;
            lz &= sp_tmp;
            shift = 0x16 - lz;
            temp2 = g_sqrt_table_u8[(u32)temp >> shift];
            hlen = (u32)(temp2 << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    scr->unkF8.vx = -ratan2(dy, hlen);
    scr->unkF8.vy = ratan2(dx, dz);
    scr->unkF8.vz = 0;
    if (arg3 == 0) {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            id == 0, 0xB, &scr->unkC8.x, &scr->unkF8.vx);
    }
    len_sq = dx * dx + dy * dy + dz * dz;
    if ((u32)len_sq < 0x400) {
        temp = (u32)g_sqrt_table_u8[len_sq] >> 3;
    } else {
        gte_Lzc(len_sq, &sp_tmp2);
        {
            s32 lz = ~1;
            s32 shift;
            s32 tbl;
            lz &= sp_tmp2;
            shift = 0x16 - lz;
            tbl = g_sqrt_table_u8[(u32)len_sq >> shift];
            temp = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    qx = (dx << 12) / temp;
    qy = (dy << 12) / temp;
    qz = (dz << 12) / temp;
    *scr->shape_points[0] = *scr->shape_points[1];
    scr->shape_points[1]->x += qx * 4;
    scr->shape_points[1]->y += qy * 4;
    scr->shape_points[1]->z += qz * 4;
    if (diff < 0x400) {
        math_PrepareSegmentCollisionFrame(scr);
        /* FAKE: do-while(0) puts id in $s7 / obj in $fp as in the target;
         * unwrapped they swap (16 instructions differ)
         * (do-while-zero-exception) */
        do {
            rec = g_fighter_bone_hit_records[id];
        } while (0);
        for (i = 0; i < 22; i++, rec++) {
            Unk80107850Rec *pos;
            if (obj->unk_26C == 0 && i >= 6 && i <= 9) {
                continue;
            }
            pos = &SPAD->unkA8[id][i];
            if (math_TestPreparedSegmentSphere(
                    scr, pos, rec->unk_0C, rec->unk_0E) != 0) {
                s32 bit = 1 << i;
                *arg1 |= bit;
                if (rec->unk_00 != 0 &&
                    math_TestPreparedSegmentSphere(
                        scr, pos, rec->unk_10, rec->unk_12) != 0) {
                    *arg2 |= bit;
                }
            }
        }
    }
    scr->unkA8.x = scr->shape_points[0]->x - qx / 4;
    scr->unkA8.y = scr->shape_points[0]->y - qy / 4;
    scr->unkA8.z = scr->shape_points[0]->z - qz / 4;
    if (collision_TestSegmentWithWork(
            &scr->unkA8.x, &scr->shape_points[1]->x, &scr->unk100[0].x,
            &scr->unkF8.vx, (s32)work) != 0 &&
        collision_GetHitTag() != 7) {
        if (*arg1 != 0) {
            p = scr->shape_points[0];
            dx = scr->unk100[0].x - p->x;
            dy = scr->unk100[0].y - p->y;
            dz = scr->unk100[0].z - p->z;
            var_14 = dx * dx + dy * dy + dz * dz;
            dx = obj->world_pos.x - p->x;
            dy = obj->world_pos.y - p->y;
            dz = obj->world_pos.z - p->z;
            if (var_14 >= dx * dx + dy * dy + dz * dz) {
                goto done;
            }
            *arg2 = 0;
            *arg1 = 0;
        }
        if (arg3 == 0) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                id == 0, 0xA, &scr->unk100[0].x, 0);
        }
    }
done:
    D_800A37E8 = -qx;
    D_800A37EA = -qy;
    D_800A37EC = -qz;
}

extern void fighter_TestTriangleBoneHits(Unk80101EC8Record *a0, s32 a1, s32 a2);
extern s32 math_PrepareTriangleCollisionFrame(Unk1F8002B8Rec *obj);

void fighter_ProcessWeaponBodyHits(s32 mode) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    /* FAKE: second handle to D_800A37E8, so the address is rematerialized at
     * each of the three calls as in the target; spelled directly, cse keeps it
     * in a callee-saved register: score 6 (pointer-alias-fake-exception;
     * vector: Q96) */
    s16 *var_1 = &D_800A37E8;
    s32 i;

    for (i = 0; i < 2; i++) {
        Unk80101EC8Record *var_3;
        Unk80101EC8Record *var_4;
        u32 var_5;
        u32 var_6;
        u32 mask_a;
        u32 mask_b;
        u32 mask_c;
        s32 var_10;
        s32 var_11;
        /* var_12: the iteration's blade (0: the unk00 / unk_210 points, 1: the
         * unk48 / unk_234 points; the knockback reads the last iteration's),
         * then whether the nearest hit came from that blade. The 4/5 arm's
         * `var_12 = 0;` re-stores the held 0 (Q85; Ruling 11,
         * reused-local-necessity)
         */
        s32 var_12;
        /* temp1 / temp2: the indices of the iteration's two points (per-branch
         * constants, Q20), then the opponent's squared distance to the blade's
         * first / second point. The unk_8C arm re-stores the held values (Q85,
         * Ruling 11) */
        s32 temp1;
        s32 temp2;
        s32 var_15;
        /* idx: the triangle index of the segment loop, then the nearest
         * hit's index, the Q34 plain copy `idx = temp3;` (Ruling 11) */
        s32 idx;
        /* dx / dy / dz: the offset between two points, one value per use --
         * the segment's length vector, a hit point from the reference point,
         * the blade's points from the opponent, the blade direction, the scaled
         * push (dy: the first two only) (Ruling 11) */
        s32 dx;
        s32 dy;
        s32 dz;
        s32 best;
        /* temp3: the hit-slot counter of the nearest-hit search, then
         * whether the nearest hit came from the alternate blade (Ruling 11) */
        s32 temp3;
        s32 j;
        BoneHitRec *rec;
        s32 var_24;
        s32 var_25;
        s32 bit;
        s32 diff;
        s32 ang;
        /* work: the segment length squared, a hit's distance past its
         * radius, the facing difference to the opponent's velocity, then the
         * push weight (Ruling 11) */
        s32 work;
        s32 flag;
        s32 var_31;

        if (mode == 1 && g_fight_paired_move_initiator == i) {
            continue;
        }
        var_4 = &D_80101EC8[i];
        var_3 = D_80101EC8;
        if (i == 0) {
            var_3++;
        }
        mask_a = 0;
        mask_b = 0;
        mask_c = 0;
        var_10 = 0;
        var_5 = 0;
        var_6 = 0;
        if (var_3->is_defeated == 0 && var_3->weapon_live_state != 0 &&
            var_3->update_count != 0) {
            var_10 = var_3->character_roster_index != 0x1F;
        }
        if (var_3->wielded_object_live_state != 0) {
            var_10++;
        }
        if (var_10 != 0 && (var_3->weapon_id == 4 || var_3->weapon_id == 5)) {
            var_10++;
        }
        if ((var_3->weapon_id == 6 || var_3->weapon_id == 7) &&
            (var_3->move_kind == 2 || var_3->move_kind == 0x1B ||
             var_3->move_kind == 0x28 || var_3->move_kind == 0x26) &&
            var_3->move_frame == var_3->weapon_hit_start_frames[0]) {
            if (var_3->weapon_ammo != 0) {
                var_3->weapon_ammo -= 1;
                scr->point_slots.unk00[0] = SPAD->unk00[i == 0][1];
                scr->point_slots.unk00[1] = SPAD->unk00[i == 0][0];
                fighter_CastWeaponSegment(var_4, &var_5, &var_6, 0);
                mask_a |= var_5;
            } else {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    i == 0, 0x32,
                    i == 0 ? &SPAD->unk00[1][1].x : &SPAD->unk00[0][1].x, 0);
                var_10 = 0;
            }
        }
        if ((var_3->character_roster_index == 0x1D ||
             var_3->character_roster_index == 0xE) &&
            var_3->unk_26C != 0 &&
            (var_3->move_kind == 2 || var_3->move_kind == 0x1B ||
             var_3->move_kind == 0x28 || var_3->move_kind == 0x26) &&
            var_3->move_frame == var_3->weapon_hit_start_frames[1] &&
            var_3->weapon_ammo != 0) {
            var_3->weapon_ammo -= 1;
            scr->point_slots.unk00[0] = SPAD->unk48[i == 0][1];
            scr->point_slots.unk00[1] = SPAD->unk48[i == 0][0];
            fighter_CastWeaponSegment(
                var_4, &var_5, &var_6, var_3->character_roster_index == 0xE);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i == 0, 0x2A,
                i == 0 ? &SPAD->unk00[1][1].x : &SPAD->unk00[0][1].x, 0);
            mask_a |= var_5;
        }
        for (var_11 = 0; var_11 < var_10; var_11++) {
            /* var_32: fighter_TestTriangleBoneHits's third argument, per-branch
             * constants 1 / 1 / 0 (Q20); the unk_8C arm's store is shared with
             * the first iteration (Q85, Ruling 11) */
            s32 var_32;

            if (var_11 == 0) {
                var_12 = 0;
                temp1 = 0;
                temp2 = 1;
                var_32 = 1;
            } else if (var_3->wielded_object_live_state != 0) {
                var_12 = 1;
                temp1 = 0;
                temp2 = 1;
                var_32 = 1;
            } else if (var_3->weapon_id == 4 || var_3->weapon_id == 5) {
                var_12 = 0;
                temp1 = 1;
                temp2 = 2;
                var_32 = 0;
            }
            if (var_12 == 0) {
                scr->point_slots.unk00[0] = SPAD->unk00[i == 0][temp1];
                scr->point_slots.unk00[1] = SPAD->unk00[i == 0][temp2];
                scr->point_slots.unk00[2] = var_3->prev_weapon_points[temp1];
                scr->point_slots.unk00[3] = var_3->prev_weapon_points[temp2];
            } else {
                scr->point_slots.unk00[0] = SPAD->unk48[i == 0][temp1];
                scr->point_slots.unk00[1] = SPAD->unk48[i == 0][temp2];
                scr->point_slots.unk00[2] =
                    var_3->prev_second_weapon_points[temp1];
                scr->point_slots.unk00[3] =
                    var_3->prev_second_weapon_points[temp2];
            }
            scr->point_slots.unk00[4].x =
                (scr->point_slots.unk00[0].x + scr->point_slots.unk00[2].x) / 2;
            scr->point_slots.unk00[4].y =
                (scr->point_slots.unk00[0].y + scr->point_slots.unk00[2].y) / 2;
            scr->point_slots.unk00[4].z =
                (scr->point_slots.unk00[0].z + scr->point_slots.unk00[2].z) / 2;
            scr->point_slots.unk00[5].x =
                (scr->point_slots.unk00[1].x + scr->point_slots.unk00[3].x) / 2;
            scr->point_slots.unk00[5].y =
                (scr->point_slots.unk00[1].y + scr->point_slots.unk00[3].y) / 2;
            scr->point_slots.unk00[5].z =
                (scr->point_slots.unk00[1].z + scr->point_slots.unk00[3].z) / 2;
            dx = scr->point_slots.unk00[1].x - scr->point_slots.unk00[3].x;
            dy = scr->point_slots.unk00[1].y - scr->point_slots.unk00[3].y;
            dz = scr->point_slots.unk00[1].z - scr->point_slots.unk00[3].z;
            work = dx * dx + dy * dy + dz * dz;
            var_15 = 2;
            if (work > 6249999) {
                var_15 = 4;
            }
            for (idx = 0; idx < var_15; idx++) {
                if (var_15 == 2) {
                    if (idx == 0) {
                        scr->shape_points[0] = &scr->point_slots.unk00[2];
                        scr->shape_points[1] = &scr->point_slots.unk00[3];
                        scr->shape_points[2] = &scr->point_slots.unk00[1];
                    } else {
                        scr->shape_points[0] = &scr->point_slots.unk00[0];
                        scr->shape_points[1] = &scr->point_slots.unk00[1];
                        scr->shape_points[2] = &scr->point_slots.unk00[2];
                    }
                } else {
                    switch (idx) {
                    case 0:
                        scr->shape_points[0] = &scr->point_slots.unk00[3];
                        scr->shape_points[1] = &scr->point_slots.unk00[2];
                        scr->shape_points[2] = &scr->point_slots.unk00[5];
                        break;
                    case 1:
                        scr->shape_points[0] = &scr->point_slots.unk00[5];
                        scr->shape_points[1] = &scr->point_slots.unk00[4];
                        scr->shape_points[2] = &scr->point_slots.unk00[2];
                        break;
                    case 2:
                        scr->shape_points[0] = &scr->point_slots.unk00[5];
                        scr->shape_points[1] = &scr->point_slots.unk00[4];
                        scr->shape_points[2] = &scr->point_slots.unk00[1];
                        break;
                    case 3:
                        scr->shape_points[0] = &scr->point_slots.unk00[1];
                        scr->shape_points[1] = &scr->point_slots.unk00[0];
                        scr->shape_points[2] = &scr->point_slots.unk00[4];
                        break;
                    }
                }
                scr->bbox_max = *scr->shape_points[0];
                scr->unk78 = scr->bbox_max;
                for (j = 1; j < 3; j++) {
                    if (scr->shape_points[j]->x < scr->unk78.x) {
                        scr->unk78.x = scr->shape_points[j]->x;
                    } else if (scr->bbox_max.x < scr->shape_points[j]->x) {
                        scr->bbox_max.x = scr->shape_points[j]->x;
                    }
                    if (scr->shape_points[j]->y < scr->unk78.y) {
                        scr->unk78.y = scr->shape_points[j]->y;
                    } else if (scr->bbox_max.y < scr->shape_points[j]->y) {
                        scr->bbox_max.y = scr->shape_points[j]->y;
                    }
                    if (scr->shape_points[j]->z < scr->unk78.z) {
                        scr->unk78.z = scr->shape_points[j]->z;
                    } else if (scr->bbox_max.z < scr->shape_points[j]->z) {
                        scr->bbox_max.z = scr->shape_points[j]->z;
                    }
                }
                fighter_TestTriangleBoneHits(
                    var_4, math_PrepareTriangleCollisionFrame(scr), var_32);
                var_5 |= scr->bone_hit_mask;
                var_6 |= scr->unkC4;
                if (var_12 != 0) {
                    mask_a |= scr->bone_hit_mask;
                }
                if (var_11 == 1 && var_12 == 0) {
                    mask_b |= scr->bone_hit_mask;
                } else {
                    mask_c |= scr->bone_hit_mask;
                }
            }
        }
        if (var_5 == 0) {
            continue;
        }
        rec = g_fighter_bone_hit_records[i];
        best = 0x7FFFFFFF;
        scr->unkC8.x = var_3->prev_weapon_points[1].x;
        scr->unkC8.y = var_3->prev_weapon_points[1].y;
        scr->unkC8.z = var_3->prev_weapon_points[1].z;
        for (temp3 = 0; temp3 < 22; temp3++, rec++) {
            if (var_5 & (1 << temp3)) {
                dx = SPAD->unkA8[i][temp3].x - scr->unkC8.x;
                dy = SPAD->unkA8[i][temp3].y - scr->unkC8.y;
                dz = SPAD->unkA8[i][temp3].z - scr->unkC8.z;
                work = dx * dx + dy * dy + dz * dz - rec->unk_0E;
                if (work < best) {
                    best = work;
                    idx = temp3;
                }
            }
        }
        if (mode == 1) {
            math_NegNormalize3D(&var_3->weapon_point_delta[0].vx, var_1);
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                i, 4, &SPAD->unkA8[i][idx].x, var_1);
            return;
        }
        var_24 = 0;
        if ((var_3->move_frame >= var_3->weapon_hit_start_frames[0] - 3 &&
             var_3->move_frame < var_3->weapon_hit_start_frames[0]) ||
            (var_3->move_frame >= var_3->weapon_hit_start_frames[1] - 3 &&
             var_3->move_frame < var_3->weapon_hit_start_frames[1])) {
            var_24 = 1;
        }
        var_25 = 0;
        if ((var_3->weapon_id == 6 || var_3->weapon_id == 7 ||
             var_3->character_roster_index == 0x1D ||
             var_3->character_roster_index == 0xE) &&
            (mask_a & (1 << idx))) {
            var_25 = 1;
        }
        if (var_25 == 0 &&
            (!(var_3->move_kind == 2 || var_3->move_kind == 0x1B ||
               var_3->move_kind == 0x28 || var_3->move_kind == 0x26) ||
             var_24 == 0)) {
            bit = 1 << idx;
            temp3 = (mask_a & bit) != 0;
            if (temp3) {
                dx = SPAD->unk48[i == 0][0].x - var_3->world_pos.x;
                dz = SPAD->unk48[i == 0][0].z - var_3->world_pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - var_3->world_pos.x;
                dz = SPAD->unk48[i == 0][1].z - var_3->world_pos.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk48[i == 0][1].x - SPAD->unk48[i == 0][0].x;
                dz = SPAD->unk48[i == 0][1].z - SPAD->unk48[i == 0][0].z;
            } else if (mask_b & bit) {
                dx = SPAD->unk00[i == 0][2].x - var_3->world_pos.x;
                dz = SPAD->unk00[i == 0][2].z - var_3->world_pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - var_3->world_pos.x;
                dz = SPAD->unk00[i == 0][1].z - var_3->world_pos.z;
                temp2 = dx * dx + dz * dz;
                dx = (SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][2].x) / 4;
                dz = (SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][2].z) / 4;
            } else {
                dx = SPAD->unk00[i == 0][0].x - var_3->world_pos.x;
                dz = SPAD->unk00[i == 0][0].z - var_3->world_pos.z;
                temp1 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - var_3->world_pos.x;
                dz = SPAD->unk00[i == 0][1].z - var_3->world_pos.z;
                temp2 = dx * dx + dz * dz;
                dx = SPAD->unk00[i == 0][1].x - SPAD->unk00[i == 0][0].x;
                dz = SPAD->unk00[i == 0][1].z - SPAD->unk00[i == 0][0].z;
            }
            if (temp2 < temp1) {
                dx = -dx;
                dz = -dz;
            }
            diff = (var_3->opponent_bearing - ratan2(dx, dz)) & 0xFFF;
            if (diff >= 0x800) {
                diff = 0x1000 - diff;
            }
            if (diff < 0x400) {
                work = (ratan2(var_3->weapon_point_delta[var_12].vx,
                               var_3->weapon_point_delta[var_12].vz) +
                        0x800 - var_4->opponent_bearing) &
                       0xFFF;
                if (work >= 0x800) {
                    work = 0x1000 - work;
                }
                work = 0x400 - work;
                if (work < 0) {
                    work = 0;
                }
                dx = dx / 4 +
                     (var_3->weapon_point_delta[temp3].vx * work / 2 >> 11);
                dz = dz / 4 +
                     (var_3->weapon_point_delta[temp3].vz * work / 2 >> 11);
                if (var_3->move_kind == 3 || var_3->move_kind == 7 ||
                    var_3->move_kind == 0xD || var_3->move_kind == 0x2C) {
                    var_4->decaying_xz_velocity.vx += dx / 2;
                    var_4->decaying_xz_velocity.vz += dz / 2;
                } else {
                    var_4->decaying_xz_velocity.vx += dx;
                    var_4->decaying_xz_velocity.vz += dz;
                }
                if ((var_4->move_kind == 0x13 || var_4->move_kind == 0x1B ||
                     var_4->move_kind == 0x30) &&
                    var_3->move_kind == 0x15) {
                    if (var_4->weapon_id == 6 || var_4->weapon_id == 7 ||
                        var_3->weapon_id == 6 || var_3->weapon_id == 7) {
                        var_3->pending_move_event = 5;
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            i == 0, 0x21,
                            i == 0 ? &D_80101EC8[1].world_pos.x
                                   : &D_80101EC8[0].world_pos.x,
                            0);
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            i == 0, 0x2D,
                            i == 0 ? &D_80101EC8[1].world_pos.x
                                   : &D_80101EC8[0].world_pos.x,
                            0);
                    } else {
                        D_800A38A8 = 1;
                        D_800A3876 = i;
                    }
                }
            }
        }
        ang = (var_4->orientation.vy - var_3->orientation.vy) & 0xFFF;
        if (ang >= 0x800) {
            ang = 0x1000 - ang;
        }
        var_12 = (mask_a & (1 << idx)) != 0;
        if (var_3->unk_AE == 0) {
            continue;
        }
        if (var_3->wielded_object_live_state != 0 &&
            (var_3->move_frame < var_3->weapon_hit_start_frames[var_12] ||
             var_3->move_frame > var_3->weapon_hit_end_frames[var_12])) {
            continue;
        }
        flag = 0;
        if (!(var_3->weapon_id == 6 || var_3->weapon_id == 7)) {
            if ((var_3->character_roster_index == 0x1D ||
                 var_3->character_roster_index == 0xE) &&
                var_12 != 0) {
                flag = 1;
            } else {
                math_NegNormalize3D(
                    &var_3->weapon_point_delta[var_12].vx, var_1);
            }
        }
        if (D_800A3140 == 0) {
            var_31 = var_6 & (1 << idx);
        } else {
            var_31 = 1;
        }
        if (mask_b & (1 << idx) & ~mask_c) {
            var_31 = 0;
        }
        /* fighter_ResolveBoneHit's sixth argument is a record pointer on the
         * calls from object_ResolveFighterHits (first argument 1) and, on this
         * call (first argument 0), the 0/1 alternate-blade flag the target
         * passes in that slot: its `tbl == NULL ? 0xB : 0x19` picks the same
         * reaction fighter_ProcessWeaponContacts picks from +0x8C. */
        fighter_ResolveBoneHit(
            0, var_4, idx, ang, var_31, (Tbl8008E194 *)var_12, flag, 0);
        var_3->attack_hit_armed = 0;
    }
}

s32 fighter_SetBasePositionSeparationCorrectionsAndGetDistance(s32 arg0) {
    s32 temp_a3;
    s32 temp_t1;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    Unk80101EC8Record *rec0;
    Unk80101EC8Record *rec1;

    rec0 = &D_80101EC8[0];
    rec1 = &D_80101EC8[1];
    temp_a3 = rec0->base_pos.x - rec1->base_pos.x;
    temp_t1 = rec0->base_pos.z - rec1->base_pos.z;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32)(g_sqrt_table_u8[temp_a0])) >> 3;
    } else {
        s32 sp_tmp;
        gte_Lzc(temp_a0, &sp_tmp);
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
        }
    }
    if (((s32)var_t0) < arg0) {
        var_a0 = ((arg0 - ((s32)var_t0)) * 0x50) / 100;
    } else {
        var_a0 = (arg0 - ((s32)var_t0)) / 16;
    }
    {
        s32 temp_v0 = arg0 - 0x64;
        s32 temp_v1_3 = -var_a0;
        rec0->decaying_xz_velocity.vx = (temp_a3 * var_a0) / temp_v0;
        rec0->decaying_xz_velocity.vz = (temp_t1 * var_a0) / temp_v0;
        rec1->decaying_xz_velocity.vx = (temp_a3 * temp_v1_3) / temp_v0;
        rec1->decaying_xz_velocity.vz = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32)var_t0;
}

s32 fighter_CalcSeparationErrorAndOpposedOffsets(void) {
    s32 temp_a3;
    s32 temp_t1;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    Unk80101EC8Record *rec0;
    Unk80101EC8Record *rec1;

    rec0 = &D_80101EC8[0];
    rec1 = &D_80101EC8[1];
    temp_a3 = rec0->world_pos.x - rec1->world_pos.x;
    temp_t1 = rec0->world_pos.z - rec1->world_pos.z;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32)(g_sqrt_table_u8[temp_a0])) >> 3;
    } else {
        s32 sp_tmp;
        gte_Lzc(temp_a0, &sp_tmp);
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(g_sqrt_table_u8[idx]));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
        }
    }
    if (((s32)var_t0) < 0x44C) {
        var_a0 = ((0x44C - ((s32)var_t0)) * 0x50) / 100;
    } else {
        var_a0 = (0x44C - ((s32)var_t0)) / 16;
    }
    {
        s32 temp_v0 = 0x3E8;
        s32 temp_v1_3 = -var_a0;
        rec0->decaying_xz_velocity.vx = (temp_a3 * var_a0) / temp_v0;
        rec0->decaying_xz_velocity.vz = (temp_t1 * var_a0) / temp_v0;
        rec1->decaying_xz_velocity.vx = (temp_a3 * temp_v1_3) / temp_v0;
        rec1->decaying_xz_velocity.vz = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32)var_t0 - 0x44C;
}

void fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents(void) {
    s32 i;
    Unk80101EC8Record *var_s0;
    s32 temp_s2;

    temp_s2 =
        fighter_SetBasePositionSeparationCorrectionsAndGetDistance(D_800A371C);

    for (i = 0; i < 2; i++) {
        Unk80101EC8Record *e = &D_80101EC8[i];
        Unk80101EC8Record *ptr = e->other;
        s32 arg1 = ptr->base_pos.x - e->base_pos.x;
        s32 arg2 = ptr->base_pos.z - e->base_pos.z;
        fighter_SetTurnDelta(e, ratan2(arg1, arg2));
    }

    {
        s32 idx;
        s32 chk;
        idx = g_fight_paired_move_initiator;
        chk = g_fight_paired_move_start_pending;
        var_s0 = &D_80101EC8[idx];

        if (chk == 0) {
            if (g_fight_missing_support_fighter_index == 0xFF) {
                if (var_s0->paired_move_hit_frame == var_s0->move_frame) {
                    fighter_ProcessWeaponBodyHits(1);
                }
            }
        }

        {
            s32 v1;
            v1 = var_s0->move_frame;
            if (v1 < (s32)D_800A38E8) {
                return;
            }
            if (v1 >= var_s0->paired_move_hit_frame) {
                return;
            }
            if (D_800A371C + 0xC8 >= temp_s2) {
                return;
            }
            {
                Unk80101EC8Record *v1ptr;
                v1ptr = var_s0->other;
                var_s0->pending_move_event = 4;
                v1ptr->pending_move_event = 5;
            }
        }
    }
}

/* Accumulates a character's shadow vectors in the scratchpad record at
 * 0x1F8002B8 (`scr`): unkA8 sums two of the character's SPAD points (unk48[k]
 * when bit k of g_fight_weapon_contact_result is set, else unk00[k]); unkB8
 * sums the matching pair fight_UpdateFighterInteractionsAndPointCaches copied
 * into D_80101EC8[k] (unk_234 / unk_210); the result goes to contact_point. */
void fighter_BlendCurrentAndPreviousContactPoints(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    Unk80101EC8Record *rec0 = &D_80101EC8[0];
    Unk80101EC8Record *rec1 = &D_80101EC8[1];

    scr->unkA8.x = 0;
    scr->unkA8.y = 0;
    scr->unkA8.z = 0;
    scr->unkB8.x = 0;
    scr->unkB8.y = 0;
    scr->unkB8.z = 0;

    if (g_fight_weapon_contact_result & 1) {
        scr->unkA8.x = SPAD->unk48[0][0].x;
        scr->unkA8.y = SPAD->unk48[0][0].y;
        scr->unkA8.z = SPAD->unk48[0][0].z;
        scr->unkA8.x += SPAD->unk48[0][1].x;
        scr->unkA8.y += SPAD->unk48[0][1].y;
        scr->unkA8.z += SPAD->unk48[0][1].z;
        scr->unkB8.x = rec0->prev_second_weapon_points[0].x;
        scr->unkB8.y = rec0->prev_second_weapon_points[0].y;
        scr->unkB8.z = rec0->prev_second_weapon_points[0].z;
        scr->unkB8.x += rec0->prev_second_weapon_points[1].x;
        scr->unkB8.y += rec0->prev_second_weapon_points[1].y;
        scr->unkB8.z += rec0->prev_second_weapon_points[1].z;
    } else {
        scr->unkA8.x = SPAD->unk00[0][0].x;
        scr->unkA8.y = SPAD->unk00[0][0].y;
        scr->unkA8.z = SPAD->unk00[0][0].z;
        scr->unkA8.x += SPAD->unk00[0][1].x;
        scr->unkA8.y += SPAD->unk00[0][1].y;
        scr->unkA8.z += SPAD->unk00[0][1].z;
        scr->unkB8.x = rec0->prev_weapon_points[0].x;
        scr->unkB8.y = rec0->prev_weapon_points[0].y;
        scr->unkB8.z = rec0->prev_weapon_points[0].z;
        scr->unkB8.x += rec0->prev_weapon_points[1].x;
        scr->unkB8.y += rec0->prev_weapon_points[1].y;
        scr->unkB8.z += rec0->prev_weapon_points[1].z;
    }
    if (g_fight_weapon_contact_result & 2) {
        scr->unkA8.x += SPAD->unk48[1][0].x;
        scr->unkA8.y += SPAD->unk48[1][0].y;
        scr->unkA8.z += SPAD->unk48[1][0].z;
        scr->unkB8.x += rec1->prev_second_weapon_points[0].x;
        scr->unkB8.y += rec1->prev_second_weapon_points[0].y;
        scr->unkB8.z += rec1->prev_second_weapon_points[0].z;
        scr->unkA8.x += SPAD->unk48[1][1].x;
        scr->unkA8.y += SPAD->unk48[1][1].y;
        scr->unkA8.z += SPAD->unk48[1][1].z;
        scr->unkB8.x += rec1->prev_second_weapon_points[1].x;
        scr->unkB8.y += rec1->prev_second_weapon_points[1].y;
        scr->unkB8.z += rec1->prev_second_weapon_points[1].z;
    } else {
        scr->unkA8.x += SPAD->unk00[1][0].x;
        scr->unkA8.y += SPAD->unk00[1][0].y;
        scr->unkA8.z += SPAD->unk00[1][0].z;
        scr->unkB8.x += rec1->prev_weapon_points[0].x;
        scr->unkB8.y += rec1->prev_weapon_points[0].y;
        scr->unkB8.z += rec1->prev_weapon_points[0].z;
        scr->unkA8.x += SPAD->unk00[1][1].x;
        scr->unkA8.y += SPAD->unk00[1][1].y;
        scr->unkA8.z += SPAD->unk00[1][1].z;
        scr->unkB8.x += rec1->prev_weapon_points[1].x;
        scr->unkB8.y += rec1->prev_weapon_points[1].y;
        scr->unkB8.z += rec1->prev_weapon_points[1].z;
    }
    scr->contact_point.x = ((scr->unkA8.x * 3) + scr->unkB8.x) >> 4;
    scr->contact_point.y = ((scr->unkA8.y * 3) + scr->unkB8.y) >> 4;
    scr->contact_point.z = ((scr->unkA8.z * 3) + scr->unkB8.z) >> 4;
}

/* Per-frame update of the two D_80101EC8 records.  The three
 * scratchpad points per character at SPAD->unk00 and the two at SPAD->unk48
 * are copied into each record's unk_210 / unk_234
 * (fighter_BlendCurrentAndPreviousContactPoints reads them back); unk_18C is
 * the centroid of SPAD->unkA8[k][1..3] and unk_174 the midpoint of
 * SPAD->unkA8[k][4..5]. */
void fight_UpdateFighterInteractionsAndPointCaches(void) {
    /* Pointer read-modify-write on a global (pointer-alias-fake-exception):
     * both players' pending_move_event values are tested before the clash
     * checks select event 0xA; contact resolution also reads and clears
     * unk_AD. */
    Unk80101EC8Record *firstPlayer = &D_80101EC8[0];
    Unk80101EC8Record *secondPlayer = &D_80101EC8[1];
    s32 i;
    u16 mode;

    mode = D_80101EC8[0].move_kind;

    if (mode == 0xF || mode == 0x1C || mode == 0x1D || mode == 0x1E ||
        mode == 0x1F || mode == 0x20 || mode == 0x21) {
        fight_UpdatePairedInputTotalsEventsAndMidpointEffects();
    } else if (mode == 0x11) {
        fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents();
    } else {
        fighter_TriggerOpponentResponseOnMoveFrame();
        fighter_ResolveBodyOverlap();
        g_fight_weapon_contact_result = fighter_ProcessWeaponContacts();
        if (g_fight_weapon_contact_result >= 0) {
            fighter_BlendCurrentAndPreviousContactPoints();
        }
        if (g_fight_weapon_contact_result >= 0 &&
            (firstPlayer->attack_hit_armed != 0 ||
             secondPlayer->attack_hit_armed != 0)) {
            fighter_ResolveWeaponContact(
                firstPlayer, &SPAD->unk2B8.rec.contact_point.x);
            fighter_ResolveWeaponContact(
                secondPlayer, &SPAD->unk2B8.rec.contact_point.x);
            secondPlayer->attack_hit_armed = 0;
            firstPlayer->attack_hit_armed = 0;
        } else {
            fighter_ProcessWeaponBodyHits(0);
        }
        if (firstPlayer->update_count >= 3 && secondPlayer->update_count >= 3 &&
            D_800A38A8 != 0 && firstPlayer->pending_move_event == -1 &&
            secondPlayer->pending_move_event == -1 &&
            firstPlayer->character_roster_index != 0x1F &&
            secondPlayer->character_roster_index != 0x1F) {
            s32 diff = firstPlayer->world_pos.y - secondPlayer->world_pos.y;
            if (diff < 0)
                diff = -diff;
            if (diff < 0x3E8) {
                firstPlayer->pending_move_event = 0xA;
                secondPlayer->pending_move_event = 0xA;
                secondPlayer->mash_contest_score = 0;
                firstPlayer->mash_contest_score = 0;
                g_fight_midpoint_effect_delay = 0;
                D_800A389C = 0;
            }
        }
    }

    if (D_80101EC8[0].move_kind == 5) {
        g_fight_followup_fighter_index = 1;
        g_main_loop_handler_index = 0x1C;
    } else if (D_80101EC8[1].move_kind == 5) {
        g_fight_followup_fighter_index = 0;
        g_main_loop_handler_index = 0x1C;
    }

    for (i = 0; i < 3; i++) {
        D_80101EC8[0].prev_weapon_points[i] = SPAD->unk00[0][i];
        D_80101EC8[1].prev_weapon_points[i] = SPAD->unk00[1][i];
    }

    for (i = 0; i < 2; i++) {
        D_80101EC8[0].prev_second_weapon_points[i] = SPAD->unk48[0][i];
        D_80101EC8[1].prev_second_weapon_points[i] = SPAD->unk48[1][i];
    }

    for (i = 0; i < 2; i++) {
        D_80101EC8[i].bone_hit_points_1_3_centroid.x =
            (SPAD->unkA8[i][1].x + SPAD->unkA8[i][2].x + SPAD->unkA8[i][3].x) /
            3;
        D_80101EC8[i].bone_hit_points_1_3_centroid.y =
            (SPAD->unkA8[i][1].y + SPAD->unkA8[i][2].y + SPAD->unkA8[i][3].y) /
            3;
        D_80101EC8[i].bone_hit_points_1_3_centroid.z =
            (SPAD->unkA8[i][1].z + SPAD->unkA8[i][2].z + SPAD->unkA8[i][3].z) /
            3;
        D_80101EC8[i].bone_hit_points_4_5_midpoint.x =
            (SPAD->unkA8[i][4].x + SPAD->unkA8[i][5].x) / 2;
        D_80101EC8[i].bone_hit_points_4_5_midpoint.y =
            (SPAD->unkA8[i][4].y + SPAD->unkA8[i][5].y) / 2;
        D_80101EC8[i].bone_hit_points_4_5_midpoint.z =
            (SPAD->unkA8[i][4].z + SPAD->unkA8[i][5].z) / 2;
    }

    {
        s16 saved = firstPlayer->pending_move_event;
        if (saved == -1) {
            object_ResolveFighterHits();
            if (firstPlayer->pending_move_event == saved) {
                fight_TriggerOpposingFighterEventFromNearBallisticRecords();
            }
        }
    }
}

extern s32 math_TestSphereAgainstPreparedAxialVertexSpan(
    s32 flag, Unk1F8002B8Rec *obj, Unk80107850Rec *pos, s32 threshold,
    s32 r_sq);
extern s32 math_TestPreparedTriangleSphere(
    s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq);

void fighter_TestTriangleBoneHits(Unk80101EC8Record *a0, s32 a1, s32 a2) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 id = a0->index;
    BoneHitRec *recbase = g_fighter_bone_hit_records[id];
    BoneHitRec *rec;
    s32 var_4 = 0;
    s32 seenMask = 0;
    s32 i;

    for (i = 0, rec = recbase; i < 0x16; i++, rec++) {
        s32 x;
        s32 y;
        s32 z;
        s32 r;
        s32 var_11;

        if (a0->unk_26C == 0 && i >= 6 && i <= 9) {
            continue;
        }

        r = rec->unk_0C;
        /* FAKE: the AABB reject flag is staged through `var_11` (set on
         * reject, read by the `continue` test, then overwritten by the call); a
         * separate flag takes $a1 instead of the target's $s0 (score 3) */
        var_11 = 0;
        x = SPAD->unkA8[id][i].x;
        if (scr->bbox_max.x < x - r || x + r < scr->unk78.x) {
            var_11 = 1;
        } else {
            y = SPAD->unkA8[id][i].y;
            if (scr->bbox_max.y < y - r || y + r < scr->unk78.y) {
                var_11 = 1;
            } else {
                z = SPAD->unkA8[id][i].z;
                if (scr->bbox_max.z < z - r || z + r < scr->unk78.z) {
                    var_11 = 1;
                }
            }
        }
        if (var_11 != 0) {
            continue;
        }

        if (a1 != 0) {
            var_11 = math_TestPreparedTriangleSphere(
                0, scr, (s32 *)&SPAD->unkA8[id][i], r, rec->unk_0E);
            if (var_11 != 0) {
                if (rec->unk_00 != 0 && a2 != 0) {
                    if (math_TestPreparedTriangleSphere(
                            1, scr, (s32 *)0, rec->unk_10, rec->unk_12) != 0) {
                        var_4 |= 1 << i;
                    }
                }
            }
        } else {
            var_11 = math_TestSphereAgainstPreparedAxialVertexSpan(
                0, scr, &SPAD->unkA8[id][i], r, rec->unk_0E);
            if (var_11 != 0) {
                if (rec->unk_00 != 0 && a2 != 0) {
                    if (math_TestSphereAgainstPreparedAxialVertexSpan(
                            1, scr, NULL, rec->unk_10, rec->unk_12) != 0) {
                        var_4 |= 1 << i;
                    }
                }
            }
        }
        if (var_11 != 0) {
            seenMask |= 1 << i;
        }
    }

    scr->bone_hit_mask = seenMask;
    scr->unkC4 = var_4;
}

/* Orients a triangle's local frame. The vertices obj->shape_points[0..2] give
 * edge vectors a = v1 - v0 (unkA8) and b = v2 - v0 (unkB8) and n = a x b
 * (unkC8). If n's components are within +-0x3FFF and |n| < 0x4000, the yaw
 * (unkF8.vy) and pitch (.vx) come from a and it returns 0; otherwise n is
 * scaled down by 64 and the angles come from n (returns 1). A rotation matrix
 * built in rot_matrix (RotMatrixY, RotMatrixX) then rotates a and b in place.
 */
s32 math_PrepareTriangleCollisionFrame(Unk1F8002B8Rec *obj) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 sp_tmp3;
    s32 len_sq;
    s32 xz_sq;
    /* FAKE: temp holds two values: n.x*n.x + n.z*n.z, then the square-root
     * table byte; one local per value: score 6 (Ruling 11, Q129) */
    s32 temp;
    s32 nxz_len;

    obj->unkA8.x = obj->shape_points[1]->x - obj->shape_points[0]->x;
    obj->unkA8.y = obj->shape_points[1]->y - obj->shape_points[0]->y;
    obj->unkA8.z = obj->shape_points[1]->z - obj->shape_points[0]->z;
    obj->unkB8.x = obj->shape_points[2]->x - obj->shape_points[0]->x;
    obj->unkB8.y = obj->shape_points[2]->y - obj->shape_points[0]->y;
    obj->unkB8.z = obj->shape_points[2]->z - obj->shape_points[0]->z;

    gte_OuterProduct0(&obj->unkA8, &obj->unkB8, &obj->unkC8);

    if ((u32)(obj->unkC8.x + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.y + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.z + 0x3FFF) < 0x7FFF) {
        /* Successive vector lengths, |n| for the guard then |a.xz| for
         * the pitch, each consumed before the next length is written (Q51). */
        /* SOTN: src/dra/4B758.c:71 @db41b28 */
        s32 len;
        gte_sqr0();
        gte_stlvnl(&obj->unk100[0]);
        len_sq = obj->unk100[0].x + obj->unk100[0].y + obj->unk100[0].z;
        if ((u32)len_sq < 0x400) {
            len = (u32)g_sqrt_table_u8[len_sq] >> 3;
        } else {
            s32 lzcr = 0;
            if (len_sq >= 0) {
                gte_Lzc(len_sq, &sp_tmp);
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                len = (u32)(g_sqrt_table_u8[(u32)len_sq >> shift] << 16) >>
                      (0x13 - ((u32)shift >> 1));
            }
        }
        if ((u32)len < 0x4000) {
            obj->unkF8.vy = 0x800 - ratan2(obj->unkA8.x, obj->unkA8.z);
            xz_sq = obj->unkA8.x * obj->unkA8.x + obj->unkA8.z * obj->unkA8.z;
            if ((u32)xz_sq < 0x400) {
                len = (u32)g_sqrt_table_u8[xz_sq] >> 3;
            } else {
                s32 lzcr = 0;
                if (xz_sq >= 0) {
                    gte_Lzc(xz_sq, &sp_tmp2);
                    lzcr = sp_tmp2;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    len = (u32)(g_sqrt_table_u8[(u32)xz_sq >> shift] << 16) >>
                          (0x13 - ((u32)shift >> 1));
                }
            }
            obj->unkF8.vx = 0x800 - ratan2(obj->unkA8.y, len);
            obj->rot_matrix.m[0][0] = 0x1000;
            obj->rot_matrix.m[0][1] = 0;
            obj->rot_matrix.m[0][2] = 0;
            obj->rot_matrix.m[1][0] = 0;
            obj->rot_matrix.m[1][1] = 0x1000;
            obj->rot_matrix.m[1][2] = 0;
            obj->rot_matrix.m[2][0] = 0;
            obj->rot_matrix.m[2][1] = 0;
            obj->rot_matrix.m[2][2] = 0x1000;
            RotMatrixY(obj->unkF8.vy, &obj->rot_matrix);
            RotMatrixX(obj->unkF8.vx, &obj->rot_matrix);
            gte_SetRotMatrix(&obj->rot_matrix);
            gte_ldlv0(&obj->unkA8);
            gte_rtv0();
            gte_stlvnl(&obj->unkA8);
            gte_ldlv0(&obj->unkB8);
            gte_rtv0();
            gte_stlvnl(&obj->unkB8);
            return 0;
        }
    }

    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);
    obj->unkC8.x >>= 6;
    obj->unkC8.y >>= 6;
    obj->unkC8.z >>= 6;
    temp = obj->unkC8.x * obj->unkC8.x + obj->unkC8.z * obj->unkC8.z;
    if ((u32)temp < 0x400) {
        nxz_len = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            gte_Lzc(temp, &sp_tmp3);
            lzcr = sp_tmp3;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            nxz_len = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, nxz_len);
    obj->rot_matrix.m[0][0] = 0x1000;
    obj->rot_matrix.m[0][1] = 0;
    obj->rot_matrix.m[0][2] = 0;
    obj->rot_matrix.m[1][0] = 0;
    obj->rot_matrix.m[1][1] = 0x1000;
    obj->rot_matrix.m[1][2] = 0;
    obj->rot_matrix.m[2][0] = 0;
    obj->rot_matrix.m[2][1] = 0;
    obj->rot_matrix.m[2][2] = 0x1000;
    RotMatrixY(obj->unkF8.vy, &obj->rot_matrix);
    RotMatrixX(obj->unkF8.vx, &obj->rot_matrix);
    gte_SetRotMatrix(&obj->rot_matrix);
    gte_ldlv0(&obj->unkA8);
    gte_rtv0();
    gte_stlvnl(&obj->unkA8);
    gte_ldlv0(&obj->unkB8);
    gte_rtv0();
    gte_stlvnl(&obj->unkB8);
    return 1;
}

s32 math_TestSphereAgainstPreparedAxialVertexSpan(
    s32 flag, Unk1F8002B8Rec *obj, Unk80107850Rec *pos, s32 threshold,
    s32 r_sq) {
    if (flag == 0) {
        obj->unkF8.vx = pos->x - obj->shape_points[0]->x;
        obj->unkF8.vy = pos->y - obj->shape_points[0]->y;
        obj->unkF8.vz = pos->z - obj->shape_points[0]->z;
        gte_ApplyRotMatrix(&obj->unkF8, &obj->unk100[0]);
    }
    {
        s32 x;
        s32 y;
        s32 sp_var;
        s32 min_z;
        s32 max_z;
        s32 az;
        s32 bz;
        s32 z;
        s32 neg_threshold = -threshold;
        s32 sq;
        /* r_sq - sq, then its square root in place (the sphere's half-chord
         * along z), as SOTN's cloaked knight takes
         * `distance = SquareRoot0(distance)` (Q51) */
        /* SOTN: src/st/nz1/e_cloaked_knight.c:83 @db41b28 */
        s32 rad;

        x = obj->unk100[0].x;
        if (x < neg_threshold || threshold < x)
            return 0;
        y = obj->unk100[0].y;
        if (y < neg_threshold || threshold < y)
            return 0;

        sq = x * x + y * y;
        if (r_sq < sq)
            return 0;
        rad = r_sq - sq;

        if ((u32)rad < 0x400) {
            rad = (u32)g_sqrt_table_u8[rad] >> 3;
        } else {
            s32 lzcr = 0;
            if (rad >= 0) {
                gte_Lzc(rad, &sp_var);
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)rad >> shift];
                rad = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_z = 0;
        min_z = 0;
        az = obj->unkA8.z;
        if (az < 0) {
            min_z = az;
        } else if (min_z < az) {
            max_z = az;
        }
        bz = obj->unkB8.z;
        if (bz < min_z) {
            min_z = bz;
        } else if (max_z < bz) {
            max_z = bz;
        }
        z = obj->unk100[0].z;
        if (max_z < z - rad || z + rad < min_z)
            return 0;
        return 1;
    }
}

/* Segment/circle test in x/z. Returns 0 when p1 and p2 lie on the same side
 * outside the +-threshold band in x or in z; otherwise solves the quadratic for
 * where p1 -> p2 meets the circle of squared radius r_sq about the origin
 * (terms >> 9; square root by g_sqrt_table_u8, GTE leading-zero count for
 * values >= 0x400) and returns whether both Q8 intersection parameters
 * are in range. Load-bearing: `var_22 = var_18; lzcr = 0;` in that order,
 * `var_18` holding the value under the root, the square root and the <<9
 * result, `num1` before `denom`, and `result` kept apart from the comparison
 * flag. The `var_22` re-store is a FAKE, explained at its site. */
s32 math_TestSegmentCircle2D(s32 threshold, s32 r_sq, s32 *p1, s32 *p2) {
    s32 x1, z1, x2, z2;

    x1 = p1[0];
    x2 = p2[0];

    if (x1 < x2) {
        if (x2 < -threshold)
            return 0;
        if (threshold >= x1)
            goto z_check;
        return 0;
    } else {
        if (x1 < -threshold)
            return 0;
        if (threshold < x2)
            return 0;
    }

z_check:
    z1 = p1[1];
    z2 = p2[1];

    if (z1 < z2) {
        if (z2 < -threshold)
            return 0;
        if (threshold >= z1)
            goto dist_calc;
        return 0;
    } else {
        if (z1 < -threshold)
            return 0;
        if (threshold < z2)
            return 0;
    }

dist_calc: {
    s32 ax = p2[0] - p1[0];
    s32 x1r = p1[0];
    s32 az = p2[1] - p1[1];
    s32 z1r = p1[1];

    s32 ax_sq = ax * ax;
    s32 az_sq = az * az;
    s32 cx = ax * x1r;
    s32 cz = az * z1r;
    s32 x1_sq = x1r * x1r;
    s32 z1_sq = z1r * z1r;

    s32 dot2 = (cx + cz) * 2;
    s32 dot2_9 = dot2 >> 9;
    s32 c_val = ((x1_sq + z1_sq) - r_sq) >> 9;
    s32 dist_sq = ax_sq + az_sq;

    s32 var_18 = (dot2_9 * dot2_9) - ((dist_sq >> 9) * (c_val << 2));
    s32 result;

    if (var_18 < 0) {
        result = 0;
    } else {
        if ((u32)var_18 < 0x400u) {
            var_18 = g_sqrt_table_u8[var_18] >> 3;
        } else {
            s32 sp_tmp;
            s32 lzcr;
            u32 var_22;
            var_22 = var_18;
            lzcr = 0;
            if (var_18 >= 0) {
                /* FAKE: same-value re-store of `var_22` (it already holds
                 * var_18): with a single def cse merges var_22 into var_18 and
                 * deletes the copy the target keeps in $a0; the store emits
                 * nothing; without it score 4 */
                var_22 = var_18;
                gte_Lzc(var_22, &sp_tmp);
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tval = g_sqrt_table_u8[var_22 >> shift];
                s32 half = (u32)shift >> 1;
                var_18 = (u32)(tval << 16) >> (0x13 - half);
            }
        }
        {
            s32 neg_b = -dot2;
            s32 num1;
            s32 denom;
            s32 t1_val;
            s32 t2_val;
            var_18 <<= 9;
            num1 = (neg_b + var_18) << 8;
            denom = dist_sq * 2;
            t1_val = num1 / denom;
            t2_val = ((neg_b - var_18) << 8) / denom;

            {
                s32 flag = 0;
                if (t1_val >= 0) {
                    flag = t2_val < 0x101;
                }
                result = flag;
            }
        }
    }
    return result;
}
}

s32 math_TestPreparedTriangleSphere(
    s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        obj->unkF8.vx = pos[0] - obj->shape_points[0]->x;
        obj->unkF8.vy = pos[1] - obj->shape_points[0]->y;
        obj->unkF8.vz = pos[2] - obj->shape_points[0]->z;
        gte_ApplyRotMatrix(&obj->unkF8, &obj->unk100[0]);
    }

    {
        s32 y = obj->unk100[0].z;
        if (y < -threshold || threshold < y)
            return 0;
    }

    {
        s32 x0 = obj->unkA8.x;
        s32 x2 = obj->unkB8.x;
        s32 z0 = obj->unkA8.y;
        s32 z2 = obj->unkB8.y;
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = obj->unk100[0].x;
        s32 pz = obj->unk100[0].y;
        /* FAKE: var_9 / var_10 hold one value per side test,
         * against cx / cz and px / pz, for the edges (0,0)-(x0,z0),
         * (0,0)-(x2,z2), (x0,z0)-(x2,z2); a pair per test: score 36 (Ruling 11)
         */
        s32 var_9 = z0 * cx - x0 * cz;
        s32 var_10 = z0 * px - x0 * pz;

        if ((var_9 ^ var_10) >= 0) {
            var_9 = z2 * cx - x2 * cz;
            var_10 = z2 * px - x2 * pz;
            if ((var_9 ^ var_10) >= 0) {
                /* FAKE: ax holds cx - x0 ahead of the dx set-up; written in
                 * the product the subtraction moves (score 2) (Q129). */
                s32 ax = cx - x0;
                s32 dx;
                /* FAKE: the third edge's z2 - z0 is staged through the finished
                 * `flag` parameter (unread after the mode test); a block-local
                 * dz takes dx's $v1: score 9 */
                flag = z2 - z0;
                dx = x2 - x0;
                var_9 = (flag * ax) - (dx * (cz - z0));
                var_10 = (flag * (px - x0)) - (dx * (pz - z0));
                if ((var_9 ^ var_10) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = obj->unk100[0].z;
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        Vec3i32 *p118;
        Vec3i32 *p124;
        Vec3i32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)g_sqrt_table_u8[dist] >> 3;
        } else {
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                /* FAKE: same-value re-store of `m`: breaks the m == dist
                 * equivalence across the skipped arm so the shift below reads
                 * the $a0 copy, not dist ($s1), as in math_TestSegmentCircle2D;
                 * without it: score 4 */
                m = dist;
                gte_Lzc(m, &sp_var);
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                sqrt_val = (u32)(g_sqrt_table_u8[(u32)m >> shift] << 16) >>
                           (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = &obj->unk118[0];
        p124 = &obj->unk118[1];
        p118->x = obj->unkA8.x - obj->unk100[0].x;
        p118->y = obj->unkA8.y - obj->unk100[0].y;
        p124->x = obj->unkB8.x - obj->unk100[0].x;
        p124->y = obj->unkB8.y - obj->unk100[0].y;
        if (math_TestSegmentCircle2D(
                sqrt_val, dist, (s32 *)p118, (s32 *)p124) != 0)
            return 1;

        p10C = &obj->unk100[1];
        p10C->x = -obj->unk100[0].x;
        p10C->y = -obj->unk100[0].y;
        if (math_TestSegmentCircle2D(
                sqrt_val, dist, (s32 *)p10C, (s32 *)p118) != 0)
            return 1;

        if (math_TestSegmentCircle2D(
                sqrt_val, dist, (s32 *)p10C, (s32 *)p124) != 0)
            return 1;
        return 0;
    }
}

/* math_PrepareTriangleCollisionFrame's frame, built from the normal: edge
 * vectors a = v1 - v0 (unkA8), b = v2 - v0 (unkB8) and n = a x b (unkC8). It
 * returns 0 while n's components are within +-0x3FFF; otherwise n is scaled
 * down by 64, the yaw (unkF8.vy) and pitch (.vx) come from n, and the rotation
 * built in rot_matrix (RotMatrixY, RotMatrixX) rotates a and b in place
 * (returns 1).
 */
s32 math_TryPrepareTriangleNormalFrame(Unk1F8002B8Rec *obj) {
    MATRIX *mat;
    s32 sp_tmp;
    s32 dist_sq;
    s32 dist;

    obj->unkA8.x = obj->shape_points[1]->x - obj->shape_points[0]->x;
    obj->unkA8.y = obj->shape_points[1]->y - obj->shape_points[0]->y;
    obj->unkA8.z = obj->shape_points[1]->z - obj->shape_points[0]->z;

    obj->unkB8.x = obj->shape_points[2]->x - obj->shape_points[0]->x;
    obj->unkB8.y = obj->shape_points[2]->y - obj->shape_points[0]->y;
    obj->unkB8.z = obj->shape_points[2]->z - obj->shape_points[0]->z;

    gte_OuterProduct0(&obj->unkA8, &obj->unkB8, &obj->unkC8);

    if ((u32)(obj->unkC8.x + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.y + 0x3FFF) < 0x7FFF &&
        (u32)(obj->unkC8.z + 0x3FFF) < 0x7FFF) {
        return 0;
    }

    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);
    obj->unkC8.x = obj->unkC8.x >> 6;
    obj->unkC8.y = obj->unkC8.y >> 6;
    /* FAKE: the scaled z delta is staged through `dist` (dead until the
     * if/else below writes the xz length into it); a block-local z temp
     * moves dist_sq from the target's $a0 to $a1: score 6 (Q129) */
    /* SOTN: src/st/no1/unk_39184.c:420 @db41b28 */
    dist = obj->unkC8.z;
    dist >>= 6;
    dist_sq = obj->unkC8.x * obj->unkC8.x + dist * dist;
    obj->unkC8.z = dist;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            gte_Lzc(dist_sq, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            dist = (u32)(g_sqrt_table_u8[(u32)dist_sq >> shift] << 16) >>
                   (0x13 - ((u32)shift >> 1));
        }
    }

    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, dist);
    mat = &obj->rot_matrix;

    /* identity 3x3 rotation matrix obj->rot_matrix */
    obj->rot_matrix.m[0][0] = 0x1000;
    obj->rot_matrix.m[0][1] = 0;
    obj->rot_matrix.m[0][2] = 0;
    obj->rot_matrix.m[1][0] = 0;
    obj->rot_matrix.m[1][1] = 0x1000;
    obj->rot_matrix.m[1][2] = 0;
    obj->rot_matrix.m[2][0] = 0;
    obj->rot_matrix.m[2][1] = 0;
    obj->rot_matrix.m[2][2] = 0x1000;
    RotMatrixY(obj->unkF8.vy, mat);
    RotMatrixX(obj->unkF8.vx, mat);

    gte_SetRotMatrix(mat);
    gte_ldlv0(&obj->unkA8);
    gte_rtv0();
    gte_stlvnl(&obj->unkA8);
    gte_ldlv0(&obj->unkB8);
    gte_rtv0();
    gte_stlvnl(&obj->unkB8);

    return 1;
}

extern s32 D_800A314C;

/* Rotates the three points p0/p1/p2 (relative to the origin) by the current GTE
 * rotation matrix, cuts the rotated triangle with the plane z = 0, and returns
 * 1 if the resulting segment (x1,y1)-(x2,y2) touches the triangle (0,0) / A / B
 * in the x-y plane: either endpoint inside it (same side of every edge as the
 * centroid), or the segment crossing one of its edges. */
s32 func_8002DE20(Unk1F8002B8Rec *obj, Unk80107850Rec *p0, Unk80107850Rec *p1,
                  Unk80107850Rec *p2) {
    s32 i;
    s32 max_i;
    s32 min_i;
    s32 mid_i;
    s32 min_z;
    s32 max_z;
    s32 z;
    s32 dz_a;
    s32 dz_b;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 cx;
    s32 cy;
    /* cross_a / cross_b: one value per same-side test, the 2D cross
     * product (edge) x (point - edge start) for the pair's first / second point
     * (Ruling 11, ordinary-c-judge-decidable) */
    s32 cross_a;
    s32 cross_b;
    /* edge A-B x ((x2,y2) - A) */
    s32 cross_ab2;

    obj->unkF8.vx = p0->x - obj->shape_points[0]->x;
    obj->unkF8.vy = p0->y - obj->shape_points[0]->y;
    obj->unkF8.vz = p0->z - obj->shape_points[0]->z;
    gte_ldv0(&obj->unkF8);
    gte_rtv0();
    obj->unkF8.vx = p1->x - obj->shape_points[0]->x;
    obj->unkF8.vy = p1->y - obj->shape_points[0]->y;
    obj->unkF8.vz = p1->z - obj->shape_points[0]->z;
    gte_stlvnl(&obj->unk118[0]);
    gte_ldv0(&obj->unkF8);
    gte_rtv0();
    obj->unkF8.vx = p2->x - obj->shape_points[0]->x;
    obj->unkF8.vy = p2->y - obj->shape_points[0]->y;
    obj->unkF8.vz = p2->z - obj->shape_points[0]->z;
    gte_stlvnl(&obj->unk118[1]);
    gte_ApplyRotMatrix(&obj->unkF8, &obj->unk118[2]);

    max_i = 0;
    min_i = 0;
    max_z = obj->unk118[0].z;
    min_z = max_z;
    for (i = 1; i < 3; i++) {
        z = obj->unk118[i].z;
        if (z < min_z) {
            min_i = i;
            min_z = z;
        } else if (max_z < z) {
            max_i = i;
            max_z = z;
        }
    }
    if (min_z > 0 || max_z < 0) {
        return 0;
    }
    if (min_z == 0 && max_z == 0) {
        min_i = 0;
        max_i = 1;
        obj->unk118[0].z--;
        obj->unk118[1].z++;
    }

    /* where the min->max edge crosses z = 0 */
    dz_a = obj->unk118[max_i].z - obj->unk118[min_i].z;
    if (dz_a == 0) {
        D_800A314C++;
        dz_a = 1;
    }
    x1 = obj->unk118[min_i].x +
         (-obj->unk118[min_i].z *
          (obj->unk118[max_i].x - obj->unk118[min_i].x)) /
             dz_a;
    y1 = obj->unk118[min_i].y +
         (-obj->unk118[min_i].z *
          (obj->unk118[max_i].y - obj->unk118[min_i].y)) /
             dz_a;

    /* where the edge through the middle point crosses z = 0 */
    mid_i = 3 - min_i - max_i;
    if (obj->unk118[mid_i].z >= 0) {
        dz_b = obj->unk118[mid_i].z - obj->unk118[min_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[min_i].x +
             (-obj->unk118[min_i].z *
              (obj->unk118[mid_i].x - obj->unk118[min_i].x)) /
                 dz_b;
        y2 = obj->unk118[min_i].y +
             (-obj->unk118[min_i].z *
              (obj->unk118[mid_i].y - obj->unk118[min_i].y)) /
                 dz_b;
    } else {
        dz_b = obj->unk118[max_i].z - obj->unk118[mid_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[mid_i].x +
             (-obj->unk118[mid_i].z *
              (obj->unk118[max_i].x - obj->unk118[mid_i].x)) /
                 dz_b;
        y2 = obj->unk118[mid_i].y +
             (-obj->unk118[mid_i].z *
              (obj->unk118[max_i].y - obj->unk118[mid_i].y)) /
                 dz_b;
    }

    cx = (obj->unkA8.x + obj->unkB8.x) / 3;
    cy = (obj->unkA8.y + obj->unkB8.y) / 3;

    /* (x1,y1) inside the triangle */
    cross_a = obj->unkA8.y * cx - obj->unkA8.x * cy;
    cross_b = obj->unkA8.y * x1 - obj->unkA8.x * y1;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.y * cx - obj->unkB8.x * cy;
        cross_b = obj->unkB8.y * x1 - obj->unkB8.x * y1;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.y - obj->unkA8.y) * (cx - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (cy - obj->unkA8.y);
            cross_b = (obj->unkB8.y - obj->unkA8.y) * (x1 - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (y1 - obj->unkA8.y);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* (x2,y2) inside the triangle */
    cross_a = obj->unkA8.y * cx - obj->unkA8.x * cy;
    cross_b = obj->unkA8.y * x2 - obj->unkA8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.y * cx - obj->unkB8.x * cy;
        cross_b = obj->unkB8.y * x2 - obj->unkB8.x * y2;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.y - obj->unkA8.y) * (cx - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (cy - obj->unkA8.y);
            cross_b = (obj->unkB8.y - obj->unkA8.y) * (x2 - obj->unkA8.x) -
                      (obj->unkB8.x - obj->unkA8.x) * (y2 - obj->unkA8.y);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* the segment against edge (0,0)-A */
    cross_a = obj->unkA8.y * x1 - obj->unkA8.x * y1;
    cross_b = obj->unkA8.y * x2 - obj->unkA8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkA8.x - x1) - (x2 - x1) * (obj->unkA8.y - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge (0,0)-B */
    cross_a = obj->unkB8.y * x1 - obj->unkB8.x * y1;
    cross_b = obj->unkB8.y * x2 - obj->unkB8.x * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkB8.x - x1) - (x2 - x1) * (obj->unkB8.y - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge A-B */
    cross_a = (obj->unkB8.y - obj->unkA8.y) * (x1 - obj->unkA8.x) -
              (obj->unkB8.x - obj->unkA8.x) * (y1 - obj->unkA8.y);
    cross_ab2 = (obj->unkB8.y - obj->unkA8.y) * (x2 - obj->unkA8.x) -
                (obj->unkB8.x - obj->unkA8.x) * (y2 - obj->unkA8.y);
    if ((cross_a ^ cross_ab2) >= 0) {
        cross_a =
            (y2 - y1) * (obj->unkA8.x - x1) - (x2 - x1) * (obj->unkA8.y - y1);
        cross_b =
            (y2 - y1) * (obj->unkB8.x - x1) - (x2 - x1) * (obj->unkB8.y - y1);
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    return 0;
}

/* Point-in-triangle test: is test_point on the same side of each triangle edge
 * as the centroid? For each of the three edges (vertex_a->vertex_b,
 * vertex_a->vertex_c, vertex_b->vertex_c) the 2D cross product of the edge
 * vector with the centroid offset and with the query-point offset must have the
 * same sign; the sign agreement is tested as `(cc ^ cp) >= 0`. Same idiom as
 * the triangle test inside math_TestPreparedTriangleSphere in this file. */
s32 math_PointInTriangleXZ(
    s32 *vertex_a, s32 *vertex_b, s32 *vertex_c, s32 *test_point) {
    s32 centroid_x = ((vertex_a[0] + vertex_b[0]) + vertex_c[0]) / 3;
    s32 centroid_z = ((vertex_a[2] + vertex_b[2]) + vertex_c[2]) / 3;
    s32 centroid_cross;
    s32 point_cross;

    {
        s32 dz = vertex_b[2] - vertex_a[2];
        s32 dx = vertex_b[0] - vertex_a[0];
        centroid_cross = (dz * (centroid_x - vertex_a[0])) -
                         (dx * (centroid_z - vertex_a[2]));
        point_cross = (dz * (test_point[0] - vertex_a[0])) -
                      (dx * (test_point[2] - vertex_a[2]));
    }
    if ((centroid_cross ^ point_cross) >= 0) {
        {
            s32 dz = vertex_c[2] - vertex_a[2];
            s32 dx = vertex_c[0] - vertex_a[0];
            centroid_cross = (dz * (centroid_x - vertex_a[0])) -
                             (dx * (centroid_z - vertex_a[2]));
            point_cross = (dz * (test_point[0] - vertex_a[0])) -
                          (dx * (test_point[2] - vertex_a[2]));
        }
        if ((centroid_cross ^ point_cross) >= 0) {
            {
                s32 dz = vertex_c[2] - vertex_b[2];
                s32 dx = vertex_c[0] - vertex_b[0];
                centroid_cross = (dz * (centroid_x - vertex_b[0])) -
                                 (dx * (centroid_z - vertex_b[2]));
                point_cross = (dz * (test_point[0] - vertex_b[0])) -
                              (dx * (test_point[2] - vertex_b[2]));
            }
            if ((centroid_cross ^ point_cross) >= 0) {
                return 1;
            }
        }
    }
    return 0;
}

void math_PrepareSegmentCollisionFrame(Unk1F8002B8Rec *scr) {
    s32 sp_tmp;
    MATRIX *mat;
    s32 dist_sq;
    s32 angle;
    s32 dist;

    scr->unkA8.x = scr->shape_points[1]->x - scr->shape_points[0]->x;
    scr->unkA8.y = scr->shape_points[1]->y - scr->shape_points[0]->y;
    scr->unkA8.z = scr->shape_points[1]->z - scr->shape_points[0]->z;
    angle = ratan2(scr->unkA8.x, scr->unkA8.z);
    dist_sq = scr->unkA8.x * scr->unkA8.x + scr->unkA8.z * scr->unkA8.z;
    scr->unkF8.vy = 0x800 - angle;

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            gte_Lzc(dist_sq, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = g_sqrt_table_u8[(u32)dist_sq >> shift];
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    angle = ratan2(scr->unkA8.y, dist);
    mat = &scr->rot_matrix;
    scr->unkF8.vx = 0x800 - angle;

    /* identity 3x3 rotation matrix at scr->rot_matrix */
    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixY(scr->unkF8.vy, mat);
    RotMatrixX(scr->unkF8.vx, mat);

    gte_SetRotMatrix(mat);
    gte_ldlv0(&scr->unkA8);
    gte_rtv0();
    gte_stlvnl(&scr->unkA8);
}

s32 math_TestPreparedSegmentSphere(
    Unk1F8002B8Rec *scr, Unk80107850Rec *pos, s32 threshold, s32 r_sq) {
    scr->unkF8.vx = pos->x - scr->shape_points[0]->x;
    scr->unkF8.vy = pos->y - scr->shape_points[0]->y;
    scr->unkF8.vz = pos->z - scr->shape_points[0]->z;
    gte_ApplyRotMatrix(&scr->unkF8, &scr->unk100[0]);

    {
        s32 y;
        /* r_sq - sq, then its square root in place (the sphere's half-chord
         * along z), as SOTN's cloaked knight takes
         * `distance = SquareRoot0(distance)` (Q51) */
        /* SOTN: src/st/nz1/e_cloaked_knight.c:83 @db41b28 */
        s32 rad;
        s32 sp_var;
        s32 min_z;
        s32 max_z;
        s32 az;
        s32 z;
        s32 x;
        s32 neg_threshold = -threshold;
        s32 sq;

        x = scr->unk100[0].x;
        if (x < neg_threshold || threshold < x)
            return 0;
        y = scr->unk100[0].y;
        if (y < neg_threshold || threshold < y)
            return 0;

        sq = x * x + y * y;
        if (r_sq < sq)
            return 0;
        rad = r_sq - sq;

        if ((u32)rad < 0x400) {
            rad = (u32)g_sqrt_table_u8[rad] >> 3;
        } else {
            s32 lzcr;
            if (rad < 0) {
                lzcr = 0;
            } else {
                gte_Lzc(rad, &sp_var);
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)rad >> shift];
                rad = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_z = 0;
        min_z = 0;
        az = scr->unkA8.z;
        if (az < 0) {
            min_z = az;
        } else {
            max_z = az;
        }
        z = scr->unk100[0].z;
        if (max_z < z - rad || z + rad < min_z)
            return 0;
        return 1;
    }
}

void math_ScaleVectorInDirectionFrame(
    s32 *vec_in, s16 *arg1, s32 *out, s32 scale_z, s32 scale_xy) {
    s32 sp_tmp;
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 angle1;
    s32 angle2;
    s32 dist_sq;
    s32 dist;

    angle1 = ratan2(arg1[0], arg1[2]);
    scr->unkF8.vy = 0x800 - angle1;
    dist_sq = arg1[0] * arg1[0] + arg1[2] * arg1[2];

    if ((u32)dist_sq < 0x400) {
        dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (dist_sq >= 0) {
            gte_Lzc(dist_sq, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            dist = (u32)(g_sqrt_table_u8[(u32)dist_sq >> shift] << 16) >>
                   (0x13 - ((u32)shift >> 1));
        }
    }

    angle2 = ratan2(arg1[1], dist);
    mat = &scr->rot_matrix;
    scr->unkF8.vx = 0x800 - angle2;

    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixY(scr->unkF8.vy, mat);
    RotMatrixX(scr->unkF8.vx, mat);

    gte_SetRotMatrix(mat);
    gte_ldlv0(vec_in);
    gte_rtv0();
    vec = &scr->unkA8;
    gte_stlvnl(vec);

    vec->z = (vec->z * scale_z) / 256;
    vec->x = (vec->x * scale_xy) / 256;
    vec->y = (vec->y * scale_xy) / 256;

    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixX(-scr->unkF8.vx, mat);
    RotMatrixY(-scr->unkF8.vy, mat);

    gte_SetRotMatrix(mat);
    gte_ldlv0(vec);
    gte_rtv0();
    gte_stlvnl(out);
}

void math_InvertMatrix3x3(void *arg0, void *arg1) {
    MATRIX *src = arg0;
    MATRIX *dst = arg1;
    s16 temp_a3;
    s16 temp_t0;
    s16 temp_t1;
    s16 temp_t2;
    s16 temp_v0;
    s16 temp_v1;
    s32 temp_a2;
    s32 temp_v1_2;

    temp_t2 = src->m[1][2];
    temp_t1 = src->m[2][1];
    temp_t0 = src->m[1][1];
    temp_a3 = src->m[2][2];
    temp_a2 = (temp_t2 * temp_t1) - (temp_t0 * temp_a3);
    temp_v1 = src->m[0][1];
    temp_v0 = src->m[0][2];
    temp_v1_2 =
        ((src->m[0][0] * (temp_a2 >> 0xC)) +
         (src->m[1][0] * (((temp_v1 * temp_a3) - (temp_v0 * temp_t1)) >> 0xC)) +
         (src->m[2][0] *
          (((temp_v0 * temp_t0) - (temp_v1 * temp_t2)) >> 0xC))) >>
        0xC;
    dst->m[0][0] = (temp_a2 / temp_v1_2);
    dst->m[0][1] =
        (((src->m[0][1] * src->m[2][2]) - (src->m[0][2] * src->m[2][1])) /
         temp_v1_2);
    dst->m[0][2] =
        (((src->m[0][2] * src->m[1][1]) - (src->m[0][1] * src->m[1][2])) /
         temp_v1_2);
    dst->m[1][0] =
        (((src->m[1][0] * src->m[2][2]) - (src->m[1][2] * src->m[2][0])) /
         temp_v1_2);
    dst->m[1][1] =
        (((src->m[0][2] * src->m[2][0]) - (src->m[0][0] * src->m[2][2])) /
         temp_v1_2);
    dst->m[1][2] =
        (((src->m[0][0] * src->m[1][2]) - (src->m[0][2] * src->m[1][0])) /
         temp_v1_2);
    dst->m[2][0] =
        (((src->m[1][1] * src->m[2][0]) - (src->m[1][0] * src->m[2][1])) /
         temp_v1_2);
    dst->m[2][1] =
        (((src->m[0][0] * src->m[2][1]) - (src->m[0][1] * src->m[2][0])) /
         temp_v1_2);
    dst->m[2][2] =
        (((src->m[0][1] * src->m[1][0]) - (src->m[0][0] * src->m[1][1])) /
         temp_v1_2);
}

void math_MatrixToAngles(MATRIX *a0, s16 *a1);

void math_MatrixToAngles(MATRIX *a0, s16 *a1) {
    MATRIX *m;
    Unk1F8002B8Rec *scr;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 c0, c1, c2;
    /* FAKE: two values -- the 3x3 determinant (divisor of the six
     * cofactors), then the square root of i0*i0 + i1*i1; a separate root
     * local: score 35 (Ruling 11) */
    s32 var_7;
    s32 d0;
    s32 i0, i1, i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    /* temp: i0*i0 + i1*i1, then its square-root table byte, looked up in
     * place as SOTN's shop takes `itemId = D_us_801814D4[itemId]` (Q51) */
    /* SOTN: src/st/lib/e_shop.c:3168 @db41b28 */
    s32 temp;
    s32 sp_tmp;

    m = &SPAD->unk2B8.rec.rot_matrix;
    *m = *a0;

    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    var_7 = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    i0 = c0 / var_7;
    i1 = c1 / var_7;
    i2 = c2 / var_7;
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / var_7;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / var_7;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / var_7;

    ang_z = -ratan2(i1, i0);
    scr = &SPAD->unk2B8.rec;
    temp = i0 * i0 + i1 * i1;
    if ((u32)temp < 0x400) {
        var_7 = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            gte_Lzc(temp, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            var_7 = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    ang_y = ratan2(i2, var_7);
    mat = &scr->rot_matrix;
    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixZ(ang_z, mat);
    RotMatrixY(ang_y, mat);

    gte_SetRotMatrix(mat);
    vec = &scr->unkA8;
    vec->x = r0;
    vec->y = r1;
    vec->z = r2;
    gte_ldlv0(vec);
    gte_rtv0();
    gte_stlvnl(vec);

    a1[0] = ratan2(vec->z, vec->y);
    a1[1] = -ang_y;
    a1[2] = -ang_z;
}

void math_ComposeAxisRotationsAndRewriteAngles(
    s16 *angles, s32 z, s32 y, s32 x) {
    MATRIX *m;
    Unk1F8002B8Rec *scr;
    MATRIX *mat;
    Vec3i32 *vec;
    s32 c0, c1, c2;
    /* FAKE: two values -- the 3x3 determinant (divisor of the six
     * cofactors), then the square root of i0*i0 + i1*i1; a separate root
     * local: score 35 (Ruling 11) */
    s32 var_7;
    s32 d0;
    s32 i0, i1, i2;
    s32 r0, r1, r2;
    s32 ang_z, ang_y;
    /* temp: i0*i0 + i1*i1, then its square-root table byte, looked up in
     * place as SOTN's shop takes `itemId = D_us_801814D4[itemId]` (Q51) */
    /* SOTN: src/st/lib/e_shop.c:3168 @db41b28 */
    s32 temp;
    s32 sp_tmp;

    scr = &SPAD->unk2B8.rec;
    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixX(x, &scr->rot_matrix);
    RotMatrixY(y, &scr->rot_matrix);
    RotMatrixZ(-z, &scr->rot_matrix);
    RotMatrixX(angles[0], &scr->rot_matrix);
    RotMatrixY(angles[1], &scr->rot_matrix);
    RotMatrixZ(angles[2], &scr->rot_matrix);

    m = &scr->rot_matrix;
    c0 = m->m[1][2] * m->m[2][1] - m->m[1][1] * m->m[2][2];
    d0 = m->m[0][0] * (c0 >> 12);
    c1 = m->m[0][1] * m->m[2][2] - m->m[0][2] * m->m[2][1];
    c2 = m->m[0][2] * m->m[1][1] - m->m[0][1] * m->m[1][2];
    var_7 = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;
    i0 = c0 / var_7;
    i1 = c1 / var_7;
    i2 = c2 / var_7;
    r0 = (m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0]) / var_7;
    r1 = (m->m[0][2] * m->m[2][0] - m->m[0][0] * m->m[2][2]) / var_7;
    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / var_7;

    ang_z = -ratan2(i1, i0);
    temp = i0 * i0 + i1 * i1;
    if ((u32)temp < 0x400) {
        var_7 = (u32)g_sqrt_table_u8[temp] >> 3;
    } else {
        s32 lzcr = 0;
        if (temp >= 0) {
            gte_Lzc(temp, &sp_tmp);
            lzcr = sp_tmp;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            temp = g_sqrt_table_u8[(u32)temp >> shift];
            var_7 = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }

    ang_y = ratan2(i2, var_7);
    mat = &scr->rot_matrix;
    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixZ(ang_z, mat);
    RotMatrixY(ang_y, mat);

    gte_SetRotMatrix(mat);
    vec = &scr->unkA8;
    vec->x = r0;
    vec->y = r1;
    vec->z = r2;
    gte_ldlv0(vec);
    gte_rtv0();
    gte_stlvnl(vec);

    angles[0] = ratan2(vec->z, vec->y);
    angles[1] = -ang_y;
    angles[2] = -ang_z;
}

s32 math_TriangleNormalYawWithPositiveYHalfTurn(
    VECTOR *a0, VECTOR *a1, VECTOR *a2) {
    Vec3i32 *p;
    s32 ret;

    /* Difference vectors (a1 - a0) and (a2 - a0) into the scratchpad
     * collision record's unkA8 / unkB8, the GTE operands below. */
    SPAD->unk2B8.rec.unkA8.x = a1->vx - a0->vx;
    SPAD->unk2B8.rec.unkA8.y = a1->vy - a0->vy;
    SPAD->unk2B8.rec.unkA8.z = a1->vz - a0->vz;
    SPAD->unk2B8.rec.unkB8.x = a2->vx - a0->vx;
    SPAD->unk2B8.rec.unkB8.y = a2->vy - a0->vy;
    SPAD->unk2B8.rec.unkB8.z = a2->vz - a0->vz;

    /* Their cross product, into unkC8. */
    gte_ldopv1(&SPAD->unk2B8.rec.unkA8);
    gte_ldopv2(&SPAD->unk2B8.rec.unkB8);
    gte_op0();
    p = &SPAD->unk2B8.rec.unkC8;
    gte_stlvnl(p);
    /* Angle of the cross product in the XZ plane, +0x800 (180 deg) when its
     * Y component is positive. */
    ret = ratan2(p->x, p->z);
    if (p->y > 0) {
        ret += 0x800;
    }
    return ret;
}

s32 fighter_IsBoneTriangleCrossYPositive(Unk80101EC8Record *arg0) {
    Unk80107850Rec *var_0;

    /* The character's points 2 and 3 relative to its point 1, into the
     * collision record's unkA8 and unkB8, the GTE operands below; their
     * cross product lands in unkC8. */
    var_0 = SPAD->unkA8[arg0->index];
    SPAD->unk2B8.rec.unkA8.x = var_0[2].x - var_0[1].x;
    SPAD->unk2B8.rec.unkA8.y = var_0[2].y - var_0[1].y;
    SPAD->unk2B8.rec.unkA8.z = var_0[2].z - var_0[1].z;
    SPAD->unk2B8.rec.unkB8.x = var_0[3].x - var_0[1].x;
    SPAD->unk2B8.rec.unkB8.y = var_0[3].y - var_0[1].y;
    SPAD->unk2B8.rec.unkB8.z = var_0[3].z - var_0[1].z;

    gte_OuterProduct0(&SPAD->unk2B8.rec.unkA8, &SPAD->unk2B8.rec.unkB8,
                      &SPAD->unk2B8.rec.unkC8);
    /* unkC8.y (MAC2) > 0 */
    return 0 < SPAD->unk2B8.rec.unkC8.y;
}

/* Starts the record (is_attached = 1, bone attached_bone = arg1): its matrix
 * becomes the inverse of bone attached_bone's rotation
 * (node_GetSlotMatrixPointerArray()'s table, inverted by math_InvertMatrix3x3)
 * times the rot[] rotation (X, Y, Z), and pos is taken into the bone's frame:
 * less the bone translation, rotated by that inverse, then halved. */
void object_AttachToOpponentBone(Obj80106A78 *arg0, s16 arg1) {
    MATRIX mat_local;
    MATRIX **mat_tbl;
    MATRIX *s2_ptr;
    MATRIX *rot_mat;

    arg0->is_attached = 1;
    arg0->attached_bone = arg1;
    mat_tbl = node_GetSlotMatrixPointerArray(arg0->owner < 1);
    rot_mat = &arg0->mtx;
    s2_ptr = mat_tbl[arg0->attached_bone];

    /* 3x3 identity rotation in the record's matrix. */
    arg0->mtx.m[0][0] = 0x1000;
    arg0->mtx.m[0][1] = 0;
    arg0->mtx.m[0][2] = 0;
    arg0->mtx.m[1][0] = 0;
    arg0->mtx.m[1][1] = 0x1000;
    arg0->mtx.m[1][2] = 0;
    arg0->mtx.m[2][0] = 0;
    arg0->mtx.m[2][1] = 0;
    arg0->mtx.m[2][2] = 0x1000;
    RotMatrixX(arg0->rot[0], rot_mat);
    RotMatrixY(arg0->rot[1], rot_mat);
    RotMatrixZ(arg0->rot[2], rot_mat);
    math_InvertMatrix3x3(s2_ptr, &mat_local);
    MulMatrix0(&mat_local, rot_mat, rot_mat);

    /* Subtract mat_tbl[arg0->attached_bone]'s translation from the position. */
    arg0->pos.x -= s2_ptr->t[0];
    arg0->pos.y -= s2_ptr->t[1];
    arg0->pos.z -= s2_ptr->t[2];

    gte_SetRotMatrix(&mat_local);
    gte_ldlv0(&arg0->pos);
    gte_rtv0();
    gte_stlvnl(&arg0->pos);

    /* Halve x, y, z (signed arithmetic shift). */
    arg0->pos.x >>= 1;
    arg0->pos.y >>= 1;
    arg0->pos.z >>= 1;
}

/* Rotates the object's local vector (pos) by bone attached_bone's matrix of
 * node_GetSlotMatrixPointerArray() and adds that matrix's translation, composes
 * the bone matrix with the object's, and hands the point and the angles
 * math_MatrixToAngles reads from that matrix to
 * node_QueueEntryNodesFromTransformOrUpdatedSlotNode / replay_RecordModelDraw.
 */
void object_SubmitAttachedTransform(Obj80106A78 *arg0) {
    s32 mac[3];
    s16 var_1[4];
    MATRIX mtx;
    MATRIX **mat_tbl;
    MATRIX *mat;
    s32 lookup;

    mat_tbl = node_GetSlotMatrixPointerArray(arg0->owner < 1);
    mat = mat_tbl[arg0->attached_bone];

    gte_SetRotMatrix(mat);
    gte_ldlv0(&arg0->pos);
    gte_rtv0();
    gte_stlvnl(mac);

    /* Add the matrix translation to the rotated vector. */
    mac[0] += mat->t[0];
    mac[1] += mat->t[1];
    mac[2] += mat->t[2];

    MulMatrix0(mat, &arg0->mtx, &mtx);
    math_MatrixToAngles(&mtx, var_1);

    lookup = g_resource_kind_entry_ids[arg0->kind];
    node_QueueEntryNodesFromTransformOrUpdatedSlotNode(lookup, 1, mac, var_1);
    replay_RecordModelDraw(arg0->slot, lookup, mac, var_1);
}

void object_SubmitPoolTransforms(void) {
    Obj80106A78 *obj;
    s32 i;
    s16 kind;
    s32 lookup;

    for (obj = g_object_pool, i = 0; i < 12; i++, obj++) {
        kind = obj->kind;
        if (kind == -1) {
            continue;
        }
        if (obj->is_attached != 0) {
            object_SubmitAttachedTransform(obj);
            continue;
        }
        if (obj->moving_ticks < 2) {
            continue;
        }
        lookup = g_resource_kind_entry_ids[kind];
        if (kind >= 0x12 && kind < 0x1E) {
            lookup = obj->unk_0B;
        } else if (kind == 0xE && obj->unk_05 == 2) {
            lookup += 3;
        }
        node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
            lookup, 1, &obj->pos.x, obj->rot);
        replay_RecordModelDraw(obj->slot, lookup, &obj->pos.x, obj->rot);
    }
}

void math_ReflectAndDampRecordVector(s32 *a0, s16 *a1) {
    s32 angle;
    s32 cos_val;
    s32 sin_val;
    s32 vx;
    s32 vz;
    s32 rx;
    s32 rz;
    s32 v48;
    angle = ratan2(a1[0], a1[2]);
    cos_val = Judge[(angle + 0x400) & 0xFFF];
    sin_val = Judge[angle & 0xFFF];
    vx = a0[0x11];
    vz = a0[0x13];
    rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
    v48 = a0[0x12];
    a0[0x11] = ((rx * cos_val) - (rz * sin_val)) >> 15;
    a0[0x13] = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    a0[0x12] = v48 >> 2;
}

void object_ResetPool(void) {
    s32 i;

    for (i = 0; i < 12; i++) {
        g_object_pool[i].kind = -1;
        g_object_pool[i].slot = 0xFF;
    }
}

void object_InitKindQueue(Unk80101EC8Record *a0) {
    s32 i;
    s16 val;

    a0->carried_object_count = 0;
    i = 0;
    do {
        val = D_8008E338[a0->character_data_index][i];
        a0->carried_object_kinds[i] = val;
        if (val != -1) {
            a0->carried_object_count++;
        }
        i++;
    } while (i < 5);
    D_800A36F2[a0->index] = D_8008E338[a0->character_data_index][0];
}

void object_FreeAttachedObjects(void) {
    s32 i = 0;
    do {
        if (g_object_pool[i].kind != -1) {
            if (g_object_pool[i].is_attached != 0) {
                g_object_pool[i].kind = -1;
            }
        }
    } while (++i < 12);
}

Obj80106A78 *object_SpawnFromFighter(Unk80101EC8Record *arg0, s32 arg1) {
    /* FAKE: unwritten pad reserving the 16 untouched local bytes of the
     * target frame (target vars=24, ours 8 without the pad) (phantom-frame pad,
     * owner ruling 2026-08-18; row granted by owner ruling 2026-09-02). SOTN
     * precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. */
    volatile u32 pre_pad[4]; /* !FAKE: without it score 2 */
    Obj80106A78 *obj;
    Tbl8008E194 *tbl;
    s32 i;

    obj = g_object_pool;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->kind == -1 && obj->slot == 0xFF)
            break;
    }
    obj->slot = i;
    obj->kind = arg1;
    obj->unk_07 = 0;
    obj->is_attached = 0;
    obj->unk_04 = 1;
    obj->owner = arg0->index;
    obj->pos.x = arg0->world_pos.x;
    obj->pos.y = arg0->world_pos.y - arg0->size_scale / 32;
    obj->pos.z = arg0->world_pos.z;
    tbl = &D_8008E194[arg1];
    obj->vel.x = (Judge[arg0->orientation.vy & 0xFFF] * tbl->unk4) >> 12;
    obj->vel.y = tbl->unk6;
    obj->vel.z =
        (Judge[(arg0->orientation.vy + 0x400) & 0xFFF] * tbl->unk4) >> 12;
    obj->pos.x += obj->vel.x;
    obj->pos.y += obj->vel.y;
    obj->pos.z += obj->vel.z;
    obj->pos.x += obj->vel.x / 2;
    obj->pos.y += obj->vel.y / 2;
    obj->pos.z += obj->vel.z / 2;
    obj->prev_pos = obj->pos;
    obj->rot[0] = 0;
    obj->rot[1] = arg0->orientation.vy;
    obj->rot[2] = 0;
    if (tbl->unk0 == 1) {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = tbl->unk8;
        obj->rot_vel[2] = 0;
    } else if (tbl->unk0 == 2) {
        obj->rot_vel[0] = tbl->unk8;
        obj->rot_vel[1] = 0;
        obj->rot_vel[2] = 0;
    } else if (tbl->unk0 == 3) {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = tbl->unk8;
        obj->rot_vel[2] = 0;
    } else {
        obj->rot_vel[0] = 0;
        obj->rot_vel[1] = 0;
        obj->rot_vel[2] = 0;
    }
    obj->is_moving = 1;
    obj->unk_05 = 0;
    obj->moving_ticks = 0;
    return obj;
}

extern Obj80106A78 *object_SpawnFromFighter(Unk80101EC8Record *, s32);

s32 object_ThrowQueuedKind(Unk80101EC8Record *a0) {
    s32 count;
    s32 idx;
    s32 kind;
    s32 id;
    Obj80106A78 *obj;
    s32 i;

    count = a0->carried_object_count;
    if (count == 0) {
        return -1;
    }
    idx = 0;
    if (count >= 2 && a0->object_variant_index != -1) {
        idx = a0->carried_object_kinds[0] == a0->wielded_object_kind;
    }
    id = a0->carried_object_kinds[idx];
    obj = object_SpawnFromFighter(a0, id);
    for (i = idx; i < a0->carried_object_count - 1; i++) {
        a0->carried_object_kinds[i] = a0->carried_object_kinds[i + 1];
    }

    a0->carried_object_count--;
    kind = obj->kind;
    if (kind == 0xE) {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            (D_800A36F2[0] ^ 0xE) != 0, 0x2F, &obj->pos.x, 0);
    } else {
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            (kind ^ D_800A36F2[0]) != 0, 0x2A, &obj->pos.x, 0);
    }
    return id;
}

extern Obj80106A78 *object_SpawnFromFighter(Unk80101EC8Record *, s32);

void object_DropQueuedHeadAt(Unk80101EC8Record *a0, Vec3i32 *a1) {
    Obj80106A78 *p;
    s32 var_1;
    s32 i;

    p = object_SpawnFromFighter(a0, a0->carried_object_kinds[0]);
    p->unk_04 = 0;
    p->pos = *a1;
    p->vel.x = (rng_Next() & 0xFF) - 0x80;
    p->vel.y = -(rng_Next() & 0x3F) - 0x80;
    p->vel.z = (rng_Next() & 0xFF) - 0x80;
    var_1 = rng_Next();
    if (var_1 & 0x1000) {
        p->rot_vel[0] = (var_1 & 0x3FF) + 0x200;
    } else {
        p->rot_vel[0] = -(var_1 & 0x3FF) - 0x200;
    }
    p->rot_vel[1] = (rng_Next() & 0x7FF) - 0x400;
    p->rot_vel[2] = 0;
    p->unk_07 = 1;
    for (i = 0; i < a0->carried_object_count - 1; i++) {
        a0->carried_object_kinds[i] = a0->carried_object_kinds[i + 1];
    }
    a0->carried_object_count--;
}

void object_SpawnScatteredAt(Unk80101EC8Record *a0, s32 a1, Vec3i32 *a2) {
    Obj80106A78 *p;
    s32 var_1;

    p = object_SpawnFromFighter(a0, a1);
    p->unk_04 = 0;
    p->pos = *a2;
    p->vel.x = (rng_Next() & 0xFF) - 0x80;
    p->vel.y = -(rng_Next() & 0x3F) - 0x80;
    p->vel.z = (rng_Next() & 0xFF) - 0x80;
    var_1 = rng_Next();
    if (var_1 & 0x1000) {
        p->rot_vel[0] = (var_1 & 0x3FF) + 0x200;
    } else {
        p->rot_vel[0] = -(var_1 & 0x3FF) - 0x200;
    }
    p->rot_vel[1] = (rng_Next() & 0x7FF) - 0x400;
    p->rot_vel[2] = 0;
    p->unk_07 = 1;
    p->unk_0B = a0->weapon_entry_id;
}

s32 object_QueueKind(Unk80101EC8Record *arg0, s32 arg1) {
    s32 i;

    if (arg0->carried_object_count == 0xC) {
        return 0;
    }
    if (arg0->object_variant_index != -1 && arg1 == arg0->wielded_object_kind) {
        for (i = arg0->carried_object_count; i > 0; i--) {
            arg0->carried_object_kinds[i] = arg0->carried_object_kinds[i - 1];
        }
        arg0->carried_object_kinds[0] = arg1;
        arg0->carried_object_count++;
    } else {
        arg0->carried_object_kinds[arg0->carried_object_count++] = arg1;
    }
    return 1;
}

s32 object_PickUpNearby(Unk80101EC8Record *arg0) {
    Obj80106A78 *p;
    s32 i;
    s32 old_val;

    for (i = 0, p = g_object_pool; i < 12; i++, p++) {
        s16 kind = p->kind;
        if (kind >= 0x12 && kind < 0x1E) {
            continue;
        }
        if (kind == -1) {
            continue;
        }
        if (p->is_moving != 0) {
            continue;
        }
        {
            s32 var_4 = arg0->collision_pos.vy;
            s32 py = p->pos.y;
            if (var_4 - 0x64 >= py) {
                continue;
            }
            if (py >= var_4 + 0x64) {
                continue;
            }
        }
        {
            s32 dx = arg0->world_pos.x - p->pos.x;
            s32 dz = arg0->world_pos.z - p->pos.z;
            if (dx * dx + dz * dz > 0xF423F) {
                continue;
            }
        }
        if (object_QueueKind(arg0, kind) == 0) {
            return -1;
        }
        old_val = p->kind;
        p->kind = -1;
        if (old_val == 0xE) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                (D_800A36F2[0] ^ 0xE) != 0, 0x2F, &arg0->world_pos.x, 0);
        } else {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                arg0->index, 0x11, &arg0->world_pos.x, 0);
        }
        return old_val;
    }
    return -1;
}

void object_ClearFixedKindRange(void) {
    s32 i = 0;
    do {
        if (g_object_pool[i].kind >= 0x12 && g_object_pool[i].kind < 0x1E) {
            g_object_pool[i].kind = -1;
        }
    } while (++i < 12);
}

void func_80030D48(void) {}

s32 math_LerpAngle(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = (arg0 - arg1) & 0xFFF;
    if (arg0 >= 0x800) {
        arg0 -= 0x1000;
    }
    return arg1 + ((arg0 * arg2) >> 12);
}

/* Per-frame update of the twelve 0x64-byte records at g_object_pool: velocity
 * turn, spin decay, gravity, collision test through
 * collision_TestSegmentExtendingShortStartWithWork, reflection off the returned
 * normal, fight_RecordReplayEventAndDispatchSoundOrEffect cues, and the rest /
 * re-hop logic. Scratch vectors live in its layout of the scratchpad area at
 * 0x1F8002B8 (Unk1F8002B8_8005344C). */
void object_UpdatePool(void) {
    Unk1F8002B8_8005344C *scr;
    Obj80106A78 *obj;
    s32 i;
    s32 half;
    s32 dot;
    s32 var_5;
    s16 kind;
    s16 *var_7;

    scr = &SPAD->unk2B8.v8005344C;
    obj = g_object_pool;
    for (i = 0; i < 12; i++, obj++) {
        /* work: the clamped turn amount (turn block), then the bounce
         * restitution factor D_8008E194[kind].unkA (Ruling 11,
         * ordinary-c-judge-decidable) */
        s32 work;
        /* temp: the ratan2() heading of the velocity (turn block), then
         * the collision_TestSegmentExtendingShortStartWithWork() collision
         * result (cleared when collision_GetHitTag() == 7) (Ruling 11,
         * ordinary-c-judge-decidable)
         */
        s32 temp;

        if (obj->kind == -1) {
            obj->slot = 0xFF;
            continue;
        }
        if (obj->is_attached != 0) {
            continue;
        }
        obj->prev_pos = obj->pos;
        if (obj->is_moving != 0) {
            obj->moving_ticks++;
        }
        if (obj->kind == 0x10 && obj->is_moving != 0 && obj->unk_05 == 0 &&
            obj->moving_ticks >= 14) {
            temp = ratan2(obj->vel.x, obj->vel.z);
            work = (0x4E - obj->moving_ticks) * 96 / 64;
            if (work < 0) {
                work = 0;
            } else if (work > 0x42) {
                work = 0x42;
            }
            scr->unk10.x = (Judge[(temp + 0x400) & 0xFFF] * obj->vel.x -
                            Judge[temp & 0xFFF] * obj->vel.z) >>
                           12;
            scr->unk10.y = obj->vel.y;
            scr->unk10.z = (Judge[temp & 0xFFF] * obj->vel.x +
                            Judge[(temp + 0x400) & 0xFFF] * obj->vel.z) >>
                           12;
            scr->unk20.x = scr->unk10.x;
            half = work / 2;
            scr->unk20.y = (Judge[(half + 0x400) & 0xFFF] * scr->unk10.y -
                            Judge[half & 0xFFF] * scr->unk10.z) >>
                           12;
            scr->unk20.z = (Judge[half & 0xFFF] * scr->unk10.y +
                            Judge[(half + 0x400) & 0xFFF] * scr->unk10.z) >>
                           12;
            obj->vel.x = (Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.x -
                          Judge[(work - temp) & 0xFFF] * scr->unk20.z) >>
                         12;
            obj->vel.y = scr->unk20.y;
            obj->vel.z =
                (Judge[(work - temp) & 0xFFF] * scr->unk20.x +
                 Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.z) >>
                12;
            obj->rot_vel[0] = obj->rot_vel[0] * 63 / 64;
            obj->rot_vel[1] = obj->rot_vel[1] * 63 / 64;
            obj->rot_vel[2] = obj->rot_vel[2] * 63 / 64;
        } else {
            obj->rot_vel[0] = obj->rot_vel[0] * 15 / 16;
            obj->rot_vel[1] = obj->rot_vel[1] * 15 / 16;
            obj->rot_vel[2] = obj->rot_vel[2] * 15 / 16;
        }
        obj->rot[0] += obj->rot_vel[0];
        obj->rot[1] += obj->rot_vel[1];
        obj->rot[2] += obj->rot_vel[2];
        obj->vel.y += 13;
        scr->cast_end.x = obj->pos.x + obj->vel.x;
        scr->cast_end.y = obj->pos.y + obj->vel.y;
        scr->cast_end.z = obj->pos.z + obj->vel.z;
        obj->pos.y -= 8;
        var_7 = scr->cast_normal;
        temp = collision_TestSegmentExtendingShortStartWithWork(
            &obj->pos.x, &scr->cast_end.x, &scr->unk10.x, var_7,
            (s32)&scr->cast_work);
        if (temp != 0 && collision_GetHitTag() == 7) {
            temp = 0;
        }
        if (temp != 0) {
            if (obj->kind == 0xF) {
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    obj->owner, 0xE, &scr->unk10.x, var_7);
                obj->kind = -1;
                continue;
            }
            dot = (obj->vel.x * var_7[0] + obj->vel.y * var_7[1] +
                   obj->vel.z * var_7[2]) /
                  2048;
            obj->vel.x -= var_7[0] * dot / 4096;
            obj->vel.y -= var_7[1] * dot / 4096;
            obj->vel.z -= var_7[2] * dot / 4096;
            obj->pos = scr->unk10;
            kind = obj->kind;
            work = D_8008E194[kind].unkA;
            if (obj->is_moving != 0) {
                obj->vel.x = obj->vel.x * work / 4096;
                obj->vel.y = obj->vel.y * work / 4096;
                obj->vel.z = obj->vel.z * work / 4096;
                var_5 = obj->vel.x * obj->vel.x + obj->vel.z * obj->vel.z;
                if (scr->cast_normal[1] >= -0x7FF) {
                    obj->rot_vel[1] +=
                        (rng_Next() & 1) ? var_5 / 64 : -var_5 / 64;
                    if (obj->kind != 0xE && obj->unk_04 != 0) {
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            obj->owner, 1, &obj->pos.x, 0);
                    }
                }
                if (obj->kind == 0xE) {
                    if (obj->unk_04 != 0) {
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            (obj->kind ^ D_800A36F2[0]) != 0, 0x2F, &obj->pos.x,
                            0);
                    }
                } else if (obj->kind < 0x12) {
                    if (var_5 > 0x10) {
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            (obj->kind ^ D_800A36F2[0]) != 0, 0x2C, &obj->pos.x,
                            0);
                    }
                } else if (obj->kind >= 0x12 && obj->kind < 0x1E) {
                    if (var_5 > 0x10) {
                        fight_RecordReplayEventAndDispatchSoundOrEffect(
                            obj->owner, 0x29, &obj->pos.x, 0);
                    }
                }
            } else if (scr->cast_normal[1] >= -0x7FF && kind == 0xE) {
                if (obj->unk_05 == 1) {
                    obj->is_moving = 1;
                    obj->rot[1] += 0x780 + (rng_Next() & 0xFF);
                    obj->vel.x = Judge[obj->rot[1] & 0xFFF] / 64;
                    obj->vel.y = -150;
                    obj->vel.z = Judge[(obj->rot[1] + 0x400) & 0xFFF] / 64;
                    obj->unk_05 = 2;
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        (obj->kind ^ D_800A36F2[0]) != 0, 0x2F, &obj->pos.x, 0);
                }
            }
            obj->unk_04 = 0;
            if (obj->unk_07 == 1) {
                obj->unk_07 = 2;
            }
            if (obj->unk_05 == 2) {
                obj->unk_05 = 3;
            }
        } else {
            obj->pos = scr->cast_end;
        }
        if (obj->vel.y >= -15 && obj->vel.y <= 15 && obj->vel.x >= -3 &&
            obj->vel.x <= 3 && obj->vel.z >= -3 && obj->vel.z <= 3) {
            if (temp != 0) {
                if (g_fight_mode_selector == 3) {
                    obj->kind = -1;
                } else {
                    obj->is_moving = 0;
                    obj->unk_05 = 1;
                    obj->vel.x = 0;
                    obj->vel.y = 0;
                    obj->vel.z = 0;
                }
            }
        } else {
            obj->is_moving = 1;
        }
        if (obj->kind == 0xE) {
            if (obj->unk_05 == 1 && !(rng_Next() & 0x133)) {
                obj->is_moving = 1;
                obj->rot_vel[1] += (rng_Next() & 0x7F) - 0x40;
                obj->vel.x = Judge[obj->rot[1] & 0xFFF] / 64;
                obj->vel.y = -150;
                obj->vel.z = Judge[(obj->rot[1] + 0x400) & 0xFFF] / 64;
                obj->unk_05 = 2;
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    (obj->kind ^ D_800A36F2[0]) != 0, 0x2F, &obj->pos.x, 0);
            }
        }
        if (obj->unk_07 == 2) {
            obj->rot[1] = math_LerpAngle(obj->rot[1], 0, 0x800);
            obj->rot[2] = math_LerpAngle(obj->rot[2], 0x400, 0xE00);
            obj->rot_vel[1] = obj->rot_vel[1] * 3 / 4;
        } else if (obj->unk_04 == 0 && D_8008E194[obj->kind].unk0 == 2) {
            obj->rot[0] = math_LerpAngle(obj->rot[0], 0, 0xE00);
            obj->rot[1] = math_LerpAngle(obj->rot[1], 0, 0x600);
            obj->rot[2] = math_LerpAngle(obj->rot[2], -0x400, 0xE00);
        }
        if (obj->pos.y > 0x3A98) {
            obj->kind = -1;
        }
    }
    object_SubmitPoolTransforms();
}

/* GTE rotate-velocity-by-table-angle (sibling of
 * math_PrepareSegmentCollisionFrame) */
void object_DeflectFromFighterHit(
    Unk1F8002B8Rec *scr, Obj80106A78 *ent, s32 idx) {
    MATRIX *mat;
    s32 angle1;
    s32 angle2;
    s32 sum_sq;
    s32 var_5;

    if (ent->kind != 0xE) {
        s32 vx = ent->vel.x;
        s32 vz = ent->vel.z;
        s32 var_8 = ent->rot_vel[1];
        sum_sq = vx * vx + vz * vz;
        if (rng_Next() & 1) {
            var_5 = sum_sq / 64;
        } else {
            var_5 = -sum_sq / 64;
        }
        ent->rot_vel[1] = var_8 + var_5;
    }

    mat = &scr->rot_matrix;
    angle1 = D_8008EBA0[idx] & 0xFFF;
    angle2 =
        (((ent->pos.x * 16) + ent->pos.y + (ent->pos.z * 8)) & 0x7FF) - 0x400;
    /* identity 3x3 rotation matrix at scr->rot_matrix */
    scr->rot_matrix.m[0][0] = 0x1000;
    scr->rot_matrix.m[0][1] = 0;
    scr->rot_matrix.m[0][2] = 0;
    scr->rot_matrix.m[1][0] = 0;
    scr->rot_matrix.m[1][1] = 0x1000;
    scr->rot_matrix.m[1][2] = 0;
    scr->rot_matrix.m[2][0] = 0;
    scr->rot_matrix.m[2][1] = 0;
    scr->rot_matrix.m[2][2] = 0x1000;
    RotMatrixY(angle1, mat);
    RotMatrixX(angle2, mat);

    gte_SetRotMatrix(mat);
    gte_ldlv0(&ent->vel);
    gte_rtv0();
    gte_stlvnl(&ent->vel);

    if ((u32)(angle1 - 0x401) < 0x7FFU) {
        ent->vel.x /= 8;
        ent->vel.y /= 8;
        ent->vel.z /= 8;
    } else {
        ent->vel.x /= 4;
        ent->vel.y /= 4;
        ent->vel.z /= 4;
    }
    ent->pos.x += ent->vel.x / 2;
    ent->pos.y += ent->vel.y / 2;
    ent->pos.z += ent->vel.z / 2;
}

extern s32 fighter_ResolveBoneHit(
    s32, Unk80101EC8Record *, s32, s32, s32, Tbl8008E194 *, s32, s32 *);

void object_ResolveFighterHits(void) {
    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;
    s32 i;
    s32 var_2;
    Obj80106A78 *obj;
    Vec3i32 *var_4 = scr->point_slots.unk00;
    Unk80101EC8Record *var_5;
    s32 var_6;
    u16 var_7;
    BoneHitRec *rec;
    s32 j;
    s32 var_10;
    s32 diff;
    s32 r;
    s32 kind;
    s32 flag;

    obj = g_object_pool;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->kind == -1)
            continue;
        if (obj->unk_04 == 0)
            continue;
        if (obj->is_moving == 0)
            continue;
        var_6 = obj->owner == 0;
        var_5 = &D_80101EC8[var_6];
        var_7 = var_5->move_kind;
        if (var_7 == 4 || var_7 == 0x14 || var_7 == 0xF || var_7 == 0x1C ||
            var_7 == 0x1D || var_7 == 0x1E || var_7 == 0x1F || var_7 == 0x20 ||
            var_7 == 0x21 || var_7 == 0x11) {
            continue;
        }

        var_4[0] = obj->prev_pos;
        var_4[1] = obj->pos;
        scr->shape_points[0] = &var_4[0];
        scr->shape_points[1] = &var_4[1];
        math_PrepareSegmentCollisionFrame(scr);

        var_10 = 0;
        rec = g_fighter_bone_hit_records[var_6];
        for (j = 0; j < 22; j++, rec++) {
            Unk80107850Rec *pos;
            if (var_5->unk_26C == 0 && j >= 6 && j <= 9)
                continue;
            pos = &SPAD->unkA8[var_6][j];
            var_10 = math_TestPreparedSegmentSphere(
                scr, pos, rec->unk_0C, rec->unk_0E);
            if (var_10 != 0) {
                var_2 = 0;
                if (rec->unk_00 != 0 && D_8008E194[obj->kind].unkD == 0) {
                    var_2 = math_TestPreparedSegmentSphere(
                                scr, pos, rec->unk_10, rec->unk_12) != 0;
                }
                break;
            }
        }
        if (var_10 == 0)
            continue;

        diff = (var_5->orientation.vy - ratan2(obj->vel.x, obj->vel.z)) & 0xFFF;
        if (diff >= 0x800)
            diff = 0x1000 - diff;
        math_NegNormalize3D(
            (s32 *)&obj->vel,
            /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached
             * by the first's address, passed to math_NegNormalize3D (Q96 /
             * Q117; as s16[3] or {x,y,z}, fighter_ResolveBoneHit scores 2) */
            &D_800A37E8);
        obj->pos.x -= obj->vel.x / 2;
        obj->pos.y -= obj->vel.y / 2;
        obj->pos.z -= obj->vel.z / 2;
        r = fighter_ResolveBoneHit(
            1, var_5, j, diff, var_2, &D_8008E194[obj->kind], 0, &flag);
        if (r == 2)
            continue;
        if (r != 0) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                (obj->kind ^ D_800A36F2[0]) != 0, 0x2B,
                &SPAD->unkA8[var_6][j].x, 0);
            object_AttachToOpponentBone(obj, rec->bone);
            obj->unk_04 = 0;
            var_7 = var_5->move_kind;
            if (var_7 == 8 || var_7 == 0x23) {
                g_gpu_jitter_burst_enabled = 1;
            }
            continue;
        }
        kind = obj->kind;
        if (kind == 0xF) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                var_6 ^ 1, 0xE, &SPAD->unkA8[var_6][j].x,
                /* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector reached
                 * by the first's address, passed to
                 * fight_RecordReplayEventAndDispatchSoundOrEffect (Q96 / Q117;
                 * as s16[3] or {x,y,z}, fighter_ResolveBoneHit scores 2) */
                &D_800A37E8);
            obj->kind = -1;
            continue;
        }
        if (kind == 0xE) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                (D_800A36F2[0] ^ 0xE) != 0, 0x2F, &obj->pos.x, 0);
        } else if (flag == 0) {
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                (kind ^ D_800A36F2[0]) != 0, 0x2B, &SPAD->unkA8[var_6][j].x, 0);
        }
        object_DeflectFromFighterHit(scr, obj, j);
        obj->unk_04 = 0;
    }
}

void fight_ClearBallisticRecords(void) {
    s32 i;
    for (i = 0; i < 4; i++) {
        D_80104E88[i].unk_00 = 0;
    }
}

Unk80104E88Rec *fight_CreateBallisticRecordAndDispatchEvent(
    Unk80101EC8Record *src, s32 type) {
    s32 speed = 0x50;
    /* FAKE: constant holder for -0xC8 (stored to unk_1C.y); the literal
     * reorders the li pair: score 7 (y store moved after .z: 8, before .x:
     * 10). */
    s32 var_1 = -0xC8;
    s32 i = 0;
    Unk80104E88Rec *ptr = D_80104E88;
    Unk80104E88Rec *s0;
    s16 var_5[3];

    for (; i < 4; i++) {
        s0 = ptr;
        if (s0->unk_00 == 0)
            break;
        ptr = s0 + 1;
    }
    if (i == 4)
        return 0;

    s0->unk_00 = type;
    s0->unk_01 = 1;
    s0->unk_02 = 0;
    s0->unk_03 = src->index;
    s0->unk_04.x = src->world_pos.x;
    {
        s32 v1 = src->size_scale;
        if (v1 < 0)
            v1 += 0x1F;
        s0->unk_04.y = src->collision_pos.vy - (v1 >> 5);
    }
    s0->unk_04.z = src->world_pos.z;
    s0->unk_1C.x = ((s32)Judge[src->orientation.vy & 0xFFF] * speed) >> 12;
    s0->unk_1C.y = var_1;
    s0->unk_1C.z =
        ((s32)Judge[(src->orientation.vy + 0x400) & 0xFFF] * speed) >> 12;
    s0->unk_10 = s0->unk_04;
    s0->unk_28 = src->collision_pos.vy;
    var_5[1] = src->orientation.vy;
    fight_RecordReplayEventAndDispatchSoundOrEffect(
        src->collision_hit_class, type == 1 ? 0xC : 0xD, &s0->unk_04.x, var_5);
    return s0;
}

void fight_UpdateBallisticRecords(void) {
    Unk1F8002B8_8005344C *scr = &SPAD->unk2B8.v8005344C;
    Unk80104E88Rec *base = D_80104E88;
    s32 i;

    i = 0;
    do {
        if (base->unk_00 != 0) {
            base->unk_02 += 1;
            base->unk_10 = base->unk_04;
            base->unk_1C.y += 0xD;
            scr->cast_end.x = base->unk_04.x + base->unk_1C.x;
            scr->cast_end.y = base->unk_04.y + base->unk_1C.y;
            scr->cast_end.z = base->unk_04.z + base->unk_1C.z;
            if (collision_TestSegmentExtendingShortStartWithWork(
                    &base->unk_04.x, &scr->cast_end.x, &scr->unk10.x,
                    scr->cast_normal, (s32)&scr->cast_work) != 0 ||
                base->unk_04.y > base->unk_28) {
                base->unk_00 = 0;
            } else {
                base->unk_04 = scr->cast_end;
            }
        }
        i++;
        base++;
    } while (i < 4);
}

void fight_TriggerOpposingFighterEventFromNearBallisticRecords(void) {
    Unk80104E88Rec *rec = D_80104E88;
    s32 i;
    for (i = 0; i < 4; i++, rec++) {
        Unk80101EC8Record *ent;
        u16 state;
        s32 dx, dy, dz;
        u32 dist_sq;
        u32 dist;

        if (rec->unk_00 == 0)
            continue;
        {
            /* the other player's index (unk_03 is the owner) */
            s32 other = (rec->unk_03 == 0);
            ent = &D_80101EC8[other];
        }
        state = ent->move_kind;
        if (state == 4 || state == 0x14 || state == 0xF || state == 0x1C ||
            state == 0x1D || state == 0x1E || state == 0x1F || state == 0x20 ||
            state == 0x21 || state == 0x11)
            continue;

        dx = ent->world_pos.x - rec->unk_04.x;
        dy = ent->world_pos.y - rec->unk_04.y;
        dz = ent->world_pos.z - rec->unk_04.z;
        dist_sq = dx * dx + dy * dy + dz * dz;
        if (dist_sq < 0x400) {
            dist = (u32)g_sqrt_table_u8[dist_sq] >> 3;
        } else {
            s32 clz = 0;
            s32 sp_tmp;
            if ((s32)dist_sq >= 0) {
                gte_Lzc(dist_sq, &sp_tmp);
                clz = sp_tmp;
            }
            {
                u32 shift = 0x16 - (clz & (u32)-2);
                dist = ((u32)g_sqrt_table_u8[dist_sq >> shift] << 16) >>
                       (0x13 - (shift >> 1));
            }
        }
        if (dist < (u32)(rec->unk_02 * 30 + 0x1F4)) {
            ent->pending_move_event = 5;
            rec->unk_00 = 0;
        }
    }
}

void fighter_ReadFrameMetadata(Unk80101EC8Record *rec) {
    u8 *ptr;
    u8 c;
    u8 val;

    ptr = rec->motion_metadata + 4;
    rec->weapon_hit_start_frames[0] = 0xFF;
    rec->weapon_hit_end_frames[0] = 0xFF;
    rec->weapon_hit_start_frames[1] = 0xFF;
    rec->weapon_hit_end_frames[1] = 0xFF;
    rec->paired_move_hit_frame = 0;
    rec->release_frame = 0;
    rec->object_pickup_frame = 0;
    rec->frame_window_start = 0;
    rec->frame_window_end = 0xFF;
    rec->opponent_transition_start_frame = 0xFF;
    rec->weapon_reload_frame = 0xFF;

    while ((c = *ptr++) != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr++;
            switch (c) {
            case 0x80:
                rec->weapon_hit_start_frames[0] = val;
                break;
            case 0x81:
                rec->weapon_hit_end_frames[0] = val;
                break;
            case 0x82:
                rec->release_frame = val;
                break;
            case 0x83:
                rec->object_pickup_frame = val;
                break;
            case 0x84:
                rec->opponent_turn_frame = val;
                break;
            case 0x85:
                rec->frame_window_start = val;
                break;
            case 0x86:
                rec->frame_window_end = val;
                break;
            case 0x87:
                rec->weapon_hit_start_frames[1] = val;
                break;
            case 0x88:
                rec->weapon_hit_end_frames[1] = val;
                break;
            case 0x89:
                rec->paired_move_hit_frame = val;
                break;
            case 0x8A:
                rec->opponent_transition_start_frame = val;
                break;
            case 0x8B:
                rec->weapon_reload_frame = val;
                break;
            }
        }
    }
}

/* 3D positional sound pan/volume: listener-relative delta of *arg1,
 * distance attenuation via the g_sqrt_table_u8 byte-LUT square root (gte_Lzc
 * for >= 0x400), then a sin/cos left/right pan scaled by distance, clamped to
 * 0x7F, sent to snd_QueueSound(arg0, R, L). */
void snd_QueuePositionalSound(s32 arg0, s32 *arg1) {
    s32 sp_tmp;
    s32 dx, dy, dz;
    u32 var_4;
    s32 distance_scale;
    s16 var_6;
    s32 var_7;
    u32 var_8;
    s32 var_9;
    s32 var_10;

    dx = g_listener_cam->w20 - arg1[0];
    dy = g_listener_cam->w24 - arg1[1];
    dz = g_listener_cam->w28 - arg1[2];

    if (((u32)(dx + 0x9C40) > 0x13880U) || ((u32)(dz + 0x9C40) > 0x13880U)) {
        var_4 = 0x9C40;
    } else {
        u32 dist_sq = (u32)((dx * dx) + (dy * dy) + (dz * dz));
        if (dist_sq < 0x400U) {
            var_4 = (u32)(u8)(g_sqrt_table_u8[dist_sq]) >> 3;
        } else {
            gte_Lzc(dist_sq, &sp_tmp);
            {
                /* Ruling 4 split; one expression: score 3. */
                u32 lzc_even = (u32)-2;
                u32 shift;
                u32 idx;
                u32 hi;
                lzc_even &= sp_tmp;
                shift = 0x16 - lzc_even;
                idx = dist_sq >> shift;
                /* Halve the table-index shift for the result scale. */
                shift = shift >> 1;
                hi = (u32)((u8)(g_sqrt_table_u8[idx]));
                var_4 = (hi << 16) >> (0x13 - shift);
            }
        }
    }

    distance_scale = (s32)((0x9C40 - var_4) << 11) / 32000;
    if (distance_scale < 0) {
        distance_scale = 0;
    }

    var_6 = g_listener_cam->unk_10.vy;
    var_7 = (s32)((dx * (s32)Judge[(var_6 + 0x400) & 0xFFF]) +
                  (dz * (s32)Judge[var_6 & 0xFFF])) >>
            12;
    var_8 = ~(u32)var_7;
    var_8 >>= 31;
    if (var_7 < 0) {
        var_7 = -var_7;
    }

    var_9 = (s32)((0x7530 - var_7) << 12) / 30000;
    var_10 = (s32)((0x2710 - var_7) << 12) / 10000;
    if (var_9 < 0)
        var_9 = 0;
    if (var_10 < 0)
        var_10 = 0;

    {
        s32 tmp = var_9;
        if (var_8 != 0) {
            var_9 = var_10;
            var_10 = tmp;
        }
    }

    var_9 = (s32)(distance_scale * var_9) >> 16;
    var_10 = (s32)(distance_scale * var_10) >> 16;
    if (var_9 >= 0x80)
        var_9 = 0x7F;
    if (var_10 >= 0x80)
        var_10 = 0x7F;

    snd_QueueSound(arg0, var_9, var_10);
}

void fight_RecordReplayEventAndDispatchSoundOrEffect(
    s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    s32 s0;
    s32 s4;

    if (g_fight_mode_selector == 3 && arg0 == 1 &&
        ((u32)(arg1 - 0x22) < 2 || (u32)(arg1 - 0x24) < 2 ||
         (u32)(arg1 - 0x26) < 2 || (u32)(arg1 - 0x28) < 2)) {
        s4 = 0;
        s0 = 0;
    } else {
        s0 = arg0 * 40;
        s4 = 0x28;
    }

    if (g_fight_mode_selector == 3 && arg0 == 1 && (u32)(arg1 - 0x2D) < 2) {
        s0 += D_8008EBCC[g_fight_step_row_column];
    }

    replay_RecordEvent(arg0, arg1, arg2, arg3);

    switch (arg1) {
    case 4:
        if (g_fight_mode_selector == 2 && D_800A389A == 0) {
            eff_StartRecycledCommandWithOrangeLight(arg2);
            break;
        }
        if (g_fight_mode_selector == 5) {
            eff_StartRecycledCommandWithOrangeLight(arg2);
            break;
        }
        eff_StartRecycledVectorCommandWithRedLight(arg2, arg3);
        break;
    case 3:
        eff_StartRecycledCommandWithRoseLight(arg2);
        break;
    case 2:
        eff_StartArgSelectedCommandWithCyanLight(arg2, arg0);
        break;
    case 15:
        eff_StartArgSelectedCommandWithGreenLight(arg2, arg0);
        break;
    case 1:
        eff_StartRecycledCommandWithPaleRedLight(arg2);
        break;
    case 5:
        snd_QueuePositionalSound(0x3C, arg2);
        snd_QueuePositionalSound(s4 + 0x3C, arg2);
        g_fight_midpoint_effect_delay =
            eff_StartCosineScaledQuadWithPaleBlueLight(arg2);
        break;
    case 6:
        snd_QueuePositionalSound(0x3C, arg2);
        snd_QueuePositionalSound(s4 + 0x3C, arg2);
        g_fight_midpoint_effect_delay =
            eff_StartSlowRateCosineScaledQuadWithPaleBlueLight(arg2);
        break;
    case 7:
        snd_QueuePositionalSound(0x3C, arg2);
        snd_QueuePositionalSound(s4 + 0x3C, arg2);
        g_fight_midpoint_effect_delay =
            eff_StartMediumRateCosineScaledQuadWithPaleBlueLight(arg2);
        break;
    case 8:
        snd_QueuePositionalSound(0x3C, arg2);
        snd_QueuePositionalSound(s4 + 0x3C, arg2);
        g_fight_midpoint_effect_delay =
            eff_StartFastRateCosineScaledQuadWithPaleBlueLight(arg2);
        break;
    case 9:
        eff_StartBillboardPairCommand(arg2);
        break;
    case 10:
        eff_StartJitteredBillboardsCommand(arg2);
        break;
    case 11:
        snd_QueuePositionalSound(s0 + 0x31, arg2);
        eff_StartRotatedQuadClusterWithNearWhiteLight(arg2, arg3);
        break;
    case 12:
        snd_QueuePositionalSound(0x78, arg2);
        eff_StartRecycledYawedParticlesWithTextureSelection(arg2, arg3[1]);
        break;
    case 13:
        snd_QueuePositionalSound(0x79, arg2);
        eff_StartRecycledYawedParticles(arg2, arg3[1]);
        break;
    case 14:
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight(
            arg2, arg3);
        break;
    case 17:
        snd_QueuePositionalSound(s0 + 0x39, arg2);
        break;
    case 18:
        eff_StartRecycledCommandWithMagentaLight(arg2);
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        break;
    case 33:
        snd_QueuePositionalSound(0x7A, arg2);
        break;
    case 34:
        snd_QueuePositionalSound(s0 + 0x35, arg2);
        break;
    case 35:
        snd_QueuePositionalSound(s0 + 0x35, arg2);
        break;
    case 36:
        snd_QueuePositionalSound(s0 + 0x34, arg2);
        break;
    case 37:
        snd_QueuePositionalSound(s0 + 0x3A, arg2);
        break;
    case 38:
        snd_QueuePositionalSound(s0 + 0x3B, arg2);
        break;
    case 39:
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        break;
    case 40:
        snd_QueuePositionalSound(s0 + 0x3E, arg2);
        break;
    case 41:
        snd_QueuePositionalSound(s0 + 0x40, arg2);
        break;
    case 42:
        snd_QueuePositionalSound(s0 + 0x21, arg2);
        break;
    case 43:
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        break;
    case 44:
        snd_QueuePositionalSound(s0 + 0x23, arg2);
        break;
    case 45:
        snd_QueuePositionalSound(s0 + 0x29, arg2);
        break;
    case 46:
        snd_QueuePositionalSound(s0 + 0x2B, arg2);
        break;
    case 47:
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        break;
    case 48:
        snd_QueuePositionalSound(s0 + 0x22, arg2);
        break;
    case 49:
        snd_QueuePositionalSound(s0 + 0x32, arg2);
        break;
    case 50:
        snd_QueuePositionalSound(s0 + 0x33, arg2);
        break;
    }
}

void fighter_DispatchFrameEvent(Unk80101EC8Record *obj, s32 kind) {
    s32 pos[3];
    s32 base;
    s32 base2;
    s32 row;
    s32 row2;
    s32 var_5;
    s32 var_6;

    if (g_fight_mode_selector == 3 && obj->index == 1 &&
        ((u32)(kind - 7) < 2 || (u32)(kind - 9) < 2 || (u32)(kind - 11) < 2 ||
         (u32)(kind - 13) < 2 || (u32)(kind - 15) < 2 || kind == 17)) {
        base2 = 0;
        base = 0;
    } else {
        base = obj->index * 40;
        base2 = obj->other->index * 40;
    }

    if (g_fight_mode_selector == 3 &&
        ((u32)(kind - 0x15) < 2 || (u32)(kind - 0x17) < 2 ||
         (u32)(kind - 0x19) < 2 || kind == 0x26 || (u32)(kind - 0x36) < 2 ||
         (u32)(kind - 0x38) < 2 || (u32)(kind - 0x3A) < 2 || kind == 0x47)) {
        if (obj->index == 1) {
            base += D_8008EBCC[g_fight_step_row_column];
        } else {
            base2 += D_8008EBCC[g_fight_step_row_column];
        }
    }

    row = base + obj->collision_hit_class * 4;
    var_5 = obj->collision_hit_class * 3;
    row2 = base2 + obj->other->collision_hit_class * 4;
    var_6 = obj->other->collision_hit_class * 3;

    switch (kind) {
    case 0:
        if (g_fight_stage_index == 11) {
            if (obj->ground_probe_floor_ys[0] - 600 <
                obj->ground_probe_points[0].y) {
                pos[0] = obj->ground_probe_points[0].x;
                pos[1] = obj->ground_probe_floor_ys[0];
                pos[2] = obj->ground_probe_points[0].z;
                eff_StartYawedQuadSlotCommand(
                    pos, obj->unk_1BA,
                    D_8008EBE0[D_8008E5A8[obj->character_roster_index]],
                    obj->index);
                if ((0x60 >> obj->collision_hit_tag) & 1) {
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        obj->index, 9, pos, 0);
                }
            }
        } else if (g_fight_stage_index == 14) {
            if (((0x60 >> obj->collision_hit_tag) & 1) &&
                obj->ground_probe_floor_ys[0] - 600 <
                    obj->ground_probe_points[0].y) {
                pos[0] = obj->ground_probe_points[0].x;
                pos[1] = obj->ground_probe_floor_ys[0];
                pos[2] = obj->ground_probe_points[0].z;
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    obj->index, 9, pos, 0);
            }
        }
        break;
    case 1:
        if (g_fight_stage_index == 11) {
            if (obj->ground_probe_floor_ys[1] - 600 <
                obj->ground_probe_points[1].y) {
                pos[0] = obj->ground_probe_points[1].x;
                pos[1] = obj->ground_probe_floor_ys[1];
                pos[2] = obj->ground_probe_points[1].z;
                eff_StartYawedQuadSlotCommand(
                    pos, obj->unk_1C2,
                    D_8008EBE0[D_8008E5A8[obj->character_roster_index]],
                    obj->index);
                if ((0x60 >> obj->collision_hit_tag) & 1) {
                    fight_RecordReplayEventAndDispatchSoundOrEffect(
                        obj->index, 9, pos, 0);
                }
            }
        } else if (g_fight_stage_index == 14) {
            if (((0x60 >> obj->collision_hit_tag) & 1) &&
                obj->ground_probe_floor_ys[1] - 600 <
                    obj->ground_probe_points[1].y) {
                pos[0] = obj->ground_probe_points[1].x;
                pos[1] = obj->ground_probe_floor_ys[1];
                pos[2] = obj->ground_probe_points[1].z;
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    obj->index, 9, pos, 0);
            }
        }
        break;
    case 2:
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            obj->index, 10, &obj->bone_hit_point_0.x, 0);
        break;
    case 3:
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            obj->index, 10, &obj->bone_hit_points_1_3_centroid.x, 0);
        break;
    case 4:
        fight_RecordReplayEventAndDispatchSoundOrEffect(
            obj->index, 10, &obj->bone_hit_points_4_5_midpoint.x, 0);
        break;
    case 7:
        snd_QueuePositionalSound(base + 0x31, &obj->world_pos.x);
        break;
    case 8:
        snd_QueuePositionalSound(base + 0x32, &obj->world_pos.x);
        break;
    case 9:
        snd_QueuePositionalSound(base + 0x33, &obj->world_pos.x);
        break;
    case 10:
        snd_QueuePositionalSound(base + 0x37, &obj->world_pos.x);
        break;
    case 11:
        snd_QueuePositionalSound(base + 0x38, &obj->world_pos.x);
        break;
    case 12:
        snd_QueuePositionalSound(base + 0x39, &obj->world_pos.x);
        break;
    case 13:
        snd_QueuePositionalSound(base + 0x3D, &obj->world_pos.x);
        break;
    case 14:
        snd_QueuePositionalSound(base + 0x3E, &obj->world_pos.x);
        break;
    case 15:
        snd_QueuePositionalSound(base + 0x3D, &obj->world_pos.x);
        break;
    case 16:
        snd_QueuePositionalSound(base + 0x3F, &obj->world_pos.x);
        break;
    case 17:
        snd_QueuePositionalSound(base + 0x40, &obj->world_pos.x);
        break;
    case 18:
        snd_QueuePositionalSound(base + 0x22, &obj->world_pos.x);
        break;
    case 19:
        snd_QueuePositionalSound(base + 0x23, &obj->world_pos.x);
        break;
    case 20:
        snd_QueuePositionalSound(base + 0x24, &obj->world_pos.x);
        break;
    case 21:
        snd_QueuePositionalSound(base + 0x25, &obj->world_pos.x);
        break;
    case 22:
        snd_QueuePositionalSound(base + 0x26, &obj->world_pos.x);
        break;
    case 23:
        snd_QueuePositionalSound(base + 0x27, &obj->world_pos.x);
        break;
    case 24:
        snd_QueuePositionalSound(base + 0x28, &obj->world_pos.x);
        break;
    case 25:
        snd_QueuePositionalSound(base + 0x29, &obj->world_pos.x);
        break;
    case 26:
        snd_QueuePositionalSound(base + 0x2A, &obj->world_pos.x);
        break;
    case 27:
        snd_QueuePositionalSound(row + 0x41, &obj->world_pos.x);
        break;
    case 28:
        snd_QueuePositionalSound(row + 0x42, &obj->world_pos.x);
        break;
    case 29:
        snd_QueuePositionalSound(row + 0x43, &obj->world_pos.x);
        break;
    case 30:
        snd_QueuePositionalSound(row + 0x44, &obj->world_pos.x);
        break;
    case 31:
        snd_QueuePositionalSound(var_5 + 0x71, &obj->world_pos.x);
        break;
    case 32:
        snd_QueuePositionalSound(var_5 + 0x72, &obj->world_pos.x);
        break;
    case 33:
        snd_QueuePositionalSound(var_5 + 0x73, &obj->world_pos.x);
        break;
    case 34:
        snd_QueuePositionalSound(0x77, &obj->world_pos.x);
        break;
    case 35:
        snd_QueuePositionalSound(0x7A, &obj->world_pos.x);
        break;
    case 36:
        snd_QueuePositionalSound(base + 0x3A, &obj->world_pos.x);
        break;
    case 37:
        snd_QueuePositionalSound(base + 0x36, &obj->world_pos.x);
        break;
    case 38:
        snd_QueuePositionalSound(base + 0x2B, &obj->world_pos.x);
        break;
    case 39:
        snd_QueuePositionalSound(base + 0x2C, &obj->world_pos.x);
        break;
    case 40:
        snd_QueuePositionalSound(base2 + 0x31, &obj->other->world_pos.x);
        break;
    case 41:
        snd_QueuePositionalSound(base2 + 0x32, &obj->other->world_pos.x);
        break;
    case 42:
        snd_QueuePositionalSound(base2 + 0x33, &obj->other->world_pos.x);
        break;
    case 43:
        snd_QueuePositionalSound(base2 + 0x37, &obj->other->world_pos.x);
        break;
    case 44:
        snd_QueuePositionalSound(base2 + 0x38, &obj->other->world_pos.x);
        break;
    case 45:
        snd_QueuePositionalSound(base2 + 0x39, &obj->other->world_pos.x);
        break;
    case 46:
        snd_QueuePositionalSound(base2 + 0x3D, &obj->other->world_pos.x);
        break;
    case 47:
        snd_QueuePositionalSound(base2 + 0x3E, &obj->other->world_pos.x);
        break;
    case 48:
        snd_QueuePositionalSound(base2 + 0x3D, &obj->other->world_pos.x);
        break;
    case 49:
        snd_QueuePositionalSound(base2 + 0x3F, &obj->other->world_pos.x);
        break;
    case 50:
        snd_QueuePositionalSound(base2 + 0x40, &obj->other->world_pos.x);
        break;
    case 51:
        snd_QueuePositionalSound(base2 + 0x22, &obj->other->world_pos.x);
        break;
    case 52:
        snd_QueuePositionalSound(base2 + 0x23, &obj->other->world_pos.x);
        break;
    case 53:
        snd_QueuePositionalSound(base2 + 0x24, &obj->other->world_pos.x);
        break;
    case 54:
        snd_QueuePositionalSound(base2 + 0x25, &obj->other->world_pos.x);
        break;
    case 55:
        snd_QueuePositionalSound(base2 + 0x26, &obj->other->world_pos.x);
        break;
    case 56:
        snd_QueuePositionalSound(base2 + 0x27, &obj->other->world_pos.x);
        break;
    case 57:
        snd_QueuePositionalSound(base2 + 0x28, &obj->other->world_pos.x);
        break;
    case 58:
        snd_QueuePositionalSound(base2 + 0x29, &obj->other->world_pos.x);
        break;
    case 59:
        snd_QueuePositionalSound(base2 + 0x2A, &obj->other->world_pos.x);
        break;
    case 60:
        snd_QueuePositionalSound(row2 + 0x41, &obj->other->world_pos.x);
        break;
    case 61:
        snd_QueuePositionalSound(row2 + 0x42, &obj->other->world_pos.x);
        break;
    case 62:
        snd_QueuePositionalSound(row2 + 0x43, &obj->other->world_pos.x);
        break;
    case 63:
        snd_QueuePositionalSound(row2 + 0x44, &obj->other->world_pos.x);
        break;
    case 64:
        snd_QueuePositionalSound(var_6 + 0x71, &obj->other->world_pos.x);
        break;
    case 65:
        snd_QueuePositionalSound(var_6 + 0x72, &obj->other->world_pos.x);
        break;
    case 66:
        snd_QueuePositionalSound(var_6 + 0x73, &obj->other->world_pos.x);
        break;
    case 67:
        snd_QueuePositionalSound(0x77, &obj->other->world_pos.x);
        break;
    case 68:
        snd_QueuePositionalSound(0x7A, &obj->other->world_pos.x);
        break;
    case 69:
        snd_QueuePositionalSound(base2 + 0x3A, &obj->other->world_pos.x);
        break;
    case 70:
        snd_QueuePositionalSound(base2 + 0x36, &obj->other->world_pos.x);
        break;
    case 71:
        snd_QueuePositionalSound(base2 + 0x2B, &obj->other->world_pos.x);
        break;
    case 72:
        snd_QueuePositionalSound(base2 + 0x2C, &obj->other->world_pos.x);
        break;
    }
}

/* Tail word after fighter_DispatchFrameEvent's 73-entry compiler-generated
 * switch table. */
const u32 D_800107BC[1] = {0x00000000};

void fighter_ProcessFrameEvents(Unk80101EC8Record *base) {
    u8 *s0 = base->motion_metadata + 5;
    u8 command = s0[-1];
    while (command != 0) {
        if (command == 0xFF) {
            u32 bits = s0[0] | (s0[1] << 8) | (s0[2] << 16) | (s0[3] << 24);
            if (bits & (1 << base->character_data_index)) {
                s0 += 4;
            } else {
                s0 += 6;
            }
        } else if (command < 0x80) {
            u8 val = *s0++;
            s16 var_11 = base->move_frame;
            if (val == var_11) {
                fighter_DispatchFrameEvent(base, command - 1);
            } else if (var_11 < val) {
                return;
            }
        } else {
            s0++;
        }
        command = *s0++;
    }
}

s32 snd_GetStageSoundIndex(void) {
    s16 idx = g_fight_stage_index - 2;
    switch (idx) {
    case 0:
        return 0;
    case 2:
        return 1;
    case 5:
        return 2;
    case 6:
        return 3;
    case 16:
        return 4;
    case 22:
        return 5;
    default:
        return 0xFF;
    }
}

void snd_ResetStageSoundState(void) {
    s32 i = 3;
    s16 *p1 = &g_snd_stage_sound_state[3];
    do {
        *p1 = 0;
        i--;
        p1--;
    } while (i >= 0);
    i = 5;
    {
        u8 *p2 = &g_snd_sequence_ticks[5];
        do {
            *p2 = 0;
            i--;
            p2--;
        } while (i >= 0);
    }
}

void snd_QueuePositionSequence(Unk80107850Rec *arg0) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (g_snd_sequence_ticks[i] == 0) {
            break;
        }
    }
    if (i == 6) {
        return;
    }
    g_snd_sequence_ticks[i] = 1;
    g_snd_sequence_positions[i] = *arg0;
}

void snd_UpdateStageSounds(void) {
    s32 i;
    u8 *tbl = D_8008E914[g_fight_stage_index];

    for (i = 0; i < 6; i++) {
        if (g_snd_sequence_ticks[i] != 0) {
            s32 var_2 = snd_GetStageSoundIndex();
            u8 cur = g_snd_sequence_ticks[i];

            if (cur == 1) {
                snd_QueuePositionalSound(
                    D_8008EBF4[var_2], &g_snd_sequence_positions[i].x);
            } else if (cur == D_8008EBFC[var_2].a) {
                snd_QueuePositionalSound(
                    D_8008EBF4[var_2] + 1, &g_snd_sequence_positions[i].x);
            } else if (cur == D_8008EBFC[var_2].b) {
                snd_QueuePositionalSound(
                    D_8008EBF4[var_2] + 2, &g_snd_sequence_positions[i].x);
            }

            {
                s32 val = g_snd_sequence_ticks[i] + 1;
                g_snd_sequence_ticks[i] = val;
                if ((u32)D_8008EBFC[var_2].b < (u32)(val & 0xFF)) {
                    g_snd_sequence_ticks[i] = 0;
                }
            }
        }
    }

    if (snd_ShouldUpdateStageSoundSlots() != 0) {
        s16 *buf = g_snd_stage_sound_state;

        for (i = 0; i < 4; i++, tbl += 2) {
            u8 *data = tbl + 1;
            s32 var_7;
            s32 type;
            s32 var_9;

            var_7 = rng_Next();
            var_9 = var_7 & 0x3FF;
            type = *tbl;

            if (type == 1)
                goto handle_type_1;
            if (type < 2)
                goto skip;
            if (type < 7)
                goto handle_type_2_6;
            continue;

        handle_type_1:
            if (buf[i] != 0)
                goto skip;
            buf[i] = 1;
            goto call_default;

        handle_type_2_6: {
            s32 var_10 = type - 2;
            s32 count;
            s32 scount;
            s32 thresh;
            s32 limit;

            count = (u16)buf[i] + 1;
            buf[i] = count;
            scount = (s16)count;
            thresh = D_8008EA44[var_10].a;
            limit = thresh * 30;

            if (limit < scount) {
                s32 ratio =
                    ((scount - limit) << 10) / (D_8008EA44[var_10].b * 30);

                if (var_9 < ratio) {
                    buf[i] = 0;
                    type = *tbl;
                    if ((u32)(type - 5) < 2) {
                        snd_QueuePositionalSound(
                            0x7B + i, D_8008EA50[type - 5]);
                        continue;
                    }
                    goto call_default;
                }
            }
            goto skip;
        }

        call_default: {
            u8 val = *data;
            snd_QueueSound(0x7B + i, val, val);
        }
        skip:;
        }
    }
}

void ResetGraphClearFrameCounterAndSetMainLoopHandler3(void) {
    gpu_ResetGraphMode1();
    g_handler_frame_counter = 0;
    g_main_loop_handler_index = 3;
}

void fight_BuildOpponentOrder(void) {
    s32 sp[2];
    s32 count;
    s32 i;
    s32 idx1;
    u8 *ptr;
    s32 var_5;

    {
        s32 mask =
            ~(1 << g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[0]]) &
            0x3EF3DF;
        s32 bits;
        count = 0;
        i = 0;
        bits = g_saved_record.unk_00 & mask;
        /* FAKE: `var_5` carries the constant 1 of `1 << i`; with the literal
           the test becomes `(bits >> i) & 1` (srav/andi instead of the
           target's `li a2,1` + sllv/and), score 4. */
        var_5 = 1;
        ptr = g_fight_opponent_order;
        do {
            if (bits & (var_5 << i)) {
                *ptr = i;
                ptr++;
                count++;
            }
            i++;
        } while (i < 0x1B);
    }

    i = 0;
    do {
        i++;
        idx1 = rand() % count;
        {
            s32 idx2 = rand() % count;
            s32 tmp = g_fight_opponent_order[idx1];
            g_fight_opponent_order[idx1] = g_fight_opponent_order[idx2];
            g_fight_opponent_order[idx2] = tmp;
        }
    } while (i < 0x6C);

    if (g_saved_record.unk_00 & 0x10020) {
        s32 v1 = g_saved_record.unk_00 & 0x20;
        s32 v0 = g_saved_record.unk_00 & 0x10000;
        sp[0] = v1;
        sp[1] = v0;
        if (v1 == 0) {
            sp[0] = v0;
            sp[1] = 0;
            goto block_12;
        }
        if (v0 != 0) {
            if (rand() & 1) {
                s32 t1 = sp[1];
                s32 t2 = sp[0];
                sp[0] = t1;
                sp[1] = t2;
            }
        }
    block_12:
        if (count >= 0xB) {
            i = count;
            do {
                g_fight_opponent_order[i] = g_fight_opponent_order[i - 1];
                i--;
            } while (i >= 0xB);
            {
                s32 dir = 0x10;
                if (sp[0] & 0x20) {
                    dir = 5;
                }
                g_fight_opponent_order[10] = dir;
            }
        } else {
            s32 dir = 0x10;
            if (sp[0] & 0x20) {
                dir = 5;
            }
            g_fight_opponent_order[count] = dir;
        }
        count++;
        if (sp[1] != 0) {
            u8 *a1 = &g_fight_opponent_order[count];
            count++;
            {
                s32 dir = 0x10;
                if (sp[1] & 0x20) {
                    dir = 5;
                }
                *a1 = dir;
            }
        }
    }
    {
        u8 lookup = g_fighter_group_flags
            [g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[0]]];
        s32 val;
        if (lookup != 0) {
            if (g_saved_record.unk_00 & 0x04000000) {
                val = 0x1A;
                goto append_last;
            }
        } else {
            if (g_saved_record.unk_00 & 0x01000000) {
                val = 0x18;
                goto append_last;
            }
        }
        goto set_count;
    append_last:
        g_fight_opponent_order[count] = val;
        count++;
    }
set_count:
    D_800A391F = count;
    g_fight_opponent_order_cursor = 0;
    D_800A37BC = 0;
}

void fight_AdvanceOpponentOrder(void) {
    u8 a0 = g_fight_opponent_order_cursor;
    u8 b = D_800A391F;

    if (a0 == b) {
        g_local_ot_draw_mode = DISP_DISABLED;
        g_gpu_jitter_burst_enabled = 0;
        if (a0 == 0x14) {
            u8 z = g_fighter_group_flags[D_80101EC8[0].character_data_index];
            s32 val = 2;
            if (z != 0)
                val = 3;
            g_fight_presentation_index = val;
            g_main_loop_handler_index = 0x12;
        } else {
            g_main_loop_handler_index = 0x20;
        }
    } else {
        u8 a1;
        D_800A376B = 0;
        g_fight_opponent_order_cursor = a0 + 1;
        a1 = g_fight_opponent_order[a0];
        g_fight_config.unk_4[1] = (&D_8008D55C)[a1];
        if (a1 == 5) {
            g_fight_config.unk_4[3] = 6;
        } else if (a1 == 0x10) {
            g_fight_config.unk_4[3] = 7;
        } else {
            u8 x = D_800A37BC;
            s8 y;
            D_800A37BC = x + 1;
            y = (&D_8008E748)[x];
            g_fight_config.unk_4[3] = y;
            if (y == 4) {
                u8 v = g_fighter_group_flags[a1];
                if (v != 0) {
                    g_fight_config.unk_4[3] = 5;
                }
            }
        }
        g_fight_config.unk_0[1] = 0x800;
        {
            s16 v = (&D_8008E75C)[a1];
            g_main_loop_handler_index = 0;
            g_fight_stage_index = v;
        }
    }
}

void fight_InsertBestTime(void) {
    /* Pointer read-modify-write on a global (pointer-alias-fake-exception):
     * the best-time table is searched, shifted down one slot and the new
     * time stored, all through record; written directly, each
     * g_saved_record.times[] access is its own lui/addu/%lo: score 34. */
    /* SOTN: src/dra/4CE2C.c:63 @db41b28 */
    FileRecord *record = &g_saved_record;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (record->times[j].unk_4 < g_fight_timer_frames) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            record->times[k] = record->times[k - 1];
        }
        record->times[n].unk_0 = (u8)D_80101EC8[0].character_data_index;
        record->times[n].unk_1 = (u8)D_80101EC8[0].weapon_id;
        record->times[n].unk_4 = g_fight_timer_frames;
    }
}

s32 fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags(void) {
    u8 state;
    s32 tableIndex;

    state = g_fight_step_table_next_row;
    tableIndex = state & 0xFF;
    if (tableIndex == 0x64) {
        D_800A36F0 = 0;
        D_800A3781 = 0;

        if (g_fight_timer_frames < 0x6979) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x20;
            if (g_fighter_group_flags[g_fight_config_id_data_rows
                                          [(s8)g_fight_config.unk_4[0]]] != 0) {
                mask = 0x10000;
            }
            flags = &g_saved_record.unk_00;
            word = *flags;
            D_800A36F0 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        if (g_fight_menu_restart_count == 0) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x1000000;
            if (g_fighter_group_flags[g_fight_config_id_data_rows
                                          [(s8)g_fight_config.unk_4[0]]] != 0) {
                mask = 0x4000000;
            }
            flags = &g_saved_record.unk_00;
            word = *flags;
            D_800A3781 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        fight_InsertBestTime();
        g_main_loop_handler_index = 4;
        return 0;
    } else {
        u8 *table = D_8008E778[tableIndex];
        u8(*ranks)[5] = D_8008EC24;
        u8(*moves)[5] = D_8008E908;
        s32 entry;
        s32 row;

        g_fight_step_table_next_row = state + 1;
        D_800A376B = 0;
        entry = table[0];
        g_fight_step_row_column = entry;
        row = g_fighter_group_flags
                  [g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[0]]] ==
              0;
        entry &= 0xFF;
        D_800A38DE = ranks[row][entry];
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        g_fight_config.unk_4[1] = moves[row][entry];
        return 1;
    }
}

void fight_SetSelectorAndMainLoopHandlerFromFlagsAndBestTimeRank(void) {
    s32 v1;
    if (D_800A36F0 != 0) {
        v1 = 6;
        if (g_fighter_group_flags[D_80101EC8[0].character_data_index] != 0) {
            v1 = 7;
        }
        g_fight_presentation_index = v1;
        g_main_loop_handler_index = 0x12;
        return;
    }
    if (D_800A3781 != 0) {
        v1 = 8;
        if (g_fighter_group_flags[D_80101EC8[0].character_data_index] != 0) {
            v1 = 9;
        }
        g_fight_presentation_index = v1;
        g_main_loop_handler_index = 0x12;
        return;
    }
    if (D_800A38E9 < 3) {
        g_main_loop_handler_index = 0x1A;
    } else {
        g_main_loop_handler_index = 8;
    }
}

void fight_RecordMatchResult(void) {
    u8 p1, p2, match_result_index;

    p1 = D_800A3898;
    if ((u8)p1 == g_fight_win_target) {
        match_result_index = g_fight_match_cursor;
        g_fight_match_results[match_result_index] = 0;
    } else {
        p2 = D_800A3899;
        if ((u8)p2 == g_fight_win_target) {
            match_result_index = g_fight_match_cursor;
            g_fight_match_results[match_result_index] = 1;
        } else if ((u8)p2 < (u8)p1) {
            match_result_index = g_fight_match_cursor;
            g_fight_match_results[match_result_index] = 0;
        } else if ((u8)p1 < (u8)p2) {
            match_result_index = g_fight_match_cursor;
            g_fight_match_results[match_result_index] = 1;
        } else {
            if ((u8)D_800A38AA < (u8)D_800A38AB) {
                D_800A3898 = p1 + 1;
                match_result_index = g_fight_match_cursor;
                g_fight_match_results[match_result_index] = 0;
            } else if ((u8)D_800A38AB < (u8)D_800A38AA) {
                D_800A3899 = p2 + 1;
                match_result_index = g_fight_match_cursor;
                g_fight_match_results[match_result_index] = 1;
            } else {
                match_result_index = g_fight_match_cursor;
                g_fight_match_results[match_result_index] = 2;
            }
        }
    }
    g_fight_match_scores[g_fight_match_cursor][0] = D_800A3898;
    g_fight_match_scores[g_fight_match_cursor][1] = D_800A3899;
    g_fight_match_cursor = g_fight_match_cursor + 1;
}

void fight_PackMatchScores(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    u8 n;
    s32 recorded_match_count;
    u8 *base;

    g_local_ot_draw_mode = DISP_LOADING;
    n = g_fight_match_count;
    if (i < n) {
        recorded_match_count = g_fight_match_cursor;
        do {
            s32 var_6;
            s32 j;
            base = g_fight_match_scores[i];
            var_6 = (i < recorded_match_count);
            j = 0;
            do {
                if (var_6) {
                    acc |= base[j] << shift;
                } else {
                    acc |= 3 << shift;
                }
                shift += 2;
                j++;
            } while (j < 2);
            i++;
            n = g_fight_match_count;
        } while (i < n);
    }

    D_800A3784 = acc;
}

extern void fight_PackMatchScores(void);

void fight_AdvanceMatch(void) {
    fight_PackMatchScores();
    if (g_fight_match_cursor == g_fight_match_count) {
        g_local_ot_draw_mode = DISP_DISABLED;
        g_gpu_jitter_burst_enabled = 0;
        g_main_loop_handler_index = 0x14;
    } else {
        s32 v1;
        D_800A38F4 = 0;
        D_800A3899 = 0;
        D_800A3898 = 0;
        D_800A38AB = 0;
        D_800A38AA = 0;
        g_fight_reinit_phase_timer = 0;
        g_fight_sound_cue_countdown = 0x50;
        g_fight_time_up_phase_timer = 0;
        D_800A37E1 = 0;
        g_fight_match_result_timer = 0;
        g_fight_mark_tally_phase_timer = 0;
        g_fight_point_award_phase_timer = 0;
        for (v1 = 0; v1 < 2; v1++) {
            g_fight_config.unk_4[v1] = D_801027A0[g_fight_match_cursor][v1];
            g_fight_config.unk_4[2 + v1] = D_801027D8[g_fight_match_cursor][v1];
        }
        gpu_HideResetAndClear();
        cdrom_PauseAndWait();
        g_main_loop_handler_index = 0;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 D_800A3140 = 0;
/* address taken (lui/addiu) in fighter_ResolveBodyOverlap: size from the blob
 * label */
s32 D_800A3144[2] = {0x15e, 0x172};
s32 D_800A314C = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 D_800A36F2[2];
