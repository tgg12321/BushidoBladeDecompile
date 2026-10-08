# Cross-Reference: Subsystems, Files, Functions, Globals

This is the lookup index. If you have a function or global and want to
know which subsystem owns it, look here.

## Subsystem → primary file → key functions → key globals

### Boot + main loop
- **File:** `ings.c` (high-level), `ings2.c` (BIOS shims), `main.c`
  (mid-game state)
- **Key functions:** `cpu_set_move_command_and_dir_for_no_action_2`
  (main entry, ings.c:584), `sys_Init`, `func_80016D78`, `motion_Open`
  (CTOR runner), `gnd_disp_loop_ctrl`
- **Key globals:** `D_800A3834` (mode), `D_800A36AC` (frame count),
  `D_800A3768` (the `func_800174F4` draw mode), `D_8008D070` (CTOR table), `D_8008D090`
  (mode dispatch table)
- **Doc:** [main_loop.md](main_loop.md)

### Combat / Hit Detection
- **File:** `code6cac_b.c` (main), `text1b.c` (helpers), `main.c`
  (coli_HitPause), `code6cac.c` (cpu_check_run_attack)
- **Key functions:** `coli_hit_body_weapon` (move setup),
  `cpu_check_tubazeri` (sword clash, GTE cross product),
  `coli_check_circle_hit_line`, `cpu_set_move_command_and_dir`,
  `cpu_check_run_attack`, `cpu_get_dist_2` (bytecode interpreter),
  `damage_DebugDisp`, `gnd_land_hit_char_tsuba`,
  `gnd_land_hit_char_die_main`, `katinuki_game_*`
- **Key globals:** `D_80106A78` (active-move slots), `D_80106A50` (move
  enable bits), `Judge` (sin/cos LUT), `D_8008E194` (waza-table base),
  `D_800A3380` (two presence flags), `D_800A3384` (two position pointers
  `func_80041EB0` sets light[1] from)
- **Doc:** [combat.md](combat.md)

### CPU / AI
- **File:** `code6cac_b.c` (heavy), `code6cac.c` (move-selection)
- **Key functions:** `cpu_set_move_command_and_dir`,
  `cpu_set_move_command_and_dir_for_no_action`, `cpu_get_dist`,
  `cpu_get_dist_2`, `cpu_check_same_dir_timer`, `cpu_side_move_dir`,
  `cpu_get_move_pattern_table_number`,
  `cpu_check_move_dir_pattern_enemy_attack`, `func_8003047C`
  (waza-queue loader)
- **Key globals:** `D_8008E338` (per-char per-stance waza table),
  `D_8008D538` (per-stance enable mask), `D_801077B0` (shuffled
  move-pool buffer), `D_8010277C..D_80102787` (tactical AI knobs),
  `D_800A36F2` (current waza ID)
- **Doc:** [ai.md](ai.md)

### Motion / Animation
- **File:** `code6cac_c_mid.c` (motion_SetMotion etc.), `text1b.c`
  (calc_loc_mat, motion_ShiftControl), `ings2.c` (motion_Open),
  `display.c` (some motion_PreCalc)
- **Key functions:** `motion_Open` (CTOR runner — misnamed),
  `motion_SetMotion` (per-frame motion select FSM),
  `motion_LoadPreCalcData_*` (load BBM data),
  `motion_SavePreCalcData_*`, `motion_ShiftControl` (shift dispatch),
  `motion_shift_check_*`, `calc_loc_mat_fw*` (matrix builders),
  `myRobGenei*` (afterimage)
- **Key globals:** `D_800A3207` (motion FSM state), `D_800A334C` (90-frame countdown of a memory-card message)
- **Doc:** [motion.md](motion.md)

### GPU / Render
- **File:** `gpu.c` (helpers), `display.c` (DMA, env, OT helpers),
  `text1b.c` (per-frame geometry, calc_loc_mat — overlaps with motion)
