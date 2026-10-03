#ifndef LIBSPU_INTERNAL_H
#define LIBSPU_INTERNAL_H

/* PsyQ LIBSPU library-internal state and helpers shared by the modules in this directory
 * (SOTN src/main/psxsdk/libspu/libspu_internal.h). */

#include <psxsdk/libspu.h>

/* The SPU register block _spu_RXX points at, as the key-on / key-off code uses it. */
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;
typedef union {
    SpuRXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;

/* One SPU_MALLOC list record (SOTN SPU_MALLOC). */
typedef struct {
    u32 addr;
    u32 size;
} SpuMemRec;

/* Sony _spu_rev_attr — ONE struct (sotn libspu_internal.h:87 struct
   SpuRevAttr), base 0x800A2888: mode / depth L,R / delay / feedback. */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;

extern s32 _spu_RXX;
extern s32 _spu_transMode;
extern s32 _spu_trans_mode;
extern s32 _spu_inTransfer;
extern u16 _spu_tsa;
extern s32 _spu_mem_mode_plus;
extern s32 _spu_mem_mode_unitM;
extern s32 _spu_EVdma;
extern s32 _spu_isCalled;
extern s32 _spu_IRQCallback;
extern s32 _spu_keystat;
extern u16 _spu_voice_centerNote[]; /* one entry per SPU voice (24) */
extern s32 _spu_rev_flag;
extern s32 _spu_rev_reserve_wa;
extern s32 _spu_rev_offsetaddr;
extern s32 _spu_rev_startaddr[];
extern SpuRevAttr _spu_rev_attr;
extern s32 _spu_AllocBlockNum;
extern s32 _spu_AllocLastNum;
extern s32 _spu_memList;
/* PsyQ LIBSPU: Sony's own header types the SPU transfer callback as a
   volatile function pointer; volatile_extern_allowlist.txt grant.
   SOTN: src/main/psxsdk/libspu/libspu_internal.h:39 @db41b28 (PS1 use:
   src/main/psxsdk/libspu/s_r.c:10) */
extern void (* volatile _spu_transferCallback)();
/* Sony _spu_RQ: one object, the pending key-on / key-off queue (PsyQ 4.0 LIBSPU S_SK relocs:
 * addends 0/2/4/6 — key-on pending [0..1], key-off pending [2..3]); _spu_init clears all 10
 * halfwords (PsyQ 4.0 spu.c). splat had split it into two D_ symbols. _spu_RQ, _spu_RQvoice,
 * _spu_RQmask and _spu_env are volatile under Ruling-4 grants (volatile_extern_allowlist.txt). */
extern volatile u16 _spu_RQ[10];
extern volatile s32 _spu_RQvoice;
extern volatile s32 _spu_RQmask;
extern volatile s32 _spu_env;

extern s32 _spu_init(s32);
extern void _spu_FiDMA(void);
extern s32 _spu_t(s32, ...);
extern s32 _spu_Fw(s32, s32);
extern s32 _spu_Fr(s32, s32);
extern void _spu_FsetRXX(s32, u32, s32);
extern s32 _spu_FsetRXXa(s32, s32);
extern void _spu_gcSPU(void);
extern void _spu_setReverbAttr(s32 *);
extern void _SpuInit(s32);
extern void _SpuDataCallback(s32);
extern s32 _SpuSetAnyVoice(s32, u32, s32, s32);
extern s32 _SpuIsInAllocateArea_(u32);

#endif /* LIBSPU_INTERNAL_H */
