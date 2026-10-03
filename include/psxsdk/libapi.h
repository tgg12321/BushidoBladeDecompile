#ifndef PSXSDK_LIBAPI_H
#define PSXSDK_LIBAPI_H

/* PsyQ LIBAPI entry points (Sony's libapi.h; SOTN include/psxsdk/libapi.h), spelled as BB2's
 * code uses them. Most are BIOS vector trampolines (src/main/psxsdk/libapi/). */

#include "common.h"
#include <psxsdk/kernel.h>

extern void ExitCriticalSection(void);
extern void EnableEvent(s32);
extern void CloseEvent(s32);
extern s32 TestEvent(s32);
extern void ReturnFromException(void);
extern void ResetEntryInt(void);
extern void HookEntryInt(s32 *);
extern void ChangeClearRCnt(s32, s32);
extern s32 StopRCnt(s32);
extern void StartPAD(void);
extern void StopPAD(void);
extern void ChangeClearPAD(s32);
extern void FlushCache(void);
extern void SetSp(u32);
extern void SetMem(s32);
extern void Exec(struct EXEC *, s32, s32 *);
extern void AddDrv(s32 *);
extern s32 open(s32 *, s32);
extern void read(s32, s32 *, s32);
extern void close(s32);
extern s32 format(s32 *);
extern s32 firstfile(s32 *, s32 *);
extern s32 nextfile(s32 *);

#endif /* PSXSDK_LIBAPI_H */