- **Key functions:** `gpu_SetDispMask`, `gpu_DrawSync`, `gpu_DrawOTag`,
  `gpu_LoadImage`, `gpu_ClearOTag`, `func_8007B844` (reverse-OT clear),
  `gpu_InitDrawEnv`, `gpu_InitDispEnv`, `disp_Init`,
  `disp_SetFramebufferMode`, `gpu_SendData`, `gpu_StartDmaList`
- **Key globals:** `g_disp_fb_base` (double-buffer base),
  `g_gpu_dev_table` (libgpu device dispatch), `g_gpu_draw_env`,
  `g_gpu_disp_env`, `g_gpu_ot_end`, `g_cam_matrix`, `g_sin_table`,
  `g_sin_lut_q*`, `g_cos_lut_q*`, `g_gte_sqrt_table`
- **Doc:** [gpu_pipeline.md](gpu_pipeline.md)

### Sound / SPU
- **File:** `text1b.c` (snd_* API + game state), `main.c` (spu_*
  low-level + voice allocator), `text1a_c.c` (`func_800450BC` family)
- **Key functions:** `snd_LoadBgm`, `snd_PlayBgm`, `snd_LoadSe`,
  `snd_PlaySe`, `snd_LoadSelection`, `snd_StopAll`, `snd_SetVolume`,
  `spu_NotifyChannel`, `coli_HitPauseKatana` + `coli_HitPauseKatana_2`
  + `exec_game` (voice-key allocator — misnamed),
  `marionation_camera_Init_80036064` (XA stream), `func_800450BC`,
  `func_80045188`, `single_game_VoiceContorol`
- **Key globals:** `g_snd_bgm_id`, `g_snd_se_id`, `g_snd_volume`,
  `g_spu_voice_key_a/b/c` (voice-key array), `g_snd_ch_data`
  (per-channel data), `g_snd_fade_curve`, `SND_CHANNEL_BGM=8`,
  `SND_CHANNEL_SE=9`, `SND_CHANNEL_UI=0xA`
- **Doc:** [sound.md](sound.md)

### File I/O / CD-ROM / Memcard
- **File:** `ings.c` (file_*), `system.c` (cdrom_*), `ings2.c` (bios_*)
- **Key functions:** `file_LoadAll`, `file_LoadSectors`,
  `file_LoadOverlay`, `file_LoadSoundData`, `cdrom_CheckReady`,
  `cdrom_SetCallbackA/B`, `cdrom_FramesToBcd`, `cdrom_BcdToFrames`,
  `bios_FileRead`, `bios_FileReadRaw`, `tslTm2LoadImage`,
  `LWCard_SetAccessData`, `_McAccessSection`, `_CardCheckPulled`
- **Key globals:** `g_file_flags`, `g_file_disc_type`,
  `g_file_disc_size`, `g_file_dma_flag`, `g_file_heap_base` (RNG
  state!), `g_cd_*` (libcd shadows),
  `g_memcard_busy`, `g_memcard_data`
- **Doc:** [file_io.md](file_io.md)

### Menus / UI / Fades
- **File:** `code6cac_c2.c`, `code6cac_c_mid.c`,
  `code6cac_c2.c` tail (options), `text1a_*.c` (mental gauge / `efc_*`)
- **Key functions:** `md_game_check_change_sub_mode`,
  `md_menu_logo_exec` (asm-only), `game_SetControllerPorts`,
  `game_SetPlayerCount`, `func_8003F168`, `func_8003F274`,
  `disp_mario_jimaku` (subtitles, asm), `DispPracticeMenuTex_*`,
  `DispUpdateStatusMessage`,
  `func_80016E60` (picks one of 3/6 menu entries from the pad),
  `FadeOut_*`, `CheckFadeEnd`, `InitFadePanel`
- **Key globals:** `D_800A36A8` (0/1 sprite-pass switch), `g_game_mode`, `g_game_pause`,
  `g_color_mode` (grayscale flag), `D_80102794` (pad input mask),
  `g_char_setup_tbl`
- **Doc:** [menus.md](menus.md)

