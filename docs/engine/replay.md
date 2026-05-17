# Replay System and Special Camera

BB2's replay and "special camera" (the cinematic kill-cam that triggers on
finishing blows) are interconnected: the replay system reuses the camera
data infrastructure that drives the kill-cams. Both are implemented
across `code6cac_b2.c` (camera plumbing, 0x80035438..0x800375EC) and
`text1b.c` (replay-specific helpers, lines 15644-16266).

## What "replay" means in BB2

BB2 does NOT have a generic match-replay feature. Instead, it has:

1. **Special camera** — the cinematic camera that triggers on certain
   game events (winning hits, finishing moves, draws). It plays back a
   pre-authored camera animation from the disc.
2. **Replay camera** — the per-attack "highlight cam" that zooms in on
   important hits during slow-motion playback. Triggered by certain hit
   types or game events.
3. **Demo / title** — the title-screen non-interactive playback. Not
   technically "replay" but uses similar camera machinery.

The naming `replay_camera_*` and `special_camera_*` are Kengo-derived;
in BB2's original source they were probably one unified "cinematic
camera system".

## `SpecialCam` global — the camera data table

`SpecialCam` (`0x8008EC34`) is a table of cinematic camera entries. Each
entry is 8 bytes:

| Offset | Field |
| --- | --- |
| 0 | BCD-encoded CD sector address (where the camera animation lives) |
| 4 | length in bytes |

There's a parallel table `D_8008EC38` (right after `SpecialCam` in
memory). Each `(SpecialCam[i], D_8008EC38[i])` pair describes one
canned camera sequence.

The camera data is **stored on disc**, not in RAM — it's only loaded when
needed (via CD seek to the BCD address, read for length bytes).

## Camera-data load — `special_camera_get_rot_dir`

`special_camera_get_rot_dir(dest)` (`code6cac_b2.c:612`) loads a camera
animation entry from disc:

1. Compute table index via `func_80036EA8(6, 0)`.
2. `func_80080258(2, &SpecialCam[index], 0)` — issue CD read command for
   that entry's sector address.
3. `func_800372F4(0x800, sp_buf, 0x80)` — wait for the read to complete
   (1 sector = 0x800 bytes).
4. If read failed, retry from step 2.
5. Copy 0x40 bytes from the buffer's offset 0x10 to `dest` (this is the
   camera transform data — 8 16-byte Quads + 1 12-byte Triple).
6. Issue a follow-up read for the next sector (`cdrom_FramesToBcd(... + 1, ...)`).
7. `func_800372F4(dest[3], dest[2], 0x80)` — wait for the data read.

`dest` becomes a struct of:
- `[0..3]` — first Quad: header info
- `[4..7]` — second Quad: more header  
- `[2]` = data pointer
- `[3]` = data length

The function uses an unusual register-pinning pattern (`s2=index`,
`s3=cam_base`, `s4=0x80`, `s5=copy_end`) — this is hand-tuned for the
PsyQ asm output.

## Replay camera init — `replay_camera_Init`

`replay_camera_Init(a0, a1)` (`code6cac_b2.c:417`) starts a replay
camera sequence:

1. If a replay is already running (`D_80101E62 != 0`), bail.
2. `D_80101E60 = a0` (sequence ID)
3. `D_80101E6C = SpecialCam[a0]` (CD address of camera data)
4. `D_80101E70 = D_8008EC38[a0]` (data length)
5. `D_80101E7C = a1` (callback / target)
6. `D_80101E62 = 2` (initial state — "loading")
7. `D_80101E78 = ceil(length / 0x800)` (sector count to read)

State machine variables in the `D_80101E5C..D_80101EA0` range:

| Variable | Use |
| --- | --- |
| `D_80101E60` | current replay sequence index |
| `D_80101E62` | replay state (0=idle, 0x10=loaded, 2=loading, etc.) |
| `D_80101E68` | "stopped" flag |
| `D_80101E6C` | CD sector pointer |
| `D_80101E70` | total bytes to read |
| `g_replay_frame_offset` (D_80101E74) | frame counter |
| `D_80101E78` | sectors remaining |
| `D_80101E7C` | per-sequence callback |
| `D_80101E80..D_80101EA4` | per-sector load state machine |

## Replay camera attack — `replay_camera_attack`

`replay_camera_attack(arg0..arg3)` (`text1b.c:16187`) is the slow-motion
attack camera handler. Currently asmfix-bridged.

`replay_camera_get_attack_number(a0..a3)` (`code6cac_c2.c:1552`) is its
companion — looks up the attack number for the current replay.

## Special camera

`special_camera_Exec()` (`code6cac_b2.c:412`) is the per-frame special-cam
ticker. Currently empty in C — bridged.

`special_camera_set_win_cam()` (`code6cac_b2.c:410`) — set up the win
camera. Currently empty.

`special_camera_check_pos_outside_ground_80036E34` (`code6cac_b2.c:448`) —
guard against the special cam going outside the stage bounds. Calls
`replay_camera_Init` to set up a stage-bound cam, then `cdrom_FramesToBcd`
to advance the read pointer by `arg2`.

