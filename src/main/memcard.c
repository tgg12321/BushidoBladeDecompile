/* The memory-card layer: memcard_Init / memcard_Quit, the software and hardware
 * event waits (memcard_PollSwEvents..memcard_AckHwEvents), memcard_CountFiles,
 * memcard_ReadFile and memcard_WriteFile. .text 0x800375EC (ROM 0x27DEC). Start
 * boundary: LEGACY (a tooling split, no evidence either way). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

/* This file's .rodata: the memory-card path formats. */
const char g_str_memcard_fmt[12] = "bu%1d%1d:*";
const char D_800109BC[12] = "bu%1d%1d:%s";

extern s32 g_memcard_sw_event_ioe;
extern s32 g_memcard_sw_event_err;
extern s32 g_memcard_sw_event_timeout;
extern s32 g_memcard_sw_event_new;
extern s32 g_memcard_hw_event_ioe;
extern s32 g_memcard_hw_event_err;
extern s32 g_memcard_hw_event_timeout;
extern s32 g_memcard_hw_event_new;

extern s32 g_memcard_file_count;

extern struct DIRENTRY g_memcard_file_list[];

void memcard_Init(void) {
    InitCARD(1);
    StartCARD();
    _bu_init();
    ChangeClearPAD(0);
    EnterCriticalSection();
    g_memcard_sw_event_ioe = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_memcard_sw_event_err = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_memcard_sw_event_timeout = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_memcard_sw_event_new = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_memcard_hw_event_ioe = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_memcard_hw_event_err = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_memcard_hw_event_timeout = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_memcard_hw_event_new = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    EnableEvent(g_memcard_sw_event_ioe);
    EnableEvent(g_memcard_sw_event_err);
    EnableEvent(g_memcard_sw_event_timeout);
    EnableEvent(g_memcard_sw_event_new);
    EnableEvent(g_memcard_hw_event_ioe);
    EnableEvent(g_memcard_hw_event_err);
    EnableEvent(g_memcard_hw_event_timeout);
    EnableEvent(g_memcard_hw_event_new);
}

void memcard_Quit(void) {
    EnterCriticalSection();
    CloseEvent(g_memcard_sw_event_ioe);
    CloseEvent(g_memcard_sw_event_err);
    CloseEvent(g_memcard_sw_event_timeout);
    CloseEvent(g_memcard_sw_event_new);
    CloseEvent(g_memcard_hw_event_ioe);
    CloseEvent(g_memcard_hw_event_err);
    CloseEvent(g_memcard_hw_event_timeout);
    CloseEvent(g_memcard_hw_event_new);
    ExitCriticalSection();
    StopCARD();
}

s32 memcard_PollSwEventsTimeout(void) {
    extern s32 g_memcard_poll_count;
    s32 result;
    s32 one;
    s32 temp;
    result = (TestEvent(g_memcard_sw_event_ioe) == 1);
    one = 1;
    if (TestEvent(g_memcard_sw_event_err) == one) {
        result = 2;
    }
    if (TestEvent(g_memcard_sw_event_timeout) == one) {
        result = 3;
    }
    if (TestEvent(g_memcard_sw_event_new) == one) {
        result = 4;
    }
    temp = g_memcard_poll_count;
    g_memcard_poll_count = temp + 1;
    if (temp >= 0x78) {
        result = 2;
    }
    return result;
}

s32 memcard_PollSwEvents(void) {
    if (TestEvent(g_memcard_sw_event_ioe) == 1) {
        return 1;
    }
    if (TestEvent(g_memcard_sw_event_err) == 1) {
        return 2;
    }
    if (TestEvent(g_memcard_sw_event_timeout) == 1) {
        return 3;
    }
    return (TestEvent(g_memcard_sw_event_new) == 1) * 4;
}

void memcard_AckSwEvents(void) {
    TestEvent(g_memcard_sw_event_ioe);
    TestEvent(g_memcard_sw_event_err);
    TestEvent(g_memcard_sw_event_timeout);
    TestEvent(g_memcard_sw_event_new);
}

s32 memcard_WaitHwEvent(void) {
    while (1) {
        if (TestEvent(g_memcard_hw_event_ioe) == 1) {
            return 1;
        }
        if (TestEvent(g_memcard_hw_event_err) == 1) {
            return 2;
        }
        if (TestEvent(g_memcard_hw_event_timeout) == 1) {
            return 3;
        }
        if (TestEvent(g_memcard_hw_event_new) == 1) {
            return 4;
        }
    }
}

void memcard_AckHwEvents(void) {
    TestEvent(g_memcard_hw_event_ioe);
    TestEvent(g_memcard_hw_event_err);
    TestEvent(g_memcard_hw_event_timeout);
    TestEvent(g_memcard_hw_event_new);
}

