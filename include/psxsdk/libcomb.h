#ifndef PSXSDK_LIBCOMB_H
#define PSXSDK_LIBCOMB_H

/* PsyQ LIBCOMB (link-cable driver) entry points (Sony's libcomb.h), spelled as BB2's code uses
 * them (src/main/psxsdk/libcomb/comb.c). */

#include "common.h"

extern void AddCOMB(void);
extern void DelCOMB(void);
extern void ChangeClearSIO(s32);

#endif /* PSXSDK_LIBCOMB_H */
