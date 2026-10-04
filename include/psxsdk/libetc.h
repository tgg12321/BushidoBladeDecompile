#ifndef PSXSDK_LIBETC_H
#define PSXSDK_LIBETC_H

/* PsyQ LIBETC entry points (Sony's libetc.h; SOTN include/psxsdk/libetc.h). Each prototype agrees
 * with its C definition in src/main/psxsdk/libetc/; where that differs from PsyQ's LIBETC.H
 * spelling the entry carries a PsyQ: note. Library-internal entry points:
 * src/main/psxsdk/libetc/libetc_internal.h. */

#include "common.h"

extern s32 CheckCallback(void);
extern s32 ResetCallback(void);
extern s32 StopCallback(void);
extern s32 RestartCallback(void);
extern s32 VSync(s32);
extern void VSyncCallback(s32); /* PsyQ: int VSyncCallback(void (*)(void)) */
extern s32 GetVideoMode(void);
extern s32 SetVideoMode(s32);

#endif /* PSXSDK_LIBETC_H */
