/* memcard_Format, 30 game functions, then the link-cable wrappers
 * comb_Init..comb_WaitRead8 and math_Popcount32. .text 0x80037F08 (ROM
 * 0x28708). Start boundary: LEGACY (a tooling split, no evidence either way).
 */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

extern char g_str_sio_800A3210[5]; /* "sio:" */

extern s32 g_comb_received_pad_word;

extern s32 g_comb_close_request;
extern s16 g_memcard_request_status;
extern s16 g_memcard_request_retry_count;
extern s16 g_memcard_request_operation;
extern s32 g_memcard_request_state;
extern u8 g_memcard_request_write_started;
extern u8 g_comb_write_not_aborted;
extern s32 g_comb_read_fd;
extern s32 g_comb_write_fd;
extern s32 g_comb_event_error;
extern s32 g_comb_event_ioer;
extern u8 g_comb_events_enabled;

extern s32 D_800A31F8;
extern u8 g_replay_write_frame;
extern Rec1C (*g_replay_fighter_frames)[2];
extern u8 g_replay_ring_wrapped;

extern u8 g_memcard_validation_load_active;

extern s32 g_comb_wait_state;
extern s32 g_comb_recv_buf[2];
extern s32 g_comb_send_buf;
extern s32 write(s32, void *, s32); /* PsyQ: long write(long, void *, long) */
extern u8 g_replay_playback_frame;

extern Unk800F34D8Save D_800F34D8; /* = g_shared_buffer + 0x100 */
extern s32 g_memcard_save_file_name;

s32 memcard_Format(s32 port, s32 card) {
    char buf[8];
    sprintf(buf, (char *)D_800109C8, port, card);
    return format(buf);
}

void memcard_BuildSaveBlock(u8 *a0) {
    s32 record_byte_sum;
    u8 *p;
    s32 i;

    record_byte_sum = 0;
    p = (u8 *)&g_saved_record;
    i = 0;
    do {
        record_byte_sum += *p++;
        i++;
    } while ((u32)i < 0x24);

    {
        /* FAKE: the save-block view in its own local; a cast of a0 at each
           use moves the copy into the loop's delay slot (score 2) */
        Unk800F34D8Save *var_3 = (Unk800F34D8Save *)a0;
        i = 0;
        do {
            var_3->rec[i] = g_saved_record;
            var_3->sum[i] = record_byte_sum;
            {
                s32 j = 0;
                do {
                    var_3->ptr[j] = 0;
                    var_3->val[j] = 0;
                    j++;
                } while (j < 0x16);
            }
            i++;
        } while (i < 3);
        var_3->unk_FC = 0;
    }
}

s32 memcard_RestoreSaveBlock(Unk800F34D8Save *arg0) {
    s32 i;
    /* j counts the checksum loop and then the pointer-restore loop, reused
       loop to loop the way SOTN's DebugCaptureScreen reuses its i (Q51) */
    /* SOTN: src/dra/42398.c:75 @aa53500 */
    s32 j;

    i = 0;
    do {
        s32 sum;
        u8 *record_byte_cursor;

        sum = 0;
        record_byte_cursor = (u8 *)&arg0->rec[i];
        j = 0;
        do {
            sum += *record_byte_cursor;
            record_byte_cursor++;
            j++;
        } while (j < 0x24U);
        if (sum == arg0->sum[i]) {
            break;
        }
        i++;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (g_memcard_validation_load_active != 0) {
        return 1;
    }

    {
        FileRecord *src = &arg0->rec[i];

        if (!(src->flags & 0x80)) {
            g_saved_record = *src;
        }

        j = 0;
        do {
            u16 *ptr = arg0->ptr[j];
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = arg0->val[j];
            }
            j++;
        } while (j < 0x16);
    }

    return 1;
}

void memcard_ClearBuffer(void) {
    u8 *p = g_shared_buffer;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}

extern u8 g_memcard_save_title[];
/* memcard_BuildFileHeader copies D_800109EC's first 16 halfwords to +0x60 of
 * its buffer and D_80010A2C's 128 bytes to +0x80: the memory-card file header's
 * icon CLUT (16 RGB555 colours; the 16 halfwords after them are zero) and its
 * 16x16 4-bit icon image. D_80010AAC is the save-file id
 * (g_memcard_save_file_name holds its address), then 3 bytes of rodata padding.
 */
