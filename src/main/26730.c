/* The CD-mix setters cdrom_SetMix and cdrom_StartMixFade. .text 0x80035F30 (ROM
 * 0x26730). Start boundary: G8: compiled -G8, as their stores straight off $gp
 * show. g_cd_atv and g_cd_atv_target are file-scope tentative definitions
 * (COMMON), which Sony's assembler addressed gp-relative at their base only
 * (owner ruling Q62, 2026-09-30, global COMMON model). */
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
    g_cd_fade_len = 0;
}

CdlATV g_cd_atv_target;

void cdrom_StartMixFade(
    s16 fade_length, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    g_cd_atv_target.val0 = (u8)arg1;
    g_cd_atv_target.val1 = (u8)arg2;
    g_cd_atv_target.val2 = (u8)arg3;
    g_cd_fade_len = fade_length;
    g_cd_fade_step = 0;
    g_cd_atv_target.val3 = (u8)arg4;
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
CdlATV g_cd_atv_target;
CdlATV g_cd_atv;
s16 g_cd_fade_step;
s16 g_cd_fade_len;