s32 memcard_CountFiles(s32 arg0, s32 arg1) {
    struct DIRENTRY *file_entry_cursor;
    s32 file_count;
    char path_pattern[32];

    file_entry_cursor = g_memcard_file_list;
    sprintf(path_pattern, (char *)g_str_memcard_fmt, arg0, arg1);
    file_count = 0;
    if (firstfile(path_pattern, file_entry_cursor) != 0) {
        do {
            file_count++;
            file_entry_cursor++;
        } while (nextfile(file_entry_cursor) != 0);
    }
    g_memcard_file_count = file_count;
    return file_count;
}

s32 memcard_CountFreeBlocks(void) {
    struct DIRENTRY *file_entry_cursor;
    s32 files_scanned;
    s32 file_count;
    /* `var_a0` sums the entries' byte sizes, then holds the block count
     * derived from that total (total / 8192), the way SOTN's cloaked knight
     * takes `distance = SquareRoot0(distance)` (Q51); returning
     * 0xF - (var_v0 >> var_5) directly: score 11 */
    /* SOTN: src/st/nz1/e_cloaked_knight.c:83 @db41b28 */
    s32 var_a0;
    /* the total biased for the signed shift (+0x1FFF when negative) */
    s32 var_v0;
    /* FAKE: constant holder for the 13 (SOTN new_var2 shape) raises var_a0's
       allocation priority; reload folds the 13 back in; literal 0xD: score 11
     */
    s32 var_5;

    var_5 = 0xD;
    files_scanned = 0;
    var_a0 = 0;
    file_count = g_memcard_file_count;
    if (files_scanned < file_count) {
        file_entry_cursor = g_memcard_file_list;
        do {
            var_a0 += file_entry_cursor++->size;
            files_scanned += 1;
        } while (files_scanned < file_count);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> var_5;
    return 0xF - var_a0;
}

s32 memcard_HasFile(u8 *requested_name) {
    s32 files_checked;
    s32 name_mismatch;
    struct DIRENTRY *file_entry_cursor;
    s8 *entry_name_cursor;
    s8 *requested_name_cursor;
    s8 *compare_end;
    s32 requested_byte;
    s32 entry_byte;
    s32 file_count;

    files_checked = 0;
    if (files_checked < g_memcard_file_count) {
        file_count = g_memcard_file_count;
        file_entry_cursor = g_memcard_file_list;
        while (files_checked < file_count) {
            name_mismatch = 0;
            entry_name_cursor = file_entry_cursor->name;
            requested_name_cursor = (s8 *)requested_name;
            /* the bound is 21 bytes: the compare runs one byte past name[20]
             * into attr, as the target does */
            compare_end = file_entry_cursor->name + 0x15;
            while (1) {
                requested_byte = (u8)*requested_name_cursor;
                if (requested_byte == 0) {
                    break;
                }
                entry_byte = (u8)*entry_name_cursor;
                if (requested_byte != entry_byte) {
                    goto block_6c;
                }
                entry_name_cursor += 1;
                requested_name_cursor += 1;
                if ((s32)entry_name_cursor >= (s32)compare_end) {
                    break;
                }
            }
        block_5c:
            files_checked += 1;
            if (name_mismatch != 0) {
                goto block_74;
            }
            return 1;
        block_6c:
            name_mismatch = 1;
            goto block_5c;
        block_74:
            file_entry_cursor++;
        }
    }
    return 0;
}

s32 memcard_ReadFile(
    s32 arg0, s32 arg1, s32 filename, void *read_dest, s32 byte_count) {
    char file_path[32];
    s32 fd;

    sprintf(file_path, (char *)D_800109BC, arg0, arg1, filename);
    fd = open(file_path, 0x8001);
    if (fd == -1) {
        return -1;
    }
    g_memcard_fd = fd;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    read(fd, read_dest, byte_count);
    return -(memcard_WaitHwEvent() != 1);
}

extern s32 write(s32, void *, s32); /* PsyQ: long write(long, void *, long) */

s32 memcard_WriteFile(s32 arg0, s32 arg1, s32 filename, void *write_src,
                      s32 arg4, s32 byte_count, s32 arg6) {
    char file_path[32];
    s32 fd;

    sprintf(file_path, (char *)D_800109BC, arg0, arg1, filename);
    if (arg6 != 0) {
        fd = open(file_path, (arg4 << 16) | 0x200);
        if (fd == -1) {
            return -1;
        }
        close(fd);
    }
    fd = open(file_path, 0x8002);
    if (fd == -1) {
        return -1;
    }
    g_memcard_fd = fd;
    memcard_AckSwEvents();
    memcard_AckHwEvents();
    write(fd, write_src, byte_count);
    return -(memcard_WaitHwEvent() != 1);
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
s32 g_memcard_fd;
s32 g_memcard_sw_event_ioe;
s32 g_memcard_sw_event_err;
s32 g_memcard_sw_event_timeout;
s32 g_memcard_sw_event_new;
s32 g_memcard_hw_event_ioe;
s32 g_memcard_hw_event_err;
s32 g_memcard_hw_event_timeout;
s32 g_memcard_hw_event_new;
s32 g_memcard_file_count;
s32 g_memcard_poll_count;