const u16 D_800109EC[32] = {
    0x7FFF, 0x77DF, 0x6B9F, 0x635E, 0x5B1E, 0x56FD, 0x4ABD, 0x427C,
    0x323C, 0x2DFB, 0x21BB, 0x199B, 0x115A, 0x0D3A, 0x051A, 0x8000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
const u8 D_80010A2C[128] = {
    0x00, 0x00, 0x00, 0x00, 0x10, 0x51, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA3, 0xC5, 0x3E, 0x00, 0x00, 0x10, 0x53, 0x38, 0xEA, 0x58, 0x29, 0x00,
    0x00, 0x84, 0xBA, 0x7D, 0xDC, 0x8D, 0x03, 0x00, 0x00, 0x10, 0x41, 0xB7,
    0xDD, 0xBB, 0x1C, 0x00, 0x00, 0x94, 0xDB, 0xCD, 0xAD, 0x11, 0x01, 0x00,
    0x00, 0xE8, 0x8E, 0x66, 0x8E, 0x00, 0x02, 0x00, 0x00, 0xA9, 0x16, 0x62,
    0x8E, 0x00, 0x17, 0x00, 0x10, 0x32, 0xC6, 0x6A, 0xAE, 0x00, 0x18, 0x00,
    0x10, 0x84, 0x9A, 0x44, 0xDD, 0x03, 0x29, 0x00, 0x00, 0x00, 0x50, 0x38,
    0xEB, 0x08, 0x6B, 0x00, 0x00, 0x30, 0xD9, 0x06, 0xE7, 0x5D, 0x6B, 0x00,
    0x00, 0xE5, 0x6C, 0x00, 0x91, 0xCD, 0x6D, 0x00, 0x00, 0xC3, 0x09, 0x00,
    0x20, 0xEA, 0x7E, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xB2, 0x8E, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
};
const char D_80010AAC[] = "BASLUS-00663BUSHIDO2";
extern u8 D_800A3200;
extern u8 D_800A3201;

void memcard_BuildFileHeader(u8 *out) {
    s32 first_class_count, second_class_count, third_class_count;
    s32 i;
    s32 mask;
    s32 bit;

    third_class_count = 0;
    second_class_count = 0;
    first_class_count = 0;
    mask = g_saved_record.unlock_mask;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = g_memcard_title_count_class_by_unlock_bit[i];
            switch (v) {
            case 0:
                first_class_count++;
                break;
            case 1:
                second_class_count++;
                break;
            case 2:
                third_class_count++;
                break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    strcpy(out + 4, g_memcard_save_title);

    out[0x22] = g_memcard_title_sjis_digits[first_class_count * 2 + 0];
    out[0x23] = g_memcard_title_sjis_digits[first_class_count * 2 + 1];
    out[0x3C] = g_memcard_title_sjis_digits[second_class_count * 2 + 0];
    out[0x3D] = g_memcard_title_sjis_digits[second_class_count * 2 + 1];

    if (third_class_count > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] =
            g_memcard_title_sjis_digits_0_to_4[third_class_count * 2 + 0];
        out[0x43] =
            g_memcard_title_sjis_digits_0_to_4[third_class_count * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        const u16 *src = D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        const u8 *src_icon_cursor = D_80010A2C;
        u8 *icon_out_cursor = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(icon_out_cursor + 0x80);
            const u16 *src = (const u16 *)src_icon_cursor;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            src_icon_cursor += 0x80;
            i++;
            icon_out_cursor += 0x80;
        } while (i <= 0);
    }
}

void memcard_StepRequest(void) {
    s32 var_v1;
    s32 var_v0;
    s32 free_blocks;
    s32 create_first;
    u16 retry_count;

    var_v1 = g_memcard_request_state;
    if (var_v1 != 1)
        goto state_other;

    var_v1 = D_800A31F8;
    if (var_v1 == -1)
        goto sub_inc;

    if (var_v1 >= 0)
        goto positive_path;
    if (var_v1 == -2)
        goto neg2_handler;
    return;

positive_path:
    if (var_v1 >= 3)
        return;
    if (var_v1 <= 0)
        return;
    if (g_memcard_request_operation == 0) {
        g_memcard_request_state = 5;
        return;
    }
    if (g_memcard_request_operation == 3)
        goto set_state_7;
    g_memcard_request_state = 3;
    return;

neg2_handler:
    if (g_memcard_request_operation != 3)
        goto neg2_else;
set_state_7:
    g_memcard_request_state = 7;
    return;
neg2_else:
    g_memcard_request_state = 0;
    g_memcard_request_status = 0xA;
    return;

sub_inc:
    retry_count = (u16)g_memcard_request_retry_count + 1;
    g_memcard_request_retry_count = retry_count;
    if ((s16)retry_count < 4)
        return;
    var_v0 = 8;
    goto finish;

state_other:
    var_v0 = 5;
    if (var_v1 == var_v0)
        goto state_5;
    if (var_v1 < 6) {
        var_v0 = 3;
        if (var_v1 == var_v0)
            goto state_3;
        return;
    }
    var_v0 = 7;
    if (var_v1 == var_v0)
        goto state_7;
    return;

state_3:
    g_memcard_request_status = 1;
    memcard_CountFiles(0, 0);
    free_blocks = memcard_CountFreeBlocks();
    if (memcard_HasFile((u8 *)g_memcard_save_file_name) != 0) {
        create_first = 0;
        if (g_memcard_request_operation == 1) {
            var_v0 = 0xD;
            goto finish;
        }
        goto setup_load;
    }
    create_first = 1;
    if (free_blocks == 0) {
        var_v0 = 7;
        goto finish;
    }
setup_load:
    g_memcard_request_write_started = 1;
    memcard_ClearBuffer();
    memcard_BuildFileHeader(g_shared_buffer);
    memcard_BuildSaveBlock(g_shared_buffer + 0x100);
    if (memcard_WriteFile(0, 0, g_memcard_save_file_name, g_shared_buffer, 1,
                          0x200, create_first) != 0) {
        close(g_memcard_fd);
        var_v0 = 3;
        goto finish;
    }
    g_memcard_request_state = 4;
    return;

state_5:
    memcard_CountFiles(0, 0);
    memcard_CountFreeBlocks();
    if (memcard_HasFile((u8 *)g_memcard_save_file_name) == 0) {
        var_v0 = 0xE;
        goto finish;
    }
    g_memcard_request_status = 4;
    memcard_ClearBuffer();
    if (memcard_ReadFile(
            0, 0, g_memcard_save_file_name, g_shared_buffer, 0x200) != 0) {
        close(g_memcard_fd);
        var_v0 = 6;
        goto finish;
    }
    g_memcard_request_state = 6;
    return;

state_7:
    var_v0 = memcard_Format(0, 0);
    if (var_v0 != 0) {
        var_v0 = 0xB;
        goto finish;
    }
    var_v0 = 0xC;
finish:
    g_memcard_request_status = var_v0;
    g_memcard_request_state = 0;
}

extern s32 memcard_RestoreSaveBlock(Unk800F34D8Save *);

/* Load/save state-machine completion handler: dispatches on
 * g_memcard_request_state (4 = after a write, 6 = after a read), reaps
 * memcard_PollSwEvents()'s status, closes the file and posts a result code to
 * g_memcard_request_status. The still-pending paths share the fail_store end
 * label (the shared-end-label recipe, .claude/rules/shared-end-label.md). */
void memcard_PollFileCompletion(void) {
    s32 ret;
    s32 in_progress_status;

    switch (g_memcard_request_state) {
    case 4:
        ret = memcard_PollSwEvents();
        if (ret == 0) {
            in_progress_status = 1;
            goto fail_store;
        }
        close(g_memcard_fd);
        if (ret == 1) {
            g_memcard_request_status = 2;
        } else {
            g_memcard_request_status = 3;
        }
        g_memcard_request_state = 0;
        return;
    case 6:
        ret = memcard_PollSwEvents();
        if (ret == 0) {
            in_progress_status = 4;
            goto fail_store;
        }
        close(g_memcard_fd);
        if (ret == 1) {
            g_memcard_request_status = 5;
            if (memcard_RestoreSaveBlock(&D_800F34D8) == 0) {
                g_memcard_request_status = 0xF;
            }
        } else {
            g_memcard_request_status = 6;
        }
        g_memcard_request_state = 0;
        return;
    }
    return;

fail_store:
    g_memcard_request_status = in_progress_status;
}

s32 memcard_Update(void) {
    if ((u32)g_memcard_request_state < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    memcard_StepRequest();
    memcard_PollFileCompletion();
    return g_memcard_request_status;
}

void memcard_RequestLoad(void) {
    g_memcard_request_status = 4;
    g_memcard_request_retry_count = 0;
    g_memcard_request_operation = 0;
    g_memcard_request_state = 1;
}

void memcard_RequestSave(void) {
    g_memcard_request_status = 1;
    g_memcard_request_operation = 1;
    g_memcard_request_write_started = 0;
    g_memcard_request_retry_count = 0;
    g_memcard_request_state = 1;
}

void memcard_RequestOverwrite(void) {
    g_memcard_request_status = 1;
    g_memcard_request_operation = 2;
    g_memcard_request_write_started = 0;
    g_memcard_request_retry_count = 0;
    g_memcard_request_state = 1;
}

void memcard_RequestFormat(void) {
    g_memcard_request_status = 9;
    g_memcard_request_operation = 3;
    g_memcard_request_retry_count = 0;
    g_memcard_request_state = 1;
}

extern u8 g_memcard_load_request_armed;
extern u8 g_memcard_validation_load_active;
extern void memcard_RequestLoad(void);
extern s32 memcard_Update(void);

s32 memcard_StartLoadOnceStepAndDrawSheetPair(void) {
    s32 result;
    s32 request_status;

    result = 0;
    if (g_memcard_load_request_armed) {
        g_memcard_load_request_armed = 0;
        g_memcard_validation_load_active = 1;
        memcard_RequestLoad();
    }
    request_status = memcard_Update();
    switch (request_status - 4) {
    case 0:
        break;
    case 1:
        result = 1;
        break;
    case 2:
        result = -1;
        break;
    case 3:
        result = -1;
        break;
    case 4:
        result = -1;
        break;
    case 5:
        result = -1;
        break;
    case 6:
        result = -1;
        break;
    case 7:
        result = -1;
        break;
    case 8:
        result = -1;
        break;
    case 9:
        result = -1;
        break;
    case 10:
        result = -1;
        break;
    case 11:
        result = -1;
        break;
    default:
        result = -1;
        break;
    }
    gpu_DrawSheetPairCenteredTileWithOptionalSelection(0, -1);
    if (result) {
        g_memcard_load_request_armed = 1;
        g_memcard_validation_load_active = 0;
    }
    return result;
}

/* Q65: this file's statics (.sbss), in address order. */
static u8 g_menu_two_choice_prompt_cursor;
static u8 g_memcard_status8_retry_count;
static u8 g_memcard_status5_retry_count;
static u8 g_memcard_status14_retry_count;
static u8 g_memcard_status10_retry_count;
static u8 g_memcard_status15_retry_count;
static u8 g_memcard_status_wait_countdown;
static u8 D_800A3334;
static u8 g_memcard_status5_retries_exhausted;
static u8 g_memcard_retry_load_status8_latched;
static u8 g_memcard_save_prompt_status5_to_7_and_13_to_15_retry_count;
static u8 g_memcard_save_prompt_status10_retry_count;
static u8 g_memcard_save_prompt_status8_retry_count;
static u8 g_memcard_message_countdown;
static u8 g_memcard_prompt_choice;
static u8 D_800A3354;

s32 menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult(void) {
    extern u8 g_menu_prompt_rearm;
    s32 result = 0;
    u32 var_1;
    if (g_menu_prompt_rearm != 0) {
        g_menu_prompt_rearm = 0;
        g_menu_two_choice_prompt_cursor = 0;
    }
    var_1 = g_pad_state.pressed;
    if (var_1 & 0x400040) {
        snd_QueueSound(1, 0x7F, 0x7F);
        if (g_menu_two_choice_prompt_cursor == 0) {
            result = 1;
        } else {
            result = -1;
        }
    } else if (var_1 & (u32)0x80008000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        g_menu_two_choice_prompt_cursor = 0;
    } else if (var_1 & 0x20002000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        g_menu_two_choice_prompt_cursor = 1;
    }
    gpu_DrawSheetPairCenteredTileWithOptionalSelection(
        0x13, g_menu_two_choice_prompt_cursor);
    if (result != 0) {
        g_menu_prompt_rearm = 1;
    }
    return result;
}

s32 memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress(void) {
    extern u8 D_800A3205;
    s32 result = 0;
    s32 v0;
    s32 var_2;

    if (D_800A3205) {
        g_memcard_status8_retry_count = 0;
        g_memcard_status5_retry_count = 0;
        g_memcard_status14_retry_count = 0;
        g_memcard_status10_retry_count = 0;
        g_memcard_status15_retry_count = 0;
        g_memcard_validation_load_active = 1;
        memcard_RequestLoad();
        D_800A3205 = 0;
        g_memcard_status_wait_countdown = 0x5A;
        D_800A3334 = 0;
        g_memcard_status5_retries_exhausted = 0;
        g_memcard_retry_load_status8_latched = 0;
    }

    v0 = memcard_Update();

    if (g_memcard_retry_load_status8_latched != 0) {
        gpu_DrawSheetPairCenteredTileWithOptionalSelection(0xA, -1);
        goto timer;
    }

    var_2 = 0;
    if (g_memcard_validation_load_active == 0) {
        if (g_memcard_status5_retries_exhausted == 1) {
            switch (v0 - 4) {
            case 0:
                var_2 = 2;
                break;
            case 1:
                var_2 = 3;
                break;
            case 2:
            case 6:
            case 10:
            case 11:
                var_2 = 4;
                break;
            case 4:
                var_2 = -1;
                break;
            default:
                var_2 = 0;
                break;
            }
        } else {
            switch (v0 - 4) {
            case 0:
                var_2 = 2;
                break;
            case 1:
                var_2 = 3;
                break;
            case 2:
                var_2 = 4;
                break;
            case 10:
                var_2 = 1;
                break;
            case 11:
                var_2 = 0xB;
                break;
            case 4:
                var_2 = -1;
                break;
            case 6:
                var_2 = 0x14;
                break;
            default:
                var_2 = 0;
                break;
            }
        }
    }

    if (var_2 >= 0) {
        gpu_DrawSheetPairCenteredTileWithOptionalSelection(var_2, -1);
    }

    if (g_memcard_validation_load_active != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            g_memcard_status8_retry_count++;
            if ((u8)g_memcard_status8_retry_count >= 5) {
                g_memcard_retry_load_status8_latched = 1;
                g_memcard_status_wait_countdown = 0x5A;
                break;
            }
            memcard_RequestLoad();
            break;
        case 11:
            g_memcard_status15_retry_count++;
            if ((u8)g_memcard_status15_retry_count >= 5) {
                g_memcard_validation_load_active = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 6:
            g_memcard_status10_retry_count++;
            if ((u8)g_memcard_status10_retry_count >= 5) {
                g_memcard_validation_load_active = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 10:
            g_memcard_status14_retry_count++;
            if ((u8)g_memcard_status14_retry_count >= 5) {
                g_memcard_validation_load_active = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 2:
            g_memcard_validation_load_active = 0;
            break;
        case 1:
            g_memcard_status5_retry_count++;
            if ((u8)g_memcard_status5_retry_count >= 5) {
                g_memcard_validation_load_active = 0;
                g_memcard_status5_retries_exhausted = 1;
            }
            memcard_RequestLoad();
            break;
        default:
            break;
        }
    } else {
        switch (v0 - 5) {
        case 0:
        case 1:
        case 5:
        case 9:
        case 10:
            goto timer;
        case 3:
            g_memcard_retry_load_status8_latched = 1;
            g_memcard_status_wait_countdown = 0x5A;
            break;
        default:
            break;
        }
    }

    goto end;

timer:
    g_memcard_status_wait_countdown--;
    if ((u8)g_memcard_status_wait_countdown == 0 ||
        (g_pad_state.pressed & 0x100010)) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = 1;
    }

end:
    if (result != 0) {
        D_800A3205 = 1;
    }
    return result;
}

s32 memcard_StepSaveOverwriteAndFormatPrompts(void) {
    extern u8 D_800A3207;
    extern u8 D_800A3206;
    extern void memcard_RequestLoad(void);
    extern void memcard_RequestSave(void);
    extern void memcard_RequestOverwrite(void);
    extern void memcard_RequestFormat(void);
    s32 result = 0;
    s32 var_1 = -1;
    s32 v0;
    s32 var_3;

    if (!D_800A3207) {
        D_800A3207 = 1;
        g_memcard_message_countdown = 0x5A;
        g_memcard_prompt_choice = 0;
        D_800A3354 = 0;
        g_memcard_validation_load_active = 0;
    }

    v0 = memcard_Update();

    if (D_800A3354 != 0) {
        gpu_DrawSheetPairCenteredTileWithOptionalSelection(0xA, -1);
        g_memcard_message_countdown--;
        if (((u8)g_memcard_message_countdown) == 0 ||
            (g_pad_state.pressed & 0x100010)) {
            snd_QueueSound(2, 0x7F, 0x7F);
            D_800A3207 = 1;
            g_memcard_message_countdown = 0x5A;
            g_memcard_prompt_choice = 0;
            D_800A3354 = 0;
            g_memcard_validation_load_active = 0;
        }
        goto end;
    }

    /* states 2 and 3 keep v0: their empty arms are the target's `== 2` /
     * `== 3` tests that branch straight to the join, as SOTN's matched
     * _SsSndSetVabAttr lists its reverb modes 1-3 (without them: score 5;
     * a switch: 14) */
    /* SOTN: src/main/psxsdk/libsnd/seqread.c:396 @db41b28 */
    if (D_800A3207 == 1) {
        v0 = 0;
    } else if (D_800A3207 == 2) {
    } else if (D_800A3207 == 3) {
    } else if (D_800A3207 == 4) {
        v0 = 0x11;
    }

    var_3 = 0;
    if (g_memcard_validation_load_active != 0) {
        goto sel_dispatch;
    }

    if (D_800A3207 == 3) {
        if (v0 == 8) {
            goto case8_sel;
        }
        if (v0 == 10) {
            goto case12_sel;
        }
        goto case9_11_sel;
    }

    switch (v0) {
    case 0:
        var_3 = 0x11;
        /* each case that passes g_memcard_prompt_choice to
           gpu_DrawSheetPairCenteredTileWithOptionalSelection sets var_1 in its
           own body (cross-jump merges the stores; one shared store reached by
           goto: score 13) */
        var_1 = g_memcard_prompt_choice;
        goto sel_dispatch;
    case 13:
    case 17:
        var_3 = 6;
        var_1 = g_memcard_prompt_choice;
        goto sel_dispatch;
    case 1:
        var_3 = (-(g_memcard_request_write_started != 0)) & 7;
        goto sel_dispatch;
    case 2:
        var_3 = 8;
        goto sel_dispatch;
    case 3:
        var_3 = 9;
        goto sel_dispatch;
    case 8:
    case8_sel:
        var_3 = -1;
        goto sel_dispatch;
    case 7:
        var_3 = 5;
        goto sel_dispatch;
    case 10:
        if (D_800A3206 == 0) {
            g_memcard_prompt_choice = 1;
        }
        if (g_pad_state.pressed & 0x400040) {
            D_800A3206 = 0;
            snd_QueueSound(1, 0x7F, 0x7F);
            var_3 = 0xD;
            if (g_memcard_prompt_choice == 0) {
                memcard_RequestFormat();
                goto sel_dispatch;
            }
            v0 = 0;
            D_800A3207 = 5;
            g_memcard_message_countdown = 0x5A;
            g_memcard_prompt_choice = 0;
            var_3 = -1;
            goto sel_dispatch;
        }
        var_3 = 0xC;
        if ((g_pad_state.pressed & 0xA000A000U) != 0) {
            D_800A3206 = 1;
        }
        var_1 = g_memcard_prompt_choice;
        goto sel_dispatch;
    case 9:
    case 11:
    case9_11_sel:
        var_3 = 0xD;
        goto sel_dispatch;
    case 12:
    case12_sel:
        var_3 = 0xF;
        goto sel_dispatch;
    default:
        var_3 = 0;
        goto sel_dispatch;
    }

sel_dispatch:
    if (var_3 >= 0) {
        gpu_DrawSheetPairCenteredTileWithOptionalSelection(var_3, var_1);
    }

    if (g_memcard_validation_load_active != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            g_memcard_save_prompt_status8_retry_count++;
            if (((u8)g_memcard_save_prompt_status8_retry_count) >= 5) {
                D_800A3354 = 1;
                g_memcard_message_countdown = 0x5A;
                break;
            }
            memcard_RequestLoad();
            break;
        case 1:
        case 2:
        case 3:
        case 9:
        case 10:
        case 11:
            g_memcard_save_prompt_status5_to_7_and_13_to_15_retry_count++;
            if (((u8)g_memcard_save_prompt_status5_to_7_and_13_to_15_retry_count) >=
                5) {
                g_memcard_validation_load_active = 0;
                memcard_RequestSave();
                break;
            }
            memcard_RequestLoad();
            break;
        case 6:
            g_memcard_save_prompt_status10_retry_count++;
            if (((u8)g_memcard_save_prompt_status10_retry_count) >= 5) {
                g_memcard_validation_load_active = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        default:
            break;
        }
    } else if (D_800A3207 == 3) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            result = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        default:
            memcard_RequestSave();
            D_800A3207 = 2;
            g_memcard_message_countdown = 0x5A;
            g_memcard_prompt_choice = 0;
            break;
        case 6:
            g_memcard_message_countdown--;
            if (((u8)g_memcard_message_countdown) == 0 ||
                (g_pad_state.pressed & 0x100010)) {
                snd_QueueSound(2, 0x7F, 0x7F);
                result = 1;
                break;
            }
            break;
        }
    } else {
        switch (v0) {
        case 8:
            D_800A3354 = 1;
            g_memcard_message_countdown = 0x5A;
            break;
        case 2:
        case 3:
        case 7:
        case 12:
            g_memcard_message_countdown--;
            if (((u8)g_memcard_message_countdown) == 0 ||
                (g_pad_state.pressed & 0x100010)) {
                snd_QueueSound(2, 0x7F, 0x7F);
                if (v0 != 7) {
                    result = 1;
                    break;
                }
                D_800A3207 = 1;
                g_memcard_message_countdown = 0x5A;
                g_memcard_prompt_choice = 0;
                D_800A3354 = 0;
                g_memcard_validation_load_active = 0;
            }
            break;
        case 11:
            g_memcard_validation_load_active = 1;
            memcard_RequestLoad();
            D_800A3207 = 3;
            g_memcard_message_countdown = 0x5A;
            break;
        case 13:
            D_800A3207 = 4;
            g_memcard_message_countdown = 0x5A;
            g_memcard_prompt_choice = 0;
            break;
        case 0:
            if (D_800A3207 == 5) {
                D_800A3207 = 1;
                break;
            }
            goto sw4_L2;
        case 17:
        sw4_L2:
            if (g_pad_state.pressed & 0x400040) {
                snd_QueueSound(1, 0x7F, 0x7F);
                if (v0 == 0) {
                    if (g_memcard_prompt_choice != 0) {
                        result = 1;
                        break;
                    }
                    g_memcard_save_prompt_status8_retry_count = 0;
                    g_memcard_save_prompt_status5_to_7_and_13_to_15_retry_count =
                        0;
                    g_memcard_save_prompt_status10_retry_count = 0;
                    g_memcard_validation_load_active = 1;
                    memcard_RequestLoad();
                } else {
                    if (g_memcard_prompt_choice != 0)
                        goto area_c_long;
                    memcard_RequestOverwrite();
                }
                D_800A3207 = 2;
                break;
            area_c_long:
                v0 = 0;
                D_800A3207 = 1;
                g_memcard_message_countdown = 0x5A;
                g_memcard_prompt_choice = 0;
                break;
            }
            goto sw4_buttons;
        case 10:
        sw4_buttons:
            if (g_pad_state.pressed & 0x80008000U) {
                snd_QueueSound(0, 0x7F, 0x7F);
                g_memcard_prompt_choice = 0;
                break;
            }
            if (g_pad_state.pressed & 0x20002000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                g_memcard_prompt_choice = 1;
            }
            break;
        default:
            break;
        }
    }

    if (v0 < 11) {
        if (v0 >= 9) {
            goto d_check;
        }
        if (v0 == 1) {
            goto d_check;
        }
        goto end;
    }
    if (v0 != 17) {
        goto end;
    }
d_check:
    if (D_800A31F8 == -1) {
        D_800A3354 = 1;
        g_memcard_message_countdown = 0x5A;
    }

end:
    if (result != 0) {
        D_800A3207 = 0;
        g_memcard_validation_load_active = 0;
    }
    return result;
}

s32 *GetSharedBuffer(void) { return (s32 *)g_shared_buffer; }

void replay_Reset(void) {
    s32 i;
    s32 j;

    g_replay_fighter_frames = (Rec1C(*)[2])g_shared_buffer;
    g_replay_write_frame = 0;
    g_replay_ring_wrapped = 0;
    for (i = 0x1F; i >= 0; i--) {
        g_replay_events[i].event_frame = 0xFF;
    }
    for (j = 0xB3; j >= 0; j--) {
        g_replay_model_draws[j].draw_age = -1;
    }
}

void replay_BeginFrame(void) {
    extern u8 g_replay_event_search_cursor;
    extern s16 g_replay_model_draw_search_cursor;
    s32 i;
    Unk800F68E0Rec *q;
    s16 val;
    s16 newval;
    for (i = 0; i < 0x20; i++) {
        if (g_replay_events[i].event_frame == g_replay_write_frame) {
            g_replay_events[i].event_frame = 0xFF;
        }
    }

    q = g_replay_model_draws;
    i = 0;
    do {
        val = q->draw_age;
        if (val != -1) {
            newval = val + 1;
            q->draw_age = newval;
            if (newval - q->draw_span >= 0x101) {
                q->draw_age = -1;
            }
        }
        i++;
        q++;
    } while (i < 0xB4);

    g_replay_event_search_cursor = 0;
    g_replay_model_draw_search_cursor = 0;
}

void replay_RecordModelDraw(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    extern s16 g_replay_model_draw_search_cursor;
    extern u8 D_800A3209;
    Unk800F68E0Rec *var_0;
    s32 i;
    s16 idx;
    s16 cur;
    s32 state;
    u8 seen;
    s32 raw;
    s32 rot;

    var_0 = g_replay_model_draws;
    i = 0;
    do {
        state = var_0->draw_age;
        if (state != -1) {
            seen = var_0->draw_span;
            if (seen == state - 1 && seen < 0xFF) {
                raw = var_0->slot_rot_x << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    var_0->draw_pos[0] == arg2[0] &&
                    var_0->draw_pos[1] == arg2[1] &&
                    var_0->draw_pos[2] == arg2[2] &&
                    ((rot - arg3[0]) & 0xFFF) == 0 &&
                    ((var_0->draw_rot_y - arg3[1]) & 0xFFF) == 0 &&
                    ((var_0->draw_rot_z - arg3[2]) & 0xFFF) == 0) {
                    var_0->draw_span = seen + 1;
                    return;
                }
            }
        }
        i++;
        var_0++;
    } while (i < 0xB4);

    idx = g_replay_model_draw_search_cursor;
    var_0 = &g_replay_model_draws[idx];
    while (idx < 0xB4) {
        if (var_0->draw_age == -1) {
            break;
        }
        cur = idx + 1;
        g_replay_model_draw_search_cursor = cur;
        var_0++;
        idx = cur;
    }

    if (g_replay_model_draw_search_cursor == 0xB4) {
        D_800A3209++;
        return;
    }

    var_0->draw_age = 0;
    var_0->model_entry_index = arg1;
    var_0->draw_span = 0;
    var_0->draw_pos[0] = arg2[0];
    var_0->draw_pos[1] = arg2[1];
    var_0->draw_pos[2] = arg2[2];
    var_0->slot_rot_x = (arg3[0] & 0xFFF) | (arg0 << 12);
    var_0->draw_rot_y = arg3[1];
    var_0->draw_rot_z = arg3[2];
}

