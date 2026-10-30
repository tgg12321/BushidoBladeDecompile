# "Replay" and "Special Camera": what the code at those names does

At nearly every address once named `replay_camera_*`, `special_camera_*` or
`marionation_camera_*` (Kengo name-matching; the naming waves retired those
names) the code is the CD reader, a menu, the save flow or geometry helpers, not
a camera. This page describes what each one does today, read from `src/` (path:line cites the
definition). The one view-transform routine among them is `gpu_ResetDrawQueueAndPrepareViewAndLighting`
(below). The game's actual replay (a 120-frame ring of fighter frames, events and
model draws, played back in modes 4 / 5) is the `replay_*` code in
`src/main/28708.c`.

## `g_cd_file_table` and `cdrom_LoadExec` (once `SpecialCam` / `special_camera_get_rot_dir`)

`g_cd_file_table` (`0x8008EC34`, `CdFileEntry[159]`, include/bb2.h:476) holds
the disc's file records: a `CdlLOC` start position and a byte size per file
(not camera data). `cdrom_GetFileIndex(group, i)` (src/main/cdrom.c:42) forms
the indices.

`cdrom_LoadExec(dest)` (src/main/cdrom.c:185) loads a PS-X EXE, the file
`cdrom_GetFileIndex(6, 0)` = 156 = `STR\MOVOVL.EXE`:

1. Seek to the file's position (`CdControl(2, ...)`) and read one sector
   (`cdrom_ReadWait(0x800, buf, 0x80)`); on failure start again.
2. Copy the `EXEC` header at offset 0x10 of that sector to `*dest`.
3. Seek to the next sector and read `dest->t_size` bytes to `dest->t_addr`;
   on failure start again from step 1.

Its only caller is `sys_Exec` (below).

## `cdrom_StartRead` (once `replay_camera_Init`)

`cdrom_StartRead(file_index, dest_addr)` (src/main/cdrom.c:13) queues a read of
file `file_index`
into the CD state block `g_cdrom_state` (`CdState`, include/game.h:2087; its `rec`
member is `Unk80101E60Rec`):

1. If a request is pending (`rec.io_state != 0`), return 0.
2. `rec.file_index = file_index`; `rec.pair = g_cd_file_table[(s16)file_index]`
   (position and size); `rec.read_dest_addr = dest_addr`; `rec.stop_request = 0`.
3. `rec.io_state = 2`; `rec.unk3E = 0`; `rec.read_sector_count = (size + 0x7FF) >> 11`, the
   sector count.
4. Return 1.

`cdrom_Update` (src/main/26940.c:222) then steps the request once per call;
`cdrom_IsIdle` (src/main/cdrom.c:11) is `rec.io_state == 0`. The old table of
"replay state" variables at `0x80101E60..0x80101EA4` is this record's members.

## `menu_UpdateEntryGridCursors` and `gpu_SetSegmentCellsGateBit` (once `replay_camera_attack` / `replay_camera_get_attack_number`)

`menu_UpdateEntryGridCursors(draw_context)` (src/main/5ED34.c:826) is a grid-menu cursor: for each of
its 1 + `g_entry_menu_init_extra_slot_count` + `g_entry_menu_grid_phase_extra_slot_count` cursors it moves a (row, column) position
over a 5-row table
(`g_entry_menu_grid_entries`) on the pad bits in `g_entry_menu_pressed_buttons`, skipping cells already taken
(`g_entry_menu_entry_flags` flag 4), plays `snd_QueueSound` sounds, confirms or cancels a
choice, and draws the cells with `gpu_AddSpriteCells`.

`gpu_SetSegmentCellsGateBit(arg0, arg1, end_x, end_z)` (src/main/2B344.c:1801) walks the cells
of a 32 x 32 grid of 2000-unit squares crossed by the line from (arg0, arg1)
to (end_x, end_z) (both offset by 0x7D00), a DDA in 4.12 fixed point, and calls
`gpu_SetCellGateBit` (src/main/2B344.c:2369) for each cell inside the grid.

## CD and system routines once named `special_camera_*` / `marionation_camera_*`

- `cdrom_Update` (src/main/26940.c:222, once `special_camera_Exec`) steps the
  CD state machine once per call; `cdrom_UpdateAudio` (src/main/26940.c:14,
  once `special_camera_set_win_cam`) is its CD-audio half; `cdrom_StartReadAt`
  (src/main/cdrom.c:31) starts a CD read of a file at a sector offset.
- `cdrom_ReadyCallback` (src/main/267A8.c:32, once
  `marionation_camera_Init_80036064`) is the CdReadyCallback handler. See
  [sound.md](sound.md) for the XA side.
- `cdrom_WaitAudioStarted` (src/main/cdrom.c:150, once
  `marionation_camera_GetMaxFrame`) steps the CD state machine until CD-audio
  playback has started (state 0x16); it returns nothing.
