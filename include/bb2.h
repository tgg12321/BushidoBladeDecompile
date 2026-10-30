#ifndef BB2_H
#define BB2_H

/* Declarations of the game's objects and functions for the translation units in
 * src/main/ (SLUS_006.63's game code); their types are in game.h. */

#include "game.h"

extern FileRecord g_saved_record;
extern s16 g_camera_distance_offset;
extern GpuDb g_gpu_db[2];
/* The current frame's ordering table (OT entry n is g_gpu_ot_ptr + n). */
extern u32 *g_gpu_ot_ptr;
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void gpu_HideResetAndClear(void);
extern Unk800EF848Rec g_gpu_vram_scrolls[];
extern u16 g_gpu_vram_scroll_presets[][7];
/* 58 byte pairs (0x80099CC8..0x80099D3B), each byte an index into
 * g_resource_entry_request_state or 0xFF for none. */
extern u8 g_resource_entry_linked_entries[58][2];

/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *g_gpu_ot_offset_ptr;

extern DR_MOVE g_gpu_fog_row_moves[31][2];
extern DR_MOVE g_gpu_fog_block_moves[19][2];
extern DR_MOVE g_gpu_draw_move_packets[2][10];

/* The VRAM rectangle font_LoadImage loads the image at g_font_image into
 * (x 0x3F0, y 0x1DC, 16 x 36). */
extern RECT g_font_rect;

extern u8 g_local_ot_draw_mode;
extern u8 g_gpu_jitter_burst_enabled;
extern s16 D_800F665C;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_loaded_resource_selector;
extern s16 g_resource_has_retained_subblocks;
extern Unk800F1198Record g_eff_billboards[];
extern Unk800A9CF8Rec g_node_frame_animation_context;
extern Unk800F0EC8Record g_eff_yawed_quad_slot_positions[][10];
extern Unk800F0E38Record g_eff_billboard_pair_positions[12];
extern MenuState g_row_menu_row_state;

/* The three scroll pages' origins: s16 (x, y) pairs for pages 4..6
 * (menu_UpdateChoiceGrid). */
extern s16 g_entry_menu_choice_grid_draw_offsets[3][2];

/* The current scroll offset, one s16 per axis (menu_UpdateChoiceGrid). */
extern s16 g_entry_menu_transition_offsets[2];

/* 20 flag bytes indexed by entry id (bit 0 = selectable; mask 4 << player,
 * i.e. bit 2 + player = taken by that player); d_8C4E4.c, read by 64FD8.c. */
extern u8 g_sel_entry_availability_and_claims[20];
extern Unk8009BCF8Record g_sel_grid_page_cells[2][10];

/* Two 2-byte records, {0x01, 0x02} and {0x03, 0x04}, indexed [row][col] by
 * menu_HandleSelPhase0CursorInput,
 * menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles and
 * menu_InitSelScreen. */
extern u8 g_sel_sheet_code_pairs[2][2];

extern u8 *g_sel_screen_work;
#define SELWORK ((SelWork *)g_sel_screen_work)

/* One PsyQ MATRIX: func_8004A940 loads it as the GTE rotation, reads t[] and
 * passes it to gte_MulMatrix0ClearTrans;
 * node_SetSlotRelativeMatrixQueueRetainedNodesAndLinkOrderingTable writes m[][]
 * and passes t to ApplyMatrix. */
extern MATRIX g_node_slot_relative_matrix;

extern Unk8009BC94Record g_entry_menu_row_choice_positions[][6];
/* The SEL.BIN menu's tables (d_8C41C.c, read by 5ED34.c): the per-mode
 * update handlers (StepSlideBuildFrameRecordAndDispatchStateHandler),
 * menu_UpdateRowCursors' row values, the entry grid and its per-value flags. */
extern void (*g_entry_menu_handlers[7])(Unk8006EACCRec *);
extern u8 D_8009BC38[];
extern SelectEntryE534 g_entry_menu_grid_entries[][6];
extern u8 g_entry_menu_entry_flags[];
/* Table of 4 sprite-sheet headers, 12 bytes each
 * (gpu_DrawTwoDigitCounterWithOptionalSecondSide,
 * gpu_DrawPackedComparisonRowsAndTotals). */
extern Unk8009B0E0Record D_8009B398[4];
extern Unk8009B400Record g_gpu_decimal_digit_cells[10];
extern Unk8009B400Record D_8009B458[3][2];
extern Unk8009B450Record g_gpu_time_field_line_origins[2];

/* Two 8-byte sprite cells, the cell table
 * gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle hands gpu_AddScaledSpriteQuads
 * for the row's header record 2 (cell 0) and records 3/4 (cell 1). */
extern Unk8009B400Record D_8009B388[2];

/* 2 x 2 table of 8-byte sprite records (gpu_DrawPairedMarkRowsAndTimeFields).
 */
extern Unk8009B400Record D_8009B5F0[2][2];

/* 2 x 2 table of 8-byte sprite records (gpu_DrawPackedComparisonRowsAndTotals).
 */
extern Unk8009B400Record D_8009B490[2][2];

extern Unk8009B0E0Record g_gpu_mode_panel_sheet_headers[9];
extern Unk8009B0E0Record D_8009B14C;
extern Unk8009B0E0Record D_8009B158;

/* 2 x 2 table of 8-byte sprite records;
 * gpu_DrawModeSizedPanelWithSelectedCellOpaque passes row 1 as the second
 * draw's cell table and stores column 1's x in each row. */
extern Unk8009B400Record D_8009B164[2][2];

/* Two 8-byte sprite records, a cell table
 * gpu_DrawModeSizedPanelWithSelectedCellOpaque passes (the sheet header's count
 * is 2) after storing each record's x. */
extern Unk8009B400Record D_8009B184[2];

/* One 8-byte sprite cell (x, y = 0, u, v = 0, w 0x14, h 0x0E), the cell
 * table gpu_DrawSpriteAtPackedTablePosition hands gpu_AddSpriteCells in its
 * descriptor. */
extern Unk8009B400Record D_800A328C;

extern Unk8009B2BCRecord g_panel_frame_sizes[3];
extern Unk800EFAE8Rec g_motion_playback_record;
extern Unk800F0C10Record g_eff_particle_group_positions[4][3];

/* Per-effect-mode state, modes 0..0x11 (eff_DrawAnimatedQuads): the timers here
 * and the position records in g_eff_animated_quad_positions. The init functions
 * eff_InitCosineScaledQuad.. func_800652AC fill them; the step functions
 * eff_DrawAndAdvanceCosineScaledQuad..func_800657B0 advance the timers. */
extern s16 g_eff_animated_quad_counters[18];

extern Unk800F0C10Record g_eff_animated_quad_positions[18];
extern Unk800948BCEntry g_resource_selector_callbacks[];

/* The per-mode main-loop handlers, indexed by g_main_loop_handler_index. */
extern void (*g_main_loop_handlers[])(void);

extern u8 g_fight_step_table[][4];

/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF (d_7D920.c). */
extern u8 g_sqrt_table_u8[0x400];

extern Unk8008DCCCEntry g_motion_fighter_data_files[18];

/* 3 x 3 s16 angle offsets, [row][col] from the pad bits
 * (fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow,
 * fighter_UpdateInputSteeredForwardMotion). */
extern s16 g_fighter_dpad_angle_offsets[3][3];

/* Judge: the sine table, one full turn in 0x1000 steps, 1.0 = 0x1000
 * (cos(a) = Judge[(a + 0x400) & 0xFFF]). */
extern s16 Judge[0x1000];

/* The 16 slots eff_SpawnJitteredBillboards spawns and
 * eff_DrawAndAdvanceJitteredBillboards draws (bit i of
 * g_eff_jittered_billboard_slot_mask live): slot i's position. */
extern Vec3i32 g_eff_jittered_billboard_positions[16];

extern Unk800EED10Entry g_arena_blocks[10];

/* The two characters' hit records (0x800F5F68..0x800F62D7):
 * fighter_ScaleBoneHitRecords fills g_fighter_bone_hit_records[ch] from the
 * template g_fighter_bone_hit_record_templates, offsets and limits scaled. */