void replay_RecordEvent(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 g_replay_replaying_event;
    extern u8 g_replay_event_search_cursor;
    Unk80101BF0Rec *var_0;
    u8 idx;
    if (g_replay_replaying_event == 0) {
        idx = g_replay_event_search_cursor;
        var_0 = &g_replay_events[idx];
        for (; idx < 0x20; idx++, var_0++) {
            if (var_0->event_frame == 0xFF) {
                break;
            }
            g_replay_event_search_cursor = idx + 1;
        }
        if (g_replay_event_search_cursor != 0x20) {
            u8 tmp = g_replay_write_frame;
            var_0->unk_1 = arg0;
            var_0->event_kind = arg1;
            var_0->event_frame = tmp;
            var_0->event_pos[0] = arg2[0];
            var_0->event_pos[1] = arg2[1];
            var_0->event_pos[2] = arg2[2];
            if (arg3 != NULL) {
                var_0->event_vector[0] = arg3[0];
                var_0->event_vector[1] = arg3[1];
                var_0->event_vector[2] = arg3[2];
            }
        }
    }
}

void replay_RecordFighter(Unk80101EC8Record *a0) {
    s16 idx;
    Rec1C *dest;

    idx = a0->index;
    dest = &g_replay_fighter_frames[g_replay_write_frame][idx];

    dest->h4 = a0->world_pos.x;
    dest->h8 = a0->world_pos.z;
    dest->h6 = a0->world_pos.y;

    {
        u16 v = a0->orientation.vy;
        u8 b = a0->draw_move_set;
        dest->hA = (v & 0xFFF) | (b << 12);
    }

    dest->hC = a0->ground_y;
    dest->b14 = a0->eased_pose_bearing_offset >> 2;
    dest->b15 = a0->eased_pose_elevation_offset >> 2;
    dest->b16 = a0->opponent_elevation_offset >> 2;
    dest->w0 = a0->cur_move;
    dest->b17 = 0;

    if (a0->cur_move_is_common != 0) {
        dest->b17 = 1;
    }
    if (a0->cur_move_slot != 0) {
        dest->b17 |= 2;
    }

    dest->hE = a0->blend_src_frame_code;
    dest->h10 = a0->blend_dst_frame_code;
    dest->h12 = a0->pose_interp_weight;
    dest->b18 = a0->frame_flags;
    dest->b19 = a0->move_frame;
}

