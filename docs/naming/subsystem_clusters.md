# Subsystem Clusters (unnamed functions)

For each unnamed function, look at its named callers and find the dominant subsystem prefix (e.g., callers all start with `cpu_*` -> the function is likely a `cpu` helper).

Total unnamed: 1152
In a cluster: 147
No cluster signal: 1005

## Cluster sizes

| subsystem | unnamed functions | rep names |
|---|---:|---|
| `cpu_*` | 22 | func_800164F8, func_80016A8C, func_80016E60, func_8001DA2C, func_8001E404... |
| `pad_*` | 18 | func_80037AA4, func_80037B00, func_80037B90, func_80037C34, func_80038148... |
| `mario_*` | 16 | func_80021210, func_800218C8, func_80021904, func_80021974, func_800219E4... |
| `motion_*` | 11 | func_80038734, func_8003879C, func_800387C0, func_800387E8, func_8006BEC4... |
| `satan_*` | 8 | func_8003E120, func_80048A7C, func_8004C1F4, func_8007352C, func_80078A68... |
| `replay_camera_*` | 8 | func_800355E8, func_80037110, func_8003ACB8, func_8003F388, func_80077940... |
| `efc_*` | 7 | func_800417D0, func_800433E4, func_80044F50, func_800455AC, func_8004876C... |
| `camera_*` | 7 | func_80052D00, func_8005E098, func_8005E51C, func_8005F1C8, func_8006E49C... |
| `obj_*` | 5 | func_80036F28, func_80085E4C, func_80085EE4, func_80085F98, func_80087F64... |
| `calc_*` | 5 | func_8001F860, func_800272FC, func_800274BC, func_8002A458, func_8002CA8C... |
| `disppracticemenutex_*` | 4 | func_8001A4F0, func_80020DDC, func_80033550, func_80044B30... |
| `single_game_*` | 4 | func_8001B138, func_8001FAE4, func_8001FB34, func_8003A728... |
| `se_*` | 4 | func_8002906C, func_8005BA8C, func_8005BD30, func_8005BDF0... |
| `special_camera_*` | 3 | func_800372F4, func_80080148, func_80080600... |
| `game_*` | 3 | func_80036D88, func_8003AA78, func_8003AAB0... |
| `gnd_*` | 3 | func_8005D46C, func_8005E54C, func_80060414... |
| `action_*` | 3 | func_8008A904, func_8008AAD4, func_8008BDE8... |
| `mk_*` | 2 | func_8001DB58, func_800325E0... |
| `md_game_*` | 2 | func_8002F770, func_80054F68... |
| `player_*` | 2 | func_8004016C, func_80045A50... |
| `hirahira_*` | 2 | func_80044DE4, func_800523E0... |
| `tsltm2loadimage_*` | 2 | func_8007997C, func_80080620... |

## Per-subsystem detail

### `cpu` (22 unnamed)

