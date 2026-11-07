/* 10 game functions, among them bits_ExtractMask3F83F8 and
 * bits_DepositMask3F83F8. .text 0x80035438 (ROM 0x25C38). Start boundary:
 * LEGACY (no evidence either way; the extra --expand-lb flag this file once
 * got changed no bytes, since every file gets --expand-lb). One object by the
 * per-file gp model (Q65 merge group). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

/* Formerly separate files, merged as one original file (Q65). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"

enum RowMenuSubflow {
    ROW_MENU_SUBFLOW_TOP_MENU = 1,
    ROW_MENU_SUBFLOW_OPTION_MENU = 2,
    ROW_MENU_SUBFLOW_ENTRY_MENU = 3,
    ROW_MENU_SUBFLOW_STORY_SPRITE_SEQUENCE = 4,
    ROW_MENU_SUBFLOW_PRACTICE_MENU = 5,
    ROW_MENU_SUBFLOW_CHANBARA_MENU = 6,
    ROW_MENU_SUBFLOW_BOOT_MEMCARD_CHECK = 7,
    ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD_PROMPT = 8,
    ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD = 9,
    ROW_MENU_SUBFLOW_TEAM_SEL_SCREEN = 0xA,
    ROW_MENU_SUBFLOW_FADE_TO_MODE = 0xB,
    ROW_MENU_SUBFLOW_FADE_TO_MOVIE = 0xC,
    ROW_MENU_SUBFLOW_OPTION_MEMCARD_LOAD = 0xD,
    ROW_MENU_SUBFLOW_OPTION_MEMCARD_SAVE = 0xE,
};

extern void AddPrim(void *, void *);

extern u8 g_row_menu_subflow_state;

void menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows(void) {
    s32 enabled_row_mask;
    g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
    CopySavedFlagsButtonMappingsAndTimesToWorkingRecord();
    if (g_saved_record.staff_roll_character_mask == 0x3F) {
        enabled_row_mask = 0xFF;
    } else {
        enabled_row_mask = 0xF7;
    }
    menu_SetEnabledRows(enabled_row_mask);
}

extern u8 g_menu_release_closes_non_common_vabs;

void menu_ReleaseResourcesConditionallyCloseLinkAndVabsAndInitRowMenu(void) {
    gpu_ResetGraphMode1();
    gpu_HideResetAndClear();
    if (g_menu_mode_subflow_init_pending == 0) {
        comb_RequestClose();
    }
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
    slot_ReleaseObjectAndResources(0);
    slot_ReleaseObjectAndResources(1);
    eff_ClearInitFlag();
    if (g_menu_release_closes_non_common_vabs != 0) {
        snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb();
        g_fight_loaded_stage_index = -1;
    }
    g_menu_release_closes_non_common_vabs = 1;
    menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows();
    InitRowMenuBlackBgResetFlow((s32)0x80118800);
    g_handler_frame_counter = 0;
    g_main_loop_handler_index = MAIN_LOOP_ROW_MENU_UPDATE;
    gpu_SetDispMaskOn();
}

void gpu_AddScreenGradient(void) {
    POLY_G4 *g;

    g = (POLY_G4 *)g_prim_buf_cursor;
    SetPolyG4(g);
    g->x0 = 0;
    g->y0 = 0;
    g->x1 = 640;
    g->y1 = 0;
    g->x2 = 0;
    g->y2 = 240;
    g->x3 = 640;
    g->y3 = 240;
    g->r0 = 0;
    g->g0 = 0;
    g->b0 = 0x80;
    g->r1 = 0;
    g->g1 = 0;
    g->b1 = 0x80;
    g->r2 = 0;
    g->g2 = 0;
    g->b2 = 0;
    g->r3 = 0;
    g->g3 = 0;
    g->b3 = 0;
    AddPrim(g_gpu_ot_ptr + 0x1007, g);
    g += 1;
    g_prim_buf_cursor = (u32 *)g;
}

void cdrom_EnableSerialMixAndStartRepeatingAudio(void) {
    snd_SerialMixOn();
    cdrom_StartAudioEntry(1);
    cdrom_SetAudioRepeat(1);
}

/* Merged section (Q65: one original file). */
#include "common.h"
#include "include_asm.h"

