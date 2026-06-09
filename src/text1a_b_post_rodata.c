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

/* jtbl_80015A54: 5 words (20B) @ 0x80015A54 */
const u32 jtbl_80015A54[5] = {
    0x80077EE4,
    0x800780C8,
    0x80077FE0,
    0x800781CC,
    0x80078254,
};

/* D_80015A68: 1 string(s), 20B @ 0x80015A68 */
const char D_80015A68[20] =
    "0123456789abcdef\0\0\0\0"
    ;

/* D_80015A7C: 1 string(s), 8B @ 0x80015A7C */
const char D_80015A7C[8] =
    "(null)\0\0"
    ;

/* D_80015A84: 1 string(s), 20B @ 0x80015A84 */
const char D_80015A84[20] =
    "0123456789ABCDEF\0\0\0\0"
    ;

/* jtbl_80015A98: 121 words (484B) @ 0x80015A98 */
const u32 jtbl_80015A98[121] = {
    0x800792A4,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x800792FC,
    0x80079878,
    0x80079878,
    0x8007930C,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079314,
    0x80079338,
    0x80079878,
    0x80079330,
    0x80079340,
    0x80079878,
    0x800793CC,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x800793D4,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079460,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x8007942C,
    0x80079878,
    0x80079878,
    0x800794E4,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x8007958C,
    0x80079878,
    0x80079878,
    0x800795C4,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079444,
    0x80079464,
    0x80079878,
    0x80079878,
    0x80079878,
    0x80079434,
    0x80079464,
    0x80079878,
    0x80079878,
    0x8007943C,
    0x80079878,
    0x800794A8,
    0x800794E8,
    0x8007951C,
    0x80079878,
    0x80079878,
    0x8007952C,
    0x80079878,
    0x80079590,
    0x80079878,
    0x80079878,
    0x800795D0,
};

/* D_80015C7C: 1 string(s), 20B @ 0x80015C7C */
const char D_80015C7C[20] =
    "0123456789ABCDEF\0\0\0\0"
    ;

/* D_80015C90: 1 string(s), 20B @ 0x80015C90 */
const char D_80015C90[20] =
    "0123456789abcdef\0\0\0\0"
    ;

/* jtbl_80015CA4: 45 words (180B) @ 0x80015CA4 */
const u32 jtbl_80015CA4[45] = {
    0x80079D10,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x80079FA8,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x8007A0B0,
    0x80079D3C,
    0x8007A188,
    0x8007A188,
    0x8007A188,
    0x80079CF8,
    0x80079D3C,
    0x8007A188,
    0x8007A188,
    0x80079D04,
    0x8007A188,
    0x8007A154,
    0x80079EA4,
    0x80079F94,
    0x8007A188,
    0x8007A188,
    0x8007A0D0,
    0x8007A188,
    0x80079D8C,
    0x8007A188,
    0x8007A188,
    0x80079FB8,
};

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

/* D_80015F04: 1 string(s), 20B @ 0x80015F04 */
const char D_80015F04[20] =
    "SetDispMask(%d)...\n\0"
    ;

/* D_80015F18: 1 string(s), 20B @ 0x80015F18 */
const char D_80015F18[20] =
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

/* D_80015F50: 1 string(s), 12B @ 0x80015F50 */
const char D_80015F50[12] =
    "ClearImage\0\0"
    ;

/* D_80015F5C: 1 string(s), 12B @ 0x80015F5C */
const char D_80015F5C[12] =
    "LoadImage\0\0\0"
    ;

/* D_80015F68: 1 string(s), 12B @ 0x80015F68 */
const char D_80015F68[12] =
    "StoreImage\0\0"
    ;

/* D_80015F74: 1 string(s), 12B @ 0x80015F74 */
const char D_80015F74[12] =
    "MoveImage\0\0\0"
    ;

/* D_80015F80: 1 string(s), 24B @ 0x80015F80 */
const char D_80015F80[24] =
    "ClearOTag(%08x,%d)...\n\0\0"
    ;

/* D_80015F98: 1 string(s), 24B @ 0x80015F98 */
const char D_80015F98[24] =
    "ClearOTagR(%08x,%d)...\n\0"
    ;