extern BoneHitRec g_fighter_bone_hit_records[2][22];

extern BoneHitRec g_fighter_bone_hit_record_templates[22];

/* The 6 slots eff_SpawnRotatedQuadCluster spawns and
 * eff_DrawAndAdvanceRotatedQuadClusters draws (bit i of
 * g_eff_rotated_quad_cluster_slot_mask live): slot i's position and its
 * RotMatrixZYX angles. */
extern Vec3i32 g_eff_rotated_quad_cluster_positions[6];

extern SVec4i16 g_eff_rotated_quad_cluster_rotations[6];
extern s16 g_eff_rotated_quad_cluster_ages[6];
extern Unk800EFB78Entry g_snd_queue[24];
extern Unk80101EC8Record D_80101EC8[];
extern const char D_800109C8[];
extern u8 g_snd_stage_cd_audio_entry_indices;
extern u8 g_fight_config_id_data_rows[];
extern u8 D_8008D55C;
extern u8 g_fight_selection_object_ids[];

/* Attachment point sets fighter_UpdateBoneWorldData rotates by a character's
 * bone matrices: D_8008D86C[weapon_id] (D_8008D864[weapon_id] points;
 * D_8008D774 when weapon_entry_id == 50) and D_8008D88C[wielded_object_kind]
 * (two points, when wielded_object_live_state
 * != 0). g_unit_y_vector is the unit y vector (0, 0x1000, 0). */
extern u8 D_8008D864[8];

extern SVec4i16 *D_8008D86C[8];
extern SVec4i16 *D_8008D88C[32];
extern SVec4i16 D_8008D774[2];
extern SVec4i16 g_unit_y_vector;
extern u8 g_fighter_group_flags[];
extern u8 g_fighter_forward_move_rate_limit_indices[0x48];
extern s16 D_8008DA50[]; /* [character_data_index] (fighter_Update) */
extern s16 D_8008DA94[]; /* [character_data_index] (fighter_Update) */
extern s16 D_8008DAD8[]; /* [character_data_index] (fighter_Update) */
/* [character_data_index][weapon_id] -> Unk80101EC8Record.motion_data_id
 * (motion_LoadFighterData) */
extern u16 g_fighter_motion_data_ids[27][8];
/* [character_data_index][weapon_id] -> Unk80101EC8Record.default_variant_index
 */
extern u8 g_fighter_default_variant_indices[27][8];
/* [character_data_index][weapon_id] -> Unk80101EC8Record.base_move_frame_step
 */
extern u16 g_fighter_base_move_frame_steps[27][6];
/* [character_data_index][weapon_id] -> Unk80101EC8Record.base_power / power */
extern u16 g_fighter_base_powers[27][6];
extern Tbl8008E194 g_object_kind_params[];

/* 8 byte pairs ({0,0} {1,0} {2,0} {3,0} {4,1} {5,1} {6,1} {7,0});
 * ResetFlowAndFetchRowTablePair returns [n][0] and caches [n][1] in
 * g_menu_pending_entry_extra_slot_count, n = g_working_record.fight_mode. */
extern u8 g_menu_mode_selector_and_extra_slot_counts[8][2];

extern Obj80106A78 g_object_pool[12];
/* [character_data_index][i] -> Unk80101EC8Record.carried_object_kinds[i]
 * (object_InitKindQueue) */
extern s8 g_fighter_initial_carried_object_kinds[27][5];
/* [character_data_index] -> Unk80101EC8Record.bone_group_6_9_lost_step_scale */
extern u16 g_fighter_lost_bone_group_step_scales[28];
/* [character_data_index][i] ->
 * Unk80101EC8Record.bone_group_10_13_step_scales[i] */
extern u16 D_8008E3F8[27][4];
/* [character_data_index][i] -> Unk80101EC8Record.unk_27E[i] */
extern u16 D_8008E4D0[27][4];
extern u8 D_8008E5A8[];
/* [character_data_index][weapon_id] of D_80101EC8 */
extern u8 g_resource_default_pair_selections[][8];
/* [character_data_index][weapon_id] of D_80101EC8 */
extern u8 g_resource_alternate_pair_selections[][6];
extern u8 D_8008E748;
extern u8 g_fight_opponent_stage_indices;
extern Unk8008EA44Entry g_snd_stage_sound_slot_trigger_delays[5];

/* per-stage s16 table, 34 entries */
extern s16 D_8008EAC0[34];

extern s16 D_8008EB04;
extern s16 D_8008EB06;
extern s16 D_8008EB08;
extern s16 D_8008EB0A;
extern s16 D_8008EB0C;
extern s32 D_8008EB10;
extern s32 D_8008EB14;
extern s32 D_8008EB18;
extern u8 g_motion_preferred_variant_nibble_pairs[][2];
/* [weapon_id][flag] -> Unk80101EC8Record.weapon_entry_id */
extern u8 D_8008EB28[8][2];
extern u8 D_8008EB38[8]; /* [weapon_id] -> Unk80101EC8Record.weapon_entry_id */
extern Tbl8008EB54Entry g_fight_midpoint_effect_offsets[6];
extern u8 g_fight_midpoint_effect_event_ids[6];
extern u8 g_resource_kind_entry_ids[];
/* per-limb angle (object_DeflectFromFighterHit: [j], j < 22) */
extern u16 g_object_bone_hit_deflect_angles[22];
extern s32 g_fight_step_row_sound_offsets[];
extern s32 g_eff_yawed_quad_tex_selector_command_bits[];
extern u8 g_snd_stage_sequence_base_sounds[6];
extern Unk8008EA44Entry g_snd_stage_sequence_cue_ticks[6];
extern u8 g_fight_time_limit_choices[4];
extern s16 g_cdrom_file_group_bases[8];

/* Twelve rows cdrom_StartAudioEntry reads by index: audio_file_index_offset,
 * the group-5 file cdrom_GetFileIndex resolves; xa_channel, cdrom_StartAudio's
 * second argument; audio_end_offset, an offset added to the file's start
 * sector (-1: none). */
typedef struct {
    u8 audio_file_index_offset;
    u8 xa_channel;
    u16 unk_2;
    s32 audio_end_offset;
} Unk8008F13CRow;

extern Unk8008F13CRow g_cdrom_audio_entries[12];
extern u8 g_memcard_title_sjis_digits_0_to_4[];
extern u8 g_memcard_title_sjis_digits[];
extern u8 g_memcard_title_count_class_by_unlock_bit[];
extern u8 *g_fight_command_streams[];
extern u8 g_fight_presentation_playback_file_id_offsets[];
extern u32 g_font_image[];
extern u32 g_gpu_last_fog_ot_index;
extern s32 g_gpu_latched_fog_depth_span;
extern s32 g_gpu_latched_fog_depth_offset;
extern s16 g_gpu_fog_latch_last_grid_and_fog_control;

/* Two per-stage tables, indexed by the stage id
 * gpu_RunGridDrawPassAndQueueFogMoves passes to
 * gpu_LatchFogDepthOnTransitionAndQueueMoves: 38 s32, then s16 pairs ([0]
 * tested, [1] passed to gpu_QueueDepthSpacedMovesAndLinkTerminalPacket; the
 * last pair is zero). */
extern s32 g_gpu_fog_depth_span_limits[38];

extern s16 g_gpu_fog_move_settings[39][2];
extern u16 g_object_scale_factors[];
extern Unk80099D88Rec g_cpu_profiles[];
extern Unk8009A8C8Entry D_8009A8C8[][8];
extern u8 D_8009A9B4[][2]; /* byte pairs (fighter_InitCpuControl) */
/* [g_fighter_group_flags flag] -> 3 bytes
 * (gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels args), stride 4 */
extern u8 g_fight_group_tint_colors[][4];
extern s16 g_fighter_forward_move_rate_limits[4];
extern s32 g_fighter_opponent_response_max_separation;
extern s32 D_800A3140;
extern u8 g_menu_mode_subflow_init_pending;
extern u8 g_fight_reinit_reuses_assets;
extern u8 g_fight_second_fighter_initial_defeats_left;
/* Three angles: gpu_PrepareViewAndFlushDrawQueue passes them to
 * math_RotMatrixZXY; gpu_UnpackGridRecordsNodesAndCellRuns zeroes them. */
