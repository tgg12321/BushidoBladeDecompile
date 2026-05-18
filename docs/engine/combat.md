# Combat and Hit Detection

Combat in BB2 is a per-frame pipeline: for each fighter, the engine reads pad
input (or AI command stream), commits a move, advances the move's animation
frame, computes the weapon and body bone positions in world space, runs
collision tests against the opposing fighter, applies damage, and updates
state. This document describes that pipeline.

Most combat logic lives in `code6cac_b.c` (`0x80026DA4..0x80035438`, 4247 LOC)
with collateral in `code6cac.c`, `main.c` (`action_*`, `damage_*`), and the
text1b.c blocks 12463-13840 (ground-hit / weapon hit). The function name
prefix `coli_` is short for "collision", `cpu_` indicates AI/move decision
helpers, `action_` is the move-execution layer, and `damage_` is the
damage-resolution layer.

## Fighter data structure

There is no formal `Fighter` typedef yet — the structure is accessed via raw
offsets throughout `code6cac_b.c`. From observed field offsets:

| Offset | Type | Use |
| --- | --- | --- |
| 0x00 | ptr | per-fighter data pointer (`*((s32 *)arg0)` is the actual fighter base; the arg passed around is a wrapper) |
| 0x04 | s16 | fighter index (0=P1, 1=P2, 2=AI/prop) |
| 0x12 | u16 | move id (current move being executed) |
| 0x14 | s16 | facing direction (0/1) |
| 0x34C | u8 | side-move counter (cooldown for tubazeri) |
| 0xF4 | s32 | world X position |
| 0xF8 | s32 | world Y position (height) |
| 0xFC | s32 | world Z position |
| 0x44 | s32 | velocity X |
| 0x48 | s32 | velocity Y |
| 0x4C | s32 | velocity Z |
| 0x50 | ptr | active-move pointer (current waza animation) |
| 0x58 | ptr | active-attack command stream pointer (move opcodes) |
| 0x5C | s16 | weapon angle / extension |
| 0x5E | s16 | move parameter (varies per move) |
| 0x60 | s16 | move parameter 2 |
| 0x88 | s16 | sentinel = -1 means "slot vacant" (for the 12-slot active-move array) |
| 0xA1-0xAC | u8[12] | decoded command-stream output (sparse field bag, see `cpu_get_dist_2`) |
| 0xB8..0xC0 | s32[3] | shifted body position |
| 0xCC..0xD8 | s32[3] | body bone position |
| 0xD8..0xE0 | s32[3] | body bone delta (per-frame) |
| 0xF4..0xFC | s32[3] | head/cursor world position |
| 0x104..0x10C | s32[3] | local frame position accumulator |
| 0x134..0x13C | s32[3] | local frame position accumulator 2 |
| 0x1A | s16 | head height offset (for `coli_hit_body_weapon`) |
| 0x1CA | u16 | direction angle (12-bit, 0..0xFFF) — used as `Judge[angle & 0xFFF]` for sin/cos LUT |
| 0x330..0x33B | u16[6] | active-move queue |

The same offset map is used for the 12-slot active-move array starting at
`D_80106A78` — each slot is 0x64 bytes (the offsets above), with slot 0 at
`D_80106A78`, slot 1 at `D_80106A78 + 0x64`, etc. The "slot vacant" check is
`*((s16 *)(slot + 0x88)) == -1 && *((u8 *)(slot + 0xA)) == 0xFF`.

## Move-command stream

Each move is described by an opcode-stream stored in the character's data
(loaded from disc into `g_char_data` at `0x800A6690`). The streams are NOT
plain animation timelines — they're tiny bytecode programs the AI/move
selector advances each frame.

The decoder is `cpu_get_dist_2(u8 *a0)` (`code6cac_b.c:2975`). It walks the
stream pointed at by `a0[0x58] + 5` and dispatches:

| Byte range | Meaning |
| --- | --- |
| `0x00` | end-of-stream (loop exit) |
| `0x01..0x7F` | "compact" opcode — advance the read head 1 byte |
| `0x80..0x8B` | "field-store" opcode: read next byte and store it into fighter slot 0xA1..0xAC (one byte per opcode 0x80+N) |
| `0x8C..0xFE` | "compact" opcode — advance 1 byte |
| `0xFF` | "long" opcode — advance 6 bytes |