- `sys_Exec(argc, argv, stack_addr)` (src/main/cdrom.c:213, once
  `marionation_camera_Init_80037468`) runs `STR\MOVOVL.EXE` (the FMV overlay)
  and comes back: `VSync(0)`, `SetDispMask(0)`, `gpu_ResetGraphMode1()`,
  `snd_Quit()`, `memcard_Quit()`, `ResetCallback()`, `CdInit()`, then
  `cdrom_LoadExec(&exec)`, `DrawSync(0)`, `ResetGraph(0)`, `StopPAD()`,
  `StopCallback()`; it sets `exec.s_addr = stack_addr`, `exec.s_size = 0` and
  calls BIOS `Exec(&exec, argc, argv)` after `EnterCriticalSection()`; on return
  it runs
  `sys_Init()`, `snd_InitAndLoadCommonVab()`, `VSync(0)`, `SetDispMask(1)`.

## `softbody_PrepareStep` (once `marionation_camera_Exec`)

`softbody_PrepareStep(arg0, softbody)` (src/main/87A0.c:133):

1. Loads `arg0->object_matrix` (a MATRIX) as the GTE rotation and translation and calls
   `softbody_LoadColliders(arg0)` (src/main/87A0.c:43).
2. Stores the translation delta `arg0->object_matrix->t - softbody->matrix.t` in the
   scratchpad words 0x1F800024..0x1F80002C (`SCRV`'s x, y, z,
   src/main/87A0.c:105).
3. Takes the squared length `len2`: above 250000 the scale is 0x100; if the
   sum is negative (overflow) it is 0; otherwise it is
   `(isqrt(len2) << 6) / 500 + 0xC0`, the integer square root read from
   `g_sqrt_table_u8` (normalized with the GTE leading-zero count above 0x400).
4. Sets each `SCRV` component to `(delta * scale) >> 1`, copies `*arg0->object_matrix`
   into `softbody->matrix` and calls `softbody_SolveSprings(softbody)`
   (src/main/87A0.c:179),
   the distance-constraint pass over a chain of 64-byte nodes described above it.

It is not a camera runner. The function once named `cpu_check_run_attack`
is `softbody_SolveSprings` (step 4); the reset wave retired that name.

## `gpu_ResetDrawQueueAndPrepareViewAndLighting`: the view transform (once `replay_camera_rob_back_win_near`)

`gpu_ResetDrawQueueAndPrepareViewAndLighting(focus_point, camera_angles, eye_distance)` (src/main/368E4.c:450) sets up the frame's view and draw state:

1. Resets the draw queue cursor and the OT pointers (`D_800A3808`,
   `g_gpu_ot_offset_ptr`) and increments `D_800A38D6`.
2. If `focus_point` is given: stores `-camera_angles` as the rotation of the view record
   `g_view_camera_root_node`, builds its matrix through `g_math_rot_matrix_funcs[0]`, and sets the
   record's translation to `focus_point + M * (0, 0, -eye_distance)`, a point `eye_distance` units back
   from `focus_point` along the view direction; then `node_SetMatrixFromNegatedRot(&g_view_camera_root_node)`,
   `camera_CopyViewMatrixWithHalvedMiddleRow()`, `gpu_ResetAndStampViewCenteredCellAttributes()`, and keeps `eye_distance` in `g_gpu_view_eye_distance`.
3. Recomputes the light matrices of `g_light_setups[0]`, `[1]`, `[4]`
   (`calc_LightMatrix`) and runs `light_ResetPointLight`, `ClearSavedWordPairsAndTwoWriteOnlyWords`,
   `func_8003F5CC`.

The two other "rob back" names sit on unrelated code: `menu_InitSubflowForMode`
(src/main/25C38.c:96, once `replay_camera_rob_back_loose2`) is a switch on its
argument (0..7) that sets the state `g_row_menu_subflow_state` and, by case, calls
`InitTimedSpriteSequence`, `InitWithCachedValueAndStoredBitsBlackBg` and `menu_InitSelWithStoredBits` (each with the buffer
address 0x80118800), `snd_SerialMixOn`, `cdrom_StartAudioEntry`,
`menu_ResetFlowBackgroundAndConfirmCursor`, `comb_Connect`, `StoreCompactedBits`, `snd_QueueSound` and
`cdrom_EnableSerialMixAndStartRepeatingAudio`;
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
comb_BeginWait();
while (1) {
    if (cdrom_IsIdle() != 0) break;
    comb_ExchangeEmptyPad();
    cdrom_Update();
    if (g_snd_initialized != 0) snd_FlushSoundQueue();
    gpu_DrawDisplayModeOnLocalOtAndSync();
    *p = *p + 2;                          // g_cdrom_state.rec.unk3E
    VSync(2);
}
comb_EndWait();
```

It returns as soon as the CD is idle; the game's frame loop is `main`.

## Status

Every function named on this page is C. `_SsInit` (0x80083A48, once
`md_game_check_change_main_mode_default`) and `SpuClearReverbWorkArea`
(0x8008A928, once `md_game_check_change_main_mode_katinuki`) are Sony library
code, not mode-transition logic.
