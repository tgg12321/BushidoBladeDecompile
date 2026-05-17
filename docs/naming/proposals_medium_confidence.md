# Naming Proposals -- Medium confidence

**Medium confidence**: weak Kengo match (size-diff > 1 but same name), single named caller (`sole_caller_path` proposal: `<caller>_helper_<addr>`), or call-graph subsystem cluster. These need callsite/body inspection before applying.

Total Medium: **199**

## Primary evidence: `manual_review` (96)

Hand-review rows for functions previously tagged `confidence=none` by
the automated analyzer. Each row links to a prose evidence file that
walks through the C body, the caller-side pattern, the (almost always
weak) Kengo cross-reference, and the reasoning. The 2026-05-17
caller-side audit pass refined several slugs and promoted one row
(`player_rob_Init`) up to the high-confidence table after three
independent medium-rank evidence rows agreed. Batch 2 (2026-05-17)
added 7 more medium entries plus 2 high (now in the high-confidence
table).

### Batch 1 (2026-05-12 + 2026-05-17 reinforcement)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80032854` | `func_80032854` | `motion_dispatch_event_handler_80032854` | manual_review=30-case switch on arg1, dispatches to func_8006xxxx family; 19 callers all in motion subsystem; arg0 is 2-player slot index | [md](evidence/func_80032854.md) |
| `0x80037110` | `func_80037110` | `special_cam_issue_command_80037110` | manual_review=callers pass small command codes {0, 1, 8, 9, 10, 11} from state-machine cases; arg0 is a command code, not a generic table index (slug refined 2026-05-17) | [md](evidence/func_80037110.md) |
| `0x80038734` | `func_80038734` | `game_state_advance_80038734` | manual_review=callers do `switch (v0 - 4)` on return; state-machine advance not input tick; body polls pad + advances state + returns state code (slug refined 2026-05-17) | [md](evidence/func_80038734.md) |
| `0x80044010` | `func_80044010` | `prim_buffer_open_slot_80044010` | manual_review=marks header `*a0 \|= 0x8000`, stores ptr+count in D_80103608/D_80103658 slot a1; caller-side audit confirms texA/texB texture-page pointers -- consider rename to `rmd_AddTbp_*` to match the Kengo partial-agreement hint | [md](evidence/func_80044010.md) |
| `0x80044100` | `func_80044100` | `prim_buffer_relocate_slot_80044100` | manual_review=advances D_80103608[a0] by a1 bytes and patches each entry by +a1 (offset-list relocation); paired with func_80044010; consider rename to `rmd_AddTbpOfst_*` | [md](evidence/func_80044100.md) |
| `0x80077820` | `func_80077820` | `disp_load_config_from_buf_80077820` | manual_review=both callers pass fixed buffer addr 0x80118800; disp_SetFramebufferMode(1,0,0,0) is unconditional (not mode-parametrised); func_80068F70(src, &D_8009BD24) is the actual copy (slug refined 2026-05-17 -- original "init_for_mode" was wrong) | [md](evidence/func_80077820.md) |
| `0x80080258` | `func_80080258` | `tslTm2_command_with_retry_80080258` | manual_review=3-retry wrapper around tslTm2LoadImage (general op dispatcher, not TIM-only -- modes {2, 9, 0xE} seen at call sites; mode 9 passes arg1=0); g_cd_callback_a save/restore (slug refined 2026-05-17) | [md](evidence/func_80080258.md) |

### Batch 2 (2026-05-17)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80022408` | `func_80022408` | `stage_find_nearest_corner_80022408` | manual_review=reads stage_GetDataPtr() + D_800A36A4*48 byte offset, iterates 4 records (6 s16 each = segment endpoints), returns idx of nearest by 2D distance squared | [md](evidence/func_80022408.md) |
| `0x80039320` | `func_80039320` | `timer_event_table_tick_80039320` | manual_review=two sequential 16-byte-stride table scans: loop 1 releases slots matching D_800A36F8 in D_80101BF0; loop 2 ticks counters in D_800F68E0 and expires past threshold | [md](evidence/func_80039320.md) |
| `0x80044098` | `func_80044098` | `tpage_slot_release_80044098` | manual_review=inverse of prim_buffer_open_slot_80044010 (clears in-use bit, un-relocates payload); 3rd function of the slot-pool lifecycle (open/relocate/release) | [md](evidence/func_80044098.md) |
| `0x80054884` | `func_80054884` | `info_draw_at_yslot_80054884` | manual_review=8-arg wrapper around func_80054604; resolves Y coord as InfoPosYTbl1[a0] + a1 - 0x131 then forwards remaining 7 args | [md](evidence/func_80054884.md) |
| `0x80077940` | `func_80077940` | `disp_pack_gpu_command_word_80077940` | manual_review=packs 4 disjoint bit-fields (10+10+1+1 bits) from sparse 26-bit arg0 into compact 22-bit D_800A35E8; matches PSX GPU coord-pack command shape | [md](evidence/func_80077940.md) |
| `0x80077984` | `func_80077984` | `disp_apply_config_with_state_80077984` | manual_review=sibling of disp_load_config_from_buf_80077820 (same disp_SetFramebufferMode tail); uses func_8006E534(a0, D_800A35E0, g_disp_config, D_800A35E8) | [md](evidence/func_80077984.md) |
| `0x80080390` | `func_80080390` | `tslTm2_command_with_retry_no_arg3_80080390` | manual_review=2-arg twin of tslTm2_command_with_retry_80080258; hardcodes a2=0 and uses different terminal tslTm2LoadImage mode | [md](evidence/func_80080390.md) |

