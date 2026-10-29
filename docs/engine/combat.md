# Combat: contact tests and hit reactions

The two fighters are `Unk80101EC8Record`s (0x44C bytes, include/game.h:1647)
in `D_80101EC8[]` (include/bb2.h:153); each record's `other` points at the
opponent. Most of the code below is in src/main/17AFC.c. Field names are the
header's `unk_NN` names; where a field's role is shown by the code it is
stated, otherwise only its use is described.

## Per-frame entry: `fight_UpdateFighterInteractionsAndPointCaches`

`fight_UpdateFighterInteractionsAndPointCaches` (src/main/17AFC.c:2391) runs once per frame; its callers are
src/main/9F9C.c:2207, :2256 and :2381, each followed by `object_UpdatePool` (the
object-pool update, src/main/17AFC.c:4029). It branches on fighter 0's state
`move_kind`:

- 0xF, 0x1C..0x21: `fight_UpdatePairedInputTotalsEventsAndMidpointEffects` (src/main/175A4.c:23) only.
- 0x11: `fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents` (src/main/17AFC.c:2253) only.
- otherwise, in order:
  1. `fighter_TriggerOpponentResponseOnMoveFrame` (src/main/17AFC.c:807): per-fighter checks of
     `character_roster_index == 0x1B` records against the opponent's state.
  2. `fighter_ResolveBodyOverlap` (src/main/17AFC.c:876): the body-overlap push (its
     comment at :766): two points per fighter tested pairwise against the
     radii `g_fighter_body_overlap_radii[0..1]`, an overlap pushing both apart along x/z
     through `decaying_xz_velocity`.
  3. `g_fight_weapon_contact_result = fighter_ProcessWeaponContacts()` (src/main/17AFC.c:1115): the blade
     contact test between the two fighters (comment at :1065); it returns
     `(fighter-1 grid << 1) | fighter-0 grid` on contact, else -1.
  4. On contact, `fighter_BlendCurrentAndPreviousContactPoints` (src/main/17AFC.c:2313), then
     `fighter_ResolveWeaponContact` (src/main/17AFC.c:634) for each fighter when either has
     `attack_hit_armed` set; otherwise `fighter_ProcessWeaponBodyHits(0)` (below).
  5. When both fighters have `update_count >= 3`, `D_800A38A8` is set, both
     `pending_move_event` are -1, neither `character_roster_index` is 0x1F and their heights `world_pos.y`
     differ by less than 1000, both get `pending_move_event = 0xA`.
