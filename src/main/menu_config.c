/* menu_PrintEditAndApplyConfigWithPerPadCursors alone. .text 0x80034708 (ROM
 * 0x24F08). Start boundary: G8: compiled -G8, as the gp-relative reads of the
 * cursor array g_config_menu_cursors show; so the <=8-byte strings sit in
 * .sdata; its .rodata is only its two 12-entry switch jump tables. */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

enum ConfigMenuRow {
    CONFIG_MENU_ROW_ROSTER = 0,
    CONFIG_MENU_ROW_WEAPON = 1,
    CONFIG_MENU_ROW_CPU_CONTROLLED = 2,
    CONFIG_MENU_ROW_CPU_BASE_LEVEL = 3,
    CONFIG_MENU_ROW_STAGE = 4,
    CONFIG_MENU_ROW_MODE = 5,
    CONFIG_MENU_ROW_VIEW_SETTING = 6,
    CONFIG_MENU_ROW_FORCE_WEAPON_INNER_BONE_HITS = 7,
    CONFIG_MENU_ROW_SAVED_GRAYSCALE = 8,
    CONFIG_MENU_ROW_SAVED_CPU_BEHAVIOR_ADJUST = 9,
    CONFIG_MENU_ROW_M_OPTION = 10,
    CONFIG_MENU_ROW_FIGHT_VIEW_OPTION = 11,
};

/* This menu's two cursors ([0] = P1 row 0..11, [1] = P2 row 0..3) and its
 * colour / format strings. */
extern s16 g_config_menu_cursors[2];
extern u8 g_config_menu_unselected_color_code[];
extern u8 g_config_menu_selected_color_code[];
extern u8 g_config_menu_left_roster_and_weapon_fmt[];
extern u8 g_config_menu_right_pad_value_fmt[];
extern u8 g_config_menu_left_cpu_controlled_fmt[];
extern u8 g_config_menu_left_cpu_base_level_fmt[];
extern u8 g_config_menu_right_cpu_base_level_fmt[];
extern u8 g_config_menu_pending_stage_and_mode_fmt[];
extern u8 g_config_menu_pending_view_setting_fmt[];
extern u8 g_config_menu_saved_cpu_behavior_adjust_fmt[];
extern u8 g_config_menu_m_option_fmt[];
extern u8 g_config_menu_fight_view_option_fmt[];
extern const char g_config_menu_force_weapon_inner_bone_hits_fmt[];
extern const char g_config_menu_saved_grayscale_fmt[];