extern u16 g_gpu_flush_rotation_angles[3];
extern u8 g_fight_second_fighter_defeats_left;
extern u8 g_fight_view_option;
/* FAKE: word 1 of the 8-byte sio send buffer as its own symbol; declaring
 * g_comb_send_buf s32[2] makes cc1 keep its address live through
 * comb_ExchangePad (35 words differ) */
extern s32 g_comb_send_buf_plus_0x4;
extern s16 g_fight_stage_index;
extern s32 g_gpu_presented_frame_count;
/* the Rec44 view record camera_ApplyFightView / camera_ApplyReplayView last
 * placed (read by snd_QueuePositionalSound as the listener) */
extern Rec44 *g_listener_cam;
extern CdlATV g_cd_atv_target;
/* FAKE: the high halfword (+2) of the s32 link pad word
 * g_comb_received_pad_word as its own symbol. comb_ExchangePad reads it lui /
 * %lo (symbol + 2) while the word and its low half go gp-relative. One-object
 * forms score: Q33 union per pad word 4 (member reads force the address into
 * $s0), (u16) word low reads 4, u16 h[2] 7, s16 h[2] with casts 4, aligned
 * halves 4. Owner ruling Q125: comb_ExchangePad only. */
extern s16 D_800A36C2;
extern s32 g_comb_received_payload_check_word;
extern s16 D_800A36C6;
extern u8 g_fight_saved_bone_group_tim_requests;
extern u16 g_fight_paired_move_flags;
extern u8 g_fight_time_limit_seconds;
/* FAKE: the high halfword (+2) of the s32 link pad word g_comb_prev_send_word
 * as its own symbol. comb_ExchangePad reads it lui / %lo (symbol + 2) while the
 * word and its low half go gp-relative. One-object forms score: Q33 union per
 * pad word 4 (member reads force the address into $s0), (u16) word low
 * reads 4, u16 h[2] 7, s16 h[2] with casts 4, aligned halves 4. Owner
 * ruling Q125: comb_ExchangePad only. */
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern MoveScript *g_fight_paired_move;
extern u8 g_fight_match_result_timer;
extern u8 g_fight_timer_unlock_bit_newly_set;
extern u8 g_main_frame_period;
/* per-player byte, [unk_04] (object_InitKindQueue) */
extern u8 D_800A36F2[2];
extern u8 g_fight_displaced_bone_group_tim_requests;
extern s16 g_fight_tracked_fighter_index;
extern u8 D_800A36F9;
extern u8 g_camera_fight_view_mode;
extern s16 g_camera_turn_easing_countdown;
extern u8 D_800A3712;
extern u8 g_fight_auto_pause_countdown;
extern CdlATV g_cd_atv;
extern s32 g_fight_paired_move_separation;
extern u8 g_fight_start_position_pair_changed;
extern s8 g_fight_followup_fighter_index;

/* Four s16 counters / latches, one per g_snd_stage_sound_slot_specs row
   snd_UpdateStageSounds walks (type 1 latches 1; types 2..6 count up until an
   rng_Next() draw clears them); snd_ResetStageSoundState clears it from the
   last element down. */
extern s16 g_snd_stage_sound_state[4];

extern u8 g_fight_missing_support_fighter_index;

/* The 8-byte CD status/result buffer CdSync / CdReady fill: result[0] is the
 * status byte; cdrom_UpdateAudio hands &result[3] / &result[5] (the reported
 * position) to CdPosToInt. Tentative definition in src/main/cdrom_update.c (Q62
 * global COMMON model). */
extern u8 g_cd_result[8];

extern u8 D_800A3769;
extern u8 g_fight_bone_group_tim_requests;
extern u8 D_800A376B;
extern u8 g_fight_playback_file_base_index;
extern s16 g_fight_paired_move_start_pending;
extern MATRIX *D_800A3778;
/* match-result table: 0/1/2 per match, indexed by g_fight_match_cursor */
extern u8 g_fight_match_results[];

/* Two buffer addresses selected by frame parity: func_80016D78 sets 0x801D8800
 * / 0x801EBC00, menu_RunPauseMenu and main read [g_gpu_presented_frame_count &
 * 1]. */
extern u32 D_800A3770[2];

extern u8 g_fight_no_restart_unlock_bit_newly_set;
extern u8 g_fight_opponent_order_cursor;
extern s32 g_fight_packed_match_scores;
extern u8 g_fight_toggle_option_bits;
extern s32 g_memcard_fd;
extern u8 g_fight_stage_command_count;
extern s32 g_fight_pending_unlock_mask_bits;
extern u8 D_800A37B0;
extern u8 g_fight_operand_tint_red;
extern u8 g_fight_operand_tint_green;
extern u8 g_fight_operand_tint_blue;
extern s32 g_handler_frame_counter;
extern u8 D_800A37BC;
extern s32 g_fight_stage_list_step_timer;
extern u8 g_fight_skip_first_defeats_left_expiry;
extern u8 D_800A37D2;
extern u8 D_800A37D3;
extern u8 D_800A37E0;
extern u8 D_800A37E1;
extern s16 D_800A37E8;
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern u8 g_fight_win_target;
extern u8 g_fight_choice_menu_first_cursor;
extern s32 g_fight_menu_restart_count;
extern u8 g_fight_reinit_phase_timer;
extern u8 g_fight_choice_menu_cursor;
extern s16 D_800A381C;
extern u8 g_fight_time_up_phase_timer;
/* the draw queue cursor (into g_draw_queue) */
extern void **g_draw_queue_cursor;
extern s16 g_fight_weapon_contact_result;
extern u8 g_fight_tally_winner_index;
extern s16 g_fight_defeat_delay_frames;
extern s32 g_motion_common_buffer;
extern s16 g_main_loop_handler_index;
extern u8 g_fight_saved_first_fighter_selection;
extern u8 *g_fight_command_stream_cursor;
extern s32 g_fight_timer_frames;
extern u8 *D_800A385C;
extern Tbl800A3860Entry *g_motion_fighter_buffers[];
extern u8 g_fight_match_cursor;
extern s16 D_800A3876;
extern u8 *g_fight_active_playback_descriptor;
extern s32 g_fighter_horizontal_separation;
extern u8 g_motion_common_loaded;
/* per player: preset motion frames (motion_InitBuffers, fighter_Update) */
extern MotionFrame *g_motion_frame_buffers[2];
extern u8 D_800A389A;
extern u8 g_fight_match_count;
extern u16 g_fight_paired_input_tally_update_count;
extern s32 g_comb_local_side;
extern u8 g_fight_presentation_index;
extern s16 D_800A38A8;
extern s16 g_fight_paired_move_initiator;
extern u8 g_fight_result_subject_index;
extern u8 g_fight_mark_tally_phase_timer;
extern s16 g_fight_tracked_view_setting;
/* per player: id of the 7-sector frame set loaded into
 * g_motion_frame_buffers[i] (0xFF = none) */
extern u8 g_motion_frame_set_ids[2];
/* per slot: model id in the g_motion_fighter_buffers[i] buffer
 * (motion_LoadFighterData); [1] = 0xFFFF:
 * motion_LoadNdataPackIntoSlot1BufferAndMarkSlot1Id passed 0x80190800
 * (g_motion_fighter_buffers[1]'s buffer) to ndata_LoadPack;
 * motion_InvalidateMarkedNdataPackAndClearSlot1Id calls ndata_InvalidatePack
 * for it */
