# "Replay" and "Special Camera": what the code at those names does

At nearly every address once named `replay_camera_*`, `special_camera_*` or
`marionation_camera_*` (Kengo name-matching; the naming waves retired those
names) the code is the CD reader, a menu, the save flow or geometry helpers, not
a camera. This page describes what each one does today, read from `src/` (path:line cites the
definition). The one view-transform routine among them is `func_80046BF4`
(below).

## `g_cd_file_table` and `cdrom_LoadExec` (once `SpecialCam` / `special_camera_get_rot_dir`)

`g_cd_file_table` (`0x8008EC34`, `CdFileEntry[159]`, include/bb2.h:476) holds
the disc's file records: a `CdlLOC` start position and a byte size per file
(not camera data). `cdrom_GetFileIndex(group, i)` (src/main/cdrom.c:41) forms
the indices.

`cdrom_LoadExec(dest)` (src/main/cdrom.c:181) loads a PS-X EXE, the file
`cdrom_GetFileIndex(6, 0)` = 156 = `STR\MOVOVL.EXE`:

1. Seek to the file's position (`CdControl(2, ...)`) and read one sector
   (`cdrom_ReadWait(0x800, buf, 0x80)`); on failure start again.
2. Copy the `EXEC` header at offset 0x10 of that sector to `*dest`.
3. Seek to the next sector and read `dest->t_size` bytes to `dest->t_addr`;
   on failure start again from step 1.

Its only caller is `sys_Exec` (below).

## `cdrom_StartRead` (once `replay_camera_Init`)

`cdrom_StartRead(a0, a1)` (src/main/cdrom.c:13) queues a read of file `a0`
into the CD state block `D_80101E58` (`CdState`, include/game.h:2079; its `rec`
member is `Unk80101E60Rec`):

1. If a request is pending (`rec.unk02 != 0`), return 0.
2. `rec.unk00 = a0`; `rec.pair = g_cd_file_table[(s16)a0]` (position and
   size); `rec.unk1C = a1`; `rec.unk08 = 0`.
3. `rec.unk02 = 2`; `rec.unk3E = 0`; `rec.unk18 = (size + 0x7FF) >> 11`, the
   sector count.
4. Return 1.

`cdrom_Update` (src/main/26940.c:222) then steps the request once per call;
`cdrom_IsIdle` (src/main/cdrom.c:11) is `rec.unk02 == 0`. The old table of
"replay state" variables at `0x80101E60..0x80101EA4` is this record's members.

## `func_80070188` and `func_8003E2D8` (once `replay_camera_attack` / `replay_camera_get_attack_number`)

`func_80070188(arg0)` (src/main/5ED34.c:826) is a grid-menu cursor: for each of
its 1 + `D_800A35B0` + `D_800A3554` cursors it moves a (row, column) position
over a 5-row table
(`D_8009BC40`) on the pad bits in `D_800A354C`, skipping cells already taken
(`D_8009BC7C` flag 4), plays `func_8005C650` sounds, confirms or cancels a
choice, and draws the cells with `func_8007352C`.

`func_8003E2D8(arg0, arg1, arg2, arg3)` (src/main/2B344.c:1739) walks the cells
of a 32 x 32 grid of 2000-unit squares crossed by the line from (arg0, arg1)
to (arg2, arg3) (both offset by 0x7D00), a DDA in 4.12 fixed point, and calls
`func_8003F388` (src/main/2B344.c:2287) for each cell inside the grid.

## CD and system routines once named `special_camera_*` / `marionation_camera_*`

- `cdrom_Update` (src/main/26940.c:222, once `special_camera_Exec`) steps the
  CD state machine once per call; `cdrom_UpdateAudio` (src/main/26940.c:14,
  once `special_camera_set_win_cam`) is its CD-audio half; `cdrom_StartReadAt`
  (src/main/cdrom.c:31) starts a CD read of a file at a sector offset.
- `cdrom_ReadyCallback` (src/main/267A8.c:32, once
  `marionation_camera_Init_80036064`) is the CdReadyCallback handler. See
  [sound.md](sound.md) for the XA side.
