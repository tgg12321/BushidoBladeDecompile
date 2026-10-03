/* CD and sound start-up: snd_SerialMixOn, cdrom_Init, cdrom_FlushInit, cdrom_ReadyCallback. .text
 * 0x80035FA8 (ROM 0x267A8). Start boundary: G8 (the end of 26730.c's -G8 unit). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include <psxsdk/libsnd.h>
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern void VSync(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void func_800174F4(void);
extern void func_8003AAB0(void);
extern void snd_Quit(void);
extern void memcard_Quit(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void snd_SerialMixOn(void) {
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
}
extern u8 D_800A31E4;
void cdrom_Init(void) {
    CdInit();
    CdSetDebug(0);
    cdrom_SetMix(0, 0, 0, 0);
    D_80101E58.rec.unk02 = 0;
    if (D_800A31E4 == 0) {
        D_800A31E4 = 1;
    }
}
void cdrom_FlushInit(void) {
    CdFlush();
    CdInit();
    VSync(4);
}
extern void CdGetSector(s32, s32);
extern s32 CdPosToInt(s32);
void cdrom_ReadyCallback(u8 arg0) {
    s32 sp[4];
    if (arg0 == 1) {
        D_80101E58.rec.unk38 = 0;
        if (D_80101E58.rec.sectors_remaining <= 0) {
            return;
        }
        CdGetSector((s32)sp, 3);
        {
            s32 v0 = CdPosToInt((s32)sp);
            if (v0 != D_80101E58.rec.expected_pos) {
                D_80101E58.rec.sectors_remaining = -2;
                goto do_stop;
            }
        }
        CdGetSector(D_80101E58.rec.dest_buffer, 0x200);
        D_80101E58.rec.dest_buffer = D_80101E58.rec.dest_buffer + 0x800;
        D_80101E58.rec.sectors_remaining = D_80101E58.rec.sectors_remaining - 1;
        D_80101E58.rec.expected_pos = D_80101E58.rec.expected_pos + 1;
        if (D_80101E58.rec.sectors_remaining == 0) {
            goto do_stop;
        }
        return;
    } else {
        D_80101E58.rec.sectors_remaining = -1;
    }
do_stop:
    CdReadyCallback(0);
    CdControlF(9, 0);
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
u8 D_800A31E4 = 0;