### Batch 3 (2026-05-17)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80034200` | `func_80034200` | `pack_player_input_bitstream_80034200` | manual_review=packs N x 2 byte values from D_800F65F8 into 2-bit slots of a single u32 stored at D_800A3784; sets g_disp_enable = DISP_LOADING during the compute | [md](evidence/func_80034200.md) |
| `0x80041430` | `func_80041430` | `player_rob_relocate_80041430` | manual_review=advances g_player_ptrs[a0] by a1 bytes and patches embedded sub-pointers at +0x2C, +0x8B4, +0x10D4, +0x112C (0x68 stride chain), +0x1A34 (20 entries); analog of prim_buffer_relocate_slot_80044100 for player records | [md](evidence/func_80041430.md) |
| `0x80044504` | `func_80044504` | `frame_context_init_scratchpad_80044504` | manual_review=writes 5 scratchpad words at 0x1F80000C..0x1F80001C as inputs for a per-frame compute kernel; calls camera_InitMatrix + game_SetPause(1); chains func_8004A4E0 (likely the hot loop) | [md](evidence/func_80044504.md) |
| `0x80044670` | `func_80044670` | `seq_state_set_with_stage_offset_80044670` | manual_review=writes a 14-byte state record (D_800A9CF8..D_800A9D04) including stage_GetId result; returns a2 + per_stage_offset * 0x68 | [md](evidence/func_80044670.md) |
| `0x80045878` | `func_80045878` | `audio_seq_load_or_get_80045878` | manual_review=load-if-absent / get-if-present for audio sequence records; chains saSeMain_80045600 + saTan5TakeGetPos_* + func_80045AA4 callback; manages paired slots a0 and a0+3 | [md](evidence/func_80045878.md) |
| `0x80069898` | `func_80069898` | `draw_shadow_3tile_stack_80069898` | manual_review=emits 3 consecutive PSX tile primitives stacked vertically with progressively dimmer RGB and increasing alpha modes; canonical 3-tile soft-shadow pattern | [md](evidence/func_80069898.md) |
| `0x80073060` | `func_80073060` | `hud_layout_emit_tiles_80073060` | manual_review=emits a fixed HUD layout via func_80072F30 / func_80072FCC: 17-tile horizontal row + 3 large widgets + 5-tile countdown meter | [md](evidence/func_80073060.md) |

### Batch 4 (2026-05-17, 41 entries)

