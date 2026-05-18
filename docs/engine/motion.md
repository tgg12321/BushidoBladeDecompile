# Motion / Animation

BB2 fighters are skeletal models — each character has a bone hierarchy
("rig"), and animations ("motions") drive per-frame bone transforms. The
data lives on disc in `.BBM` files under `disc/MOTION/` (one per character:
Y123.BBM, N123.BBM, K123.BBM, T123.BBM, S123.BBM, etc. — see strings at
`0x8001036C..0x800103C0`).

A "waza" is a combat move, and `motion_SetMotion`/`motion_SetExMotion` are
the functions that select which waza animation plays next on which fighter.

## Pipeline overview

```
disc/MOTION/X123.BBM   (per-character bone+motion bundle)
       |
       v
   bios_FileRead via libcd  ----> MotDataBaseAddress (0x80104F38)
       |
       v
   motion_LoadPreCalcData_*  (extract & sort frame data into RAM)
       |
       v
   motion_SetMotion / motion_SetExMotion  (pick waza index)
       |
       v
   per-frame: motion_GameCalcMotion + motion_shift_check_*
       |       (interpolate bones, advance frame counter)
       v
   calc_loc_mat_fw* (per-bone local matrix from rotation+translation)
       |       (often uses GTE for rotation composition)
       v
   bone-tree walk: accumulate world matrix at each bone
       |
       v
   GTE RTPT/RTPS for vertex projection per polygon
       |
       v
   GPU OT (Ordering Table) primitive insertion
```

## On-disc format (BBM files)

BBM = "Bushido Blade Motion bundle". Loaded by `file_LoadAll` in `ings.c`,
the file is a single sequential block; the engine parses it after load into
`MotDataBaseAddress`-relative tables. The exact layout has not been
exhaustively documented in C yet, but observed access patterns suggest:

- Header: file size, table-of-contents offsets
- Bone-tree definition: parent index per bone, base transform
- N "motion" entries, each:
  - Number of frames
  - Per-frame keyframe data for each bone: (rotation Euler vec3, translation
    vec3, often packed as int16)
- Frame timing / blending hints

Strings naming the files (`g_str_bbm_y123` etc.) live at `0x8001036C` and
imply at least 5 distinct character files (Y, N, K, T, S — initials of
character names like "Tatsumi", "Mikado", "Hongoh", etc.).

## RAM data structures

### `MotDataBaseAddress` (0x80104F38)

Base pointer to the loaded motion data region. Set during `file_LoadAll`
chain; read by every `motion_*` function. Each fighter's active motion is
indexed off this base.

### Per-fighter motion state (within fighter struct, offset 0x50)

The "active move pointer" field (offset 0x50 of the fighter struct) points
into `MotDataBaseAddress`-relative tables. The 0x12 field is the current
move ID. The 0x58 field is the current command-stream pointer (see
[combat.md](combat.md)).

### Active-motion array `D_80106A78`

12 slots * 0x64 bytes each — the per-fighter "currently executing moves"
list. Each slot tracks:
- 0x00-0x07: slot status + move type
- 0x08-0x0F: timing
- 0x2C-0x34: world position of the swept volume
- 0x38-0x40: end position of the swept volume
- 0x44-0x4C: rotation/extension parameters
- 0x54-0x60: hit response data
- 0x88: sentinel = -1 if slot vacant

`coli_hit_body_weapon` fills these in when a move is committed; the
hit-detection helpers walk them every frame.

## The `motion_*` functions

### `motion_Open` / `motion_Close` (ings2.c:525, 543)

These are MIS-NAMED — they're actually the engine-wide CTOR/DTOR runners.
The Kengo rename labelled them `motion_Open/Close` because the matching PS2
function happened to be in the motion subsystem (Lightweight likely
re-used the same crt mechanism for both). What they actually do:

- Walk the function-pointer array at `D_8008D070`
- Call each non-null function pointer; the loop terminator is `D_00000000`
  (the count is encoded as the count *minus one* in `D_00000000`'s value
  via raw inline-asm pointer arithmetic — a Lightweight idiom)
- `motion_Open` runs the CTORs once at boot; `motion_Close` runs the
  symmetric DTORs at shutdown

Neither actually touches motion data. The real motion-init for the engine
runs during `file_LoadAll` of the per-character BBMs.

### `motion_SetMotion` (code6cac_c_mid.c:883)

The "set/transition motion" decision function. Run by `saRobDraw`
(`code6cac_b2.c:296`) — the gameplay-loop body. It examines:

- `D_800A3207` — current motion-system state (1..4 represent boot, normal,
  hit-stop, special, reset)
