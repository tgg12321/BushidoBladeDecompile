# Data symbol evidence

Per-symbol prose evidence files for `D_*` globals that have been
hand-reviewed and assigned a proposed name. Parallel to the
function-side `docs/naming/evidence/` directory.

## How this differs from `data_symbols_quick_wins.md`

`docs/naming/data_symbols_quick_wins.md` is a mechanical
classification of every D_* symbol by access-width pattern (lw/lh/lb
only, function-pointer call shape, GTE direct reference, etc.). It
produces type-style names like `wtbl_*` / `stbl_*` / `btbl_*` /
`fp_*` -- useful as quick hints, but explicitly NOT semantically
meaningful and explicitly NOT for bulk-renaming.

The files in this directory take the opposite approach: pick one
data symbol, read every function that touches it, document the
access pattern, and propose a *semantically meaningful* name (e.g.,
`g_rand_state`, `g_disp_config`, `g_tpage_slot_ptr`). One file per
symbol, more discursive than mechanical.

## What's here

Initial batch (2026-05-17), six symbols referenced by the
already-named functions from the 10-function batch in `evidence/`:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_800F1848.md](D_800F1848.md) | `g_rand_state` | high | LCG state owned exclusively by `bb2_rand` |
| [D_8008F13C.md](D_8008F13C.md) | `g_special_cam_command_table` | high | 8-byte-stride command-descriptor table touched only by `special_cam_issue_command` |
| [D_80103608.md](D_80103608.md) | `g_tpage_slot_ptr` | medium-high | Paired-array slot pointer pool (manager pair + 10 consumers) |
| [D_80103658.md](D_80103658.md) | `g_tpage_slot_count` | medium-high | Partner count array to the above |
| [D_8009BD24.md](D_8009BD24.md) | `g_disp_config` | medium-high | Display-config struct loaded by `disp_load_config_from_buf` |
| [D_800A379E.md](D_800A379E.md) | `g_motion_state_code` | medium-high | Motion-state s16 returned by `game_state_advance` |

Per-character state struct batch (2026-05-17), three symbols + one
struct-shape documentation:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_80101F00_char_state.md](D_80101F00_char_state.md) | `g_char_state[]` (struct base) | medium | 1100-byte per-character struct; partial field layout decoded |
| [D_800A3860.md](D_800A3860.md) | `g_motion_data_base_ptr` | medium-high | Array of per-character data-region base pointers (3 sibling slots initialised together at known upper-RAM addresses) |
| [D_801027B0.md](D_801027B0.md) | `g_motion_keyframe_table_ptr` | medium | 20-byte-stride per-character motion-keyframe table pointer array (5 tables per character?) |

The per-character struct base (`D_80101F00`) doesn't have an
auto-detected symbol -- no asm references byte 0 of the struct
directly. The evidence file documents the struct *shape* and the
decoded fields so a future reviewer can either name the base
manually or name each field-level symbol consistently.

Top-30 most-referenced unnamed globals batch (2026-05-17, batch 4),
16 symbols mined from the highest-reference-count entries in the
unnamed `D_*` pool:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_8008D118.md](D_8008D118.md) | `g_isqrt_lut` | high | byte LUT indexed by sum-of-squares; used in marionation_camera_Exec + cpu_check_run_attack distance computations |
| [D_800A36AC.md](D_800A36AC.md) | `g_frame_parity` | high | `& 1` selector for alternating framebuffers/CLUTs in func_80016E60 + func_8003DBE4 |
| [D_800A3708.md](D_800A3708.md) | `g_stage_data_ptr` | high | stage data pointer; +0x4C/+0x54 are collision centers in stage_InitCollision |
| [D_80101EC8.md](D_80101EC8.md) | `g_practice_menu_table` | high | base of per-character records, stride 0x44C matches D_80101F00 char_state stride |
| [D_800A3748.md](D_800A3748.md) | `g_practice_menu_index` | high | index into g_practice_menu_table; -1 sentinel |
| [D_800A378C.md](D_800A378C.md) | `g_palette_base_ptr` | medium | 4-byte-stride palette table indexed in func_8003DBE4 / func_8003DDF8 |
| [D_80102794.md](D_80102794.md) | `g_seq_active_voice_mask` | medium | bitmask of currently-active SEQ voices, diffed each tick in single_game_VoiceContorol |
| [D_8009BC1C.md](D_8009BC1C.md) | `g_efc_buki_dispatch_table` | medium | function-pointer table indexed by D_800A3580 in func_8006EACC |
| [D_800A3580.md](D_800A3580.md) | `g_efc_buki_dispatch_idx` | medium | index into above; range-check `(idx - 2) >= 2U` |
| [D_800A3820.md](D_800A3820.md) | `g_hira_packet_cursor` | medium | s32-stride packet write cursor in DispHira; reset to &D_80102C00 |
| [D_800F62E0.md](D_800F62E0.md) | `g_mdec_fp_buf` | medium | s16 coefficient buffer used by decBs0 (MDEC bitstream decoder) |
| [D_800A38B4.md](D_800A38B4.md) | `g_cpu_move_pattern_cursor` | medium | 4-byte-aligned cursor in cpu_get_move_pattern_table_number (198 references, the most-referenced unnamed global) |
| [D_800A374C.md](D_800A374C.md) | `g_dma_buf_base` | medium | shared >16 KiB workspace; base for OT in _GetBattleSwichData, data buffer at +0x401C |
| [D_800A3834.md](D_800A3834.md) | `g_game_mode_code` | medium | game state enum (0/1/9/0xD/0x1B); set by scene_teardown variants |
| [D_800A38DC.md](D_800A38DC.md) | `g_voice_state_code` | medium | voice subsystem state (0/3/5), companion to g_game_mode_code |
| [D_800A31FC.md](D_800A31FC.md) | `g_damage_debug_enabled` | medium | toggle for damage_DebugDisp HUD |

