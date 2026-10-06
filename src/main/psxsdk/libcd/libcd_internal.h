#ifndef LIBCD_INTERNAL_H
#define LIBCD_INTERNAL_H

/* PsyQ LIBCD library-internal state and the BIOS-layer (bios.c) entry points the command layer
 * (sys.c) calls (SOTN src/main/psxsdk/libcd/libcd_internal.h). */

#include <psxsdk/libcd.h>

extern s32 CD_status; /* Sony's int CD_status (bios.c) */
extern u8 CD_pos[4]; /* Sony's u_char CD_pos[4] (SOTN: src/main/psxsdk/libcd/bios.c:42 @aa53500) */
extern u8 CD_mode;
extern u8 CD_com;
extern CdlCB CD_cbsync;
extern CdlCB CD_cbready;
extern s32 CD_debug;
extern s32 CD_comstr[];
extern s32 CD_intstr[];

extern s32 CD_init(void);
extern void CD_initintr(void);
extern s32 CD_initvol(void);
extern void CD_flush(void);
extern s32 CD_sync(s32, u8 *);
extern s32 CD_ready(s32, u8 *);
extern s32 CD_cw(u8, u8 *, u8 *, s32);
extern s32 CD_vol(CdlATV *);
extern s32 CD_getsector(s32, s32);
extern s32 CD_getsector2(s32, s32);
extern s32 CD_datasync(s32);

/* CDREAD read-mode setter (not in PsyQ LIBCD.H; CdInit calls it). */
extern s32 CdReadMode(s32);

#endif /* LIBCD_INTERNAL_H */
