# Combat: contact tests and hit reactions

The two fighters are `Unk80101EC8Record`s (0x44C bytes, include/game.h:1647)
in `D_80101EC8[]` (include/bb2.h:153); each record's `other` points at the
opponent. Most of the code below is in src/main/17AFC.c. Field names are the
header's `unk_NN` names; where a field's role is shown by the code it is
stated, otherwise only its use is described.

## Per-frame entry: `fight_UpdateFighterInteractionsAndPointCaches`

`fight_UpdateFighterInteractionsAndPointCaches` (src/main/17AFC.c:2194) runs once per frame; its callers are
src/main/9F9C.c:2044, :2088 and :2197, each followed by `object_UpdatePool` (the
object-pool update, src/main/17AFC.c:3991). It branches on fighter 0's state
`move_kind`:

- 0xF, 0x1C..0x21: `fight_UpdatePairedInputTotalsEventsAndMidpointEffects` (src/main/175A4.c:21) only.
- 0x11: `fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents` (src/main/17AFC.c:2061) only.
- otherwise, in order:
  1. `fighter_TriggerOpponentResponseOnMoveFrame` (src/main/17AFC.c:736): per-fighter checks of
     `unk_0C == 0x1B` records against the opponent's state.
  2. `fighter_ResolveBodyOverlap` (src/main/17AFC.c:773): the body-overlap push (its
     comment at :766): two points per fighter tested pairwise against the
     radii `D_800A3144[0..1]`, an overlap pushing both apart along x/z
     through `unk_134`.
  3. `D_800A3824 = fighter_ProcessWeaponContacts()` (src/main/17AFC.c:1077): the blade
     contact test between the two fighters (comment at :1065); it returns
     `(fighter-1 grid << 1) | fighter-0 grid` on contact, else -1.
  4. On contact, `fighter_BlendCurrentAndPreviousContactPoints` (src/main/17AFC.c:2118), then
     `fighter_ResolveWeaponContact` (src/main/17AFC.c:574) for each fighter when either has
     `unk_AD` set; otherwise `fighter_ProcessWeaponBodyHits(0)` (below).
  5. When both fighters have `update_count >= 3`, `D_800A38A8` is set, both
     `pending_move_event` are -1, neither `unk_0C` is 0x1F and their heights `world_pos.y`
     differ by less than 1000, both get `pending_move_event = 0xA`.
