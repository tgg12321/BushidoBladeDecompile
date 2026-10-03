/* .rodata 0x80016074..0x8001622C: the LIBCD SYS and BIOS strings (the CD_comstr / CD_intstr
 * names and the CD timeout / DiskError messages), read by src/system.c. A data-only file between
 * libcd/event.o and system.o in .rodata; restructure step 4e folds it into its LIBCD modules. The
 * LIBGPU and LIBCD EVENT strings that preceded them moved to libgpu/prim.c, libgpu/sys.c and
 * libcd/event.c (step 4d, Q106 D4). */
#include "common.h"

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