### Replay / Special Camera
- **File:** `code6cac_b2_post.c`..`code6cac_b5_post.c` (replay_camera_Init etc.),
  `text1b_tu1d.c` (replay_camera_attack etc.), `code6cac_c2.c`
  (replay_camera_get_attack_number), `text1a_c.c`
  (replay_camera_rob_back_loose3)
- **Key functions:** `replay_camera_Init`, `replay_camera_attack`,
  `replay_camera_get_attack_number`, `replay_camera_rob_back_*`,
  `special_camera_Exec` (mostly empty stub),
  `special_camera_get_rot_dir` (disc loader),
  `marionation_camera_Init_80036064` (XA),
  `marionation_camera_Init_80037468` (cinematic-mode entry),
  `marionation_camera_GetMaxFrame`, `game_FrameInit`, `func_80036F40`
- **Key globals:** `SpecialCam` (`0x8008EC34` — disc table of
  cinematic entries), `D_8008EC38` (length table),
  `D_80101E5C..D_80101EA4` (replay state machine)
- **Doc:** [replay.md](replay.md)

### Stage / World
- **File:** `code6cac_c2.c` (stage_*, stage open),
  `text1a*.c` (gnd_* helpers)
- **Key functions:** `func_80046798`, `func_800467A8`,
  `func_80046F14`, `func_8003F274`, `func_8003F5CC`,
  `func_8003F568`, `func_8003F168`, `gnd_init_*`,
  `gnd_get_fog`, `gnd_set_fog*`, `gnd_open`, `gnd_close_*`,
  `gnd_disp_loop_ctrl` (the gameplay draw)
- **Key globals:** `D_80099478`, `D_8009947A`, `D_8009947C`,
  `D_800A8FB0` (32x32 grid read by the draw walks), `D_800A93B0` /
  `D_800A93BC` (passed only to the empty `func_80017F98`), `D_800948BC`
  (`{init, unk4}` hook pairs)

### Character / Player
- **File:** `code6cac.c` (player_*), `text1b.c` (`obj_*`)
- **Key functions:** `func_80041604`, `func_800415C4`,
  `game_GetPlayerCount`, `obj_InitChars`, `obj_InitTask`,
  `obj_InitPair`, `obj_InitAll`, `obj_Reset`, `obj_ExecTask`,
  `obj_InitTaskCamera`, `obj_UpdatePosition`
- **Key globals:** `D_800A9A10` (model object slots), `D_80094B88` (per-slot codes),
  `g_char_setup_tbl`, `D_8009BA7C`

### System / IRQ / Timer
- **File:** `ings2.c`, `system.c` (CD overlap)
- **Key functions:** `sys_VSync`, `sys_SetVsyncMode`, `sys_SetTimer`,
  `irq_DisableInterrupts`, `irq_AcknowledgeVblank`,
  `irq_EnableInterrupts`, `irq_SetAlarm`, `irq_Reset`,
  `EnterCriticalSection`, `ExitCriticalSection`
- **Key globals:** `pCallbacks`, `D_800A1578` (intrEnv; `.inInterrupt` = CheckCallback),
  `g_sys_vsync_mode`, `g_sys_timer`, `video_mode`,
  `g_sys_dma_region`

## Reverse index: global → subsystem

If you've seen a global in the source and want to know what it does:

### `0x800A_0xxx` range (low BSS)
- `D_800A9A10` — model object slots (`func_80045878`)
- `D_80094B88` — per-slot 5-bit codes (`func_80041604`)
- `D_800948BC` — hook pairs (`func_8003F168` / `func_8003E6D8`)
- `g_snd_se_bank` (0x80099C34) — Sound
- `D_80099478`, `D_8009947A`, `D_8009947C` — `func_800460E4` state / `func_80046F14` block
- `g_cd_*` (0x800A11B4..14C0) — CD-ROM (system.c)
- `g_sys_*` (0x800A14CC..2664) — System/IRQ
- `g_spu_*`, `g_snd_*` (0x800A2870..3404) — Sound
- `g_game_*` (0x800A322C..3374) — Game state
- `D_800A3768`, `D_800A36A8` — the `func_800174F4` draw mode and a 0/1
  sprite-pass switch