The 6 highs from this batch were applied to `named_syms.txt` in the
same commit (SHA1-verified clean rebuild).

Continuing pass batch 5 (2026-05-17), next 18 symbols from ranks 31-60:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_800A3044.md](D_800A3044.md) | `g_spu_reg_base_ptr` | high | SPU register block pointer; volatile-u16 +0xA OR'd with 0x20 == voice on/off |
| [D_80106F28.md](D_80106F28.md) | `g_seq_bank_table` | high | bank pointer table; per-channel stride 0xB0 matches func_8008{5114,5210,5064} |
| [D_800A334C.md](D_800A334C.md) | `g_motion_play_countdown` | high | u8 watchdog set to 0x5A (1.5s); paired with g_seq_active_voice_mask |
| [D_800A3874.md](D_800A3874.md) | `g_match_round_index` | high | monotonic round counter; indexes D_800A377C and the per-round ranking table |
| [D_800A3418.md](D_800A3418.md) | `g_gnd_landing_rand_seed` | medium | XOR'd with bb2_rand each tick; scaled to spawn-offset values |
| [D_800A377C.md](D_800A377C.md) | `g_match_round_result_table` | medium | byte array indexed by g_match_round_index; {0,1,2} outcomes |
| [D_800F33D8.md](D_800F33D8.md) | `g_pad_analog_state` | medium | block address passed to pad_FuncAnalog helpers |
| [D_80104E80.md](D_80104E80.md) | `g_blood_spot_timer` | medium | SetBloodSpot countdown (60 or 120 frames) |
| [D_800A36A4.md](D_800A36A4.md) | `g_se_active_set_id` | medium | currently-loaded SE data set ID in se_data_set |
| [D_800F1140.md](D_800F1140.md) | `g_text_render_v3_0` | medium | first of 3 consecutive cached vector slots in text/UI render |
| [D_800F1144.md](D_800F1144.md) | `g_text_render_v3_1` | medium | second of the v3 triple |
| [D_800F1148.md](D_800F1148.md) | `g_text_render_v3_2` | medium | third of the v3 triple |
| [D_800A3464.md](D_800A3464.md) | `g_text_render_mask` | medium | mask companion to the v3 triple |
| [D_800A31F4.md](D_800A31F4.md) | `g_pad_analog_mode` | medium | small enum (3/5) in pad_FuncAnalog |
| [D_800A3207.md](D_800A3207.md) | `g_motion_init_flag` | medium | init-once flag in motion_SetMotion |
| [D_800A3894.md](D_800A3894.md) | `g_seq_start_pending` | medium | pending-seq-start flag checked in md_game_check_change_sub_mode |
| [D_800A3844.md](D_800A3844.md) | `g_mental_bar_cursor` | medium | suDispMentalBar packet append cursor |
| [D_800A38A4.md](D_800A38A4.md) | `g_cpu_move_dir_state` | medium | CPU AI move-dir sub-state, range-checked {4,5} |

The 4 highs from this batch were applied to `named_syms.txt` in the
same commit.