- `func_800164F8` called by ['cpu_set_move_command_and_dir_for_no_action_2', 'func_80016C3C', 'func_80044FA0']
- `func_80016A8C` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_80016E60` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_8001DA2C` called by ['cpu_get_move_pattern_table_number', 'func_8001EFA0', 'func_8003993C', 'func_8003B9D0', 'func_8003C9A4']
- `func_8001E404` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_8002006C` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_80020CDC` called by ['cpu_side_move_dir_2', 'func_80020D70', 'func_80035480', 'func_8003B9D0', 'func_8003BE10']
- `func_80022408` called by ['cpu_get_move_pattern_table_number', 'func_8001C8DC', 'func_8003C9A4', 'func_8003CE18']
- `func_80023F08` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_8002C61C` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_80030D7C` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_80032C50` called by ['cpu_check_same_dir_timer', 'func_8003339C']
- `func_800372C0` called by ['SetCurrentCursor', 'cpu_get_move_pattern_table_number', 'cpu_set_move_command_and_dir_for_no_action_2', 'func_80016E60', 'func_800342A0']
- `func_80039320` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_800397A0` called by ['cpu_get_move_pattern_table_number', 'func_8001E878', 'func_8001EFA0']
- `func_80054884` called by ['cpu_side_move_dir_2', 'func_8003B9D0', 'func_8003CE18']
- `func_800550E8` called by ['cpu_get_move_pattern_table_number']
- `func_8005C8A8` called by ['cpu_get_move_pattern_table_number', 'func_80016E60', 'func_8003C9A4']
- `func_8005FBC8` called by ['cpu_side_move_dir_2']
- `func_80078968` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_800789D8` called by ['cpu_set_move_command_and_dir_for_no_action_2']
- `func_8007BC08` called by ['cpu_set_move_command_and_dir_for_no_action_2', 'func_80016A8C', 'func_80016E60', 'func_8006E10C', 'func_8006E2A8']

### `pad` (18 unnamed)

- `func_80037AA4` called by ['pad_FuncAnalog']
- `func_80037B00` called by ['pad_FuncAnalog']
- `func_80037B90` called by ['pad_FuncAnalog']
- `func_80037C34` called by ['pad_FuncAnalog']
- `func_80038148` called by ['pad_FuncAnalog']
- `func_80038170` called by ['pad_FuncAnalog']
- `func_80078958` called by ['pad_press_control']
- `func_80078978` called by ['func_8003A174', 'func_800886C4', 'pad_press_control']
- `func_80078998` called by ['func_80037804', 'func_800378A8', 'func_8003791C', 'func_80037964', 'func_800379D8']
- `func_800789A8` called by ['func_8003A360', 'func_800886C4', 'pad_press_control']
- `func_80078A18` called by ['func_80016514', 'func_800165F8', 'func_80037C34', 'func_80038658', 'func_8003A264']
- `func_80078A58` called by ['func_80016A18', 'func_80078C00', 'func_80078C9C', 'func_80078D38', 'func_8007A370']
- `func_80078E20` called by ['pad_Init']
- `func_80078F20` called by ['pad_Init']
- `func_80078F74` called by ['pad_Init']
- `func_8007A370` called by ['pad_press_control']
- `func_8007A3C8` called by ['pad_press_control']
- `func_8008C464` called by ['func_8003A174', 'func_8003A264', 'func_8003A2DC', 'func_8003A308', 'func_8003A39C']

### `mario` (16 unnamed)

- `func_80021210` called by ['mario_test_Exec']
- `func_800218C8` called by ['func_8001C4C0', 'func_8001C51C', 'func_8001EEB4', 'mario_test_Exec']
- `func_80021904` called by ['func_8001C8DC', 'mario_test_Exec']
- `func_80021974` called by ['func_8001C4C0', 'func_8001C51C', 'mario_test_Exec']
- `func_800219E4` called by ['mario_test_Exec']
- `func_80022580` called by ['func_8001C51C', 'func_8003AF40', 'mario_test_Exec']
- `func_80022F34` called by ['func_8001C51C', 'mario_test_Exec']
- `func_8003043C` called by ['mario_test_Exec']
- `func_80032040` called by ['mario_test_Exec']
- `func_80033510` called by ['mario_test_Exec']
- `func_800392C8` called by ['func_8001C51C', 'mario_test_Exec']
- `func_8003FFE0` called by ['mario_test_Exec']
- `func_80040510` called by ['func_8001C8DC', 'func_8003AF40', 'mario_test_Exec']
- `func_800493E4` called by ['func_8003AFFC', 'func_8003B10C', 'mario_test_Exec']
- `func_800494D4` called by ['func_8003AFFC', 'func_8003B10C', 'mario_test_Exec']
- `func_80049584` called by ['func_8003AFFC', 'func_8003B10C', 'mario_test_Exec']

### `motion` (11 unnamed)

- `func_80038734` called by ['func_8003880C', 'func_80038988', 'motion_SetMotion']
- `func_8003879C` called by ['motion_SetMotion']
- `func_800387C0` called by ['motion_SetMotion']
- `func_800387E8` called by ['motion_SetMotion']
- `func_8006BEC4` called by ['func_8003880C', 'func_800388A8', 'func_80038988', 'motion_SetMotion']
- `func_8007E8AC` called by ['func_80063084', 'func_80073C78', 'motion_SetExMotion']
- `func_8007EA0C` called by ['func_800620B8', 'func_800646E8', 'motion_SetExMotion']
- `func_80082D34` called by ['motion_make_table']
- `func_800831D8` called by ['motion_make_table']
- `func_80083210` called by ['func_80082D34', 'motion_make_table']
- `func_80083220` called by ['motion_make_table']

### `satan` (8 unnamed)

- `func_8003E120` called by ['camera_SetMatrix_8003E164', 'func_80040510', 'func_80049584', 'saTan4FireDisp']
- `func_80048A7C` called by ['saTan4FireDisp']
- `func_8004C1F4` called by ['func_8004C1F4', 'saTan2LineDraw']
- `func_8007352C` called by ['func_8005C8A8', 'func_8005D814', 'func_8005E098', 'func_8005E54C', 'func_8005F1C8']
- `func_80078A68` called by ['func_80046AE8', 'saTan5TakeAnim2_2']
- `func_80084A7C` called by ['saTan0Main']
- `func_8008AF84` called by ['saTan2Main']
- `func_8008BB24` called by ['saTan1MainJump']

### `replay_camera` (8 unnamed)

- `func_800355E8` called by ['func_80035828', 'replay_camera_rob_back_loose2']
- `func_80037110` called by ['camera_SetMatrix_8001DA8C', 'func_800355E8', 'func_80035828', 'func_800371AC', 'replay_camera_rob_back_loose2']
- `func_8003ACB8` called by ['replay_camera_rob_back_loose2']
- `func_8003F388` called by ['replay_camera_get_attack_number']
- `func_80077940` called by ['func_80035828', 'replay_camera_rob_back_loose2']
- `func_80077984` called by ['func_80035828', 'replay_camera_rob_back_loose2']
- `func_80077A28` called by ['replay_camera_rob_back_loose2']
- `func_80077A80` called by ['replay_camera_rob_back_loose2']

### `efc` (7 unnamed)

- `func_800417D0` called by ['efc_rob_set_type_flash', 'func_80040D48', 'func_800417D0', 'func_800477E8', 'func_80049A2C']
- `func_800433E4` called by ['efc_rob_set_type_particle']
- `func_80044F50` called by ['efc_rob_set_type_particle']
- `func_800455AC` called by ['efc_rob_set_type_particle', 'func_80045878', 'func_800460E4', 'func_800467B8', 'func_800469C4']
- `func_8004876C` called by ['efc_buki_draw_zanzou', 'func_800484A0']
- `func_80049C24` called by ['efc_rob_set_type_particle']
- `func_8006920C` called by ['efc_buki_ZanzouClose', 'efc_rob_Close', 'func_80076FF8', 'func_80077D10']

### `camera` (7 unnamed)

- `func_80052D00` called by ['camera_check_inside_screen_rob_dpos', 'func_80053304', 'func_8005344C', 'func_80053584']
- `func_8005E098` called by ['camera_set_target_zoom', 'func_8005E51C']
- `func_8005E51C` called by ['camera_set_target_zoom']
- `func_8005F1C8` called by ['camera_set_target_zoom']
- `func_8006E49C` called by ['camera_get_rot_normal_rad', 'func_80068F70', 'func_8006E534', 'func_800770B8', 'func_80078824']
- `func_8006E950` called by ['camera_get_rot_normal_rad', 'func_80068F70', 'func_8006E534', 'func_800770B8', 'func_80078824']
- `func_80077D10` called by ['camera_get_rot_normal_rad']

### `obj` (5 unnamed)

- `func_80036F28` called by ['func_8005B7C4', 'func_8005B8B8', 'func_8005BA8C', 'func_8005FBC8', 'func_800602AC']
- `func_80085E4C` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80085EE4` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80085F98` called by ['func_8005B43C', 'get_point_value', 'mario_getMarioVoiceData_8005BE84', 'obj_InitAll', 'obj_Reset']
- `func_80087F64` called by ['GetAllocPacketSize', 'func_8005B6AC', 'func_8005B6FC', 'func_8005BA8C', 'func_8005BDF0']

### `calc` (5 unnamed)

- `func_8001F860` called by ['calc_teasi_loc_fw', 'func_80023648', 'func_80023F08', 'func_8002C0DC']
- `func_800272FC` called by ['calc_teasi_loc_fw']
- `func_800274BC` called by ['calc_loc_mat_fw', 'func_80031B24']
- `func_8002A458` called by ['calc_loc_mat_fw']
- `func_8002CA8C` called by ['calc_loc_mat_fw']

### `disppracticemenutex` (4 unnamed)

- `func_8001A4F0` called by ['DispPracticeMenuTex_A', 'func_8001A820']
- `func_80020DDC` called by ['DispPracticeMenuTex_B']
- `func_80033550` called by ['DispPracticeMenuTex_C']
- `func_80044B30` called by ['DispPracticeMenuTex_C']

### `single_game` (4 unnamed)

- `func_8001B138` called by ['single_game_VoiceContorol']
- `func_8001FAE4` called by ['single_game_CheckStatusUpDataTotalOver']
- `func_8001FB34` called by ['single_game_CheckStatusUpDataTotalOver']
- `func_8003A728` called by ['func_8003AA48', 'single_game_VoiceContorol']

### `se` (4 unnamed)

- `func_8002906C` called by ['se_data_set']
- `func_8005BA8C` called by ['se_data_set']
- `func_8005BD30` called by ['se_data_set']
- `func_8005BDF0` called by ['se_data_set']

### `special_camera` (3 unnamed)

- `func_800372F4` called by ['func_80037348', 'special_camera_get_rot_dir']
- `func_80080148` called by ['func_80036034', 'game_FrameInit', 'saEft00Add', 'special_camera_Exec', 'special_camera_set_win_cam']
- `func_80080600` called by ['func_80035F30', 'special_camera_set_win_cam']

### `game` (3 unnamed)

- `func_80036D88` called by ['game_FrameLoop']
- `func_8003AA78` called by ['camera_SetMatrix_8001DBE4', 'game_FrameLoop']
- `func_8003AAB0` called by ['camera_SetMatrix_8001DBE4', 'game_FrameLoop']

### `gnd` (3 unnamed)

- `func_8005D46C` called by ['gnd_disp_loop_ctrl']
- `func_8005E54C` called by ['func_8003C560', 'gnd_disp_loop_ctrl']
- `func_80060414` called by ['gnd_disp_loop_ctrl']

### `action` (3 unnamed)

- `func_8008A904` called by ['action_CheckHitZangeki']
- `func_8008AAD4` called by ['action_CheckHitZangeki']
- `func_8008BDE8` called by ['action_CheckHitZangeki']