- `g_game_timer` (0x800A3790) — Game state
- `D_800A3834` — game-mode dispatch register
- `D_800A6690` — transform-node records queued by the grid draw walks
- `D_800A8FB0` — 32x32 grid read by the draw walks
- `SpecialCam` (0x8008EC34) — Replay/Camera disc table

### `0x800E_xxxx` range (mid BSS)
- `D_800EEDB0`, `D_800EEDD0` (view matrix copies), `g_cam_bone_data2` (0x800EEDB0..) — GPU camera
- `g_cam_fov_*` (0x800F62F8..) — GPU camera
- `g_snd_config_tbl` (0x800EF7BC) — Sound
- `g_snd_fade_curve` (0x800EF800) — Sound
- `g_snd_ch_data` (0x800EF848) — Sound (per-channel data)
- `D_800F0D78[16]` (0x800F0D78) — effect slot positions (func_800645B0 / func_800646E8)
- `MarioCam_str` (0x800F19D0) — Replay/Special-cam debug
- `g_gpu_color_table` (0x800F189C) — GPU
- `g_color_mode` (0x800F6652) — Menus (grayscale)
- `g_game_p1_ctrl`, `g_game_p2_ctrl`, `D_800F665C` (write-only),
  `g_game_pause` (0x800F6654..665C) — Game state
- `D_800F66A0` — table of rotation-matrix routines (`math_RotMatrixZYX` etc.)
- `g_disp_fb_base`, `g_disp_fb_flag` (0x800F7438, 0x800F7450) — GPU
- `D_800F6740` (0x800F6740) — 8 object records of 0x34 bytes (`func_80017D84` allocates; 6CF8.c)
- `g_memcard_busy` (0x800FF578) — Memcard
- `g_pad_data` (0x800FF580) — Input

### `0x801xx_xxxx` range (high BSS, data + tables)
- `g_memcard_slot` (0x80101BCC) — Memcard
- `g_memcard_data` (0x80103600) — Memcard
- `D_80104F38` (0x80104F38) — Sound (VAB sound-pack buffer)
- `D_80106A50..` — File I/O / character flags
- `D_80106A78` — Combat (active-move slot array, 12 * 0x64 bytes)
- `D_80107850..58` — AI tactical position arrays
- `D_80102794` — Pad input mask (combined P1+P2)

### Function name prefix conventions

