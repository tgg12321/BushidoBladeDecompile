#ifndef PSXSDK_LIBCD_H
#define PSXSDK_LIBCD_H

/* PsyQ LIBCD types and entry points (Sony's libcd.h; SOTN
 * include/psxsdk/libcd.h). Each prototype agrees with its C definition in
 * src/main/psxsdk/libcd/; where that differs from PsyQ's LIBCD.H spelling the
 * entry carries a PsyQ: note. Library-internal state and entry points shared by
 * the modules: src/main/psxsdk/libcd/libcd_internal.h. */

#include "common.h"

/* libcd CdlATV, the attenuator block CdMix takes: the CD-audio mix currently
 * applied (g_cd_atv, cdrom_SetMix) and the fade target (D_800A36B8) that
 * func_80036140 steps toward and finally copies over it. Both are COMMON
 * symbols, gp-relative only at byte 0 (ASPSX 2.34, Q62). */
typedef struct {
    u8 val0;
    u8 val1;
    u8 val2;
    u8 val3;
} CdlATV;

/* libcd CdlLOC, a disc position: BCD minute / second / sector plus the track
 * byte (CdControl(CdlSetloc, ...), CdPosToInt, CdIntToPos). */
typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdlLOC;

/* libcd CdlCB: the sync / ready / read callback (the interrupt code, the result
 * bytes). */
typedef void (*CdlCB)(u8, u8 *);

/* PsyQ 4.0 LIBCD cdread.c module state (SOTN's cdread.c: D_80032DBC): one
 * volatile block at 0x800A14D0, between g_CdReadCallback_func and the saved
 * result pointer D_800A1504. One object: CdReadSync reads cnt / t2 / sectors
 * off a cached &t1. Users are all in libcd/cdread.c (Q99,
 * aggregate-merge-family). */
typedef struct {
    /* 0x00 */ s32 sectors;   /* D_800A14D0 */
    /* 0x04 */ s32 buf;       /* D_800A14D4 */
    /* 0x08 */ s32 p;         /* D_800A14D8 */
    /* 0x0C */ s32 mode;      /* D_800A14DC */
    /* 0x10 */ s32 size;      /* D_800A14E0 */
    /* 0x14 */ s32 cnt;       /* D_800A14E4 */
    /* 0x18 */ s32 t2;        /* D_800A14E8 */
    /* 0x1C */ s32 t1;        /* D_800A14EC */
    /* 0x20 */ s32 pos;       /* D_800A14F0 */
    /* 0x24 */ CdlCB cbsync;  /* D_800A14F4 */
    /* 0x28 */ CdlCB cbready; /* D_800A14F8 */
    /* 0x2C */ s32 cbdata;    /* D_800A14FC */
    /* 0x30 */ s32 tslmode;   /* 0x800A1500 */
} CdlREAD;

extern volatile CdlREAD D_800A14D0;

/* libcd control entry points (src/main/psxsdk/libcd/sys.c): com, param bytes,
 * result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

extern s32 CdInit(void);
extern s32 CdReset(s32);
extern void CdFlush(void);
extern s32 CdSetDebug(s32);
extern u32 CdStatus(void);    /* PsyQ: int CdStatus(void) */
extern u32 CdMode(void);      /* PsyQ: int CdMode(void) */
extern u32 CdLastCom(void);   /* PsyQ: int CdLastCom(void) */
extern void *CdLastPos(void); /* PsyQ: CdlLOC *CdLastPos(void) */
extern void *CdComstr(u8);    /* PsyQ: char *CdComstr(u_char) */
extern void *CdIntstr(u8);    /* PsyQ: char *CdIntstr(u_char) */
extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
extern CdlCB CdSyncCallback(CdlCB);
extern CdlCB CdReadyCallback(CdlCB);
/* PsyQ: void (*CdDataCallback(void (*func)())) */
extern s32 CdDataCallback(s32);
extern void CdDataSync(s32);       /* PsyQ: int CdDataSync(int) */
extern s32 CdGetSector(s32, s32);  /* PsyQ: int CdGetSector(void *, int) */
extern s32 CdGetSector2(s32, s32); /* PsyQ: int CdGetSector2(void *, int) */
extern s32 CdMix(CdlATV *);
extern CdlLOC *CdIntToPos(s32, CdlLOC *);
extern s32 CdPosToInt(CdlLOC *);
extern s32 CdRead(s32, s32, s32); /* PsyQ: int CdRead(int, u_long *, int) */
extern s32 CdReadSync(s32, s32);  /* PsyQ: int CdReadSync(int, u_char *) */
extern CdlCB CdReadCallback(CdlCB);
extern void CdReadBreak(void);

#endif /* PSXSDK_LIBCD_H */
