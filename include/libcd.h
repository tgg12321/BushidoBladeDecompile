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

/* libcd control entry points (system.c): com, param bytes, result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

#endif /* LIBCD_H */