| Prefix | Subsystem |
| --- | --- |
| `coli_` | Combat collision (some misnamed for SPU) |
| `cpu_` | AI move decision |
| `action_` | Move execution (mostly asm) |
| `damage_` | Damage resolution |
| `katinuki_` | Katinuki finisher mechanic |
| `motion_` | Animation playback |
| `myRobGenei*` | Zanzou (afterimage) rendering |
| `calc_loc_mat_*` | Bone-matrix builder |
| `gnd_` | Ground / stage |
| `stage_` | Stage selection / config |
| `player_` | Player object lifecycle |
| `obj_` | Object/task system |
| `game_` | High-level game state |
| `md_game_` | Mode-handler functions |
| `md_menu_` | Menu-mode handlers |
| `md_option_` | Options-menu handlers |
| `cdrom_` | CD-ROM library wrapper |
| `bios_` | BIOS syscall wrapper |
| `file_` | High-level file I/O |
| `gpu_` | GPU helper |
| `disp_` | Display setup |
| `sys_` | System / boot |
| `irq_` | Interrupt handling |
| `spu_` | SPU low-level |
| `snd_` | High-level sound API |
| `seq_` | Sequence scheduler |
| `saTan*` | Mental gauge / training |
| `saSe*` | Sound effects (training) |
| `saEft*` | Effects (asm-only) |
| `saFid*` | File-ID loaders |
| `efc_` | Effect rendering (mostly asm) |
| `replay_camera_` | Replay camera |
| `special_camera_` | Special / cinematic camera |
| `marionation_camera_` | Cinematic camera entry/exit |
| `tslT*`, `tslDr*`, `tslSm*`, `tslPoly*`, `tslDma*`, `tslFile*` | "TSL" library wrappers (Lightweight's in-house helpers — same naming as Kengo) |
| `LWCard_` | LightWeight memory card |
| `Vu0SetLightColMatrix_*` | libgpu light-matrix (misnamed; not VU0) |
| `pad_` | Controller input |
| `_DispXxx` | Text/menu renderers |
| `_McXxx` | Memory card |
| `_GetBattleSwichData`, `_CardCheckPulled`, `_SelectSection` | Misc helpers |

## Cluster additions from placeholder-refinement traces (2026-05-17)

The following clusters were promoted from placeholders to semantic
names during the placeholder-refinement sprint (see
the subsystem docs' naming-pass cross-references for
full traces).  Indexed here by address range for reverse lookup:

### `0x8009_xxxx` range — sprite-sheet headers and cells
- `D_8009B2C8`, `D_8009B610`, `D_8009B63C`, `D_8009B6F0`, `D_8009B770`,
  `D_8009B7A0`, `D_8009B7AC` (12-byte `Unk8009B0E0Record` sprite-sheet headers) and
  `D_8009B340`, `D_8009B358`, `D_8009B388`, `D_8009B634`, `D_8009B660`, `D_8009B708`,
  `D_8009B758`, `D_8009B7D0`, `D_8009B7D8`, `D_8009B800`, `D_8009B820`, `D_8009B840`
  (8-byte `Unk8009B400Record` sprite cells), 0x8009B2C8..0x8009B850 — each set read by
  one draw helper through `func_8007352C` / `func_80073728` (§21)
- `D_8009B850` (0x8009B850) — u16 packed screen positions: x = (v >> 7) + 0x37,
  y = (v & 0x7F) + 0x2A, read by `func_80060414` (3AB48.c:5523-5524) (§19)
- `g_trig_sin_cos_table_packed` (0x8009C928, 16384 bytes) — walk-direction
  cos/sin LUT consumed by `motutil_GetWalkDir` (§16)

### `0x800A_36xx` range — alarm / IRQ-callback cluster
- `g_alarm_armed_flag` (0x800A26D0), `g_alarm_secondary_cb_ptr`
  (0x800A26D4, was `_plus_4`), `g_alarm_callback_ptr` (0x800A26D8),
  `g_alarm_callback_pending` (0x800A26DC), `g_alarm_active_sentinel`
  (0x800A26DE), `g_alarm_pending_priority_flag` (0x800A26E0, NEW) — §22
- `D_800A36E0` (0x800A36E0) — cursor over the current parity bank of the
  DR_MODE buffer `D_800F1438` (51268.c:5210-5212, :5235)
- `D_800A36EC` — base pointer set once to `D_800F33D8`, indexed
  [frame][fighter]
- `g_main_flags_bitmask_reg` (0x800A289C) — main flags reg
- `D_800A35D0` (0x800A35D0) — s16[2][2], one pair of steps per player that
  `func_800692C0` adds into the SelWork f40 counters (51268.c:3829)

### `0x800E_F0xx` range — SPU voice0E setup cluster (§13/14)
- `g_snd_voice_init_block` (0x800EF070, ~0x68 bytes) — SPU voice ID 0xE struct
- `g_snd_voice_init_vol_baseline` (0x800EF0BC, = -0x2EE0)
- `g_snd_voice_init_pitch_baseline` (0x800EF0C4, = -0xFA0)
- `g_snd_voice_envelope_block_a/b` (0x800EF0D8, 0x800EF168) — scratchpad-DMA src
- `g_snd_wave_phase_table` (0x800EF558, 17 × s32)
- `g_snd_wave_output_table` (0x800EF59C, 9 × 17 s32)

### `0x800E_FBxx` range — sound-data pointer cluster (§12)
- `g_snd_data_buf_base` (0x800EFB14) — sound buffer base + header
- `g_snd_data_subblock_{0..4}_ptr` (0x800EFB18..0x800EFB28) — 5 cached
  subblock pointers; relocated by `func_80054FDC(delta)` when buffer moves
- `D_800EFB0C` — `D_800EFAE8.unk24` (s16[4]): the negated rotation angles
  `func_8005490C` writes; `func_8005507C` returns its address

### `0x800F_0xxx` range — flare slot pool (§20)
- `D_800F0E38` (`Unk800F0E38Record[12]`, x / y / z at +0 / +4 / +8)
- `D_800F0BEC` (12 × s16 per-slot age)
- `D_800A3448` — live mask of the 12 flare slots
- (Pool A's mask is `D_800A3444` / data at D_800F0D78..)

### `0x800F_33xx` range — saTan0Main MIDI dispatch (§11)
- `g_seq_event_handler_90_NoteOn` (0x800F3340)
- `g_seq_event_handler_C0_PgmChange` (0x800F3344)
- `g_seq_event_handler_E0_PitchBend` (0x800F3348)
- `g_seq_event_handler_FF_Meta` (0x800F334C)
- `g_seq_event_handler_B0_CtrlChange` (0x800F3350)
- **NEVER WRITTEN** in shipped EXE (verified via byte-level binary
  scan); dispatch arms in saTan0Main are effectively dead code
- `D_800F33D8` (512 bytes) — one scratch region: `func_800174F4` primitive
  output, the memcard save/load image (§17), per-frame Rec1C records and
  `sys_Exec`'s argument
- `g_main_dispatch_fn0..4` aliases retained pointing at the same addresses

### `0x800F_FF5x` range — camera view-state MATRIX (§18)
- `g_camera_view_state` (0x800FF558, 32 bytes) — PsyQ `MATRIX` struct:
  9 × s16 rotation matrix at +0x00..+0x11, u16 pad at +0x12, s32 t[3]
  (TRX/TRY/TRZ = camera XYZ pos) at +0x14..+0x1F
- Built by `func_80048BA4` (asm-only), consumed by `func_80052930`
  (text1b.c:10798, GTE MVMVA wrapper)

### `0x8008_EAxx` range — `func_8003CF84` counter thresholds (§15)
`func_8003CF84` compares the counter `D_800A37B8` with each s16 threshold
and acts on a match (sounds go through `func_8005C650`):
- `D_8008EAC0[34]` (0x8008EAC0) — indexed by the fighter's `unk_0A` class
  index; mostly 130, else -1 (0xFFFF), 135, 230 or 0; queues `40 * p + 0x2D`
- `D_8008EB04` (155) / `D_8008EB06` (159) — queue `40 * D_800A3748 + 0x31`
  / `+ 0x36`
- `D_8008EB08` (160) — queues 0x53 or 0x2B by `D_800A3748`
- `D_8008EB0A` (198) — queues 0x71
- `D_8008EB0C` (159) — builds a point with `func_80021D10`, adds
  `D_8008EB10` / `D_8008EB14` / `D_8008EB18` (s32, = (0, -800, 0)) and
  passes it to `func_800618B4`
- `D_8008EB1C` (12 bytes; not read by `func_8003CF84`) — two-byte rows
  `func_800224E0` indexes by `D_800A384C` and matches against the nibbles
  of `D_8008DB1C`

### `0x8008_3Exx` range — DispStuff IRQ-callback alabels (§22)
- `g_irq_handler_entry_no_pri` (0x80083EDC) — alabel inside DispStuff,
  "fire pending primary + always-call secondary"
- `g_irq_handler_entry_with_pri` (0x80083F1C) — alabel, one-shot
  deferred-fire using `g_alarm_pending_priority_flag`

## See also

- [README.md](README.md) — entry point with the high-level architecture
  diagram
- [memory_layout.md](memory_layout.md) — PS1 memory regions and gp window
- [psyq_usage.md](psyq_usage.md) — PsyQ library identification
- memory_layout.md § Naming-pass data clusters — cluster summary (full
  traces at `pre-slim-2026-10-01:docs/engine/recent_naming_findings.md`)
- `named_syms.txt` and `symbol_addrs.txt` (project root) — primary symbol
  vocabulary (authoritative name→address map)
- `engine/queue.json` (project root) — the ordered decomp worklist (functions
  still carrying cheats)