void replay_AdvanceFrame(void) {
    u8 val = g_replay_write_frame;
    if (val == 0x77) {
        g_replay_write_frame = 0;
        g_replay_ring_wrapped = 1;
    } else {
        g_replay_write_frame = val + 1;
    }
}

void replay_Start(void) {
    gpu_ResetGraphMode1();
    gpu_RestoreObjectClutOffsets();
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    func_8001B6F4();
    eff_StopCommands();
    g_replay_playback_frame = 0;
    g_main_loop_handler_index = 5;
}

extern s32 collision_GetHitTag(void);

void fighter_ProbeVerticalCollisionAtPointPairMidpointForTagAndClass(
    Unk80101EC8Record *arg0, s32 *arg1, s32 *arg2) {
    s32 sp10[3];
    s32 sp20[3];
    s32 sp30[4];
    s16 sp40[4];
    s32 mid_x, mid_y, mid_z;
    s32 result;

    mid_x =
        (s32)(arg0->ground_probe_points[0].x + arg0->ground_probe_points[1].x) /
        2;
    sp10[0] = mid_x;
    mid_y =
        (s32)(arg0->ground_probe_points[0].y + arg0->ground_probe_points[1].y) /
        2;
    sp10[1] = mid_y - 0x190;
    mid_z =
        (s32)(arg0->ground_probe_points[0].z + arg0->ground_probe_points[1].z) /
        2;
    sp20[0] = mid_x;
    sp20[1] = mid_y + 0x190;
    sp10[2] = mid_z;
    sp20[2] = mid_z;
    if (collision_TestSegment(sp10, sp20, sp30, sp40) != 0) {
        result = collision_GetHitTag();
        *arg1 = result;
        if (result == 7) {
            goto neg;
        }
        if (result == 0) {
            goto neg;
        }
        *arg2 = ((0x2A >> result) ^ 1) & 1;
    } else {
        *arg1 = -1;
    neg:
        *arg2 = -1;
    }
}