void menu_PrintEditAndApplyConfigWithPerPadCursors(void) {
    s32 i;
    u8 *unselected_prefix;
    u8 *selected_prefix;

    g_handler_frame_counter++;
    rand();
    unselected_prefix = g_config_menu_unselected_color_code;
    selected_prefix = g_config_menu_selected_color_code;
    font_Printf(g_config_menu_left_roster_and_weapon_fmt,
                (s32)(g_config_menu_cursors[0] == 0 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.fighter_setup[0]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == CONFIG_MENU_ROW_ROSTER
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.fighter_setup[1]);
    font_Printf(g_config_menu_left_roster_and_weapon_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_WEAPON
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.fighter_setup[2]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == 1 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.fighter_setup[3]);
    font_Printf(g_config_menu_left_cpu_controlled_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_CPU_CONTROLLED
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.fighter_setup[4]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == CONFIG_MENU_ROW_CPU_CONTROLLED
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.fighter_setup[5]);
    font_Printf(g_config_menu_left_cpu_base_level_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_CPU_BASE_LEVEL
                          ? selected_prefix
                          : unselected_prefix),
                g_fight_config.cpu_base_levels[0] >> 8);
    font_Printf(g_config_menu_right_cpu_base_level_fmt,
                (s32)(g_config_menu_cursors[1] == CONFIG_MENU_ROW_CPU_BASE_LEVEL
                          ? selected_prefix
                          : unselected_prefix),
                g_fight_config.cpu_base_levels[1] >> 8);
    font_Printf(g_config_menu_pending_stage_and_mode_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_STAGE
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.pending_stage_index);
    font_Printf(g_config_menu_pending_stage_and_mode_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_MODE
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.pending_mode_selector);
    font_Printf(g_config_menu_pending_view_setting_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_VIEW_SETTING
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.pending_view_setting);
    font_Printf((u8 *)g_config_menu_force_weapon_inner_bone_hits_fmt,
                (s32)(g_config_menu_cursors[0] ==
                              CONFIG_MENU_ROW_FORCE_WEAPON_INNER_BONE_HITS
                          ? selected_prefix
                          : unselected_prefix),
                (s8)g_fight_config.pending_force_weapon_inner_bone_hits);
    font_Printf(
        (u8 *)g_config_menu_saved_grayscale_fmt,
        (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_SAVED_GRAYSCALE
                  ? selected_prefix
                  : unselected_prefix),
        g_saved_record.flags & 1);
    font_Printf(g_config_menu_saved_cpu_behavior_adjust_fmt,
                (s32)(g_config_menu_cursors[0] ==
                              CONFIG_MENU_ROW_SAVED_CPU_BEHAVIOR_ADJUST
                          ? selected_prefix
                          : unselected_prefix),
                (g_saved_record.flags >> 1) & 1);
    font_Printf(g_config_menu_m_option_fmt,
                (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_M_OPTION
                          ? selected_prefix
                          : unselected_prefix),
                g_config_menu_m_option);
    font_Printf(
        g_config_menu_fight_view_option_fmt,
        (s32)(g_config_menu_cursors[0] == CONFIG_MENU_ROW_FIGHT_VIEW_OPTION
                  ? selected_prefix
                  : unselected_prefix),
        g_fight_view_option);

    for (i = 0; i < 2; i++) {
        if (g_pad_state.pressed & (PADLup << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_config_menu_cursors[i] > 0) {
                g_config_menu_cursors[i]--;
            } else {
                g_config_menu_cursors[i] = (i != 0) ? 3 : 11;
            }
        } else if (g_pad_state.pressed & (PADLdown << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_config_menu_cursors[i] < ((i != 0) ? 3 : 11)) {
                g_config_menu_cursors[i]++;
            } else {
                g_config_menu_cursors[i] = 0;
            }
        } else if (g_pad_state.pressed & (PADLleft << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (g_config_menu_cursors[i]) {
            case CONFIG_MENU_ROW_ROSTER:
                g_fight_config.fighter_setup[i]--;
                break;
            case CONFIG_MENU_ROW_WEAPON:
                g_fight_config.fighter_setup[2 + i]--;
                break;
            case CONFIG_MENU_ROW_CPU_CONTROLLED:
                g_fight_config.fighter_setup[4 + i]--;
                break;
            case CONFIG_MENU_ROW_CPU_BASE_LEVEL:
                g_fight_config.cpu_base_levels[i] -= 0x80;
                break;
            case CONFIG_MENU_ROW_STAGE:
                g_fight_config.pending_stage_index--;
                break;
            case CONFIG_MENU_ROW_MODE:
                g_fight_config.pending_mode_selector--;
                break;
            case CONFIG_MENU_ROW_VIEW_SETTING:
                g_fight_config.pending_view_setting--;
                break;
            case CONFIG_MENU_ROW_FORCE_WEAPON_INNER_BONE_HITS:
                g_fight_config.pending_force_weapon_inner_bone_hits--;
                break;
            case CONFIG_MENU_ROW_SAVED_GRAYSCALE:
                g_saved_record.flags ^= 1;
                break;
            case CONFIG_MENU_ROW_SAVED_CPU_BEHAVIOR_ADJUST:
                g_saved_record.flags ^= 2;
                break;
            case CONFIG_MENU_ROW_M_OPTION:
                g_config_menu_m_option--;
                break;
            case CONFIG_MENU_ROW_FIGHT_VIEW_OPTION:
                g_fight_view_option--;
                break;
            }
        } else if (g_pad_state.pressed & (PADLright << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (g_config_menu_cursors[i]) {
            case CONFIG_MENU_ROW_ROSTER:
                g_fight_config.fighter_setup[i]++;
                break;
            case CONFIG_MENU_ROW_WEAPON:
                g_fight_config.fighter_setup[2 + i]++;
                break;
            case CONFIG_MENU_ROW_CPU_CONTROLLED:
                g_fight_config.fighter_setup[4 + i]++;
                break;
            case CONFIG_MENU_ROW_CPU_BASE_LEVEL:
                g_fight_config.cpu_base_levels[i] += 0x80;
                break;
            case CONFIG_MENU_ROW_STAGE:
                g_fight_config.pending_stage_index++;
                break;
            case CONFIG_MENU_ROW_MODE:
                g_fight_config.pending_mode_selector++;
                break;
            case CONFIG_MENU_ROW_VIEW_SETTING:
                g_fight_config.pending_view_setting++;
                break;
            case CONFIG_MENU_ROW_FORCE_WEAPON_INNER_BONE_HITS:
                g_fight_config.pending_force_weapon_inner_bone_hits++;
                break;
            case CONFIG_MENU_ROW_SAVED_GRAYSCALE:
                g_saved_record.flags ^= 1;
                break;
            case CONFIG_MENU_ROW_SAVED_CPU_BEHAVIOR_ADJUST:
                g_saved_record.flags ^= 2;
                break;
            case CONFIG_MENU_ROW_M_OPTION:
                g_config_menu_m_option++;
                break;
            case CONFIG_MENU_ROW_FIGHT_VIEW_OPTION:
                g_fight_view_option++;
                break;
            }
        }
        g_fight_config.fighter_setup[i] =
            ((s8)g_fight_config.fighter_setup[i] + 33) % 33;
        g_fight_config.fighter_setup[2 + i] =
            ((s8)g_fight_config.fighter_setup[2 + i] + 8) % 8;
        g_fight_config.fighter_setup[4 + i] =
            ((s8)g_fight_config.fighter_setup[4 + i] + 2) % 2;
    }
    g_fight_config.pending_stage_index =
        ((s8)g_fight_config.pending_stage_index + 38) % 38;
    g_fight_config.pending_mode_selector =
        ((s8)g_fight_config.pending_mode_selector + 7) % 7;
    g_fight_config.pending_view_setting &= 1;
    g_fight_config.pending_force_weapon_inner_bone_hits &= 1;
    g_config_menu_m_option = (g_config_menu_m_option + 4) % 4;
    g_fight_view_option &= 1;
    if (g_pad_state.pressed & 0x08000800) {
        snd_QueueSound(1, 0x7F, 0x7F);
        ApplyPendingConfigAndInitializeMode();
    }
}

/* Q65: this file's initialized small data (.sdata, 0x800A3174..0x800A31D8),
 * in address order; values from the original EXE. */
s16 g_config_menu_cursors[2] = {0, 0};
u8 g_config_menu_unselected_color_code[] = "~c777";
u8 g_config_menu_selected_color_code[] = "~c737";
u8 g_config_menu_left_roster_and_weapon_fmt[] = "%s%5d";
u8 g_config_menu_right_pad_value_fmt[] = "%s%3d\n";
u8 g_config_menu_left_cpu_controlled_fmt[] = "%sC%4d";
u8 g_config_menu_left_cpu_base_level_fmt[] = "%sCL%3x";
u8 g_config_menu_right_cpu_base_level_fmt[] = "%s%3x\n";
u8 g_config_menu_pending_stage_and_mode_fmt[] = "%s%5d\n";
u8 g_config_menu_pending_view_setting_fmt[] = "%sO%4d\n";
u8 g_config_menu_saved_cpu_behavior_adjust_fmt[] = "%sL%4d\n";
u8 g_config_menu_m_option_fmt[] = "%sM%4d\n";
u8 g_config_menu_fight_view_option_fmt[] = "%sD%4d\n";