extern u16 g_motion_fighter_loaded_data_ids[2];
extern u8 D_800A38D4;
extern s16 g_fight_mode_selector;
extern u8 g_fight_step_row_object_id;
extern u8 g_fight_nearest_start_pair_index;
extern u8 g_fight_start_position_pair_index;
extern u8 D_800A38E1;
extern u8 g_fight_step_table_next_row;
extern s32 g_comb_received_unlock_mask_bits;
extern u8 g_fight_paired_move_event_min_frame;
extern u8 g_fight_best_time_rank;
extern u8 g_fight_step_row_tint_red;
extern u8 g_fight_step_row_tint_green;
extern u8 g_fight_step_row_tint_blue;
extern s32 D_800A38F0;
extern s32 g_fight_time_limit_elapsed_frames;
extern u8 g_fight_stage_list_draw_cursor;
extern u16 g_fight_rng_seed;
extern u8 g_snd_initialized;
extern u8 g_fight_stage_start_count;
extern u8 D_800A390C;
extern u8 g_main_present_skip_frames;
extern s8 g_fight_loaded_stage_index;
extern u8 D_800A390F;
extern s16 g_fight_midpoint_effect_delay;
extern u8 g_pad_remote_button_mapping_20;
extern u8 g_pad_remote_button_mapping_40;
extern u8 g_pad_remote_button_mapping_80;
extern u8 g_fight_displaced_first_fighter_selection;

/* Six per-slot counters, 0 = free: snd_QueuePositionSequence claims a slot with
   1, snd_UpdateStageSounds advances and frees them, snd_ResetStageSoundState
   zeroes all six. g_fight_sound_cue_countdown is not an element: it is its own
   countdown byte, and snd_UpdateStageSounds uses only its address, as the loop
   bound. */
extern u8 g_snd_sequence_ticks[6];

extern u8 g_fight_sound_cue_countdown;
extern u8 g_fight_opponent_order_count;
extern u8 g_fight_point_award_phase_timer;
extern u8 g_comb_abort_request;
extern u8 g_fight_choice_confirm_delay;
extern Unk800A3D40Rec g_bitstream_delta_accumulators[];

/* Three word pairs: the uncalled func_8003F5A8 stores pair i,
 * ClearSavedWordPairsAndTwoWriteOnlyWords zeroes all six, func_8003F5CC passes
 * pair i to func_80017F98(a, b, i), an empty function; nothing else reads them.
 */
extern s32 D_800A93B0[3];

extern s32 D_800A93BC[3];
extern Rec44 g_camera_alternate_view_record;
extern Rec44 g_camera_default_view_record;

/* Per-match snapshot: fight_RecordMatchResult stores both players' counters
 * (D_800A3898 / D_800A3899) for match g_fight_match_cursor;
 * fight_PackMatchScores packs the pairs into g_fight_packed_match_scores,
 * fight_TallyMatchResultsAndInitPresentation sums them. */
extern u8 g_fight_match_scores[8][2];

extern Unk800F68E0Rec g_replay_model_draws[0xB4];
/* The two InitPAD receive buffers, 0x24 bytes each (sys_Init); pad_Update
 * copies the first two words of each. */
extern s32 g_pad_buf[2][9];
extern Unk800FF5C8Rec g_camera_view_offset;
extern Unk80101BF0Rec g_replay_events[0x20];
extern Unk80101DF0Record g_view_camera_root_node;
extern Unk80101DF0Record g_view_identity_root_node;
extern Unk80101DF0Record g_view_offset_node;
extern Unk80101DF0Record g_wave_draw_node;
extern Unk800F62E0Rec g_light_setups[8];
extern RotMatrixFunc g_math_rot_matrix_funcs[6];

/* The CD file table at 0x8008EC34: one 8-byte record per disc file, indexed by
 * the file numbers cdrom_GetFileIndex forms. `loc` is sought (cdrom_StartRead
 * copies the record into g_cdrom_state.rec.pair, whose position is sought;
 * cdrom_LoadExec seeks to it directly); `size` is cdrom_StartRead's sector
 * count, cdrom_GetFileSize's result and cdrom_StartAudio's end position. */
extern CdFileEntry g_cd_file_table[159]; /* 0x8008EC34..0x8008F12B */

extern CdState g_cdrom_state;
/* the common file's section pointers */
extern Unk801027B0Rec g_motion_common_sections;
extern Unk80102778Rec g_fight_config;
extern PadState g_pad_state;
extern Unk801027B0Rec g_motion_fighter_sections[]; /* per load slot */
/* [i] points to block i's slots, the words after its header word
 * (reloc_RelocateBlock records it and g_reloc_block_slot_counts[i] holds the
 * slot count). Each slot is a word holding a block-relative offset that
 * reloc_RelocateBlock turns into an address by adding the block's base, unless
 * header bit 15 marks the block relocated; reloc_UnrelocateBlock subtracts the
 * base again (and clears bit 15), and reloc_RebaseBlock moves [i] by a1 / 4
 * words and adds a1 to each slot.  gpu_OffsetBlockTexPrimStreams /
 * gpu_OffsetBlockTexPrimStream store a slot to scratchpad word 0, the u16 *
 * cursor gpu_OffsetTexPrimStream reads, and softbody_CreateForNode reads u16
 * data through one.
 */
extern s32 *g_reloc_block_entry_tables[];
extern Unk80104E88Rec g_fight_ballistic_records[];
extern s32 g_snd_fake_open_vab_copy;

/* fight_BuildOpponentOrder's list: the set bit numbers of
 * g_saved_record.unlock_mask & mask, shuffled, plus up to three appended values
 * (g_fight_opponent_order_count entries; fight_AdvanceOpponentOrder reads entry
 * g_fight_opponent_order_cursor, then advances it). */
extern u8 g_fight_opponent_order[24];