`marionation_camera_Init_80036064` (`code6cac_b2.c:380`) — XA stream
controller for in-engine cinematics. See [sound.md](sound.md).

`marionation_camera_Init_80037468` (`code6cac_b2.c:651`) — the
"enter cinematic mode" handler:

1. `sys_VSync(0)` — sync first
2. `gpu_SetDispMask(0)` — hide display during transition
3. `gpu_EnableDisplay()` + various setup
4. `pad_Init()` — re-init pad for cinematic
5. `irq_Reset()` — reset IRQ table
6. `EnterCriticalSection`
7. `func_80078948(sp, a0, a1)` — install cinematic handler
8. `sys_Init()` — re-init systems for cinematic mode
9. `file_LoadSoundData()` — reload sound data
10. `gpu_SetDispMask(1)` — re-show display

This is essentially "tear down gameplay, set up cinematic" — a full
state reset.

`marionation_camera_GetMaxFrame()` (`code6cac_b2.c:575`) — returns the
max frame count of the currently-loaded cinematic sequence.

## `marionation_camera_Exec` — the active-cam runner

`marionation_camera_Exec(arg0, arg1)` (`code6cac.c:226`) is the per-frame
camera update for the special/replay system. Currently has heavy use of
scratchpad `0x1F800024..0x1F80002C` for distance vectors and inline GTE
ops:

1. Loads matrix data from `arg0[1]` into GTE registers via raw `ctc2`
   ops (no PsyQ macro — uses `.word` form).
2. Loads more matrix data via raw `mtc2` from `arg0[1] + 0x14`.
3. Calls `getScreenPosition()`.
4. Computes camera-to-target distance vector to scratchpad
   `0x1F800024..0x1F80002C`.
5. Uses the GTE-square-root idiom (`mtc2 $t4, $30; swc2 $31, 0($t4)`) to
   compute `sqrt(distance_sq)`.
6. Rescales the distance vector by a perceptual-loudness factor (the
   `0xC0..0x100` range divided by distance — see `code6cac.c:307-309`).
7. Copies result fields from `arg0[1]` to `arg1+0x14..0x24`.
8. Calls `cpu_check_run_attack(arg1)` — sees if the fighter should
   commit a charging attack based on the camera position.

The scratchpad use is for inter-pass communication: the camera
distance/scale computed in step 4-6 is consumed by `cpu_check_run_attack`
in step 8.

## game_FrameLoop / game_FrameInit

`game_FrameInit` (`code6cac_b2.c:459`) sets up the "demo frame" mode
(replay or title-screen demo):

- `cdrom_SetCallbackB(0)` — clear CD streaming callback
- `func_80035F30(0,0,0,0)` — clear camera state
- `func_80080148()` — `cdrom_ClearIrq()`
- `func_80080390(9, 0)` — stop SE channel XA
- Set state values for "demo mode active"

`game_FrameLoop` (`code6cac_b2.c:471`) is the demo-mode per-frame loop:

```c
while (1) {
    if (func_80036D88() != 0) break;     // D_80101E62 == 0?
    func_8003AA48();                      // per-frame demo work
    special_camera_Exec();                // tick special camera
    if (D_800A3906 != 0) func_8005C6D0(); // ground tick
    gnd_disp_loop_ctrl();                 // draw
    *s0 = *s0 + 2;                        // advance frame counter
    sys_VSync(2);                          // wait 2 vsyncs
}
```

This is a stripped-down version of the main game loop — it runs through
the cinematic until the user presses a button (`func_80036D88` checks
the abort flag).

## Win-camera variants — `replay_camera_rob_back_*`

Multiple variants exist for the "rob falls back, win pose" cinematic:
- `replay_camera_rob_back_loose2` (`code6cac_b2.c:192`)
- `replay_camera_rob_back_loose3` (`text1a_c.c:269`)
- `replay_camera_rob_back_win_near` (asm-only at 0x80046BF4)

Each is a parameterization for a different cinematic angle (loser's POV,
winner's POV, near vs far, etc.).

"rob" likely means "robot" — Kengo's internal term for the
articulated-character renderer that runs both BB2's humans and Kengo's
characters.

## Mode interactions

Replay/special-cam playback happens inside the "demo" mode dispatch
(D_800A3834 = 1 in gameplay can transition to mode 8 when finish detected,
which then sets up the cinematic). The Kengo-named functions
`md_game_check_change_main_mode_default` and `_katinuki` handle these
transitions.

## Where unmatched replay/cam asm still hides

- `special_camera_Exec` (currently empty stub) — the core per-frame
  ticker.
- `special_camera_set_win_cam` (empty stub) — win-pose camera setup.
- `replay_camera_attack` (asmfix-bridged) — slow-motion attack cam.
- Most `func_8003XXXX` helpers between `replay_camera_Init` and
  `marionation_camera_Init_80037468` — uncategorized helpers for the
  cinematic system.
