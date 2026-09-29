#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "system.h"
#include "psx.h"
#include "sound.h"

/* Forward declarations */

extern void _SpuInit(s32);
extern s32 SpuIsTransferCompleted(s32);
extern void _SsSeqPlay(s16, s16);
extern s16 SsVabOpenHeadWithMode(u8 *, s16, s16, u32);
extern s32 _SpuSetAnyVoice(s32, u32, s32, s32);

/* Externs for globals */
extern s16 _svm_damper;
extern s16 _svm_stereo_mono;
extern s32 g_spu_busy;


extern void DMACallback(s32, s32);

/* PsyQ LIBSPU: _spu_transferCallback — Sony's own header types the SPU
   transfer callback as a volatile function pointer (sotn-decomp
   libspu_internal.h:39); volatile is original semantics, not coercion */
extern void (* volatile g_spu_init_flag)();
extern s32 _spu_keystat;
extern s32 _spu_trans_mode;



extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void AddDrv(s32 *);
extern s32 D_800A307C;

extern s32 TestEvent(s32);
extern s32 _spu_init(s32);
extern s16 func_80087770(s16, u16, u16, s16);
extern s16 _SsVmGetSeqVol(s32, s16 *, s16 *);
extern void _spu_FsetRXX(s32, u32, s32);
extern s32 _spu_rev_flag;
extern s32 _spu_rev_reserve_wa;
extern s32 _spu_rev_offsetaddr;
extern s32 _spu_rev_attr_plus_0x4;
extern s16 _spu_rev_attr_plus_0x8;
extern s16 _spu_rev_attr_plus_0xA;
extern s32 _spu_rev_attr_plus_0xC;
extern s32 _spu_rev_attr_plus_0x10;
extern volatile s32 _spu_RQvoice; /* _spu_RQvoice — Ruling-4 grant (volatile_extern_allowlist.txt:44) */
extern volatile s32 _spu_RQmask;
extern s16 _spu_voice_centerNote_plus_0x2E;
extern volatile s32 _spu_env;

extern s32 _spu_AllocBlockNum;
extern s32 _spu_AllocLastNum;
extern s32 _spu_memList;
/* PsyQ libspu memList record (_spu_memList entries) */
typedef struct {
    u32 addr;
    u32 size;
} SpuMemRec;

extern s32 _spu_rev_startaddr[]; /* _spu_rev_startaddr */

extern s32 _spu_zerobuf;
extern s32 _spu_transMode;
extern s32 _spu_mem_mode_plus;
extern s32 _spu_mem_mode;
extern s32 _spu_mem_mode_unit;
extern s32 _spu_mem_mode_unitM;
extern volatile u32 *D_800A2CEC;
extern volatile s32 _spu_transferCallback;
extern s32 _spu_RXX;
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;
typedef union {
    SpuRXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;
extern SpuUnion D_800F7298;
extern s32 _spu_addrMode;
extern u16 _spu_tsa;
extern s32 _spu_inTransfer;
extern s32 _spu_IRQCallback;
extern s32 D_800A2D1C;
extern const char D_800163D8[16];
extern const char D_800163E8[16];
extern void printf(s32 *, s32 *);
extern void _spu_Fw1ts(void);
/* Sony _spu_RQ: ONE u16[4] object (PsyQ 4.0 LIBSPU S_SK relocs: addends 0/2/4/6 —
 * key-on pending [0..1], key-off pending [2..3]); splat split it into two D_
 * symbols. Ruling-4 grant, volatile_extern_allowlist.txt:40-41. */
extern volatile u16 _spu_RQ[10]; /* _spu_RQ; _spu_init clears all 10 (PsyQ 4.0 spu.c) */

extern s32 D_800A2D2C;
extern s32 D_800A2D30;
extern s32 D_800A2D34;
/* Sony _spu_madr/_spu_bcr/_spu_chcr: pointers to the SPU DMA (ch4) MMIO
 * registers 0x1F8010C0/C4/C8 (asm/data/7D920.data.s); pointee volatile per
 * mmio-volatile-type-level. */
extern volatile s32 *D_800A2CE0;
extern volatile s32 *D_800A2CE4;
extern volatile s32 *D_800A2CE8;

/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */

extern void SpuSetCommonAttr(void *);
void SsSetMVol(s16 a0, s16 a1) {
    s32 buf[10];
    buf[0] = 3;
    *(s16 *)&buf[1] = (s16)(a0 * 129);
    *((s16 *)&buf[1] + 1) = (s16)(a1 * 129);
    SpuSetCommonAttr(buf);
}
extern void VSyncCallback(s32);
extern s32 InterruptCallback(s32, s32);
extern void ResetRCnt(s32);
extern void SetRCnt(s32, s32, s32);
extern s32 _snd_seq_tick_env_plus_0x8;
static void _SsTrapIntrVSync(void); /* _SsTrapIntrVSync (ssstart.c static) */
static void _SsSeqCalledTbyT_1per2(void); /* _SsSeqCalledTbyT_1per2 (ssstart.c static) */

/* PsyQ 4.0 LIBSND ssstart: _SsStart + SndSeqTickEnv (_snd_seq_tick_env @
   D_800A26CC) — verbatim-linked Sony object (census 2026-07-09); C ref:
   sotn-decomp src/main/psxsdk/libsnd/ssstart.c (BB2's 4.0 rev uses 0x7F for
   the case-0 sentinel where SOTN's rev uses 0xFF) */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unk12;
    /* 0x10 */ u8 unk16;
    /* 0x11 */ u8 unk17;
    /* 0x12 */ u8 unk18;
    /* 0x13 */ u8 unk19;
} SndSeqTickEnv;
extern SndSeqTickEnv _snd_seq_tick_env;

void _SsStart(s32 arg0) {
    u16 rcnt_target;
    u32 rcnt_spec;

    s32 wait = 1000;
    while (--wait >= 0) {
    }

    _snd_seq_tick_env.unk16 = 0;
    _snd_seq_tick_env.unk18 = 6;
    _snd_seq_tick_env.unk17 = 0;
    _snd_seq_tick_env.unk12 = 0;
    rcnt_spec = 0xF2000002;
    rcnt_target = 0x44E8;
    switch (_snd_seq_tick_env.unk0) {
    case 0:
        _snd_seq_tick_env.unk18 = 0x7F;
        return;

    case 5:
        _snd_seq_tick_env.unk18 = 0;
        if (arg0 == 0) {
            _snd_seq_tick_env.unk16 = 1;
        } else {
            rcnt_spec = 0xF2000003;
            rcnt_target = 1;
        }
        break;

    case 3:
        rcnt_target = 0x89D0;
        break;

    case 2:
        break;

    default:
        if (_snd_seq_tick_env.unk4 == 0) {
            if (_snd_seq_tick_env.unk0 < 0x46) {
                rcnt_target = 0x204CC0 / _snd_seq_tick_env.unk0;
                _snd_seq_tick_env.unk17++;
            } else {
                rcnt_target = 0x409980 / _snd_seq_tick_env.unk0;
            }
        } else {
            return;
        }
        break;
    }

    if (_snd_seq_tick_env.unk16 != 0) {
        EnterCriticalSection();
        VSyncCallback(_snd_seq_tick_env_plus_0x8);
    } else {
        s32 de;
        s32 a1_val;
        EnterCriticalSection();
        ResetRCnt(rcnt_spec);
        SetRCnt(rcnt_spec, rcnt_target, 0x1000);
        de = _snd_seq_tick_env.unk18;
        if (de == 0) {
            s32 ret = InterruptCallback(0, 0);
            de = _snd_seq_tick_env.unk18;
            a1_val = (s32)&_SsTrapIntrVSync;
            _snd_seq_tick_env.unk12 = ret;
        } else {
            a1_val = (s32)&_SsSeqCalledTbyT_1per2;
            if (_snd_seq_tick_env.unk17 == 0) {
                a1_val = _snd_seq_tick_env_plus_0x8;
            }
        }
        InterruptCallback(de, a1_val);
    }
    ExitCriticalSection();
}
/* kengo:MED  |  sa_tan5/saTan5TakeAnim2_2  |  154i  |  x2 size collision */
/* PsyQ 4.0 LIBSND ssstart: SsStart / SsStart2 / _SsTrapIntrVSync /
   _SsSeqCalledTbyT_1per2 + sscall: SsSeqCalledTbyT — verbatim-linked Sony
   objects (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libsnd/{ssstart.c,sscall.c}. Only SsStart (=DispStuff)
   has a glabel: SsStart2 + the tick trampolines are statics inside the
   splat extent; SsSeqCalledTbyT is address-referenced only by the
   SndSeqTickEnv .data initializer (raw .word @0x800A26D4). */
extern s32 _snd_ev_flag;  /* _snd_ev_flag */
extern s16 _snd_seq_s_max;  /* _snd_seq_s_max */
extern s16 _snd_seq_t_max;  /* _snd_seq_t_max */
extern s32 _snd_openflag;  /* _snd_openflag */
extern s32 _ss_score;  /* _ss_score (per-SEP score-block pointer table) */
extern void _SsVmFlush(void);   /* SpuVmFlush */
void _SsSndPlay(s16 a0, s16 a1); /* _SsSndPlay */
void _SsSndCrescendo(s16 arg0, s16 arg1);     /* _SsSndCrescendo */
void _SsSndDecrescendo(s16 arg0, s16 arg1);     /* _SsSndDecrescendo */
void _SsSndTempo(s16 a0, s16 a1);         /* _SsSndTempo */
void _SsSndPause(s16 a0, s16 a1);    /* _SsSndPause */
void _SsSndReplay(s32 a0, s16 a1);   /* _SsSndReplay */
void _SsSndStop(s16 a0, s16 a1);         /* _SsSndStop */

void SsStart(void) {
    _SsStart(1);
}
static void SsStart2(void) {
    _SsStart(0);
}
static void _SsTrapIntrVSync(void) {
    if (_snd_seq_tick_env.unk12 != 0) {
        ((void (*)(void))_snd_seq_tick_env.unk12)();
    }
    ((void (*)(void))_snd_seq_tick_env.unk8)();
}
static void _SsSeqCalledTbyT_1per2(void) {
    /* The 1-per-2 tick toggle: a standalone word AFTER the declared
       SndSeqTickEnv block (which ends at +0x13) — its own splat symbol
       (dlabel D_800A26E0 in 7D920.data.s; named_syms.txt:
       g_alarm_pending_priority_flag), the only C handle for this memory
       in the TU. */
    extern s32 _snd_seq_tick_env_plus_0x14;
    if (_snd_seq_tick_env_plus_0x14 == 0) {
        _snd_seq_tick_env_plus_0x14 = 1;
    } else {
        _snd_seq_tick_env_plus_0x14 = 0;
        ((void (*)(void))_snd_seq_tick_env.unk8)();
    }
}
#define SS_SCORE_FLAG(i, j) \
    (*(s32 *)(((s32 *)&_ss_score)[i] + (j) * 0xB0 + 0x98))
static void SsSeqCalledTbyT(void) {
    int i;
    int j;
    if (_snd_ev_flag != 1) {
        _snd_ev_flag = 1;

        _SsVmFlush();

        for (i = 0; i < _snd_seq_s_max; i++) {
            s32 bit = 1 << i;
            if (_snd_openflag & bit) {
                for (j = 0; j < _snd_seq_t_max; j++) {
                    if (SS_SCORE_FLAG(i, j) & 1) {
                        _SsSndPlay(i, j);

                        if (SS_SCORE_FLAG(i, j) & 0x10) {
                            _SsSndCrescendo(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x20) {
                            _SsSndDecrescendo(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x40) {
                            _SsSndTempo(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x80) {
                            _SsSndTempo(i, j);
                        }
                    }
                    if (SS_SCORE_FLAG(i, j) & 2) {
                        _SsSndPause(i, j);
                    }
                    if (SS_SCORE_FLAG(i, j) & 8) {
                        _SsSndReplay((s16)i, j);
                    }
                    if (SS_SCORE_FLAG(i, j) & 4) {
                        _SsSndStop(i, j);
                        SS_SCORE_FLAG(i, j) = 0;
                    }
                }
            }
        }
        _snd_ev_flag = 0;
    }
}
/* kengo:LOW  |  su_menu_ending/_DispStuff  |  209i  |  PS2 UI — reverted */
void _SsSndCrescendo(s16 a0, s16 a1) {
    /* FAKE: named intermediate for the bank index, mechanism: combine.c
       try_combine folds the s16->int `ashiftrt:16` together with the scale
       `ashift:2` into one `ashiftrt:14` and emits it at the LATER insn's slot,
       and sched.c rank_for_schedule leaves the independent insns in RTL/LUID
       order - so the index conversion must be its own statement ahead of the
       table-address statement; lever-exhaustion: hypotheses.md H8 - 13
       single-expression spellings of this preamble measured, all score 2
       (v5,v6,v7,wa-we,x1,x2,y1-y4 in tmp/grind/_SsSndCrescendo/s7/). */
    s32 bank_no = a0;
    /* FAKE: C-level pointer alias to the _ss_score global, mechanism: the
       `movsi` of the symbol_ref has to be emitted BETWEEN the surviving
       `ashift:16` and combine.c's folded `ashiftrt:14`; fold() moves the
       constant ADDR_EXPR to operand 1 of any single pointer-sum expression,
       which therefore evaluates the whole index first; lever-exhaustion:
       hypotheses.md H8 - the same 13 measured spellings, all score 2. */
    s32 *score_tbl = (s32 *)&_ss_score;
    s32 *bank = score_tbl + bank_no;
    u8 *base = (u8 *)(*bank + (s16)a1 * 0xB0);
    u16 voll, volr;

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
    } else if (*(s16 *)(base + 0x4C) > 0) {
        if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((voll + 1) <= (voll + *(s16 *)(base + 0x4A)))
                    func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll + 1), (u16)(volr + 1), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
        }
    } else if (*(s16 *)(base + 0x4C) < 0) {
        *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
        if (*(s16 *)(base + 0x4A) >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if (((voll - *(s16 *)(base + 0x4C)) >= 0x7F) &&
                ((volr - *(s16 *)(base + 0x4C)) >= 0x7F)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
            }
            if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                *(s16 *)(base + 0x48))
                func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - *(s16 *)(base + 0x4C)),
                              (u16)(volr - *(s16 *)(base + 0x4C)), 1);
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
            *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x10;
        }
        if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
            *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
void _SsSndDecrescendo(s16 a0, s16 a1) {
    s32 bank_no = a0;
    s32 *score_tbl = (s32 *)&_ss_score;
    s32 *bank = score_tbl + bank_no;
    u8 *base = (u8 *)(*bank + (s16)a1 * 0xB0);
    u16 voll, volr;

    if (--(*(s32 *)(base + 0xA0)) < 0) {
        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
    } else if (*(s16 *)(base + 0x4C) > 0) {
        if ((*(s32 *)(base + 0xA0) % *(s16 *)(base + 0x4C)) == 0) {
            *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) - 1;
            if (*(s16 *)(base + 0x4A) >= 0) {
                _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((((u16)voll - 1) >= ((u16)voll - *(s16 *)(base + 0x4A))) ||
                    (((u16)volr - 1) >= ((u16)volr - *(s16 *)(base + 0x4A)))) {
                    if ((voll == 0) || (volr == 0)) {
                        *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
                    } else {
                        func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - 1),
                                      (u16)(volr - 1), 1);
                    }
                }
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
            }
            if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
                *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
        }
    } else if (*(s16 *)(base + 0x4C) < 0) {
        *(u16 *)(base + 0x4A) = *(u16 *)(base + 0x4A) + *(s16 *)(base + 0x4C);
        if (*(s16 *)(base + 0x4A) >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if ((((u16)voll + *(s16 *)(base + 0x4C)) <= 0) &&
                (((u16)volr + *(s16 *)(base + 0x4C)) <= 0)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
            }
            if (((*(s32 *)(base + 0x9C) - *(s32 *)(base + 0xA0)) * -*(s16 *)(base + 0x4C)) <
                *(s16 *)(base + 0x48)) {
                if ((voll == 0) || (volr == 0)) {
                    *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
                } else {
                    func_80087770((s16)(a0 | (a1 << 8)),
                                  (u16)(voll + *(s16 *)(base + 0x4C)),
                                  (u16)(volr + *(s16 *)(base + 0x4C)), 1);
                }
            }
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
            *(s32 *)(((s16)a1 * 0xB0) + *bank + 0x98) &= ~0x20;
        }
        if ((*(s32 *)(base + 0xA0) == 0) || (*(s16 *)(base + 0x4A) <= 0))
            *(s32 *)(((s32 *)&_ss_score)[a0] + (s16)a1 * 0xB0 + 0x98) &= ~0x20;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)(base + 0x5C), (s16 *)(base + 0x5E));
}
void _SsSndPause(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    u8 *entry;
    entry = (u8 *)(*base_ptr + offset);
    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    entry[0x14] = 0;
    offset += *base_ptr;
    *(s32 *)(offset + 0x98) &= ~2;
}

void _SsSndPlay(s16 a0, s16 a1) {
    _SsSeqPlay(a0, a1);
}

/* PsyQ LIBSND/MIDIREAD: _SsSeqPlay — census-matched Sony library object.
 * Body: the published psxsdk reference control flow (sotn-decomp
 * src/main/psxsdk/libsnd/seqread.c _SsSeqPlay) with BB2's byte-offset
 * record layout (D_80106F28 table, 0xB0 stride; delta +0x90, unk70 +0x54,
 * unk6E +0x52; _SsGetSeqData == func_80084CC0). Adopted per the
 * reference-adoption path because it measures 0 (2026-08-11 campaign;
 * the prior goto-loop spelling needed 15 regfix rules the nested
 * do/while + if/else-if ladder makes unnecessary). */
