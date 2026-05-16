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
- `D_800A334C` — frame counter (decrements 0x5A=90 down to 0)
- `D_800A3354` — special state flag (transition pending)
- `D_800A31FC` — "no input" flag
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
- `motion_make_table` (referenced in `display.c:853`) — motion-table
  precomputer