void menu_InitSubflowForMode(s32 mode_selector) {
    s32 handshake_result;

    if ((u32)mode_selector >= 8) {
        return;
    }

    switch (mode_selector) {
    case FIGHT_MODE_STORY:
        if (g_menu_mode_subflow_init_pending == 0) {
            InitTimedSpriteSequence(0x80118800);
            snd_SerialMixOn();
            cdrom_StartAudioEntry(0);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_STORY_SPRITE_SEQUENCE;
            return;
        }
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case FIGHT_MODE_PRACTICE:
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_PRACTICE_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case FIGHT_MODE_CHANBARA:
        if (g_menu_mode_subflow_init_pending == 0) {
            menu_ResetFlowBackgroundAndConfirmCursor();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_CHANBARA_MENU;
        } else {
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
        }
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case FIGHT_MODE_NETWORK:
        if (g_menu_mode_subflow_init_pending != 0) {
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
            return;
        }

        handshake_result = comb_Connect();
        if (handshake_result == 1) {
            StoreCompactedBits((g_saved_record.unlock_mask |
                                g_comb_received_unlock_mask_bits | 0x7007) &
                               0x003FF3FF);
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
            return;
        }

        if (handshake_result == -1) {
            snd_QueueSound(2, 0x7F, 0x7F);
            g_main_loop_handler_index = MAIN_LOOP_ROW_MENU_UPDATE;
            g_handler_frame_counter = 0;
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
        }
        return;

    case FIGHT_MODE_VSCPU:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x003EF3DF);
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case FIGHT_MODE_TEAM:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x003FF3FF);
        menu_InitSelWithStoredBits(0x80118800);
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TEAM_SEL_SCREEN;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case FIGHT_MODE_VS:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x053FF3FF);
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 7:
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_OPTION_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;
    }
}

/* Merged section (Q65: one original file). */
extern u8 g_row_menu_boot_memcard_load_pending;
extern void menu_InitSubflowForMode(s32);
extern void gpu_AddScreenGradient(void);
extern void menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows(void);
extern void cdrom_EnableSerialMixAndStartRepeatingAudio(void);

