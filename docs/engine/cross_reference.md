# Cross-Reference: Subsystems, Files, Functions, Globals

This is the lookup index. If you have a function or global and want to
know which subsystem owns it, look here.

## Subsystem → primary file → key functions → key globals

### Boot + main loop
- **File:** `ings.c` (high-level), `ings2.c` (BIOS shims), `main.c`
  (mid-game state)
- **Key functions:** `cpu_set_move_command_and_dir_for_no_action_2`
  (main entry, ings.c:584), `sys_Init`, `sys_GameInit`, `motion_Open`
  (CTOR runner), `gnd_disp_loop_ctrl`
- **Key globals:** `D_800A3834` (mode), `g_frame_parity` (D_800A36AC) (frame slot),
  `g_disp_enable` (display), `D_8008D070` (CTOR table), `D_8008D090`
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
  `g_anim_hit_flags`, `g_anim_hit_data`
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
- **Key globals:** `MotDataBaseAddress` (0x80104F38), `D_800A3207`
  (motion FSM state), `g_motion_play_countdown` (D_800A334C) (motion frame counter)
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
- **File:** `sound.c` (snd_* API + game state), `main.c` (spu_*
  low-level + voice allocator), `text1a_c.c` (seq_*)
- **Key functions:** `snd_LoadBgm`, `snd_PlayBgm`, `snd_LoadSe`,
  `snd_PlaySe`, `snd_LoadSelection`, `snd_StopAll`, `snd_SetVolume`,
  `spu_NotifyChannel`, `coli_HitPauseKatana` + `coli_HitPauseKatana_2`
  + `exec_game` (voice-key allocator — misnamed),
  `marionation_camera_Init_80036064` (XA stream), `seq_Start`,
  `seq_Reset`, `single_game_VoiceContorol`
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
  state!), `g_file_data_buf`, `g_cd_*` (libcd shadows),
  `g_memcard_busy`, `g_memcard_data`
- **Doc:** [file_io.md](file_io.md)

### Menus / UI / Fades
- **File:** `code6cac_c2.c`, `code6cac_c_ab.c`, `code6cac_c_mid.c`,
  `config.c` (options), `text1a.c` (mental gauge / `efc_*`)
- **Key functions:** `md_game_check_change_sub_mode`,
  `md_menu_logo_exec` (asm-only), `game_SetControllerPorts`,
  `game_SetPlayerCount`, `stage_ExecInitFunc`, `stage_InitCollision`,
  `disp_mario_jimaku` (subtitles, asm), `DispPracticeMenuTex_*`,
  `DispUpdateStatusMessage`, `func_80016E60` (pause submenu),
  `FadeOut_*`, `CheckFadeEnd`, `InitFadePanel`
- **Key globals:** `g_disp_fade`, `g_game_mode`, `g_game_pause`,
  `g_color_mode` (grayscale flag), `D_80102794` (pad input mask),
  `selCharaID`, `g_char_setup_tbl`
- **Doc:** [menus.md](menus.md)

### Replay / Special Camera
- **File:** `code6cac_b2.c` (replay_camera_Init etc.), `text1b.c`
  (replay_camera_attack etc.), `code6cac_c2.c`
  (replay_camera_get_attack_number), `text1a_c.c`
  (replay_camera_rob_back_loose3)
- **Key functions:** `replay_camera_Init`, `replay_camera_attack`,
  `replay_camera_get_attack_number`, `replay_camera_rob_back_*`,
  `special_camera_Exec` (mostly empty stub),
  `special_camera_get_rot_dir` (disc loader),
  `marionation_camera_Init_80036064` (XA),
  `marionation_camera_Init_80037468` (cinematic-mode entry),
  `marionation_camera_GetMaxFrame`, `game_FrameInit`, `game_FrameLoop`
- **Key globals:** `SpecialCam` (`0x8008EC34` — disc table of
  cinematic entries), `D_8008EC38` (length table),
  `D_80101E5C..D_80101EA4` (replay state machine)
- **Doc:** [replay.md](replay.md)

### Stage / World
- **File:** `config.c` (stage_*), `code6cac_c2.c` (stage open),
  `text1a*.c` (gnd_* helpers)
- **Key functions:** `stage_GetId`, `stage_GetVariant`,
  `stage_GetDataPtr`, `stage_InitCollision`, `stage_ApplyLighting`,
  `stage_ClearLighting`, `stage_ExecInitFunc`, `gnd_init_*`,
  `gnd_get_fog`, `gnd_set_fog*`, `gnd_open`, `gnd_close_*`,
  `gnd_disp_loop_ctrl` (the gameplay draw)
