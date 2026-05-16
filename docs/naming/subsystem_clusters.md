# Subsystem Clusters (unnamed functions)

For each unnamed function, look at its named callers and find the dominant subsystem prefix (e.g., callers all start with `cpu_*` -> the function is likely a `cpu` helper).

Total unnamed: 1152
In a cluster: 108
No cluster signal: 1044

## Cluster sizes

| subsystem | unnamed functions | rep names |
|---|---:|---|
| `pad_*` | 12 | func_80037AA4, func_80037B00, func_80037B90, func_80037C34, func_80038148... |
| `calc_*` | 10 | func_800272FC, func_8002A458, func_8002CA8C, func_800430E4, func_80047BE0... |
| `motion_*` | 10 | func_8003879C, func_800387C0, func_800387E8, func_8005B8B8, func_8005C650... |
| `satan_*` | 10 | func_80048A7C, func_8005B6AC, func_8006C168, func_8006D74C, func_8006E068... |
| `cpu_*` | 8 | func_80016A8C, func_80016E60, func_800372C0, func_800550E8, func_8005FBC8... |
| `mario_*` | 7 | func_80021210, func_800219E4, func_8003043C, func_80032040, func_80033510... |
| `single_game_*` | 4 | func_8001B138, func_8001FAE4, func_8001FB34, func_80032854... |
| `se_*` | 4 | func_8002906C, func_8005BA8C, func_8005BD30, func_8005BDF0... |
| `replay_camera_*` | 4 | func_8003ACB8, func_8003F388, func_80077A28, func_80077A80... |
| `efc_*` | 4 | func_800433E4, func_80044F50, func_80049C24, func_8006920C... |
| `gnd_*` | 4 | func_800486FC, func_8004881C, func_8005D46C, func_80060414... |
| `tslglobalmemfree_*` | 4 | func_8005C074, func_8005C5A8, func_800859F0, func_80088584... |
| `obj_*` | 4 | func_80085E4C, func_80085EE4, func_80085F98, func_80087F64... |
| `disppracticemenutex_*` | 3 | func_80020DDC, func_80033550, func_80044B30... |
| `camera_*` | 3 | func_8005E51C, func_8005F1C8, func_80077D10... |
| `action_*` | 3 | func_8008A904, func_8008AAD4, func_8008BDE8... |
| `ang_*` | 2 | func_8001924C, func_8008386C... |
| `marionation_*` | 2 | func_80037774, func_80078948... |
| `hirahira_*` | 2 | func_80044DE4, func_800523E0... |

## Per-subsystem detail

### `pad` (12 unnamed)

- `func_80037AA4` called by ['pad_FuncAnalog']
- `func_80037B00` called by ['pad_FuncAnalog']
- `func_80037B90` called by ['pad_FuncAnalog']
- `func_80037C34` called by ['pad_FuncAnalog']
- `func_80038148` called by ['pad_FuncAnalog']
- `func_80038170` called by ['pad_FuncAnalog']
- `func_80078958` called by ['pad_press_control']
- `func_80078E20` called by ['pad_Init']
- `func_80078F20` called by ['pad_Init']
- `func_80078F74` called by ['pad_Init']
- `func_8007A370` called by ['pad_press_control']
- `func_8007A3C8` called by ['pad_press_control']

### `calc` (10 unnamed)

- `func_800272FC` called by ['calc_teasi_loc_fw']
- `func_8002A458` called by ['calc_loc_mat_fw']
- `func_8002CA8C` called by ['calc_loc_mat_fw']
- `func_800430E4` called by ['calc_loc_mat_fw_8004A940']
- `func_80047BE0` called by ['calc_loc_mat_fw_8004A940']
- `func_8004C404` called by ['calc_loc_mat_fw_8004A940']
- `func_8004DA74` called by ['calc_loc_mat_fw_8004A940']
- `func_80055948` called by ['calc_loc_mat_fw_80055B60']
- `func_80056CB8` called by ['calc_loc_mat_fw_80055B60']
- `func_80058580` called by ['calc_loc_mat_fw_80055B60']

### `motion` (10 unnamed)

- `func_8003879C` called by ['motion_SetMotion']
- `func_800387C0` called by ['motion_SetMotion']
- `func_800387E8` called by ['motion_SetMotion']
- `func_8005B8B8` called by ['motion_Close_8001D998']
- `func_8005C650` called by ['camera_set_target_zoom', 'cpu_get_move_pattern_table_number', 'func_80016E60', 'func_800325E0', 'func_80034708']
- `func_80078A28` called by ['motion_LoadPreCalcData_80037F08']
- `func_8008008C` called by ['DispUpdateStatusMessage', 'SetPacketData', 'cpu_side_move_dir_3', 'func_8008C464', 'motion_SavePreCalcData_80080014']
- `func_80082D34` called by ['motion_make_table']
- `func_800831D8` called by ['motion_make_table']
- `func_80083220` called by ['motion_make_table']

### `satan` (10 unnamed)

