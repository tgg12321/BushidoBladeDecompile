#ifndef SYSTEM_H
#define SYSTEM_H

/* System - IRQ, file I/O, memory card */

#include "common.h"

/* Named globals */
/* The 0x24-byte file record at 0x80106A50 (func_80037F40 checksums it as one
 * block; func_800167EC initialises it). */
typedef struct {
    u8 unk_0;
    u8 unk_1;
    s32 unk_4;
} FileTimeRec;
typedef struct {
    s32 unk_00;             /* 0x80106A50 */
    u8 unk_04;              /* 0x80106A54 */
    u8 unk_05[3];
    FileTimeRec times[3];   /* 0x80106A58 */
    u8 color[3];            /* 0x80106A70 */
    u8 flags;               /* 0x80106A73: bits 0/1/2 = file_GetFlag0/1/2 */
} FileRecord;
extern FileRecord D_80106A50;
extern s16 D_800A3710;

/* PsyQ libcd bios.c's command-timeout alarm (Sony's Alarm_t {int, int, char *};
 * SOTN: src/main/psxsdk/libcd/bios.c:24 @aa53500; object map:
 * pre-slim-2026-10-01:memory/closer/libcd-identity.md): armed and polled by libcd's command
 * wait loops in src/main/psxsdk/libcd/bios.c. */
typedef struct {
    s32 time;   /* 0x800F19B8: VSync(-1) deadline */
    s32 count;  /* 0x800F19BC: poll count */
    char *name; /* 0x800F19C0: caller name for the timeout report */
} Alarm_t;
extern Alarm_t Alarm;

/* Functions */
extern void ResetCallback(void);

/* PsyQ libapi struct EXEC: the PS-EXE header body (0x3C bytes, 0x10 into the image). */
typedef struct EXEC {
    u32 pc0, gp0;
    u32 t_addr, t_size;
    u32 d_addr, d_size;
    u32 b_addr, b_size;
    u32 s_addr, s_size;
    u32 sp, fp, gp, ret, base;
} EXEC;

#endif /* SYSTEM_H */