void _SsSeqPlay(s16 a0, s16 a1) {
    u8 *base;
    s32 var_s0;

    base = (u8 *)(*(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0);

    if (*(s32 *)(base + 0x90) - *(s16 *)(base + 0x54) > 0) {
        if (*(s16 *)(base + 0x52) > 0) {
            (*(s16 *)(base + 0x52))--;
        } else if (*(s16 *)(base + 0x52) == 0) {
            *(s16 *)(base + 0x52) = *(s16 *)(base + 0x54);
            (*(s32 *)(base + 0x90))--;
        } else {
            *(s32 *)(base + 0x90) -= *(s16 *)(base + 0x54);
        }
    } else if (*(s32 *)(base + 0x90) <= *(s16 *)(base + 0x54)) {
        var_s0 = *(s32 *)(base + 0x90);
        do {
            do {
                func_80084CC0(a0, a1);
            } while (*(s32 *)(base + 0x90) == 0);
            var_s0 += *(s32 *)(base + 0x90);
        } while (var_s0 < *(s16 *)(base + 0x54));
        *(s32 *)(base + 0x90) = var_s0 - *(s16 *)(base + 0x54);
    }
}
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void _SsSeqGetEof(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *base = (u8 *)(*base_ptr + (s16)a1 * 0xB0);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~1;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~2;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 0x200;
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
        base[0x14] = 0;
        _SsSndNextSep(base[0x22], base[0x23]);
        _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    }
    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    *(s32 *)(base + 0x90) = *(s16 *)(base + 0x54);
}
extern void (*D_800F3340)(s16, s16, u8, u8);
extern void (*D_800F3344)(s16, s16, u8);
extern void (*D_800F3348)(s16, s16);
extern void (*D_800F334C)(s16, s16, u8);
extern void (*D_800F3350)(s16, s16, u8);
extern s32 _SsReadDeltaValue(s32, s16);

s32 func_80084CC0(s16 a0, s16 a1)
{
  /* 100% pure C: no regfix rules, no register pins, no inline asm.
   *
   * One non-obvious choice: the 2nd switch's 0x90 case reads the sequence
   * pointer into a *block-local* `cp` instead of the shared `cmd_ptr`.
   * Sharing one variable made GCC's global allocator place `cmd_ptr` in $a2
   * (reusing the handler arg register, since the temp dies just before arg
   * setup); splitting that one use shrinks cmd_ptr's live range / conflicts
   * so the allocator gives it $v1 (its default-order preference) across the
   * 1st-switch cases — exactly target. Retires the last pin. */
  u8 *state;
  s32 cmd;
  u8 *cmd_ptr;
  u8 *ptr;
  u32 data;
  u8 b;
  u8 prev;
  u8 databyte;
  u8 next;
  s32 ret;
  state = (u8 *) (((s32) ((void **) (&_ss_score))[a0]) + (a1 * 0xB0));
  ptr = *((u8 **) state);
  *((u8 **) state) = ptr + 1;
  b = ptr[0];
  ret = 0;
  if (((*((s32 *) (((u8 *) (((s32) ((void **) (&_ss_score))[a0]) + (a1 * 0xB0))) + 0x98))) & 0x401) == 0x401)
  {
    if (((s32) (ptr + 1)) == ((*((s32 *) (state + 0x10))) + 1))
    {
      ((void (*)(s16, s16, u8, u8 *)) _SsSeqGetEof)(a0, a1, ((u8 *) (*((s32 *) (state + 0x10))))[1], ptr);
      return -1;
    }
  }
  if (b & 0x80)
  {
    state[0x17] = b & 0xF;
    cmd = b & 0xF0;
    switch (cmd)
    {
      case 0x90:
        state[0x16] = 0x90;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        b = cmd_ptr[0];
        data = b;
        *((u8 **) state) = cmd_ptr + 2;
        next = cmd_ptr[1];
        *((s32 *) (state + 0x90)) = _SsReadDeltaValue(a0, a1);
        D_800F3340(a0, a1, data, next);
        goto end;

      case 0xB0:
        state[0x16] = 0xB0;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        D_800F3350(a0, a1, databyte);
        goto end;

      case 0xC0:
        state[0x16] = 0xC0;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        D_800F3344(a0, a1, databyte);
        goto end;

      case 0xE0:
        state[0x16] = 0xE0;
        *((u8 **) state) = (*((u8 **) state)) + 1;
        D_800F3348(a0, a1);
        goto end;

      case 0xF0:
        state[0x16] = 0xFF;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        if (databyte == 0x2F)
      {
        ret = 1;
        ((void (*)(s16, s16, u8, u8 *)) _SsSeqGetEof)(a0, a1, 0x2F, ptr);
        goto end;
      }
        D_800F334C(a0, a1, databyte);
        goto end;

      default:
        goto end;

    }

  }
  else
  {
    prev = state[0x16];
    switch (prev)
    {
      case 0x90:
      {
        u8 *cp = *((u8 **) state);
        *((u8 **) state) = cp + 1;
        next = cp[0];
        *((s32 *) (state + 0x90)) = _SsReadDeltaValue(a0, a1);
        D_800F3340(a0, a1, b, next);
        goto end;
      }

      case 0xB0:
        D_800F3350(a0, a1, b);
        goto end;

      case 0xC0:
        D_800F3344(a0, a1, b);
        goto end;

      case 0xE0:
        D_800F3348(a0, a1);
        goto end;

      case 0xFF:
        next = b;
        if (next == 0x2F)
      {
        ret = 1;
        ((void (*)(s16, s16, u8, u8 *)) _SsSeqGetEof)(a0, a1, 0x2F, ptr);
        goto end;
      }
        D_800F334C(a0, a1, b);

      default:
        goto end;

    }

  }
  end:
  return ret;

}
/* kengo:MED  |  sa_tan0/saTan0Main  |  233i */
s32 _SsReadDeltaValue(s32 arg0, s16 arg1) {
    s32 result;
    u8 *ptr;
    u8 **base;
    s32 val;
    s32 byte;

    base = (u8 **)((*(s32 *)((u8 *)&_ss_score + ((s32)(arg0 << 16) >> 14))) + (arg1 * 0xB0));
    ptr = *base;
    *base = ptr + 1;
    val = *ptr;
    if (val == 0) {
        return 0;
    }
    result = val << 2;
    if (val & 0x80) {
        val &= 0x7F;
        do {
            ptr = *base;
            *base = ptr + 1;
            byte = *ptr;
            val = (val << 7) + (byte & 0x7F);
        } while (byte & 0x80);
    }
    result = val << 2;
    result = (result + val) << 1;
    *(s32 *)((u8 *)base + 0x88) += result;
    return result;
}
/* PsyQ LIBSND next.c: _SsSndNextSep — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/next.c (mixed
   score-pointer / full-index accesses are the original's spelling) */
void _SsSndNextSep(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 base = *base_ptr;
    u8 *entry = (u8 *)(base + (s16)a1 * 0xB0);
    entry[0x20] = 1;
    entry[0x21] = 0;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x100;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~2;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~4;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x200;
    *(s32 *)entry = *(s32 *)(entry + 4);
    entry[0x14] = 1;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 1;
}
/* PsyQ 4.0 LIBSND replay: _SsSndReplay — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/replay.c */
void _SsSndReplay(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 base = *base_ptr;
    u8 *entry = (u8 *)(base + (s16)a1 * 0xB0);
    entry[0x14] = 1;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
}
/* PsyQ 4.0 LIBSND SSSTOP: _SsSndStop — verbatim-linked Sony object (LIBSND
   hunt 2026-07-10: bit-verbatim vs the Jun-06-1997 4.0 build, 118 words);
   C ref: sotn-decomp src/main/psxsdk/libsnd/stop.c (interim 4.0 build adds
   the ~0x400 flag clear + NotifyChannel/ResetCounter pair). */
void _SsSndStop(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *p = (u8*)(*base_ptr + (s16)a1 * 0xB0);
    s32 i;
    u8 *ip;

    *(s32*)(p + 0x98) &= ~1;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~2;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~8;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~0x400;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) |= 4;

    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    _SsVmDamperOff();

    i = 0;
    p[0x14] = 0;
    *(s32*)(p + 0x88) = 0;
    p[0x1C] = 0;
    p[0x18] = 0;
    p[0x19] = 0;
    p[0x1E] = 0;
    p[0x1A] = 0;
    p[0x1B] = 0;
    p[0x1F] = 0;
    p[0x17] = 0;
    p[0x21] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x15] = 0;
    p[0x16] = 0;

    *(s32*)(p + 0x90) = *(s32*)(p + 0x84);
    *(s32*)(p + 0x94) = *(s32*)(p + 0x8C);
    *(s16*)(p + 0x54) = *(u16*)(p + 0x56);
    *(s32*)(p + 0x0) = *(s32*)(p + 0x4);
    *(s32*)(p + 0x8) = *(s32*)(p + 0x4);

    do {
        ip = p + i;
        ip[0x37] = (u8)i;
        ip[0x27] = 0x40;
        *(s16 *)(p + i * 2 + 0x60) = 0x7F;
        i++;
    } while (i < 0x10);
    *(s16*)(p + 0x5C) = 0x7F;
    *(s16*)(p + 0x5E) = 0x7F;
}

void SsSeqStop(s16 a0) {
    _SsSndStop(a0, 0);
}

void SsSepStop(s16 a0, s16 a1) {
    _SsSndStop(a0, a1);
}
/* PsyQ LIBSND ssvol: SsSetSerialVol — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/scssvol.c.
   SpuCommonAttr per PsyQ libspu.h (sizeof = 0x28 — matches the frame). */
typedef struct {
    s16 left, right;
} SpuVolume;
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

void SsSetSerialVol(s16 s_num, s16 voll, s16 volr) {
    SpuCommonAttr attr;
    if ((u8)s_num == 0) {
        attr.mask = 0xC0;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.cd.volume.left = voll * 258;
        attr.cd.volume.right = volr * 258;
    }
    if ((u8)s_num == 1) {
        attr.mask = 0xC00;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.ext.volume.left = voll * 258;
        attr.ext.volume.right = volr * 258;
    }
    SpuSetCommonAttr((s32 *)&attr);
}
extern s32 VBLANK_MINUS;
void SsSetTickMode(s32 arg) {
    s32 mode;
    s32 v;

    mode = GetVideoMode();

    if (arg & 0x1000) {
        _snd_seq_tick_env.unk4 = 1;
        _snd_seq_tick_env.unk0 = arg & 0xFFF;
    } else {
        _snd_seq_tick_env.unk4 = 0;
        _snd_seq_tick_env.unk0 = arg;
    }

    v = _snd_seq_tick_env.unk0;
    if (v >= 6) goto big_v;
    switch (v) {
    case 4: {
        s32 t = 50;
        VBLANK_MINUS = t;
        if (mode == 1) _snd_seq_tick_env.unk0 = 5;
        else _snd_seq_tick_env.unk0 = t;
        break;
    }
    case 1: {
        s32 t = 60;
        VBLANK_MINUS = t;
        if (mode == 0) _snd_seq_tick_env.unk0 = 5;
        else _snd_seq_tick_env.unk0 = t;
        break;
    }
    case 3:
        VBLANK_MINUS = 120;
        break;
    case 2:
        VBLANK_MINUS = 240;
        break;
    case 5:
        if (mode == 0) VBLANK_MINUS = 60;
        else if (mode == 1) VBLANK_MINUS = 50;
        else VBLANK_MINUS = 60;
        break;
    case 0:
        if (mode == 0) VBLANK_MINUS = 60;
        else if (mode == 1) VBLANK_MINUS = 50;
        else VBLANK_MINUS = 60;
        break;
    default:
        VBLANK_MINUS = 60;
        break;
    }
    return;
big_v:
    VBLANK_MINUS = v;
}
/* kengo:MED  |  am_rmd/SetBloodSpot  |  91i */
/* PsyQ 4.0 LIBSND TEMPO: _SsSndTempo — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/tempo.c (interim
   4.0 build adds the counter<0 early clear-and-return). */
void _SsSndTempo(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&_ss_score;
    s32 *tbl = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *p = (u8 *)(*tbl + (s16)a1 * 0xB0);

    *(s32 *)(p + 0xA8) = *(s32 *)(p + 0xA8) - 1;
    if (*(s32 *)(p + 0xA8) < 0) {
        *(s32 *)((u8 *)(*tbl + (s16)a1 * 0xB0) + 0x98) &= ~0x40;
        *(s32 *)((u8 *)(*tbl + (s16)a1 * 0xB0) + 0x98) &= ~0x80;
        return;
    }

    if (*(s16 *)(p + 0x4E) > 0) {
        u32 new_val;
        if ((*(s32 *)(p + 0xA8) % *(s16 *)(p + 0x4E)) != 0) {
            return;
        }
        if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
            new_val = *(u32 *)(p + 0x94) - 1;
            goto tempo_store;
        }
        if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
            new_val = *(u32 *)(p + 0x94) + 1;
        tempo_store:
            *(u32 *)(p + 0x94) = new_val;
        }
    } else {
        if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
            *(u32 *)(p + 0x94) = *(u32 *)(p + 0x94) + *(s16 *)(p + 0x4E);
            if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
                *(u32 *)(p + 0x94) = *(u32 *)(p + 0xAC);
            }
        } else if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
            *(u32 *)(p + 0x94) = *(u32 *)(p + 0x94) - *(s16 *)(p + 0x4E);
            if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
                *(u32 *)(p + 0x94) = *(u32 *)(p + 0xAC);
            }
        }
    }

    *(s16 *)(p + 0x54) =
        (*(s16 *)(p + 0x50) * *(u32 *)(p + 0x94) * 10) / (u32)(VBLANK_MINUS * 60);
    if (*(s16 *)(p + 0x54) <= 0) {
        *(s16 *)(p + 0x54) = 1;
    }
    if ((*(s32 *)(p + 0xA8) == 0) || (*(u32 *)(p + 0x94) == *(u32 *)(p + 0xAC))) {
        s32 shifted2 = a0 << 16;
        s32 *addr2 = (s32 *)&_ss_score;
        s32 *tbl2 = (s32 *)((u8 *)addr2 + (shifted2 >> 14));
        *(s32 *)((u8 *)(*tbl2 + (s16)a1 * 0xB0) + 0x98) &= ~0x40;
        *(s32 *)((u8 *)(*tbl2 + (s16)a1 * 0xB0) + 0x98) &= ~0x80;
    }
}
extern u8 _SsVmMaxVoice;

/* func_800858D0: reset the per-voice state record (_svm_voice) of every
 * voice up to _SsVmMaxVoice and key each one off (func_8008B488 with a
 * one-voice mask, then _SsVmKeyOffNow). Plain C since the 2026-09-24
 * _svm_voice aggregate merge: with the stores spelled as record-field
 * writes, the former empty-if (F6) closer and the u/offset carrier locals
 * are no longer needed (memory/grind/vmNoiseOn/evidence.md). */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (_SsVmMaxVoice != 0) {
        do {
            _svm_voice[var_s0].unk2 = 0x18;
            _svm_voice[var_s0].unk6 = 0;
            _svm_voice[var_s0].unke = 0xFF;
            _svm_voice[var_s0].unk10 = 0;
            _svm_voice[var_s0].prog = 0;
            _svm_voice[var_s0].tone = 0xFF;
            buf[0] = 1 << var_s0;
            func_8008B488(buf);
            _svm_cur.voice = var_s0;
            _SsVmKeyOffNow(1);
            var_s0 = var_s0 + 1;
        } while (var_s0 < _SsVmMaxVoice);
    }
}
extern u8 _svm_vab_used[];
extern s32 _svm_vab_start[];
s32 SsUtGetVBaddrInSB(s16 a0) {
    if ((u16)a0 >= 0x11) {
        return -1;
    }
    if (_svm_vab_used[a0] != 1) {
        return -1;
    }
    return _svm_vab_start[a0];
}
/* PsyQ LIBSND UT_KEYV. BB2 keeps the 4.0 routine's source shape but uses the
   later 54-byte voice-state stride (rather than 4.0's 52-byte layout). */
typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag;
    s16 reserved[4];
} VagAtr;

extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;
extern void vmNoiseOn(u8);
extern s32 note2pitch2(u16, u16);
extern void _SsVmKeyOnNow(s32, u16);

s16 SsUtKeyOnV(s16 voice, s16 vabId, s16 prog, s16 tone, s16 note, s16 fine,
                s16 voll, s16 volr) {
    s32 toneIndex;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice < 0 || voice >= 24) {
        _snd_ev_flag = 0;
        return -1;
    }
    if (_SsVmVSetUp(vabId, prog)) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.seq_sep_no = 0x21;
    _svm_cur.note = note;
    _svm_cur.fine = fine;
    _svm_cur.tone = tone;

    if (voll == volr) {
        _svm_cur.pan = 0x40;
        _svm_cur.volume = voll;
    } else if (volr < voll) {
        _svm_cur.pan = (volr * 0x40) / voll;
        _svm_cur.volume = voll;
    } else {
        _svm_cur.pan = 0x7F - ((voll * 0x40) / volr);
        _svm_cur.volume = volr;
    }

    _svm_cur.mvol = _svm_pg[prog].mvol;
    _svm_cur.mpan = _svm_pg[prog].mpan;
    _svm_cur.prog_tones = _svm_pg[prog].tones;

    toneIndex = _svm_cur.tone + (_svm_cur.field_7_fake_program * 0x10);
    _svm_cur.tone_prior = _svm_tn[toneIndex].prior;
    _svm_cur.tone_vag_idx = _svm_tn[toneIndex].vag;
    _svm_cur.tone_vol = _svm_tn[toneIndex].vol;
    _svm_cur.tone_pan = _svm_tn[toneIndex].pan;
    _svm_cur.tone_center = _svm_tn[toneIndex].center;
    _svm_cur.tone_shift = _svm_tn[toneIndex].shift;
    _svm_cur.tone_mode = _svm_tn[toneIndex].mode;
    _svm_cur.tone_min = _svm_tn[toneIndex].min;
    _svm_cur.tone_max = _svm_tn[toneIndex].max;

    if (_svm_cur.tone_vag_idx == 0) {
        _snd_ev_flag = 0;
        return -1;
    }

    _svm_cur.voice = voice;
    _svm_voice[voice].unke = 0x21;
    _svm_voice[voice].vabId = vabId;
    _svm_voice[voice].unk10 = _svm_cur.field_7_fake_program;
    _svm_voice[voice].prog = prog;
    _svm_voice[voice].unk0 = _svm_cur.tone_vag_idx;
    _svm_voice[voice].tone = _svm_cur.tone;
    _svm_voice[voice].note = note;
    _svm_voice[voice].unk1b = 1;
    _svm_voice[voice].unk2 = 0;
    _SsVmDoAllocate();
    if (_svm_cur.tone_vag_idx == 0xFF) {
        vmNoiseOn(voice);
    } else {
        _SsVmKeyOnNow(1, note2pitch2(note, fine));
    }
    _snd_ev_flag = 0;
    return voice;
}

