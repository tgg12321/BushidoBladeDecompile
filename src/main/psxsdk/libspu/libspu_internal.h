#ifndef LIBSPU_INTERNAL_H
#define LIBSPU_INTERNAL_H

/* PsyQ LIBSPU library-internal state and helpers shared by the modules in this
 * directory (SOTN src/main/psxsdk/libspu/libspu_internal.h). */

#include <psxsdk/libspu.h>

/* The SPU register block (Sony's SPU_RXX / union SpuUnion, as in SOTN and
 * psyz): one register set per voice, then the common registers; `raw` indexes
 * it by halfword. _spu_RXX points at 0x1F801C00 (SPU MMIO), hence volatile
 * (mmio-volatile-type-level). */
typedef struct tagSpuVoiceRegister {
    /* 0x00 */ SpuVolume volume;
    /* 0x04 */ u16 pitch;
    /* 0x06 */ u16 addr;
    /* 0x08 */ u16 adsr[2];
    /* 0x0C */ u16 volumex;
    /* 0x0E */ u16 loop_addr;
} SPU_VOICE_REG;

typedef struct tagSpuControl {
    /* 0x000 */ SPU_VOICE_REG voice[24];
    /* 0x180 */ SpuVolume main_vol;
    /* 0x184 */ SpuVolume rev_vol;
    /* 0x188 */ u16 key_on[2];
    /* 0x18C */ u16 key_off[2];
    /* 0x190 */ u16 chan_fm[2];
    /* 0x194 */ u16 noise_mode[2];
    /* 0x198 */ u16 rev_mode[2];
    /* 0x19C */ u32 chan_on;
    /* 0x1A0 */ u16 unk;
    /* 0x1A2 */ u16 rev_work_addr;
    /* 0x1A4 */ u16 irq_addr;
    /* 0x1A6 */ u16 trans_addr;
    /* 0x1A8 */ u16 trans_fifo;
    /* 0x1AA */ u16 spucnt;
    /* 0x1AC */ u16 data_trans;
    /* 0x1AE */ u16 spustat;
    /* 0x1B0 */ SpuVolume cd_vol;
    /* 0x1B4 */ SpuVolume ex_vol;
    /* 0x1B8 */ SpuVolume main_volx;
    /* 0x1BC */ SpuVolume unk_vol;
    /* 0x1C0 */ u16 dAPF1;
    /* 0x1C2 */ u16 dAPF2;
    /* 0x1C4 */ u16 vIIR;
    /* 0x1C6 */ u16 vCOMB1;
    /* 0x1C8 */ u16 vCOMB2;
    /* 0x1CA */ u16 vCOMB3;
    /* 0x1CC */ u16 vCOMB4;
    /* 0x1CE */ u16 vWALL;
    /* 0x1D0 */ u16 vAPF1;
    /* 0x1D2 */ u16 vAPF2;
    /* 0x1D4 */ u16 mLSAME;
    /* 0x1D6 */ u16 mRSAME;
    /* 0x1D8 */ u16 mLCOMB1;
    /* 0x1DA */ u16 mRCOMB1;
    /* 0x1DC */ u16 mLCOMB2;
    /* 0x1DE */ u16 mRCOMB2;
    /* 0x1E0 */ u16 dLSAME;
    /* 0x1E2 */ u16 dRSAME;
    /* 0x1E4 */ u16 mLDIFF;
    /* 0x1E6 */ u16 mRDIFF;
    /* 0x1E8 */ u16 mLCOMB3;
    /* 0x1EA */ u16 mRCOMB3;
    /* 0x1EC */ u16 mLCOMB4;
    /* 0x1EE */ u16 mRCOMB4;
    /* 0x1F0 */ u16 dLDIFF;
    /* 0x1F2 */ u16 dRDIFF;
    /* 0x1F4 */ u16 mLAPF1;
    /* 0x1F6 */ u16 mRAPF1;
    /* 0x1F8 */ u16 mLAPF2;
    /* 0x1FA */ u16 mRAPF2;
    /* 0x1FC */ u16 vLIN;
    /* 0x1FE */ u16 vRIN;
} SPU_RXX;

typedef union SpuUnion {
    volatile SPU_RXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;

/* One reverb preset (Sony rev_param_entry): a mask of the registers to set,
 * then one value per reverb register, in register order (0x1C0..0x1FE). The
 * preset table _spu_rev_param holds ten; SpuSetReverbModeParam builds one and
 * _spu_setReverbAttr writes it to the registers. */
typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u16 dAPF1, dAPF2;
    /* 0x08 */ u16 vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4;
    /* 0x12 */ u16 vWALL, vAPF1, vAPF2;
    /* 0x18 */ u16 mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    /* 0x24 */ u16 dLSAME, dRSAME;
    /* 0x28 */ u16 mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    /* 0x34 */ u16 dLDIFF, dRDIFF;
    /* 0x38 */ u16 mLAPF1, mRAPF1, mLAPF2, mRAPF2;
    /* 0x40 */ u16 vLIN, vRIN;
} RevParamEntry;

/* One SPU_MALLOC list record (SOTN SPU_MALLOC). */
typedef struct {
    u32 addr;
    u32 size;
} SpuMemRec;

/* Sony _spu_rev_attr (0x800A2888; sotn libspu_internal.h:87 struct
   SpuRevAttr): mode / depth L,R / delay / feedback. */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;

extern SpuUnion *_spu_RXX;
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
extern SpuMemRec *_spu_memList;
/* Sony's own header types the transfer callback as a volatile function
   pointer (legitimate-volatile-interrupt-touched). SOTN:
   src/main/psxsdk/libspu/libspu_internal.h:39 @db41b28 (PS1 use:
   src/main/psxsdk/libspu/s_r.c:10) */
extern void (* volatile _spu_transferCallback)();
/* _spu_RQ: the pending key-on [0..1] / key-off [2..3] queue; _spu_init clears
 * all 10 halfwords. These four are volatile under Ruling-4
 * (legitimate-volatile-interrupt-touched) grants. */
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
extern void _spu_setReverbAttr(RevParamEntry *);
extern void _SpuInit(s32);
extern void _SpuDataCallback(s32);
extern s32 _SpuSetAnyVoice(s32, u32, s32, s32);
extern s32 _SpuIsInAllocateArea_(u32);

#endif /* LIBSPU_INTERNAL_H */
