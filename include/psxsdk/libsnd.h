#ifndef PSXSDK_LIBSND_H
#define PSXSDK_LIBSND_H

/* PsyQ LIBSND public types and entry points (Sony's libsnd.h; SOTN include/psxsdk/libsnd.h).
 * Type layouts and prototypes are spelled as BB2's code uses them (the module definitions in
 * src/main/psxsdk/libsnd/). Library-internal state: src/main/psxsdk/libsnd/libsnd_i.h. */

#include "common.h"

/* PsyQ ProgAtr (libsnd.h) - program attribute record, 16 bytes; BB2 reads
   reserved2 as two u16 halves (VAG start-address pair). _svm_pg table. */
typedef struct {
    u8 tones;
    u8 mvol;
    u8 prior;
    u8 mode;
    u8 mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1;
    u16 reserved2;
    u16 reserved3;
} ProgAtr;

/* PsyQ VagAtr (libsnd.h) - tone attribute record, 32 bytes; _svm_tn table. */
typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag;
    s16 reserved[4];
} VagAtr;

/* PsyQ VabHdr (libsnd) — VAB bank header */
typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u8 vs;
    u8 vspad;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;

extern void SsInit(void);
extern void SsStart(void);
extern void SsEnd(void);
extern void SsQuit(void);
extern void SsSetTickMode(s32);
extern void SsSetStereo(void);
extern void SsSetMono(void);
extern void SsSetMVol(s16, s16);
extern void SsSetSerialVol(s16, s16, s16);
extern s32 SsSetReservedVoice(s32);
extern void SsSetSerialAttr(s32, s32, s32);
extern void SsSetAutoKeyOffMode(s16);
extern s16 SsVabOpenHead(s32, s16);
extern s16 SsVabOpenHeadWithMode(u8 *, s16, s16, u32);
extern s16 SsVabFakeBody(s16);
extern s16 SsVabTransCompleted(s16);
extern void SsVabClose(s16);
extern void SsSeqStop(s16);
extern void SsSepStop(s16, s16);
extern s16 SsUtKeyOnV(s16, s16, s16, s16, s16, s16, s16, s16);
extern s16 SsUtKeyOffV(s16);
extern s32 SsUtSetDetVVol(s16, s16, s16);
extern s32 func_80086130(s16, s16, s16);
extern void SsUtReverbOn(void);
extern void SsUtReverbOff(void);
extern void SsUtSetReverbDepth(s16, s16);
extern s16 SsUtGetReverbType(void);
extern s32 SsUtGetVBaddrInSB(s16);

#endif /* PSXSDK_LIBSND_H */
