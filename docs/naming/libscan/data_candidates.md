# Data candidates (libscan-xref for globals)

| Sony global | base addr | modules | our names at base (refs) | referenced offsets -> our names |
|---|---|---|---|---|
| CD_cbready | 0x800A11B8 | 1 | g_cd_callback_b(8), g_cd_callback_b(8), D_800A11B8(21), g_cd_callback_b(8) |  |
| CD_cbsync | 0x800A11B4 | 1 | g_cd_callback_a(20), g_cd_callback_a(20), D_800A11B4(45), g_cd_callback_a(20) |  |
| CD_com | 0x800A11D5 | 1 | g_cd_ready_flag2(3), g_cd_ready_flag2(3), D_800A11D5(25), g_cd_ready_flag2(3) |  |
| CD_comstr | 0x800A11DC | 1 | g_cd_cmd_table(2), g_cd_cmd_table(2), D_800A11DC(23), g_cd_cmd_table(2) |  |
| CD_debug | 0x800A11C0 | 1 | g_cd_debug_level(3), g_cd_debug_level(3), g_cd_debug_level(3) |  |
| CD_intstr | 0x800A125C | 1 | g_cd_result_table(2), g_cd_result_table(2), D_800A125C(17), g_cd_result_table(2) |  |
| CD_mode | 0x800A11D4 | 1 | g_cd_ready_flag(3), g_cd_ready_flag(3), g_cd_ready_flag(3) |  |
| CD_pos | 0x800A11D0 | 1 | g_cd_param(2), g_cd_param(2), g_cd_param(2) |  |
| CD_status | 0x800A11C4 | 1 | g_cd_mode(8), g_cd_mode(8), g_cd_mode(8) |  |
| GPU_printf | 0x8009BE70 | 1 | g_gpu_debug_func(29), g_gpu_debug_func(29) |  |
| SQRT | 0x8009C7A8 | 2 | g_gte_sqrt_table(4), g_gte_sqrt_table(4) |  |
| VBLANK_MINUS | 0x80104E80 | 3 | g_blood_spot_timer(0), D_80104E80(34) |  |
| Vcount | 0x800A2634 | 1 | g_sys_dma_region(5), g_irq_vsync_counter(0), g_sys_dma_region(5), D_800A2634(21), g_sys_dma_region(5) |  |
| _SsMarkCallback | 0x80106FA8 | 1 | g_char_p19_field_388(0), D_80106FA8(4) |  |
| _SsVmMaxVoice | 0x80101BCC | 1 | g_memcard_slot(5), g_memcard_slot(5), D_80101BCC(39), g_memcard_slot(5) |  |
| _ctype_ | 0x8009BD8C | 2 | ? | +0x1: g_char_class_table, D_8009BD8D |
| _que | 0x80103680 | 1 | g_gpu_packet_queue_base(0), D_80103680(18) | +0x4: g_gpu_packet_buffer_64x96, D_80103684; +0x8: g_gpu_packet_buffer_64x96_plus_4, D_80103688; +0xC: g_gpu_packet_buffer_data_8010368C, D_8010368C |
| _snd_ev_flag | 0x800FF630 | 2 | g_ings2_counter_FF630(0), D_800FF630(30) |  |
| _snd_openflag | 0x801027E4 | 2 | g_char_curr_keyframe_idx_p3(0), D_801027E4(8) |  |
| _snd_seq_s_max | 0x801077A8 | 1 | D_801077A8(6) |  |
| _snd_seq_t_max | 0x801077AA | 1 | D_801077AA(6) |  |
| _snd_seq_tick_env | 0x800A26CC | 2 | g_blood_spot_pos(0), D_800A26CC(45) | +0x4: g_alarm_armed_flag, D_800A26D0; +0xC: g_alarm_callback_ptr, D_800A26D8; +0x10: g_alarm_callback_pending, D_800A26DC; +0x11: g_alarm_callback_pending_plus_1, D_800A26DD; +0x12: g_alarm_active_sen |
| _spu_AllocBlockNum | 0x800A2D38 | 4 | g_spu_voice_key_a(10), g_spu_voice_key_a(10), D_800A2D38(12), g_spu_voice_key_a(10) |  |
| _spu_AllocLastNum | 0x800A2D3C | 4 | g_spu_voice_key_b(9), g_spu_voice_key_b(9), D_800A2D3C(35), g_spu_voice_key_b(9) |  |
| _spu_EVdma | 0x800A2870 | 3 | g_snd_irq_handle(6), g_snd_irq_handle(6), D_800A2870(14), g_snd_irq_handle(6) |  |
| _spu_FiDMA | 0x80088BA0 | 1 | g_snd_irq_data(2), g_snd_irq_data(2), _spu_FiDMA(6), D_80088BA0(3), g_snd_irq_data(2) |  |
| _spu_IRQCallback | 0x800A2D18 | 1 | g_spu_timer(3), g_spu_timer(3), D_800A2D18(6), g_spu_timer(3) |  |
| _spu_RQ | 0x800F7420 | 2 | g_spu_reg_pair(0), D_800F7420(28) | +0x2: D_800F7422; +0x4: g_spu_reg_pair_b, D_800F7424; +0x6: D_800F7426 |
| _spu_RQmask | 0x800A28A0 | 1 | g_spu_voice_mask(0), D_800A28A0(29) |  |
| _spu_RQvoice | 0x800A289C | 1 | g_main_flags_bitmask_reg(0), D_800A289C(14) |  |
| _spu_RXX | 0x800A2CDC | 10 | g_spu_base_addr(40), g_spu_base_addr(40), D_800A2CDC(310), g_spu_base_addr(40) |  |
| _spu_env | 0x800A2CD4 | 1 | g_spu_pending_irq_mask(0), D_800A2CD4(22) |  |
| _spu_inTransfer | 0x800A2D10 | 2 | g_spu_busy(8), g_spu_busy(8), _spu_inTransfer(18), g_spu_busy(8) |  |
| _spu_isCalled | 0x800A2CD8 | 1 | g_snd_init_flag(6), g_snd_init_flag(6), D_800A2CD8(8), g_snd_init_flag(6) |  |
| _spu_keystat | 0x800A2874 | 3 | g_spu_xfer_mask(0), D_800A2874(22) |  |
| _spu_memList | 0x800A2D40 | 6 | g_spu_voice_key_c(11), g_spu_voice_key_c(11), D_800A2D40(35), g_spu_voice_key_c(11) |  |
| _spu_mem_mode_plus | 0x800A2D04 | 5 | g_spu_addr_shift(7), g_spu_addr_shift(7), D_800A2D04(40), g_spu_addr_shift(7) |  |
| _spu_mem_mode_unitM | 0x800A2D0C | 1 | g_spu_init_mask(0), D_800A2D0C(11) |  |
| _spu_rev_attr | 0x800A2888 | 1 | D_800A2888(29) | +0x4: g_spu_voice_alloc_offset_plus_8, D_800A288C; +0x8: g_spu_voice_alloc_offset_plus_8_plus_4, D_800A2890; +0xA: g_spu_voice_alloc_offset_plus_8_plus_6, D_800A2892; +0xC: g_spu_voice_alloc_offset_pl |
| _spu_rev_flag | 0x800A287C | 1 | g_spu_xfer_callback_arg(0), D_800A287C(16) |  |
| _spu_rev_offsetaddr | 0x800A2884 | 3 | g_spu_voice_alloc_offset(0), D_800A2884(16) |  |
| _spu_rev_param | 0x800A2D94 | 1 | ? |  |
| _spu_rev_reserve_wa | 0x800A2880 | 2 | g_spu_voice_alloc_current_a0(0), D_800A2880(10) |  |
| _spu_rev_startaddr | 0x800A2D44 | 3 | g_spu_init_reg_D1_val(0), D_800A2D44(16) |  |
| _spu_transMode | 0x800A2CF8 | 3 | g_spu_reverb_mode(5), g_spu_reverb_mode(5), D_800A2CF8(20), g_spu_reverb_mode(5) |  |
| _spu_trans_mode | 0x800A2878 | 2 | g_snd_reverb_flag(4), g_snd_reverb_flag(4), D_800A2878(6), g_snd_reverb_flag(4) |  |
| _spu_transferCallback | 0x800A2D14 | 2 | g_spu_init_flag(5), g_spu_init_flag(5), _spu_transferCallback(30), g_spu_init_flag(5) |  |
| _spu_tsa | 0x800A2CF4 | 1 | g_spu_xfer_addr(4), g_spu_xfer_addr(4), D_800A2CF4(27), g_spu_xfer_addr(4) |  |
| _spu_zerobuf | 0x800A28D4 | 1 | g_satan0_gauge_data_block_800A28D4(0), D_800A28D4(4) |  |
| _ss_score | 0x80106F28 | 7 | g_seq_bank_table(0), D_80106F28(63) |  |
| _svm_auto_kof_mode | 0x80103600 | 1 | g_memcard_data(2), g_memcard_data(2), D_80103600(6), g_memcard_data(2) |  |
| _svm_cur | 0x801027F0 | 2 | D_801027F0(35) | +0x1: g_char_curr_keyframe_idx_p3_plus_13, D_801027F1; +0x2: g_char_p3_byte_f2_801027F2, D_801027F2; +0x6: g_char_p3_field_96, D_801027F6; +0x7: g_weapon_frame_row, D_801027F7; +0xC: g_weapon_frame_co |
| _svm_pg | 0x800FF6A0 | 1 | g_voice_state_ptr(0), D_800FF6A0(17) |  |
| _svm_rattr | 0x800F5750 | 2 | g_buki_cmd_buf_id(0), D_800F5750(13) | +0x4: g_buki_cmd_buf_arg, D_800F5754; +0x8: g_buki_cmd_buf_x, D_800F5758; +0xA: g_buki_cmd_buf_y, D_800F575A |
| _svm_stereo_mono | 0x800FF578 | 1 | g_memcard_busy(3), g_memcard_busy(3), D_800FF578(12), g_memcard_busy(3) |  |
| _svm_tn | 0x80101BC8 | 2 | g_weapon_frame_data_base(0), D_80101BC8(16) |  |
| _svm_vab_count | 0x80107808 | 2 | g_snd_ch_count(2), g_snd_ch_count(2), D_80107808(22), g_snd_ch_count(2) |  |
| _svm_vab_pg | 0x800F6660 | 2 | g_snd_ch_texture_base_ptr(0), D_800F6660(7) |  |
| _svm_vab_start | 0x80107810 | 4 | g_snd_ch_addr(6), g_snd_ch_addr(6), D_80107810(10), g_snd_ch_addr(6) |  |
| _svm_vab_tn | 0x800F6700 | 2 | g_snd_ch_texture_meta_ptr(0), D_800F6700(7) |  |
| _svm_vab_total | 0x801077C8 | 2 | g_char_p21_field_310(0), D_801077C8(8) |  |
| _svm_vab_used | 0x80102A68 | 6 | g_snd_ch_status(11), g_snd_ch_status(11), D_80102A68(43), g_snd_ch_status(11) |  |
| _svm_vab_vh | 0x800F66B8 | 2 | g_snd_ch_texture_tpage(0), D_800F66B8(7) |  |
| _svm_vh | 0x80101BC4 | 1 | g_main_value_101BC4(0), D_80101BC4(8) |  |
| kMaxPrograms | 0x800FF634 | 2 | g_snd_ch_texture_max_frame(0), D_800FF634(16) |  |
| rcossin_tbl | 0x8009C928 | 5 | g_trig_sin_cos_table_packed(0) |  |
| rsin_tbl | 0x8009BF94 | 2 | g_sin_lut_q1(6), g_sin_lut_q1(6), g_sin_lut_q1(6) |  |