- `g_motion_play_countdown` (D_800A334C) — frame counter (decrements 0x5A=90 down to 0)
- `D_800A3354` — special state flag (transition pending)
- `g_damage_debug_enabled` (D_800A31FC) — "no input" flag
- `D_80102794` — pad input mask
- `func_80038734()` — gets the current "selected motion index"

Based on these inputs and the input pad's button bits (`0x100010` = SELECT
combined p1+p2 mask, `0x400040` = CROSS), it computes:

- `sel`  — the motion to pass to `func_8006BEC4(sel, sel2)` to commit
- `sel2` — secondary parameter (often `D_800A3350`, a sub-state)

Returns 0 (continue normal flow) or sets `D_800A3834 = 8` (jump to title)
if a critical state was reached. This is the function that decides "play
attack motion 6", "play defend motion 0xC", etc.

The switch-on-v0 in `motion_SetMotion` is essentially a state machine —
each case represents what the fighter was doing in the previous frame, and
chooses the next motion based on the input. v0 values 0..17 represent move
classifications:

| v0 | meaning (inferred) |
| --- | --- |
| 0 | idle / no current motion — pick a new one based on `D_800A3350` |
| 1 | committed-attack motion in progress |
| 2 | secondary attack — `sel = 8` |
| 3 | tertiary attack — `sel = 9` |
| 7 | mid-recovery — `sel = 5` |
| 8 | reserved (interruption / continue) |
| 9, 11 | block / wind-down — `sel = 0xC` |
| 10 | menu-pause variant |
| 12 | end-of-motion — `sel = 0xF` |
| 13, 17 | weapon-clash hold — `sel = 6` |

### `motion_LoadPreCalcData_*` family

There are FIVE functions with this name at different addresses, each a
different stage of the load pipeline:

- `motion_LoadPreCalcData_80037F08` (`code6cac_c_mid.c:184`) — wraps a
  generic file-load helper for `D_800109C8`. Calls `func_80079A30` and
  `func_80078A28` (libcd file I/O).
- `motion_LoadPreCalcData_8005B98C` (`text1b.c:12292`) — calls
  `saFidLoad(a0, 8)` then `saFidLoad(a0, 4)`. "Fid" probably means "file
  index" — these load motion-tables 8 and 4 from a specific FID.
- `motion_LoadPreCalcData_8007DC68` (`display.c:1013`) — display-side
  variant.

The Kengo name `motion_LoadPreCalcData` indicates "load and pre-compute
motion data" — i.e., decode the per-frame bone transforms into a fast-
lookup table at load time rather than per-frame.

### `motion_SavePreCalcData_*` family

Three variants in `display.c:3484..3492` and one in `code6cac_c_mid.c:1560,
1618`. Symmetric to `LoadPreCalcData` — save the pre-computed data back to
RAM after a state change. Currently all four `display.c` ones are tiny
stubs (1-2 lines).

### `motion_shift_check_*` family

`motion_shift_check_m_hit_stop` (`code6cac_c_mid.c:607`) — "should the
motion shift due to mid-hit-stop?" One of several per-frame "should I
interrupt the current motion?" checks. Other variants:
- `motion_shift_check_e_kawashi` (asm only, 0x80030B10) — "should I evade"
- `motion_ShiftControl` (text1b.c:16317) — the main shift dispatcher

`motion_ShiftControl` is what fires when the engine needs to switch motions
mid-frame (e.g., a sword clash interrupts an attack).

### `motion_GameCalcMotion` (0x8002872C, asm-only)

Per-frame motion frame advance — the "tick" function. Not yet decompiled
to C.

### `motion_CheckSituation` (0x800477E8, asm-only)

Per-frame "what state is the motion in" query — returns the v0 value
that `motion_SetMotion` switches on.

### `motion_SetExMotion` (referenced but not seen in src/)

The Kengo name suggests "Set Extension Motion" — for motions that don't
come from the main character BBM but from a separate "extension" file (a
special winning pose, victory animation, etc.). Sets the fighter's motion
to an "extension" type tracked separately from the main waza.

## Local matrix calculation — the `calc_loc_mat_fw_*` family

`calc_loc_mat_fw` (full name "calculate local matrix forward") is the
per-bone matrix builder. It takes a bone's rotation Euler angles and
translation, and produces a 3x3 rotation matrix + 3-element translation
ready for GTE multiplication.