/* PsyQ LIBSND UT_KEYV: SsUtKeyOffV — the module's second exported entry point, which
   splat merged into SsUtKeyOnV. Split out 2026-09-07 (docs/naming/libscan/
   near-tier-ruling-2026-09-07.md; XDEF +0x394, follows a real jr $ra); must stay
   immediately after its former host so the link order reproduces the byte layout. */
s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice >= 0 && voice < 24) {
        _svm_cur.voice = voice;
        _SsVmKeyOffNow(0);
        _snd_ev_flag = 0;
        return 0;
    }
    _snd_ev_flag = 0;
    return -1;
}
/* kengo:MED  |  am_rmd/AllocBukiRmd  |  259i */
extern s32 _svm_rattr;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
extern s32 SpuSetReverbModeParam();
void SsUtSetReverbDepth(s16 a0, s16 a1) {
    s32 x = (s16)a0 * 32767 / 127;
    s32 y = (s16)a1 * 32767 / 127;
    s32 *buf = &_svm_rattr;
    *buf = 6;
    _svm_rattr_plus_0x8 = x;
    _svm_rattr_plus_0xA = y;
    SpuSetReverbModeParam(buf);
}
extern s32 _svm_rattr;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
extern s32 _svm_rattr_plus_0x4;
s16 SsUtSetReverbType(s16 a0) {
    s32 neg = 0;
    s16 v1 = a0;
    s32 s0;
    if ((s32)(a0 << 16) < 0) {
        neg = 1;
        v1 = -a0;
    }
    if ((u16)v1 < 0xA) {
        _svm_rattr = 1;
        if (neg) {
            _svm_rattr_plus_0x4 = (s16)((v1 | 0x100) << 16 >> 16);
        } else {
            _svm_rattr_plus_0x4 = (s16)(v1 << 16 >> 16);
        }
        s0 = (s16)(v1 << 16 >> 16);
        if (s0 == 0) {
            SpuSetReverb(0);
        }
        SpuSetReverbModeParam(&_svm_rattr);
        return s0;
    }
    return -1;
}
s16 SsUtGetReverbType(void) {
    return *(s16 *)&_svm_rattr_plus_0x4;
}

void SsUtReverbOff(void) {
    SpuSetReverb(0);
}

extern s16 _svm_sreg_buf[];

extern u8 _svm_sreg_dirty[];

void SsUtReverbOn(void) {
    SpuSetReverb(1);
}

s32 func_80085FD8(s16 a0) {
    if ((u16)a0 < 0x18) {
        SpuGetVoiceVolume(a0);
        return 0;
    }
    return -1;
}

s32 SsUtSetDetVVol(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        _svm_sreg_buf[idx * 8 + 1] = y;
        _svm_sreg_buf[idx * 8] = x;
        _svm_sreg_dirty[idx] |= 3;
        return 0;
    }
    return -1;
}

s32 func_80086080(s16 a0, s16 *a1, s16 *a2) {
    u16 raw1, raw2;

    if ((u16)a0 < 0x18) {
        SpuGetVoiceVolume(a0, &raw1, &raw2);
        *a1 = (s16)raw1 / 129;
        *a2 = (s16)raw2 / 129;
        return 0;
    }
    return -1;
}
s32 func_80086130(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        s16 vx = x * 129;
        s16 vy = y * 129;

        _svm_sreg_buf[idx * 8 + 1] = vy;
        _svm_sreg_buf[idx * 8] = vx;
        _svm_sreg_dirty[idx] |= 3;
        return 0;
    }
    return -1;
}
extern s32 _svm_envx_hist[];
/* Sony LIBSND `_SsVmDoAllocate` (psyz vm_aloc2.c analog): set up the
   allocated voice's SPU shadow registers (start address, ADSR) and mark the
   voice's dirty bits. BB2 deltas vs psyz: _svm_voice stride 54, ADSR indexed
   by voiceOffset through the flat s16 shadow view D_80102A78[]. */
static inline void vmSetStartAddr(u16 addr) {
    _svm_sreg_buf[_svm_cur.voiceOffset + 3] = addr;
    _svm_sreg_dirty[_svm_cur.voice] |= 8;
}

void _SsVmDoAllocate(void) {
    int i;
    int progIdx;

    _svm_cur.voiceOffset = _svm_cur.voice * 8;
    _svm_cur.field_0x1e = _svm_cur.field_7_fake_program * 16 + _svm_cur.tone;
    _svm_voice[_svm_cur.voice].unk6 = 0x7FFF;
    for (i = 0; i < 16; i++) {
        _svm_envx_hist[i] &= ~(1 << _svm_cur.voice);
    }
    if ((_svm_cur.tone_vag_idx & 1) > 0) {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved2);
    } else {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved3);
    }
    _svm_sreg_buf[_svm_cur.voiceOffset + 4] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr1;
    _svm_sreg_buf[_svm_cur.voiceOffset + 5] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr2 + _svm_damper;
    _svm_sreg_dirty[_svm_cur.voice] |= 0x30;
}

void _SsVmDamperOff(void) {
    _svm_damper = 0;
}

INCLUDE_ASM("asm/funcs", _SsVmFlush);
/* kengo:HIGH  |  is_action/action_CheckHitZangeki  |  271i */
/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */
extern s32 D_800F19D0[2];
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u8  _svm_auto_kof_mode;
extern s16 kMaxPrograms;
extern u16 _svm_vab_count;
extern u16 D_800F1B14;
extern u16 D_800F2B68;

void _SsVmInit(s32 a0) {
    s32 buf[16];
    u16 i;

    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(0x20, D_800F19D0);

    i = 0;
    do {
        _svm_sreg_buf[i] = 0;
        i++;
    } while (i < 0xC0);
    i = 0;
    do {
        _svm_sreg_dirty[i] = 0;
        i++;
    } while (i < 0x18);
    _svm_vab_count = 0;
    i = 0;
    do {
        _svm_vab_used[i] = 0;
        i++;
    } while (i < 0x10);

    {
        u16 masked = (u8)a0;
        if (masked >= 0x18) {
            _SsVmMaxVoice = 0x18;
        } else {
            _SsVmMaxVoice = masked;
        }
    }

    buf[1] = 0x60093;
    i = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;

    if (_SsVmMaxVoice != 0) {
        do {
            _svm_voice[i].unk2 = 0x18;
            _svm_voice[i].unke = -1;
            _svm_voice[i].unk0 = 0xFF;
            _svm_voice[i].unk1b = 0;
            _svm_voice[i].unk04 = 0;
            _svm_voice[i].unk6 = 0;
            _svm_voice[i].unk10 = 0;
            _svm_voice[i].prog = 0;
            _svm_voice[i].tone = 0xFF;
            _svm_voice[i].unk8 = 0;
            _svm_voice[i].unkc = 0;
            _svm_voice[i].unka = 0x40;
            _svm_voice[i].auto_vol = 0;
            _svm_voice[i].unk1e = 0;
            _svm_voice[i].unk20 = 0;
            _svm_voice[i].unk22 = 0;
            _svm_voice[i].auto_pan = 0;
            _svm_voice[i].unk2a = 0;
            _svm_voice[i].unk2c = 0;
            _svm_voice[i].unk2e = 0;
            _svm_voice[i].start_pan = 0;
            _svm_voice[i].start_vol = 0;
            buf[0] = 1 << i;
            func_8008B488(buf);
            _svm_cur.voice = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < _SsVmMaxVoice);
    }

    _svm_rattr_plus_0x8 = 0x3FFF;
    _svm_rattr_plus_0xA = 0x3FFF;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    D_800F1B14 = 0;
    D_800F2B68 = 0;
    _svm_rattr = 0;
    _svm_rattr_plus_0x4 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
/* kengo:HIGH  |  md_game/md_game_end  |  249i */
extern u16 D_800A26E4[];

/* PsyQ LIBSND VM_N2P: note2pitch — a second exported entry point that splat
   merged into func_80086818. Split out 2026-08-10 (docs/naming/libscan/
   boundary_fixes.md); must stay immediately after its former host so the
   link order reproduces the original byte layout. C ref: psyz
   decomp/src/libsnd/vm_n2p.c (PsyQ 4.0). */
u16 note2pitch(void) {
    s32 octave;
    s32 note;
    s32 shiftVal;
    s32 semitones;
    s16 semitone;
    u32 tableIndex;
    u16 step;
    s16 shift;
    u16 pitch;
    s32 noteIndex;

    note = _svm_cur.note + (60 - _svm_cur.tone_center);
    shiftVal = _svm_cur.tone_shift;
    step = shiftVal / 8;
    semitones = (s16)note;
    octave = semitones / 12;
    semitone = semitones - (octave * 12);
    if (step >= 16) {
        step = 15;
    }
    noteIndex = semitone * 16;
    tableIndex = noteIndex + step;
    pitch = D_800A26E4[tableIndex];
    shift = octave - 5;
    if (shift > 0) {
        pitch <<= shift;
    } else if (shift < 0) {
        pitch >>= -shift;
    }
    return pitch;
}

/* PsyQ 4.0 LIBSND vmanager (VM_N2P): note2pitch2 — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libsnd/vmanager.c */
s32 note2pitch2(u16 arg0, u16 arg1) {
    s16 octave;
    s16 var_a2;
    s16 var_a3;
    short new_var;
    u16 var_v1;
    s32 pos;
    s32 tone;

    tone = _svm_cur.tone + (_svm_cur.field_7_fake_program * 0x10);
    var_a3 = (arg1 + _svm_tn[tone].shift) / 8;
    var_a2 = 0;
    if (var_a3 >= 16) {
        var_a2 = 1;
        var_a3 -= 16;
    }
    new_var = arg0 + 60 - _svm_tn[tone].center + var_a2;
    octave = new_var / 12;
    pos = (new_var % 12) * 16;
    var_v1 = D_800A26E4[pos + var_a3];

    octave -= 5;
    if (octave > 0) {
        var_v1 <<= octave;
    } else if (octave < 0) {
        var_v1 >>= -octave;
    }
    return var_v1;
}
extern void SpuSetNoiseVoice(s32, s32);
extern s32 SpuSetNoiseClock(s32);
extern u16 _svm_okon2;
extern u16 _svm_okof2;
/* Sony LIBSND `vmNoiseOn` (vm_no1.c): compute the noise voice's L/R volume
   (score channel volume x program volume x tone volume, then three pan
   stages and the optional mono fold), set the SPU noise clock from the
   note, queue the volume shadow registers, claim the voice for noise
   (pitch slot 0xA, noise state 2, every other voice's noise bit cleared),
   and set the key-on / reverb bits before switching the SPU noise voice on.
   Shape follows sotn-decomp src/main/psxsdk/libsnd/vmanager.c vmNoiseOn
   (US main build, matched); BB2's build calls SpuSetNoiseClock /
   SpuSetNoiseVoice where SOTN pokes the SPU registers directly.
   Symbol map: D_80102A78 <- _svm_sreg_buf (s16 view); D_800F65E0 <-
   _svm_sreg_dirty; D_800F1B14/D_800F2B68 <- _svm_orev1/2;
   D_800F1B10/12 <- _svm_okon1/2; D_801078D8/DA <- _svm_okof1/2. */
void vmNoiseOn(u8 vc) {
    u8 *score;
    s16 voice;
    s16 bitsLower;
    s16 bitsUpper;
    u32 voll_t, volr_t;
    u32 voll, volr;
    /* SOTN-verbatim (sotn-decomp src/main/psxsdk/libsnd/vmanager.c
       vmNoiseOn): temp holds the tone pan, then the program pan, then the
       voice pan, one per pan stage below. Owner Ruling 8, 2026-09-24
       (ordinary-c-judge-decidable.md), vmNoiseOn only. */
    u32 temp;
    u32 idx;

    score = (u8 *)(&_ss_score)[_svm_cur.seq_sep_no & 0xFF] +
            ((_svm_cur.seq_sep_no & 0xFF00) >> 8) * 0xB0;

    voll_t = *(u16 *)(score + 0x58) * 0x81;
    volr_t = *(u16 *)(score + 0x5A) * 0x81;

    voll_t = (voll_t * _svm_cur.mvol) / 0x7F;
    volr_t = (volr_t * _svm_cur.mvol) / 0x7F;

    voll_t = (voll_t * _svm_cur.tone_vol) / 0x7F;
    volr_t = (volr_t * _svm_cur.tone_vol) / 0x7F;

    temp = _svm_cur.tone_pan;
    if (temp < 0x40) {
        voll = voll_t;
        volr = (volr_t * temp) / 0x3F;
    } else {
        voll = (voll_t * (0x7F - temp)) / 0x3F;
        volr = volr_t;
    }
    temp = _svm_cur.mpan;
    if (temp < 0x40) {
        volr = (volr * temp) / 0x3F;
    } else {
        voll = (voll * (0x7F - temp)) / 0x3F;
    }
    temp = _svm_cur.pan;
    if (temp < 0x40) {
        volr = (temp * volr) / 0x3F;
    } else {
        voll = (voll * (0x7F - temp)) / 0x3F;
    }

    if (_svm_stereo_mono == 1) {
        if (voll < volr) {
            voll = volr;
        } else {
            volr = voll;
        }
    }

    /* FAKE: named-intermediate (no-new-park-categories.md 'Named-intermediate
       declaration order', once-written per ordinary-c-judge-decidable.md
       Ruling 1) - idx is the voice index, bound before the SpuSetNoiseClock
       call so its pseudo is live across that call and global.c seats it in
       call-saved $s0 as the target does (sched1 still places the zero-extend
       after the jal: no dependence ties it to the call). Measured on this
       final body (memory/grind/vmNoiseOn/evidence.md): vc at each use puts
       the index in $a0 and drops $s3 from the frame; idx also at the two
       _svm_voice[] uses differs too (the target zero-extends vc again there). */
    idx = vc;
    SpuSetNoiseClock((_svm_cur.note - _svm_cur.tone_center) & 0x3F);

    _svm_sreg_buf[idx * 8 + 0] = voll;
    _svm_sreg_buf[idx * 8 + 1] = volr;
    _svm_sreg_dirty[idx] |= 3;
    if (idx < 0x10) {
        bitsLower = 1 << idx;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (idx - 0x10);
    }
    _svm_voice[vc].unk04 = 0xA;
    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        _svm_voice[voice].unk1b &= 1;
    }
    _svm_voice[vc].unk1b = 2;

    _svm_okon1 |= bitsLower;
    _svm_okon2 |= bitsUpper;

    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;

    if (_svm_cur.tone_mode & 4) {
        D_800F1B14 |= bitsLower;
        D_800F2B68 |= bitsUpper;
    } else {
        D_800F1B14 &= ~bitsLower;
        D_800F2B68 &= ~bitsUpper;
    }

    SpuSetNoiseVoice(1, ((bitsUpper & 0xFF) << 16) | bitsLower);
}
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
/* Sony LIBSND `_SsVmKeyOffNow` (probable): mark the current voice's pending
   key-off bit, release the voice slot, and drop the matching key-on bit.
   Body is psyz vm_nowof.c verbatim (with BB2's _svm_voice record layout).
   Symbol map: D_801078D8/DA <- _svm_okof1/_svm_okof2; D_800F1B10/12 <-
   _svm_okon1/_svm_okon2. */
void _SsVmKeyOffNow(s32 mode) {
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;

    voice = _svm_cur.voice;
    if (voice < 16) {
        bitsLower = 1 << voice;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (voice - 16);
    }
    _svm_voice[voice].unk1b = 0;
    _svm_voice[voice].unk04 = 0;
    _svm_voice[voice].unk0 = 0;
    _svm_okof1 |= bitsLower;
    _svm_okof2 |= bitsUpper;
    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
}
/* Sony LIBSND `_SsVmKeyOnNow` (VM_NOWON): compute the current voice's L/R
   volume (VAB master x volume x program/tone volume, score channel volume,
   three pan stages, optional mono fold, square-law curve), queue the SPU
   register shadow + pitch, and set the key-on/reverb bits. Shape follows the
   SOTN psxsdk SpuVmKeyOnNow (sotn-decomp src/main/psxsdk/libsnd/vmanager.c)
   and the ps2sdk libsnd2 vm_nowon.c port; BB2's build squares the volumes
   only for sequence voices (seq_sep_no != 0x21) and has no unk1b store.
   Symbol map: D_80102A78 <- _svm_sreg_buf (s16 view); D_800F65E0 <-
   _svm_sreg_dirty; D_800F4E1C <- _svm_voice[].unk04;
   D_800F1B14/D_800F2B68 <- _svm_orev1/2; D_800F1B10/12 <- _svm_okon1/2;
   D_801078D8/DA <- _svm_okof1/2. */