extern Unk80107850Rec g_snd_sequence_positions[6];
extern void camera_UpdateReplayView(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void font_Printf(u8 *, s32, ...);
extern void fighter_StartMoveScript(s32, MoveScript *, s32);
extern void fighter_Init(s32, s32, s32, s32, s32);
extern s32 cdrom_GetFileIndex(s32, s32);
extern void comb_ExchangePad(PadState *);
extern void fight_ScanCommandStreamAndLoadImage(u8 *);
extern void gpu_BuildFogImageLevels(RECT *, s32);
extern void func_8003F1E4(s32);
extern void node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(s32, s32);
extern void node_SetSlotRelativeMatrixQueueRetainedNodesAndLinkOrderingTable(
    s32, s32, s32);
extern void resource_RequestEntryWithLinkedEntriesAndLatchCategory(s32);
extern void resource_SetPairSelectionSlotWithBoundChecks(s32, s32);
extern void resource_RankRequestedEntriesAndReloadOnChange(s32);
extern s32 snd_ClosePairedFileVabsThenLoadFromConsecutiveFiles(s32);
extern void snd_FlushSoundQueue(void);
extern s32 memcard_StepSaveOverwriteAndFormatPrompts(void);
extern void gpu_SetSegmentCellsGateBit(s32, s32, s32, s32);
extern void cdrom_Update(void);

/* Declared by more than one translation unit: data, then functions, by name. */
extern u8 D_8008E908[][5];
extern u8 g_snd_stage_sound_slot_specs[][8];
extern s32 g_snd_stage_sound_slot_positions[2][4];
extern u8 g_fight_step_row_object_ids[][5];
/* 51268's sprite texture records (TexRec runs 0x8009B890..0x8009B9FF; sizes
 * from the data labels) */
extern TexRec D_8009B890[4];
extern TexRec D_8009B8B0[3];
extern TexRec D_8009B8C8[1];
extern TexRec D_8009B8D0[1];
extern TexRec D_8009B8D8[1];
extern TexRec D_8009B8E0[1];
extern TexRec g_eff_jittered_billboard_tex_recs[7];
extern TexRec g_eff_yawed_slot_quad_tex_recs[4];
extern TexRec D_8009B940[1];
extern TexRec D_8009B948[1];
extern TexRec D_8009B950[1];
extern TexRec D_8009B958[1];
extern TexRec D_8009B960[1];
extern TexRec D_8009B968[1];
extern TexRec D_8009B970[1];
extern TexRec D_8009B978[1];
extern TexRec D_8009B980[1];
extern TexRec D_8009B988[1];
extern TexRec D_8009B990[1];
extern TexRec D_8009B998[4];
extern TexRec D_8009B9B8[4];
extern TexRec D_8009B9D8[1];
extern TexRec D_8009B9E0[1];
extern TexRec D_8009B9E8[1];
extern TexRec D_8009B9F0[2];
/* one table of 12 records: [0..5] / [6..9] the two sprite sets, [10] / [11]
 * their g_working_record.grayscale CLUT substitutes (eff_DrawBillboardList) */
extern TexRec g_eff_billboard_tex_recs[12];
extern u8 g_eff_command_handler_bases[];
extern Unk8009BD24Block g_working_record;
extern s32 g_ndata_pack_loaded;
extern s32 g_gpu_jittered_quad_pair_cycle;
extern s32 g_eff_point_light_countdown;
extern u32 g_pad_direction_positive_masks[2];
extern Unk80101DF0Record *g_view_identity_root_ptr;
extern Unk80101DF0Record *g_view_camera_root_ptr;
extern s32 g_memcard_poll_count;
extern void mem_MoveBlockRoundedUpToWords(s32, s32, s32);
extern u8 g_fight_stage_list_entries[];
extern u16 g_comb_send_payload;
extern POLY_FT4 *g_eff_quad_cursor;
extern u32 *D_800A3808;
extern s16 g_cd_fade_step;
extern u8 g_fight_step_row_column;
extern s16 g_cd_fade_len;
extern u8 *D_800A3894;
extern u32 *g_prim_buf_cursor; /* the draw-chunk builders' word cursor */
extern s32 D_800A38D0;
extern u16 D_800A38D6;
extern s32 D_800A38FC;
extern s32 D_800A3908;
extern u8 g_comb_initial_pad_exchange_pending;
extern u8 g_arena;
extern s16 g_eff_jittered_billboard_ages[];
extern s16 g_eff_billboard_pair_ages[];
extern s32 g_eff_command_phase[];
/* A 16-byte vector: thirteen functions store x / y / z; func_80061064 passes
 * it to light_SetPointLight, which copies all 16 bytes to
 * g_light_point_position. */
extern VECTOR g_eff_point_light_position;
extern Unk800F1B18Rec g_motion_decoders[];
extern u8 g_shared_buffer[];
extern s16 D_800F6650;
extern s16 g_gpu_grid_and_fog_control;
extern u8 D_801027A0[5][2];
extern u8 D_801027D8[5][2];
/* the draw queue: record pointers (0x80102C00..0x801035FF) */
extern void *g_draw_queue[640];
extern s32 g_eff_command_handlers[];
extern Func80017A44Output g_softbody_slots[8];
extern s32 g_slot_tim_group_requests[];
extern Unk80045878Obj *g_slot_objects[];

extern s32 bits_DepositMask3F83F8(s32);
extern s32 bits_ExtractMask3F83F8(s32);
extern s16 *camera_CalcAngles(void);
extern void camera_CopyViewMatrixWithHalvedMiddleRow(void);
extern u32 cdrom_GetFileSize(s32);
extern void cdrom_Init(void);
extern void cdrom_ReadyCallback(u8, u8 *);
extern void cdrom_SetMix(s32, s32, s32, s32);
extern s32 cdrom_StartAudio(s32, s32);
extern s32 cdrom_StartRead(s32, s32);
extern s32 cdrom_StartReadAt(s32, s32, s32, s32);
extern MATRIX *camera_GetViewMatrixWithHalvedMiddleRow(void);
extern void eff_Init(void);
extern void func_800164F8(void);
extern u32 gpu_GetSavedGrayscaleFlag(void);
extern u32 fighter_GetSavedCpuBehaviorAdjustFlag(void);
extern u32 snd_GetSavedCdAudioSuppressFlag(void);
extern void snd_InitAndLoadCommonVab(void);
extern void eff_ClearInitFlag(void);
extern void gpu_DrawDisplayModeOnLocalOtAndSync(void);
extern s32 softbody_Create(Func80017A44Input *);
extern void softbody_UpdateList(Unk80017FA0Rec *, s32);
extern void pad_SetDefaultSavedButtonMappings(void);
extern s32 pad_PackSavedButtonMappingLowNibbles(void);
extern void pad_UnpackButtonMappingNibblesToCells(s32);
/* No prototype: defined void (s32), but menu_RunPauseMenu calls it with no
 * argument. */
extern void pad_Update();
extern void motion_InitStream(s32, u32 *);
extern void motion_DecodeFrame(s32, s32, MotionFrame *, u16 *);
extern void camera_UpdatePadOffset(s32 *);
extern void func_8001B6F4(void);
extern void camera_UpdateReplayViewWithSineYawOffset(Rec1C *, Rec1C *, s32);
extern void camera_UpdateReplayViewWithConstantYawOffset(Rec1C *, Rec1C *, s32);
extern void fight_InitSettings(void);
extern void fight_SplitFrameCounterIntoMinutesSecondsHundredths(
    Unk8001CD68Rec *);
extern void fight_ResetGraphAndReloadResourcesOnCachedKeyChange(void);
extern void snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode(void);
extern s32 snd_ShouldUpdateStageSoundSlots(void);
extern void fight_WaitForCounterThenClearAndEnableDisplay(void);
extern void camera_ApplyReplayView(s32);
extern void motion_ApplyRecordedPoseAdjustments(
    Unk80101EC8Record *, Rec1C *, MotionFrame *, MotionFrame *);
extern void fighter_SetTurnDelta(Unk80101EC8Record *, s32);
extern void fighter_StoreBoneLocalVectorAndArmEventTrigger(
    Unk80101EC8Record *, s32, s16 *);
extern s32 fight_UpdateSeparationAndPairedMoves(void);
extern void fighter_UpdateBoneWorldData(
    Unk80101EC8Record *, Unk80107850Rec *, Unk80107850Rec *, Unk80107850Rec *);
extern void motion_InvalidateMarkedNdataPackAndResetLoadCaches(void);
extern void motion_InvalidateMarkedNdataPackAndClearSlot1Id(void);
extern void motion_InitBuffers(void);
extern void fighter_AssignMotionSlotAndScanDataIdNibbles(s32);
extern void *motion_ResolveMoveData(Unk80101EC8Record *, s32, s16 *);
extern void motion_ResetVariantIndex(s32);
extern MoveScript *motion_GetCurrentVariantScript(s32);
extern MoveScript *motion_GetDefaultVariantScript(s32);
extern MoveScript *motion_GetCommonScriptAtFighterBufferOffset(s32);
extern MoveScript *motion_GetCommonScriptAtFighterBufferIndexedOffset(s32, s32);
extern void fight_GetStartPosition(s32, s32 *, s32);
extern s32 fight_FindNearestStartPair(Vec3i32 *);
extern void fighter_ResetBoneGroupHitState(Unk80101EC8Record *);
extern void fighter_ConfigureCpuControl(void);
extern s32 fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow(
    Unk80101EC8Record *, s32 *);
extern void fighter_Update(s32, PadState *);
extern void fight_UpdatePairedInputTotalsEventsAndMidpointEffects(void);
extern void motion_SetFixedChannelBlock(MotionFrame *);
extern s32 fighter_TestMoveFrameWindowOrObjectVariantKind(Unk80101EC8Record *);
/* fighter_TryDropQueuedHeadAtBone (17AFC.c, void (Unk80101EC8Record *)) has no
 * declaration: 175A4.c calls it implicitly, and any truthful declaration
 * changes fight_UpdatePairedInputTotalsEventsAndMidpointEffects's register-save
 * order. */
extern void fighter_ResolveBodyOverlap(void);
extern void fight_ClearPositionRecordUsage(void);
extern s32 fighter_ProcessWeaponContacts(void);
extern void fighter_ProcessWeaponBodyHits(s32);
extern s32 fighter_CalcSeparationErrorAndOpposedOffsets(void);
extern void fight_UpdateFighterInteractionsAndPointCaches(void);
extern void math_ScaleVectorInDirectionFrame(s32 *, s16 *, s32 *, s32, s32);
extern void math_InvertMatrix3x3(void *, void *);
extern void math_ComposeAxisRotationsAndRewriteAngles(s16 *, s32, s32, s32);
extern s32 fighter_IsBoneTriangleCrossYPositive(Unk80101EC8Record *);
extern void object_ResetPool(void);
extern void object_InitKindQueue(Unk80101EC8Record *);
extern void object_FreeAttachedObjects(void);
extern s32 object_ThrowQueuedKind(Unk80101EC8Record *);
extern void object_DropQueuedHeadAt(Unk80101EC8Record *, Vec3i32 *);
extern void object_SpawnScatteredAt(Unk80101EC8Record *, s32, Vec3i32 *);
extern s32 object_PickUpNearby(Unk80101EC8Record *);
extern void object_ClearFixedKindRange(void);
extern void object_UpdatePool(void);
extern void object_ResolveFighterHits(void);
extern void fight_ClearBallisticRecords(void);
extern Unk80104E88Rec *fight_CreateBallisticRecordAndDispatchEvent(
    Unk80101EC8Record *, s32);
extern void fight_UpdateBallisticRecords(void);
extern void fight_TriggerOpposingFighterEventFromNearBallisticRecords(void);
extern void fighter_ReadFrameMetadata(Unk80101EC8Record *);
extern void snd_QueuePositionalSound(s32, s32 *);
extern void fight_RecordReplayEventAndDispatchSoundOrEffect(
    s32, s32, s32 *, s16 *);
extern void fighter_ProcessFrameEvents(Unk80101EC8Record *);
extern void snd_ResetStageSoundState(void);
extern void snd_UpdateStageSounds(void);
extern void fight_BuildOpponentOrder(void);
extern void fight_AdvanceOpponentOrder(void);
extern s32 fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags(void);
extern void fight_SetSelectorAndMainLoopHandlerFromFlagsAndBestTimeRank(void);
extern void fight_RecordMatchResult(void);
extern void fight_AdvanceMatch(void);
extern void ApplyPendingConfigAndInitializeMode(void);
extern void CopyWorkingFlagsAndButtonMappingsToSavedRecord(void);
extern void ApplyWorkingRecordToPendingConfigAndInitializeMode(void);
extern void CopySavedFlagsButtonMappingsAndTimesToWorkingRecord(void);
extern void cdrom_StartMixFade(s16, s32, s32, s32, s32);
extern s32 cdrom_StartAudioEntry(s32);
extern void cdrom_SetAudioRepeat(s16);
extern void cdrom_WaitAudioStarted(void);
extern void cdrom_PauseAndWait(void);
extern void sys_ExecMovieOverlay(s32, s32, s32, s32, s32);
extern s32 memcard_PollSwEvents(void);
extern s32 memcard_CountFreeBlocks(void);
extern s32 memcard_HasFile(u8 *);
extern s32 func_80037D14(s32, s32);
extern s32 memcard_StartLoadOnceStepAndDrawSheetPair(void);
extern s32 menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult(void);
extern s32 memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress(void);
extern s32 *GetSharedBuffer(void);
extern void replay_Reset(void);
extern void replay_BeginFrame(void);
extern void replay_RecordModelDraw(s32, s32, s32 *, s16 *);
/* replay_RecordEvent (28708.c, u8 / u8 / s32 * / u16 *) has no prototype here:
 * 17AFC.c declares it unprototyped, as its prototype changes 17AFC.c's bytes.
 */
extern void replay_RecordFighter(Unk80101EC8Record *);
extern void replay_AdvanceFrame(void);
extern void comb_Init(void);
extern void comb_ReadCtsSetRts(void);
extern void comb_EnableEvents(void);
extern void comb_ResetClose(void);
extern void comb_RequestClose(void);
extern void comb_ExchangeEmptyPad(void);
extern void comb_BeginWait(void);
extern void comb_EndWait(void);
extern s32 comb_Connect(void);
extern void fight_FillMissingConfigInitFighterAndLoadObject(s32);
extern void fight_RequestBothFightersEntriesAndReloadOnChange(void);
extern void fight_InitCommandStream(s32);
extern void
fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields(
    void);
extern s32 fight_ConfigureFighter1FromOperandsAndLoad(u8 *);
extern void func_8003B534(s32);
extern void func_8003B56C(s32);
extern void fight_RunCommandStream(void);
extern void font_LoadImage(void);
extern void font_BeginFrame(void);
extern void font_AddTpage(void);
extern void bitstream_InitDeltaAccumulator(s32, s32);
extern s16 *bitstream_AccumulateDeltas(s32);
extern void gpu_InitFogDrawMoves(void);
extern void gpu_LatchFogDepthOnTransitionAndQueueMoves(s32, s32);
extern void gpu_SetLastDepthIndexOtEntryLinkToPacket(u32);
extern void gpu_BuildFogRowLevels(void);
extern void gpu_BuildFogBlockLevels(void);
extern void gpu_MoveObjectClutsToFogBlock(s32);
extern void gpu_RestoreObjectClutOffsets(void);
extern s32 gpu_GetFogClutObject(void);
extern u32 gpu_GetLastFogOtIndex(void);
extern s32 gpu_DepthToOtIndex(s32, s32);
extern void gpu_SetViewOriginSegmentCellsGateBit(s32, s32);
extern void
    gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards(s32);
extern void gpu_UnpackGridRecordsNodesAndCellRuns(u16 *, s32);
extern void func_8003F218(s32);
extern s32 func_8003F268(void);
extern void gpu_SetCellForceBit(s16 *);
extern void gpu_ReplaceCellAttributesWithMaskedValue(s32, s32, s32);
extern void softbody_UpdateObject(Unk80045878Obj *);
extern void softbody_FreeAllAndClearWriteOnlyWords(void);
extern void softbody_SetupObject(Unk80045878Obj *, s32);
extern void softbody_RequestRebuild(Unk80045878Obj *);
extern void softbody_RequestSnap(s32);
extern void softbody_SetObjectGround(Unk80045878Obj *, s32);
extern void softbody_FreeObject(Unk80045878Obj *);
extern void softbody_FreeObjectById(s32);
extern void softbody_RebaseObject(Unk80045878Obj *, s32);
extern void gpu_QueueSlotDrawMovesForSelectedSet(s32, s32);
extern void node_SetWordOnAllListedNodes(Unk80045878Node *, s32);
extern void slot_ClearObjectPointersAndTextureRequests(void);
extern Unk80045878Obj *slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
    s32, s32, s32);