First batch at 50-target scale. Includes two recognised function
families: `trigger_event_*` (6 members, 3 event-pair codes × 2 flag
states) and `multi_stage_init_*` (8 members, one per character slot).
Concise per-function evidence files; see
[evidence/](evidence/) for the full prose.

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80019488` | `func_80019488` | `pack_status_nibble_triplet_80019488` | manual_review=packs low nibbles of D_80106A70..72 into a 12-bit u32 | [md](evidence/func_80019488.md) |
| `0x80021280` | `func_80021280` | `char_motion_nibble_scan_80021280` | manual_review=per-char (1100-byte stride) scans 4 nibbles of field_48 for type-4/5 stage markers, stashes positions | [md](evidence/func_80021280.md) |
| `0x80030524` | `func_80030524` | `clear_entries_marked_in_flag_table_80030524` | manual_review=iterates 12 entries at D_80106A7A (0x64 stride); clears those marked in parallel flag table | [md](evidence/func_80030524.md) |
| `0x80030900` | `func_80030900` | `spawn_hit_particle_80030900` | manual_review=spawns particle via coli_hit_body_weapon; randomises vel/spin/rotation; pops front of per-char hit queue | [md](evidence/func_80030900.md) |
| `0x80033498` | `func_80033498` | `get_stage_idx_80033498` | manual_review=pure switch on D_800A36A4 -- 2 mapping 6 specific stage values to indices 0..5 (else 0xFF) | [md](evidence/func_80033498.md) |
| `0x80033898` | `func_80033898` | `gpu_enable_and_state_reset_80033898` | manual_review=calls gpu_EnableDisplay then sets D_800A37B8=0, D_800A3834=3; mode-transition / scene-init hook | [md](evidence/func_80033898.md) |
| `0x80036034` | `func_80036034` | `vsync_4_with_helpers_80036034` | manual_review=runs 2 helpers + sys_VSync(4); 4-frame pause between work units | [md](evidence/func_80036034.md) |
| `0x80037540` | `func_80037540` | `special_cam_init_with_two_lookups_80037540` | manual_review=looks up 2 SpecialCam entries; dispatches marionation_camera_Init_80037468 for a transition | [md](evidence/func_80037540.md) |
| `0x80038658` | `func_80038658` | `file_io_state_advance_80038658` | manual_review=state-machine advance for file-I/O in D_800A31F4; writes result to g_motion_state_code | [md](evidence/func_80038658.md) |
| `0x80044498` | `func_80044498` | `clear_array_8010367E_80044498` | manual_review=clears 20 s16 entries at D_8010367E down to D_80103658; resets the g_tpage_slot_count[] array | [md](evidence/func_80044498.md) |
| `0x80045080` | `func_80045080` | `ndata_offset_minus_inf_len_80045080` | manual_review=reads D_800963EE[a0] as NDATA length-sectors, multiplies by 2048; computes buffer-remaining check | [md](evidence/func_80045080.md) |
| `0x80049584` | `func_80049584` | `efc_rob_type_dispatch_80049584` | manual_review=compares per-frame state vs last-frame in D_800EF980; dispatches efc_rob_set_type_particle only if state changed | [md](evidence/func_80049584.md) |
| `0x80049718` | `func_80049718` | `efc_anim_vehicle_setup_80049718` | manual_review=emits one or two 0x68-byte effect entries into OT chain; handles free-floating and vehicle-mounted modes | [md](evidence/func_80049718.md) |
| `0x80053584` | `func_80053584` | `setup_struct_53584_callback_80053584` | manual_review=copies 2 input structs into D_800EFA00; installs func_80053E9C as callback; dispatches func_80052D00 | [md](evidence/func_80053584.md) |
| `0x80060544` | `func_80060544` | `hud_emit_5part_layout_80060544` | manual_review=multi-part HUD layout emitter using static-geometry record pointers; emits 7 prim entries | [md](evidence/func_80060544.md) |
| `0x80060768` | `func_80060768` | `hud_emit_warning_tiles_80060768` | manual_review=emits 4 red-tinted HUD tiles with D_800A32B4/D_800A32B6 frame-counter Y-position pulse | [md](evidence/func_80060768.md) |
| `0x80061250` | `func_80061250` | `anim_emit_210009_with_setup_80061250` | manual_review=opcode-family emitter (0x210009 or 0x21000A); magenta tint 0xFF0060; vec3 from arg0 | [md](evidence/func_80061250.md) |
| `0x80061454` | `func_80061454` | `anim_emit_29000B_80061454` | manual_review=opcode 0x29000B emitter with light-blue 0x8080FF tint; vec3 from arg0 | [md](evidence/func_80061454.md) |
| `0x80061658` | `func_80061658` | `anim_emit_21000CD_80061658` | manual_review=switch on arg1 between opcodes 0x21000C / 0x21000D; cyan 0x10FFFF tint | [md](evidence/func_80061658.md) |
| `0x80061710` | `func_80061710` | `anim_emit_21000EF_80061710` | manual_review=switch on arg1 between opcodes 0x21000E / 0x21000F; green 0x10FF10 tint | [md](evidence/func_80061710.md) |
| `0x80067060` | `func_80067060` | `trigger_event_4_2_no_flag_80067060` | manual_review=family member: func_80067200(4, 2, 0); sets D_800F1128 = 1 | [md](evidence/func_80067060.md) |
| `0x80067094` | `func_80067094` | `trigger_event_4_2_with_flag_80067094` | manual_review=family member: func_80067200(4, 2, 1); sets D_800F1128 = 2 | [md](evidence/func_80067094.md) |
| `0x80067130` | `func_80067130` | `trigger_event_6_3_no_flag_80067130` | manual_review=family member: func_80067200(6, 3, 0); sets D_800F1130 = 1 | [md](evidence/func_80067130.md) |
| `0x80067164` | `func_80067164` | `trigger_event_6_3_with_flag_80067164` | manual_review=family member: func_80067200(6, 3, 1); sets D_800F1130 = 2 | [md](evidence/func_80067164.md) |
| `0x80067198` | `func_80067198` | `trigger_event_7_2_no_flag_80067198` | manual_review=family member: func_80067200(7, 2, 0); sets D_800F1134 = 1 | [md](evidence/func_80067198.md) |
| `0x800671CC` | `func_800671CC` | `trigger_event_7_2_with_flag_800671CC` | manual_review=family member: func_80067200(7, 2, 1); sets D_800F1134 = 2 | [md](evidence/func_800671cc.md) |
| `0x800676C8` | `func_800676C8` | `multi_stage_init_0_0_800676C8` | manual_review=family member: 3-helper init chain with mode (0, 0) | [md](evidence/func_800676c8.md) |
| `0x80067704` | `func_80067704` | `multi_stage_init_1_1_80067704` | manual_review=family member: 3-helper init with mode (1, 1) | [md](evidence/func_80067704.md) |
| `0x80067740` | `func_80067740` | `multi_stage_init_2_1_80067740` | manual_review=family member: 3-helper init with mode (2, 1) | [md](evidence/func_80067740.md) |
| `0x8006777C` | `func_8006777C` | `multi_stage_init_3_0_8006777C` | manual_review=family member: 3-helper init with mode (3, 0) | [md](evidence/func_8006777c.md) |
| `0x800677B8` | `func_800677B8` | `multi_stage_init_4_2_800677B8` | manual_review=family member: 3-helper init with mode (4, 2) | [md](evidence/func_800677b8.md) |
| `0x800677F4` | `func_800677F4` | `multi_stage_init_5_3_800677F4` | manual_review=family member: 3-helper init with mode (5, 3) | [md](evidence/func_800677f4.md) |
| `0x80067830` | `func_80067830` | `multi_stage_init_6_3_80067830` | manual_review=family member: 3-helper init with mode (6, 3) | [md](evidence/func_80067830.md) |
| `0x8006786C` | `func_8006786C` | `multi_stage_init_7_2_8006786C` | manual_review=family member: 3-helper init with mode (7, 2); completes 8-member family | [md](evidence/func_8006786c.md) |
| `0x80069120` | `func_80069120` | `sync_flag_check_advance_80069120` | manual_review=returns 44-byte record table entry; side-effect sync via func_8006E8CC on dirty-bit change | [md](evidence/func_80069120.md) |
| `0x80069250` | `func_80069250` | `pad_check_dispatch_80069250` | manual_review=per-frame pad handler; on PAD_CIRCLE+TRIANGLE plays SFX volume 127 and returns 1 | [md](evidence/func_80069250.md) |
| `0x80077724` | `func_80077724` | `setup_double_buffer_swap_80077724` | manual_review=per-frame double-buffer swap; toggles buffer ptr, advances frame counters, dispatches draw via func_80077374 | [md](evidence/func_80077724.md) |
| `0x80078654` | `func_80078654` | `draw_progress_crossfade_80078654` | manual_review=per-frame progress/loading-bar emitter with 15-tick fade-out past threshold D_800A3608 >= 0xB04 | [md](evidence/func_80078654.md) |
| `0x80078824` | `func_80078824` | `disp_init_video_overlay_80078824` | manual_review=video/overlay subsystem init; matches disp-init family tail (disp_SetFramebufferMode(1,0,0,0); return 1) | [md](evidence/func_80078824.md) |
| `0x80085270` | `func_80085270` | `spu_channel_reset_80085270` | manual_review=per-channel SPU reset; uses spu_NotifyChannel/spu_ResetCounter; 0xB0-byte per-channel record stride | [md](evidence/func_80085270.md) |
| `0x80085448` | `func_80085448` | `settings_set_volume_pair_80085448` | manual_review=2-channel SPU volume setter; clamps to 0x7F and applies 129*2 = 258 PSX volume scale | [md](evidence/func_80085448.md) |

### Batch 5 (2026-05-17, family-parent deepen pass)

Names the parent dispatchers that the batch-4 function families
(`trigger_event_*`, `multi_stage_init_*`) flow through. Naming
these 4 lifts the 14 child names from address-suffix placeholders
toward clean semantic names.

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80067200` | `func_80067200` | `efc_gte_setup_and_draw_80067200` | manual_review=GTE-using effect dispatcher; loads rotation matrix from D_800A3474 via ctc2 (cop2 regs 0..4); branches on size class to set texture-coord limits 0x40/0x20 vs 0x60/0x30 (texture page sizes); called by the 6-member trigger_event_* family | [md](evidence/func_80067200.md) |
| `0x800678A8` | `func_800678A8` | `efc_init_phase1_setup_buffer_800678A8` | manual_review=phase 1 of 3-helper effect-init triple; sets primitive type code 0x2E, default texture sizes 0x40/0x20, per-mode init values in the effect buffer at D_800A34EC | [md](evidence/func_800678a8.md) |
| `0x80067D14` | `func_80067D14` | `efc_init_phase2_randomize_80067D14` | manual_review=phase 2 of effect-init triple (1108 asm lines, largest); calls bb2_rand early; walks effect buffer at 8 offsets writing randomised values; heavy GTE usage for per-particle transforms | [md](evidence/func_80067d14.md) |
| `0x80068D88` | `func_80068D88` | `efc_init_phase3_finalize_80068D88` | manual_review=phase 3 of effect-init triple (87 asm lines, smallest); computes derived summary value via reciprocal-multiply / divide-by-8 sequence; updates buffer +0x80 / +0x7C fields | [md](evidence/func_80068d88.md) |

