/* ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests and
 * ApplyPendingConfigAndInitializeMode. .text 0x800343F0 (ROM 0x24BF0). Start
 * boundary: PHASE (rodata-align site 3), placed by the per-file gp model (Q65).
 */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests lives in this file
 * because the file boundary follows the per-file gp evidence (owner ruling
 * Q65). */
void ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests(void) {
    s8 val_85 = (s8)g_fight_config.pending_mode_selector;
    s8 val_86 = (s8)g_fight_config.pending_view_setting;
    s8 val_84 = (s8)g_fight_config.pending_stage_index;
    s32 val_87 = (s8)g_fight_config.unk_F;

    g_fight_tracked_fighter_index = 0;
    g_fight_mode_selector = val_85;
    g_fight_tracked_view_setting = val_86;
    D_800A3140 = val_87;
    g_fight_stage_index = val_84;
    slot_RequestTimGroupsAndMarkReloadOnChange(0, 0);
    slot_RequestTimGroupsAndMarkReloadOnChange(1, 0);
    g_fight_bone_group_tim_requests = 0;
    D_800A376B = 0;
    g_fight_menu_restart_count = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}

void ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests(void);

const char g_str_chanbara[] = "CHANBARA";
const char g_str_practice[] = "PRACTICE";
const char D_80010834[] = "%sCR%3d\n";
const char g_config_menu_saved_grayscale_fmt[] = "%sCO%3d\n";

void ApplyPendingConfigAndInitializeMode(void) {
    ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests();

    switch (g_fight_mode_selector) {
    case 6:
        g_fight_config.pending_view_setting = 1;
        g_local_ot_draw_mode = 1;
        g_main_loop_handler_index = 0;
        g_fight_tracked_fighter_index = (g_comb_local_side != 0);
        break;
    case 0:
        fight_InitCommandStream(
            g_fight_config_id_data_rows[(s8)g_fight_config.unk_4[0]]);
        g_fight_config.unk_4[1] = 0;
        fight_RunCommandStream();
        fighter_ClearCpuTally(1);
        break;
    case 1:
        g_fight_config.unk_4[5] = 1;
        fight_BuildOpponentOrder();
        g_local_ot_draw_mode = 1;
        fight_AdvanceOpponentOrder();
        break;
    case 3:
        g_fight_config.unk_4[5] = 1;
        g_fight_config.unk_4[3] = 0;
        g_fight_config.unk_4[2] = 0;
        g_fight_step_table_next_row = 0;
        g_fight_start_position_pair_index = 0;
        g_fight_timer_frames = 0;
        g_fight_start_position_pair_changed = 0;
        g_fight_stage_index = 0x22;
        fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags();
        g_local_ot_draw_mode = 1;
        g_main_loop_handler_index = 0;
        break;
    case 5:
        g_fight_stage_index = (D_800A38E1 & 1) ? 0x21 : 0x20;
        g_fight_match_cursor = 0;
        gpu_ResetGraphMode1();
        eff_Init();
        fight_AdvanceMatch();
        break;
    case 2:
        g_fight_auto_pause_countdown = (D_800A389A == 0) << 1;
        g_fight_stage_index = (D_800A389A != 0) ? 0x24 : 0x23;
        g_fight_config.unk_4[5] = 1;
        if (D_800A389A == 0) {
            g_fight_config.unk_4[1] =
                g_fighter_group_flags[g_fight_config_id_data_rows
                                          [(s8)g_fight_config.unk_4[0]]]
                    ? 0xE
                    : 0x1D;
        }
        g_main_loop_handler_index = 0;
        break;
    case 4:
        g_local_ot_draw_mode = 1;
        g_main_loop_handler_index = 0;
        break;
    default:
        g_main_loop_handler_index = 0;
        break;
    }
    if ((s8)g_fight_config.unk_4[5] != 0) {
        fighter_ClearCpuTally(1);
    }
}
