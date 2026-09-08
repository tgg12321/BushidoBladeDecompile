# libscan-xref data web (Sony global name -> EXE address, from accepted verbatim placements)

| Sony global | EXE addr | our name(s) | referenced from |
|---|---|---|---|
| CD_cbready | 0x800A11B8 | g_cd_callback_b, g_cd_callback_b, D_800A11B8, g_cd_callback_b **<- differs** | LIBCD/SYS@0x8008009C |
| CD_cbsync | 0x800A11B4 | g_cd_callback_a, g_cd_callback_a, D_800A11B4, g_cd_callback_a **<- differs** | LIBCD/SYS@0x8008009C |
| CD_com | 0x800A11D5 | g_cd_ready_flag2, g_cd_ready_flag2, D_800A11D5, g_cd_ready_flag2 **<- differs** | LIBCD/SYS@0x8008009C |
| CD_comstr | 0x800A11DC | g_cd_cmd_table, g_cd_cmd_table, D_800A11DC, g_cd_cmd_table **<- differs** | LIBCD/SYS@0x8008009C |
| CD_debug | 0x800A11C0 | g_cd_debug_level, g_cd_debug_level, g_cd_debug_level **<- differs** | LIBCD/SYS@0x8008009C |
| CD_intstr | 0x800A125C | g_cd_result_table, g_cd_result_table, D_800A125C, g_cd_result_table **<- differs** | LIBCD/SYS@0x8008009C |
| CD_mode | 0x800A11D4 | g_cd_ready_flag, g_cd_ready_flag, g_cd_ready_flag **<- differs** | LIBCD/SYS@0x8008009C |
| CD_pos | 0x800A11D0 | g_cd_param, g_cd_param, g_cd_param **<- differs** | LIBCD/SYS@0x8008009C |
| CD_status | 0x800A11C4 | g_cd_mode, g_cd_mode, g_cd_mode **<- differs** | LIBCD/SYS@0x8008009C |
| GPU_printf | 0x8009BE70 | g_gpu_debug_func, g_gpu_debug_func **<- differs** | LIBGPU/PRIM@0x8007A788 |
| SQRT | 0x8009C7A8 | g_gte_sqrt_table, g_gte_sqrt_table **<- differs** | LIBGTE/MSC01@0x8007E11C; LIBGTE/MSC09@0x8007E43C |
| VBLANK_MINUS | 0x80104E80 | g_blood_spot_timer, D_80104E80 **<- differs** | LIBSND/SSINIT@0x80083A48; LIBSND/SSTICK@0x80085544; LIBSND/TEMPO@0x800856B0 |
| Vcount | 0x800A2634 | g_sys_dma_region, g_irq_vsync_counter, g_sys_dma_region, D_800A2634, g_sys_dma_region **<- differs** | LIBETC/VSYNC@0x800828CC |
| _SsMarkCallback | 0x80106FA8 | g_char_p19_field_388, D_80106FA8 **<- differs** | LIBSND/SSINIT@0x80083A48 |
| _SsVmMaxVoice | 0x80101BCC | g_memcard_slot, g_memcard_slot, D_80101BCC, g_memcard_slot **<- differs** | LIBSND/VS_SRV@0x80087F34 |
| _ctype_ | 0x8009BD8C | ? **<- differs** | LIBC2/PRNT@0x80079244; LIBC2/PUTCHAR@0x8007997C |
| _que | 0x80103680 | g_gpu_packet_queue_base, D_80103680 **<- differs** | LIBGPU/SYS@0x8007AE7C |
| _snd_ev_flag | 0x800FF630 | g_ings2_counter_FF630, D_800FF630 **<- differs** | LIBSND/SSCALL@0x80083F6C; LIBSND/SSINIT@0x80083A48 |
| _snd_openflag | 0x801027E4 | g_char_curr_keyframe_idx_p3, D_801027E4 **<- differs** | LIBSND/SSCALL@0x80083F6C; LIBSND/SSINIT@0x80083A48 |
| _snd_seq_s_max | 0x801077A8 | D_801077A8 **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _snd_seq_t_max | 0x801077AA | D_801077AA **<- differs** | LIBSND/SSCALL@0x80083F6C |
| _snd_seq_tick_env | 0x800A26CC | g_blood_spot_pos, D_800A26CC **<- differs** | LIBSND/SSEND@0x80083954; LIBSND/SSTICK@0x80085544 |
| _spu_AllocBlockNum | 0x800A2D38 | g_spu_voice_key_a, g_spu_voice_key_a, D_800A2D38, g_spu_voice_key_a **<- differs** | LIBSPU/S_INI@0x800885CC; LIBSPU/S_M_F@0x800899A8; LIBSPU/S_M_INIT@0x80089384; LIBSPU/S_M_M@0x800893D8 |
| _spu_AllocLastNum | 0x800A2D3C | g_spu_voice_key_b, g_spu_voice_key_b, D_800A2D3C, g_spu_voice_key_b **<- differs** | LIBSPU/S_INI@0x800885CC; LIBSPU/S_M_INIT@0x80089384; LIBSPU/S_M_INT@0x800896A0; LIBSPU/S_M_M@0x800893D8 |
| _spu_EVdma | 0x800A2870 | g_snd_irq_handle, g_snd_irq_handle, D_800A2870, g_snd_irq_handle **<- differs** | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_ITC@0x8008AEB0; LIBSPU/S_Q@0x800892F8 |
| _spu_FiDMA | 0x80088BA0 | g_snd_irq_data, g_snd_irq_data, _spu_FiDMA, D_80088BA0, g_snd_irq_data | LIBSPU/S_INI@0x800885CC |
| _spu_IRQCallback | 0x800A2D18 | g_spu_timer, g_spu_timer, D_800A2D18, g_spu_timer **<- differs** | LIBSPU/S_Q@0x800892F8 |
| _spu_RQ | 0x800F7420 | g_spu_reg_pair, D_800F7420 **<- differs** | LIBSPU/SPU@0x80088740; LIBSPU/S_SK@0x8008AAD4 |
| _spu_RQmask | 0x800A28A0 | g_spu_voice_mask, D_800A28A0 **<- differs** | LIBSPU/S_SK@0x8008AAD4 |
| _spu_RQvoice | 0x800A289C | g_main_flags_bitmask_reg, D_800A289C **<- differs** | LIBSPU/S_SK@0x8008AAD4 |
| _spu_RXX | 0x800A2CDC | g_spu_base_addr, g_spu_base_addr, D_800A2CDC, g_spu_base_addr **<- differs** | LIBSPU/SR_GAKS@0x8008B330; LIBSPU/S_GKS@0x8008ACD0; LIBSPU/S_GVEX@0x8008BDE8; LIBSPU/S_GVV@0x8008BD88; LIBSPU/S_SCA@0x80 |
| _spu_env | 0x800A2CD4 | g_spu_pending_irq_mask, D_800A2CD4 **<- differs** | LIBSPU/S_SK@0x8008AAD4 |
| _spu_inTransfer | 0x800A2D10 | g_spu_busy, g_spu_busy, _spu_inTransfer, g_spu_busy | LIBSPU/S_IT@0x8008AF58; LIBSPU/S_ITC@0x8008AEB0 |
| _spu_isCalled | 0x800A2CD8 | g_snd_init_flag, g_snd_init_flag, D_800A2CD8, g_snd_init_flag **<- differs** | LIBSPU/S_Q@0x800892F8 |
| _spu_keystat | 0x800A2874 | g_spu_xfer_mask, D_800A2874 **<- differs** | LIBSPU/SR_GAKS@0x8008B330; LIBSPU/S_GKS@0x8008ACD0; LIBSPU/S_SK@0x8008AAD4 |
| _spu_memList | 0x800A2D40 | g_spu_voice_key_c, g_spu_voice_key_c, D_800A2D40, g_spu_voice_key_c **<- differs** | LIBSPU/S_INI@0x800885CC; LIBSPU/S_M_F@0x800899A8; LIBSPU/S_M_INIT@0x80089384; LIBSPU/S_M_INT@0x800896A0; LIBSPU/S_M_M@0x |
| _spu_mem_mode_plus | 0x800A2D04 | g_spu_addr_shift, g_spu_addr_shift, D_800A2D04, g_spu_addr_shift **<- differs** | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_M_INIT@0x80089384; LIBSPU/S_M_M@0x800893D8; LIBSPU/S_M_UTIL@0x80089E30; LIBSPU/S_STSA |
| _spu_mem_mode_unitM | 0x800A2D0C | g_spu_init_mask, D_800A2D0C **<- differs** | LIBSPU/S_M_M@0x800893D8 |
| _spu_rev_attr | 0x800A2888 | D_800A2888 **<- differs** | LIBSPU/S_SRMP@0x80089F3C |
| _spu_rev_flag | 0x800A287C | g_spu_xfer_callback_arg, D_800A287C **<- differs** | LIBSPU/S_SR@0x80089D60 |
| _spu_rev_offsetaddr | 0x800A2884 | g_spu_voice_alloc_offset, D_800A2884 **<- differs** | LIBSPU/S_M_M@0x800893D8; LIBSPU/S_SR@0x80089D60; LIBSPU/S_SRMP@0x80089F3C |
| _spu_rev_param | 0x800A2D94 | ? **<- differs** | LIBSPU/S_SRMP@0x80089F3C |
| _spu_rev_reserve_wa | 0x800A2880 | g_spu_voice_alloc_current_a0, D_800A2880 **<- differs** | LIBSPU/S_M_M@0x800893D8; LIBSPU/S_SR@0x80089D60 |
| _spu_rev_startaddr | 0x800A2D44 | g_spu_init_reg_D1_val, D_800A2D44 **<- differs** | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_INI@0x800885CC; LIBSPU/S_SRMP@0x80089F3C |
| _spu_transMode | 0x800A2CF8 | g_spu_reverb_mode, g_spu_reverb_mode, D_800A2CF8, g_spu_reverb_mode **<- differs** | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_INI@0x800885CC; LIBSPU/S_STM@0x8008AE7C |
| _spu_trans_mode | 0x800A2878 | g_snd_reverb_flag, g_snd_reverb_flag, D_800A2878, g_snd_reverb_flag **<- differs** | LIBSPU/S_ITC@0x8008AEB0; LIBSPU/S_STM@0x8008AE7C |
| _spu_transferCallback | 0x800A2D14 | g_spu_init_flag, g_spu_init_flag, _spu_transferCallback, g_spu_init_flag | LIBSPU/S_CRWA@0x8008A928; LIBSPU/S_Q@0x800892F8 |
| _spu_tsa | 0x800A2CF4 | g_spu_xfer_addr, g_spu_xfer_addr, D_800A2CF4, g_spu_xfer_addr **<- differs** | LIBSPU/S_STSA@0x8008AE24 |
| _spu_zerobuf | 0x800A28D4 | g_satan0_gauge_data_block_800A28D4, D_800A28D4 **<- differs** | LIBSPU/S_CRWA@0x8008A928 |
| _ss_score | 0x80106F28 | g_seq_bank_table, D_80106F28 **<- differs** | LIBSND/MIDITIME@0x80085064; LIBSND/NEXT@0x80085114; LIBSND/PAUSE@0x800848AC; LIBSND/REPLAY@0x80085210; LIBSND/SSCALL@0x8 |
| _svm_auto_kof_mode | 0x80103600 | g_memcard_data, g_memcard_data, D_80103600, g_memcard_data **<- differs** | LIBSND/VS_AUTO@0x80087F00 |
| _svm_cur | 0x801027F0 | D_801027F0 **<- differs** | LIBSND/VM_N2P@0x80086B38; LIBSND/VM_VSU@0x80087E3C |
| _svm_pg | 0x800FF6A0 | g_voice_state_ptr, D_800FF6A0 **<- differs** | LIBSND/VM_VSU@0x80087E3C |
| _svm_rattr | 0x800F5750 | g_buki_cmd_buf_id, D_800F5750 **<- differs** | LIBSND/UT_RDEP@0x80085E4C; LIBSND/UT_REV@0x80085EE4 |
| _svm_stereo_mono | 0x800FF578 | g_memcard_busy, g_memcard_busy, D_800FF578, g_memcard_busy **<- differs** | LIBSND/VS_MONO@0x80087F10 |
| _svm_tn | 0x80101BC8 | g_weapon_frame_data_base, D_80101BC8 **<- differs** | LIBSND/VM_N2P@0x80086B38; LIBSND/VM_VSU@0x80087E3C |
| _svm_vab_count | 0x80107808 | g_snd_ch_count, g_snd_ch_count, D_80107808, g_snd_ch_count **<- differs** | LIBSND/VS_VAB@0x80087F64; LIBSND/VS_VH@0x80088058 |
| _svm_vab_pg | 0x800F6660 | g_snd_ch_texture_base_ptr, D_800F6660 **<- differs** | LIBSND/VM_VSU@0x80087E3C; LIBSND/VS_VH@0x80088058 |
| _svm_vab_start | 0x80107810 | g_snd_ch_addr, g_snd_ch_addr, D_80107810, g_snd_ch_addr **<- differs** | LIBSND/UT_GVBA@0x800859F0; LIBSND/VS_VAB@0x80087F64; LIBSND/VS_VH@0x80088058; LIBSND/VS_VTB@0x800884C4 |
| _svm_vab_tn | 0x800F6700 | g_snd_ch_texture_meta_ptr, D_800F6700 **<- differs** | LIBSND/VM_VSU@0x80087E3C; LIBSND/VS_VH@0x80088058 |
| _svm_vab_total | 0x801077C8 | g_char_p21_field_310, D_801077C8 **<- differs** | LIBSND/VS_VH@0x80088058; LIBSND/VS_VTB@0x800884C4 |
| _svm_vab_used | 0x80102A68 | g_snd_ch_status, g_snd_ch_status, D_80102A68, g_snd_ch_status **<- differs** | LIBSND/UT_GVBA@0x800859F0; LIBSND/VM_VSU@0x80087E3C; LIBSND/VS_VAB@0x80087F64; LIBSND/VS_VFB@0x80087FE8; LIBSND/VS_VH@0x |
| _svm_vab_vh | 0x800F66B8 | g_snd_ch_texture_tpage, D_800F66B8 **<- differs** | LIBSND/VM_VSU@0x80087E3C; LIBSND/VS_VH@0x80088058 |
| _svm_vh | 0x80101BC4 | g_main_value_101BC4, D_80101BC4 **<- differs** | LIBSND/VM_VSU@0x80087E3C |
| kMaxPrograms | 0x800FF634 | g_snd_ch_texture_max_frame, D_800FF634 **<- differs** | LIBSND/VM_VSU@0x80087E3C; LIBSND/VS_VH@0x80088058 |
| rcossin_tbl | 0x8009C928 | g_trig_sin_cos_table_packed **<- differs** | LIBGTE/FGO_01@0x8007F35C; LIBGTE/FGO_03@0x8007F5EC; LIBGTE/FGO_04@0x8007F87C; LIBGTE/FGO_05@0x8007FA1C; LIBGTE/FGO_06@0x |
| rsin_tbl | 0x8009BF94 | g_sin_lut_q1, g_sin_lut_q1, g_sin_lut_q1 **<- differs** | LIBGTE/GEO_00@0x8007DF20; LIBGTE/GEO_01@0x8007DFEC |
