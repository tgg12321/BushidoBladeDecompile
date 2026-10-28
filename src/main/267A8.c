/* CD and sound start-up: snd_SerialMixOn, cdrom_Init, cdrom_FlushInit,
 * cdrom_ReadyCallback. .text 0x80035FA8 (ROM 0x267A8). Start boundary: G8 (the
 * end of 26730.c's -G8 unit). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

void snd_SerialMixOn(void) {
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
}

extern u8 D_800A31E4;

void cdrom_Init(void) {
    CdInit();
    CdSetDebug(0);
    cdrom_SetMix(0, 0, 0, 0);
    g_cdrom_state.rec.io_state = 0;
    if (D_800A31E4 == 0) {
        D_800A31E4 = 1;
    }
}

void cdrom_FlushInit(void) {
    CdFlush();
    CdInit();
    VSync(4);
}

void cdrom_ReadyCallback(u8 arg0, u8 *result) {
    s32 sp[4];

    if (arg0 == 1) {
        g_cdrom_state.rec.read_timeout = 0;
        if (g_cdrom_state.rec.cd20 <= 0) {
            return;
        }
        CdGetSector((s32)sp, 3);
        if (CdPosToInt((CdlLOC *)sp) != g_cdrom_state.rec.expected_pos) {
            g_cdrom_state.rec.cd20 = -2;
        } else {
            CdGetSector(g_cdrom_state.rec.dest_buffer, 0x200);
            g_cdrom_state.rec.dest_buffer += 0x800;
            g_cdrom_state.rec.cd20--;
            g_cdrom_state.rec.expected_pos++;
            if (g_cdrom_state.rec.cd20 != 0) {
                return;
            }
        }
    } else {
        g_cdrom_state.rec.cd20 = -1;
    }
    CdReadyCallback(0);
    CdControlF(9, 0);
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
u8 D_800A31E4 = 0;