### Batch 5 (2026-05-17, 30 medium entries)

Going deeper into the queue (0-caller leaf utilities + small wrappers).
Includes the motion-ex-init triple, player-slot getters, scratchpad
helpers, scene/state functions, audio chain triggers, HUD emitters,
varargs entry-table builder.

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80017848` | `func_80017848` | `link_add_with_dist_check_80017848` | manual_review=adds bidirectional link between 2 slots in a context graph (checks duplicates, computes math_Distance3D, stores 16-byte link record) | [md](evidence/func_80017848.md) |
| `0x80021904` | `func_80021904` | `char_get_motion_keyframe_table_entry_prev_80021904` | manual_review=direct sibling of func_80021974 (named char_get_motion_keyframe_*); reads D_80101F4E (PREVIOUS keyframe) instead of D_80101F4C (current) | [md](evidence/func_80021904.md) |
| `0x80022224` | `func_80022224` | `stage_find_far_corners_with_rand_80022224` | manual_review=picks one of the 2 furthest stage corners via bb2_rand & 1; companion to stage_find_nearest_corner_80022408 (batch 4) | [md](evidence/func_80022224.md) |
| `0x80023648` | `func_80023648` | `char_physics_step_horizontal_velocity_80023648` | manual_review=per-frame horizontal-velocity update using sin/cos from named Judge LUT; updates position +0xD8/+0xE0 | [md](evidence/func_80023648.md) |
| `0x80030208` | `func_80030208` | `hit_queue_process_pending_80030208` | manual_review=per-frame processor of 12 entries at D_80106A78 (0x64 stride); dispatches via func_80049718 (= efc_anim_vehicle_setup) + saSeInit_2 | [md](evidence/func_80030208.md) |
| `0x80032064` | `func_80032064` | `particle_spawn_entry_80032064` | manual_review=spawns particle into 4-slot pool at D_80104E88 (0x2C stride); uses named Judge sin LUT; dispatches via func_80032854 (motion_dispatch_event_handler) | [md](evidence/func_80032064.md) |
| `0x80038988` | `func_80038988` | `file_io_state_dispatch_80038988` | manual_review=state-machine on game_state_advance_80038734's return; maintains 5 per-state counters with 5-shot thresholds; 1.5-sec timeout | [md](evidence/func_80038988.md) |
| `0x80039680` | `func_80039680` | `snapshot_char_state_for_record_80039680` | manual_review=snapshots char state (position, rotation, motion id, flags) into 28-byte slot in ring buffer indexed by D_800A36F8; 2 chars per frame | [md](evidence/func_80039680.md) |
| `0x80040400` | `func_80040400` | `anim_chain_insert_entry_80040400` | manual_review=appends entry to per-player 0x68-byte animation chain (matches player_rob_relocate's +0x112C chain stride) | [md](evidence/func_80040400.md) |
| `0x80041554` | `func_80041554` | `player_get_field_8_80041554` | manual_review=1-line getter for halfword[4] (byte +8) of g_player_ptrs[a0]; returns -1 if slot empty | [md](evidence/func_80041554.md) |
| `0x80041650` | `func_80041650` | `player_get_field_2_low5_80041650` | manual_review=1-line getter for halfword[1] & 0x1F (low 5 bits at +2) of player struct | [md](evidence/func_80041650.md) |
| `0x80043244` | `func_80043244` | `classify_magnitude_4bins_80043244` | manual_review=4-bin distance-magnitude classifier with threshold structure consistent with LOD tier selection | [md](evidence/func_80043244.md) |
| `0x80043278` | `func_80043278` | `scratchpad_fixed_point_op_80043278` | manual_review=scratchpad-accessing bit op using shift amount from 0x1F800008; extracts 12-bit result | [md](evidence/func_80043278.md) |
| `0x80044170` | `func_80044170` | `packed_entry_table_builder_80044170` | manual_review=varargs builder for packed entry-table; copies subset of entries selected by index list, rebuilds offsets | [md](evidence/func_80044170.md) |
| `0x80046048` | `func_80046048` | `audio_chain_fade_load_80046048` | manual_review=chains tpage_slot_relocate(6, a1) + walks slot 6 payload + saFidLoad for each entry | [md](evidence/func_80046048.md) |
| `0x80048530` | `func_80048530` | `variable_lookup_table_dispatch_80048530` | manual_review=2-level table indexer + dispatch (arg1 picks sub-table, arg2 picks 12-byte entry); calls func_800485EC with 6 fields | [md](evidence/func_80048530.md) |
| `0x80053304` | `func_80053304` | `setup_struct_53584_ricochet_variant_80053304` | manual_review=variant of setup_struct_53584_callback; for close-range targets (squared dist <= 0x9C3F) computes reflected target and uses func_80053754 callback | [md](evidence/func_80053304.md) |
| `0x80053694` | `func_80053694` | `read_struct_53694_position_80053694` | manual_review=reads position + rotation from the struct at D_800A33F4 (managed by setup_struct_53584_callback batch 4); returns 0 if sentinel 0x7FFFFFFF | [md](evidence/func_80053694.md) |
| `0x80055948` | `func_80055948` | `flag_command_stream_interpreter_80055948` | manual_review=walks compressed bytecode stream that sets/clears bits in 32-bit flag word at +0x3C8 of arg0; uses inter-command delay bytes | [md](evidence/func_80055948.md) |
| `0x80057094` | `func_80057094` | `ai_compute_target_direction_80057094` | manual_review=AI helper using named single_game_getEnemyCharId to compute 8-direction command code from current+target position | [md](evidence/func_80057094.md) |
| `0x80062020` | `func_80062020` | `copy_terminated_triples_80062020` | manual_review=linked-list copier for 12-byte (3 s32) records terminated by low-bit-clear flag; output to D_800F1198 | [md](evidence/func_80062020.md) |
| `0x80065000` | `func_80065000` | `motion_ex_init_state_80065000` | manual_review=init/reset for motion-ex-play family state; copies 3 fields from D_800A347C; clears counter; stores stage selector from packed bits | [md](evidence/func_80065000.md) |
| `0x80065134` | `func_80065134` | `motion_ex_init_state_b_80065134` | manual_review=2nd init for motion-ex-play (mirrors 80065000 but different slot D_800F0D24..0xD2C; clears id 0xB counter) | [md](evidence/func_80065134.md) |
| `0x80065264` | `func_80065264` | `motion_ex_init_state_c_80065264` | manual_review=3rd init for motion-ex-play (different slot D_800F0D60..0xD68; clears id 0x10 counter) | [md](evidence/func_80065264.md) |
| `0x80065484` | `func_80065484` | `motion_ex_play_id5_dynamic_80065484` | manual_review=variable-rate variant of motion-ex-play family; per-stage step multiplier 1x/2x/3x for motion id 5 | [md](evidence/func_80065484.md) |
| `0x80065680` | `func_80065680` | `motion_ex_play_idCE_80065680` | manual_review=combined wrapper: plays motion id 0xC AND 0xE in sequence with separate counters and 11-play cap | [md](evidence/func_80065680.md) |
| `0x80074220` | `func_80074220` | `hud_emit_4tile_layout_80074220` | manual_review=fixed multi-layer HUD layout: 1 tile + 3 polygon prims (loop) + textured page + 3 PolyF4 quads to OT slots 0x78/7C/80 | [md](evidence/func_80074220.md) |
| `0x80075670` | `func_80075670` | `pad_handle_swap_80075670` | manual_review=per-player pad-input handler with right-button swap + left-button conditional swap + per-button SFX feedback | [md](evidence/func_80075670.md) |
| `0x80077860` | `func_80077860` | `pad_check_dispatch_wrapper_80077860` | manual_review=wrapper around named pad_check_dispatch_80069250; clears D_800A35E4 on input-consumed | [md](evidence/func_80077860.md) |
| `0x80077894` | `func_80077894` | `get_disp_config_lookup_index_80077894` | manual_review=pad-input-driven setter for low 4 bits of D_8009BD38 (the disp-config selector); companion to disp_lookup_config_pair_80077904 | [md](evidence/func_80077894.md) |
| `0x80077904` | `func_80077904` | `disp_lookup_config_pair_80077904` | manual_review=indexes paired-byte tables D_8009BD58/D_8009BD59 by (D_8009BD38 & 0xF) * 2; stores one byte in D_800A35E0, returns the other | [md](evidence/func_80077904.md) |

Note: `func_80021974` (batch 1) and `func_80054434` (batch 2) are
genuinely low-confidence (`rank=low` in CSV) and are not listed
above. There is no `proposals_low_confidence.md` file yet; reviewers
can find the low-confidence entries directly in `proposals.csv`
filtered by `confidence=low`. `func_80040510` was promoted from
batch 1 to the high-confidence table; batch 2 added two more high
entries (`game_event_shutdown_*` and `game_event_wait_any_*`).

## Primary evidence: `sole_caller_path` (74)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80016A8C` | `func_80016A8C` | `cpu_set_move_command_and_dir_for_no_action_2_helper_80016A8C` | sole_caller_path=cpu_set_move_command_and_dir_for_no_action_2_helper_80016A8C | [md](evidence/func_80016A8C.md) |
| `0x80016E60` | `func_80016E60` | `cpu_set_move_command_and_dir_for_no_action_2_helper_80016E60` | sole_caller_path=cpu_set_move_command_and_dir_for_no_action_2_helper_80016E60 | [md](evidence/func_80016E60.md) |
| `0x8001945C` | `func_8001945C` | `kgm_clamp_patch_init_helper_8001945C` | sole_caller_path=kgm_clamp_patch_init_helper_8001945C | [md](evidence/func_8001945C.md) |
| `0x8001B138` | `func_8001B138` | `single_game_VoiceContorol_helper_8001B138` | sole_caller_path=single_game_VoiceContorol_helper_8001B138 | [md](evidence/func_8001B138.md) |
| `0x8001DB58` | `func_8001DB58` | `mk_leaf_newpos_helper_8001DB58` | sole_caller_path=mk_leaf_newpos_helper_8001DB58 | [md](evidence/func_8001DB58.md) |
| `0x8001FAE4` | `func_8001FAE4` | `single_game_CheckStatusUpDataTotalOver_helper_8001FAE4` | sole_caller_path=single_game_CheckStatusUpDataTotalOver_helper_8001FAE4 | [md](evidence/func_8001FAE4.md) |
| `0x8001FB34` | `func_8001FB34` | `single_game_CheckStatusUpDataTotalOver_helper_8001FB34` | sole_caller_path=single_game_CheckStatusUpDataTotalOver_helper_8001FB34 | [md](evidence/func_8001FB34.md) |
| `0x80020DDC` | `func_80020DDC` | `DispPracticeMenuTex_B_helper_80020DDC` | sole_caller_path=DispPracticeMenuTex_B_helper_80020DDC | [md](evidence/func_80020DDC.md) |
| `0x80021210` | `func_80021210` | `mario_test_Exec_helper_80021210` | sole_caller_path=mario_test_Exec_helper_80021210 | [md](evidence/func_80021210.md) |
| `0x800219E4` | `func_800219E4` | `mario_test_Exec_helper_800219E4` | sole_caller_path=mario_test_Exec_helper_800219E4 | [md](evidence/func_800219E4.md) |
| `0x800272FC` | `func_800272FC` | `calc_teasi_loc_fw_helper_800272FC` | sole_caller_path=calc_teasi_loc_fw_helper_800272FC | [md](evidence/func_800272FC.md) |
| `0x8002906C` | `func_8002906C` | `se_data_set_helper_8002906C` | sole_caller_path=se_data_set_helper_8002906C | [md](evidence/func_8002906C.md) |
| `0x8002CA8C` | `func_8002CA8C` | `calc_loc_mat_fw_helper_8002CA8C` | sole_caller_path=calc_loc_mat_fw_helper_8002CA8C | [md](evidence/func_8002CA8C.md) |
| `0x80032040` | `func_80032040` | `mario_test_Exec_helper_80032040` | sole_caller_path=mario_test_Exec_helper_80032040 | [md](evidence/func_80032040.md) |
| `0x80033510` | `func_80033510` | `mario_test_Exec_helper_80033510` | sole_caller_path=mario_test_Exec_helper_80033510 | [md](evidence/func_80033510.md) |
| `0x80033550` | `func_80033550` | `DispPracticeMenuTex_C_helper_80033550` | sole_caller_path=DispPracticeMenuTex_C_helper_80033550 | [md](evidence/func_80033550.md) |
| `0x800343F0` | `func_800343F0` | `DispSamnailWindow_helper_800343F0` | sole_caller_path=DispSamnailWindow_helper_800343F0 | [md](evidence/func_800343F0.md) |
| `0x80036D88` | `func_80036D88` | `game_FrameLoop_helper_80036D88` | sole_caller_path=game_FrameLoop_helper_80036D88 | [md](evidence/func_80036D88.md) |
| `0x80037AA4` | `func_80037AA4` | `pad_FuncAnalog_helper_80037AA4` | sole_caller_path=pad_FuncAnalog_helper_80037AA4 | [md](evidence/func_80037AA4.md) |
| `0x80037B00` | `func_80037B00` | `pad_FuncAnalog_helper_80037B00` | sole_caller_path=pad_FuncAnalog_helper_80037B00 | [md](evidence/func_80037B00.md) |
| `0x80037B90` | `func_80037B90` | `pad_FuncAnalog_helper_80037B90` | sole_caller_path=pad_FuncAnalog_helper_80037B90; string_adjacent_info=? | [md](evidence/func_80037B90.md) |
| `0x80037C34` | `func_80037C34` | `pad_FuncAnalog_helper_80037C34` | sole_caller_path=pad_FuncAnalog_helper_80037C34; string_adjacent_info=? | [md](evidence/func_80037C34.md) |
| `0x80038170` | `func_80038170` | `pad_FuncAnalog_helper_80038170` | sole_caller_path=pad_FuncAnalog_helper_80038170 | [md](evidence/func_80038170.md) |
| `0x8003879C` | `func_8003879C` | `motion_SetMotion_helper_8003879C` | sole_caller_path=motion_SetMotion_helper_8003879C | [md](evidence/func_8003879C.md) |
| `0x800387E8` | `func_800387E8` | `motion_SetMotion_helper_800387E8` | sole_caller_path=motion_SetMotion_helper_800387E8 | [md](evidence/func_800387E8.md) |
| `0x8003ACB8` | `func_8003ACB8` | `replay_camera_rob_back_loose2_helper_8003ACB8` | sole_caller_path=replay_camera_rob_back_loose2_helper_8003ACB8 | [md](evidence/func_8003ACB8.md) |
| `0x8003B20C` | `func_8003B20C` | `DispSamnailWindow_helper_8003B20C` | sole_caller_path=DispSamnailWindow_helper_8003B20C | [md](evidence/func_8003B20C.md) |
| `0x8003B3A4` | `func_8003B3A4` | `suDispMentalBar_helper_8003B3A4` | sole_caller_path=suDispMentalBar_helper_8003B3A4 | [md](evidence/func_8003B3A4.md) |
| `0x8003DBE4` | `func_8003DBE4` | `tslLineG5Init_helper_8003DBE4` | sole_caller_path=tslLineG5Init_helper_8003DBE4 | [md](evidence/func_8003DBE4.md) |
| `0x8003F388` | `func_8003F388` | `replay_camera_get_attack_number_helper_8003F388` | sole_caller_path=replay_camera_get_attack_number_helper_8003F388 | [md](evidence/func_8003F388.md) |
| `0x8003FA24` | `func_8003FA24` | `tslPrintScreen_helper_8003FA24` | sole_caller_path=tslPrintScreen_helper_8003FA24; string_adjacent_info=? | [md](evidence/func_8003FA24.md) |
| `0x8003FECC` | `func_8003FECC` | `tslPrintScreen_helper_8003FECC` | sole_caller_path=tslPrintScreen_helper_8003FECC | [md](evidence/func_8003FECC.md) |
| `0x8003FFE0` | `func_8003FFE0` | `mario_test_Exec_helper_8003FFE0` | sole_caller_path=mario_test_Exec_helper_8003FFE0 | [md](evidence/func_8003FFE0.md) |
| `0x80041988` | `func_80041988` | `AllocRobRmd_helper_80041988` | sole_caller_path=AllocRobRmd_helper_80041988 | [md](evidence/func_80041988.md) |
| `0x80043398` | `func_80043398` | `AllocRobRmd_helper_80043398` | sole_caller_path=AllocRobRmd_helper_80043398 | [md](evidence/func_80043398.md) |
| `0x800433E4` | `func_800433E4` | `efc_rob_set_type_particle_helper_800433E4` | sole_caller_path=efc_rob_set_type_particle_helper_800433E4 | [md](evidence/func_800433E4.md) |
| `0x80044B30` | `func_80044B30` | `DispPracticeMenuTex_C_helper_80044B30` | sole_caller_path=DispPracticeMenuTex_C_helper_80044B30 | [md](evidence/func_80044B30.md) |
| `0x80044DE4` | `func_80044DE4` | `hirahira_w_ctrl_helper_80044DE4` | sole_caller_path=hirahira_w_ctrl_helper_80044DE4 | [md](evidence/func_80044DE4.md) |
| `0x80044F50` | `func_80044F50` | `efc_rob_set_type_particle_helper_80044F50` | sole_caller_path=efc_rob_set_type_particle_helper_80044F50 | [md](evidence/func_80044F50.md) |
| `0x80045A50` | `func_80045A50` | `player_Destroy_helper_80045A50` | sole_caller_path=player_Destroy_helper_80045A50 | [md](evidence/func_80045A50.md) |
| `0x80048A7C` | `func_80048A7C` | `saTan4FireDisp_helper_80048A7C` | sole_caller_path=saTan4FireDisp_helper_80048A7C | [md](evidence/func_80048A7C.md) |
| `0x80049C24` | `func_80049C24` | `efc_rob_set_type_particle_helper_80049C24` | sole_caller_path=efc_rob_set_type_particle_helper_80049C24 | [md](evidence/func_80049C24.md) |
| `0x8005BA8C` | `func_8005BA8C` | `se_data_set_helper_8005BA8C` | sole_caller_path=se_data_set_helper_8005BA8C | [md](evidence/func_8005BA8C.md) |
| `0x8005BD30` | `func_8005BD30` | `se_data_set_helper_8005BD30` | sole_caller_path=se_data_set_helper_8005BD30 | [md](evidence/func_8005BD30.md) |
| `0x8005BDF0` | `func_8005BDF0` | `se_data_set_helper_8005BDF0` | sole_caller_path=se_data_set_helper_8005BDF0 | [md](evidence/func_8005BDF0.md) |
| `0x8005D46C` | `func_8005D46C` | `gnd_disp_loop_ctrl_helper_8005D46C` | sole_caller_path=gnd_disp_loop_ctrl_helper_8005D46C | [md](evidence/func_8005D46C.md) |
| `0x8005E51C` | `func_8005E51C` | `camera_set_target_zoom_helper_8005E51C` | sole_caller_path=camera_set_target_zoom_helper_8005E51C | [md](evidence/func_8005E51C.md) |
| `0x8005F1C8` | `func_8005F1C8` | `camera_set_target_zoom_helper_8005F1C8` | sole_caller_path=camera_set_target_zoom_helper_8005F1C8 | [md](evidence/func_8005F1C8.md) |
| `0x8005FBC8` | `func_8005FBC8` | `cpu_side_move_dir_2_helper_8005FBC8` | sole_caller_path=cpu_side_move_dir_2_helper_8005FBC8 | [md](evidence/func_8005FBC8.md) |
| `0x80060414` | `func_80060414` | `gnd_disp_loop_ctrl_helper_80060414` | sole_caller_path=gnd_disp_loop_ctrl_helper_80060414 | [md](evidence/func_80060414.md) |
| `0x800620B8` | `func_800620B8` | `DispHira_helper_800620B8` | sole_caller_path=DispHira_helper_800620B8 | [md](evidence/func_800620B8.md) |
| `0x80077A28` | `func_80077A28` | `replay_camera_rob_back_loose2_helper_80077A28` | sole_caller_path=replay_camera_rob_back_loose2_helper_80077A28 | [md](evidence/func_80077A28.md) |
| `0x80077A80` | `func_80077A80` | `replay_camera_rob_back_loose2_helper_80077A80` | sole_caller_path=replay_camera_rob_back_loose2_helper_80077A80 | [md](evidence/func_80077A80.md) |
| `0x80077D10` | `func_80077D10` | `camera_get_rot_normal_rad_helper_80077D10` | sole_caller_path=camera_get_rot_normal_rad_helper_80077D10 | [md](evidence/func_80077D10.md) |
| `0x800789D8` | `func_800789D8` | `cpu_set_move_command_and_dir_for_no_action_2_helper_800789D8` | sole_caller_path=cpu_set_move_command_and_dir_for_no_action_2_helper_800789D8 | [md](evidence/func_800789D8.md) |
| `0x80078E20` | `func_80078E20` | `pad_Init_helper_80078E20` | sole_caller_path=pad_Init_helper_80078E20 | [md](evidence/func_80078E20.md) |
| `0x80078F74` | `func_80078F74` | `pad_Init_helper_80078F74` | sole_caller_path=pad_Init_helper_80078F74 | [md](evidence/func_80078F74.md) |
| `0x80079244` | `func_80079244` | `debug_printf_helper_80079244` | sole_caller_path=debug_printf_helper_80079244; string_adjacent_info=? | [md](evidence/func_80079244.md) |
| `0x8007A370` | `func_8007A370` | `pad_press_control_helper_8007A370` | sole_caller_path=pad_press_control_helper_8007A370 | [md](evidence/func_8007A370.md) |
| `0x8007A3C8` | `func_8007A3C8` | `pad_press_control_helper_8007A3C8` | sole_caller_path=pad_press_control_helper_8007A3C8 | [md](evidence/func_8007A3C8.md) |
| `0x800800CC` | `func_800800CC` | `saEft00Add_helper_800800CC` | sole_caller_path=saEft00Add_helper_800800CC | [md](evidence/func_800800CC.md) |
| `0x80082D34` | `func_80082D34` | `motion_make_table_helper_80082D34` | sole_caller_path=motion_make_table_helper_80082D34; string_adjacent_info=? | [md](evidence/func_80082D34.md) |
| `0x80083220` | `func_80083220` | `motion_make_table_helper_80083220` | sole_caller_path=motion_make_table_helper_80083220 | [md](evidence/func_80083220.md) |
| `0x800841E0` | `func_800841E0` | `DispStuff_helper_800841E0` | sole_caller_path=DispStuff_helper_800841E0 | [md](evidence/func_800841E0.md) |
| `0x80084500` | `func_80084500` | `DispStuff_helper_80084500` | sole_caller_path=DispStuff_helper_80084500 | [md](evidence/func_80084500.md) |
| `0x80084A7C` | `func_80084A7C` | `saTan0Main_helper_80084A7C` | sole_caller_path=saTan0Main_helper_80084A7C | [md](evidence/func_80084A7C.md) |
| `0x800856B0` | `func_800856B0` | `DispStuff_helper_800856B0` | sole_caller_path=DispStuff_helper_800856B0 | [md](evidence/func_800856B0.md) |
| `0x80086CF8` | `func_80086CF8` | `AllocBukiRmd_helper_80086CF8` | sole_caller_path=AllocBukiRmd_helper_80086CF8 | [md](evidence/func_80086CF8.md) |
| `0x800872A4` | `func_800872A4` | `AllocBukiRmd_helper_800872A4` | sole_caller_path=AllocBukiRmd_helper_800872A4 | [md](evidence/func_800872A4.md) |
| `0x8008A904` | `func_8008A904` | `action_CheckHitZangeki_helper_8008A904` | sole_caller_path=action_CheckHitZangeki_helper_8008A904 | [md](evidence/func_8008A904.md) |
| `0x8008AAD4` | `func_8008AAD4` | `action_CheckHitZangeki_helper_8008AAD4` | sole_caller_path=action_CheckHitZangeki_helper_8008AAD4 | [md](evidence/func_8008AAD4.md) |
| `0x8008AF84` | `func_8008AF84` | `saTan2Main_helper_8008AF84` | sole_caller_path=saTan2Main_helper_8008AF84 | [md](evidence/func_8008AF84.md) |
| `0x8008BB24` | `func_8008BB24` | `saTan1MainJump_helper_8008BB24` | sole_caller_path=saTan1MainJump_helper_8008BB24 | [md](evidence/func_8008BB24.md) |
| `0x8008BDE8` | `func_8008BDE8` | `action_CheckHitZangeki_helper_8008BDE8` | sole_caller_path=action_CheckHitZangeki_helper_8008BDE8 | [md](evidence/func_8008BDE8.md) |

