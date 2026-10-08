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

Each move is described by an opcode-stream stored in the character's data.
The streams are NOT
plain animation timelines — they're tiny bytecode programs the AI/move
selector advances each frame.

The decoder is `cpu_get_dist_2(u8 *a0)` (`code6cac_b_tu2.c:5519`). It walks the
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

`cpu_check_same_dir_timer` (`code6cac_b_tu2.c:6005`) is the same kind of decoder
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
(`code6cac_b_tu2.c:4863`) commits it:

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

### `cpu_check_tubazeri` (`code6cac_b_tu2.c:4325`) — sword clash

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

### `coli_check_circle_hit_line` (`code6cac_b_tu2.c:4398`)

A variant that tests a circle (a weapon arc swept over a frame) against a
line segment (the opposing weapon's reach over the same frame). Uses the same
scratchpad scheme to compute deltas, runs the GTE cross product, and returns
which limb/axis intersected.

### `gnd_land_hit_char_tsuba` (`text1b_tu1b.c:4733`)

Ground/weapon-vs-body test. Called from the main loop's gameplay handler.
Returns the new active fighter struct pointer if a hit was detected; the
caller uses this to start the hit-reaction animation.

### `gnd_land_hit_char_die_main` (now `func_800422BC`, `text1a_post.c`)

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
mostly in the `text1b*.c` files (`sound.c` was merged into `text1b.c`):

- `saTan0Init`, `saTan1GaugeInit`, `saTan2GaugeInit_80077B20` —
  initialization functions for the various gauge bar variants.
- `saTan3GaugeMain_8006A564`, `saTan4GaugeMain`, `saTan1GaugeMain` — per-frame
  update functions.
- `saTan5TakeAnim2_2` (`main.c:143`) — handles the volume/SE pacing during
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
- `katinuki_game_setData_8003D2C4` (called from `func_80016D78`) — initializes
  the katinuki state at boot.
- `katinuki_game_getMyWeaponId` (`code6cac_b_tu2.c:63`) — looks up the
  fighter's weapon id for katinuki damage calculation.
- `md_game_check_change_main_mode_katinuki` (`main.c:3152`) — checks
  whether the player has triggered katinuki conditions this frame to
  transition out of the normal game-mode dispatch.

The 5 `katinuki_game_get_katinuki_max_num_*` are presumably one each for the
different game modes (Single, Slash, Story, Practice, VS).

## Hit-pause helpers — `coli_HitPauseKatana`

`coli_HitPauseKatana` (`main.c:2597`) and `coli_HitPauseKatana_2`
(`main.c:2787`) are the misnamed "Hit Pause" functions — they're actually
the **SPU voice-key allocator** (despite the `coli_` name). The Kengo
rename collided two functions; the real implementation manages SPU voice
slots for sound effects during hit pauses (the engine slows down the SPU
voice playback when a hit lands, hence the name).

The function manages a free-list of voice keys via the `g_spu_voice_key_a/b/c`
globals. See [sound.md](sound.md) for details.

## Where unmatched asm still hides in combat

- `action_check_defense2` (`0x80089EB0`) — combat block/defense check.
  (NB: `0x80086130`, formerly listed here as `action_check_defense`, is
  actually `char_store_pos_pair_80086130` — a bounds-checked store of a
  ×129-scaled s16 pair into 16-byte slot records, called from
  `obj_InitChars` during character setup; its inverse getter
  `func_80086080` divides by 129. Not a combat function — see
  `named_syms.txt` supersede note.)
- `action_CheckHitZangeki` (0x800863DC) — slash-hit test per its Kengo name;
  the address is now `_SsVmFlush` (PsyQ libsnd, `main.c`).
- `coli_calc_motion` (`0x8003D888`, `_2` at `0x8008ACD0`) — motion vs collision
  resolver.
- `damage_CalcHitDamage` / `damage_CalcHitDamage2` (`0x8003880C`, `0x80082A14`)
  — actual damage calculation.
- Big chunks of the `code6cac_b.c` `func_8003XXX` cluster (over 100 functions
  that wrap the AI/move dispatch).

## Ground / arena init (2026-05-17)

`gnd_init_8001B294(p1_char_ptr, p2_char_ptr)` (`code6cac_tu2.c:841`) sets
up the per-round arena state when a fight starts.  It initialises the `Rec44` record `D_800F6608`:

- `unk_00`: the fighters' midpoint (the record's only midpoint write; other
  writers copy a fighter's position, store constants or interpolate)
- `h30[0]` / `h30[1]`: cell x / z set to 100 (the out-of-range reset), the
  middle element to 0
- `unk_10.vy`: `0x400 - ratan2` of the fighters' separation (9F9C.c:798-800)
- `b1E`: cleared (9F9C.c:803)

| Symbol | Address | Field / role |
|--------|---------|------|
| `D_800F6608` | `0x800F6608` | `unk_00.x`; `camera_CalcEye` places the eye behind `unk_00` |
| `D_800F660C` | `0x800F660C` | `unk_00.y` |
| `D_800F6610` | `0x800F6610` | `unk_00.z` |
| `D_800F6618` | `0x800F6618` | `unk_10.vx`: the X rotation `camera_CalcEye` applies |
| `D_800F661A` | `0x800F661A` | `unk_10.vy`: the Y rotation; also the angle `func_800325E0` pans by |
| `D_800F661C` | `0x800F661C` | `unk_10.vz`: the Z rotation (written 0) |
| `D_800F6620` | `0x800F6620` | `w18`: the eye's distance from `unk_00` |
| `D_800F6626` | `0x800F6626` | `b1E`: hysteresis flag on `w18`, recomputed by `func_8001A820` |
| `D_800F6638` | `0x800F6638` | `h30[0][0]`: cell x (`func_8003F3D4`'s column); 100 = out of range |
| `D_800F663A` | `0x800F663A` | `h30[0][1]`: written 0; no role shown |
| `D_800F663C` | `0x800F663C` | `h30[0][2]`: cell z (`func_8003F3D4`'s row); 100 = out of range |
| `D_800F6640` | `0x800F6640` | `h30[1][0]`: cell x; 100 = out of range |
| `D_800F6642` | `0x800F6642` | `h30[1][1]`: written 0; no role shown |
| `D_800F6644` | `0x800F6644` | `h30[1][2]`: cell z; 100 = out of range |

`D_800F5328` (`0x800F5328`) is a record of the same `Rec44` type: position,
rotation and distance fed to `func_80046BF4` / `camera_CalcEye`;
`g_listener_cam` points at it (9F9C.c:2100).

## `func_80033DF4` / `func_80033FE4` cluster (2026-05-17)

`func_80033DF4` steps a counter through a table and, at 0x64, sets two
flags from bits of `D_80106A50.unk_00`; `func_80033FE4` turns the flags
into a selector and the next handler index. No access shows a practice
mode, a lesson or an unlock.

### Counter and flag state

| Symbol | Address | Role |
|--------|---------|------|
| `D_800A38E2` | `0x800A38E2` | Step counter: table row index, end branch at 0x64 |
| `D_800A38E9` | `0x800A38E9` | Insertion rank 0..3 (func_80033D38); < 3 picks mode 0x1A |
| `D_800A36F0` | `0x800A36F0` | 0/1: bit 0x20 / 0x10000 was still clear |
| `D_800A3781` | `0x800A3781` | 0/1: bit 0x1000000 / 0x4000000 was still clear |
| `D_800A38A4` | `0x800A38A4` | Selector 0..9 (6/7 from D_800A36F0, 8/9 from D_800A3781) |
| `D_800A3834` | `0x800A3834` | D_8008D090 index (main loop) |

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

### Row params (`func_80033DF4` copies a row of `D_8008E778`)

| Symbol | Address | Role |
|--------|---------|------|
| `D_800A38DE` | `0x800A38DE` | `D_8008EC24[row][table[0]]` |
| `D_800A38EC` | `0x800A38EC` | table[1]; `func_80041BF4`'s r |
| `D_800A38ED` | `0x800A38ED` | table[2]; `func_80041BF4`'s g |
| `D_800A38EE` | `0x800A38EE` | table[3]; `func_80041BF4`'s b |

### Cross-reference with mode handlers

`func_80033FE4` selects these `D_8008D090` entries
(see [main_loop.md](main_loop.md) for full table):

- `func_8003C040` (mode 0x12) branches on `D_800A38A4` (4..9) and
  indexes `D_8009016C` / `D_8008EA70` with it.
- Mode 0x1A (`scene_teardown_80035DC8`) when neither flag is set and
  `D_800A38E9 < 3`.

### Move-enable bitmap

`g_file_disc_size` at `0x80106A50` is **MISNAMED** — it's actually the
per-character move-enable bitmap.  `func_80033DF4` queries it as
`&D_80106A50` and tests bits like `0x20`, `0x10000`, `0x1000000`,
`0x4000000` to decide which characters can use which moves.  See
[../naming/MISNOMERS.md](../naming/MISNOMERS.md).

## Cross-references (naming pass 2026-05-17; full traces at `pre-slim-2026-10-01:docs/engine/recent_naming_findings.md`)

`func_8003CF84` (2B344.c) compares the counter `D_800A37B8` with s16
thresholds and acts on each match (sounds go through `func_8005C650`):
- §15 threshold cluster — `D_8008EAC0[34]` (indexed by the fighter's
  `unk_0A` class index; mostly 130, else -1, 135, 230 or 0) queues
  `40*p + 0x2D`; the single thresholds 155/159/160/198
  (`D_8008EB04`..`D_8008EB0A`) queue `40*p + 0x31`, `40*p + 0x36`, 0x53 or
  0x2B, and 0x71; at 159 (`D_8008EB0C`) a point from `func_80021D10` on
  `D_800A3818`, offset by (0, -800, 0), goes to `func_800618B4`.  The
  per-player sound ids are 40-entry (0x28) blocks indexed by
  `40*p + base_id`.