- **Key globals:** `g_stage_id`, `g_stage_variant`, `g_stage_data`,
  `g_stage_collision` (32x32 grid), `g_stage_light_pos`,
  `g_stage_light_dir`, `g_stage_init_tbl` (per-stage init fn ptrs)

### Character / Player
- **File:** `code6cac.c` (player_*), `text1b.c` (`obj_*`)
- **Key functions:** `player_SetCharId`, `player_Destroy`,
  `game_GetPlayerCount`, `obj_InitChars`, `obj_InitTask`,
  `obj_InitPair`, `obj_InitAll`, `obj_Reset`, `obj_ExecTask`,
  `obj_InitTaskCamera`, `obj_UpdatePosition`
- **Key globals:** `g_player_ptrs`, `g_player_char_ids`, `g_char_data`,
  `g_char_setup_tbl`, `chractar_use_pset_combo_id_table`

### System / IRQ / Timer
- **File:** `ings2.c`, `system.c` (CD overlap)
- **Key functions:** `sys_VSync`, `sys_SetVsyncMode`, `sys_SetTimer`,
  `irq_DisableInterrupts`, `irq_AcknowledgeVblank`,
  `irq_EnableInterrupts`, `irq_SetAlarm`, `irq_Reset`,
  `EnterCriticalSection`, `ExitCriticalSection`, `func_80082A14`
  (VSync wait loop)
- **Key globals:** `g_sys_irq_vtable`, `g_sys_vblank_count`,
  `g_sys_vsync_mode`, `g_sys_timer`, `g_sys_video_mode`,
  `g_sys_dma_region`

## Reverse index: global → subsystem

If you've seen a global in the source and want to know what it does:

### `0x800A_0xxx` range (low BSS)
- `g_player_ptrs` (0x800A9A10) — Character/Player
- `g_player_char_ids` (0x80094B88) — Character/Player
- `g_stage_init_tbl` (0x800948BC) — Stage
- `g_snd_se_bank` (0x80099C34) — Sound
- `g_stage_id`, `_variant`, `_data` (0x80099478..7C) — Stage
- `g_cd_*` (0x800A11B4..14C0) — CD-ROM (system.c)
- `g_sys_*` (0x800A14CC..2664) — System/IRQ
- `g_spu_*`, `g_snd_*` (0x800A2870..3404) — Sound
- `g_game_*` (0x800A322C..3374) — Game state
- `g_disp_enable`, `g_disp_fade` (0x800A3768, 0x800A36A8) — Display
- `g_game_timer` (0x800A3790) — Game state
- `D_800A3834` — game-mode dispatch register
- `g_char_data` (0x800A6690) — Character (14 KB region)
- `g_stage_collision` (0x800A8FB0) — Stage (32x32 grid)
- `SpecialCam` (0x8008EC34) — Replay/Camera disc table

### `0x800E_xxxx` range (mid BSS)
- `g_cam_matrix`, `g_cam_bone_data*` (0x800EEDB0..) — GPU camera
- `g_cam_fov_*` (0x800F62F8..) — GPU camera
- `g_snd_config_tbl` (0x800EF7BC) — Sound
- `g_snd_fade_curve` (0x800EF800) — Sound
- `g_snd_ch_data` (0x800EF848) — Sound (per-channel data)
- `videoDec` (0x800F0D80) — MDEC overlay
- `MarioCam_str` (0x800F19D0) — Replay/Special-cam debug
- `g_gpu_color_table` (0x800F189C) — GPU
- `g_color_mode` (0x800F6652) — Menus (grayscale)
- `g_game_p1_ctrl`, `g_game_p2_ctrl`, `g_game_mirror_mode`,
  `g_game_pause` (0x800F6654..665C) — Game state
- `g_anim_func_table` (0x800F66A0) — Motion/Animation
- `g_disp_fb_base`, `g_disp_fb_flag` (0x800F7438, 0x800F7450) — GPU
- `g_file_data_buf` (0x800F6740) — File I/O
- `g_memcard_busy` (0x800FF578) — Memcard
- `g_pad_data` (0x800FF580) — Input

### `0x801xx_xxxx` range (high BSS, data + tables)
- `g_memcard_slot` (0x80101BCC) — Memcard
- `g_memcard_data` (0x80103600) — Memcard
- `MotDataBaseAddress` (0x80104F38) — Motion
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

## See also

- [README.md](README.md) — entry point with the high-level architecture
  diagram
- [memory_layout.md](memory_layout.md) — PS1 memory regions and gp window
- [psyq_usage.md](psyq_usage.md) — PsyQ library identification
- `named_syms.txt` and `symbol_addrs.txt` (project root) — primary symbol
  vocabulary
- `SUBSYSTEM_MAP_2026-05-12.md` (project root) — object-level address map
  for the source files (text1b.c landmark map)