## Primary evidence: `kengo_pattern` (16)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80037250` | `func_80037250` | `replay_camera_check_stage_80037250` | kengo_pattern=replay_camera_check_stage_80037250 | [md](evidence/func_80037250.md) |
| `0x800387C0` | `func_800387C0` | `motion_SetMotion_helper_800387C0` | kengo_pattern=motion_SavePreCalcData_800387C0; sole_caller_path=motion_SetMotion_helper_800387C0 | [md](evidence/func_800387C0.md) |
| `0x8003A41C` | `func_8003A41C` | `replay_camera_check_stage_8003A41C` | kengo_pattern=replay_camera_check_stage_8003A41C | [md](evidence/func_8003A41C.md) |
| `0x8003B2C8` | `func_8003B2C8` | `suDispMentalBar_helper_8003B2C8` | kengo_pattern=coli_cancel_hit_pause_katana_8003B2C8; sole_caller_path=suDispMentalBar_helper_8003B2C8 | [md](evidence/func_8003B2C8.md) |
| `0x8003B4DC` | `func_8003B4DC` | `suDispMentalBar_helper_8003B4DC` | kengo_pattern=coli_cancel_hit_pause_katana_8003B4DC; sole_caller_path=suDispMentalBar_helper_8003B4DC | [md](evidence/func_8003B4DC.md) |
| `0x8003E2C8` | `func_8003E2C8` | `replay_camera_check_stage_8003E2C8` | kengo_pattern=replay_camera_check_stage_8003E2C8 | [md](evidence/func_8003E2C8.md) |
| `0x80044E64` | `func_80044E64` | `myRobGeneiClose_80044E64` | kengo_pattern=myRobGeneiClose_80044E64 | [md](evidence/func_80044E64.md) |
| `0x80044E6C` | `func_80044E6C` | `myRobGeneiClose_80044E6C` | kengo_pattern=myRobGeneiClose_80044E6C | [md](evidence/func_80044E6C.md) |
| `0x800457D4` | `func_800457D4` | `myRobGeneiClose_800457D4` | kengo_pattern=myRobGeneiClose_800457D4 | [md](evidence/func_800457D4.md) |
| `0x800457DC` | `func_800457DC` | `efc_ougi_Init_800457DC` | kengo_pattern=efc_ougi_Init_800457DC | [md](evidence/func_800457DC.md) |
| `0x800457FC` | `func_800457FC` | `efc_ougi_Init_800457FC` | kengo_pattern=efc_ougi_Init_800457FC | [md](evidence/func_800457FC.md) |
| `0x80045808` | `func_80045808` | `efc_ougi_Init_80045808` | kengo_pattern=efc_ougi_Init_80045808 | [md](evidence/func_80045808.md) |
| `0x80045814` | `func_80045814` | `replay_camera_check_stage_80045814` | kengo_pattern=replay_camera_check_stage_80045814 | [md](evidence/func_80045814.md) |
| `0x80054410` | `func_80054410` | `efc_ougi_Init_80054410` | kengo_pattern=efc_ougi_Init_80054410 | [md](evidence/func_80054410.md) |
| `0x8005507C` | `func_8005507C` | `replay_camera_check_stage_8005507C` | kengo_pattern=replay_camera_check_stage_8005507C | [md](evidence/func_8005507C.md) |
| `0x8005508C` | `func_8005508C` | `replay_camera_check_stage_8005508C` | kengo_pattern=replay_camera_check_stage_8005508C | [md](evidence/func_8005508C.md) |

