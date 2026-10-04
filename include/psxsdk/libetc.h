#ifndef PSXSDK_LIBETC_H
#define PSXSDK_LIBETC_H

/* PsyQ LIBETC entry points (Sony's libetc.h; SOTN include/psxsdk/libetc.h), spelled as BB2's code
 * uses them (the module definitions in src/main/psxsdk/libetc/). */

#include "common.h"

extern void ResetCallback(void);
extern void StopCallback(void);
extern void RestartCallback(void);
extern s32 VSync(s32);
extern void VSyncCallback(s32);
extern s32 GetVideoMode(void);
extern s32 SetVideoMode(s32);

#endif /* PSXSDK_LIBETC_H */