Variants observed:
- `calc_loc_mat_fw` (`code6cac_b.c:750`) — empty stub in C; the real
  implementation is in `asm/funcs/calc_loc_mat_fw.s` (1074 instructions,
  the largest function in the codebase). Hand-unrolled GTE pipeline that
  builds 3 axis rotations and concatenates them. Kengo's
  `se_fc/calc_loc_mat_fw` was matched but at -38 / 3.5% no-affinity
  fallback — so the C is not yet matched.
- `calc_loc_mat_fw_8004A940` (text1b.c, asmfix bridge) — same function at
  a different address, called from a different entry point.
- `calc_loc_mat_fw_80055B60` (text1b.c:11498) — third variant, currently
  asmfix-bridged.

These are the heart of the per-frame bone-tree walk. They consume:
- Bone's rotation (3 Euler angles, typically 12-bit fixed point)
- Bone's translation (3 s32 world units)
- Optionally a parent matrix to compose with

And produce:
- A 4x3 matrix in MATRIX-struct format (3x3 rotation + 3-vec translation)

The function is so large because it manually expands the rotation matrix
elements (sin/cos products) and uses interleaved GTE ops + CPU arithmetic
for pipelining.

## Bone hierarchy walk

The fighter's bone hierarchy is walked per-frame for every visible fighter
(typically 2 player fighters + their weapons + AI fighters in some modes).
Each bone's world matrix is the parent's world matrix * the bone's local
matrix. The bones are stored in a topological order so a single forward
pass suffices.

After the world-matrix accumulation, the engine submits the per-poly draw
calls to the GTE for vertex projection (`gte_rtpt()` for triangle vertices,
`gte_rtps()` for single points), then writes the projected screen-space
coords into a GPU primitive in the OT.

The bone-tree walk is not yet decompiled — it lives in the asmfix-bridged
functions in `text1b.c` (the 0x10558-0x11227 range per `SUBSYSTEM_MAP_*`).

## Pre-calc cache (the `myRobGenei*` family)

`myRobGeneiOpen` / `myRobGeneiDraw` / `myRobGeneiDraw2` / `myRobGeneiDraw3`
(scattered, see `named_syms.txt`) are the "shadow afterimage" effect functions
— they cache previous frames' bone positions for the "zanzou" / motion-blur
visual effect when an attack lands.

The "Genei" = phantom/afterimage — Kengo names them `myRob` (player-character)
"phantom". The functions render a ghost copy of the fighter at their
previous frame's pose, blended into the framebuffer.

## Where unmatched motion asm still hides

- `motion_GameCalcMotion` (`0x8002872C`) — the per-frame frame advancer
- `motion_CheckSituation` (`0x800477E8`) — situation check helper
- The `calc_loc_mat_fw_*` family at `code6cac_b.c:750`, the asmfix-bridged
  `text1b.c:11498`, and 2-3 more variants — these are the largest single
  functions in the binary (1000+ instructions each)
- Much of the `text1b.c` 11421-13840 range (motion utility helpers,
  ex-motion state tables, etc.)
- `motion_make_table` — now identified at `0x80082D34` (was previously
  marked "referenced in display.c:853" without a known location)

## Motion-ex per-id state arrays (2026-05-17)

The engine maintains 8 parallel "motion-ex" slots (IDs 0-7), each with
a 12-byte state block plus per-id flag and counter cells.  Each slot
is initialized by its own dedicated `motion_ex_Init_idN` function.

### State block table — `0x800F0CA0`

Stride 12 bytes per ID, 8 IDs total:

| ID | Pos field | Extra field | Data ptr field |
|----|-----------|-------------|----------------|
| 0  | `g_motion_ex_state_id0_pos` (0x800F0CA0) | _id0_extra (0x800F0CA4) | _id0_data_ptr (0x800F0CA8) |
| 1  | _id1_pos (0x800F0CAC) | _id1_extra (0x800F0CB0) | _id1_data_ptr (0x800F0CB4) |
| 2  | _id2_pos (0x800F0CB8) | _id2_extra (0x800F0CBC) | _id2_data_ptr (0x800F0CC0) |
| 3  | _id3_pos (0x800F0CC4) | _id3_extra (0x800F0CC8) | _id3_data_ptr (0x800F0CCC) |
| 4  | _id4_pos (0x800F0CD0) | _id4_extra (0x800F0CD4) | _id4_data_ptr (0x800F0CD8) |
| 5  | _id5_pos (0x800F0CDC) | _id5_extra (0x800F0CE0) | _id5_data_ptr (0x800F0CE4) |
| 6  | _id6_pos (0x800F0CE8) | _id6_extra (0x800F0CEC) | _id6_data_ptr (0x800F0CF0) |
| 7  | _id7_pos (0x800F0CF4) | _id7_extra (0x800F0CF8) | _id7_data_ptr (0x800F0CFC) |

