#ifndef PSXSDK_LIBCARD_H
#define PSXSDK_LIBCARD_H

/* PsyQ LIBCARD entry points (Sony's libcard.h; SOTN include/psxsdk/libcard.h), spelled as BB2's
 * code uses them (src/main/psxsdk/libcard/). */

#include "common.h"

extern void InitCARD(s32);
extern void StartCARD(void);
extern void StopCARD(void);
extern void _card_info(s32);
extern void _card_load(s32);
extern void _card_clear(s32);

#endif /* PSXSDK_LIBCARD_H */