void _SsVmKeyOnNow(s32 vagCount, u16 pitch) {
    u8 *score;
    u16 pos;
    s16 bitsLower;
    s16 bitsUpper;
    u32 voll, volr;
    u32 voll_t, volr_t;
    s32 mvol_scaled;

    mvol_scaled = _svm_vh->mvol * 0x3FFF;
    voll_t = (_svm_cur.volume * mvol_scaled) / 16129;
    volr_t = ((voll_t * _svm_cur.mvol) * _svm_cur.tone_vol) / 16129;
    pos = _svm_cur.voice * 8;
    voll_t = volr_t;
    score = (u8 *)(&_ss_score)[_svm_cur.seq_sep_no & 0xFF] +
            ((_svm_cur.seq_sep_no & 0xFF00) >> 8) * 0xB0;
    if (_svm_cur.seq_sep_no != 0x21) {
        voll_t = (voll_t * *(u16 *)(score + 0x58)) / 127;
        volr_t = (volr_t * *(u16 *)(score + 0x5A)) / 127;
    }
    if (_svm_cur.tone_pan < 64) {
        voll = voll_t;
        volr = (volr_t * _svm_cur.tone_pan) / 63;
    } else {
        voll = (voll_t * (127 - _svm_cur.tone_pan)) / 63;
        volr = volr_t;
    }
    if (_svm_cur.mpan < 64) {
        volr = (volr * _svm_cur.mpan) / 63;
    } else {
        voll = (voll * (127 - _svm_cur.mpan)) / 63;
    }
    if (_svm_cur.pan < 64) {
        volr = (volr * _svm_cur.pan) / 63;
    } else {
        voll = (voll * (127 - _svm_cur.pan)) / 63;
    }
    if (_svm_stereo_mono == 1) {
        if (voll < volr) {
            voll = volr;
        } else {
            volr = voll;
        }
    }
    if (_svm_cur.seq_sep_no != 0x21) {
        voll = (voll * voll) / 0x3FFF;
        volr = (volr * volr) / 0x3FFF;
    }
    _svm_sreg_buf[pos + 2] = pitch;
    _svm_sreg_buf[pos + 0] = voll;
    _svm_sreg_buf[pos + 1] = volr;
    _svm_sreg_dirty[_svm_cur.voice] |= 7;
    _svm_voice[_svm_cur.voice].unk04 = pitch;
    if (_svm_cur.voice < 16) {
        bitsLower = 1 << _svm_cur.voice;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (_svm_cur.voice - 16);
    }
    if (_svm_cur.tone_mode & 4) {
        D_800F1B14 |= bitsLower;
        D_800F2B68 |= bitsUpper;
    } else {
        D_800F1B14 &= ~bitsLower;
        D_800F2B68 &= ~bitsUpper;
    }
    _svm_okon1 |= bitsLower;
    _svm_okon2 |= bitsUpper;
    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;
}
/* func_80087770: Sony LIBSND vmanager _SsVmSetSeqVol. The volume chain
 * follows ps2sdk libsnd2 vm/vm_seq.c _SsVmSetSeqVol (BB2's build has no
 * _snd_vmask / vab-id checks); SOTN's SpuVmSetSeqVol is the same API in a
 * different build that only writes voll/volr * 0x81 per voice.
 * Store a sequence's master volume pair (clamped to 0x7F) in its score block,
 * then recompute the shadow volume registers of every voice that sequence
 * owns: a shared base from the VAB master volume and the voice's channel
 * volume, scaled by the program and tone volumes and by the sequence's
 * per-side volume, panned by the tone, program and voice pans, folded to mono
 * if requested, then squared into _svm_sreg_buf. The fourth argument is
 * unused (callers pass 1). */
s16 func_80087770(s16 seq_sep_no, u16 voll, u16 volr, s16 arg3) {
    u8 *score;
    s16 voice;
    u32 voll_t;
    u32 volr_t;
    u16 left;
    u16 right;
    u8 tone_pan;
    u8 prog_pan;
    u8 voice_pan;

    score = (u8 *)(&_ss_score)[seq_sep_no & 0xFF] +
            ((seq_sep_no & 0xFF00) >> 8) * 0xB0;
    *(u16 *)(score + 0x58) = voll;
    *(u16 *)(score + 0x5A) = volr;
    if (*(u16 *)(score + 0x58) >= 0x7F) {
        *(u16 *)(score + 0x58) = 0x7F;
    }
    if (*(u16 *)(score + 0x5A) >= 0x7F) {
        *(u16 *)(score + 0x5A) = 0x7F;
    }

    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        if (_svm_voice[voice].unke == seq_sep_no) {
            _SsVmVSetUp(_svm_voice[voice].vabId, _svm_voice[voice].unk10);
            voll_t = _svm_vh->mvol * 0x3FFF *
                     (_svm_voice[voice].unk8 *
                      *(s16 *)(score + 0x60 + _svm_voice[voice].unkc * 2) / 0x7F) /
                     0x3F01;
            volr_t = voll_t * _svm_pg[_svm_voice[voice].prog].mvol *
                     _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].vol /
                     0x3F01;
            voll_t = voll_t * _svm_pg[_svm_voice[voice].prog].mvol *
                     _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].vol /
                     0x3F01;
            voll_t = voll_t * *(u16 *)(score + 0x58) / 0x7F;
            volr_t = volr_t * *(u16 *)(score + 0x5A) / 0x7F;

            tone_pan = _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].pan;
            if (tone_pan < 0x40) {
                left = voll_t;
                right = volr_t * tone_pan / 0x3F;
            } else {
                left = voll_t * (0x7F - tone_pan) / 0x3F;
                right = volr_t;
            }
            prog_pan = _svm_pg[_svm_voice[voice].prog].mpan;
            if (prog_pan < 0x40) {
                right = right * prog_pan / 0x3F;
            } else {
                left = left * (0x7F - prog_pan) / 0x3F;
            }
            voice_pan = _svm_voice[voice].unka;
            if (voice_pan < 0x40) {
                right = right * voice_pan / 0x3F;
            } else {
                left = left * (0x7F - voice_pan) / 0x3F;
            }
            if (_svm_stereo_mono == 1) {
                if (right > left) {
                    left = right;
                } else {
                    right = left;
                }
            }
            left = left * left / 0x3FFF;
            right = right * right / 0x3FFF;
            _svm_sreg_buf[voice * 8] = left;
            _svm_sreg_buf[voice * 8 + 1] = right;
            _svm_sreg_dirty[voice] |= 3;
        }
    }
    return seq_sep_no;
}
s16 _SsVmGetSeqVol(s32 a0, s16 *a1, s16 *a2) {
    u8 *base;
    /* FAKE: materializes &D_801027F0.seq_sep_no once in a reg for store+reload; all pointer-free spellings measured 12 (s2, tmp/grind/func_80087CAC/s2/) */
    s16 *ptr;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&_ss_score)[(u8)a0];
    ptr = &_svm_cur.seq_sep_no;
    *ptr = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    *a1 = *(u16 *)(p + 0x58);
    *a2 = *(u16 *)(p + 0x5A);
    return *ptr;
}

s16 func_80087D10(s32 a0) {
    u8 *base;
    s32 slot;
    base = (u8 *)*(s32 *)((u8 *)&_ss_score + (u8)a0 * 4);
    _svm_cur.seq_sep_no = a0;
    slot = (a0 & 0xFF00) >> 8;
    base += slot * 176;
    return *(s16 *)(base + 0x58);
}

s16 func_80087D58(s32 a0) {
    u8 *base;
    s32 slot;
    base = (u8 *)*(s32 *)((u8 *)&_ss_score + (u8)a0 * 4);
    _svm_cur.seq_sep_no = a0;
    slot = (a0 & 0xFF00) >> 8;
    base += slot * 176;
    return *(s16 *)(base + 0x5A);
}
extern u8 _SsVmMaxVoice;
void _SsVmSeqKeyOff(s16 a0) {
    s32 s0 = 0;
    s16 s1;
    if (_SsVmMaxVoice == 0) {
        return;
    }
    s1 = (s16)a0;
    do {
        if (_svm_voice[(u8)s0].unke == s1) {
            _svm_cur.voice = (u8)s0;
            _SsVmKeyOffNow(0);
        }
        s0++;
    } while ((u8)s0 < _SsVmMaxVoice);
}
extern u8 _svm_vab_used[];
extern s16 kMaxPrograms;
extern s32 _svm_vab_vh[];
extern s32 _svm_vab_pg[];
extern s32 _svm_vab_tn[];
s32 _SsVmVSetUp(s32 a0, s32 a1) {
    u16 a0h;
    s16 a1h;
    s32 idx;
    s32 sa1;
    s32 v0;
    int v1;
    s32 v2;
    s32 entry;
    s32 ret;
    a0h = a0;
    a1h = a1;
    if ((a0 & 0xFFFFu) >= 0x10) goto fail;
    idx = (s16)a0h;
    if (_svm_vab_used[idx] != 1) return -1;
    sa1 = a1h;
    if (sa1 < kMaxPrograms) goto ok;
fail:
    return -1;
ok:
    ret = idx << 2;
    v0 = *(s32 *)((u8 *)_svm_vab_vh + ret);
    v1 = *(s32 *)((u8 *)_svm_vab_pg + ret);
    v2 = *(s32 *)((u8 *)_svm_vab_tn + ret);
    ret = sa1 << 4;
    _svm_cur.vabId = (u8) a0h;
    _svm_cur.prog = (u8) a1h;
    ret += v1;
    entry = *((s32 *) (ret + 8));
    _svm_vh = (VabHdr *)v0;
    _svm_pg = v1;
    _svm_tn = (VagAtr *)v2;
    _svm_cur.field_7_fake_program = (u8)entry;
    return 0;
}
/* kengo:MED  |  am_rmd/AddTbpOfst  |  49i */
extern u8 _svm_auto_kof_mode;
void SsSetAutoKeyOffMode(u8 a0) {
    _svm_auto_kof_mode = a0;
}
void SsSetMono(void) {
    _svm_stereo_mono = 1;
}

void SsSetStereo(void) {
    _svm_stereo_mono = 0;
}

extern u8 _SsVmMaxVoice;
s32 SsSetReservedVoice(s32 a0) {
    u8 v = (u8)a0;
    if (v >= 0x19 || v == 0) {
        return 0xFF;
    }
    _SsVmMaxVoice = a0;
    return v;
}
extern u8 _svm_vab_used[];
extern s32 _svm_vab_start[];
extern u16 _svm_vab_count;

void SsVabClose(s16 a0) {
    if ((u16)a0 < 0x10) {
        s16 idx = a0;
        if (_svm_vab_used[idx] == 1) {
            SpuFree(_svm_vab_start[idx]);
            _svm_vab_used[idx] = 0;
            _svm_vab_count--;
        }
    }
}
extern u8 _svm_vab_used[];

s16 SsVabFakeBody(s16 a0) {
    if ((u16)a0 < 0x11) {
        if (_svm_vab_used[a0] == 2) {
            _spu_setInTransfer(0);
            _svm_vab_used[a0] = 1;
            return a0;
        }
    }
    return -1;
}

s16 SsVabOpenHead(s32 a0, s16 a1) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 0, 0);
}

s16 SsVabOpenHeadSticky(s32 a0, s16 a1, s32 a2) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 1, (u32)a2);
}

s16 SsVabFakeHead(s32 a0, s16 a1, s32 a2) {
    return SsVabOpenHeadWithMode((u8 *)a0, a1, 1, (u32)a2);
}
extern u16 _svm_vab_count;
extern s32 _svm_vab_start[];
extern s32 _svm_vab_total[];
extern s32 SpuMalloc(s32);
/* PsyQ 4.0 LIBSND vs_vh: SsVabOpenHeadWithMode — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/vs_vh.c */
s16 SsVabOpenHeadWithMode(u8 *addr, s16 vabid, s16 arg2, u32 sbaddr) {
    int vagLens[256];
    s32 i;
    s32 var_s0;
    s16 vabId_2;
    u16 temp_v1;
    u16 *ptr_vag_off_table;
    u32 magic;
    u32 spuAllocMem;
    u8 num_vags;
    ProgAtr *pProgTable;
    u8 *var_a2;
    VabHdr *vab_hdr_2;
    u32 sum;
    vabId_2 = 0x10;
    if (_spu_getInTransfer() == 1) {
        return -1;
    }
    _spu_setInTransfer(1);
    if (vabid >= 0x10) {
        _spu_setInTransfer(0);
        return -1;
    }
    if (vabid == -1) {
        for (i = 0; i < 16; i++) {
            if (_svm_vab_used[i] == 0) {
                _svm_vab_used[i] = 1;
                vabId_2 = i;
                _svm_vab_count++;
                break;
            }
        }
    } else {
        var_a2 = _svm_vab_used;
        if (var_a2[vabid] == 0) {
            _svm_vab_used[vabid] = 1;
            vabId_2 = vabid;
            _svm_vab_count++;
        }
    }
    if (vabId_2 >= 0x10) {
        _spu_setInTransfer(0);
        return -1;
    }
    var_a2 = addr;
    _svm_vab_vh[vabId_2] = (s32)var_a2;

    var_a2 = var_a2 + 0x20;
    vab_hdr_2 = (VabHdr *)addr;
    magic = vab_hdr_2->form;
    if ((magic >> 8) != ('V' << 0x10 | 'A' << 0x8 | 'B')) {
        _svm_vab_used[vabId_2] = 0;
        _spu_setInTransfer(0);
        _svm_vab_count -= 1;
        return -1;
    }
    if ((magic & 0xFF) == 'p') {
        if (vab_hdr_2->ver >= 5) {
            kMaxPrograms = 0x80;
        } else {
            kMaxPrograms = 0x40;
        }
    } else {
        kMaxPrograms = 0x40;
    }
    if (vab_hdr_2->ps <= kMaxPrograms) {
        _svm_vab_pg[vabId_2] = (s32)var_a2;
        pProgTable = (ProgAtr *)var_a2;
        var_a2 = var_a2 + (kMaxPrograms * 0x10);
        var_s0 = 0;
        for (i = 0; i < kMaxPrograms; i++) {
            pProgTable[i].reserved1 = var_s0;
            if (pProgTable[i].tones != 0) {
                var_s0++;
            }
        }
        var_s0 = 0;
        _svm_vab_tn[vabId_2] = (s32)var_a2;
        ptr_vag_off_table = (u16 *)(var_a2 + (vab_hdr_2->ps << 9));
        num_vags = vab_hdr_2->vs;
        for (i = 0; i < 256; i++) {
            if (num_vags >= i) {
                temp_v1 = *ptr_vag_off_table;
                if (vab_hdr_2->ver >= 5) {
                    vagLens[i] = temp_v1 * 8;
                } else {
                    vagLens[i] = temp_v1 * 4;
                }
                var_s0 += vagLens[i];
            }
            ptr_vag_off_table++;
        }
        if (arg2 == 0) {
            spuAllocMem = SpuMalloc(var_s0);
            if (spuAllocMem == -1) {
                _svm_vab_used[vabId_2] = 0;
                _spu_setInTransfer(0);
                _svm_vab_count -= 1;
                return -1;
            }
        } else {
            spuAllocMem = sbaddr;
        }
        sum = spuAllocMem + var_s0;
        if (sum > 0x80000U) {
        end:
            _svm_vab_used[vabId_2] = 0;

            _spu_setInTransfer(0);
            _svm_vab_count -= 1;
            return -1;
        }
        _svm_vab_start[vabId_2] = spuAllocMem;
        var_s0 = 0;
        for (i = 0; i <= num_vags; i++) {
            var_s0 += vagLens[i];
            if (!(i & 1)) {
                pProgTable[i / 2].reserved2 = (spuAllocMem + var_s0) >> 3;
            } else {
                pProgTable[i / 2].reserved3 = (spuAllocMem + var_s0) >> 3;
            }
        }

        _svm_vab_total[vabId_2] = var_s0;
        _svm_vab_used[vabId_2] = 2;
    } else {
        goto end;
    }
    return vabId_2;
}
/* kengo:MED  |  sa_tan2/saTan2Main  |  247i */
extern u8 _svm_vab_used[];
extern s32 _svm_vab_start[];
extern s32 _svm_vab_total[];
extern s32 SpuSetTransferStartAddr(s32);
extern s32 SpuWrite(s32, s32);
s16 SsVabTransBody(s32 a0, s16 a1) {
    if ((u16)a1 >= 0x11) {
        _spu_setInTransfer(0);
        return -1;
    }
    if (_svm_vab_used[a1] != 2) {
        _spu_setInTransfer(0);
        return -1;
    }
    {
        s32 s0 = _svm_vab_start[a1];
        SpuSetTransferMode(0);
        SpuSetTransferStartAddr(s0);
        SpuWrite(a0, _svm_vab_total[a1]);
        _svm_vab_used[a1] = 1;
    }
    return a1;
}

s16 SsVabTransCompleted(s16 a0) {
    return SpuIsTransferCompleted(a0);
}

void SpuInit(void) {
    _SpuInit(0);
}

