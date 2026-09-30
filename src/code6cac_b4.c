/* The CD-mix setters cdrom_SetMix and func_80035F78, moved unchanged out of
 * code6cac_b2_post.c into their own translation unit compiled -G8 (Makefile
 * GP_FILES): their original bytes write g_cd_atv / D_800A36B8 / D_800A3854 /
 * D_800A3840 straight off $gp, which the original compiler emits only at -G8.
 * Both are INCLUDE_ASM again (2026-09-30): their C matched only through the
 * per-function maspsx COMMON gate, which the owner ruled a cheat. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern void CdMix(CdlATV *);
extern s16 D_800A3854;
INCLUDE_ASM("asm/funcs", cdrom_SetMix);
extern s16 D_800A3840;
INCLUDE_ASM("asm/funcs", func_80035F78);