The "field-store" opcodes 0x80..0x8B are the interesting ones. The switch at
`code6cac_b.c:3010` maps them to specific fighter fields:

| Opcode | Stores to fighter offset | Meaning (inferred) |
| --- | --- | --- |
| 0x80 | 0xA1 | weapon-active flag |
| 0x81 | 0xA3 | next move queued |
| 0x82 | 0xA7 | hit response selector |
| 0x83 | 0xA8 | damage scalar |
| 0x84 | 0xA9 | combo flag |
| 0x85 | 0xA5 | weapon-pose timer |
| 0x86 | 0xA6 | weapon-pose value |
| 0x87 | 0xA2 | guard flag |
| 0x88 | 0xA4 | block timer |
| 0x89 | 0xAA | move counter |
| 0x8A | 0xAB | move tag A |
| 0x8B | 0xAC | move tag B |

`cpu_get_dist_2` is called every frame for each active fighter, after first
clearing the field bag (all bytes to 0xFF or 0). It's effectively "evaluate
this frame of the move animation". When the AI moves between waza, it
overwrites the `0x58`-stream pointer and resets the field bag.

## AI/CPU command-stream advance

`cpu_check_same_dir_timer` (`code6cac_b.c:3262`) is the same kind of decoder
but executes a different bytecode form found at `fighter[0x58]+5`:

| Byte | Meaning |
| --- | --- |
| `0` | exit |
| `0xFF` | bitmask-test: read 4 bytes as packed mask; if `mask & (1 << fighter[0xA])` is set, advance 4 bytes; otherwise 6 bytes |
| `< 0x80` | direction-compare: if `*p == fighter[0x40]` then call `func_80032C50(fighter, opcode - 1)` else if `dir < opcode` exit |
| `>= 0x80` | skip 1 byte |

`func_80032C50` is the move-direction match callback — sets up the new
direction. The whole construct lets each waza include a "if facing
right/left, do X; otherwise Y" branch.

These are the two scripting layers — one for "what should I do this frame
of the current move" (`cpu_get_dist_2`) and one for "should I commit to a
different move" (`cpu_check_same_dir_timer`).

## Move queue / commit (`coli_hit_body_weapon` and `cpu_set_move_command_and_dir`)

When the AI or pad input decides on a move, `cpu_set_move_command_and_dir`
(`code6cac_b.c:2540`) commits it:

1. Calls `coli_hit_body_weapon(a0, move_id)` — allocates a slot in the
   12-entry active-move array at `D_80106A78`, populates the fighter's
   world-position (offset 0x2C..0x34), weapon offset (computed via
   `Judge[]` sin/cos lookup applied to the move-data weapon vector), and
   the move's animation parameters.
2. Resets the slot's flags (offset 4, 0xB).
3. Copies the fighter velocity vector to offset 0x2C of the slot.
4. Initializes random "blood spray" / hit-direction values from `rng_Next()`
   into offsets 0x44, 0x48, 0x4C, and various motion parameters at
   0x5C..0x60.

`coli_hit_body_weapon` itself (`code6cac_b.c:2377`) builds the swept-volume
representation of the body + weapon for the upcoming collision tests. It:

- Finds a free slot in `D_80106A78[12]` (each 0x64 bytes).
- Sets up "head + neck + weapon" composite via two `Judge[]` lookups —
  `Judge[angle & 0xFFF]` and `Judge[(angle + 0x400) & 0xFFF]` are sin and cos
  of the 12-bit direction angle, both scaled by the move's weapon-reach
  parameter from `D_8008E194 + arg1*14`.
- Stores 3 representative points for swept-volume tests:
  - "center" (0x2C..0x34) — fighter pos plus shifted weapon offset
  - "tip"    (0x38..0x40) — center + same offset (i.e., 2x weapon offset
    away from fighter)
  - "rotation axis" (0x44..0x4C) — pure weapon vector
- The waza-table type byte at `(&D_8008E194)[arg1*14]` (cast to s16) selects
  one of three swept-volume modes (line, sphere, capsule).

This data is what the per-frame `cpu_check_tubazeri` / `coli_check_circle_hit_line`
tests against.

## Per-frame hit detection