void _SpuInit(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    ResetCallback();
    _spu_init(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = (u16 *)&_spu_voice_centerNote_plus_0x2E;
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    SpuStart();
    _spu_rev_flag = 0;
    _spu_rev_reserve_wa = 0;
    _spu_rev_attr_plus_0x4 = 0;
    _spu_rev_attr_plus_0x8 = 0;
    _spu_rev_attr_plus_0xA = 0;
    _spu_rev_attr_plus_0xC = 0;
    _spu_rev_attr_plus_0x10 = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr[0];
    _spu_FsetRXX(0xD1, _spu_rev_startaddr[0], 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
}
extern s32 _spu_isCalled;
extern s32 _spu_EVdma;
void _spu_FiDMA(void);

void SpuStart(void) {
    s32 v0;
    if (_spu_isCalled == 0) {
        _spu_isCalled = 1;
        EnterCriticalSection();
        _SpuDataCallback((s32)_spu_FiDMA);
        v0 = OpenEvent((s32)0xF0000009, 0x20, 0x2000, 0);
        _spu_EVdma = v0;
        EnableEvent(v0);
        ExitCriticalSection();
    }
}
/* SPU-module debug strings (rodata 0x800163D8..0x80016420). Defined HERE —
 * before func_80088740 (_spu_init), their first user — so they emit into
 * .rodata AHEAD of the compiler-emitted jump tables of func_8008AF9C
 * (SpuSetCommonAttr), matching the original Sony spu.c literal order. */
const char D_800163D8[16] = "SPU:T/O [%s]\n";
const char D_800163E8[16] = "wait (reset)";
const char D_800163F8[20] = "wait (wrdy H -> L)";
const char D_8001640C[20] = "wait (dmaf clear/W)";

/* PsyQ 4.0 LIBSPU spu.c: _spu_init — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_init) */
s32 _spu_init(s32 a0) {
    u32 i;
    s32 channel;

    *D_800A2CEC |= 0xB0000;

    _spu_transMode = 0;
    _spu_addrMode = 0;
    _spu_tsa = 0;
    *(volatile u16 *)(_spu_RXX + 0x180) = 0;
    *(volatile u16 *)(_spu_RXX + 0x182) = 0;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = 0;
    _spu_Fw1ts();

    *(volatile u16 *)(_spu_RXX + 0x180) = 0;
    *(volatile u16 *)(_spu_RXX + 0x182) = 0;

    if (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) {
        i = 0;
        do {
            if (++i > 0xF00) {
                printf(&D_800163D8, &D_800163E8);
                break;
            }
        } while (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF);
    }

    channel = 0;
    _spu_mem_mode = 2;
    _spu_mem_mode_plus = 3;
    _spu_mem_mode_unit = 8;
    _spu_mem_mode_unitM = 7;
    *(volatile u16 *)(_spu_RXX + 0x1AC) = 4;
    *(volatile u16 *)(_spu_RXX + 0x184) = 0;
    *(volatile u16 *)(_spu_RXX + 0x186) = 0;
    *(volatile u16 *)(_spu_RXX + 0x18C) = 0xFFFF;
    *(volatile u16 *)(_spu_RXX + 0x18E) = 0xFFFF;
    *(volatile u16 *)(_spu_RXX + 0x198) = 0;
    *(volatile u16 *)(_spu_RXX + 0x19A) = 0;
    for (channel = 0; channel < 10; channel++) {
        _spu_RQ[channel] = 0;
    }

    if (a0 == 0) {
        s32 kon;
        s32 koff;
        volatile u16 *vp;

        _spu_tsa = 0x200;
        *(volatile u16 *)(_spu_RXX + 0x190) = 0;
        *(volatile u16 *)(_spu_RXX + 0x192) = 0;
        *(volatile u16 *)(_spu_RXX + 0x194) = 0;
        *(volatile u16 *)(_spu_RXX + 0x196) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B0) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B2) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B4) = 0;
        *(volatile u16 *)(_spu_RXX + 0x1B6) = 0;
        _spu_FwriteByIO((s32)&D_800A2D1C, 0x10);

        vp = (volatile u16 *)_spu_RXX;
        for (channel = 0; channel < 0x18; channel++) {
            vp[channel * 8 + 0] = 0;
            vp[channel * 8 + 1] = 0;
            vp[channel * 8 + 2] = 0x3FFF;
            vp[channel * 8 + 3] = 0x200;
            vp[channel * 8 + 4] = 0;
            vp[channel * 8 + 5] = 0;
        }

        kon = 0xFFFF;
        koff = 0xFF;
        *(volatile u16 *)(_spu_RXX + 0x188) = kon;
        *(volatile u16 *)(_spu_RXX + 0x18A) = koff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();

        *(volatile u16 *)(_spu_RXX + 0x18C) = kon;
        *(volatile u16 *)(_spu_RXX + 0x18E) = koff;
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
        _spu_Fw1ts();
    }

    _spu_inTransfer = 1;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = 0xC000;
    _spu_transferCallback = 0;
    _spu_IRQCallback = 0;
    return 0;
}
/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object
   (census 2026-07-09).  C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO).

   `_spu_RXX` (0x800A2CDC) holds the SPU register-file base (0x1F801C00), so
   `_spu_RXX + 0x1A6` is the SPU transfer/control register block at 0x1F801DA6:
   transfer address, data FIFO, SPUCNT, transfer control, SPUSTAT — five
   consecutive 16-bit hardware registers.  Sony's own libspu reaches them
   through `union SpuUnion *_spu_RXX` with the SPUR()/SPUW() field macros; the
   struct below is that same register block, and every access in this function
   goes through it, exactly as the original source does.

   Applying this body also deletes the redundant forward declaration
   `extern void _spu_FwriteByIO(s32, s32);` (HEAD src/main.c:1898); both call
   sites (src/main.c:1719, :1905) pass s32 values and compile unchanged. */
typedef struct {
    u16 trans_addr;  /* 0x1DA6 */
    u16 trans_fifo;  /* 0x1DA8 */
    u16 spucnt;      /* 0x1DAA */
    u16 trans_ctrl;  /* 0x1DAC */
    u16 spustat;     /* 0x1DAE */
} SpuCtrlRegs;

#define SPU_CTRL ((volatile SpuCtrlRegs *)(_spu_RXX + 0x1A6))

void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;

    cur = (u16 *)addr;
    spustat = SPU_CTRL->spustat & 0x7FF;
    SPU_CTRL->trans_addr = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num; i += 2) {
            SPU_CTRL->trans_fifo = *cur++;
        }
        cnt = SPU_CTRL->spucnt;
        cnt &= ~0x30;
        cnt |= 0x10;
        SPU_CTRL->spucnt = cnt;
        _spu_Fw1ts();
        if (SPU_CTRL->spustat & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (SPU_CTRL->spustat & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = SPU_CTRL->spucnt;
    j = 0;
    cnt &= ~0x30;
    SPU_CTRL->spucnt = cnt;
    if ((SPU_CTRL->spustat & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((SPU_CTRL->spustat & 0x7FF) != spustat);
    }
}
/* PsyQ LIBSPU spu.c: _spu_FiDMA + _spu_Fr_ — two further exported entry
   points that splat merged into func_800889D4. Split out 2026-08-10
   (docs/naming/libscan/boundary_fixes.md); both must stay immediately after
   their former host, in address order, so the link order reproduces the
   original byte layout. _spu_FiDMA.s also keeps the address label that marks
   its entry point, since that address is referenced as data elsewhere.
   Do NOT spell that label's symbol name in this file: engine/queue.py's
   not_a_c_function_text() word-searches the raw .c text (comments included),
   so naming it here makes it read as a C function and flood the queue as an
   unscorable distance -1 item, which sorts to the very top. */
/* PsyQ LIBSPU spu.c `_spu_FiDMA` (C ref: Xeeynamo/psyz decomp/src/libspu/spu.c:161).
   SPU DMA-completion interrupt handler: waits for the transfer-mode bits
   (0x30) in SPUCNT (_spu_RXX + 0x1AA) to clear with a bounded spin, then
   dispatches either the installed transfer callback or the SPU DMA event. */
void _spu_FiDMA(void) {
    u32 timeout;

    if (D_800A2D2C == 0) {
        _spu_Fw1ts();
    }
    *(volatile u16 *)(_spu_RXX + 0x1AA) =
        *(volatile u16 *)(_spu_RXX + 0x1AA) & ~0x30;
    timeout = 0;
    while (*(volatile u16 *)(_spu_RXX + 0x1AA) & 0x30) {
        timeout++;
        if (timeout > 0xF00) {
            break;
        }
    }
    if (_spu_transferCallback) {
        ((void (*)(void))_spu_transferCallback)();
        return;
    }
    DeliverEvent(0xF0000009, 0x20);
}
/* PsyQ 4.0 LIBSPU spu.c: _spu_Fr_ — unreferenced in BB2 (dead code carried
   by the linked Sony object; SpuRGetAllKeysStatus/S_SCA precedent).
   C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_r_); this build's
   WASTE_TIME() is the out-of-line _spu_Fw1ts call. */
void _spu_Fr_(s32 addr, u16 mode, s32 size) {
    *(volatile u16 *)(_spu_RXX + 0x1A6) = mode;
    _spu_Fw1ts();
    *(volatile u16 *)(_spu_RXX + 0x1AA) = *(volatile u16 *)(_spu_RXX + 0x1AA) | 0x30;
    _spu_Fw1ts();
    _spu_FsetDelayR();
    *D_800A2CE0 = addr;
    *D_800A2CE4 = (size << 16) | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
/* PsyQ 4.0 LIBSPU spu.c: _spu_t — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_t) */
typedef char *va_list;
#define va_start(ap, parmN) ((ap) = (va_list)(&(parmN) + 1))
#define va_arg(ap, T) ((ap) += sizeof(T), *(T *)((ap) - sizeof(T)))

s32 _spu_t(s32 mode, ...) {
    s32 var_a2;
    u32 i;
    va_list args;
    u32 count;
    u16 ck2;
    u16 cnt;
    u16 t;

    va_start(args, mode);

    switch (mode) {
    case 2:
        count = va_arg(args, u32);
        _spu_tsa = count >> _spu_mem_mode_plus;
        *(volatile u16 *)(_spu_RXX + 0x1A6) = _spu_tsa;
        break;

    case 1:
        t = *(volatile u16 *)(_spu_RXX + 0x1A6);
        i = 0;
        D_800A2D2C = 0;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1A6) != _spu_tsa);
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x20;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        break;

    case 0:
        t = *(volatile u16 *)(_spu_RXX + 0x1A6);
        i = 0;
        D_800A2D2C = 1;
        if ((t & 0xFFFF) != _spu_tsa) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1A6) != _spu_tsa);
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x30;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        break;

    case 3:
        if (D_800A2D2C == 1) {
            ck2 = 0x30;
        } else {
            ck2 = 0x20;
        }
        i = 0;
        while ((*(volatile u16 *)(_spu_RXX + 0x1AA) & 0x30) != ck2) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        if (D_800A2D2C == 1) {
            _spu_FsetDelayR();
        } else {
            _spu_FsetDelayW();
        }
        count = va_arg(args, u32);
        D_800A2D30 = count;
        count = va_arg(args, u32);
        D_800A2D34 = (count / 64);
        D_800A2D34 += ((count % 64) ? 1 : 0);
        *D_800A2CE0 = D_800A2D30;
        *D_800A2CE4 = (D_800A2D34 << 16) | 0x10;
        if (D_800A2D2C == 1) {
            var_a2 = 0x1000200;
        } else {
            var_a2 = 0x1000201;
        }
        *D_800A2CE8 = var_a2;
        break;
    }
    return 0;
}
/* kengo:MED  |  sa_tan0/saTan0GaugeDraw  |  164i */
s32 _spu_Fw(s32 a0, s32 a1) {
    if (_spu_transMode == 0) {
        _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
        _spu_t(1);
        _spu_t(3, a0, a1);
    } else {
        _spu_FwriteByIO(a0, a1);
    }
    return a1;
}
s32 _spu_Fr(s32 a0, s32 a1) {
    _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
    _spu_t(0);
    _spu_t(3, a0, a1);
    return a1;
}
void _spu_FsetRXX(s32 arg0, u32 arg1, s32 arg2) {
    if (arg2 == 0) {
        *(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1;
        return;
    }
    *(volatile u16 *)(arg0 * 2 + _spu_RXX) = arg1 >> _spu_mem_mode_plus;
}
s32 _spu_FsetRXXa(s32 mode, s32 val) {
    s32 aligned;
    if (_spu_mem_mode != 0) {
        u32 step = _spu_mem_mode_unit;
        if ((u32)val % step != 0) {
            val += step;
            val &= ~_spu_mem_mode_unitM;
        }
    }
    aligned = (s32)((u32)val >> _spu_mem_mode_plus);
    if (mode == -2) goto ret_val_m2;
    if (mode != -1) goto store;
    return aligned & 0xFFFF;
ret_val_m2:
    return val;
store:
    ((s16 *)_spu_RXX)[mode] = (s16)aligned;
    return val;
}
s32 _spu_FgetRXXa(s32 index, s32 mode) {
    u16 val = ((u16 *)_spu_RXX)[index];
    if (mode == -1) {
        return val;
    }
    return val << _spu_mem_mode_plus;
}
void _spu_FsetPCR(s32 arg0) {
    *D_800A2CEC &= 0xFFF8FFFF;
    if (arg0 != 0) {
        *D_800A2CEC |= 0x30000;
    } else {
        *D_800A2CEC |= 0x50000;
    }
}
extern volatile u32 *g_spu_dma_ctrl;
void _spu_FsetDelayW(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_FROM_RAM;
}
void _spu_FsetDelayR(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_TO_RAM;
}
void _spu_Fw1ts(void) {
    volatile s32 i;
    volatile s32 v = 0xD;
    for (i = 0; i < 0x3C; i++) {
        v = v * 13;
    }
}
void _SpuDataCallback(s32 a0) {
    DMACallback(4, a0);
}
/* PsyQ LIBSPU s_q.c: SpuQuit — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_q.c.
   g_spu_init_flag = _spu_transferCallback, g_spu_timer = _spu_IRQCallback
   (both volatile fn ptrs per Sony's header), g_snd_init_flag =
   _spu_isCalled. */


void SpuQuit(void) {
    if (_spu_isCalled == 1) {
        _spu_isCalled = 0;
        EnterCriticalSection();
        g_spu_init_flag = 0;
        _spu_IRQCallback = 0;
        _SpuDataCallback(0);
        CloseEvent(_spu_EVdma);
        DisableEvent(_spu_EVdma);
        ExitCriticalSection();
    }
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel DisableEvent\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xD\n"
    "    nop\n"
    "endlabel DisableEvent\n"
    ".set reorder\n"
    ".set at\n"
);




extern s32 _spu_AllocBlockNum;
extern s32 _spu_AllocLastNum;
extern s32 _spu_memList;

/* PsyQ LIBSPU s_m_init.c: SpuInitMalloc — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/
   s_m_init.c */
s32 SpuInitMalloc(s32 num, s32 *top) {
    s32 size;

    if (num > 0) {
        size = 0x10000 << _spu_mem_mode_plus;
        top[0] = 0x40001010;
        _spu_memList = (s32)top;
        _spu_AllocLastNum = 0;
        _spu_AllocBlockNum = num;
        top[1] = size - 0x1010;
        return num;
    }
    return 0;
}
extern void _spu_gcSPU(void);
/* PsyQ 4.0 LIBSPU s_m_m: SpuMalloc — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_m_m.c */
/* Self-referential on purpose: the object `_spu_memList` (Sony's SPU_MALLOC list pointer,
   declared s32 here) is viewed as SpuMemRec* through this macro; a macro name inside its
   own replacement list is not re-expanded (C90 6.8.3.4). Data wave 2026-09-07 renamed the
   symbol from g_spu_voice_key_c. */
#define _spu_memList ((SpuMemRec *)_spu_memList)
s32 SpuMalloc(s32 size) {
    s32 var_s2;
    s32 var_s3;
    s32 i;

    i = 0;
    var_s2 = -1;

    if (_spu_rev_reserve_wa == 0) {
        var_s3 = 0;
    } else {
        var_s3 = (0x10000 - _spu_rev_offsetaddr) << _spu_mem_mode_plus;
    }

    size += (size & ~_spu_mem_mode_unitM) ? _spu_mem_mode_unitM : 0;
    size >>= _spu_mem_mode_plus;
    size <<= _spu_mem_mode_plus;

    if (_spu_memList[0].addr & 0x40000000) {
        var_s2 = 0;
    } else {
        _spu_gcSPU();

        for (; i < _spu_AllocBlockNum; i++) {
            if (_spu_memList[i].addr & 0x40000000 ||
                (_spu_memList[i].addr & 0x80000000 &&
                 _spu_memList[i].size >= size)) {
                var_s2 = i;
                break;
            }
        }
    }

    if (var_s2 == -1)
        return -1;

    if (_spu_memList[var_s2].addr & 0x40000000) {
        if (var_s2 < _spu_AllocBlockNum &&
            _spu_memList[var_s2].size - var_s3 >= size) {
            s32 next = var_s2 + 1;

            /* Sony s_m_m.c has this volatile re-read verbatim (SOTN psxsdk
               keeps it with a "why the volatile?" note) -- original semantics */
            _spu_memList[next].addr =
                (*(volatile u32 *)&_spu_memList[var_s2].addr & 0x0FFFFFFF) +
                    size |
                0x40000000;
            _spu_memList[next].size = _spu_memList[var_s2].size - size;

            _spu_AllocLastNum = next;
            _spu_memList[var_s2].size = size;
            _spu_memList[var_s2].addr &= 0x0FFFFFFF;

            _spu_gcSPU();

            return _spu_memList[var_s2].addr;
        }
    } else {
        if (size < _spu_memList[var_s2].size &&
            _spu_AllocLastNum < _spu_AllocBlockNum) {
            u32 _addr = _spu_memList[var_s2].addr + size;
            u32 _size = _spu_memList[var_s2].size - size;
            SpuMemRec *kb =
                (SpuMemRec *)((_spu_AllocLastNum << 3) + (s32)_spu_memList);
            u32 swapAddr = kb->addr;
            u32 swapSize = kb->size;

            kb->addr = _addr | 0x80000000;
            kb->size = _size;
            _spu_AllocLastNum++;
            kb[1].addr = swapAddr;
            kb[1].size = swapSize;
        }

        _spu_memList[var_s2].size = size;
        _spu_memList[var_s2].addr &= 0x0FFFFFFF;
        _spu_gcSPU();

        return _spu_memList[var_s2].addr;
    }
    return -1;
}
/* kengo:HIGH  |  is_coli/coli_HitPauseKatana  |  178i  |  x2 size collision */
/* Shape note: phase 1's inner scan exits by `goto`, not `break`.
   stmt.c:expand_end_loop rolls a leading conditional exit to the bottom of the
   loop only when that exit jumps to the loop's own end_label/alt_end_label
   (the `last_test_insn` scan). A `break` qualifies, so the loop gets rotated
   and jump.c:duplicate_loop_exit_test then peels a guard copy (+8 insns). A
   `goto` to a user label after the loop does not target end_label, so
   last_test_insn stays 0, no rotation happens, and the emitted loop has the
   target's shape: test at top, unconditional `j` back-edge, `j++` in its delay
   slot.
   Depends on the maspsx .L-label load-delay nop gate
   (maspsx_label_nop_funcs.txt) for two hazard nops the assembler emits and
   maspsx's $L-only is_label() misses — as for siblings SpuFree and _spu_init
   in this same translation unit. */
/* PsyQ 4.0 LIBSPU s_m_int.c: _spu_gcSPU -- verbatim-linked Sony object
   (census 2026-07-09); C ref: Xeeynamo/psyz decomp/src/libspu/s_m_int.c */
void _spu_gcSPU(void) {
    s32 i;
    s32 j;

    for (i = 0; i <= _spu_AllocLastNum;) {
        if (_spu_memList[i].addr & 0x80000000) {
            for (j = i + 1;; j++) {
                if (_spu_memList[j].addr != 0x2FFFFFFF) {
                    goto scanned;
                }
            }
        scanned:
            if ((_spu_memList[j].addr & 0x80000000) &&
                ((_spu_memList[j].addr & 0x0FFFFFFF) ==
                 (_spu_memList[i].addr & 0x0FFFFFFF) + _spu_memList[i].size)) {
                _spu_memList[j].addr = 0x2FFFFFFF;
                _spu_memList[i].size += _spu_memList[j].size;
                continue;
            }
        }
        i++;
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].size == 0) {
            _spu_memList[i].addr = 0x2FFFFFFF;
        }
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= _spu_AllocLastNum; j++) {
            if (_spu_memList[j].addr & 0x40000000) {
                break;
            }
            if ((_spu_memList[j].addr & 0x0FFFFFFF) <
                (_spu_memList[i].addr & 0x0FFFFFFF)) {
                u32 swapAddr = _spu_memList[i].addr;
                u32 swapSize = _spu_memList[i].size;
                _spu_memList[i].addr = _spu_memList[j].addr;
                _spu_memList[i].size = _spu_memList[j].size;
                _spu_memList[j].addr = swapAddr;
                _spu_memList[j].size = swapSize;
            }
        }
    }

    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (_spu_memList[i].addr == 0x2FFFFFFF) {
            _spu_memList[i].addr = _spu_memList[_spu_AllocLastNum].addr;
            _spu_memList[i].size = _spu_memList[_spu_AllocLastNum].size;
            _spu_AllocLastNum = i;
            break;
        }
    }

    for (i = _spu_AllocLastNum - 1; i >= 0; i--) {
        if (!(_spu_memList[i].addr & 0x80000000)) {
            break;
        }
        _spu_memList[i].addr &= 0x0FFFFFFF;
        _spu_memList[i].addr |= 0x40000000;
        _spu_memList[i].size += _spu_memList[_spu_AllocLastNum].size;
        _spu_AllocLastNum = i;
    }
}
/* kengo:HIGH  |  md_game/exec_game  |  194i */
extern s32 _spu_AllocBlockNum;
extern void _spu_gcSPU(void);
/* PsyQ 4.0 LIBSPU s_m_f: SpuFree — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_m_f.c */
void SpuFree(u32 arg0) {
    s32 i;

    for (i = 0; i < _spu_AllocBlockNum; i++) {
        if (((SpuMemRec *)_spu_memList)[i].addr & 0x40000000) {
            break;
        }
        if (((SpuMemRec *)_spu_memList)[i].addr == arg0) {
            ((SpuMemRec *)_spu_memList)[i].addr |= 0x80000000;
            break;
        }
    }
    _spu_gcSPU();
}

