#ifndef PSXSDK_LIBSN_H
#define PSXSDK_LIBSN_H

/* SN Systems host-file (PCdrv) entry points (PsyQ's libsn.h), spelled as BB2's
 * code uses them (src/main/psxsdk/libsn/). */

#include "common.h"

extern s32 PCopen(s32, s32, s32);
extern void PCclose(s32);
extern s32 PClseek(s32, s32, s32);

#endif /* PSXSDK_LIBSN_H */