The main hit-test loop runs once per frame per fighter pair. The primitives:

### `cpu_check_tubazeri` (`code6cac_b.c:1907`) — sword clash

Tubazeri is the "blade-lock" state when two weapons cross. The function takes
three fighter pointers (`a0`, `a1`, `a2`) — your fighter, your weapon-target,
and the second fighter — and uses the GTE to compute the cross product of
the two weapon vectors:

1. Writes `(a1.pos - a0.pos)` to scratchpad `0x1F800360..0x1F800368` (3
   words — diff vector A).
2. Writes `(a2.pos - a0.pos)` to scratchpad `0x1F800370..0x1F800378` (diff
   vector B).
3. Loads diff A into GTE coef regs `$0/$2/$4` via `ctc2`.
4. Loads diff B into GTE IR regs `$9/$10/$11` via `lwc2`.
5. Issues GTE OP cross product (`.word 0x4B70000C`).
6. Reads MAC1/MAC2/MAC3 results back via `swc2 $25/$26/$27` to
   `0x1F800380..0x1F800388`.
7. Calls `single_game_getEnemyCharId(cross[0], cross[2])` to resolve the
   collision result against the enemy ID.
8. If the cross-product Y component (sign of orientation) is positive, OR
   the result with `0x800`.

This is the "are these two blades crossing in 3D" test, used to enter
blade-lock state.

### `coli_check_circle_hit_line` (`code6cac_b.c:1992`)

