/* One game function, fight_UpdatePairedInputTotalsEventsAndMidpointEffects.
 * .text 0x80026DA4 (ROM 0x175A4). Start boundary: LEGACY (a tooling split, no
 * evidence either way). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* Updates the two active player records in practice modes. The contest
 * chooses a winner, the distance/input checks choose follow-up states, and
 * the tail emits a mode-specific effect at the players' midpoint.
 *
 * FAKE: leave the void callee fighter_TryDropQueuedHeadAtBone undeclared, as in
 * the matched SOTN callers below. Declared truthfully (void, Unk80101EC8Record
 * *) it scores 7: the s0 save and the first record-base load move ahead of the
 * fighter_CalcSeparationErrorAndOpposedOffsets call (8 words differ in the
 * linked EXE).
 * SOTN: src/main/psxsdk/libsnd/ssclose.c:8 and
 * src/main/psxsdk/libsnd/vmanager.c:1107
 * @db41b28eee52969244a52cc269c8163d1ed8826a */
void fight_UpdatePairedInputTotalsEventsAndMidpointEffects(void) {
    Unk80101EC8Record *record;
    Unk80101EC8Record *var_1;
    Unk80101EC8Record *current;
    s32 separation_error;
    s32 i;
    s32 idx;
    s32 var_6;
    s32 kind;
    s32 pos[3];

    separation_error = fighter_CalcSeparationErrorAndOpposedOffsets();
    record = D_80101EC8;
    g_fight_weapon_contact_result = -1;
    if ((u16)D_80101EC8[0].move_kind == 0x1C) {
        if (D_80101EC8[0].move_frame != 4)
            goto tail;
        idx = D_800A3876;
        if (idx == -1)
            goto tail;
        /* `record` holds the selected record and `var_1` its partner, then
           both return to the fixed pair, the way SOTN's tempEntity moves
           between entities and returns to entity zero at the join (Q51) */
        /* SOTN: src/st/lib/unk_3B53C.c:41-55
         * @db41b28eee52969244a52cc269c8163d1ed8826a */
        record = &D_80101EC8[idx];
        var_1 = D_80101EC8;
        if (idx == 0) {
            var_1++;
        }
        record->pending_move_event = 3;
        var_1->pending_move_event = 4;
    } else if ((u16)D_80101EC8[0].move_kind != 0xF) {
        for (i = 0; i < 2; i++) {
            current = record + i;
            if (current->pad_input.pressed & 0x20) {
                current->mash_contest_score += current->power;
            }
            if (current->pad_input.pressed & 0x40) {
                current->mash_contest_score += current->power;
            }
            if ((u16)current->move_kind == 0x1D ||
                (u16)current->move_kind == 0x1E ||
                (u16)current->move_kind == 0x20) {
                if (current->pad_input.held & 0x1000) {
                    var_6 = 1;
                } else if (current->pad_input.held & 0x4000) {
                    var_6 = -1;
                } else {
                    var_6 = 0;
                }
                current->decaying_xz_velocity.vx -=
                    (Judge[(current->orientation.vy + 0x400) & 0xFFF] * var_6) /
                    256;
                current->decaying_xz_velocity.vz +=
                    (Judge[(u16)current->orientation.vy & 0xFFF] * var_6) / 256;
            }
        }
        /* FAKE: re-sets record to the base it already holds after the drift
         * loop (a same-value re-store, shaped like SOTN's prim re-assignment);
         * without it: score 61 */
        /* SOTN: src/weapon/w_011.c:343-348
         * @db41b28eee52969244a52cc269c8163d1ed8826a */
        record = D_80101EC8;
        var_1 = record + 1;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (D_80101EC8[0].mash_contest_score >
                D_80101EC8[1].mash_contest_score) {
                D_80101EC8[0].pending_move_event = 3;
                D_80101EC8[1].pending_move_event = 4;
                if ((u16)D_80101EC8[1].move_kind == 0x21) {
                    fighter_TryDropQueuedHeadAtBone(var_1);
                }
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    1, 0x2D, &var_1->world_pos.x, (s16 *)0);
            } else if (D_80101EC8[1].mash_contest_score >
                       D_80101EC8[0].mash_contest_score) {
                D_80101EC8[1].pending_move_event = 3;
                D_80101EC8[0].pending_move_event = 4;
                if ((u16)D_80101EC8[0].move_kind == 0x21) {
                    fighter_TryDropQueuedHeadAtBone(record);
                }
                fight_RecordReplayEventAndDispatchSoundOrEffect(
                    0, 0x2D, &record->world_pos.x, (s16 *)0);
            }
            var_1->mash_contest_score = 0;
            record->mash_contest_score = 0;
            D_800A389C = 0;
        }
        if (separation_error > 200) {
            record->pending_move_event = 2;
            var_1->pending_move_event = 2;
        } else {
            s32 diff = record->world_pos.y - var_1->world_pos.y;
            if (diff < 0)
                diff = -diff;
            if (diff >= 1000) {
                record->pending_move_event = 2;
                var_1->pending_move_event = 2;
            }
        }
        if ((u16)record->move_kind == 0x1D) {
            if (record->pad_input.held & 0x8000) {
                if (var_1->pad_input.held & 0x8000) {
                    record->pending_move_event = 2;
                    var_1->pending_move_event = 2;
                } else {
                    record->pending_move_event = 2;
                    var_1->pending_move_event = 5;
                }
            } else if (var_1->pad_input.held & 0x8000) {
                record->pending_move_event = 5;
                var_1->pending_move_event = 2;
            }
        }
    }
    /* FAKE: restores record zero at the three-path join after the
       selected-pair work, completing the pointer reuse above; reverted
       together with that reuse: score 18 (Q129) */
    /* SOTN: src/st/lib/unk_3B53C.c:41-55
     * @db41b28eee52969244a52cc269c8163d1ed8826a */
    record = D_80101EC8;
tail:
    /* FAKE: sets the second fixed record from `record` at this join (a
       restore after the selected pair, the first write on the goto-tail
       paths); D_80101EC8[1] direct in the tail: score 7 (Q129) */
    /* SOTN: src/st/lib/unk_3B53C.c:41-55
     * @db41b28eee52969244a52cc269c8163d1ed8826a */
    var_1 = record + 1;
    if (g_fight_midpoint_effect_delay == 0) {
        switch ((u16)D_80101EC8[0].move_kind) {
        case 0xF:
            kind = -1;
            break;
        case 0x1C:
            kind = 0;
            break;
        case 0x21:
            kind = 1;
            break;
        case 0x20:
            kind = 2;
            break;
        case 0x1D:
            kind = 3;
            break;
        case 0x1E:
            kind = 4;
            break;
        case 0x1F:
            kind = 5;
            break;
        }
        if (kind >= 0) {
            pos[0] = (record->world_pos.x + var_1->world_pos.x) / 2;
            pos[1] = (record->base_pos.y + var_1->base_pos.y) / 2;
            pos[2] = (record->world_pos.z + var_1->world_pos.z) / 2;
            pos[0] += (Judge[(u16)record->opponent_bearing & 0xFFF] *
                       g_fight_midpoint_effect_offsets[kind].unk0) >>
                      12;
            pos[1] += g_fight_midpoint_effect_offsets[kind].unk2;
            pos[2] += (Judge[(record->opponent_bearing + 0x400) & 0xFFF] *
                       g_fight_midpoint_effect_offsets[kind].unk0) >>
                      12;
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                0, D_8008EB6C[kind], pos, (s16 *)0);
        }
    } else {
        g_fight_midpoint_effect_delay--;
    }
}
