/* Rodata sub-TU split out for the 101C.rodata_text1a_b_post cluster
 * (rodata-cleanup project, docs/rodata-cleanup-project.md, 2026-06-09).
 * MULTI-FILE cluster: 68 symbols spanning display.c, ings2.c, system.c,
 * text1b_b.c, and others (per the inventory CSV). Sub-TU pattern packs
 * all the bytes into one file that takes the asm/data slot (between
 * text1b_b.o and main.o in bb2.ld). Jtbl entries use literal hex
 * addresses; the script now resolves named function-symbol references
 * via undefined_syms_auto.txt + symbol_addrs.txt or falls back to the
 * .s comment column. */
#include "common.h"

/* Auto-extracted from asm/data/101C.rodata_text1a_b_post.s */

/* D_80015D58: 1 string(s), 24B @ 0x80015D58 */
const char D_80015D58[24] =
    "tpage: (%d,%d,%d,%d)\n\0\0\0"
    ;

/* D_80015D70: 1 string(s), 16B @ 0x80015D70 */
const char D_80015D70[16] =
    "clut: (%d,%d)\n\0\0"
    ;

/* D_80015D80: 1 string(s), 24B @ 0x80015D80 */
const char D_80015D80[24] =
    "clip (%3d,%3d)-(%d,%d)\n\0"
    ;

/* D_80015D98: 1 string(s), 16B @ 0x80015D98 */
const char D_80015D98[16] =
    "ofs  (%3d,%3d)\n\0"
    ;

/* D_80015DA8: 1 string(s), 24B @ 0x80015DA8 */
const char D_80015DA8[24] =
    "tw   (%d,%d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015DC0: 1 string(s), 12B @ 0x80015DC0 */
const char D_80015DC0[12] =
    "dtd   %d\n\0\0\0"
    ;

/* D_80015DCC: 1 string(s), 12B @ 0x80015DCC */
const char D_80015DCC[12] =
    "dfe   %d\n\0\0\0"
    ;

/* D_80015DD8: 1 string(s), 28B @ 0x80015DD8 */
const char D_80015DD8[28] =
    "disp   (%3d,%3d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015DF4: 1 string(s), 28B @ 0x80015DF4 */
const char D_80015DF4[28] =
    "screen (%3d,%3d)-(%d,%d)\n\0\0\0"
    ;

/* D_80015E10: 1 string(s), 12B @ 0x80015E10 */
const char D_80015E10[12] =
    "isinter %d\n\0"
    ;

/* D_80015E1C: 2 string(s), 64B @ 0x80015E1C */
const char D_80015E1C[64] =
    "isrgb24 %d\n\0$Id: sys.c,v 1.129 1"
    "996/12/25 03:36:20 noda Exp $\0\0\0"
    ;

/* D_80015E5C: 1 string(s), 32B @ 0x80015E5C */
const char D_80015E5C[32] =
    "ResetGraph:jtb=%08x,env=%08x\n\0\0\0"
    ;

/* D_80015E7C: 1 string(s), 20B @ 0x80015E7C */
const char D_80015E7C[20] =
    "ResetGraph(%d)...\n\0\0"
    ;

/* D_80015E90: 1 string(s), 24B @ 0x80015E90 */
const char D_80015E90[24] =
    "SetGraphReverse(%d)...\n\0"
    ;

/* D_80015EA8: 1 string(s), 44B @ 0x80015EA8 */
const char D_80015EA8[44] =
    "SetGraphDebug:level:%d,type:%d r"
    "everse:%d\n\0\0"
    ;

/* D_80015ED4: 1 string(s), 20B @ 0x80015ED4 */
const char D_80015ED4[20] =
    "SetGrapQue(%d)...\n\0\0"
    ;

/* D_80015EE8: 1 string(s), 28B @ 0x80015EE8 */
const char D_80015EE8[28] =
    "DrawSyncCallback(%08x)...\n\0\0"
    ;

/* g_str_setdispmask: 1 string(s), 20B @ 0x80015F04 */
const char g_str_setdispmask[20] =
    "SetDispMask(%d)...\n\0"
    ;

/* g_str_drawsync: 1 string(s), 20B @ 0x80015F18 */
const char g_str_drawsync[20] =
    "DrawSync(%d)...\n\0\0\0\0"
    ;

/* D_80015F2C: 1 string(s), 12B @ 0x80015F2C */
const char D_80015F2C[12] =
    "%s:bad RECT\0"
    ;

/* D_80015F38: 1 string(s), 20B @ 0x80015F38 */
const char D_80015F38[20] =
    "(%d,%d)-(%d,%d)\n\0\0\0\0"
    ;

/* D_80015F4C: 1 string(s), 4B @ 0x80015F4C */
const char D_80015F4C[4] =
    "%s:\0"
    ;

