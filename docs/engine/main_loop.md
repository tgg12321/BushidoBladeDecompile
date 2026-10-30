# Boot sequence and main loop

## Entry point

The PS-X EXE header's entry is `0x800836EC`, `__SN_ENTRY_POINT`
(asm/funcs/__SN_ENTRY_POINT.s; `INCLUDE_ASM` in
src/main/psxsdk/libsn/73E98.c:15, PsyQ 4.0 LIBSN module SNMAIN). It is SN's
start-up code, in order:

1. zeroes the words from `D_800A3308` up to `D_801078E0` (the BSS);
2. sets `$sp` to `_ramsize - 8` ORed with `0x80000000` (the high half of
   `D_80000004`);
3. computes the heap: base `D_801078E0` (top bits stripped, then ORed with
   0x80000000) into `__heapbase`, size `_ramsize - 8 - _stacksize - base` into
   `__heapsize`;
4. saves `$ra` in `D_800A3668`, sets `$gp` to `_gp` and `$fp = $sp`;
5. calls `InitHeap(__heapbase + 4, __heapsize)`, then `main`; a `break`
   follows the call.

## `main`

`main` (src/main/6CF8.c:423):

1. Start-up: `__main()` (the SN constructor walker; the game has no
   constructors, src/main/psxsdk/libsn/73E98.c:16-22), `SetSp(0x801FFF00)`,
   `SetMem(2)`, `sys_Init()`, `func_80016D78()`, `SetDispMask(1)`, then
   `gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth(0x80118800, ...)` (src/main/6CF8.c:188), which reads file
   `cdrom_GetFileIndex(2, 0x61)`, shows it for 150 frames and darkens it over
   the last 29.
2. `g_main_loop_handler_index = 0xF` (the first mode, `ExecMovieOverlayAndReinitialize`), `g_main_present_skip_frames = 0`,
   `g_gpu_presented_frame_count = 0`.
