/* menu_PrintEditAndApplyConfigWithPerPadCursors alone. .text 0x80034708 (ROM
 * 0x24F08). Start boundary: G8: compiled -G8, as the gp-relative reads of the
 * cursor array g_config_menu_cursors show; so the <=8-byte strings sit in
 * .sdata; its .rodata is only its two 12-entry switch jump tables. */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* This menu's two cursors ([0] = P1 row 0..11, [1] = P2 row 0..3) and its
 * colour / format strings. */
extern s16 g_config_menu_cursors[2];
extern u8 g_config_menu_unselected_color_code[];
extern u8 g_config_menu_selected_color_code[];
extern u8 D_800A3188[];
extern u8 g_config_menu_right_pad_value_fmt[];
extern u8 D_800A3198[];
extern u8 g_config_menu_left_cpu_base_level_fmt[];
extern u8 g_config_menu_right_cpu_base_level_fmt[];
extern u8 g_config_menu_pending_stage_and_mode_fmt[];
extern u8 g_config_menu_pending_view_setting_fmt[];
extern u8 g_config_menu_saved_cpu_behavior_adjust_fmt[];
extern u8 D_800A31C8[];
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
    font_Printf(D_800A3188,
                (s32)(g_config_menu_cursors[0] == 0 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[0]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == 0 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[1]);
    font_Printf(D_800A3188,
                (s32)(g_config_menu_cursors[0] == 1 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[2]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == 1 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[3]);
    font_Printf(D_800A3198,
                (s32)(g_config_menu_cursors[0] == 2 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[4]);
    font_Printf(g_config_menu_right_pad_value_fmt,
                (s32)(g_config_menu_cursors[1] == 2 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_4[5]);
    font_Printf(g_config_menu_left_cpu_base_level_fmt,
                (s32)(g_config_menu_cursors[0] == 3 ? selected_prefix
                                                    : unselected_prefix),
                g_fight_config.cpu_base_levels[0] >> 8);
    font_Printf(g_config_menu_right_cpu_base_level_fmt,
                (s32)(g_config_menu_cursors[1] == 3 ? selected_prefix
                                                    : unselected_prefix),
                g_fight_config.cpu_base_levels[1] >> 8);
    font_Printf(g_config_menu_pending_stage_and_mode_fmt,
                (s32)(g_config_menu_cursors[0] == 4 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.pending_stage_index);
    font_Printf(g_config_menu_pending_stage_and_mode_fmt,
                (s32)(g_config_menu_cursors[0] == 5 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.pending_mode_selector);
    font_Printf(g_config_menu_pending_view_setting_fmt,
                (s32)(g_config_menu_cursors[0] == 6 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.pending_view_setting);
    font_Printf((u8 *)g_config_menu_force_weapon_inner_bone_hits_fmt,
                (s32)(g_config_menu_cursors[0] == 7 ? selected_prefix
                                                    : unselected_prefix),
                (s8)g_fight_config.unk_F);
    font_Printf((u8 *)g_config_menu_saved_grayscale_fmt,
                (s32)(g_config_menu_cursors[0] == 8 ? selected_prefix
                                                    : unselected_prefix),
                g_saved_record.flags & 1);
    font_Printf(g_config_menu_saved_cpu_behavior_adjust_fmt,
                (s32)(g_config_menu_cursors[0] == 9 ? selected_prefix
                                                    : unselected_prefix),
                (g_saved_record.flags >> 1) & 1);
    font_Printf(D_800A31C8,
                (s32)(g_config_menu_cursors[0] == 10 ? selected_prefix
                                                     : unselected_prefix),
                D_800A36F9);
    font_Printf(g_config_menu_fight_view_option_fmt,
                (s32)(g_config_menu_cursors[0] == 11 ? selected_prefix
                                                     : unselected_prefix),
                g_fight_view_option);

    for (i = 0; i < 2; i++) {
        if (g_pad_state.pressed & (0x1000 << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_config_menu_cursors[i] > 0) {
                g_config_menu_cursors[i]--;
            } else {
                g_config_menu_cursors[i] = (i != 0) ? 3 : 11;
            }
        } else if (g_pad_state.pressed & (0x4000 << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_config_menu_cursors[i] < ((i != 0) ? 3 : 11)) {
                g_config_menu_cursors[i]++;
            } else {
                g_config_menu_cursors[i] = 0;
            }
        } else if (g_pad_state.pressed & (0x8000 << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (g_config_menu_cursors[i]) {
            case 0:
                g_fight_config.unk_4[i]--;
                break;
            case 1:
                g_fight_config.unk_4[2 + i]--;
                break;
            case 2:
                g_fight_config.unk_4[4 + i]--;
                break;
            case 3:
                g_fight_config.cpu_base_levels[i] -= 0x80;
                break;
            case 4:
                g_fight_config.pending_stage_index--;
                break;
            case 5:
                g_fight_config.pending_mode_selector--;
                break;
            case 6:
                g_fight_config.pending_view_setting--;
                break;
            case 7:
                g_fight_config.unk_F--;
                break;
            case 8:
                g_saved_record.flags ^= 1;
                break;
            case 9:
                g_saved_record.flags ^= 2;
                break;
            case 10:
                D_800A36F9--;
                break;
            case 11:
                g_fight_view_option--;
                break;
            }
        } else if (g_pad_state.pressed & (0x2000 << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (g_config_menu_cursors[i]) {
            case 0:
                g_fight_config.unk_4[i]++;
                break;
            case 1:
                g_fight_config.unk_4[2 + i]++;
                break;
            case 2:
                g_fight_config.unk_4[4 + i]++;
                break;
            case 3:
                g_fight_config.cpu_base_levels[i] += 0x80;
                break;
            case 4:
                g_fight_config.pending_stage_index++;
                break;
            case 5:
                g_fight_config.pending_mode_selector++;
                break;
            case 6:
                g_fight_config.pending_view_setting++;
                break;
            case 7:
                g_fight_config.unk_F++;
                break;
            case 8:
                g_saved_record.flags ^= 1;
                break;
            case 9:
                g_saved_record.flags ^= 2;
                break;
            case 10:
                D_800A36F9++;
                break;
            case 11:
                g_fight_view_option++;
                break;
            }
        }
        g_fight_config.unk_4[i] = ((s8)g_fight_config.unk_4[i] + 33) % 33;
        g_fight_config.unk_4[2 + i] = ((s8)g_fight_config.unk_4[2 + i] + 8) % 8;
        g_fight_config.unk_4[4 + i] = ((s8)g_fight_config.unk_4[4 + i] + 2) % 2;
    }
    g_fight_config.pending_stage_index =
        ((s8)g_fight_config.pending_stage_index + 38) % 38;
    g_fight_config.pending_mode_selector =
        ((s8)g_fight_config.pending_mode_selector + 7) % 7;
    g_fight_config.pending_view_setting &= 1;
    g_fight_config.unk_F &= 1;
    D_800A36F9 = (D_800A36F9 + 4) % 4;
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
u8 D_800A3188[] = "%s%5d";
u8 g_config_menu_right_pad_value_fmt[] = "%s%3d\n";
u8 D_800A3198[] = "%sC%4d";
u8 g_config_menu_left_cpu_base_level_fmt[] = "%sCL%3x";
u8 g_config_menu_right_cpu_base_level_fmt[] = "%s%3x\n";
u8 g_config_menu_pending_stage_and_mode_fmt[] = "%s%5d\n";
u8 g_config_menu_pending_view_setting_fmt[] = "%sO%4d\n";
u8 g_config_menu_saved_cpu_behavior_adjust_fmt[] = "%sL%4d\n";
u8 D_800A31C8[] = "%sM%4d\n";
u8 g_config_menu_fight_view_option_fmt[] = "%sD%4d\n";
