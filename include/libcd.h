#ifndef LIBCD_H
#define LIBCD_H

#include "common.h"

/* libcd CdlATV, the attenuator block CdMix takes (CdMix(CdlATV *)): the CD-audio
 * mix currently applied (g_cd_atv, cdrom_SetMix) and the fade target
 * (D_800A36B8, func_80035F78) that func_80036140 steps toward and finally copies
 * over it with one struct assignment (the unaligned lwl/lwr/swl/swr at 80036310).
 * Both were tentative definitions in the CD module's file: ASPSX 2.34 gives such
 * a COMMON symbol gp only at its base, so byte 0 is gp-relative and bytes 1..3
 * are lui/%lo in all three accessors. Modelled by maspsx for every file from the
 * declarations (owner Q62); the tentative definitions are in
 * the CD module's two -G8 units, src/code6cac_b4.c and src/code6cac_b5.c. */
typedef struct {
    u8 val0;
    u8 val1;
    u8 val2;
    u8 val3;
} CdlATV;

/* PsyQ 4.0 LIBCD cdread.c module state (BB2 links Sony's CDREAD object verbatim; SOTN's
 * psxsdk cdread.c names the same block D_80032DBC): one volatile block at 0x800A14D0,
 * preceded by CD_ReadCallbackFunc (g_CdReadCallback_func, 0x800A14CC) and followed by the
 * saved result pointer D_800A1504. Evidence it is one object: CdReadSync caches &t1 in $s1
 * and reads cnt / t2 / sectors at -0x8 / -0x4 / -0x1C off it (0x800827E8), and the la-form
 * member reads in cd_read_retry / CdReadBreak / CdRead / cb_read are cse's related-value
 * addressing of one symbol. Members: system.c (CdRead family), ings2.c (CdReadMode). */
typedef struct {
    /* 0x00 */ s32 sectors; /* D_800A14D0 */
    /* 0x04 */ s32 buf;     /* D_800A14D4 */
    /* 0x08 */ s32 p;       /* D_800A14D8 */
    /* 0x0C */ s32 mode;    /* D_800A14DC */
    /* 0x10 */ s32 size;    /* D_800A14E0 */
    /* 0x14 */ s32 cnt;     /* D_800A14E4 */
    /* 0x18 */ s32 t2;      /* D_800A14E8 */
    /* 0x1C */ s32 t1;      /* D_800A14EC */
    /* 0x20 */ s32 pos;     /* D_800A14F0 */
    /* 0x24 */ s32 cbsync;  /* D_800A14F4 */
    /* 0x28 */ s32 cbready; /* D_800A14F8 */
    /* 0x2C */ s32 cbdata;  /* D_800A14FC */
    /* 0x30 */ s32 tslmode; /* D_800A1500 */
} CdlREAD;
extern volatile CdlREAD D_800A14D0;

/* libcd control entry points (system.c): com, param bytes, result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

#endif /* LIBCD_H */
