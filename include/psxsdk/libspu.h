#ifndef PSXSDK_LIBSPU_H
#define PSXSDK_LIBSPU_H

/* PsyQ LIBSPU public types and entry points (Sony's libspu.h; SOTN include/psxsdk/libspu.h).
 * Type layouts and prototypes are spelled as BB2's code uses them (the module definitions in
 * src/main/psxsdk/libspu/). Library-internal state: src/main/psxsdk/libspu/libspu_internal.h. */

#include "common.h"

typedef struct {
    s16 left, right;
} SpuVolume;

/* SpuSetVoiceAttr's argument (sizeof = 0x40). */
typedef struct {
    /* 0x00 */ u32 voice;
    /* 0x04 */ u32 mask;
    /* 0x08 */ SpuVolume volume;
    /* 0x0C */ SpuVolume volmode;
    /* 0x10 */ SpuVolume volumex;
    /* 0x14 */ u16 pitch;
    /* 0x16 */ u16 note;
    /* 0x18 */ u16 sample_note;
    /* 0x1A */ s16 envx;
    /* 0x1C */ u32 addr;
    /* 0x20 */ u32 loop_addr;
    /* 0x24 */ s32 a_mode;
    /* 0x28 */ s32 s_mode;
    /* 0x2C */ s32 r_mode;
    /* 0x30 */ u16 ar;
    /* 0x32 */ u16 dr;
    /* 0x34 */ u16 sr;
    /* 0x36 */ u16 rr;
    /* 0x38 */ u16 sl;
    /* 0x3A */ u16 adsr1;
    /* 0x3C */ u16 adsr2;
} SpuVoiceAttr;

typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuReverbAttr;

/* SpuSetCommonAttr's argument (sizeof = 0x28). */
typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ SpuVolume mvol;
    /* 0x08 */ SpuVolume mvolmode;
    /* 0x0C */ SpuVolume mvolx;
    struct {
        /* 0x10 */ SpuVolume volume;
        /* 0x14 */ s32 reverb;
        /* 0x18 */ s32 mix;
    } cd;
    struct {
        /* 0x1C */ SpuVolume volume;
        /* 0x20 */ s32 reverb;
        /* 0x24 */ s32 mix;
    } ext;
} SpuCommonAttr;

extern void SpuInit(void);
extern void SpuStart(void);
extern void SpuQuit(void);
extern s32 SpuSetTransferMode(s32);
extern s32 SpuSetTransferStartAddr(s32);
extern s32 SpuWrite(s32, s32);
extern s32 SpuRead(s32, s32);
extern s32 SpuIsTransferCompleted(s32);
extern s32 SpuInitMalloc(s32, s32 *);
extern s32 SpuMalloc(s32);
extern void SpuFree(u32);
extern void SpuSetKey(s32, u32);
extern s32 SpuGetKeyStatus(s32);
extern void SpuGetAllKeysStatus(u8 *);
extern void SpuGetVoiceEnvelope(s32, u16 *);
extern void SpuSetNoiseVoice(s32, s32);
extern s32 SpuSetNoiseClock(s32);
extern s32 SpuSetReverb(s32);
extern void SpuSetReverbVoice(s32, s32);
extern s32 SpuSetReverbModeParam(SpuReverbAttr *);

#endif /* PSXSDK_LIBSPU_H */