- Then for every state: a fighter whose `move_kind` is 5 ends the round
  (`g_main_loop_handler_index = 0x1C`, with `g_fight_followup_fighter_index` the other fighter's index); the
  scratchpad body points are copied into `prev_weapon_points` / `prev_second_weapon_points`, `bone_hit_points_1_3_centroid`
  (the centroid of points 1..3) and `bone_hit_points_4_5_midpoint` (the midpoint of points 4..5)
  (src/main/17AFC.c:2290-2316); and while fighter 0's `pending_move_event` is -1,
  `object_ResolveFighterHits` (src/main/17AFC.c:4495) tests the pool objects against the
  fighters, followed by `fight_TriggerOpposingFighterEventFromNearBallisticRecords` (src/main/17AFC.c:4695) when that
  left `pending_move_event` unchanged.

`pending_move_event` is the reaction code these paths write; -1 means none pending.

## Blade against body: `fighter_ProcessWeaponBodyHits`

`fighter_ProcessWeaponBodyHits(mode)` (src/main/17AFC.c:1701) runs for each fighter `i` as the
target, with the opponent as the attacker:

1. It decides how many blade quads to test (`var_10`): one when the
   attacker's `is_defeated == 0`, `weapon_live_state != 0`, `update_count != 0` and `character_roster_index != 0x1F`,
   one more with `wielded_object_live_state` set, and one more when `weapon_id` is 4 or 5 and any is
   tested. (With `mode == 1` the fighter `g_fight_paired_move_initiator` is skipped as a target.)
   Attackers with `weapon_id` 6 or 7, in states 2 / 0x1B / 0x28 / 0x26 at frame
   `move_frame == weapon_hit_start_frames[0]`, also sweep a segment with `fighter_CastWeaponSegment`
   (src/main/17AFC.c:1461, comment at :1399) while the counter `weapon_ammo`
   lasts (once it is spent they play cue 0x32 and test no quads); `character_roster_index`
   0x1D / 0xE (with `bone_group_6_9_intact` set) do the same with the second blade
   (`weapon_hit_start_frames[1]`).
2. Each blade quad (this frame's points from the scratchpad and last frame's
   from `prev_weapon_points` / `prev_second_weapon_points`) is split into 2 or 4 triangles (4 when the tip
   moved more than 2500 units) and each triangle goes through `math_PrepareTriangleCollisionFrame`
   (src/main/17AFC.c:2415) and `fighter_TestTriangleBoneHits` (src/main/17AFC.c:2504), which
   leave bit masks of struck limb records in the scratch record.
3. Of the struck records among the target's 22 limb records (`g_fighter_bone_hit_records[i]`)
   it picks the one nearest the attacker's point `prev_weapon_points[1]`
   (src/main/17AFC.c:1844-1860). With `mode == 1` it only plays cue 4 there
   and returns.
4. Unless the struck limb is in the sweep / second-blade mask and the
   attacker has `weapon_id` 6 or 7 or `character_roster_index` 0x1D / 0xE, or the attacker is in an attack
   state (2, 0x1B, 0x28, 0x26) within three frames before a window start, it
   pushes the target along x/z through
   `decaying_xz_velocity` when the facing difference is under 0x400.
5. If the attacker's `weapon_hit_window_open` is set and (with `wielded_object_live_state`) its frame `move_frame`
   is inside the window `weapon_hit_start_frames[k]..weapon_hit_end_frames[k]`, it calls
   `fighter_ResolveBoneHit(0, target, limb, facing difference, ...)`.

## Hit reaction: `fighter_ResolveBoneHit`

`fighter_ResolveBoneHit(src, rec, limb, thresh, flag, tbl, arg6, out)`
(src/main/17AFC.c:375) is called with `src == 0` from `fighter_ProcessWeaponBodyHits` and with
`src == 1` and a `Tbl8008E194` record from `object_ResolveFighterHits` (the comment at
src/main/17AFC.c:2146 records both call shapes). Its branches, in order:

- `rec->character_roster_index == 0x1C`: when the limb is not 1..3, or the dot product of
  `bone_y_axis` with the vector `D_800A37E8` is under 0x400, it plays cue 0x12,
  sets `pending_move_event = 0x1E` (and, with `src == 0`, the attacker's `pending_move_event` to 0xB
  or 0x19) and returns 0.
- `rec->character_roster_index == 0xD` (outside state 0x2E): `fighter_RelocateAndStartEventMove`, see
  [ai.md](ai.md).
- `move_kind` 4 or 0x14, or `character_roster_index == 0x1F`: `fighter_StoreBoneLocalVectorAndArmEventTrigger`
  (src/main/9F9C.c:2996) and `fighter_DispatchBoneHitSoundEvent` (src/main/17AFC.c:262); the
  latter case also sets `pending_move_event = 7`.
- The target in an attack state (2, 0x1B, 0x28, 0x26) with `move_frame < attack_early_frame_limit`:
  with `src == 0` the reaction code
  comes from `g_fighter_power_difference_move_events[math_ClassifyAbsMagnitude(power_difference)][power_difference_negative][same]`, where `power_difference` is
  the difference of the two `power` values and `same` whether their `unk_AF`
  match; codes 0..2 play cue pairs (0x25 / 0x26 / 0x2D), and code 2 also runs
  `fighter_TryDropQueuedHeadAtBone`. With `src == 1` the table record's `unkC` picks code 0
  or 1.
- Otherwise `fighter_DispatchBoneHitSoundEvent` runs and the code is chosen by the target's state
  (6 / 9: codes 3, 4, 6 or 7), or is 0x1B when `src == 1`, `tbl->unk0 == 4`
  and the target's `character_data_index` is 2, 4, 0xE or 0xF; else a `switch` on the struck
  limb picks it.

## Frame windows: `fighter_ReadFrameMetadata` and `fighter_ProcessFrameEvents`

Both walk the byte stream at `rec->motion_metadata + 4`; 0 ends it.

- `fighter_ReadFrameMetadata` (src/main/17AFC.c:4744; called at src/main/9F9C.c:3608)
  resets the window fields and reads: 0xFF skips six bytes, a byte below 0x80
  skips one, and 0x80..0x8B take the next byte into `weapon_hit_start_frames[0]`,
  `weapon_hit_end_frames[0]`, `release_frame`, `object_pickup_frame`,
  `opponent_turn_frame`, `frame_window_start`, `frame_window_end`, `weapon_hit_start_frames[1]`,
  `weapon_hit_end_frames[1]`, `paired_move_hit_frame`, `opponent_transition_start_frame`, `weapon_reload_frame`
  (src/main/17AFC.c:4568-4604). `weapon_hit_start_frames[k]` /
  `weapon_hit_end_frames[k]` are the frame windows the contact tests compare `move_frame` with.
- `fighter_ProcessFrameEvents` (src/main/17AFC.c:5360; called at src/main/9F9C.c:5780 and
  src/main/28708.c:1434) reads the same stream: 0xFF tests a 32-bit mask
  against `1 << rec->character_data_index` and skips 4 or 6 bytes; a byte `c` below 0x80
  is followed by a frame number, and when that equals `move_frame` it calls
  `fighter_DispatchFrameEvent(rec, c - 1)` (src/main/17AFC.c:5051); it stops at the first
  frame number above `move_frame`.

## Geometry helpers

- `math_TestSegmentCircle2D` (src/main/17AFC.c:2796): segment against circle in x/z.
- `math_TryPrepareTriangleNormalFrame` (src/main/17AFC.c:3018) / `math_PrepareTriangleCollisionFrame`: a triangle's
  local frame from its edge vectors and normal.
- `func_8002DE20` (src/main/17AFC.c:3101): cuts a triangle rotated by the GTE
  matrix with the plane z = 0 and tests the cut segment against a second
  triangle.
- `math_PointInTriangleXZ` (src/main/17AFC.c:3294): point in triangle.
- `math_TriangleNormalYawWithPositiveYHalfTurn` (src/main/17AFC.c:3759): the GTE cross product of
  `(a1 - a0)` and `(a2 - a0)`, returning `ratan2` of its x/z (+0x800 when y is
  positive). `fighter_IsBoneTriangleCrossYPositive` (src/main/17AFC.c:3788) takes the same cross
  product for a fighter's scratchpad points 2 and 3 about point 1 and returns
  1 when its y is positive, else 0; the result is stored in `bone_triangle_cross_y_positive`
  (src/main/9F9C.c:5624).

## Cues

`fight_RecordReplayEventAndDispatchSoundOrEffect(who, cue, pos, dir)` (src/main/17AFC.c:4895) dispatches the
effect and sound cues the paths above play; sound cues are `who * 40 + id`
(per-player blocks of 40) panned by `snd_QueuePositionalSound` (src/main/17AFC.c:4815)
from `pos` relative to `g_listener_cam`.