- `cdrom_WaitAudioStarted` (src/main/cdrom.c:146, once
  `marionation_camera_GetMaxFrame`) steps the CD state machine until CD-audio
  playback has started (state 0x16); it returns nothing.
- `sys_Exec(a0, a1, a2)` (src/main/cdrom.c:209, once
  `marionation_camera_Init_80037468`) runs `STR\MOVOVL.EXE` (the FMV overlay)
  and comes back: `VSync(0)`, `SetDispMask(0)`, `gpu_ResetGraphMode1()`,
  `snd_Quit()`, `memcard_Quit()`, `ResetCallback()`, `CdInit()`, then
  `cdrom_LoadExec(&exec)`, `DrawSync(0)`, `ResetGraph(0)`, `StopPAD()`,
  `StopCallback()`; it sets `exec.s_addr = a2`, `exec.s_size = 0` and calls
  BIOS `Exec(&exec, a0, a1)` after `EnterCriticalSection()`; on return it runs
  `sys_Init()`, `snd_InitAndLoadCommonVab()`, `VSync(0)`, `SetDispMask(1)`.

## `func_80018094` (once `marionation_camera_Exec`)

`func_80018094(arg0, arg1)` (src/main/87A0.c:107):

1. Loads `arg0->unk4` (a MATRIX) as the GTE rotation and translation and calls
   `func_80017FA0(arg0)` (src/main/87A0.c:42).
2. Stores the translation delta `arg0->unk4->t - arg1->matrix.t` in the
   scratchpad words 0x1F800024..0x1F80002C (`SCRV`'s x, y, z,
   src/main/87A0.c:105).
3. Takes the squared length `len2`: above 250000 the scale is 0x100; if the
   sum is negative (overflow) it is 0; otherwise it is
   `(isqrt(len2) << 6) / 500 + 0xC0`, the integer square root read from
   `g_sqrt_table_u8` (normalized with the GTE leading-zero count above 0x400).
4. Sets each `SCRV` component to `(delta * scale) >> 1`, copies `*arg0->unk4`
   into `arg1->matrix` and calls `func_80018300(arg1)` (src/main/87A0.c:174),
   the distance-constraint pass over a chain of 64-byte nodes described above it.

It is not a camera runner. The function once named `cpu_check_run_attack`
is `func_80018300` (step 4); the reset wave retired that name.

## `func_80046BF4`: the view transform (once `replay_camera_rob_back_win_near`)

`func_80046BF4(a0, a1, a2)` (src/main/368E4.c:448) sets up the frame's view and draw state:

1. Resets the draw queue cursor and the OT pointers (`D_800A3808`,
   `D_800A378C`) and increments `D_800A38D6`.
2. If `a0` is given: stores `-a1` as the rotation of the view record
   `D_80101DF0`, builds its matrix through `D_800F66A0[0]`, and sets the
   record's translation to `a0 + M * (0, 0, -a2)`, a point `a2` units back
   from `a0` along the view direction; then `func_800418D0(&D_80101DF0)`,
   `func_80047210()`, `func_8003F274()`, and keeps `a2` in `D_800A33C0`.
3. Recomputes the light matrices of `D_800F62E0[0]`, `[1]`, `[4]`
   (`calc_LightMatrix`) and runs `func_800420D0`, `func_8003F568`,
   `func_8003F5CC`.

The two other "rob back" names sit on unrelated code: `func_80035618`
(src/main/25C38.c:96, once `replay_camera_rob_back_loose2`) is a switch on its
argument (0..7) that sets the state `D_800A3740` and, by case, calls
`func_800784E4`, `func_80077984` and `func_80077A80` (each with the buffer
address 0x80118800), `snd_SerialMixOn`, `cdrom_StartAudioEntry`,
`func_80077A28`, `func_8003ACB8`, `func_80077940`, `func_8005C650` and
`func_800355E8`;
`math_RotMatrixYXZ` (src/main/32D04.c:268, once
`replay_camera_rob_back_loose3`) builds a rotation matrix from three angles.

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

## Status

Every function named on this page is C. `_SsInit` (0x80083A48, once
`md_game_check_change_main_mode_default`) and `SpuClearReverbWorkArea`
(0x8008A928, once `md_game_check_change_main_mode_katinuki`) are Sony library
code, not mode-transition logic.