- Then for every state: a fighter whose `move_kind` is 5 ends the round
  (`g_main_loop_handler_index = 0x1C`, with `D_800A3748` the other fighter's index); the
  scratchpad body points are copied into `unk_210` / `unk_234`, `unk_18C`
  (the centroid of points 1..3) and `unk_174` (the midpoint of points 4..5)
  (src/main/17AFC.c:2252-2278); and while fighter 0's `pending_move_event` is -1,
  `object_ResolveFighterHits` (src/main/17AFC.c:4246) tests the pool objects against the
  fighters, followed by `fight_TriggerOpposingFighterEventFromNearBallisticRecords` (src/main/17AFC.c:4432) when that
  left `pending_move_event` unchanged.

`pending_move_event` is the reaction code these paths write; -1 means none pending.

## Blade against body: `fighter_ProcessWeaponBodyHits`

`fighter_ProcessWeaponBodyHits(mode)` (src/main/17AFC.c:1568) runs for each fighter `i` as the
target, with the opponent as the attacker:

1. It decides how many blade quads to test (`var_10`): one when the
   attacker's `is_defeated == 0`, `unk_92 != 0`, `update_count != 0` and `unk_0C != 0x1F`,
   one more with `unk_8C` set, and one more when `weapon_id` is 4 or 5 and any is
   tested. (With `mode == 1` the fighter `D_800A38AE` is skipped as a target.)
   Attackers with `weapon_id` 6 or 7, in states 2 / 0x1B / 0x28 / 0x26 at frame
   `move_frame == weapon_hit_start_frames[0]`, also sweep a segment with `fighter_CastWeaponSegment`
   (src/main/17AFC.c:1423, comment at :1399) while the counter `weapon_ammo`
   lasts (once it is spent they play cue 0x32 and test no quads); `unk_0C`
   0x1D / 0xE (with `unk_26C` set) do the same with the second blade
   (`weapon_hit_start_frames[1]`).
2. Each blade quad (this frame's points from the scratchpad and last frame's
   from `unk_210` / `unk_234`) is split into 2 or 4 triangles (4 when the tip
   moved more than 2500 units) and each triangle goes through `math_PrepareTriangleCollisionFrame`
   (src/main/17AFC.c:2377) and `fighter_TestTriangleBoneHits` (src/main/17AFC.c:2296), which
   leave bit masks of struck limb records in the scratch record.
3. Of the struck records among the target's 22 limb records (`D_800F5F68[i]`)
   it picks the one nearest the attacker's point `unk_210[1]`
   (src/main/17AFC.c:1806-1822). With `mode == 1` it only plays cue 4 there
   and returns.
4. Unless the struck limb is in the sweep / second-blade mask and the
   attacker has `weapon_id` 6 or 7 or `unk_0C` 0x1D / 0xE, or the attacker is in an attack
   state (2, 0x1B, 0x28, 0x26) within three frames before a window start, it
   pushes the target along x/z through
   `unk_134` when the facing difference is under 0x400.
5. If the attacker's `unk_AE` is set and (with `unk_8C`) its frame `move_frame`
   is inside the window `weapon_hit_start_frames[k]..weapon_hit_end_frames[k]`, it calls
   `fighter_ResolveBoneHit(0, target, limb, facing difference, ...)`.

## Hit reaction: `fighter_ResolveBoneHit`

`fighter_ResolveBoneHit(src, rec, limb, thresh, flag, tbl, arg6, out)`
(src/main/17AFC.c:319) is called with `src == 0` from `fighter_ProcessWeaponBodyHits` and with
`src == 1` and a `Tbl8008E194` record from `object_ResolveFighterHits` (the comment at
src/main/17AFC.c:1954 records both call shapes). Its branches, in order:

- `rec->unk_0C == 0x1C`: when the limb is not 1..3, or the dot product of
  `unk_1EC` with the vector `D_800A37E8` is under 0x400, it plays cue 0x12,
  sets `pending_move_event = 0x1E` (and, with `src == 0`, the attacker's `pending_move_event` to 0xB
  or 0x19) and returns 0.
- `rec->unk_0C == 0xD` (outside state 0x2E): `fighter_RelocateAndStartEventMove`, see
  [ai.md](ai.md).
- `move_kind` 4 or 0x14, or `unk_0C == 0x1F`: `fighter_StoreBoneLocalVectorAndArmEventTrigger`
  (src/main/9F9C.c:2779) and `fighter_DispatchBoneHitSoundEvent` (src/main/17AFC.c:238); the
  latter case also sets `pending_move_event = 7`.
- The target in an attack state (2, 0x1B, 0x28, 0x26) with `move_frame < unk_A0`:
  with `src == 0` the reaction code
  comes from `D_8008EB74[math_ClassifyAbsMagnitude(diff)][sign][same]`, where `diff` is
  the difference of the two `unk_20` values and `same` whether their `unk_AF`
  match; codes 0..2 play cue pairs (0x25 / 0x26 / 0x2D), and code 2 also runs
  `fighter_TryDropQueuedHeadAtBone`. With `src == 1` the table record's `unkC` picks code 0
  or 1.
- Otherwise `fighter_DispatchBoneHitSoundEvent` runs and the code is chosen by the target's state
  (6 / 9: codes 3, 4, 6 or 7), or is 0x1B when `src == 1`, `tbl->unk0 == 4`
  and the target's `character_data_index` is 2, 4, 0xE or 0xF; else a `switch` on the struck
  limb picks it.

## Frame windows: `fighter_ReadFrameMetadata` and `fighter_ProcessFrameEvents`

Both walk the byte stream at `rec->motion_metadata + 4`; 0 ends it.

- `fighter_ReadFrameMetadata` (src/main/17AFC.c:4505; called at src/main/9F9C.c:3376)
  resets the window fields and reads: 0xFF skips six bytes, a byte below 0x80
  skips one, and 0x80..0x8B take the next byte into `weapon_hit_start_frames[0]`,
  `weapon_hit_end_frames[0]`, `unk_A7`, `object_pickup_frame`,
  `opponent_turn_frame`, `unk_A5`, `unk_A6`, `weapon_hit_start_frames[1]`,
  `weapon_hit_end_frames[1]`, `unk_AA`, `unk_AB`, `unk_AC`
  (src/main/17AFC.c:4530-4566). `weapon_hit_start_frames[k]` /
  `weapon_hit_end_frames[k]` are the frame windows the contact tests compare `move_frame` with.
- `fighter_ProcessFrameEvents` (src/main/17AFC.c:5100; called at src/main/9F9C.c:5148 and
  src/main/28708.c:1434) reads the same stream: 0xFF tests a 32-bit mask
  against `1 << rec->character_data_index` and skips 4 or 6 bytes; a byte `c` below 0x80
  is followed by a frame number, and when that equals `move_frame` it calls
  `fighter_DispatchFrameEvent(rec, c - 1)` (src/main/17AFC.c:4807); it stops at the first
  frame number above `move_frame`.

## Geometry helpers

- `math_TestSegmentCircle2D` (src/main/17AFC.c:2592): segment against circle in x/z.
- `math_TryPrepareTriangleNormalFrame` (src/main/17AFC.c:2807) / `math_PrepareTriangleCollisionFrame`: a triangle's
  local frame from its edge vectors and normal.
- `func_8002DE20` (src/main/17AFC.c:2889): cuts a triangle rotated by the GTE
  matrix with the plane z = 0 and tests the cut segment against a second
  triangle.
- `math_PointInTriangleXZ` (src/main/17AFC.c:3082): point in triangle.
- `math_TriangleNormalYawWithPositiveYHalfTurn` (src/main/17AFC.c:3535): the GTE cross product of
  `(a1 - a0)` and `(a2 - a0)`, returning `ratan2` of its x/z (+0x800 when y is
  positive). `fighter_IsBoneTriangleCrossYPositive` (src/main/17AFC.c:3563) takes the same cross
  product for a fighter's scratchpad points 2 and 3 about point 1 and returns
  1 when its y is positive, else 0; the result is stored in `unk_1DA`
  (src/main/9F9C.c:5038).

## Cues

`fight_RecordReplayEventAndDispatchSoundOrEffect(who, cue, pos, dir)` (src/main/17AFC.c:4657) dispatches the
effect and sound cues the paths above play; sound cues are `who * 40 + id`
(per-player blocks of 40) panned by `snd_QueuePositionalSound` (src/main/17AFC.c:4576)
from `pos` relative to `g_listener_cam`.