extern u8 g_replay_replaying_event;

void replay_PlayFrame(void) {
    MotionFrame work[2];
    /* FAKE: frame layout - slot_BlendTwoPosesIntoNodeMatrices writes two
     * MATRIXes (0x40 bytes); 0x88 keeps the frame (MATRIX[2]: score 58;
     * MATRIX[4] also differs) */
    s32 sp120[34];
    s32 pos[3];
    s16 rot[3];
    u16 sp1C0[200];
    s32 out_b1;
    s32 out_b2;
    s32 idx;
    s32 var_8;
    s32 i;
    Rec1C *p;
    Unk80101EC8Record *var_11;
    Unk80101BF0Rec *e;
    Unk800F68E0Rec *s;
    /* temp: the 0/1 weapon-set selector in the per-player loop, then the
     * replay window of the event loop (Ruling 11, ordinary-c-judge-decidable)
     */
    s32 temp;
    u8 var_15;
    u8 *var_16;

    if (g_replay_ring_wrapped != 0) {
        idx = (g_replay_write_frame + g_replay_playback_frame) % 120;
        var_8 = (g_replay_playback_frame << 12) / 120;
    } else {
        idx = g_replay_playback_frame;
        var_8 = (idx << 12) / g_replay_write_frame;
    }
    D_800A3778 = camera_GetViewMatrixWithHalvedMiddleRow();
    /* The frame record's address is written out at each argument (F3
     * compound-address duplication, no-new-park-categories); a `rec` local:
     * score 20. */
    camera_UpdateReplayViewWithSineYawOffset(
        &g_replay_fighter_frames[idx][g_fight_followup_fighter_index],
        g_fight_followup_fighter_index == 0 ? &g_replay_fighter_frames[idx][1]
                                            : &g_replay_fighter_frames[idx][0],
        var_8);
    camera_UpdateReplayViewWithConstantYawOffset(
        &g_replay_fighter_frames[idx][g_fight_followup_fighter_index],
        g_fight_followup_fighter_index == 0 ? &g_replay_fighter_frames[idx][1]
                                            : &g_replay_fighter_frames[idx][0],
        var_8);
    camera_ApplyReplayView(var_8);

    for (i = 0; i < 2; i++) {
        /* entry: the frame's entry in the practice weapon table (if arm) or
         * the character's weapon table (else arm) (Ruling 11,
         * ordinary-c-judge-decidable) */
        u16 *entry;

        p = &g_replay_fighter_frames[idx][i];
        var_11 = &D_80101EC8[i];
        motion_DecodeFrame((p->hE >> 14) & 3, p->hE & 0x3FFF, &work[0], sp1C0);
        motion_DecodeFrame(
            (p->h10 >> 14) & 3, p->h10 & 0x3FFF, &work[1], sp1C0);
        motion_ApplyRecordedPoseAdjustments(var_11, p, &work[0], &work[1]);
        slot_BlendTwoPosesIntoNodeMatrices(
            i, (u8 *)&work[0], (u8 *)&work[1], p->h12, (MATRIX *)sp120);
        pos[0] = p->h4;
        pos[1] = p->h6;
        pos[2] = p->h8;
        rot[0] = 0;
        rot[1] = p->hA;
        rot[2] = 0;
        func_80040D48(i, 1, pos, rot, 0, p->hC);
        if (p->b18 & 1) {
            node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
                var_11->weapon_entry_id, i * 2 + 0x8000, 0, 0);
        }
        if (p->b18 & 4) {
            node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
                g_resource_kind_entry_ids[var_11->wielded_object_kind],
                i * 2 + 0x8001, 0, 0);
        }
        if (p->b18 & 2) {
            node_QueueTableEntryNodePairUnderSlotNode(
                var_11->weapon_entry_id, i * 2, (p->b18 >> 4) & 1);
        }
        if (p->b18 & 8) {
            node_QueueTableEntryNodePairUnderSlotNode(
                g_resource_kind_entry_ids[var_11->wielded_object_kind],
                i * 2 | 1, (p->b18 >> 5) & 1);
        }
        fighter_UpdateBoneWorldData(
            var_11, SPAD->bone_hit_points[i], SPAD->weapon_points[i],
            SPAD->second_weapon_points[i]);
        fighter_ProbeVerticalCollisionAtPointPairMidpointForTagAndClass(
            var_11, &out_b1, &out_b2);
        if (out_b1 != 1) {
            var_11->collision_hit_tag = out_b1;
            if (out_b2 != -1) {
                var_11->collision_hit_class = out_b2;
            }
        }
        var_15 = var_11->move_frame;
        var_11->move_frame = p->b19;
        var_16 = var_11->motion_metadata;
        if (p->b17 & 1) {
            entry = &g_motion_common_sections
                         .motion_entries[p->w0->motion_bank_entry_index * 2];
            var_11->motion_metadata =
                g_motion_common_sections.motion_metadata_base + entry[1];
        } else {
            temp = (p->b17 >> 1) & 1;
            entry = &g_motion_fighter_sections[temp]
                         .motion_entries[p->w0->motion_bank_entry_index * 2];
            var_11->motion_metadata =
                g_motion_fighter_sections[temp].motion_metadata_base + entry[1];
        }
        if (p->b18 & 0x40) {
            fighter_ProcessFrameEvents(var_11);
        }
        var_11->move_frame = var_15;
        var_11->motion_metadata = var_16;
        gpu_QueueSlotDrawMovesForSelectedSet(i, (p->hA >> 12) & 7);
    }

    e = g_replay_events;
    for (i = 0; i < 0x20; i++, e++) {
        if (e->event_frame == idx) {
            pos[0] = e->event_pos[0];
            pos[1] = e->event_pos[1];
            pos[2] = e->event_pos[2];
            rot[0] = e->event_vector[0];
            rot[1] = e->event_vector[1];
            rot[2] = e->event_vector[2];
            g_replay_replaying_event = 1;
            fight_RecordReplayEventAndDispatchSoundOrEffect(
                e->unk_1, e->event_kind, pos, rot);
            g_replay_replaying_event = 0;
        }
    }

    if (g_replay_ring_wrapped != 0) {
        temp = 0x77 - g_replay_playback_frame;
    } else {
        s32 next_count = g_replay_playback_frame + 1;
        temp = g_replay_write_frame - next_count;
    }
    s = g_replay_model_draws;
    for (i = 0; i < 0xB4; i++, s++) {
        if (s->draw_age >= temp && s->draw_age - s->draw_span <= temp) {
            pos[0] = s->draw_pos[0];
            pos[1] = s->draw_pos[1];
            pos[2] = s->draw_pos[2];
            rot[0] = s->slot_rot_x;
            rot[1] = s->draw_rot_y;
            rot[2] = s->draw_rot_z;
            node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
                s->model_entry_index, 1, pos, rot);
        }
    }

    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(1);
    snd_UpdateStageSounds();
    g_replay_playback_frame++;
    if ((g_replay_ring_wrapped != 0
             ? g_replay_playback_frame == 0x78
             : g_replay_playback_frame == g_replay_write_frame) ||
        (g_pad_state.pressed & 0x400040)) {
        switch (g_fight_mode_selector) {
        case 0:
            if (D_80101EC8[0].is_defeated == 0) {
                s32 var_19 = g_fight_saved_first_fighter_selection != 0xFF;
                if (D_800A3712 != 0) {
                    D_800A38D4 = 2;
                    if (g_fight_saved_first_fighter_selection != 0xFF) {
                        g_fight_pending_unlock_mask_bits |=
                            1 << g_fight_config_id_data_rows[(s8)g_fight_config
                                                                 .unk_4[0]];
                    }
                    if (var_19) {
                        fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields();
                    }
                }
                snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
                if (D_800A3894 != 0) {
                    if (D_800A3712 != 0) {
                        func_8003B534(var_19 ? 3 : 6);
                    } else {
                        fight_ConfigureFighter1FromOperandsAndLoad(
                            D_800A3894 + 6);
                        func_8003B534(var_19 ? 2 : 5);
                    }
                } else {
                    fight_RunCommandStream();
                }
                return;
            }
            g_fight_followup_fighter_index = 1;
            if (g_fight_saved_first_fighter_selection == 0xFF) {
                g_main_loop_handler_index = 0xC;
                break;
            }
            snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
            fight_RestoreFighter0SelectionAndStateKeepingDisplacedPairAndResetFighter0Fields();
            fight_FillMissingConfigInitFighterAndLoadObject(0);
            fight_RequestBothFightersEntriesAndReloadOnChange();
            if (D_800A3712 != 0 && D_80101EC8[1].is_defeated != 0) {
                func_8003B534(6);
            } else {
                func_8003B534(4);
            }
            return;
        case 3:
            if (D_80101EC8[0].is_defeated == 0) {
                g_main_loop_handler_index = 0x1E;
            } else {
                g_main_loop_handler_index = 0xC;
            }
            break;
        default:
            if (D_80101EC8[0].is_defeated == 0 ||
                D_80101EC8[1].is_defeated == 0) {
                g_main_loop_handler_index = 0x10;
            } else {
                g_main_loop_handler_index = 0xC;
            }
            break;
        }
    }
}

