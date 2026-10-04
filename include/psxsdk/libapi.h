#ifndef PSXSDK_LIBAPI_H
#define PSXSDK_LIBAPI_H

/* PsyQ LIBAPI entry points (Sony's libapi.h; SOTN include/psxsdk/libapi.h). Most are BIOS vector
 * trampolines (src/main/psxsdk/libapi/). Each prototype agrees with its definition and every
 * caller; where that differs from PsyQ's LIBAPI.H spelling the entry carries a PsyQ: note. */

#include "common.h"
#include <psxsdk/kernel.h>

/* PsyQ: long SetRCnt(unsigned long, unsigned short, long) -- rcnt_StartCnt1 (368E4) passes the
 * target -1 as a full word (li -1), which a u16 parameter would turn into 0xFFFF. */
extern s32 SetRCnt(u32, s32, s32);
extern s32 GetRCnt(u32);
extern s32 ResetRCnt(u32);
extern s32 StartRCnt(u32);
extern s32 StopRCnt(u32);
extern void DeliverEvent(u32, u32);
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void EnableEvent(s32);   /* PsyQ: long EnableEvent(long) */
extern void CloseEvent(s32);    /* PsyQ: long CloseEvent(long) */
extern s32 TestEvent(s32);
extern void ReturnFromException(void);
extern void ResetEntryInt(void);         /* PsyQ: not in LIBAPI.H */
extern void HookEntryInt(s32 *);         /* PsyQ: not in LIBAPI.H */
extern void ChangeClearRCnt(s32, s32);   /* PsyQ: not in LIBAPI.H */
extern void StartPAD(void);     /* PsyQ: long StartPAD(void) */
extern void StopPAD(void);
extern void ChangeClearPAD(s32);
extern void FlushCache(void);
extern void SetSp(u32);         /* PsyQ: unsigned long SetSp(unsigned long) */
extern void SetMem(s32);
extern void Exec(struct EXEC *, s32, s32 *); /* PsyQ: long Exec(struct EXEC *, long, char **) */
extern void AddDrv(s32 *);               /* PsyQ: not in LIBAPI.H */
extern s32 open(s32 *, s32);    /* PsyQ: long open(char *, unsigned long) */
extern void read(s32, s32 *, s32); /* PsyQ: long read(long, void *, long) */
extern void close(s32);         /* PsyQ: long close(long) */
extern s32 format(s32 *);       /* PsyQ: long format(char *) */
extern s32 firstfile(s32 *, s32 *); /* PsyQ: struct DIRENTRY *firstfile(char *, struct DIRENTRY *) */
extern s32 nextfile(s32 *);     /* PsyQ: struct DIRENTRY *nextfile(struct DIRENTRY *) */

#endif /* PSXSDK_LIBAPI_H */