void menu_UpdateRowMenuAndSubflows(void) {
    s32 ret;

    g_main_frame_period = 1;
    rand();
    if (g_row_menu_subflow_state == ROW_MENU_SUBFLOW_TOP_MENU &&
        g_row_menu_boot_memcard_load_pending != 0) {
        g_row_menu_boot_memcard_load_pending = 0;
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_BOOT_MEMCARD_CHECK;
    }
    if (g_menu_mode_subflow_init_pending != 0) {
        menu_InitSubflowForMode(g_fight_mode_selector);
        g_menu_mode_subflow_init_pending = 0;
    }
    switch (g_row_menu_subflow_state) {
    case ROW_MENU_SUBFLOW_BOOT_MEMCARD_CHECK:
        gpu_AddScreenGradient();
        ret = memcard_StartLoadOnceStepAndDrawSheetPair();
        if (ret == 1) {
            g_row_menu_subflow_state =
                ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD_PROMPT;
            break;
        }
        if (ret == -1) {
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
        }
        break;
    case ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD_PROMPT:
        gpu_AddScreenGradient();
        ret = menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult();
        if (ret == 1) {
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD;
            break;
        }
        if (ret == -1) {
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
        }
        break;
    case ROW_MENU_SUBFLOW_BOOT_MEMCARD_LOAD:
        gpu_AddScreenGradient();
        if (memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress() != 0) {
            menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows();
        }
        break;
    case ROW_MENU_SUBFLOW_TOP_MENU:
        if (g_handler_frame_counter == 0) {
            snd_SerialMixOn();
            cdrom_StartAudioEntry(7);
            cdrom_SetAudioRepeat(1);
        }
        g_handler_frame_counter++;
        if (g_pad_state.held != 0) {
            g_handler_frame_counter = 1;
        }
        if (g_handler_frame_counter >= 0xCA9) {
            g_main_loop_handler_index = MAIN_LOOP_MOVIE;
        }
        ret = menu_UpdateRowMenuStoreResult(
            g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            cdrom_StartMixFade(0x1E, 0, 0, 0, 0);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_FADE_TO_MODE;
        } else if (ret == -1) {
            cdrom_StartMixFade(0x1E, 0, 0, 0, 0);
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_FADE_TO_MOVIE;
        } else {
            break;
        }
        g_handler_frame_counter = 0x3C;
        break;
    case ROW_MENU_SUBFLOW_FADE_TO_MODE:
        if (--g_handler_frame_counter == 0) {
            cdrom_PauseAndWait();
            g_fight_config.pending_mode_selector =
                ResetFlowAndFetchRowTablePair();
            menu_InitSubflowForMode((s8)g_fight_config.pending_mode_selector);
        }
        break;
    case ROW_MENU_SUBFLOW_FADE_TO_MOVIE:
        if (--g_handler_frame_counter == 0) {
            cdrom_PauseAndWait();
            g_main_loop_handler_index = MAIN_LOOP_MOVIE;
        }
        break;
    case ROW_MENU_SUBFLOW_OPTION_MENU:
        switch (menu_RunOptionModeFlowStateWithRowBitEdit(
            g_pad_state.held, g_pad_state.pressed)) {
        case -1:
        case 1:
            cdrom_PauseAndWait();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_handler_frame_counter = 0;
            if (g_saved_record.staff_roll_character_mask == 0x3F) {
                menu_SetEnabledRows(0xFF);
            } else {
                menu_SetEnabledRows(0xF7);
            }
            break;
        case 0:
            break;
        case 2:
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_OPTION_MEMCARD_LOAD;
            break;
        case 3:
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_OPTION_MEMCARD_SAVE;
            break;
        }
        break;
    case ROW_MENU_SUBFLOW_OPTION_MEMCARD_LOAD:
        func_8006C1FC(0, 0);
        if (memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress() != 0) {
            SetFlowStateOne();
            CopySavedFlagsButtonMappingsAndTimesToWorkingRecord();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_OPTION_MENU;
        }
        break;
    case ROW_MENU_SUBFLOW_OPTION_MEMCARD_SAVE:
        func_8006C1FC(0, 0);
        if (memcard_StepSaveOverwriteAndFormatPrompts() != 0) {
            SetFlowStateOne();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_OPTION_MENU;
        }
        break;
    case ROW_MENU_SUBFLOW_ENTRY_MENU:
        ret = StepThenCloseVab1OnNonzero(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto reset_mode;
        }
        goto check_quit;
    case ROW_MENU_SUBFLOW_STORY_SPRITE_SEQUENCE:
        ret = StepTimedSpriteSequenceWithSkip(g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_quit;
    case ROW_MENU_SUBFLOW_PRACTICE_MENU:
        ret =
            menu_ResetFlowThenRunUpdate(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_cancel;
    case ROW_MENU_SUBFLOW_CHANBARA_MENU:
        ret = func_80077A60(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_cancel;
        }
    load:
        cdrom_PauseAndWait();
        StoreCompactedBits(g_saved_record.unlock_mask & 0x3EF3DF);
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = ROW_MENU_SUBFLOW_ENTRY_MENU;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        break;
    check_cancel:
        if (ret == -1) {
            cdrom_PauseAndWait();
            g_row_menu_subflow_state = ROW_MENU_SUBFLOW_TOP_MENU;
            g_handler_frame_counter = 0;
        }
        break;
    case ROW_MENU_SUBFLOW_TEAM_SEL_SCREEN:
        ret = func_80077AC0(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_quit;
        }
    reset_mode:
        cdrom_PauseAndWait();
        g_main_loop_handler_index = MAIN_LOOP_FIGHT_INIT;
        ApplyWorkingRecordToPendingConfigAndInitializeMode();
        break;
    check_quit:
        if (ret == -1) {
            cdrom_PauseAndWait();
            g_main_loop_handler_index = MAIN_LOOP_ROW_MENU_INIT;
            g_menu_release_closes_non_common_vabs = 0;
        }
        break;
    }
    if (g_main_loop_handler_index != MAIN_LOOP_ROW_MENU_UPDATE) {
        g_main_frame_period = 2;
    }
}

void menu_ReleaseResourcesCloseVabsAndInitRowMenu(void) {
    gpu_ResetGraphMode1();
    gpu_HideResetAndClear();
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
    slot_ReleaseObjectAndResources(0);
    slot_ReleaseObjectAndResources(1);
    eff_ClearInitFlag();
    snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb();
    InitRowMenuBlackBgResetFlow((s32)0x80118800);
    g_main_loop_handler_index = MAIN_LOOP_MEMCARD_SAVE_UPDATE;
    gpu_SetDispMaskOn();
}

void DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit(void) {
    g_main_frame_period = 1;
    gpu_AddScreenGradient();
    if (memcard_StepSaveOverwriteAndFormatPrompts() != 0) {
        g_main_loop_handler_index = MAIN_LOOP_ROW_MENU_INIT;
        g_main_frame_period = 2;
    }
}

s32 bits_ExtractMask3F83F8(s32 scattered_bits) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (scattered_bits & (one << i)) {
                result |= (one << j);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}

s32 bits_DepositMask3F83F8(s32 packed_bits) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (packed_bits & (one << j)) {
                result |= (one << i);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}

/* Q65: this file's initialized small data (.sdata), in address order. */
u8 g_menu_release_closes_non_common_vabs = 1;
u8 g_row_menu_boot_memcard_load_pending = 1;
u8 g_menu_mode_subflow_init_pending = 0;
/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
u8 g_row_menu_subflow_state;