## Primary evidence: `call_graph_cluster` (9)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x800372C0` | `func_800372C0` | `cpu_helper_800372C0` | call_graph_cluster=cpu_helper_800372C0 | [md](evidence/func_800372C0.md) |
| `0x8006920C` | `func_8006920C` | `efc_helper_8006920C` | call_graph_cluster=efc_helper_8006920C | [md](evidence/func_8006920C.md) |
| `0x8007352C` | `func_8007352C` | `satan_helper_8007352C` | call_graph_cluster=satan_helper_8007352C; string_adjacent_info=? | [md](evidence/func_8007352C.md) |
| `0x8007B3A8` | `func_8007B3A8` | `gpu_helper_8007B3A8` | call_graph_cluster=gpu_helper_8007B3A8; string_adjacent=bad_func_8007B3A8 | [md](evidence/func_8007B3A8.md) |
| `0x80080148` | `func_80080148` | `special_camera_helper_80080148` | call_graph_cluster=special_camera_helper_80080148 | [md](evidence/func_80080148.md) |
| `0x80085E4C` | `func_80085E4C` | `obj_helper_80085E4C` | call_graph_cluster=obj_helper_80085E4C | [md](evidence/func_80085E4C.md) |
| `0x80085EE4` | `func_80085EE4` | `obj_helper_80085EE4` | call_graph_cluster=obj_helper_80085EE4 | [md](evidence/func_80085EE4.md) |
| `0x80085F98` | `func_80085F98` | `obj_helper_80085F98` | call_graph_cluster=obj_helper_80085F98 | [md](evidence/func_80085F98.md) |
| `0x80087F64` | `func_80087F64` | `obj_helper_80087F64` | call_graph_cluster=obj_helper_80087F64 | [md](evidence/func_80087F64.md) |

## Primary evidence: `psyq_idiom_legacy` (2)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80016C80` | `func_80016C80` | `psyq_memcpy_legacy_80016C80` | psyq_idiom_legacy=psyq_memcpy_legacy_80016C80; string_adjacent=eff_init_func_80016C80 | [md](evidence/func_80016C80.md) |
| `0x800550E8` | `func_800550E8` | `cpu_get_move_pattern_table_number_helper_800550E8` | psyq_idiom_legacy=psyq_memcpy_legacy_800550E8; sole_caller_path=cpu_get_move_pattern_table_number_helper_800550E8 | [md](evidence/func_800550E8.md) |

## Primary evidence: `gte_op` (2)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x8002A458` | `func_8002A458` | `calc_loc_mat_fw_helper_8002A458` | gte_op=gte_helper_8002A458; sole_caller_path=calc_loc_mat_fw_helper_8002A458 | [md](evidence/func_8002A458.md) |
| `0x800523E0` | `func_800523E0` | `hirahira_w_ctrl_helper_800523E0` | gte_op=gte_helper_800523E0; sole_caller_path=hirahira_w_ctrl_helper_800523E0 | [md](evidence/func_800523E0.md) |

