# Naming Proposals -- Medium confidence

**Medium confidence**: weak Kengo match (size-diff > 1 but same name), single named caller (`sole_caller_path` proposal: `<caller>_helper_<addr>`), or call-graph subsystem cluster. These need callsite/body inspection before applying.

Total Medium: **101**

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

## Primary evidence: `gte_op` (2)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x8002A458` | `func_8002A458` | `calc_loc_mat_fw_helper_8002A458` | gte_op=gte_helper_8002A458; sole_caller_path=calc_loc_mat_fw_helper_8002A458 | [md](evidence/func_8002A458.md) |
| `0x800523E0` | `func_800523E0` | `hirahira_w_ctrl_helper_800523E0` | gte_op=gte_helper_800523E0; sole_caller_path=hirahira_w_ctrl_helper_800523E0 | [md](evidence/func_800523E0.md) |