A variant that tests a circle (a weapon arc swept over a frame) against a
line segment (the opposing weapon's reach over the same frame). Uses the same
scratchpad scheme to compute deltas, runs the GTE cross product, and returns
which limb/axis intersected.

### `gnd_land_hit_char_tsuba` (`text1b.c:12577`)

Ground/weapon-vs-body test. Called from the main loop's gameplay handler.
Returns the new active fighter struct pointer if a hit was detected; the
caller uses this to start the hit-reaction animation.

### `gnd_land_hit_char_die_main` (`text1a.c:1373`)

Death/death-blow handler — runs when a hit damages the fighter past their
hitpoints. Triggers the "katinuki" finisher animation if appropriate.

## Damage resolution

`damage_DebugDisp` (`code6cac_c_mid.c:247`) is a debug walker over the
fighter health-table — it iterates 3 slots of 0x24 bytes each at offset 0x6C
in the fighter and verifies a checksum. The actual damage application happens
in `damage_CalcHitDamage` at `0x8003880C` (asm-only, undecompiled in C):

- Args: hit-result struct, fighter slot
- Computes damage from weapon-type and hit-zone
- Subtracts from fighter HP
- If HP <= 0, sets a death flag that `gnd_land_hit_char_die_main` picks up

The fighter HP itself lives at offsets within the 0x6C+ region of each
fighter struct. There is more than one HP value tracked (probably "current"
and "max" or "left arm / right arm / body" given fighting-game conventions);
the multiple 0x24-byte regions in `damage_DebugDisp` look like 3 hit-zones
with independent HP, but this is not yet fully traced.

## "Mental" / spirit gauge (the saTan1..saTan5 family)

BB2's gauge HUD is an in-house particle/UI system that uses functions named
`saTan<N>...` (apparently "samurai tanren" — "samurai training"). These live
mostly in `text1b.c` and `sound.c`:

- `saTan0Init`, `saTan1GaugeInit`, `saTan2GaugeInit_80077B20` —
  initialization functions for the various gauge bar variants.
- `saTan3GaugeMain_8006A564`, `saTan4GaugeMain`, `saTan1GaugeMain` — per-frame
  update functions.
- `saTan5TakeAnim2_2` (`main.c:109`) — handles the volume/SE pacing during
  certain animations; uses irq-set-alarm to wait between sound events.
- `saTanMainDispGnd_80046020` — gauge rendering against the ground.

Each gauge is updated by `saTan4GaugeMain` style functions that decrement a
"timer" field, advance the gauge by 1 unit per N ticks, and dispatch sound
effects when crossing thresholds.

## Katinuki finisher

"Katinuki" (literally "victory cut-through") is BB2's instant-kill finishing
move — a precise hit on an exposed vital area finishes the fight in one
strike. The mechanic is implemented across several functions all named
`katinuki_*`:

- `katinuki_game_get_katinuki_max_num_*` (5 variants at different addresses,
  all in `named_syms.txt`) — return the maximum number of consecutive
  katinuki finishes for the current game mode.
- `katinuki_game_setData_8003D2C4` (called from `sys_GameInit`) — initializes
  the katinuki state at boot.
- `katinuki_game_getMyWeaponId` (`code6cac_b.c:1986`) — looks up the
  fighter's weapon id for katinuki damage calculation.
- `md_game_check_change_main_mode_katinuki` (`main.c:2240`) — checks
  whether the player has triggered katinuki conditions this frame to
  transition out of the normal game-mode dispatch.

The 5 `katinuki_game_get_katinuki_max_num_*` are presumably one each for the
different game modes (Single, Slash, Story, Practice, VS).

## Hit-pause helpers — `coli_HitPauseKatana`

`coli_HitPauseKatana` (`main.c:1693`) and `coli_HitPauseKatana_2`
(`main.c:1983`) are the misnamed "Hit Pause" functions — they're actually
the **SPU voice-key allocator** (despite the `coli_` name). The Kengo
rename collided two functions; the real implementation manages SPU voice
slots for sound effects during hit pauses (the engine slows down the SPU
voice playback when a hit lands, hence the name).

The function manages a free-list of voice keys via the `g_spu_voice_key_a/b/c`
globals. See [sound.md](sound.md) for details.

## Where unmatched asm still hides in combat

- `action_check_defense` (`0x80086130`, also `_2` at `0x80089EB0`) — combat
  block/defense check.
- `action_CheckHitZangeki` (`asm/funcs/action_CheckHitZangeki.s`) — slash-hit
  test.
- `coli_calc_motion` (`0x8003D888`, `_2` at `0x8008ACD0`) — motion vs collision
  resolver.
- `damage_CalcHitDamage` / `damage_CalcHitDamage2` (`0x8003880C`, `0x80082A14`)
  — actual damage calculation.
- Big chunks of the `code6cac_b.c` `func_8003XXX` cluster (over 100 functions
  that wrap the AI/move dispatch).

## Ground / arena init (2026-05-17)

`gnd_init_8001B294(p1_char_ptr, p2_char_ptr)` (`code6cac.c:1067`) sets
up the per-round arena state when a fight starts.  It builds:

- **Camera midpoint** (X/Y/Z averages of P1/P2 positions, +0xF4/+0xF8/+0xFC)
- **4 gauge slots** with max/cur defaults (max=100, cur=0)
- **Facing angle** (BAM): `0x400 - single_game_getEnemyCharId(dx, dy)`
- **Round timer max** = 0x1388 (5000 frames)
- **Init flag** (reset to 0 at end)

| Symbol | Address | Role |
|--------|---------|------|
| `g_gnd_midpoint_x` | `0x800F6608` | Midpoint of P1/P2 X positions |
| `g_gnd_midpoint_y` | `0x800F660C` | Midpoint Y |
| `g_gnd_midpoint_z` | `0x800F6610` | Midpoint Z |
| `g_gnd_camera_flag` | `0x800F6618` | Camera/state flag |
| `g_gnd_facing_angle` | `0x800F661A` | Facing angle in BAM |
| `g_gnd_misc_counter` | `0x800F661C` | Counter |
| `g_gnd_round_time_max` | `0x800F6620` | Round timer max = 0x1388 |
| `g_gnd_init_complete_flag` | `0x800F6626` | Init flag (reset to 0 at end) |
| `g_gnd_gauge_a_max` | `0x800F6638` | Gauge A max = 0x64 (100) |
| `g_gnd_gauge_a_cur` | `0x800F663A` | Gauge A current = 0 |
| `g_gnd_gauge_b_max` | `0x800F663C` | Gauge B max |
| `g_gnd_gauge_c_max` | `0x800F6640` | Gauge C max |
| `g_gnd_gauge_c_cur` | `0x800F6642` | Gauge C current |
| `g_gnd_gauge_d_max` | `0x800F6644` | Gauge D max |

A parallel cluster exists for Mario character at `g_mario_voice_data`
(`0x800F5328` — see [sound.md](sound.md)).

## Practice-lesson cluster (2026-05-17)

The practice mode tracks lesson progress + move-unlock state through
several state cells, driven by `func_80033DF4` (lesson-progress
checker) and `func_80033FE4` (move-unlock dispatcher).

### Lesson-progress state

| Symbol | Address | Role |
|--------|---------|------|
| `g_practice_lesson_progress` | `0x800A38E2` | Progress counter (0x64 = complete) |
| `g_practice_lesson_subcounter` | `0x800A38E9` | Sub-counter (< 3 triggers end-of-lesson) |
| `g_practice_p1_move_unlock_pending` | `0x800A36F0` | bool: P1 unlocked a move |
| `g_practice_p2_move_unlock_pending` | `0x800A3781` | bool: P2 unlocked a move |
| `g_practice_unlock_anim_id` | `0x800A38A4` | Animation ID (6/7=P1, 8/9=P2) |
| `g_practice_mode_dispatch_idx` | `0x800A3834` | g_module_func_tbl index |

### Lesson init params (set by func_8001C444)

| Symbol | Address | Default |
|--------|---------|---------|
| `g_practice_lesson_size_a` | `0x80102778` | 0x800 |
| `g_practice_lesson_size_b` | `0x8010277A` | 0x800 |
| `g_practice_lesson_char_id` | `0x8010277C` | 1 |
| `g_practice_lesson_class` | `0x8010277D` | 0x10 |
| `g_practice_lesson_slot_0` | `0x8010277E` | 0 |
| `g_practice_lesson_slot_1` | `0x8010277F` | 0 |
| `g_practice_lesson_state` | `0x80102780` | 0 |
| `g_practice_lesson_init_param` | `0x80102781` | 0 |
| `g_practice_lesson_count_a` | `0x80102784` | 0xC |
| `g_practice_lesson_count_b` | `0x80102785` | 4 |
| `g_practice_lesson_flag_a/b` | `0x80102786/87` | 0 |

### Lesson runtime params (fetched from `cpu_practice_honmokuroku_data_tbl`)

| Symbol | Address | Role |
|--------|---------|------|
| `g_practice_lesson_param0` | `0x800A38DE` | Lesson table fetch via `&D_8008EC24 + char-class offset` |
| `g_practice_lesson_param1` | `0x800A38EC` | Lesson table[1] |
| `g_practice_lesson_param2` | `0x800A38ED` | Lesson table[2] |
| `g_practice_lesson_param3` | `0x800A38EE` | Lesson table[3] |

### Cross-reference with mode handlers

Practice mode flows through several `g_module_func_tbl` entries
(see [main_loop.md](main_loop.md) for full table):

- `mode_handler_18_UnlockAnimDispatch` (`0x8003C040`, mode 0x12) is
  the unlock-celebration dispatcher.  Reads `g_practice_unlock_anim_id`
  to choose the specific character/move combo for the animation.
- Mode 0x1A (`scene_teardown_80035DC8`) is the post-celebration teardown.

### Move-enable bitmap

`g_file_disc_size` at `0x80106A50` is **MISNAMED** — it's actually the
per-character move-enable bitmap.  `func_80033DF4` queries it as
`&D_80106A50` and tests bits like `0x20`, `0x10000`, `0x1000000`,
`0x4000000` to decide which characters can use which moves.  See
[../naming/MISNOMERS.md](../naming/MISNOMERS.md).

## Cross-references (recent_naming_findings.md addendum 2026-05-17)

The post-fight WIN-animation sound trigger script lives in
code6cac_c2.c:870-899.  Drives the "katinuki" (win) sequence that
plays SFX cues at exact frame counts and transitions back to the
title menu:

- [§15 Win-animation sound trigger cluster](recent_naming_findings.md#15-win-animation-sound-trigger-cluster-d_8008eac0-0x8008eb28)
  — `g_winanim_per_stage_intro_frame[34]` (per-stage intro SFX frame
  targets, mostly 130f or disabled), single triggers at frames
  155/159/160/198 for callouts/special/fanfare, particle effect
  spawned at +0/-800/+0 offset (upward) from `D_800A3818`.  Total
  win-animation playback ≈ 200 frames (3.3s).  Per-character SFX
  banks are 40-entry (0x28) blocks indexed by `40*p + base_id`.
