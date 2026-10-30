# Menus, mode handlers and the scene script

The game's top level is the mode handler table `g_main_loop_handlers` (34 entries,
src/main/d_7D870.c:24), called with the mode value `g_main_loop_handler_index`; see
[main_loop.md](main_loop.md) for the table and the loop. This page covers the
pad masks the handlers test, the handlers in src/main/2B344.c that run the
scene script, and the two menu modules (src/main/596CC.c and
src/main/5ED34.c).

## Pad masks

`pad_Update` (src/main/87A0.c:623) builds each player's 16 button bits as
`~((byte2 << 8) | byte3)` of the controller packet (src/main/87A0.c:655) and
packs the two players into one word, player 0 in the low half
(src/main/87A0.c:670). `g_pad_state.pressed` is the newly pressed bits
(`held & ~previous`, src/main/87A0.c:708; `PadState`, include/game.h:839).
With the standard digital-pad bit order, the masks the handlers use are:

| Mask | Button (either player) |
| --- | --- |
| `0x00100010` | Triangle |
| `0x00200020` | Circle |
| `0x00400040` | Cross |
| `0x00800080` | Square |
| `0x08000800` | Start |
| `0x10001000` / `0x40004000` | Up / Down |
| `0x20002000` / `0x80008000` | Right / Left |

## Mode handlers and the scene script (src/main/2B344.c)

`fight_RunCommandStream` (src/main/2B344.c:320) steps a byte script at `g_fight_command_stream_cursor`
until an opcode ends the step. The opcodes and what they do:

- 0: mode 18 (with `g_fight_presentation_index` from `g_fighter_group_flags[fighter 0's character_data_index]`), or
  mode 10 when `g_fight_menu_restart_count` is set.
- 1: one operand byte into `g_fight_stage_index` and the history `g_fight_stage_list_entries[]`;
  mode 22.
- 2: keeps a 0x1D-byte block in `D_800A3894`, sets up fighter 0
  (`fight_FillMissingConfigInitFighterAndLoadObject(0)`), then as 18.
- 3: two operand bytes into `g_fight_config.unk_4[1]` / `[3]`; sets up fighter 1
  (`fight_FillMissingConfigInitFighterAndLoadObject(1)`, `fight_RequestBothFightersEntriesAndReloadOnChange`).
- 16 / 17: `fight_ConfigureFighter1FromLookupAndOperandsAndLoad` / `fight_ConfigureFighter1FromOperandsAndLoad` on the operands, which return
  how many bytes they used.
- 18: mode 0.
- 19: keeps a 4-byte block in `g_fight_active_playback_descriptor`; mode 6.
- 20: keeps a 12-byte block in `D_800A385C` (`fight_SetFixedFighterBytesAndLoadBothFighters`,
  `func_8003B56C(1)`).

Handlers that the script's modes reach:

- `fight_StartDescriptorPlaybackAndAudio` (src/main/2B344.c:477, mode 6) loads the scene the
  `g_fight_active_playback_descriptor` block describes (`motion_StartPlaybackAtIndexedBasePlusOffset` with its byte 0 and the
  fighters its flag byte 3 selects), starts its CD audio track (`cdrom_GetFileIndex(5, g_fight_active_playback_descriptor[1])`) and sets mode 7.
- `fight_HandleDescriptorPlaybackCompletion` (src/main/2B344.c:549, mode 7) counts `g_handler_frame_counter`; while
  `motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush()` is nonzero it waits for Cross. It then pauses the CD and,
  depending on the pending `D_800A3894` / `D_800A385C` blocks, sets mode 0
  or calls `fight_RunCommandStream` for the next script step.
- `fight_InitScrollingSpriteDisplayAndAudio` (src/main/2B344.c:594, mode 10) resets the scene state
  (`motion_InvalidateMarkedNdataPackAndResetLoadCaches`, `slot_ReleaseObjectAndResources`, `eff_ClearInitFlag`, `snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb`,
  `InitScrollingSpriteDisplay(0x80118800)`), starts CD audio
  (`cdrom_GetFileIndex(5, 0x20)`), waits for it, sets mode 0xB and turns the
  display on.