3. The loop (src/main/6CF8.c:447):
   - picks the buffer `g_gpu_db[g_gpu_presented_frame_count & 1]`, clears its OT
     (`ClearOTagR(ot, 0x1008)`), sets `g_gpu_ot_ptr` and the primitive cursor
     `g_prim_buf_cursor = D_800A3770[idx]`;
   - `eff_SelectQuadBuffer(idx)` (src/main/51268.c), `font_BeginFrame()`
     (src/main/2B344.c:1131), `pad_Update(skip)` (last frame's `g_main_present_skip_frames`),
     `cdrom_Update()`,
     `snd_FlushSoundQueue()` (src/main/4BC3C.c);
   - if the link-abort request `g_comb_abort_request` is set (its one setter is
     `comb_Abort`, src/main/28708.c:1607): `cdrom_PauseAndWait()`, display off
     (`g_local_ot_draw_mode = DISP_DISABLED`), clears the request and `g_menu_mode_subflow_init_pending`, and
     sets mode 8;
   - calls the mode handler `g_main_loop_handlers[g_main_loop_handler_index]()` (src/main/6CF8.c:468),
     then `font_AddTpage()` (src/main/2B344.c:1146);
   - waits until root counter 2 (`GetRCnt(0xF2000001)`) reaches
     `((g_main_frame_period - 1) << 8) + 0x80`, calling `rand()` while it waits; then
     `VSync(1)`, `DrawSync(0)`, `VSync(0)`, `ResetRCnt(0xF2000001)`;
   - unless frames are being skipped (`g_main_present_skip_frames != 0`), `PutDispEnv` /
     `PutDrawEnv` for the buffer;
   - checks the primitive buffer: the space left
     (`D_800A3770[idx] + 0x13400 - g_prim_buf_cursor`) updates the low-water
     mark `g_main_prim_buffer_min_remaining`; below 0 it prints `g_str_prim_overflow` and stops in
     `func_800164F8` (src/main/6CF8.c:488-499);
   - decrements `g_main_present_skip_frames`, or else submits the OT (`DrawOTag`) and
     increments `g_gpu_presented_frame_count`;
   - in mode 1, when no frame was skipped, Start (`0x08000800`) or the
     `g_fight_auto_pause_countdown` countdown (with `g_fight_mode_selector == 2`) opens the pause menu
     `menu_RunPauseMenu` (src/main/6CF8.c:508-522).

## Start-up routines

- `sys_Init` (src/main/6CF8.c:175): `ResetCallback`, `InitPAD` on
  `g_pad_buf`, `StartPAD`, `ChangeClearPAD(0)`, `disp_Init`, display off,
  `g_gpu_jitter_burst_enabled = 0`, `cdrom_Init`, `memcard_Init`, `rcnt_StartCnt1Wrapper`.
- `func_80016D78` (src/main/6CF8.c:285): sets the two primitive buffers
  `D_800A3770[0..1] = 0x801D8800 / 0x801EBC00` and their size `D_800A3798 =
  0x13400`, loads the common sound bank (`snd_InitAndLoadCommonVab`,
  src/main/6CF8.c:265), `pad_ResetStateMarkValid`, `font_LoadImage`,
  `fight_InitSettings`, `func_80046B44`, sets `g_main_frame_period = 2` and clears the
  start-up flags and `g_comb_abort_request`.
- `sys_Panic` (src/main/6CF8.c:239) prints `g_str_overflow` and stops in
  `func_800164F8`; `eff_Init` and `snd_InitAndLoadCommonVab` call it when
  their data is too large.

## The pause menu: `menu_RunPauseMenu`

`menu_RunPauseMenu` (src/main/6CF8.c:312) runs its own frame loop (OT, `pad_Update`,
`gpu_DrawModeSizedPanelWithSelectedCellOpaque` to draw, `cdrom_Update`, `snd_FlushSoundQueue`, present) with 3
choices, or 6 when `g_fight_mode_selector == 2` and `D_800A389A` is 0. Up / Down move
the choice, Left / Right toggle bits of `g_fight_toggle_option_bits` on choices 3..5, Cross
confirms and Triangle cancels. Choice 1 sets `g_menu_mode_subflow_init_pending = 1` and mode 8;
choice 2 sets mode 8.

## The mode table `g_main_loop_handlers`

`g_main_loop_handlers` is a 34-entry table of handlers, defined in the inline
assembly of src/main/d_7D870.c:24. "Sets" lists the modes each handler
assigns to `g_main_loop_handler_index` itself.

| Mode | Handler | Sets | What it does |
| --- | --- | --- | --- |
| 0 | `fight_Init` (src/main/9F9C.c:1874) | 1 | sets up the fight (fighters through `fighter_Init`, the object pool, the view) |
| 1 | `fight_Update` (src/main/9F9C.c:2189) | | per-frame fight: `fighter_BuildCpuOrMappedPadState` / `fighter_Update` per fighter, `fight_UpdateFighterInteractionsAndPointCaches`, `object_UpdatePool` ([combat.md](combat.md), [motion.md](motion.md)) |
| 2 | `ResetGraphClearFrameCounterAndSetMainLoopHandler3` (src/main/17AFC.c:5721) | 3 | resets graphics, clears `g_handler_frame_counter` |
| 3 | `menu_PrintEditAndApplyConfigWithPerPadCursors` (src/main/24F08.c:28) | | prints the `g_fight_config` settings with `font_Printf` and steps the two cursors `g_config_menu_cursors`; calls `ApplyPendingConfigAndInitializeMode` |
| 4 | `replay_Start` (src/main/28708.c:1302) | 5 | view and fighter setup |
| 5 | `replay_PlayFrame` (src/main/28708.c:1360) | 0xC, 0x10, 0x1E | rebuilds the fighters from the per-frame records `g_replay_fighter_frames` ([motion.md](motion.md)) |
| 6 | `fight_StartDescriptorPlaybackAndAudio` (src/main/2B344.c:478) | 7 | loads a script scene ([menus.md](menus.md)) |
| 7 | `fight_HandleDescriptorPlaybackCompletion` (src/main/2B344.c:550) | 0 | waits, then the next script step |
| 8 | `menu_ReleaseResourcesConditionallyCloseLinkAndVabsAndInitRowMenu` (src/main/25C38.c:34) | 9 | resets the scene, requests the link close (`comb_RequestClose`, unless `g_menu_mode_subflow_init_pending` is set), sets up the row menu (`InitRowMenuBlackBgResetFlow`) |
| 9 | `menu_UpdateRowMenuAndSubflows` (src/main/25C38.c:195) | 0, 0xF, 8 | the row-menu flow (`menu_UpdateRowMenuStoreResult`) with CD audio and memory-card calls |
| 10 | `fight_InitScrollingSpriteDisplayAndAudio` (src/main/2B344.c:595) | 0xB | resets the scene, starts CD audio track `(5, 0x20)` |
| 11 | `fight_UpdateScrollingSpriteDisplayAndMergeBitFlags` (src/main/2B344.c:614) | 0x1A, 8 | waits for Cross, merges unlock bits into `g_saved_record` |
| 12 | `fight_InitFighterFocusViewAndThreeItemMenu` (src/main/9F9C.c:2226) | 0xD | |
| 13 | `fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange` (src/main/9F9C.c:2241) | 0, 8 | per-frame fight variant with the view `camera_SetFighterFocusView` and the cursor `g_fight_choice_menu_cursor` |
| 14 | `func_80035430` (src/main/25788.c:202) | | empty |
| 15 | `ExecMovieOverlayAndReinitialize` (src/main/2B344.c:651) | 8 | runs the FMV overlay (`sys_ExecMovieOverlay`) |
| 16 | `func_8001EEB4` (src/main/9F9C.c:2340) | 0x11 | when gated, starts an indexed common script on fighter `g_fight_followup_fighter_index` (`fighter_StartMoveScript`) and sets its `bone_group_6_9_intact` |
| 17 | `fight_UpdateInterpolatedFighterViewAndSelectNextPhaseByModeOnTimeoutOrPadPress` (src/main/9F9C.c:2365) | 0xC, 2 | per-frame fight variant with the view `camera_SetInterpolatedFighterView`; after frame 0x69 or on Cross it sets 0xC (`g_fight_mode_selector` 4 / 6, or 1 with `g_fight_followup_fighter_index != 0`), calls `fight_AdvanceOpponentOrder` (`g_fight_mode_selector` 1, `g_fight_followup_fighter_index == 0`), or sets 2 |
| 18 | `fight_InitImageAndPlaybackPresentation` (src/main/2B344.c:666) | 0x13 | scene setup chosen by `g_fight_presentation_index` |
| 19 | `fight_UpdateSplitWipeImageAndPlaybackPresentation` (src/main/2B344.c:725) | 0x12, 0x18, 0x1A, 0xA, 8 | waits on `motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush`, then picks the next mode |
| 20 | `fight_TallyMatchResultsAndInitPresentation` (src/main/2B344.c:754) | 0x15 | |
| 21 | `fight_UpdateMatchTallyPresentation` (src/main/2B344.c:786) | 0x12, 0x18 | timed sounds, ends on Cross or frame 0xF1 |
| 22 | `fight_InitCommandStreamImageLoad` (src/main/2B344.c:421) | 0x17 | |
| 23 | `fight_UpdateCommandStreamImageLoadAndResume` (src/main/2B344.c:432) | | on its third frame runs the next script step (`fight_RunCommandStream`) |
| 24 | `fight_InitRotatingViewThreeOptionChoice` (src/main/2B344.c:877) | 0x19 | |
| 25 | `fight_UpdateRotatingViewThreeOptionChoice` (src/main/2B344.c:886) | 0, 8 | view (`gpu_ResetDrawQueueAndPrepareViewAndLighting`) and its own per-frame work |
| 26 | `menu_ReleaseResourcesCloseVabsAndInitRowMenu` (src/main/25C38.c:394) | 0x1B | resets the scene, sets up the row menu (`InitRowMenuBlackBgResetFlow`) |
| 27 | `DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit` (src/main/25C38.c:407) | 8 | screen gradient and the memory-card flow `memcard_StepSaveOverwriteAndFormatPrompts` |
| 28 | `fight_InitNearestStartPairPlayback` (src/main/2B344.c:990) | 0x1D | view and scene setup |
| 29 | `fight_UpdateNearestStartPairPlaybackAndReinitMotionStreams` (src/main/2B344.c:1039) | 0x18 | frame-threshold cues on `g_handler_frame_counter` |
| 30 | `fight_PackTimeRecordsAndInitPresentation` (src/main/2B344.c:822) | 0x1F | stores the three clock records |
| 31 | `fight_UpdateTimeRecordPresentation` (src/main/2B344.c:868) | | on Cross or frame 0xF1 calls `fight_SetSelectorAndMainLoopHandlerFromFlagsAndBestTimeRank` (src/main/17AFC.c:5999), which sets 0x12, 0x1A or 8 |
| 32 | `fight_InitRotatingViewDecimalPresentation` (src/main/2B344.c:959) | 0x21 | |
| 33 | `fight_UpdateRotatingViewDecimalPresentation` (src/main/2B344.c:967) | 8 | |

Modes the main loop itself sets: 0xF at start, 8 on a link abort. Many
handlers draw through `gpu_DrawDisplayModeOnLocalOtAndSync` (src/main/6CF8.c:526), which returns at
once while `g_local_ot_draw_mode` is `DISP_DISABLED` and otherwise switches on it.