void comb_Init(void) {
    s32 neg1;
    EnterCriticalSection();
    neg1 = -1;
    do {
        g_comb_event_ioer = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    } while (g_comb_event_ioer == neg1);
    neg1 = -1;
    do {
        g_comb_event_error = OpenEvent(0xF000000B, 0x8000, 0x2000, 0);
    } while (g_comb_event_error == neg1);
    ExitCriticalSection();
    neg1 = -1;
    VSync(2);
    AddCOMB();
    do {
        g_comb_write_fd = open(g_str_sio_800A3210, 2);
    } while (g_comb_write_fd == neg1);
    neg1 = -1;
    do {
        g_comb_read_fd = open(g_str_sio_800A3210, 0x8001);
    } while (g_comb_read_fd == neg1);
    _comb_control(2, 0, 0);
    _comb_control(1, 3, 0xE100);
    _comb_control(1, 4, 1);
}

void comb_Close(void) {
    close(g_comb_read_fd);
    close(g_comb_write_fd);
    EnterCriticalSection();
    CloseEvent(g_comb_event_ioer);
    CloseEvent(g_comb_event_error);
    ExitCriticalSection();
    VSync(2);
    DelCOMB();
    _comb_control(1, 1, 0);
}

s32 comb_IsCtsDsrClear(void) { return (_comb_control(0, 0, 0) & 0x180) == 0; }

