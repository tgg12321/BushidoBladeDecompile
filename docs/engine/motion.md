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
   per-bone local matrix from rotation+translation
       |       (GTE matrix setup lives in the display-list opcode handlers
       |        dispatched by func_8004A4E0 — NOT in the
       |        misnamed calc_loc_mat_fw* symbols; see the naming note below)
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

### `motion_SetMotion` (code6cac_c_mid.c:867)

The "set/transition motion" decision function. Run by `saRobDraw`
(now `func_80035E38`, `code6cac_b2_post.c`) — the gameplay-loop body. It examines:

- `D_800A3207` — current motion-system state (1..4 represent boot, normal,
  hit-stop, special, reset)
- `D_800A334C` — the 90-frame display countdown of a memory-card message (0x5A down to 0)
- `D_800A3354` — special state flag (transition pending)
- `D_800A31FC` — a 0/1 flag of the memory-card status flow
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
- `motion_LoadPreCalcData_8005B98C` (`text1b_tu1b.c:3886`) — calls
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

`motion_shift_check_m_hit_stop` (`code6cac_c_mid.c:586`) — "should the
motion shift due to mid-hit-stop?" One of several per-frame "should I
interrupt the current motion?" checks. Other variants:
- `motion_shift_check_e_kawashi` (asm only, 0x80030B10) — "should I evade"
- `motion_ShiftControl` (text1b_tu1d.c:1317) — the main shift dispatcher

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

## Naming note — the `calc_loc_mat_fw_*` symbols are MISNOMERS

The Kengo-derived name `calc_loc_mat_fw` ("calculate local matrix forward")
attached to three UNRELATED BB2 functions, none of which is a per-bone
matrix builder (MISNOMERS.md pass-6; `named_syms.txt` MISNAMED flags;
verified against the bodies 2026-07-13):

- `calc_loc_mat_fw` (0x8002AB08, now `func_8002AB08` in `code6cac_b_tu2.c`, ~1074
  insns) — a scratchpad-staged per-frame fighter/camera state processor
  (walks the `D_80101EC8` fighter table at stride 0x44C, stages rows
  through scratchpad 0x1F8000xx, computes midpoints and bounding min/max,
  calls `special_camera_Init`). Pass-6 replacement name:
  `gpu_dma_schedule_8002AB08`.
- `calc_loc_mat_fw_8004A940` (0x8004A940) — a u16 stream/opcode
  DISPATCHER (lhu; advance; jalr through a handler table). The GTE
  matrix work (`ctc2`/`mvmva`) lives in its dispatched handlers. This —
  not 0x8002AB08 — is what the display-list walker
  `func_8004A4E0` calls (5 jal sites, text1b.c).
- `calc_loc_mat_fw_80055B60` (0x80055B60, text1b.c) — enemy targeting /
  angle-difference helper (0xFFF-truncated deltas, ±0x800 angle wrap,
  `single_game_getEnemyCharId`).

The actual per-bone local-matrix construction is performed inside the
opcode handlers dispatched by 0x8004A940 during the
`func_8004A4E0` display-list walk; no single "calc_loc_mat"
function exists under that name in BB2.

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

The bone-tree walk lives in the asmfix-bridged functions in `text1b.c`
(the ~0x10558-0x11227 address range). It is decompiled to C but some of those
functions still carry cheats — check `engine/queue.json` for their status.

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
- The three functions carrying the misnomer `calc_loc_mat_fw_*` (see the
  naming note above — a fighter/camera state processor, a u16 opcode
  dispatcher, and a targeting helper; 2 of 3 are 1000+ instructions)
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
| 1 | `func_80064ED8` | `0x80064ED8` |
| 2 | `func_80064F20` | `0x80064F20` |
| 3 | `func_80064F68` | `0x80064F68` |
| 4 | `func_80064FB4` | `0x80064FB4` |
| 5 | `func_80065000` | `0x80065000` |

Each init reads 3 words through `D_800A347C` (the current command
block's three-word copy, 51268.c:103),
writes them to its state block, stores 1 into its `D_800F10D0` slot,
resets the per-id counter.

### Dispatch-offset table — `D_800F10D0`

28-entry s32 table (`func_80060C60` zeroes 28 entries) of dispatch
offsets (values 0, 1, 2), not flags: `func_80060A68` / `func_80060B70`
add `D_800F10D0[idx]` to `D_8009BA60[idx]` to pick a
`chractar_use_pset_combo_id_table` entry and call it, and
`func_80061064` runs `func_80060B70` for every idx whose
`D_800F1150[idx]` is nonzero. The slots the init functions store 1
into: `func_80064ED8` [5] (`D_800F10E4`), `func_80064F20` [6]
(`D_800F10E8`), `func_80064F68` [9] (`D_800F10F4`), `func_80064FB4`
[10] (`D_800F10F8`), `func_80065000` [11] (`D_800F10FC`); also
`func_800645B0` [7] (`D_800F10EC`) and `func_80062FEC` [8]
(`D_800F10F0`). `D_800F10E0` is slot [4]. The id number of an init
function is not its slot index.

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

## Memory-card setter cluster (2026-05-17)

Not motion state: these cells were once read as a "motion shift"
cluster, but every access belongs to the memory-card state machine
`func_800383A4` (28708.c), which the four setters below arm
(28708.c:457-485). Each sets the status code and the operation
selector, zeroes the retry counter and sets the state to 1.

| Symbol | Address | Role |
|--------|---------|------|
| `D_800A379E` | `0x800A379E` | Operation status / result code; `func_80038734` returns it |
| `D_800A37C8` | `0x800A37C8` | Operation selector (0 load, 1/2 write, 3 `memcard_Format`) |
| `D_800A31F4` | `0x800A31F4` | State (1 start, 3/5/7 write/read/format, 4/6 wait on `memcard_PollSwEvents`, 0 idle) |
| `D_800A3814` | `0x800A3814` | Retry counter |
| `D_800A38CC` | `0x800A38CC` | Write-path flag; zeroed only by `func_8003879C` / `func_800387C0` |

| Function | Address | `D_800A379E` | `D_800A37C8` |
|----------|---------|--------------|--------------|
| `func_8003877C` | `0x8003877C` | 4 | 0 |
| `func_8003879C` | `0x8003879C` | 1 | 1 |
| `func_800387C0` | `0x800387C0` | 1 | 2 |
| `func_800387E8` | `0x800387E8` | 9 | 3 |

## Cross-references (naming pass 2026-05-17; full traces at `pre-slim-2026-10-01:docs/engine/recent_naming_findings.md`)

One cluster from the placeholder-refinement pass:

- §20 Flare slot pool (12 slots)
  — `D_800F0E38` (`Unk800F0E38Record[12]`, x / y / z words at +0 / +4 /
  +8) + `D_800F0BEC` (12 × s16 per-slot age: 0 when `func_80062FEC`
  takes the slot, +1 per `func_80063084` pass). Allocated by
  `func_80062FEC` (51268.c) via the `g_particle_slot_bitmap_plus_4`
  busy bitmap and drawn by `func_80063084`. Pool A (32 slots at
  `0x800F0D78` with random spread) is the parallel pool.
