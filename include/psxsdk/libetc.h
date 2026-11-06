#ifndef PSXSDK_LIBETC_H
#define PSXSDK_LIBETC_H

/* PsyQ LIBETC entry points (Sony's libetc.h; SOTN include/psxsdk/libetc.h).
 * Each prototype agrees with its C definition in src/main/psxsdk/libetc/; where
 * that differs from PsyQ's LIBETC.H spelling the entry carries a PsyQ: note.
 * Library-internal entry points: src/main/psxsdk/libetc/libetc_internal.h. */

#include "common.h"

extern s32 CheckCallback(void);
extern s32 ResetCallback(void);
extern s32 StopCallback(void);
extern s32 RestartCallback(void);
extern s32 VSync(s32);
extern void VSyncCallback(s32); /* PsyQ: int VSyncCallback(void (*)(void)) */
extern s32 GetVideoMode(void);
extern s32 SetVideoMode(s32);

#define PADL2 1
#define PADR2 2
#define PADR1 8
#define PADRup 0x10
#define PADRright 0x20
#define PADRdown 0x40
#define PADRleft 0x80
#define PADselect 0x100
#define PADi 0x200
#define PADj 0x400
#define PADLup 0x1000
#define PADLright 0x2000
#define PADLdown 0x4000
#define PADLleft 0x8000

#endif /* PSXSDK_LIBETC_H */