void comb_ReadCtsSetRts(void) {
    if (_comb_control(3, 1, 0) != 0) {
        g_comb_local_side = 1;
    } else {
        g_comb_local_side = 0;
    }
    _comb_control(3, 0, 1);
}

void comb_EnableEvents(void) {
    EnableEvent(g_comb_event_error);
    EnableEvent(g_comb_event_ioer);
    g_comb_events_enabled = 1;
    g_comb_close_request = 0;
}

void comb_ResetClose(void) {
    g_comb_events_enabled = 0;
    g_comb_close_request = 0;
    _comb_control(2, 0, 0);
    _comb_control(1, 1, 0);
    comb_Close();
    g_main_loop_handler_index = 8;
}

void comb_Abort(void) {
    comb_ResetClose();
    g_comb_abort_request = 1;
}

void comb_RequestClose(void) { g_comb_close_request = 1; }

s32 comb_WriteWaitCallback(s32 a0, u32 a1) {
    if (a1 > 0x10000) {
        g_comb_write_not_aborted = 0;
        return 0;
    }
    return 1;
}

s32 comb_Write8(void *buf) {
    s32 s1;
    s32 s0;

    s1 = GetRCnt(0xF2000001);
    if (s1 >= 0x401) {
        ResetRCnt(0xF2000001);
        s1 = 0;
    }

    while (1) {
        while (1) {
            if (_comb_control(3, 1, 0) != 0) {
                break;
            }
            if (GetRCnt(0xF2000001) - s1 >= 0x7801) {
                return 0;
            }
        }

        s0 = 0;
        do {
            if (_comb_control(3, 1, 0) == 0) {
                break;
            }
            s0++;
        } while (s0 < 1000);

        if (s0 >= 1000) {
            break;
        }
    }

    _comb_control(1, 1, 1);
    g_comb_write_not_aborted = 1;
    _comb_control(4, 0, (s32)&comb_WriteWaitCallback);
    write(g_comb_write_fd, &g_comb_send_buf, 8);
    _comb_control(4, 0, 0);
    _comb_control(1, 1, 0);
    return g_comb_write_not_aborted;
}

void comb_Read8(void) { read(g_comb_read_fd, g_comb_recv_buf, 8); }

