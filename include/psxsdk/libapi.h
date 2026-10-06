#ifndef PSXSDK_LIBAPI_H
#define PSXSDK_LIBAPI_H

/* PsyQ LIBAPI entry points (Sony's libapi.h; SOTN include/psxsdk/libapi.h). Most are BIOS vector
 * trampolines (src/main/psxsdk/libapi/). Each prototype agrees with its C / asm definition; where
 * that differs from PsyQ's LIBAPI.H spelling the entry carries a PsyQ: note. */

#include "common.h"
#include <psxsdk/kernel.h>

/* PsyQ's directory entry (sys/file.h), what firstfile / nextfile fill (sizeof = 0x28). */
struct DIRENTRY {
    char name[20];
    s32 attr;
    s32 size;
    struct DIRENTRY *next;
    s32 head;
    char system[4];
};

/* An SIO port's registers (hardware I/O, volatile at the use: mmio-volatile-type-level): port 0
 * (controllers / memory cards) at 0x1F801040, D_8009BD84 in pad.c; port 1 (link cable) at
 * 0x1F801050, libcomb's D_800A3044. */
typedef struct {
    u8 data;
    u8 unk1[3];
    u16 stat;
    u16 unk6;
    u16 mode;
    u16 ctrl;
    u16 misc;
    u16 baud;
} SioRegs;

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
extern struct DIRENTRY *firstfile(s32 *, struct DIRENTRY *); /* PsyQ: struct DIRENTRY *firstfile(char *, struct DIRENTRY *) */
extern struct DIRENTRY *nextfile(struct DIRENTRY *);

#endif /* PSXSDK_LIBAPI_H */
