/* The CD-mix setters cdrom_SetMix and func_80035F78. .text 0x80035F30 (ROM 0x26730).
 * Start boundary: G8. Their own translation unit, compiled -G8 (Makefile
 * GP_FILES): their original bytes write g_cd_atv / D_800A36B8 / D_800A3854 /
 * D_800A3840 straight off $gp, which the original compiler emits only at -G8.
 * g_cd_atv and D_800A36B8 are declared here the way their bytes show the
 * original did: file-scope tentative definitions (no initializer; their
 * original bytes are zero). Sony's assembler gave such a COMMON variable gp at
 * its base only, never at an offset, which the target's stores show; maspsx
 * models that for every file (owner ruling Q62, 2026-09-30, global COMMON
 * model), not per function. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

CdlATV g_cd_atv;
void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    g_cd_atv.val0 = (u8)arg0;
    g_cd_atv.val1 = (u8)arg1;
    g_cd_atv.val2 = (u8)arg2;
    g_cd_atv.val3 = (u8)arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
CdlATV D_800A36B8;
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8.val0 = (u8)arg1;
    D_800A36B8.val1 = (u8)arg2;
    D_800A36B8.val2 = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36B8.val3 = (u8)arg4;
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
CdlATV D_800A36B8;
CdlATV g_cd_atv;
s16 D_800A3840;
s16 D_800A3854;
