/* 10 game functions, among them bits_ExtractMask3F83F8 and
 * bits_DepositMask3F83F8. .text 0x80035438 (ROM 0x25C38). Start boundary:
 * LEGACY (no evidence either way; the extra --expand-lb flag this file once
 * got changed no bytes, since every file gets --expand-lb). One object by the
 * per-file gp model (Q65 merge group). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* Formerly separate files, merged as one original file (Q65). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"

extern void AddPrim(void *, void *);

extern u8 g_row_menu_subflow_state;

void menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows(void) {
    s32 a0;
    g_row_menu_subflow_state = 1;
    CopySavedFlagsButtonMappingsAndTimesToWorkingRecord();
    if (g_saved_record.unk_04 == 0x3F) {
        a0 = 0xFF;
    } else {
        a0 = 0xF7;
    }
    menu_SetEnabledRows(a0);
}

extern u8 D_800A31D8;

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
    if (D_800A31D8 != 0) {
        snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb();
        g_fight_loaded_stage_index = -1;
    }
    D_800A31D8 = 1;
    menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows();
    InitRowMenuBlackBgResetFlow((s32)0x80118800);
    g_handler_frame_counter = 0;
    g_main_loop_handler_index = 9;
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

void menu_InitSubflowForMode(s32 arg0) {
    s32 temp;

    if ((u32)arg0 >= 8) {
        return;
    }

    switch (arg0) {
    case 0:
        if (g_menu_mode_subflow_init_pending == 0) {
            InitTimedSpriteSequence(0x80118800);
            snd_SerialMixOn();
            cdrom_StartAudioEntry(0);
            g_row_menu_subflow_state = 4;
            return;
        }
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = 3;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 2:
        g_row_menu_subflow_state = 5;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 3:
        if (g_menu_mode_subflow_init_pending == 0) {
            menu_ResetFlowBackgroundAndConfirmCursor();
            g_row_menu_subflow_state = 6;
        } else {
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = 3;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
        }
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 6:
        if (g_menu_mode_subflow_init_pending != 0) {
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = 3;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
            return;
        }

        temp = comb_Connect();
        if (temp == 1) {
            StoreCompactedBits(
                (g_saved_record.unlock_mask | D_800A38E4 | 0x7007) &
                0x003FF3FF);
            InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
            g_row_menu_subflow_state = 3;
            cdrom_EnableSerialMixAndStartRepeatingAudio();
            return;
        }

        if (temp == -1) {
            snd_QueueSound(2, 0x7F, 0x7F);
            g_main_loop_handler_index = 9;
            g_handler_frame_counter = 0;
            g_row_menu_subflow_state = 1;
        }
        return;

    case 1:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x003EF3DF);
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = 3;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 5:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x003FF3FF);
        menu_InitSelWithStoredBits(0x80118800);
        g_row_menu_subflow_state = 0xA;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 4:
        StoreCompactedBits(g_saved_record.unlock_mask & 0x053FF3FF);
        InitWithCachedValueAndStoredBitsBlackBg(0x80118800);
        g_row_menu_subflow_state = 3;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;

    case 7:
        g_row_menu_subflow_state = 2;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        return;
    }
}

/* Merged section (Q65: one original file). */
extern u8 D_800A31D9;
extern void menu_InitSubflowForMode(s32);
extern void gpu_AddScreenGradient(void);
extern void menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows(void);
extern void cdrom_EnableSerialMixAndStartRepeatingAudio(void);

