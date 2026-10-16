# Replay System and Special Camera

BB2's replay and "special camera" (the cinematic kill-cam that triggers on
finishing blows) are interconnected: the replay system reuses the camera
data infrastructure that drives the kill-cams. Both are implemented
across `code6cac_b2_post.c`..`code6cac_b5_post.c` (camera plumbing,
0x80035438..0x800375EC) and `text1b_tu1d.c` (replay-specific helpers).

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

## `g_cd_file_table` and `cdrom_LoadExec` (once `SpecialCam` / `special_camera_get_rot_dir`)

`g_cd_file_table` (`0x8008EC34`, `CdFileEntry[159]`) holds the disc's file
records: a `CdlLOC` start position and a byte size per file (not camera data).
`cdrom_GetFileIndex(group, i)` forms the indices.

`cdrom_LoadExec(dest)` (0x80037348, `src/main/cdrom.c`) loads a PS-X EXE, the file
`cdrom_GetFileIndex(6, 0)` = 156 = `STR\MOVOVL.EXE`:

1. Seek to the file's position (`CdControl(2, ...)`) and read one sector
   (`cdrom_ReadWait(0x800, buf, 0x80)`); on failure start again.
2. Copy the `EXEC` header at offset 0x10 of that sector to `*dest`.
3. Seek to the next sector and read `dest->t_size` bytes to `dest->t_addr`;
   on failure start again from step 1.

Its only caller is `sys_Exec`.

## Replay camera init — `replay_camera_Init`

`replay_camera_Init(a0, a1)` (0x80036D98, now `cdrom_StartRead`, `code6cac_b5_post.c`) starts a replay
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

`replay_camera_attack(arg0..arg3)` (`text1b_tu1d.c:1040`) is the slow-motion
attack camera handler. Currently asmfix-bridged.

`replay_camera_get_attack_number(a0..a3)` (`code6cac_c2.c:1552`) is its
companion — looks up the attack number for the current replay.

## Special camera

No camera code lives at the addresses once named `special_camera_*`: `cdrom_Update`
(0x80036940, `src/main/26940.c`) steps the CD state machine once per call,
`cdrom_UpdateAudio` (0x80036140) is its CD-audio half, and `cdrom_StartReadAt`
(0x80036E34, `src/main/cdrom.c`) starts a CD read of a file at a sector offset.

`marionation_camera_Init_80036064` (now `cdrom_ReadyCallback`, `code6cac_b4_post.c`) — XA stream
controller for in-engine cinematics. See [sound.md](sound.md).

`marionation_camera_Init_80037468` (now `sys_Exec`, `code6cac_b5_post.c`) — the
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

`cdrom_WaitAudioStarted` (0x80037260, `src/main/cdrom.c`) steps the CD state machine
until CD-audio playback has started (state 0x16); it returns nothing.

## `marionation_camera_Exec` — the active-cam runner

`marionation_camera_Exec(arg0, arg1)` (`code6cac.c:264`) is the per-frame
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

## cdrom_Pause / cdrom_WaitIdle

`cdrom_Pause` (0x80036EC0, once `game_FrameInit`; `src/main/cdrom.c`) stops CD
activity, not a demo mode:

- `CdReadyCallback(0)` — clear the CD ready callback
- `cdrom_SetMix(0, 0, 0, 0)` — CD-audio mix off
- `CdFlush()`, `CdControlF(9, 0)` (CdlPause)
- the CD state's stop request set and state 0xB (the read error / reset path)

`cdrom_WaitIdle` (`src/main/cdrom.c`) is a wait its callers enter around
CD reads:

```c
func_8003AA78();
while (1) {
    if (cdrom_IsIdle() != 0) break;
    func_8003AA48();
    cdrom_Update();
    if (D_800A3906 != 0) func_8005C6D0();
    func_800174F4();
    *p = *p + 2;                          // D_80101E58.rec.unk3E
    VSync(2);
}
func_8003AAB0();
```

It returns as soon as the CD is idle; the game's frame loop is `main`.

## Win-camera variants — `replay_camera_rob_back_*`

Multiple variants exist for the "rob falls back, win pose" cinematic:
- `replay_camera_rob_back_loose2` (now `func_80035618`, `code6cac_b2_post.c`)
- `replay_camera_rob_back_loose3` (`text1a_c.c:289`)
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