extern void node_RefreshObjectMatrixPointerTables(Unk80045878Obj *);
extern void node_BuildProjectedNodeList(Unk80045878Obj *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void slot_BlendTwoPosesIntoNodeMatrices(s32, u8 *, u8 *, s32, MATRIX *);
extern void PatchColorImmediatesAtListedCodeSites(s32);
extern void slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers(s32, s32);
extern void node_RebaseParents(s32, Unk80045878Node *, s32);
extern Unk80045878Obj *slot_GetObject(s32);
extern void node_UpdateWorldMatrix(Unk80101DF0Record *);
extern void node_SetMatrixFromNegatedRot(Unk80101DF0Record *);
extern void gpu_LoadRequestedAvailableTimGroupsForSlot(s32, s32, s32, s32);
extern void gpu_BackUpFixedSlotVramRowsAndRecordAssetIndex(Unk80045878Obj *);
extern void gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(s32, s32, s32);
extern void light_SetPointLight(VECTOR *, s32);
extern void light_ResetPointLight(void);
extern void light_SetPointLightTarget(s32, s32);
extern void light_ApplyPointLightTargets(void);
extern void light_DisablePointLight(void);
extern void ApplyLightPresetRowBackgroundClearAndFarColor(s32);
extern void math_InitRotMatrixFuncs(void);
extern void gpu_OffsetBlockTexPrimStreams(s16, s16, s16, s16, s32);
extern void gpu_OffsetBlockTexPrimStreamsByPages(s16, s16, s16, s16, s16);
extern s16 gpu_OffsetTPage(s16, s16, s16);
extern s16 gpu_OffsetClut(s16, s16, s32);
extern void reloc_RelocateBlock(s32 *, s16);
extern void reloc_UnrelocateBlock(s16);
/* No prototype: defined void (s32, s32), but resource_RebaseLoadedBlockSections
 * calls it as reloc_RebaseBlock(8). */
extern void reloc_RebaseBlock();
extern s32 reloc_KeepSelectedEntries(s32 *, s16 *);
extern s32 reloc_CopySelectedEntries(s32, s32 *, s16 *);
extern void reloc_ClearCounts(void);
extern void func_800444BC(void);
extern void func_800444E0(void);
extern void gpu_PrepareViewAndFlushDrawQueue(u32 *);
extern s32 node_BindFrameDataAndReserveEntryBuffer(s16 *, s16, s32);
extern void node_InitFrameEntriesFromWorldTranslations(void);
extern void node_AdvanceFrameAnimatedEntriesAndQueueDraws(void);
extern void node_StartFrameAnimatedEntryAndInitPairedNode(s32, s32);
extern void resource_RebaseFrameAnimatedEntryBlockAndPointers(s32);
extern void math_LerpShortVectorsNegateYZ(s16 *, s16 *, s32, s32);
extern void func_80044F30(s32, s32);
extern void func_80044F80(s32, s32);
extern s32 ndata_ReadChecked(s32, s32);
extern s32 ndata_GetArenaRoomAfter(s32);
extern s32 ndata_ReadFromPack(s32, s32);
extern s32 ndata_IsPackLoaded(void);
extern void ndata_StartReadInf(void);
extern void arena_Reset(void);
extern void arena_UpdatePeak(s32);
extern void arena_Free(s32);
extern void arena_Resize(s32, s32);
extern s32 *arena_OpenBlock(s32);
extern void arena_SetBlockEnd(s32, s32);
extern void arena_SetMoveCallback(s32, s32);
extern s32 *arena_FindBlock(s32);
extern s32 arena_GetBlockAddr(s32);
extern s32 arena_GetFreeSize(void);
extern s32 arena_GetSize(void);
extern void *arena_GetBase(void);
extern void arena_Copy(s32, s32, s32);
extern Unk80045878Obj *slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock(
    s32, s32, s32);
extern void slot_ResizeAssetBlockAndUpdatePeak(s32, s32);
extern void slot_StopAllVoicesCloseVabAndFreeObjectAndAssetBlocks(s32);
extern void resource_LoadSelectedEntriesTexturesAndSound(s32, s32, s16 *, s32);
extern void resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs(void);
extern void resource_SelectRetainedSubblockOrApplyHeaderSections(s32);
extern s32 fight_GetPositionRecordList(void);
extern void resource_FreeNodeAssetBlock(void);
extern s32 *motion_LoadPlaybackBlock(s32);
extern void motion_FreePlaybackBlock(void);
extern void gpu_ResetDrawQueueAndPrepareViewAndLighting(
    Vec3i32 *, SVECTOR *, s32);
extern void
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(
        s32);
extern void gpu_SetRaisedOtCutoffRequested(s32);
extern s32 gpu_GetRaisedOtCutoffRequested(void);
extern void gpu_RunGridDrawPassAndQueueFogMoves(s32);
extern void math_SetLightPlaneProjection(void);
extern void gpu_LoadTimGroup(u32 *, s32);
extern void gpu_LoadTimGroupOffset(u32 *, s32, s16, s16);
extern void gpu_LoadTimGroupSplitOffset(u32 *, s32, s16, s16, s16, s16);
/* gpu_LoadTim and gpu_LoadTimClut have no prototype: 368E4.c calls them
 * above their old-style (K&R) definitions with int arguments, and a prototype's
 * s16 / u16 parameters change its bytes. */
extern s32 gpu_RefreshGrayscaleMode(void);
extern void math_GrayscaleRgb555(u16 *, s32, u16 *);
extern void gpu_TintVramRowInPlace(s32, s32, s32, s32, s32, s32);
extern s32 node_LoadSlotMappedAssetAndInitRetainedDrawNodes(s32);
extern void ResetEntrySelectionState(void);
extern void node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
    s32, s32, s32 *, s16 *);