Continuing pass batch 6 (2026-05-17), next 27 symbols from ranks 61-90:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_80101E02.md](D_80101E02.md) | `g_replay_pos_x` | high | X of replay-camera pos triple (paired with E04/E44) |
| [D_80101E04.md](D_80101E04.md) | `g_replay_pos_y` | high | Y of replay-camera pos triple |
| [D_80101E44.md](D_80101E44.md) | `g_replay_pos_z` | high | Z of replay-camera pos triple; passed to replay_camera_get_attack_number |
| [D_800A37C8.md](D_800A37C8.md) | `g_pad_analog_substate` | high | gates pad_FuncAnalog state transitions {0, 3} |
| [D_800F65F8.md](D_800F65F8.md) | `g_match_round_byte_a` | high | per-round byte indexed by g_match_round_index * 2 |
| [D_800F19BC.md](D_800F19BC.md) | `g_irq_dispatch_counter` | medium | sequence counter in system.c IRQ dispatch |
| [D_80101E70.md](D_80101E70.md) | `g_replay_camera_state` | medium | volatile state in replay_camera_Init |
| [D_800A33EA.md](D_800A33EA.md) | `g_gnd_target_id` | medium | -1 sentinel; target ID in gnd_close |
| [D_800A3740.md](D_800A3740.md) | `g_replay_camera_mode` | medium | small enum {1, 3, 4} in replay_camera_rob_back_loose2 |
| [D_800A36FA.md](D_800A36FA.md) | `g_gnd_init_flag` | medium | init flag cleared by gnd_init helpers |
| [D_80101F32.md](D_80101F32.md) | `g_seq_state_p1` | medium | SEQ state code (paired with D_8010237E p2 side) |
| [D_800A38C4.md](D_800A38C4.md) | `g_title_seq_state` | medium | title-screen SEQ active flag |
| [D_800A384C.md](D_800A384C.md) | `g_practice_select_cursor` | medium | indexes halfword table at D_8008EB1C stride 2 |
| [D_800A3500.md](D_800A3500.md) | `g_text1b_passthrough_arg` | medium | stashed arg passed to func_8006E950 |
| [D_800A3518.md](D_800A3518.md) | `g_text1b_audio_state_a` | medium | audio-stream state, address-passed to func_8006E390 |
| [D_800A3528.md](D_800A3528.md) | `g_text1b_audio_counter` | medium | tick counter paired with audio_state_a |
| [D_800A37A8.md](D_800A37A8.md) | `g_mental_bar_packet_buf` | medium | byte array, indexed by D_800A37A0 counter |
| [D_800F6608.md](D_800F6608.md) | `g_grid_pos_vec3` | medium | Vec3 of position; copied from player struct +0xB8 |
| [D_800A3929.md](D_800A3929.md) | `g_cpu_pattern_tick` | medium | tick counter in cpu_get_move_pattern_table_number |
| [D_800A376A.md](D_800A376A.md) | `g_tslTm_loaded_flags` | medium | per-slot 4-bit flags in tslDrTex1Init |
| [D_800EFB38.md](D_800EFB38.md) | `g_sound_buf_a` | medium | sound buffer A (paired with B at D_800EFB78) |
| [D_800EFB78.md](D_800EFB78.md) | `g_sound_buf_b` | medium | sound buffer B |
| [D_8010277E.md](D_8010277E.md) | `g_cpu_dir_buffer_e` | medium | byte in D_8010277C-F CPU AI dir cluster |
| [D_800A32BC.md](D_800A32BC.md) | `g_text1b_render_timer` | medium | 10-frame decrementing render timer |
| [D_800A37D4.md](D_800A37D4.md) | `g_gnd_fog_value` | medium | volatile fog value mirrored to D_800A3720 |
| [D_800A38A0.md](D_800A38A0.md) | `g_dialog_active_flag` | medium | dialog/samnail-window active flag |
| [D_800A3878.md](D_800A3878.md) | `g_input_flag_struct_ptr` | medium | pointer to struct with bit-flag field at +0x3 |

5 highs applied to named_syms.txt this batch.

## Workflow

Each evidence file follows the same skeleton:

1. **Header** -- address, access width / size, touchers (count +
   files), proposed name, confidence.
2. **Sole / primary toucher** -- a short prose paragraph + the C
   body of the function that most clearly establishes the role.
3. **Readers / writers** -- enumeration of the other touchers and
   what they do with the symbol.
4. **Why this name** -- the reasoning + the confidence rationale.
5. **Confidence-raising next steps** -- what a reviewer could do to
   lift the proposal from its current confidence to higher.
6. **Reviewer alternatives** -- plausible alternative names with
   when each would be preferred.

The format mirrors the function evidence in `../evidence/` so a
reviewer who has read those will find the structure familiar.

## Application gating

These are proposals only. To actually rename a symbol in
`named_syms.txt`, follow the same gating as function renames:

1. Move the line from `undefined_syms_auto.txt` to
   `named_syms.txt`, replacing `D_<addr>` with the proposed name.
2. Re-run splat so the new name propagates to the generated header.
3. `make` and verify SHA1 still matches the original.
4. Spot-check that the new symbol name reads naturally in 2-3
   touching functions.

A failed step 3 means the proposed name conflicts with something
unexpected (e.g., a function with the same name) -- back out.

A passing step 4 means the rename is a net readability improvement.