void SpuSetNoiseVoice(s32 a0, s32 a1) {
    _SpuSetAnyVoice(a0, a1, 0xCA, 0xCB);
}

s32 _SpuSetAnyVoice(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (_spu_env & 1) {
        var_t0 = ((D_800F7298.raw[addr2] & 0xFF) << 16) | D_800F7298.raw[addr1];
    } else {
        var_t0 = ((((SpuUnion *)_spu_RXX)->raw[addr2] & 0xFF) << 16) | ((SpuUnion *)_spu_RXX)->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] |= bits;
            D_800F7298.raw[addr2] |= (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] |= bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] &= ~bits;
            D_800F7298.raw[addr2] &= ~((bits >> 16) & 0xFF);
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] &= ~bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] = bits;
            D_800F7298.raw[addr2] = (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] = bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
/* kengo:HIGH  |  is_coli/coli_HitPauseKatana_2  |  178i  |  x2 size collision */
s32 SpuSetNoiseClock(s32 a0) {
    s32 val;
    if (a0 < 0) {
        val = 0;
    } else if (a0 >= 0x40) {
        val = 0x3F;
    } else {
        val = a0;
    }
    {
        volatile u16 *ptr = (volatile u16 *)(_spu_RXX + 0x1AA);
        u16 tmp = *ptr;
        *ptr = (tmp & 0xC0FF) | ((val & 0x3F) << 8);
    }
    return val;
}
/* PsyQ 4.0 LIBSPU s_sr: SpuSetReverb — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_sr.c */
s32 SpuSetReverb(s32 on_off) {
    u16 cnt;
    switch (on_off) {
    case 0:
        cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
        _spu_rev_flag = 0;
        cnt &= ~0x80;
        *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        break;

    case 1:
        if ((_spu_rev_reserve_wa != on_off) && _SpuIsInAllocateArea_(_spu_rev_offsetaddr)) {
            cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
            _spu_rev_flag = 0;
            cnt &= ~0x80;
            *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA);
            _spu_rev_flag = on_off;
            cnt |= 0x80;
            *(volatile u16 *)((u8 *)_spu_RXX + 0x1AA) = cnt;
        }
        break;
    }

    return _spu_rev_flag;
}
/* PsyQ 4.0 LIBSPU s_m_util: _SpuIsInAllocateArea / _SpuIsInAllocateArea_ —
   verbatim-linked Sony object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libspu/s_m_util.c (4.0 adds the NULL list guard) */
s32 _SpuIsInAllocateArea(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)_spu_memList;
    s32 i;

    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}

s32 _SpuIsInAllocateArea_(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)_spu_memList;
    s32 i;

    arg0 <<= _spu_mem_mode_plus;
    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}
/* PsyQ 4.0 LIBSPU s_srmp: SpuSetReverbModeParam — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_srmp.c.
   4.0 deltas vs the SOTN revision: the DELAYTIME/FEEDBACK gates are range
   compares (mode >= ECHO && mode <= DELAY) with no default-arm clears, and
   the depth/zero split threads off the var_s4 flag. Reverb preset table:
   D_800A2D94 (= Sony rev_param table, 10 entries x 0x44). */
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

typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuReverbAttr;

extern RevParamEntry _spu_rev_param[]; /* rev_param preset table */

/* Sony _spu_rev_attr — ONE struct (sotn libspu_internal.h:87 struct
   SpuRevAttr), base 0x800A2888. Members == the split splat symbols
   D_800A288C (mode) / D_800A2890/92 (depth L/R) / D_800A2894 (delay) /
   D_800A2898 (feedback), which other already-matched functions in this TU
   still reference by their per-member names (same linked bytes). */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;
extern SpuRevAttr _spu_rev_attr; /* _spu_rev_attr */

void _spu_setReverbAttr(s32 *arg0);
s32 SpuClearReverbWorkArea(u32 rev_mode);

static inline void _memcpy(char *dst, char *src, u32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}