void menu_UpdateRowMenuAndSubflows(void) {
    s32 ret;

    g_main_frame_period = 1;
    rand();
    if (g_row_menu_subflow_state == 1 && D_800A31D9 != 0) {
        D_800A31D9 = 0;
        g_row_menu_subflow_state = 7;
    }
    if (g_menu_mode_subflow_init_pending != 0) {
        menu_InitSubflowForMode(g_fight_mode_selector);
        g_menu_mode_subflow_init_pending = 0;
    }
    switch (g_row_menu_subflow_state) {
    case 7:
        gpu_AddScreenGradient();
        ret = memcard_StartLoadOnceStepAndDrawSheetPair();
        if (ret == 1) {
            g_row_menu_subflow_state = 8;
            break;
        }
        if (ret == -1) {
            g_row_menu_subflow_state = 1;
        }
        break;
    case 8:
        gpu_AddScreenGradient();
        ret = menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult();
        if (ret == 1) {
            g_row_menu_subflow_state = 9;
            break;
        }
        if (ret == -1) {
            g_row_menu_subflow_state = 1;
        }
        break;
    case 9:
        gpu_AddScreenGradient();
        if (memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress() != 0) {
            menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows();
        }
        break;
    case 1:
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
            g_main_loop_handler_index = 0xF;
        }
        ret = menu_UpdateRowMenuStoreResult(
            g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            cdrom_StartMixFade(0x1E, 0, 0, 0, 0);
            g_row_menu_subflow_state = 0xB;
        } else if (ret == -1) {
            cdrom_StartMixFade(0x1E, 0, 0, 0, 0);
            g_row_menu_subflow_state = 0xC;
        } else {
            break;
        }
        g_handler_frame_counter = 0x3C;
        break;
    case 11:
        if (--g_handler_frame_counter == 0) {
            cdrom_PauseAndWait();
            g_fight_config.pending_mode_selector =
                ResetFlowAndFetchRowTablePair();
            menu_InitSubflowForMode((s8)g_fight_config.pending_mode_selector);
        }
        break;
    case 12:
        if (--g_handler_frame_counter == 0) {
            cdrom_PauseAndWait();
            g_main_loop_handler_index = 0xF;
        }
        break;
    case 2:
        switch (menu_RunOptionModeFlowStateWithRowBitEdit(
            g_pad_state.held, g_pad_state.pressed)) {
        case -1:
        case 1:
            cdrom_PauseAndWait();
            g_row_menu_subflow_state = 1;
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_handler_frame_counter = 0;
            if (g_saved_record.unk_04 == 0x3F) {
                menu_SetEnabledRows(0xFF);
            } else {
                menu_SetEnabledRows(0xF7);
            }
            break;
        case 0:
            break;
        case 2:
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_row_menu_subflow_state = 0xD;
            break;
        case 3:
            CopyWorkingFlagsAndButtonMappingsToSavedRecord();
            g_row_menu_subflow_state = 0xE;
            break;
        }
        break;
    case 13:
        func_8006C1FC(0, 0);
        if (memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress() != 0) {
            SetFlowStateOne();
            CopySavedFlagsButtonMappingsAndTimesToWorkingRecord();
            g_row_menu_subflow_state = 2;
        }
        break;
    case 14:
        func_8006C1FC(0, 0);
        if (memcard_StepSaveOverwriteAndFormatPrompts() != 0) {
            SetFlowStateOne();
            g_row_menu_subflow_state = 2;
        }
        break;
    case 3:
        ret = StepThenCloseVab1OnNonzero(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto reset_mode;
        }
        goto check_quit;
    case 4:
        ret = StepTimedSpriteSequenceWithSkip(g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_quit;
    case 5:
        ret =
            menu_ResetFlowThenRunUpdate(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_cancel;
    case 6:
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
        g_row_menu_subflow_state = 3;
        cdrom_EnableSerialMixAndStartRepeatingAudio();
        break;
    check_cancel:
        if (ret == -1) {
            cdrom_PauseAndWait();
            g_row_menu_subflow_state = 1;
            g_handler_frame_counter = 0;
        }
        break;
    case 10:
        ret = func_80077AC0(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_quit;
        }
    reset_mode:
        cdrom_PauseAndWait();
        g_main_loop_handler_index = 0;
        ApplyWorkingRecordToPendingConfigAndInitializeMode();
        break;
    check_quit:
        if (ret == -1) {
            cdrom_PauseAndWait();
            g_main_loop_handler_index = 8;
            D_800A31D8 = 0;
        }
        break;
    }
    if (g_main_loop_handler_index != 9) {
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
    g_main_loop_handler_index = 0x1B;
    gpu_SetDispMaskOn();
}

void DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit(void) {
    g_main_frame_period = 1;
    gpu_AddScreenGradient();
    if (memcard_StepSaveOverwriteAndFormatPrompts() != 0) {
        g_main_loop_handler_index = 8;
        g_main_frame_period = 2;
    }
}

s32 bits_ExtractMask3F83F8(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << i)) {
                result |= (one << j);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}

s32 bits_DepositMask3F83F8(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << j)) {
                result |= (one << i);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}

/* Q65: this file's initialized small data (.sdata), in address order. */
u8 D_800A31D8 = 1;
u8 D_800A31D9 = 1;
u8 g_menu_mode_subflow_init_pending = 0;
/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
u8 g_row_menu_subflow_state;