(All entries named `g_motion_ex_state_idN_*`; abbreviated above for table width.)

### Per-id init functions

| ID | Init function | Address |
|----|--------------|---------|
| 1 | `motion_ex_Init_id1` | `0x80064ED8` |
| 2 | `motion_ex_Init_id2` | `0x80064F20` |
| 3 | `motion_ex_Init_id3` | `0x80064F68` |
| 4 | `motion_ex_Init_id4` | `0x80064FB4` |
| 5 | `motion_ex_Init_id5` | `0x80065000` |

Each init reads 3 fields from `g_text1b_render_buf_ptr` (D_800A347C),
writes them to its state block, sets the per-id flag, resets the
per-id counter.

### Per-id flag table — `g_motion_state_flag_table` (`0x800F10D0`)

24-entry s32 flag table; index from `*(u16*)D_800A3468` (the text1b
render state).  The previously-named `g_motion_ex_flag_table_4`
(`0x800F10E0`) is actually +0x10 from the true base — it's index 4
of this larger table.  Per-id flag aliases for IDs 1-3 exist
(`g_motion_ex_flag_id1/2/3` at +4/+8/+0xC); IDs 4-7 don't follow
strict +4 stride (see init functions for actual mapping).

### Per-id counter table — `g_motion_ex_counter_table_base` (`0x800F0BA8`)

2-byte stride per motion ID; tick counter (reset to 0 in init):

| ID | Counter address | Default |
|----|----------------|---------|
| 1 | `g_motion_ex_counter_id1` (0x800F0BAA) | 0 |
| 2 | `g_motion_ex_counter_id2` (0x800F0BAC) | 0 |
| 3 | `g_motion_ex_counter_id3` (0x800F0BAE) | 0x40 (64 frames) |
| 4 | `g_motion_ex_counter_id4` (0x800F0BB0) | 0x40 |
| 5 | `g_motion_ex_counter_id5` (0x800F0BB2) | 0 |

Plus `g_motion_ex_counter_p1` (0x800F0BC0) and `g_motion_ex_counter_p2`
(0x800F0BC4) for the per-player counters at offsets +0x18 and +0x1C
from the table base.

## Motion-shift state cluster (2026-05-17)

The "motion shift" subsystem (related to motion blending / state
transitions) maintains a small state cluster.  Note that the variables
overlap with the misnomer `pad_FuncAnalog` (which is actually a
memcard state machine, not pad-analog handling) — the variables are
reused across both subsystems.

| Symbol | Address | Role |
|--------|---------|------|
| `g_motion_state_code` | `0x800A379E` | Sub-id (1, 4, 9, 0xA) |
| `g_pad_analog_substate` | `0x800A37C8` | Mode (0=hit-stop, 1/2/3=others) |
| `g_pad_analog_mode` | `0x800A31F4` | State-machine variable |
| `g_pad_analog_frame_counter` | `0x800A3814` | Per-frame tick |
| `g_motion_shift_flag2` | `0x800A38CC` | Zeroed in mode 1/2 setters, NOT in mode 3 or hit_stop |

| Function | Address | Role |
|----------|---------|------|
| `motion_shift_check_m_hit_stop` | (existing) | Sets state to 0 (hit-stop) |
| `motion_shift_set_mode1` | `0x8003879C` | Mode 1, sub-id 1 |
| `motion_shift_set_mode2` | `0x800387C0` | Mode 2, sub-id 1 |
| `motion_shift_set_mode3` | `0x800387E8` | Mode 3, sub-id 9 |

## Cross-references (recent_naming_findings.md addendum 2026-05-17)

One cluster from the placeholder-refinement pass extends the motion-state
data model:

- [§20 Motion-ex pool B (12-slot effect-spawn pool)](recent_naming_findings.md#20-motion-ex-pool-b-12-slot-effect-spawn-pool-d_800f0e380x800f0bec)
  — `g_motion_ex_pool_b_xyz_x/y/z` at `0x800F0E38` (12 slots × 12 bytes
  column-major XYZ) + `g_motion_ex_pool_b_flag` at `0x800F0BEC` (12 ×
  s16). Allocated by `func_80062FEC` (text1b.c:14112) via the
  `g_particle_slot_bitmap_plus_4` busy bitmap. Pool A (32 slots at
  `0x800F0D78` with random spread, used by text1b.c:14236) and Pool B
  (12 slots, no spread, precision effects) are two parallel substrates
  for the motion-ex spawned-effect subsystem.
