#ifndef PSXSDK_LIBCD_H
#define PSXSDK_LIBCD_H

/* PsyQ LIBCD types and entry points (Sony's libcd.h; SOTN include/psxsdk/libcd.h). Each prototype
 * agrees with its C definition in src/main/psxsdk/libcd/; where that differs from PsyQ's
 * LIBCD.H spelling the entry carries a PsyQ: note. Library-internal state and entry
 * points shared by the modules: src/main/psxsdk/libcd/libcd_internal.h. */

#include "common.h"

/* libcd CdlATV, the attenuator block CdMix takes (CdMix(CdlATV *)): the CD-audio
 * mix currently applied (g_cd_atv, cdrom_SetMix) and the fade target
 * (D_800A36B8, func_80035F78) that func_80036140 steps toward and finally copies
 * over it with one struct assignment (the unaligned lwl/lwr/swl/swr at 80036310).
 * Both were tentative definitions in the CD module's file: ASPSX 2.34 gives such
 * a COMMON symbol gp only at its base, so byte 0 is gp-relative and bytes 1..3
 * are lui/%lo in all three accessors. Modelled by maspsx for every file from the
 * declarations (owner Q62); the tentative definitions are in
 * the CD module's two -G8 units, src/main/26730.c and src/main/26940.c. */
typedef struct {
    u8 val0;
    u8 val1;
    u8 val2;
    u8 val3;
} CdlATV;

/* libcd CdlLOC, a disc position: BCD minute / second / sector plus the track byte
 * (CdControl(CdlSetloc, ...), CdPosToInt, CdIntToPos). */
typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdlLOC;

/* libcd CdlCB: the sync / ready / read callback (the interrupt code, the result bytes). */
typedef void (*CdlCB)(u8, u8 *);

/* PsyQ 4.0 LIBCD cdread.c module state (BB2 links Sony's CDREAD object verbatim; SOTN's
 * psxsdk cdread.c names the same block D_80032DBC): one volatile block at 0x800A14D0,
 * preceded by CD_ReadCallbackFunc (g_CdReadCallback_func, 0x800A14CC) and followed by the
 * saved result pointer D_800A1504. Evidence it is one object: CdReadSync caches &t1 in $s1
 * and reads cnt / t2 / sectors at -0x8 / -0x4 / -0x1C off it (0x800827E8), and the la-form
 * member reads in cd_read_retry / CdReadBreak / CdRead / cb_read are cse's related-value
 * addressing of one symbol. All its users are in src/main/psxsdk/libcd/cdread.c (the Q99
 * admission in .claude/rules/aggregate-merge-family.md names this declaration). */
typedef struct {
    /* 0x00 */ s32 sectors; /* D_800A14D0 */
    /* 0x04 */ s32 buf;     /* D_800A14D4 */
    /* 0x08 */ s32 p;       /* D_800A14D8 */
    /* 0x0C */ s32 mode;    /* D_800A14DC */
    /* 0x10 */ s32 size;    /* D_800A14E0 */
    /* 0x14 */ s32 cnt;     /* D_800A14E4 */
    /* 0x18 */ s32 t2;      /* D_800A14E8 */
    /* 0x1C */ s32 t1;      /* D_800A14EC */
    /* 0x20 */ s32 pos;     /* D_800A14F0 */
    /* 0x24 */ CdlCB cbsync;  /* D_800A14F4 */
    /* 0x28 */ CdlCB cbready; /* D_800A14F8 */
    /* 0x2C */ s32 cbdata;  /* D_800A14FC */
    /* 0x30 */ s32 tslmode; /* D_800A1500 */
} CdlREAD;
extern volatile CdlREAD D_800A14D0;

/* libcd control entry points (src/main/psxsdk/libcd/sys.c): com, param bytes, result bytes. */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdControlB(u8 com, u8 *param, u8 *result);
s32 CdControlF(u8 com, u8 *param);

extern s32 CdInit(void);
extern s32 CdReset(s32);
extern void CdFlush(void);
extern s32 CdSetDebug(s32);
extern u32 CdStatus(void);   /* PsyQ: int CdStatus(void) */
extern u32 CdMode(void);     /* PsyQ: int CdMode(void) */
extern u32 CdLastCom(void);  /* PsyQ: int CdLastCom(void) */
extern void *CdLastPos(void); /* PsyQ: CdlLOC *CdLastPos(void) */
extern void *CdComstr(u8);   /* PsyQ: char *CdComstr(u_char) */
extern void *CdIntstr(u8);   /* PsyQ: char *CdIntstr(u_char) */
extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
extern CdlCB CdSyncCallback(CdlCB);
extern CdlCB CdReadyCallback(CdlCB);
extern s32 CdDataCallback(s32);  /* PsyQ: void (*CdDataCallback(void (*func)())) */
extern void CdDataSync(s32);     /* PsyQ: int CdDataSync(int) */
extern s32 CdGetSector(s32, s32);  /* PsyQ: int CdGetSector(void *, int) */
extern s32 CdGetSector2(s32, s32); /* PsyQ: int CdGetSector2(void *, int) */
extern s32 CdMix(CdlATV *);
extern CdlLOC *CdIntToPos(s32, CdlLOC *);
extern s32 CdPosToInt(CdlLOC *);
extern s32 CdRead(s32, s32, s32);  /* PsyQ: int CdRead(int, u_long *, int) */
extern s32 CdReadSync(s32, s32);   /* PsyQ: int CdReadSync(int, u_char *) */
extern CdlCB CdReadCallback(CdlCB);
extern void CdReadBreak(void);

#endif /* PSXSDK_LIBCD_H */