extern void node_QueueTableEntryNodePairUnderSlotNode(s32, s32, s32);
extern s32 snd_CopySelectedPackSectionsAndLoadVabPacks(s32, s32);
extern void resource_InvalidateEntrySelectionCache(void);
extern void calc_LightMatrix(Unk800F62E0Rec *);
extern s16 *gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision(
    s32, s16 *, s16 *, s32);
extern void gte_LerpMatrixRowsAndRebuildOrthonormalBasis(
    MATRIX *, MATRIX *, s32, MATRIX *);
/* No prototype: defined with no arguments, but
 * gpu_UnpackGridRecordsNodesAndCellRuns / softbody_CreateForNode pass it one.
 */
extern void sys_StoreZeroPastScratchpadOnFailedCheck();
extern s32 collision_TestSegmentExtendingShortStartWithWork(
    s32 *, s32 *, s32 *, s16 *, s32);
extern s32 collision_TestSegment(s32 *, s32 *, s32 *, s16 *);
extern s32 collision_TestSegmentWithWork(s32 *, s32 *, s32 *, s16 *, s32);
extern void collision_SetGrid(s32);
extern void collision_RebaseGrid(s32);
extern void motion_StartPlaybackAtIndexedBasePlusOffset(
    s32, s32, s32, s32, s32, s32, s32, s32);
extern void motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock(void);
extern s32 motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush(void);
extern void motion_RebasePlayback(s32);
extern s16 *motion_GetPlaybackViewAngles(void);
extern s32 *node_GetRootNodeMatrixTranslation(void);
extern void fighter_ClearCpuTally(s32);
extern void fighter_HalveCpuTally(s32);
extern void fighter_InitCpuControl(s32, u16 *, u16 *);
extern void fighter_BuildCpuPad(s32, PadState *);
extern void snd_StopAllVoices(void);
extern void snd_ResetVoicesAndQueue(void);
extern void snd_StopAllVoicesAndCloseSlotVab(s32);
extern void snd_StopAllVoicesAndCloseSelectedEntryVabs(void);
extern void snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb(void);
extern void snd_StopAllVoicesAndClosePairedFileVabs(void);
extern void snd_StopAllVoicesAndCloseSingleFileVab(void);
extern s32 snd_CloseThenReadSingleFileAndLoadVab(s32);
extern s32 snd_SetReverb(s32);
extern s32 snd_ReloadListedVabsFromFileAndInstallBillboards(s32, s32, s32, s32);
extern void snd_RebaseListedVabs(s32);
extern void snd_CloseListedVabs(void);
extern void snd_StopVoicesAndReverb(void);
extern void snd_StopVoicesSetMaxMasterVolumeStereoAndClearAutoKeyOffMode(void);
extern s32 snd_QueueSound(s32, s32, s32); /* FAKE: int, see definition */
extern s32 gpu_DrawModeSizedPanelWithSelectedCellOpaque(s32, s32, s32, s32);
extern s32 gpu_DrawFixedSpritePairWithSelectedSheets(s32, s32);
extern s32 gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle(s32, s32);
extern s32 gpu_DrawTimeFieldsAndDecimalCounter(Unk8001CD68Rec *, s32, s32, s32);
extern s32 gpu_DrawTwoDigitCounterWithOptionalSecondSide(s32, s32, s32, s32);
extern s32 gpu_DrawDecrementedSingleSideCounter(s32, s32, s32);
extern s32 gpu_DrawPackedComparisonRowsAndTotals(u32, s32, s32);
extern s32 gpu_DrawPairedMarkRowsAndTimeFields(Unk8001CD68Rec *, s32, s32, s32);
extern s32 gpu_DrawSelectedStretchedSpritePair(s32, s32, s32);
extern void gpu_ReadCdFileUploadTwoRegionsAndResetSplitWipe(s32, u8 *);
extern s32 gpu_DrawImageAndAdvanceSplitWipe(s32, s32);
extern s32 gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite(s32, s32, s32);
extern void gpu_ReadCdFileRelocateTwoImagesAndUploadFourRegions(s32, s32 *);
extern s32 gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells(s32, s32);
extern void gpu_ResetGrowingBarProgress(void);
extern s32 gpu_DrawIndexedCellsDigitPairsAndGrowingBars(s32, s32, s32);
extern s32 eff_LoadImagesAndSetup(s32, s32);
extern void eff_SelectQuadBuffer(s32);
extern void func_80061064(s16 *, s32 *);
extern void eff_StartRotatedQuadClusterWithNearWhiteLight(s32 *, s16 *);
extern void eff_StartRecycledCommandWithRoseLight(s32 *);
extern s32 eff_StartCosineScaledQuadWithPaleBlueLight(s32 *);
extern s32 eff_StartSlowRateCosineScaledQuadWithPaleBlueLight(s32 *);
extern s32 eff_StartMediumRateCosineScaledQuadWithPaleBlueLight(s32 *);
extern s32 eff_StartFastRateCosineScaledQuadWithPaleBlueLight(s32 *);
extern void eff_StartRecycledCommandWithPaleRedLight(s32 *);
extern void eff_StartArgSelectedCommandWithCyanLight(s32 *, s32);
extern void eff_StartArgSelectedCommandWithGreenLight(s32 *, s32);
extern void eff_StartRecycledCommandWithOrangeLight(s32 *);
extern void eff_StartRecycledVectorCommandWithRedLight(s32 *, s16 *);
extern void eff_StartBillboardPairCommand(s32 *);
extern void eff_StartJitteredBillboardsCommand(s32 *);
extern void eff_StartYawedQuadSlotCommand(s32 *, s16, s32, s32);
extern void eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight(
    s32 *, s16 *);
