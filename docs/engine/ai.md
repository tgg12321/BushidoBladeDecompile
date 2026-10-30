# CPU opponent: what the code establishes

No routine in `src/` is identified as the CPU opponent's decision logic, i.e. code
that picks moves for a computer-controlled fighter. Both fighters are
`Unk80101EC8Record`s (0x44C bytes, include/game.h:1647) in `D_80101EC8[]`
(include/bb2.h:153). This page describes the routines nearest that topic: the
per-fighter kind queue and its object pool, the shuffled ID list built for a
match sequence, and three standalone helpers. All are C.

## The per-fighter kind queue and the object pool `g_object_pool`

Each fighter record holds a queue of up to 12 kinds: a count `carried_object_count` and
the entries `carried_object_kinds[12]` (include/game.h:1603-1607). `g_object_pool[12]`
(include/bb2.h:191) is a pool of 0x64-byte `Obj80106A78` objects
(include/game.h:1695); a free slot has `kind == -1` and `slot == 0xFF`.

- `object_InitKindQueue(rec)` (src/main/17AFC.c:4038) seeds the queue: it copies the five entries of
  `g_fighter_initial_carried_object_kinds[rec->character_data_index]` (`s8 [27][5]`, include/bb2.h:193), counts the ones
  other than -1, and stores the first in `D_800A36F2[rec->index]`. Called from
  src/main/9F9C.c:4067.
- `object_QueueKind(fighter_record, object_kind)` (src/main/17AFC.c:4227) appends a kind, or puts it
  first when `fighter_record->object_variant_index != -1` and `object_kind == fighter_record->wielded_object_kind`; it returns 0 when
  the queue already holds 12.
- `object_SpawnFromFighter(rec, kind)` (src/main/17AFC.c:4069) allocates the first free
  pool slot and places the object at the fighter (`world_pos`, raised by
  `size_scale / 32`). Its velocity comes from `g_object_kind_params[kind]`
  (`Tbl8008E194`, include/game.h:1858): the horizontal speed `launch_speed`
  along the fighter's heading `orientation.vy`, the vertical speed
  `launch_vel_y`, and a spin `spin_rate` about Y or X by `unk0`.
- `object_ThrowQueuedKind(rec)` (src/main/17AFC.c:4137) takes an entry off the queue
  (index 1 instead of 0 when it holds two or more, the head equals
  `rec->wielded_object_kind` and `rec->object_variant_index != -1`), spawns it through `object_SpawnFromFighter`, and plays a cue
  through `fight_RecordReplayEventAndDispatchSoundOrEffect`. `object_DropQueuedHeadAt(fighter_record, drop_position)` (src/main/17AFC.c:4175)
  spawns the head entry at `drop_position`, and `object_SpawnScatteredAt(fighter_record, object_kind, drop_position)`
  (src/main/17AFC.c:4204) spawns `object_kind` at `drop_position`; both give the object a
  random velocity and spin from `rng_Next`.
- `object_PickUpNearby(fighter_record)` (src/main/17AFC.c:4249) takes a pool object back into
  the queue: the first object whose kind is not -1 or 0x12..0x1D, whose
  `is_moving` is 0, which lies within 100 units of the fighter's `collision_pos.vy`
  vertically and within a squared horizontal distance of 0xF423F of
  `world_pos`. It frees the slot and returns the kind (-1 if none, or if the
  queue is full).
- The fighter update calls these at src/main/9F9C.c:5730 (`object_SpawnScatteredAt`),
  :5743 (`object_ThrowQueuedKind`) and :5750 / :5760 (`object_PickUpNearby`), gated on the
  record's `move_kind` / `move_frame` values.
- Pool cleanup: `object_ResetPool` (src/main/17AFC.c:4029) frees all twelve
  slots (called at src/main/9F9C.c:1778); `object_FreeAttachedObjects`
  (src/main/17AFC.c:3880) frees objects with `is_attached != 0` and `object_ClearFixedKindRange`
  (src/main/17AFC.c:4085) frees kinds 0x12..0x1D (both called at
  src/main/9F9C.c:1189-1190).

## The shuffled ID list `g_fight_opponent_order`