- `fight_UpdateScrollingSpriteDisplayAndMergeBitFlags` (src/main/2B344.c:613, mode 11) waits for Cross (0x40,
  player 0) or `StepScrollingSpriteDisplay()`, merges `g_fight_pending_unlock_mask_bits` into
  `g_saved_record.unlock_mask` and a per-character bit into `unk_04`, and sets mode
  0x1A when either changed, else 8.
- `ExecMovieOverlayAndReinitialize` (src/main/2B344.c:650, mode 15) runs the FMV overlay
  through `sys_ExecMovieOverlay` and sets mode 8.
- `fight_UpdateMatchTallyPresentation` (src/main/2B344.c:788, mode 21) plays sounds at frames
  0x1E / 0x43 and, on Cross or after frame 0xF1, sets mode 0x18 or 0x12.
- `fight_PackTimeRecordsAndInitPresentation` (src/main/2B344.c:824, mode 30) copies the three clock
  records `g_saved_record.times` into the record `GetWorkingRecord()` returns, as
  minutes, seconds and hundredths (its comment, src/main/2B344.c:819).

## The row menu (src/main/596CC.c)

Cited by function name.

- `menu_LoadResourcesInitStateSelectFirstEnabledRow` sets up the menu block `g_row_menu_work_block`, clears the row index,
  steps it to the first enabled row, and sets the cursor field to 4 (its
  caller is `InitRowMenuBlackBgResetFlow`, src/main/64FD8.c:1258).
- `g_menu_packed_cursors` packs three cursors: bits 0-3 the row index 0..7, which skips
  rows whose bit in `g_row_menu_row_state.rows_on` is clear; bits 10-12 a cursor
  0..5; bits 13-15 a cursor 0..2.
- `pad_StepOffsetsGetDirectionEvents` reads the pad word and returns, in its high half, 1 or 2
  for a step one way or the other.
- `menu_UpdateRowMenu` (called from `menu_UpdateRowMenuStoreResult`, src/main/64FD8.c:1273) moves
  the row index with that result, applies the row's `opt[].mode` to
  `g_row_menu_working_record_ptr->view_setting`, and on Cross (with per-player conditions on the
  row's `state`) returns that `state`; Triangle returns -2, otherwise -1.
- `menu_HandleOptionMenuChoices` steps the bits 10-12 cursor; in positions 0..2 the
  low-half result toggles `g_row_menu_working_record_ptr->grayscale`, `unk20_1` and `cd_audio_suppressed`, and Cross advances the
  cursor; positions 3, 4, 5 return 2, 3, 1 on Cross, and Triangle returns 1.
- `menu_HandleMemCardChoices` steps the bits 13-15 cursor and returns 2, 3 or 1 on Cross
  (position 2 also advances the bits 10-12 cursor); Triangle returns 1.

## The selection screen (src/main/5ED34.c)

- `StepSlideBuildFrameRecordAndDispatchStateHandler(arg0, pad)` (src/main/5ED34.c:216) keeps the pad word in
  `g_entry_menu_pressed_buttons`, runs `StepSlideProgressCommitPendingStateAndQueueSound`, calls `gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset`
  (src/main/5ED34.c:503) unless the screen mode
  `g_entry_menu_handler_index` is 2 or 3, and calls `g_entry_menu_handlers[g_entry_menu_handler_index]`; it returns
  `g_entry_menu_exit_result`. Its caller is `StepThenCloseVab1OnNonzero` (src/main/64FD8.c:1307).
- `StepSlideProgressCommitPendingStateAndQueueSound` (src/main/5ED34.c:249) slides `g_entry_menu_slide_displacement` by 0x20 a frame:
  states 1 / 3 (low byte of `g_entry_menu_slide_phase`) slide in to 0x1E8 and then switch
  `g_entry_menu_handler_index` to `g_entry_menu_next_handler_index`; states 2 / 4 slide back to 0, playing sound 5
  as they start.
- `menu_UpdateEntryGridCursors` (src/main/5ED34.c:826) is the grid cursor of that screen
  (see [replay.md](replay.md)).