/* g_str_clearimage: 1 string(s), 12B @ 0x80015F50 */
const char g_str_clearimage[12] =
    "ClearImage\0\0"
    ;

/* g_str_loadimage: 1 string(s), 12B @ 0x80015F5C */
const char g_str_loadimage[12] =
    "LoadImage\0\0\0"
    ;

/* g_str_storeimage: 1 string(s), 12B @ 0x80015F68 */
const char g_str_storeimage[12] =
    "StoreImage\0\0"
    ;

/* D_80015F74: 1 string(s), 12B @ 0x80015F74 */
const char D_80015F74[12] =
    "MoveImage\0\0\0"
    ;

/* g_str_clearotag: 1 string(s), 24B @ 0x80015F80 */
const char g_str_clearotag[24] =
    "ClearOTag(%08x,%d)...\n\0\0"
    ;

/* D_80015F98: 1 string(s), 24B @ 0x80015F98 */
const char D_80015F98[24] =
    "ClearOTagR(%08x,%d)...\n\0"
    ;

/* g_str_drawotag: 1 string(s), 20B @ 0x80015FB0 */
const char g_str_drawotag[20] =
    "DrawOTag(%08x)...\n\0\0"
    ;

/* g_str_putdrawenv: 1 string(s), 24B @ 0x80015FC4 */
const char g_str_putdrawenv[24] =
    "PutDrawEnv(%08x)...\n\0\0\0\0"
    ;

/* D_80015FDC: 1 string(s), 28B @ 0x80015FDC */
const char D_80015FDC[28] =
    "DrawOTagEnv(%08x,&08x)...\n\0\0"
    ;

/* D_80015FF8: 1 string(s), 24B @ 0x80015FF8 */
const char D_80015FF8[24] =
    "PutDispEnv(%08x)...\n\0\0\0\0"
    ;

/* g_str_gpu_timeout: 1 string(s), 52B @ 0x80016010 */
const char g_str_gpu_timeout[52] =
    "GPU timeout:que=%d,stat=%08x,chc"
    "r=%08x,madr=%08x,\0\0\0"
    ;

/* D_80016044: 1 string(s), 24B @ 0x80016044 */
const char D_80016044[24] =
    "func=(%08x)(%08x,%08x)\n\0"
    ;

/* g_str_cdinit_fail: 1 string(s), 24B @ 0x8001605C */
const char g_str_cdinit_fail[24] =
    "CdInit: Init failed\n\0\0\0\0"
    ;

/* g_str_none: 1 string(s), 8B @ 0x80016074 (CdComstr / CdIntstr out-of-range name) */
const char g_str_none[8] =
    "none\0\0\0"
    ;

/* D_8001607C: 29 string(s), 316B @ 0x8001607C (the CD_comstr / CD_intstr names) */
const char D_8001607C[316] =
    "CdlReadS\0\0\0\0CdlSeekP\0\0\0\0"
    "CdlSeekL\0\0\0\0CdlGetTD\0\0\0\0CdlGetTN"
    "\0\0\0\0CdlGetlocP\0\0CdlGetlocL\0\0?\0\0\0"
    "CdlSetmode\0\0CdlSetfilter\0\0\0\0CdlD"
    "emute\0\0\0CdlMute\0CdlReset\0\0\0\0CdlP"
    "ause\0\0\0\0CdlStop\0CdlStandby\0\0CdlR"
    "eadN\0\0\0\0CdlBackward\0CdlForward\0\0"
    "CdlPlay\0CdlSetloc\0\0\0CdlNop\0\0CdlS"
    "ync\0DiskError\0\0\0DataEnd\0Acknowle"
    "dge\0Complete\0\0\0\0DataReady\0\0\0NoIn"
    "tr\0\0"
    ;

/* D_800161B8: 1 string(s), 16B @ 0x800161B8 */
const char D_800161B8[16] =
    "CD timeout: \0\0\0\0"
    ;

/* D_800161C8: 1 string(s), 28B @ 0x800161C8 */
const char D_800161C8[28] =
    "%s:(%s) Sync=%s, Ready=%s\n\0\0"
    ;

/* D_800161E4: 1 string(s), 12B @ 0x800161E4 */
const char D_800161E4[12] =
    "DiskError: \0"
    ;

/* D_800161F0: 1 string(s), 28B @ 0x800161F0 */
const char D_800161F0[28] =
    "com=%s,code=(%02x:%02x)\n\0\0\0\0"
    ;

/* D_8001620C: 1 string(s), 20B @ 0x8001620C */
const char D_8001620C[20] =
    "CDROM: unknown intr\0"
    ;

/* D_80016220: 2 string(s), 12B @ 0x80016220 */
const char D_80016220[12] =
    "(%d)\n\0\0\0\0\0\0\0"
    ;