s32 comb_WaitRead8(void) {
    s32 s0;
    s32 s1;
    s32 a1;
    s32 a0;
    s32 v0;

    s1 = 0;
    s0 = GetRCnt(0xF2000001);
    if (s0 >= 0x401) {
        goto overflow;
    }
    goto loop_check;
overflow:
    ResetRCnt(0xF2000001);
    s0 = 0;
loop_check:
    if (TestEvent(g_comb_event_ioer) != 0) {
        goto success;
    }
    if (TestEvent(g_comb_event_error) == 0) {
        goto poll;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    _comb_control(2, 0, 0);
    s0 = 0;
    comb_Read8();
    ResetRCnt(0xF2000001);
poll:
    if (((_comb_control(0, 0, 0) >> 7) & 3) == 1) {
        goto loop_check;
    }
    if (GetRCnt(0xF2000001) - s0 < 0x3C01) {
        goto loop_check;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    goto overflow;
success:
    a1 = g_comb_recv_buf[0];
    a0 = g_comb_recv_buf[1];
    v0 = a1 ^ (a1 >> 16);
    v0 = v0 ^ (a0 >> 16);
    v0 = v0 & 0xFFFF;
    if ((a0 & 0xFFFF) == v0) {
        goto match;
    }
    D_800A38D0 += 1;
    return 0;
ret0_tramp:
    return 0;
match:
    g_comb_received_pad_word = a1;
    g_comb_received_payload_check_word = a0;
    return 1;
}

s32 math_Popcount32(u32 arg0) {
    s32 count = 0;
    s32 i;
    for (i = 0; i < 32; i++) {
        count += (arg0 >> i) & 1;
    }
    return count;
}

extern s32 g_comb_prev_send_word;

/* Clears g_comb_wait_state while g_comb_events_enabled is 0. Otherwise packs
 * bits of g_comb_local_side, g_comb_close_request and g_comb_wait_state,
 * a0->type[0] and the low half of a0->held into g_comb_send_buf, puts
 * g_comb_send_payload and a 16-bit xor check in g_comb_send_buf_plus_0x4, and
 * calls comb_Write8, comb_Read8 or comb_WaitRead8 as
 * g_comb_initial_pad_exchange_pending and g_comb_local_side select, returning
 * early when a transfer fails or after comb_ResetClose. If
 * g_comb_initial_pad_exchange_pending is then 0, it stores new values to
 * a0->held and a0->type. */
void comb_ExchangePad(PadState *a0) {
    /* buttons holds the input pad button word, then an outgoing button
     * halfword (this exchange's, or the previous one's from
     * g_comb_prev_send_word) being packed into held, as SOTN's
     * CheckStageCollision reuses y for successive CheckCollision coordinates
     * (Q51; separate locals: score 33). */
    /* SOTN: src/dra/692E8.c:447 @db41b28 */
    s32 buttons;
    s32 var_1;
    s32 hi16;
    s32 flag;
    s32 var_4;
    s32 c0lo;
    s32 t;
    /* FAKE: constant-holder displaces the g_comb_send_buf_plus_0x4 store from
     * the slot before the branch (emits nothing); the literal: score 3 */
    s32 var_7;

    if (g_comb_events_enabled != 0) {
        buttons = a0->held;
        var_4 = g_comb_local_side;
        var_7 = 0;
        var_1 = (var_4 << 31) | (g_comb_close_request << 30) |
                ((g_comb_wait_state & 3) << 28) | (a0->type[0] << 16) |
                (buttons & 0xFFFF);
        g_comb_send_buf = var_1;
        hi16 = g_comb_send_payload << 16;
        var_1 = var_1 ^ (var_1 >> 16);
        var_1 = var_1 ^ (hi16 >> 16);
        var_1 = var_1 & 0xFFFF;
        hi16 = hi16 | var_1;
        g_comb_send_buf_plus_0x4 = hi16;
        flag = g_comb_initial_pad_exchange_pending;

        if (flag != 0) {
            if (var_4 == 0) {
                comb_Read8();
            } else {
                if (comb_Write8(&g_comb_send_buf) == 0) {
                    comb_Abort();
                    return;
                }
                D_800A3908 += math_Popcount32(buttons & 0xFFFF);
                comb_Read8();
            }
        } else {
            if (comb_WaitRead8() == 0) {
                comb_Abort();
                return;
            }
            if (g_comb_local_side == 1) {
                if (g_comb_received_pad_word & 0x40000000) {
                    comb_ResetClose();
                    return;
                }
                if (g_comb_prev_send_word & 0x40000000) {
                    comb_ResetClose();
                    return;
                }
            }
            if (comb_Write8(&g_comb_send_buf) == 0) {
                comb_Abort();
                return;
            }
            D_800A3908 += math_Popcount32(buttons & 0xFFFF);
            comb_Read8();
            if (g_comb_local_side == 0) {
                if (g_comb_close_request != var_7 ||
                    (g_comb_received_pad_word & 0x40000000)) {
                    comb_ResetClose();
                    return;
                }
            }
        }

        if (g_comb_initial_pad_exchange_pending == 0) {
            /* FAKE: the record's held / type stores go through pointers; stored
               as members (in-struct memory) sched moves the D_800A36C2 /
               D_800A36D2 loads above them (score 22). */
            u32 *var_8 = &a0->held;
            s16 *type = a0->type;

            D_800A38FC += math_Popcount32((u16)g_comb_received_pad_word);
            c0lo = (u16)g_comb_received_pad_word;
            if (g_comb_local_side == 0) {
                buttons = (u16)g_comb_send_buf;
                *var_8 = (c0lo << 16) | buttons;
                t = D_800A36C2;
                type[1] = t & 0xF;
            } else {
                buttons = (u16)g_comb_prev_send_word;
                *var_8 = (buttons << 16) | c0lo;
                t = D_800A36C2;
                type[0] = t & 0xF;
                t = D_800A36D2;
                type[1] = t & 0xF;
            }
            if (g_comb_local_side == 0) {
                if (((g_comb_received_pad_word >> 28) & 3) == 2 &&
                    g_comb_wait_state == 2) {
                    g_comb_wait_state = 0;
                }
            } else {
                if (((g_comb_received_pad_word >> 28) & 3) == 2 &&
                    ((g_comb_prev_send_word >> 28) & 3) == 2) {
                    g_comb_wait_state = 0;
                }
            }
        }
        g_comb_initial_pad_exchange_pending = 0;
        g_comb_prev_send_word = g_comb_send_buf;
        D_800A36D4 = g_comb_send_buf_plus_0x4;
    } else {
        g_comb_wait_state = 0;
    }
}

void comb_ExchangeEmptyPad(void) {
    PadState pad;
    pad.held = 0;
    pad.type[1] = 4;
    pad.type[0] = 4;
    comb_ExchangePad(&pad);
}

void comb_BeginWait(void) {
    g_comb_wait_state = 1;
    VSync(2);
    comb_ExchangeEmptyPad();
    VSync(2);
}

void comb_EndWait(void) {
    s32 val;
    g_comb_wait_state = 2;
    VSync(2);
    val = 2;
    do {
        comb_ExchangeEmptyPad();
        if (g_comb_events_enabled == 0) {
            goto end;
        }
        ResetRCnt(0xF2000001);
        do {
        } while (GetRCnt(0xF2000001) < 0x100);
    } while (g_comb_wait_state == val);
end:
    VSync(2);
    comb_ExchangeEmptyPad();
    VSync(2);
}

/* .data 0x8008F19C..0x8008F220 (cdrom.c's .data precedes; 2B344.c's
 * D_8008F220.. follows), in address order:
 * memcard_BuildFileHeader's full-width Shift-JIS digit tables
 * g_memcard_title_sjis_digits_0_to_4
 * ("0".."4") and g_memcard_title_sjis_digits ("0".."9"), the full-width save
 * title g_memcard_save_title
 * ("BB2 Narukagami:? VS Shainto:?", the two "?" overwritten with digits) and
 * the per-unlock-bit title-counter selectors
 * g_memcard_title_count_class_by_unlock_bit (27 used; the 28th byte is zero).
 * Only this file reads them. Defined after every function, so no body sees a
 * definition where it saw an extern. */
u8 g_memcard_title_sjis_digits_0_to_4[12] = {
    0x82, 0x4F, 0x82, 0x50, 0x82, 0x51, 0x82, 0x52, 0x82, 0x53, 0x00, 0x00,
};
u8 g_memcard_title_sjis_digits[24] = {
    0x82, 0x4F, 0x82, 0x50, 0x82, 0x51, 0x82, 0x52, 0x82, 0x53, 0x82, 0x54,
    0x82, 0x55, 0x82, 0x56, 0x82, 0x57, 0x82, 0x58, 0x00, 0x00, 0x00, 0x00,
};
u8 g_memcard_save_title[68] = {
    0x82, 0x61, 0x82, 0x61, 0x82, 0x51, 0x81, 0x40, 0x82, 0x6D, 0x82, 0x81,
    0x82, 0x92, 0x82, 0x95, 0x82, 0x8B, 0x82, 0x81, 0x82, 0x87, 0x82, 0x81,
    0x82, 0x8D, 0x82, 0x89, 0x81, 0x46, 0x81, 0x48, 0x81, 0x40, 0x82, 0x75,
    0x82, 0x72, 0x81, 0x40, 0x82, 0x72, 0x82, 0x88, 0x82, 0x81, 0x82, 0x89,
    0x82, 0x8E, 0x82, 0x94, 0x82, 0x8F, 0x81, 0x46, 0x81, 0x48, 0x81, 0x40,
    0x81, 0x40, 0x81, 0x40, 0x00, 0x00, 0x00, 0x00,
};
u8 g_memcard_title_count_class_by_unlock_bit[28] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x03, 0x03, 0x03, 0x02, 0x03, 0x00,
};

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 g_memcard_save_file_name = (s32)D_80010AAC;
s32 g_memcard_request_state = 0;
s32 D_800A31F8 = 0;
u8 g_memcard_validation_load_active = 0;
u8 D_800A31FD = 0;  /* not named by any code or data: size from the gap */
s16 D_800A31FE = 0; /* not named by any code or data: size from the gap */
u8 D_800A3200 = 0x81;
u8 D_800A3201 = 0x7b;
u8 D_800A3202 = 0; /* not named by any code or data: size from the gap */
u8 g_memcard_load_request_armed = 1;
u8 g_menu_prompt_rearm = 1;
u8 D_800A3205 = 1;
u8 D_800A3206 = 0;
u8 D_800A3207 = 0;
u8 g_replay_replaying_event = 0;
u8 D_800A3209 = 0;
s16 D_800A320A = 0; /* not named by any code or data: size from the gap */
u8 g_comb_events_enabled = 0;
char g_str_sio_800A3210[5] = "sio:";
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
s32 g_comb_recv_buf[2];
s32 g_comb_send_buf;
s32 g_comb_received_pad_word;
s32 g_comb_prev_send_word;
Rec1C (*g_replay_fighter_frames)[2];
u8 g_replay_write_frame;
s16 g_replay_model_draw_search_cursor;
s32 g_comb_close_request;
s32 g_comb_read_fd;
s32 g_comb_event_ioer;
s32 g_comb_write_fd;
u8 g_replay_ring_wrapped;
u8 g_replay_event_search_cursor;
s16 g_memcard_request_status;
u16 g_comb_send_payload;
s16 g_memcard_request_operation;
u8 g_replay_playback_frame;
s32 g_comb_event_error;
s16 g_memcard_request_retry_count;
u8 g_comb_write_not_aborted;
s32 g_comb_wait_state;
u8 g_memcard_request_write_started;
s32 D_800A38D0;
s32 D_800A38FC;
s32 D_800A3908;
u8 g_comb_initial_pad_exchange_pending;