/* D_80015FB0: 1 string(s), 20B @ 0x80015FB0 */
const char D_80015FB0[20] =
    "DrawOTag(%08x)...\n\0\0"
    ;

/* D_80015FC4: 1 string(s), 24B @ 0x80015FC4 */
const char D_80015FC4[24] =
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

/* D_80016010: 1 string(s), 52B @ 0x80016010 */
const char D_80016010[52] =
    "GPU timeout:que=%d,stat=%08x,chc"
    "r=%08x,madr=%08x,\0\0\0"
    ;

/* D_80016044: 1 string(s), 24B @ 0x80016044 */
const char D_80016044[24] =
    "func=(%08x)(%08x,%08x)\n\0"
    ;

/* D_8001605C: 1 string(s), 24B @ 0x8001605C */
const char D_8001605C[24] =
    "CdInit: Init failed\n\0\0\0\0"
    ;

/* D_80016074: 30 string(s), 324B @ 0x80016074 */
const char D_80016074[324] =
    "none\0\0\0\0CdlReadS\0\0\0\0CdlSeekP\0\0\0\0"
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

/* jtbl_8001622C: 5 words (20B) @ 0x8001622C */
const u32 jtbl_8001622C[5] = {
    0x80080BE0,
    0x80080B94,
    0x80080A94,
    0x80080C64,
    0x80080CE8,
};

/* D_80016240: 1 string(s), 8B @ 0x80016240 */
const char D_80016240[8] =
    "CD_sync\0"
    ;

/* D_80016248: 1 string(s), 12B @ 0x80016248 */
const char D_80016248[12] =
    "CD_ready\0\0\0\0"
    ;

/* D_80016254: 1 string(s), 8B @ 0x80016254 */
const char D_80016254[8] =
    "%s...\n\0\0"
    ;

/* D_8001625C: 1 string(s), 16B @ 0x8001625C */
const char D_8001625C[16] =
    "%s: no param\n\0\0\0"
    ;

/* D_8001626C: 2 string(s), 60B @ 0x8001626C */
const char D_8001626C[60] =
    "CD_cw\0\0\0$Id: bios.c,v 1.86 1997/"
    "03/28 07:42:42 makoto Exp $\0"
    ;

/* D_800162A8: 1 string(s), 12B @ 0x800162A8 */
const char D_800162A8[12] =
    "CD_init:\0\0\0\0"
    ;

/* D_800162B4: 1 string(s), 12B @ 0x800162B4 */
const char D_800162B4[12] =
    "addr=%08x\n\0\0"
    ;

/* D_800162C0: 1 string(s), 12B @ 0x800162C0 */
const char D_800162C0[12] =
    "CD_datasync\0"
    ;

/* D_800162CC: 1 string(s), 8B @ 0x800162CC */
const char D_800162CC[8] =
    "<NULL>\0\0"
    ;

/* D_800162D4: 1 string(s), 24B @ 0x800162D4 */
const char D_800162D4[24] =
    "CdRead: sector error\n\0\0\0"
    ;

/* D_800162EC: 1 string(s), 24B @ 0x800162EC */
const char D_800162EC[24] =
    "CdRead: Shell open...\n\0\0"
    ;

/* D_80016304: 1 string(s), 20B @ 0x80016304 */
const char D_80016304[20] =
    "CdRead: retry...\n\0\0\0"
    ;

/* D_80016318: 2 string(s), 68B @ 0x80016318 */
const char D_80016318[68] =
    "VSync: timeout\n\0$Id: intr.c,v 1."
    "76 1997/02/12 12:45:05 makoto Ex"
    "p $\0"
    ;

/* D_8001635C: 1 string(s), 28B @ 0x8001635C */
const char D_8001635C[28] =
    "unexpected interrupt(%04x)\n\0"
    ;

/* D_80016378: 1 string(s), 28B @ 0x80016378 */
const char D_80016378[28] =
    "intr timeout(%04x:%04x)\n\0\0\0\0"
    ;

/* D_80016394: 1 string(s), 28B @ 0x80016394 */
const char D_80016394[28] =
    "DMA bus error: code=%08x\n\0\0\0"
    ;

/* D_800163B0: 1 string(s), 16B @ 0x800163B0 */
const char D_800163B0[16] =
    "MADR[%d]=%08x\n\0\0"
    ;