s32 SpuSetReverbModeParam(SpuReverbAttr *attr) {
    RevParamEntry entry;
    u32 var_s0;
    u16 cnt;

    s32 var_s7 = 0;
    s32 var_s4 = 0;
    s32 var_s6 = 0;
    s32 sp58 = 0;
    s32 var_fp = 0;

    u32 mask = attr->mask;
    s32 bSetAll = attr->mask == 0;

    entry.flags = 0;
    if (bSetAll || (mask & 0x1)) {
        var_s0 = attr->mode;
        if (attr->mode & 0x100) {
            var_s0 &= ~0x100;
            sp58 = 1;
        }
        if (var_s0 >= 0xA || _SpuIsInAllocateArea_(_spu_rev_startaddr[var_s0])) {
            return -1;
        }
        var_s4 = 1;
        _spu_rev_attr.mode = var_s0;
        _spu_rev_offsetaddr = _spu_rev_startaddr[_spu_rev_attr.mode];
        _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                sizeof(RevParamEntry));
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
            _spu_rev_attr.feedback = 0x7F;
            _spu_rev_attr.delay = 0x7F;
            break;
        case 8: /* SPU_REV_MODE_DELAY */
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0x7F;
            break;
        default:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0;
            break;
        }
    }
    if (bSetAll || (mask & 0x8)) {
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_s6 = 1;
            if (!var_s4) {
                _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                        sizeof(RevParamEntry));
                entry.flags = 0x0C011C00;
            }
            _spu_rev_attr.delay = attr->delay;
            entry.mLSAME = ((_spu_rev_attr.delay << 0xD) / 0x7F) - entry.dAPF1;
            entry.mRSAME = ((_spu_rev_attr.delay << 0xC) / 0x7F) - entry.dAPF2;
            entry.mLCOMB1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mRCOMB1;
            entry.dLSAME = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.dRSAME;
            entry.mLAPF1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mLAPF2;
            entry.mRAPF1 = ((_spu_rev_attr.delay << 0xC) / 0x7F) + entry.mRAPF2;
            break;
        default:
            break;
        }
    }
    if (bSetAll || (mask & 0x10)) {
        switch (_spu_rev_attr.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_fp = 1;
            if (!var_s4) {
                if (!var_s6) {
                    _memcpy((char *)&entry, (char *)&_spu_rev_param[_spu_rev_attr.mode],
                            sizeof(RevParamEntry));
                    entry.flags = 0x80;
                } else {
                    entry.flags |= 0x80;
                }
            }
            _spu_rev_attr.feedback = attr->feedback;
            entry.vWALL = (_spu_rev_attr.feedback * 0x8100) / 0x7F;
            break;
        default:
            break;
        }
    }
    if (var_s4) {
        var_s7 = (*(volatile u16 *)(_spu_RXX + 0x1AA) >> 7) & 1;
        if (var_s7) {
            cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~0x80;
            *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }
    if (!var_s4) {
        if (bSetAll || (mask & 0x2)) {
            *(u16 *)(_spu_RXX + 0x184) = attr->depth.left;
            _spu_rev_attr.depth.left = attr->depth.left;
        }
        if (bSetAll || (mask & 0x4)) {
            *(u16 *)(_spu_RXX + 0x186) = attr->depth.right;
            _spu_rev_attr.depth.right = attr->depth.right;
        }
    } else {
        *(u16 *)(_spu_RXX + 0x184) = 0;
        *(u16 *)(_spu_RXX + 0x186) = 0;
        _spu_rev_attr.depth.left = 0;
        _spu_rev_attr.depth.right = 0;
    }
    if (var_s4 || var_s6 || var_fp) {
        _spu_setReverbAttr((s32 *)&entry);
    }
    if (sp58) {
        SpuClearReverbWorkArea(_spu_rev_attr.mode);
    }
    if (var_s4) {
        _spu_FsetRXX(0xD1, _spu_rev_offsetaddr, 0);
        if (var_s7) {
            cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
            cnt |= 0x80;
            *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }
    return 0;
}
void _spu_setReverbAttr(s32 *arg0) {
    s32 flags = arg0[0];
    s32 zero = flags == 0;

    if (zero || (flags & 0x1)) { *(u16 *)(_spu_RXX + 0x1C0) = *(u16 *)((s32)arg0 + 0x4); }
    if (zero || (flags & 0x2)) { *(u16 *)(_spu_RXX + 0x1C2) = *(u16 *)((s32)arg0 + 0x6); }
    if (zero || (flags & 0x4)) { *(u16 *)(_spu_RXX + 0x1C4) = *(u16 *)((s32)arg0 + 0x8); }
    if (zero || (flags & 0x8)) { *(u16 *)(_spu_RXX + 0x1C6) = *(u16 *)((s32)arg0 + 0xA); }
    if (zero || (flags & 0x10)) { *(u16 *)(_spu_RXX + 0x1C8) = *(u16 *)((s32)arg0 + 0xC); }
    if (zero || (flags & 0x20)) { *(u16 *)(_spu_RXX + 0x1CA) = *(u16 *)((s32)arg0 + 0xE); }
    if (zero || (flags & 0x40)) { *(u16 *)(_spu_RXX + 0x1CC) = *(u16 *)((s32)arg0 + 0x10); }
    if (zero || (flags & 0x80)) { *(u16 *)(_spu_RXX + 0x1CE) = *(u16 *)((s32)arg0 + 0x12); }
    if (zero || (flags & 0x100)) { *(u16 *)(_spu_RXX + 0x1D0) = *(u16 *)((s32)arg0 + 0x14); }
    if (zero || (flags & 0x200)) { *(u16 *)(_spu_RXX + 0x1D2) = *(u16 *)((s32)arg0 + 0x16); }
    if (zero || (flags & 0x400)) { *(u16 *)(_spu_RXX + 0x1D4) = *(u16 *)((s32)arg0 + 0x18); }
    if (zero || (flags & 0x800)) { *(u16 *)(_spu_RXX + 0x1D6) = *(u16 *)((s32)arg0 + 0x1A); }
    if (zero || (flags & 0x1000)) { *(u16 *)(_spu_RXX + 0x1D8) = *(u16 *)((s32)arg0 + 0x1C); }
    if (zero || (flags & 0x2000)) { *(u16 *)(_spu_RXX + 0x1DA) = *(u16 *)((s32)arg0 + 0x1E); }
    if (zero || (flags & 0x4000)) { *(u16 *)(_spu_RXX + 0x1DC) = *(u16 *)((s32)arg0 + 0x20); }
    if (zero || (flags & 0x8000)) { *(u16 *)(_spu_RXX + 0x1DE) = *(u16 *)((s32)arg0 + 0x22); }
    if (zero || (flags & 0x10000)) { *(u16 *)(_spu_RXX + 0x1E0) = *(u16 *)((s32)arg0 + 0x24); }
    if (zero || (flags & 0x20000)) { *(u16 *)(_spu_RXX + 0x1E2) = *(u16 *)((s32)arg0 + 0x26); }
    if (zero || (flags & 0x40000)) { *(u16 *)(_spu_RXX + 0x1E4) = *(u16 *)((s32)arg0 + 0x28); }
    if (zero || (flags & 0x80000)) { *(u16 *)(_spu_RXX + 0x1E6) = *(u16 *)((s32)arg0 + 0x2A); }
    if (zero || (flags & 0x100000)) { *(u16 *)(_spu_RXX + 0x1E8) = *(u16 *)((s32)arg0 + 0x2C); }
    if (zero || (flags & 0x200000)) { *(u16 *)(_spu_RXX + 0x1EA) = *(u16 *)((s32)arg0 + 0x2E); }
    if (zero || (flags & 0x400000)) { *(u16 *)(_spu_RXX + 0x1EC) = *(u16 *)((s32)arg0 + 0x30); }
    if (zero || (flags & 0x800000)) { *(u16 *)(_spu_RXX + 0x1EE) = *(u16 *)((s32)arg0 + 0x32); }
    if (zero || (flags & 0x1000000)) { *(u16 *)(_spu_RXX + 0x1F0) = *(u16 *)((s32)arg0 + 0x34); }
    if (zero || (flags & 0x2000000)) { *(u16 *)(_spu_RXX + 0x1F2) = *(u16 *)((s32)arg0 + 0x36); }
    if (zero || (flags & 0x4000000)) { *(u16 *)(_spu_RXX + 0x1F4) = *(u16 *)((s32)arg0 + 0x38); }
    if (zero || (flags & 0x8000000)) { *(u16 *)(_spu_RXX + 0x1F6) = *(u16 *)((s32)arg0 + 0x3A); }
    if (zero || (flags & 0x10000000)) { *(u16 *)(_spu_RXX + 0x1F8) = *(u16 *)((s32)arg0 + 0x3C); }
    if (zero || (flags & 0x20000000)) { *(u16 *)(_spu_RXX + 0x1FA) = *(u16 *)((s32)arg0 + 0x3E); }
    if (zero || (flags & 0x40000000)) { *(u16 *)(_spu_RXX + 0x1FC) = *(u16 *)((s32)arg0 + 0x40); }
    if (zero || (flags < 0)) { *(u16 *)(_spu_RXX + 0x1FE) = *(u16 *)((s32)arg0 + 0x42); }
}

void SpuSetReverbVoice(s32 a0, s32 a1) {
    _SpuSetAnyVoice(a0, a1, 0xCC, 0xCD);
}

/* PsyQ 4.0 LIBSPU s_crwa: SpuClearReverbWorkArea — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libspu/s_crwa.c */
s32 SpuClearReverbWorkArea(u32 rev_mode) {
    volatile s32 callback;
    s32 oldTransmode;
    s32 var_s2;
    s32 var_s3;
    s32 transmodeCleared;
    u32 var_s0;
    u32 var_s1;

    callback = 0;
    transmodeCleared = 0;
    if (rev_mode >= 10 || _SpuIsInAllocateArea_(_spu_rev_startaddr[rev_mode])) {
        return -1;
    }
    if (rev_mode == 0) {
        var_s1 = 0x10 << _spu_mem_mode_plus;
        var_s2 = 0xFFF0 << _spu_mem_mode_plus;
    } else {
        var_s1 = (0x10000 - _spu_rev_startaddr[rev_mode]) << _spu_mem_mode_plus;
        var_s2 = _spu_rev_startaddr[rev_mode] << _spu_mem_mode_plus;
    }
    oldTransmode = _spu_transMode;
    if (_spu_transMode == 1) {
        _spu_transMode = 0;
        transmodeCleared = 1;
    }
    var_s3 = 1;
    if (_spu_transferCallback != 0) {
        callback = _spu_transferCallback;
        _spu_transferCallback = 0;
    }
    while (var_s3 != 0) {
        var_s0 = var_s1;
        if (var_s1 > 0x400) {
            var_s0 = 0x400;
        } else {
            var_s3 = 0;
        }

        _spu_t(2, var_s2);
        _spu_t(1);
        _spu_t(3, &_spu_zerobuf, var_s0);
        WaitEvent(_spu_EVdma);
        var_s1 -= 0x400;
        var_s2 += 0x400;
    }
    if (transmodeCleared != 0) {
        _spu_transMode = oldTransmode;
    }
    if (callback != 0) {
        _spu_transferCallback = callback;
    }
    return 0;
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel WaitEvent\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel WaitEvent\n"
    ".set reorder\n"
    ".set at\n"
);
/* PsyQ 4.0 LIBSPU s_sk: SpuSetKey — verbatim-linked Sony object (census
 * 2026-07-09); C ref: sotn-decomp src/psxsdk/libspu/s_sk.c shape + PsyQ 4.0
 * S_SK object relocs (_spu_RQ = one u16[4]). Volatile decls are Ruling-4
 * ground-truth-codegen grants (volatile_extern_allowlist.txt:40-44). */

void SpuSetKey(s32 on_off, u32 voice_bit) {
    u16 lo;
    u16 hi;
    u32 hi2;

    voice_bit &= 0xFFFFFF;
    lo = voice_bit;
    hi2 = voice_bit >> 16;
    hi = hi2;

    switch (on_off) {
    case 1:
        if (_spu_env & 1) {
            _spu_RQ[0] = lo;
            _spu_RQ[1] = hi;
            _spu_RQmask |= 1;
            _spu_RQvoice |= voice_bit;
            if (_spu_RQ[2] & voice_bit) {
                _spu_RQ[2] &= ~voice_bit;
            }
            if (_spu_RQ[3] & hi2) {
                _spu_RQ[3] &= ~hi2;
            }
        } else {
            u32 stat = _spu_keystat | voice_bit;
            ((SpuRXX *)_spu_RXX)->key_on[0] = lo;
            ((SpuRXX *)_spu_RXX)->key_on[1] = hi;
            _spu_keystat = stat;
        }
        break;
    case 0:
        if (_spu_env & 1) {
            _spu_RQ[2] = lo;
            _spu_RQ[3] = hi;
            _spu_RQmask |= 1;
            _spu_RQvoice &= ~voice_bit;
            if (_spu_RQ[0] & voice_bit) {
                _spu_RQ[0] &= ~voice_bit;
            }
            if (_spu_RQ[1] & hi2) {
                _spu_RQ[1] &= ~hi2;
            }
        } else {
            ((SpuRXX *)_spu_RXX)->key_off[0] = lo;
            ((SpuRXX *)_spu_RXX)->key_off[1] = hi;
            _spu_keystat &= ~voice_bit;
        }
        break;
    }
}
s32 SpuGetKeyStatus(s32 arg0) {
    s32 bit_found;
    s32 i;
    s32 one;
    s32 mask;
    s32 base;
    s32 flags;

    bit_found = -1;
    i = 0;
    one = 1;
    for (; i < 0x18; i++) {
        mask = one << i;
        if (arg0 & mask) {
            bit_found = i;
            break;
        }
    }

    if (bit_found != -1) goto work;
    return -1;
work:
    base = bit_found << 4;
    mask = _spu_RXX;
    flags = _spu_keystat;
    base = base + mask;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
    if (!flags) goto no_flags;
    if (!base) goto ret3;
    goto ret1;
no_flags:
    return (s32)(base != 0) << 1;
ret3:
    return 3;
ret1:
    return 1;
}
s32 SpuRead(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    _spu_Fr(a0, a1);
    if (g_spu_init_flag == 0) {
        g_spu_busy = 0;
    }
    return a1;
}
s32 SpuWrite(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    _spu_Fw(a0, a1);
    if (g_spu_init_flag == 0) {
        g_spu_busy = 0;
    }
    return a1;
}
extern s32 _spu_FsetRXXa(s32, s32);
s32 SpuSetTransferStartAddr(s32 a0) {
    s32 v0;
    if ((u32)(a0 - 0x1010) > (u32)0x7EFE8) {
        return 0;
    }
    v0 = _spu_FsetRXXa(-1, a0);
    _spu_tsa = (u16)v0;
    return (u32)(u16)v0 << _spu_mem_mode_plus;
}
/* PsyQ LIBSPU s_stm.c: SpuSetTransferMode — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_stm.c */
s32 SpuSetTransferMode(s32 mode) {
    s32 transMode;

    switch (mode) {
        case 0:
            transMode = 0;
            break;
        case 1:
            transMode = 1;
            break;
        default:
            transMode = 0;
    }
    _spu_trans_mode = mode;
    _spu_transMode = transMode;
    return transMode;
}
s32 SpuIsTransferCompleted(s32 arg0) {
    s32 var_v0;

    if ((_spu_trans_mode == 1) || (g_spu_busy == 1)) {
        return 1;
    }
    var_v0 = TestEvent(_spu_EVdma);
    if (arg0 == 1) {
        if (var_v0 == 0) {
            do {
                var_v0 = TestEvent(_spu_EVdma);
            } while (var_v0 == 0);
        }
        var_v0 = 1;
        goto block_8;
    }
    if (var_v0 == 1) {
block_8:
        g_spu_busy = var_v0;
    }
    return var_v0;
}
void _spu_setInTransfer(s32 a0) {
    if (a0 == 1) {
        g_spu_busy = 0;
    } else {
        g_spu_busy = 1;
    }
}

s32 _spu_getInTransfer(void) {
    return g_spu_busy != 1;
}

/* PsyQ 4.0 LIBSPU s_sca: SpuSetCommonAttr — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_sca.c
   (4.0 block order: mvol L/R, cd vol L/R, ext vol L/R, cd rev/mix,
   ext rev/mix). */
void SpuSetCommonAttr(void *arg0) {
    SpuCommonAttr *attr = arg0;
    u16 mvol_mode_left;
    u16 mvol_mode_right;
    u16 vol_total_left;
    u16 vol_total_right;
    u32 mask;
    s32 bSetAll;
    u16 cnt;

    vol_total_left = 0;
    vol_total_right = 0;
    mask = attr->mask;
    bSetAll = attr->mask == 0;

    if (bSetAll || (mask & 0x1)) {
        if (bSetAll || (mask & 0x4)) {
            switch ((s16)attr->mvolmode.left) {
            case 1:
                mvol_mode_left = 0x8000;
                break;
            case 2:
                mvol_mode_left = 0x9000;
                break;
            case 3:
                mvol_mode_left = 0xA000;
                break;
            case 4:
                mvol_mode_left = 0xB000;
                break;
            case 5:
                mvol_mode_left = 0xC000;
                break;
            case 6:
                mvol_mode_left = 0xD000;
                break;
            case 7:
                mvol_mode_left = 0xE000;
                break;
            case 0:
                vol_total_left = attr->mvol.left;
                mvol_mode_left = 0;
                break;
            default:
                vol_total_left = attr->mvol.left;
                mvol_mode_left = 0;
                break;
            }
        } else {
            vol_total_left = attr->mvol.left;
            mvol_mode_left = 0;
        }

        if (mvol_mode_left != 0) {
            if ((s16)attr->mvol.left >= 0x80) {
                vol_total_left = 0x7F;
            } else if ((s16)attr->mvol.left < 0) {
                vol_total_left = 0;
            } else {
                vol_total_left = attr->mvol.left;
            }
        }
        vol_total_left &= 0x7FFF;
        *(u16 *)(_spu_RXX + 0x180) = vol_total_left | mvol_mode_left;
    }

    if (bSetAll || (mask & 0x2)) {
        if (bSetAll || (mask & 0x8)) {
            switch ((s16)attr->mvolmode.right) {
            case 1:
                mvol_mode_right = 0x8000;
                break;
            case 2:
                mvol_mode_right = 0x9000;
                break;
            case 3:
                mvol_mode_right = 0xA000;
                break;
            case 4:
                mvol_mode_right = 0xB000;
                break;
            case 5:
                mvol_mode_right = 0xC000;
                break;
            case 6:
                mvol_mode_right = 0xD000;
                break;
            case 7:
                mvol_mode_right = 0xE000;
                break;
            case 0:
                vol_total_right = attr->mvol.right;
                mvol_mode_right = 0;
                break;
            default:
                vol_total_right = attr->mvol.right;
                mvol_mode_right = 0;
                break;
            }
        } else {
            vol_total_right = attr->mvol.right;
            mvol_mode_right = 0;
        }

        if (mvol_mode_right != 0) {
            if ((s16)attr->mvol.right >= 0x80) {
                vol_total_right = 0x7F;
            } else if ((s16)attr->mvol.right < 0) {
                vol_total_right = 0;
            } else {
                vol_total_right = attr->mvol.right;
            }
        }
        vol_total_right &= 0x7FFF;
        *(u16 *)(_spu_RXX + 0x182) = vol_total_right | mvol_mode_right;
    }

    if (bSetAll || (mask & 0x40)) {
        *(u16 *)(_spu_RXX + 0x1B0) = attr->cd.volume.left;
    }

    if (bSetAll || (mask & 0x80)) {
        *(u16 *)(_spu_RXX + 0x1B2) = attr->cd.volume.right;
    }

    if (bSetAll || (mask & 0x400)) {
        *(u16 *)(_spu_RXX + 0x1B4) = attr->ext.volume.left;
    }

    if (bSetAll || (mask & 0x800)) {
        *(u16 *)(_spu_RXX + 0x1B6) = attr->ext.volume.right;
    }

    if (bSetAll || (mask & 0x100)) {
        if (attr->cd.reverb == 0) {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~4;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt |= 4;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x200)) {
        if (attr->cd.mix == 0) {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~1;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt |= 1;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x1000)) {
        if (attr->ext.reverb == 0) {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~8;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt |= 8;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x2000)) {
        if (attr->ext.mix == 0) {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt &= ~2;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(_spu_RXX + 0x1AA);
            cnt |= 2;
            *(u16 *)(_spu_RXX + 0x1AA) = cnt;
        }
    }
}
/* PsyQ 4.0 LIBSPU sr_gaks: SpuRGetAllKeysStatus — verbatim-linked Sony object
   (census 2026-07-09; module SR_GAKS spans 0x8008B330..0x8008B488). This
   entry point is UNREFERENCED in BB2 (dead code pulled in by whole-object
   linking) — no glabel exists at 0x8008B330, so it shares func_8008AF9C's
   splat extent. C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c. */
static s32 SpuRGetAllKeysStatus(s32 min, s32 max, s8 *status) {
    s32 voice;
    u16 volumex;

    if (min < 0) {
        min = 0;
    }
    if (min >= 24) {
        return -3;
    }
    if (max >= 24) {
        max = 23;
    }
    if (max < 0 || max < min) {
        return -3;
    }

    max++;
    for (voice = min; voice < max; voice++) {
        s32 off = voice << 4;
        s32 bit;
        volumex = *(u16 *)((off + _spu_RXX) + 0xC);
        bit = _spu_keystat & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else {
            if (volumex != 0) {
                status[voice] = 2;
            } else {
                status[voice] = 0;
            }
        }
    }

    return 0;
}
/* PsyQ LIBSPU sr_gaks.c: SpuGetAllKeysStatus — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c
   (SpuRGetAllKeysStatus inlined with min=0, max=NUM_SPU_CHANNELS) */
void SpuGetAllKeysStatus(u8 *status) {
    s32 limit = 24;
    s32 voice = 0;

    do {
        s32 off = voice << 4;
        u16 volumex;
        s32 bit;
        volumex = *((u16 *)((off + _spu_RXX) + 0xC));
        bit = _spu_keystat & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else if (volumex != 0) {
            status[voice] = 2;
        } else {
            status[voice] = 0;
        }
        voice++;
    } while (voice < limit);
}
/* func_8008B488: per-voice SPU attribute setter with the shape of PsyQ
 * LIBSPU's SpuSetVoiceAttr (C ref: sotn-decomp src/main/psxsdk/libspu/s_sva.c
 * and psyz decomp/src/libspu/sr_sv.c). BB2 links an older build: no min/max
 * voice range, a different block order, and the SR mode defaulting to 0x100.
 * The name stays auto (near-tier-ruling-2026-09-07: no verbatim caller pins
 * it). SpuVoiceAttr per PsyQ libspu.h (sizeof = 0x40, the callers' s32[16]). */
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

extern u16 _spu_voice_centerNote[];

void func_8008B488(SpuVoiceAttr *attr) {
    volatile s32 i;
    volatile s32 v;
    s32 voice;
    s32 pos;
    u32 mask;
    s32 bSetAll;

    mask = attr->mask;
    bSetAll = mask == 0;
    for (voice = 0; voice < 24; voice++) {
        u16 temp; /* two values: the clamped sustain rate (SR block), then the
                   * clamped sustain level (SL block); Ruling 11, proof in
                   * memory/grind/func_8008B488/r11/proof.md */

        if ((attr->voice & (1 << voice)) == 0) {
            continue;
        }
        pos = voice * 8;

        if (bSetAll || (mask & 0x10)) {
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) = attr->pitch;
        }
        if (bSetAll || (mask & 0x40)) {
            _spu_voice_centerNote[voice] = attr->sample_note;
        }
        if (bSetAll || (mask & 0x20)) {
            u16 center;
            u16 note;

            center = _spu_voice_centerNote[voice];
            note = attr->note;
            *(volatile u16 *)(_spu_RXX + (pos + 2) * 2) =
                _spu_note2pitch(center >> 8, center & 0xFF, note >> 8, note & 0xFF);
        }
        if (bSetAll || (mask & 0x1)) {
            u16 volmode_left;
            u16 vol_left;

            vol_left = attr->volume.left & 0x7FFF;
            volmode_left = 0;
            if (bSetAll || (mask & 0x4)) {
                switch (attr->volmode.left) {
                case 1:
                    volmode_left = 0x8000;
                    break;
                case 2:
                    volmode_left = 0x9000;
                    break;
                case 3:
                    volmode_left = 0xA000;
                    break;
                case 4:
                    volmode_left = 0xB000;
                    break;
                case 5:
                    volmode_left = 0xC000;
                    break;
                case 6:
                    volmode_left = 0xD000;
                    break;
                case 7:
                    volmode_left = 0xE000;
                    break;
                }
            }
            if (volmode_left != 0) {
                if (attr->volume.left >= 0x80) {
                    vol_left = 0x7F;
                } else if (attr->volume.left < 0) {
                    vol_left = 0;
                }
            }
            *(volatile u16 *)(_spu_RXX + pos * 2) = vol_left | volmode_left;
        }
        if (bSetAll || (mask & 0x2)) {
            u16 volmode_right;
            u16 vol_right;

            vol_right = attr->volume.right & 0x7FFF;
            volmode_right = 0;
            if (bSetAll || (mask & 0x8)) {
                switch (attr->volmode.right) {
                case 1:
                    volmode_right = 0x8000;
                    break;
                case 2:
                    volmode_right = 0x9000;
                    break;
                case 3:
                    volmode_right = 0xA000;
                    break;
                case 4:
                    volmode_right = 0xB000;
                    break;
                case 5:
                    volmode_right = 0xC000;
                    break;
                case 6:
                    volmode_right = 0xD000;
                    break;
                case 7:
                    volmode_right = 0xE000;
                    break;
                }
            }
            if (volmode_right != 0) {
                if (attr->volume.right >= 0x80) {
                    vol_right = 0x7F;
                } else if (attr->volume.right < 0) {
                    vol_right = 0;
                }
            }
            *(volatile u16 *)(_spu_RXX + (pos + 1) * 2) = vol_right | volmode_right;
        }
        if (bSetAll || (mask & 0x80)) {
            _spu_FsetRXXa(pos | 3, attr->addr);
        }
        if (bSetAll || (mask & 0x10000)) {
            _spu_FsetRXXa(pos | 7, attr->loop_addr);
        }
        if (bSetAll || (mask & 0x20000)) {
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = attr->adsr1;
        }
        if (bSetAll || (mask & 0x40000)) {
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = attr->adsr2;
        }
        if (bSetAll || (mask & 0x800)) {
            u16 ar_rate;
            s32 amode;
            s32 adsr;

            ar_rate = attr->ar;
            if (ar_rate >= 0x80) {
                ar_rate = 0x7F;
            }
            amode = 0;
            if (bSetAll || (mask & 0x100)) {
                if (attr->a_mode == 5) {
                    amode = 0x80;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((ar_rate | amode) << 8);
        }
        if (bSetAll || (mask & 0x1000)) {
            u16 dr_rate;
            s32 adsr;

            dr_rate = attr->dr;
            if (dr_rate >= 0x10) {
                dr_rate = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            adsr &= 0xFF0F;
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (dr_rate << 4);
        }
        if (bSetAll || (mask & 0x2000)) {
            s32 smode;
            s32 adsr;

            temp = attr->sr;
            if (temp >= 0x80) {
                temp = 0x7F;
            }
            smode = 0x100;
            if (bSetAll || (mask & 0x200)) {
                switch (attr->s_mode) {
                case 1:
                    smode = 0;
                    break;
                case 5:
                    smode = 0x200;
                    break;
                case 7:
                    smode = 0x300;
                    break;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);
            adsr &= 0x3F;
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((temp | smode) << 6);
        }
        if (bSetAll || (mask & 0x4000)) {
            u16 rr_rate;
            s32 rmode;
            s32 adsr;

            rr_rate = attr->rr;
            if (rr_rate >= 0x20) {
                rr_rate = 0x1F;
            }
            rmode = 0;
            if (bSetAll || (mask & 0x400)) {
                switch (attr->r_mode) {
                case 3:
                    break;
                case 7:
                    rmode = 0x20;
                    break;
                }
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);
            adsr &= 0xFFC0;
            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (rr_rate | rmode);
        }
        if (bSetAll || (mask & 0x8000)) {
            s32 adsr;

            temp = attr->sl;
            if (temp >= 0x10) {
                temp = 0xF;
            }
            adsr = *(volatile u16 *)(_spu_RXX + (pos + 4) * 2);
            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | temp;
        }
    }
    v = 1;
    for (i = 0; i < 2; i++) {
        v *= 13;
    }
}
/* kengo:MED  |  sa_tan1/saTan1MainJump  |  413i  |  -10 */
/* PsyQ LIBSPU S_N2P: _spu_2pitch — a second exported entry point that splat
   merged into func_8008B488. Split out 2026-08-10 (docs/naming/libscan/
   boundary_fixes.md); must stay immediately after its former host so the
   link order reproduces the original byte layout. */
/* Pitch interpolation helper: scales `atten` (12.12 fixed) by the 48th-root-
   of-two step 0x103B/0x1000 once per 32 cents (rem >> 5), then linearly
   interpolates the remaining 0..31 cents between the two adjacent steps.
   COMPLETED-C 2026-09-02 (grinder s1, pure C, no FAKE constructs). */
inline u32 _spu_2pitch(u32 atten, u32 rem) {
    u32 ratio = 0x103B;
    u32 lower = atten << 12;
    s32 i;
    s32 steps = rem >> 5;
    u32 frac = rem & 0x1F;
    u32 upper;

    upper = atten * ratio;
    for (i = 0; i < steps; i++) {
        lower = atten * ratio;
        ratio *= 0x103B;
        ratio >>= 12;
        upper = atten * ratio;
    }
    return (lower + (((upper - lower) >> 5) * frac)) >> 12;
}
/* CANDIDATE — _spu_note2pitch (src/main.c)  sandbox --disable all = 0  (s2b, 2026-09-06)
 * Supersedes the s2 body (per-arm `(u16)` casts) that layer-1 FAILed and whose
 * casts are now on this function's BANNED list. No cast, no per-arm narrowing.
 *
 * CHASSIS (both required, unchanged from s1/s2):
 *  1. The sibling `_spu_2pitch` (COMPLETED-C, defined immediately above in
 *     src/main.c) must carry the GNU89 `inline` keyword:
 *         inline u32 _spu_2pitch(u32 atten, u32 rem) { ... }
 *     GCC 2.7.2 integrates it here AND still emits the out-of-line body;
 *     sandbox _spu_2pitch --disable all = 0 with the keyword in place. The
 *     target's tail (0x103B curve walk, `upper` spilled to 0x8($sp), the
 *     16-byte frame) IS that inlined copy.  Measured 2026-09-06 (s2b): making
 *     the sibling's first parameter `u16` instead breaks it (sibling 1,
 *     _spu_note2pitch 22) — the parameter stays u32.
 *  2. include/m2c_context.h:1184 prototype `u16 _spu_note2pitch(u16,u16,u16,u16);`
 *     (already so at HEAD; the sole caller is func_8008B488).
 *
 * WHAT CLOSES THE LAST 2 INSNS (residual: `andi $a2,$v0,0xFFFF` vs
 *  `addiu $a0,$zero,0x103B` at .L8008BBB0):
 *  the u16->u32 widening of the octave attenuation must land in a pseudo that
 *  is NOT single-set at sched1 time, otherwise sched.c adjust_priority ->
 *  birthing_insn_p (reg_n_sets[i]==1) boosts the andi to LAUNCH_PRIORITY, the
 *  backward list scheduler picks it first and therefore EMITS it last (after
 *  the li).  Staging the value through `diff` — a local the function already
 *  owns, dead from the `if (diff >= 0)` test onward — makes that pseudo
 *  two-set, the boost does not fire, the pair falls to the LUID tie-break and
 *  comes out in the target's order.  `diff` is also the variable the target
 *  keeps in $a2, so the seats match too (`andi $a2,$v0,0xFFFF`).
 *  Measured this session: staging through `diff` = 0; through `absdiff` = 5;
 *  `tgt` = 8; `cen` = 28; `oct` = 28; a FRESH u32 receiver = 2 (single-set,
 *  boost still fires); no staging at all (widen at the call) = 2.
 *  Family: staged-value-reused-variable (.claude/rules/staged-value-reused-variable.md).
 */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 atten;
    u32 pitch;
    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = (diff < 0) ? -diff : diff;
    oct = absdiff / 1536;
    rem = absdiff % 1536;
    if (diff >= 0) {
        atten = 0x1000 << oct;
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = 0x1000 >> oct;
    }
    /* FAKE: the octave attenuation is staged through `diff` (dead from the
       `diff >= 0` test above onward; nothing reads it after this point) and
       consumed on the very next line, mechanism: GCC 2.7.2 sched.c
       adjust_priority -> birthing_insn_p (reg_n_sets[regno]==1) — a two-set
       pseudo is not boosted to LAUNCH_PRIORITY, so the widening `andi` is
       emitted before the inlinee's `li 0x103B` as in the target,
       lever-exhaustion: memory/grind/_spu_note2pitch/hypotheses.md s1 H1/H5/H6
       and s2b (24 measured spellings, all >= 2 without this staging) */
    diff = atten;
    pitch = _spu_2pitch(diff, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch) {
    u16 search;
    s32 bit;
    s32 oct;
    s32 scale;
    u32 curve;
    u32 lower;
    u32 upper;
    u32 step;
    u32 acc;
    u32 next;
    u32 lo;
    u32 hi;
    s32 base;
    s32 i;
    s32 inner;
    s32 result;
    s32 quot;
    s32 rem;
    s32 note;
    s32 fine;

    search = ~pitch;
    bit = 0;
    for (i = 15; i >= 0; i--) {
        if (!((search >> i) & 1)) {
            bit = i;
            break;
        }
    }
    oct = bit - 12;
    scale = 1 << bit;
    curve = 0x1000;
    for (i = 0; i < 0x30; i++) {
        lower = scale * curve;
        curve *= 0x103B;
        curve >>= 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        for (inner = 0, base = i * 32, acc = 0, next = step; inner < 0x20; inner++) {
            lo = lower + acc;
            hi = lower + next;
            lo >>= 12;
            hi >>= 12;
            if (pitch >= lo && pitch < hi) {
                result = base + inner;
                goto found;
            }
            next += step;
            acc += step;
        }
    }
    result = 0x600;
found:
    quot = result / 128;
    rem = result % 128;
    note = cen_note + quot + oct * 12;
    fine = cen_fine + rem;
    return (note << 8) | fine;
}
void SpuGetVoiceVolume(s32 arg0, u16 *arg1, u16 *arg2) {
    u16 temp_v1;
    u16 temp_a0_2;
    u32 temp_a3;
    u32 temp_v1_2;
    s32 temp;

    temp = (arg0 << 4) + _spu_RXX;
    temp_v1 = *(u16 *)(temp);
    temp_a0_2 = *(u16 *)(temp + 2);
    temp_a3 = temp_v1 & 0xFFFF;
    if (temp_a3 >= 0x4000U) {
        u32 sub = 0x8000;
        *arg1 = temp_a3 - sub;
    } else {
        *arg1 = temp_v1;
    }
    temp_v1_2 = temp_a0_2 & 0xFFFF;
    if (temp_v1_2 >= 0x4000U) {
        u32 sub = 0x8000;
        *arg2 = temp_v1_2 - sub;
        return;
    }
    *arg2 = temp_a0_2;
}

void SpuGetVoiceEnvelope(s32 a0, u16 *a1) {
    a0 = (a0 << 4) + _spu_RXX;
    *a1 = *(u16 *)(a0 + 0xC);
}

void AddCOMB(void) {
    s32 v0;
    v0 = EnterCriticalSection();
    AddDrv(&D_800A307C);
    if (v0 == 1) {
        ExitCriticalSection();
    }
}
extern s32 g_str_sio;

void DelCOMB(void) {
    s32 v0;
    v0 = EnterCriticalSection();
    DelDrv(&g_str_sio);
    FlushCache();
    if (v0 == 1) {
        ExitCriticalSection();
    }
}

void ChangeClearSIO(void) {
}
/* PsyQ LIBCOMB comb: SioAnsyncRead (static) — verbatim-linked Sony object
   (census 2026-07-09; ground truth tmp/libscan/psyq40/LIBCOMB.LIB). The
   SIO async state (D_800F1B00/04 buf/len) is mutated at interrupt time by
   HandleSio (static @0x8008C9F4) — volatile is original semantics
   (operator-audited grant 2026-07-10, volatile_extern_allowlist.txt). */
extern volatile s32 D_800F1AFC;
extern volatile s32 D_800F1B00;
extern volatile s32 D_800F1B04;
extern s32 D_800A3044;
s32 SioAnsyncRead(int a0, int a1) {
    volatile s32 *flag = &D_800F1AFC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1B04 = a1;
    D_800F1B00 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        {
            volatile u16 *ptr = (volatile u16 *)(reg + 0xA);
            *ptr |= 0x800;
            *ptr |= 0x20;
        }
    }
    return 0;
}
extern volatile u16 regs_plus_0x8;
extern volatile u16 regs_plus_0xA;
extern u16 regs_plus_0xE;
extern s32 (*CombWaitCallback)(s32, s32);
extern s16 D_800A3074[4];
extern void DeliverEvent(s32, s32);

s32 SioSyncroRead(u8 *arg0, s32 arg1) {
    s32 r_arg1 = arg1;
    volatile s32 *flag = &D_800F1AFC;
    s32 count;
    s32 retries = 0;
    s32 pkt_len;
    s32 spu;
    s32 (*cb)(s32, s32);

    if (*flag != 0) return -1;
    goto main_work;

cleanup_A:
    {
        u16 saved = *((volatile u16 *)(spu + 0xA));
        *((volatile u16 *)(spu + 0xA)) = 0x50;
        {
            /* FAKE: redundant second handle to D_800F1AE0, mechanism: MEM_VOLATILE_P blocks combine.c's single-use symbol-address fold into the load (non-volatile handle measured folded: 159i vs target 160), lever-exhaustion: memory/grind/SioSyncroRead/hypotheses.md [s1-H2]; volatility copied from the granted decl (Ruling-4 grant, docs/grind/decisions.md 2026-08-25 10:34) */
            volatile u16 *p_ae0 = &regs_plus_0x8;
            *((volatile u16 *)(spu + 8)) = *p_ae0;
        }
        *((volatile u16 *)(spu + 0xE)) = regs_plus_0xE;
        *((volatile u16 *)(spu + 0xA)) |= 0x10;
        *((volatile u16 *)(spu + 0xA)) = saved;
        *((volatile u16 *)(spu + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x8000);
        {
            /* FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: s1's shared-label form measured 9/160 with s3<->s4 seat swap (solver model tmp/ra_solver_work/SioSyncroRead.model.json: p74 3refs/len102 vs p78 3refs/len56) */
            volatile s32 *p_b04a = &D_800F1B04;
            return r_arg1 - *p_b04a;
        }
    }

cleanup_B:
    {
        s32 base = D_800A3044;
        *((volatile u16 *)(base + 0xA)) &= 0xFFDF;
        DeliverEvent(0xF000000B, 0x100);
        {
            /* FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: as cleanup_A duplicate */
            volatile s32 *p_b04b = &D_800F1B04;
            return r_arg1 - *p_b04b;
        }
    }

main_work:
    {
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M1 (main.c:3044); s1's direct-global form measured 158i/24 (variant A) */
        volatile u16 *p_ae2 = &regs_plus_0xA;
        u32 mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    flag[2] = r_arg1;
    flag[1] = (s32)arg0;
    *flag = 0;
    *((volatile u16 *)(D_800A3044 + 0xA)) |= 0x20;

    count = 0;
    if (flag[2] == 0) goto final_cleanup;
    {
        volatile s32 *st = flag;

        do {
            spu = D_800A3044;
            if ((*((volatile u16 *)(spu + 4))) & 0x38) goto cleanup_A;
            if (!((*((volatile u16 *)(spu + 4))) & 2)) {
                do {
                    cb = CombWaitCallback;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(1, prev) == 0) goto cleanup_B;
                    }
                } while (!((*((volatile u16 *)(D_800A3044 + 4))) & 2));
            }

            *((u8 *)D_800F1B00) = *((u8 *)D_800A3044);
            st[1]++;
            count += 1;
            st[2]--;
            if (count == pkt_len) {
                count = 0;
                *((volatile u16 *)(D_800A3044 + 0xA)) ^= 2;
            }
        } while (st[2] != 0);
    }

final_cleanup:
    *((volatile u16 *)(D_800A3044 + 0xA)) &= 0xFFDF;

return_val:
    {
        /* FAKE: redundant second handle to D_800F1B04, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M4 (main.c:3104); s1's direct-global form measured 158i/24 (variant A) */
        volatile s32 *p_b04 = &D_800F1B04;
        return r_arg1 - *p_b04;
    }
}

/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_3  |  160i  |  x4 size collision */
/* PsyQ LIBCOMB comb: SioAnsyncWrite (static) — verbatim-linked Sony object.
   D_800F1AF0/AF4/AF8 are mutated at interrupt time by HandleSio — volatile
   is original semantics (operator-audited grant 2026-07-10). */
extern volatile s32 sen;
extern volatile s32 sen_plus_0x4;
extern volatile s32 sen_plus_0x8;
extern volatile s32 sen_plus_0xC;
extern s32 D_800A3044;
s32 SioAnsyncWrite(int a0, int a1) {
    volatile s32 *flag = &sen;
    if (*flag != 0) {
        return -1;
    }
    sen_plus_0x8 = a1;
    sen_plus_0x4 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        sen_plus_0xC = *(volatile u16 *)(reg + 4) & 0x80;
        *(volatile u16 *)(reg + 0xA) |= 0x400;
    }
    return 0;
}
s32 SioSyncroWrite(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &sen;
    s32 retries;
    s32 pkt_len;
    s32 i;
    s32 (*cb)(s32, s32);

    retries = 0;
    if (*flag != 0) return -1;
    {
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: memory/grind/SioSyncroWrite/hypotheses.md [s4-M1] (direct-global form measured 158i/mismatch) */
        volatile u16 *p_ae2 = &regs_plus_0xA;
        u32 mode;
        mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    sen_plus_0x8 = arg1;
    sen_plus_0x4 = (s32)arg0;
    i = 0;
    if (sen_plus_0x8 == 0) goto done;
    for (;;) {
        volatile s32 *st = flag;

        while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
            cb = CombWaitCallback;
            if (cb != 0) {
                s32 prev = retries;
                retries += 1;
                if (cb(2, prev) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    goto done;
                }
            }
        }
        if (i == 0) {
            sen_plus_0xC = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
        }
        *((u8 *)D_800A3044) = *((u8 *)sen_plus_0x4);
        st[1]++;
        i += 1;
        st[2]--;
        if (i == pkt_len) {
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == st[3]) {
                /* FAKE: redundant second handle to D_800F1AF8, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M2] (direct-global form measured 158i/mismatch) */
                volatile s32 *p_af8 = &sen_plus_0xC;
                do {
                    cb = CombWaitCallback;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(2, prev) == 0) {
                            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M3] (direct-global form measured 158i/mismatch) */
                            volatile s32 *p_af4b = &sen_plus_0x8;
                            DeliverEvent(0xF000000B, 0x100);
                            return (arg1 - *p_af4b) - 1;
                        }
                    }
                } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
            }
            i = 0;
        }
        {
            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-H1/H2] (plain-global and function-scope-pointer forms both measured negative) */
            volatile s32 *remaining = &sen_plus_0x8;
            if (*remaining == 0) break;
        }
    }

