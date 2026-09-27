/* The CD-mix setters cdrom_SetMix and func_80035F78, moved unchanged out of
 * code6cac_b2_post.c into their own translation unit compiled -G8 (Makefile
 * GP_FILES): their original bytes write g_cd_atv / D_800A36B8 / D_800A3854 /
 * D_800A3840 straight off $gp, which the original compiler emits only at -G8.
 * The declarations below are the source file's own, for the names this code uses. */
#include "common.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern void CdMix(CdlATV *);
extern s16 D_800A3854;
void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    g_cd_atv.val0 = (u8)arg0;
    g_cd_atv.val1 = (u8)arg1;
    g_cd_atv.val2 = (u8)arg2;
    g_cd_atv.val3 = (u8)arg3;
    CdMix(&g_cd_atv);
    D_800A3854 = 0;
}
extern s16 D_800A3840;
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8.val0 = (u8)arg1;
    D_800A36B8.val1 = (u8)arg2;
    D_800A36B8.val2 = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36B8.val3 = (u8)arg4;
}
