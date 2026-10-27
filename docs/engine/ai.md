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

- `object_InitKindQueue(rec)` (src/main/17AFC.c:3728) seeds the queue: it copies the five entries of
  `D_8008E338[rec->character_data_index]` (`s8 [27][5]`, include/bb2.h:193), counts the ones
  other than -1, and stores the first in `D_800A36F2[rec->index]`. Called from
  src/main/9F9C.c:3783.
- `object_QueueKind(rec, kind)` (src/main/17AFC.c:3901) appends a kind, or puts it
  first when `rec->unk_88 != -1` and `kind == rec->unk_14`; it returns 0 when
  the queue already holds 12.
- `object_SpawnFromFighter(rec, kind)` (src/main/17AFC.c:3756) allocates the first free
  pool slot and places the object at the fighter (`world_pos`, raised by
  `size_scale / 32`). Its velocity comes from `D_8008E194[kind]`
  (`Tbl8008E194`, include/game.h:1661): the horizontal speed `unk4` along the
  fighter's heading `orientation.vy`, the vertical speed `unk6`, and a spin
  `unk8` about Y or X by `unk0`.
- `object_ThrowQueuedKind(rec)` (src/main/17AFC.c:3819) takes an entry off the queue
  (index 1 instead of 0 when it holds two or more, the head equals
  `rec->unk_14` and `rec->unk_88 != -1`), spawns it through `object_SpawnFromFighter`, and plays a cue
  through `fight_RecordReplayEventAndDispatchSoundOrEffect`. `object_DropQueuedHeadAt(rec, pos)` (src/main/17AFC.c:3853)
  spawns the head entry at `pos`, and `object_SpawnScatteredAt(rec, kind, pos)`
  (src/main/17AFC.c:3879) spawns `kind` at `pos`; both give the object a
  random velocity and spin from `rng_Next`.
- `object_PickUpNearby(rec)` (src/main/17AFC.c:3919) takes a pool object back into
  the queue: the first object whose kind is not -1 or 0x12..0x1D, whose
  `unk_50` is 0, which lies within 100 units of the fighter's `collision_pos.vy`
  vertically and within a squared horizontal distance of 0xF423F of
  `world_pos`. It frees the slot and returns the kind (-1 if none, or if the
  queue is full).
- The fighter update calls these at src/main/9F9C.c:5101 (`object_SpawnScatteredAt`),
  :5122 (`object_ThrowQueuedKind`) and :5128 / :5133 (`object_PickUpNearby`), gated on the
  record's `move_kind` / `move_frame` values.
- Pool cleanup: `object_ResetPool` (src/main/17AFC.c:3719) frees all twelve
  slots (called at src/main/9F9C.c:1769); `object_FreeAttachedObjects`
  (src/main/17AFC.c:3764) frees objects with `is_attached != 0` and `object_ClearFixedKindRange`
  (src/main/17AFC.c:3967) frees kinds 0x12..0x1D (both called at
  src/main/9F9C.c:1180-1181).

## The shuffled ID list `D_801077B0`

`ApplyPendingConfigAndInitializeMode` case 1 (src/main/24BF0.c:54) calls `fight_BuildOpponentOrder` and then
`fight_AdvanceOpponentOrder`.

`fight_BuildOpponentOrder` (src/main/17AFC.c:5279):

1. Collects the IDs `i` in 0..26 whose bit is set in
   `g_saved_record.unk_00 & 0x3EF3DF`, minus the bit
   `D_8008D538[g_fight_config.unk_4[0]]` (src/main/17AFC.c:5288), into
   `D_801077B0`.
2. Swaps two `rand() % count` positions 108 times (src/main/17AFC.c:5309).
3. If bit 0x20 or 0x10000 of `g_saved_record.unk_00` is set, adds ID 5 (bit 0x20)
   and / or ID 0x10 (bit 0x10000), in random order when both are set: the first
   goes to position 10, shifting the rest up (or at the end of a shorter list),
   and the second is appended.
4. Appends 0x1A or 0x18 when bit 0x04000000 or 0x01000000 is set (which one
   is tested depends on `g_fighter_group_flags`), stores the count in `D_800A391F`, and
   clears the cursor `D_800A3783`.

`fight_AdvanceOpponentOrder` (src/main/17AFC.c:5397) steps the cursor: it sets
`g_fight_config.unk_4[1] = D_8008D55C[D_801077B0[cursor]]`, or, once the cursor
reaches the count, sets the mode `g_main_loop_handler_index` to 0x12 (count 20) or 0x20.
The 27 IDs match the row count of the per-character tables (`D_8008E338`,
`D_8008DE34`, include/bb2.h:181-193), which is consistent with the list being
an opponent order; the code does not name it.

## Random numbers

- `rand` (src/main/psxsdk/libc2/rand.c:8) is PsyQ's linear congruential
  generator: `x = x * 0x41C64E6D + 0x3039`, returning bits 16..30.
- `rng_Next` (src/main/6CF8.c:415) is the game's own generator:
  `r = x * 5497 + 0x7FA9`, `x = (x >> 16) ^ r`, returning the low 15 bits;
  `rng_SetSeed` (src/main/6CF8.c:413) sets its state `g_rng_state`.

## Standalone helpers

- `math_ReflectAndDampRecordVector(a0, a1)` (src/main/17AFC.c:3694) takes the angle
  `ratan2(a1[0], a1[2])`, rotates the X/Z pair at `a0[0x11]` / `a0[0x13]`
  into that angle's frame (`>> 12`), negates the second component, rotates back
  with `>> 15` (an eighth of the magnitude), and divides `a0[0x12]` by four. No direct caller appears in `src/`.
- `fighter_RelocateAndStartEventMove(rec)` (src/main/17AFC.c:178), called from `fighter_ResolveBoneHit`
  (src/main/17AFC.c:370) when `rec->unk_0C == 0xD`, moves the fighter's
  positions (`world_pos`, `base_pos`, `collision_pos`) to a target: with
  `g_fight_stage_index == 3` a percentage blend from the fighter's position toward the
  point (0x2EE0 or -0x2710, 0x1770 or -0x1770), picked by the other fighter's
  position, with a weight that shrinks from 60 to 10 as the counter `unk_34C`
  (capped at 0x40) grows; otherwise the entry
  `fight_GetStartPositionPairTable()[g_fight_stage_index * 12 + index * 3]`. It then zeroes `unk_104` /
  `unk_134`, switches its move (the script `motion_ResolveMoveData` resolves from
  `cur_move->event_move_table_id` and that record's halfword 0x1D, made current by
  `fighter_StartMoveScript`) and plays
  cue 0x30.
- `camera_SetFighterFocusView(rec, a1)` (src/main/9F9C.c:980) sets the view target
  `D_800F6608.unk_00` to `rec->unk_174`, its rotation to `(0x120, a1, 0)` and
  its distance `w18` to 0x1162. Its caller is `fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange`
  (src/main/9F9C.c:2081).