done:
    {
        /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M4] (direct-global form measured 158i/mismatch) */
        volatile s32 *p_af4 = &sen_plus_0x8;
        return arg1 - *p_af4;
    }
}
/* kengo:MED  |  am_rmd/SetPacketData  |  159i */
INCLUDE_ASM("asm/funcs", _comb_control);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel AddDrv\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x47\n"
    "    nop\n"
    "endlabel AddDrv\n"
    ".set reorder\n"
    ".set at\n"
);

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel DelDrv\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x48\n"
    "    nop\n"
    ".global g_data_start\n"
    ".type g_data_start, @function\n"
    "g_data_start:\n"
    "    .aent g_data_start\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    ".global g_module_func_tbl\n"
    ".type g_module_func_tbl, @function\n"
    "g_module_func_tbl:\n"
    "    .aent g_module_func_tbl\n"
    "    .word 0x8001DCB0\n"
    "    .word 0x8001E878\n"
    "    .word 0x80033898\n"
    "    .word 0x80034708\n"
    "    .word 0x800397D4\n"
    "    .word 0x8003993C\n"
    "    .word 0x8003B9D0\n"
    "    .word 0x8003BCB4\n"
    "    .word 0x80035480\n"
    "    .word 0x80035828\n"
    "    .word 0x8003BE10\n"
    "    .word 0x8003BEA8\n"
    "    .word 0x8001EA04\n"
    "    .word 0x8001EA84\n"
    "    .word 0x80035430\n"
    "    .word 0x8003BFC4\n"
    "    .word 0x8001EEB4\n"
    "    .word 0x8001EFA0\n"
    "    .word 0x8003C040\n"
    "    .word 0x8003C2C0\n"
    "    .word 0x8003C42C\n"
    "    .word 0x8003C560\n"
    "    .word 0x8003B870\n"
    "    .word 0x8003B8E4\n"
    "    .word 0x8003C958\n"
    "    .word 0x8003C9A4\n"
    "    .word 0x80035DC8\n"
    "    .word 0x80035E38\n"
    "    .word 0x8003CE18\n"
    "    .word 0x8003CF84\n"
    "    .word 0x8003C714\n"
    "    .word 0x8003C8B4\n"
    "    .word 0x8003CCCC\n"
    "    .word 0x8003CD10\n"
    ".global g_sqrt_table_u8\n"
    ".type g_sqrt_table_u8, @function\n"
    "g_sqrt_table_u8:\n"
    "    .aent g_sqrt_table_u8\n"
    "    .word 0x0D0B0800\n"
    "    .word 0x15131110\n"
    "endlabel DelDrv\n"
    ".set reorder\n"
    ".set at\n"
);

/* Rodata moved from asm/data/101C.rodata_main_post.s (rodata-cleanup project,
 * docs/rodata-cleanup-project.md, 2026-06-09). All 10 symbols are owned by
 * main.c functions (4 debug-printf strings + 4 switch jtbls + 2 SIO debug
 * strings); per the inventory CSV every owner has file=main. Placed at
 * end-of-file so the existing 24 bytes of main.o(.rodata) (from earlier
 * inline-asm rodata) stay at the original offsets (0..24 = 0x800163C0..
 * 0x800163D8) and the new declarations land at the retired block's slot
 * (0x800163D8..0x800164AC). Jtbl entries use literal addresses because the
 * referenced `.L<n>` labels live inside the asmfile-bridged stub function
 * bodies (saTan1MainJump, func_8008AF9C) and aren't visible to C source —
 * the linker produces identical bytes for either form. */
/* jtbl_80016420/jtbl_80016440 (the two SpuSetCommonAttr switch tables) are
 * now COMPILER-EMITTED by func_8008AF9C's switches (they emit into .rodata
 * at that function's file position, right after the four SPU debug strings
 * below, which moved up beside their owner functions for the same reason —
 * see the comment above D_800163D8). jtbl_80016460/jtbl_80016480 are likewise
 * compiler-emitted, by func_8008B488's two volume-mode switches (2026-09-28);
 * the zero word between them at 0x8001647C is the `.align 3` the compiler
 * puts before the second table, which is why main is not in
 * RODATA_ALIGN2_FILES. */
const char D_8001649C[12] = "SIO console";
const char D_800164A8[4] = "sio";