extern void eff_StartRecycledYawedParticles(s32 *, s16);
extern void eff_StartRecycledCommandWithMagentaLight(s32 *);
extern void eff_SetRotMatrix(s16 *, s32 *, MATRIX *);
extern void eff_DrawBillboardList(s16 *, s32 *);
extern void menu_SetEnabledRows(s32);
extern s32 menu_LoadResourcesInitStateSelectFirstEnabledRow(
    s32, Unk8009BD24Block *);
extern void reloc_RelocateOffsetList(s32 *, s32);
extern s32 gpu_DrawWhiteTileAndSheetsTestPadMask(s32, s32);
extern s32 pad_StepOffsetsGetDirectionEvents(u32 *, s32, s16 *, s16 *);
extern s32 menu_UpdateRowMenu(s32, s32);
extern void gpu_AddThreeGreyTilesStackedUp(s32 *, u16 *, s32);
extern void gpu_SetTileRgbFromBlockFlag(void *);
extern void gpu_SetPolyF4RgbFromBlockFlag(void *);
extern s32 menu_UpdateOptionMenuChoices(s32, u32);
extern void gpu_DrawSheetPairCenteredTileWithOptionalSelection(s32, s32);
extern s32 func_8006C1FC(s32, s32);
extern void menu_ResetRowBitCursors(void);
extern s32 menu_UpdateRowBitMenu(s32, s32);
extern s32 menu_UpdateStoredChoice(s32, s32);
extern s32 menu_ResetConfirmCursor(void);
extern void gpu_DrawIndexedCellsAndDecimalDigitPairs(
    s32 *, s32 *, Unk8006D808Rec *, s32, s32);
extern s32 menu_UpdateConfirmMenu(s32, s32);
extern s32 gpu_BeginFullscreenImageDisplay(void);
extern s32 gpu_EndFullscreenImageDisplay(void);
extern void reloc_RelocateSelfOffsets(s32 *);
extern s32 gpu_GetSheetTPage(Unk8009B0E0Record *, s32);
extern s32 gpu_InitPrimBufferPair(s32, Unk8006E49CRec *);
extern s32 InitDisplayStateLoadPackAndSetEntryFlags(
    s32, s32, Unk8009BD24Block *, u32);
extern void gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
    s32, Unk8006E950Head *);
extern s32 resource_RelocateNineOffsetListsAndLoadVabPack(Unk8006EA28Rec *);
extern s32 StepSlideBuildFrameRecordAndDispatchStateHandler(s32, s32);
extern void gpu_AddPanelGrid(Unk8006EACCRec *);
extern s32 gpu_AddSpriteCells(Unk8007352CRec *);
extern s32 gpu_AddScaledSpriteQuads(Unk8007352CRec *, s32);
/* No prototype: defined old-style (63D2C.c) with an s16 parameter, which a
 * prototype cannot match. */
extern s32 gpu_AddRotatedSpriteQuads();
extern void gpu_DrawThreeSheetsWithOptionalTileAndOpaqueQuads(
    Unk8006EACCRec *, s32);
extern void menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles(
    Unk8006EACCRec *);
extern s32 InitRowMenuBlackBgResetFlow(s32);
extern s32 menu_UpdateRowMenuStoreResult(s32, s32);
extern s32 ResetFlowAndFetchRowTablePair(void);
extern void StoreCompactedBits(s32);
extern s32 InitWithCachedValueAndStoredBitsBlackBg(s32);
extern s32 StepThenCloseVab1OnNonzero(s32, s32);
extern s32 menu_ResetFlowThenRunUpdate(s32, s32);
extern s32 func_80077A60(s32, s32);
extern s32 func_80077AC0(s32, s32);
extern void menu_ResetFlowBackgroundAndConfirmCursor(void);
extern s32 menu_InitSelWithStoredBits(s32);
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern void SetFlowStateOne(void);
extern s32 menu_RunOptionModeFlowStateWithRowBitEdit(s32, s32);
extern Unk8009BD24Block *GetWorkingRecord(void);
extern s32 InitTimedSpriteSequence(s32);
extern s32 StepTimedSpriteSequenceWithSkip(s32);
extern s32 InitScrollingSpriteDisplay(s32);
extern s32 StepScrollingSpriteDisplay(void);
extern void eff_StopCommands(void);
extern void cdrom_WaitIdle(void);
extern void *gpu_GetGridNodeRecordTable(void);
extern void *node_GetSlotMatrixPointerArray(s32);
extern void func_80046B44(void);
extern void resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
    s32, s32);
extern void gpu_SetDrawEnvBg(s32, s32, s32, s32);
extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);
extern s32 gte_SumSquares3(s32, s32, s32);
extern s32 math_FovToScreenDist(s32);
extern s32 math_Grayscale3(s32, s32, s32);
extern void math_MatrixToAnglesYXZ(MATRIX *, SVECTOR *);
extern void math_RotMatrixYXZ(u16 *, MATRIX *);
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern void math_TransposeMatrixInPlace(MATRIX *);
extern void memcard_AckHwEvents(void);
extern void memcard_AckSwEvents(void);
extern s32 memcard_CountFiles(s32, s32);
extern void memcard_Init(void);
extern s32 memcard_PollSwEventsTimeout(void);
extern void memcard_Quit(void);
extern s32 memcard_ReadFile(s32, s32, s32, void *, s32);
extern s32 memcard_WaitHwEvent(void);
extern s32 memcard_WriteFile(s32, s32, s32, void *, s32, s32, s32);
extern s32 softbody_GetBufferSize(s32, s32);
extern void softbody_Free(s32);
extern void softbody_FreeAll(void);
extern void pad_ResetState(void);
extern void pad_ResetStateMarkValid(void);
extern void slot_ReleaseObjectAndResources(s32);
extern void slot_RequestTimGroupsAndMarkReloadOnChange(s32, s32);
extern void rcnt_StartCnt1(void);
extern s32 rng_Next(void);
extern void rng_SetSeed(s32);
extern void scratchpad_Restore(void);
extern void scratchpad_Save(void);
extern void ndata_InvalidatePack(void);
extern void ndata_LoadPack(s32, s32);
extern void snd_CloseVab1(void);
extern void snd_Init(void);
extern s32 snd_LoadCommonVab(s32);
extern void snd_Quit(void);
extern void snd_SerialMixOn(void);
extern void resource_FreeSlotsAndBlocksAndInvalidateEntryCache(void);
extern void snd_VabFakeOpen8And4(s32);
extern void snd_VabFakeOpen9(s32);
extern void func_8003F5CC(void);
extern void ClearSavedWordPairsAndTwoWriteOnlyWords(void);
extern void resource_InvokeLoadedSelectorInitCallback(void);
extern s32 resource_GetLoadedSelector(void);
extern void *fight_GetStartPositionPairTable(void);
extern void gpu_ResetAndStampViewCenteredCellAttributes(void);
extern void sys_Init(void);
extern void sys_Panic(void);
extern void func_80017F90(void);
extern void func_80017F98(s32, s32, s32);

/* Sony library functions the game declares differently from the library. */
extern void ResetGraph(s32);

#endif /* BB2_H */