`ApplyPendingConfigAndInitializeMode` case 1 (src/main/24BF0.c:54) calls `fight_BuildOpponentOrder` and then
`fight_AdvanceOpponentOrder`.

`fight_BuildOpponentOrder` (src/main/17AFC.c:5737):

1. Collects the IDs `i` in 0..26 whose bit is set in
   `g_saved_record.unlock_mask & 0x3EF3DF`, minus the bit
   `g_fight_config_id_data_rows[g_fight_config.unk_4[0]]` (src/main/17AFC.c:5747-5748), into
   `g_fight_opponent_order`.
2. Swaps two `rand() % count` positions 108 times (src/main/17AFC.c:5768-5778).
3. If bit 0x20 or 0x10000 of `g_saved_record.unlock_mask` is set, adds ID 5 (bit 0x20)
   and / or ID 0x10 (bit 0x10000), in random order when both are set: the first
   goes to position 10, shifting the rest up (or at the end of a shorter list),
   and the second is appended.
4. Appends 0x1A or 0x18 when bit 0x04000000 or 0x01000000 is set (which one
   is tested depends on `g_fighter_group_flags`), stores the count in `g_fight_opponent_order_count`, and
   clears the cursor `g_fight_opponent_order_cursor`.

`fight_AdvanceOpponentOrder` (src/main/17AFC.c:5858) steps the cursor: it sets
`g_fight_config.unk_4[1] = D_8008D55C[g_fight_opponent_order[cursor]]`, or, once the cursor
reaches the count, sets the mode `g_main_loop_handler_index` to 0x12 (count 20) or 0x20.
The 27 IDs match the row count of the per-character tables (`g_fighter_initial_carried_object_kinds`,
`g_fighter_base_move_frame_steps`, include/bb2.h:181-193), which is consistent with the list being
an opponent order; the code does not name it.

## Random numbers

- `rand` (src/main/psxsdk/libc2/rand.c:8) is PsyQ's linear congruential
  generator: `x = x * 0x41C64E6D + 0x3039`, returning bits 16..30.
- `rng_Next` (src/main/6CF8.c:415) is the game's own generator:
  `r = x * 5497 + 0x7FA9`, `x = (x >> 16) ^ r`, returning the low 15 bits;
  `rng_SetSeed` (src/main/6CF8.c:413) sets its state `g_rng_state`.

## Standalone helpers

- `math_ReflectAndDampRecordVector(a0, a1)` (src/main/17AFC.c:4004) takes the angle
  `ratan2(a1[0], a1[2])`, rotates the X/Z pair at `a0[0x11]` / `a0[0x13]`
  into that angle's frame (`>> 12`), negates the second component, rotates back
  with `>> 15` (an eighth of the magnitude), and divides `a0[0x12]` by four. No direct caller appears in `src/`.
- `fighter_RelocateAndStartEventMove(rec)` (src/main/17AFC.c:193), called from `fighter_ResolveBoneHit`
  (src/main/17AFC.c:431) when `rec->character_roster_index == 0xD`, moves the fighter's
  positions (`world_pos`, `base_pos`, `collision_pos`) to a target: with
  `g_fight_stage_index == 3` a percentage blend from the fighter's position toward the
  point (0x2EE0 or -0x2710, 0x1770 or -0x1770), picked by the other fighter's
  position, with a weight that shrinks from 60 to 10 as the counter `reposition_count`
  (capped at 0x40) grows; otherwise the entry
  `fight_GetStartPositionPairTable()[g_fight_stage_index * 12 + index * 3]`. It then zeroes `base_velocity` /
  `decaying_xz_velocity`, switches its move (the script `motion_ResolveMoveData` resolves from
  `cur_move->event_move_table_id` and that record's halfword 0x1D, made current by
  `fighter_StartMoveScript`) and plays
  cue 0x30.
- `camera_SetFighterFocusView(fighter_record, yaw)` (src/main/9F9C.c:1043) sets the view target
  `g_camera_default_view_record.focus_point` to `fighter_record->bone_hit_points_4_5_midpoint`, its rotation to `(0x120, yaw, 0)` and
  its distance `eye_distance` to 0x1162. Its caller is `fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange`
  (src/main/9F9C.c:2251).
