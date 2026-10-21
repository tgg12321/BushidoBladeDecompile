#ifndef PSXSDK_LIBCARD_H
#define PSXSDK_LIBCARD_H

/* PsyQ LIBCARD entry points (Sony's libcard.h; SOTN include/psxsdk/libcard.h),
 * spelled as BB2's code uses them (src/main/psxsdk/libcard/). */

#include "common.h"

extern void InitCARD(s32);
extern void StartCARD(void);
extern void StopCARD(void);
extern void _card_info(s32);
extern void _card_load(s32);
extern void _card_clear(s32);
/* PsyQ: long _card_write(long, long, unsigned char *) */
extern s32 _card_write(s32, s32, u8 *);
/* BIOS B(0x50) takes no arguments, but _card_clear's call passes its channel
 * through, so it stays unprototyped. */
extern void _new_card();
/* LIBCARD A74 / A75: the BIOS B(0x4A) / B(0x4B) trampolines InitCARD and
 * StartCARD call (PsyQ: not in LIBCARD.H). */
extern void InitCARD2(s32);
extern void StartCARD2(void);

#endif /* PSXSDK_LIBCARD_H */