- `func_80048A7C` called by ['saTan4FireDisp']
- `func_8005B6AC` called by ['saTanMainDispGnd_80046020']
- `func_8006C168` called by ['saTan2InfoInit_8006C1FC']
- `func_8006D74C` called by ['saTanMainDispGnd_80077A04']
- `func_8006E068` called by ['saTan2InfoInit_80077A60']
- `func_8007352C` called by ['func_8005C8A8', 'func_8005D814', 'func_8005E098', 'func_8005E54C', 'func_8005F1C8']
- `func_80077724` called by ['saTan2InfoInit_80077AC0']
- `func_80084A7C` called by ['saTan0Main']
- `func_8008AF84` called by ['saTan2Main']
- `func_8008BB24` called by ['saTan1MainJump']

### `cpu` (8 unnamed)

- `func_80016A8C` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_80016E60` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_800372C0` called by ['SetCurrentCursor', 'cpu_get_move_pattern_table_number', 'cpu_set_move_command_and_dir_for_no_action_2', 'func_80016E60', 'func_800342A0']
- `func_800550E8` called by ['cpu_get_move_pattern_table_number']
- `func_8005FBC8` called by ['cpu_side_move_dir_2']
- `func_80078968` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_800789D8` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_80079154` called by ['calc_loc_mat_fw_80055B60', 'cpu_set_move_command_and_dir_for_no_action', 'cpu_set_move_command_and_dir_for_no_action_2', 'func_80021DB0', 'func_80022224']

### `mario` (7 unnamed)

- `func_80021210` called by ['mario_test_Exec']
- `func_800219E4` called by ['mario_test_Exec']
- `func_8003043C` called by ['mario_test_Exec']
- `func_80032040` called by ['mario_test_Exec']
- `func_80033510` called by ['mario_test_Exec']
- `func_8003FFE0` called by ['mario_test_Exec']
- `func_80085FB8` called by ['mario_getMarioVoiceData_8005BE84']

### `single_game` (4 unnamed)

- `func_8001B138` called by ['single_game_VoiceContorol']
- `func_8001FAE4` called by ['single_game_CheckStatusUpDataTotalOver']
- `func_8001FB34` called by ['single_game_CheckStatusUpDataTotalOver']
- `func_80032854` called by ['calc_loc_mat_fw', 'calc_teasi_loc_fw', 'cpu_check_tubazeri_2', 'cpu_side_move_dir', 'func_80023F08']

### `se` (4 unnamed)

- `func_8002906C` called by ['se_data_set']
- `func_8005BA8C` called by ['se_data_set']
- `func_8005BD30` called by ['se_data_set']
- `func_8005BDF0` called by ['se_data_set']

### `replay_camera` (4 unnamed)

- `func_8003ACB8` called by ['replay_camera_rob_back_loose2']
- `func_8003F388` called by ['replay_camera_get_attack_number']
- `func_80077A28` called by ['replay_camera_rob_back_loose2']
- `func_80077A80` called by ['replay_camera_rob_back_loose2']

### `efc` (4 unnamed)

- `func_800433E4` called by ['efc_rob_set_type_particle']
- `func_80044F50` called by ['efc_rob_set_type_particle']
- `func_80049C24` called by ['efc_rob_set_type_particle']
- `func_8006920C` called by ['efc_buki_ZanzouClose', 'efc_rob_Close', 'func_80076FF8', 'func_80077D10']

### `gnd` (4 unnamed)

- `func_800486FC` called by ['efc_buki_draw_zanzou', 'func_80042478', 'func_800467B8', 'func_800484A0', 'gnd_init_80041688']
- `func_8004881C` called by ['func_80042478', 'gnd_init_80041688', 'gnd_land_hit_char_die_main', 'saTan4FireDisp']
- `func_8005D46C` called by ['gnd_disp_loop_ctrl']
- `func_80060414` called by ['gnd_disp_loop_ctrl']

### `tslglobalmemfree` (4 unnamed)

- `func_8005C074` called by ['tslGlobalMemFree_8005C2A8']
- `func_8005C5A8` called by ['tslGlobalMemFree_8005C2A8']
- `func_800859F0` called by ['tslGlobalMemFree_8005C2A8']
- `func_80088584` called by ['tslGlobalMemFree_8005C2A8']

### `obj` (4 unnamed)

- `func_80085E4C` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80085EE4` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80085F98` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80087F64` called by ['GetAllocPacketSize', 'func_8005B6AC', 'func_8005B6FC', 'func_8005BA8C', 'func_8005BDF0']

### `disppracticemenutex` (3 unnamed)

- `func_80020DDC` called by ['DispPracticeMenuTex_B']
- `func_80033550` called by ['DispPracticeMenuTex_C']
- `func_80044B30` called by ['DispPracticeMenuTex_C']

### `camera` (3 unnamed)

- `func_8005E51C` called by ['camera_set_target_zoom']
- `func_8005F1C8` called by ['camera_set_target_zoom']
- `func_80077D10` called by ['camera_get_rot_normal_rad']

### `action` (3 unnamed)

- `func_8008A904` called by ['action_CheckHitZangeki']
- `func_8008AAD4` called by ['action_CheckHitZangeki']
- `func_8008BDE8` called by ['action_CheckHitZangeki']

