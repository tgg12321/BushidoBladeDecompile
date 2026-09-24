/* Rodata sub-TU: the tail of the 101C.rodata_text1a_b_post cluster, split
 * off text1a_b_post_rodata.c at 0x80016240 so that system.o(.rodata) --
 * getintr's own switch jump table (0x8001622C, 20B) -- links between the
 * two halves (TU re-split recipe, jtbl-rodata-split-infrastructure.md). */
#include "common.h"

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

