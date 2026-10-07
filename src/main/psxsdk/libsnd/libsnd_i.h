#ifndef LIBSND_I_H
#define LIBSND_I_H

/* PsyQ LIBSND library-internal state and helpers shared by the modules in this
 * directory (SOTN src/main/psxsdk/libsnd/libsnd_i.h). */

#include <psxsdk/libspu.h>
#include <psxsdk/libsnd.h>

/* Sony LIBSND `_svm_cur` (vmanager current-voice state; psyz
   libsnd_private.h `struct struct_svm`), base 0x801027F0. One object: the
   original binary addresses its fields off a single base (func_800861BC:
   `addiu $t1, $v1, -2` from &voiceOffset to &voice). Replaces the splat
   per-word scalars D_801027F1/F6/F7/FC and D_80102806/08/0A/0C/0E
   (per-word splat symbol -> aggregate merge family, owner ruling).
   Field names follow psyz; `char` fields are u8 under -funsigned-char. */
struct struct_svm {
    u8 prog_tones;
    u8 vabId;
    u8 note;
    u8 fine;
    u8 volume;
    u8 pan;
    u8 prog;
    u8 field_7_fake_program;
    u8 field_8_unknown;
    u8 field_0x9;
    u8 mvol;
    u8 mpan;
    u8 tone;
    u8 tone_vol;
    u8 tone_pan;
    u8 tone_prior;
    u8 tone_center;
    u8 tone_shift;
    u8 tone_min;
    u8 tone_max;
    u8 tone_mode;
    u8 pad;
    short seq_sep_no;
    short tone_vag_idx;
    short voice;
    short voiceOffset;
    short field_0x1e;
};
extern struct struct_svm _svm_cur; /* _svm_cur */

/* Sony LIBSND `_svm_voice` (vmanager per-voice state; psyz libsnd_private.h
   `struct SpuVoice`), base 0x800F4E18, one record per SPU voice (24). BB2's
   record is 0x36 bytes: psyz's 0x34-byte layout with one extra halfword at
   +0x0C (unkc), so every psyz field from `note` on sits 2 bytes later. The
   original binary indexes it as a record table: stride-54 addressing off the
   one base in _SsVmInit/_SsVmKeyOffNow/SsUtKeyOnV/_SsVmSeqKeyOff and the
   asm-only _SsVmFlush. Field names follow psyz. Replaces the splat per-word
   scalars _svm_voice_plus_0x2..0x1D and D_800F4E20..4A (per-word splat
   symbol -> aggregate merge family, owner ruling). */
struct SpuVoice {
    s16 unk0;      /* 0x00 */
    s16 unk2;      /* 0x02 */
    s16 unk04;     /* 0x04 */
    u16 unk6;      /* 0x06 */
    s16 unk8;      /* 0x08 */
    u8 unka;       /* 0x0A */
    u8 unkb;       /* 0x0B */
    s16 unkc;      /* 0x0C: BB2-only */
    s16 note;      /* 0x0E */
    s16 unke;      /* 0x10 */
    s16 unk10;     /* 0x12 */
    s16 prog;      /* 0x14 */
    s16 tone;      /* 0x16 */
    s16 vabId;     /* 0x18 */
    s16 unk18;     /* 0x1A */
    u8 pad4[1];    /* 0x1C */
    u8 unk1b;      /* 0x1D */
    s16 auto_vol;  /* 0x1E */
    s16 unk1e;     /* 0x20 */
    s16 unk20;     /* 0x22 */
    s16 unk22;     /* 0x24 */
    s16 start_vol; /* 0x26 */
    s16 end_vol;   /* 0x28 */
    s16 auto_pan;  /* 0x2A */
    s16 unk2a;     /* 0x2C */
    s16 unk2c;     /* 0x2E */
    s16 unk2e;     /* 0x30 */
    s16 start_pan; /* 0x32 */
    s16 end_pan;   /* 0x34 */
};
extern struct SpuVoice _svm_voice[24]; /* _svm_voice */

extern VabHdr *_svm_vh; /* _svm_vh: current VAB header */

/* Sony LIBSND `_ss_score` (per-SEP score table), declared as SOTN declares
   it: `struct SeqStruct *_ss_score[32]` (sotn-decomp
   src/main/psxsdk/libsnd/libsnd_i.h:176 @aa53500). Base 0x80106F28, 32
   words, up to _SsMarkCallback at 0x80106FA8. Each entry points at one
   SEP's array of per-sequence score blocks: `_ss_score[sep][seq]`.
   The RECORD LAYOUT is BB2's own, not SOTN's. BB2 links a different LIBSND
   build (the interim 4.0-lineage build of memory/closer/libsnd-hunt-report.md)
   whose score block is 0xB0 bytes (SOTN's is 0xAC) and orders its fields
   differently. Every offset below is the one BB2's code uses. The comment
   names the SOTN member that does the same job in the same Sony function
   (SOTN stop.c, seqread.c, next.c, cres.c, tempo.c, vmanager.c @aa53500).
   Members no BB2 code touches are unkNN pads. Replaces the raw
   `extern s32 _ss_score` + byte-offset casts. */
struct SeqStruct {
    u8 *read_pos;     /* 0x00: SOTN read_pos */
    u8 *next_sep_pos; /* 0x04: SOTN next_sep_pos */
    u8 *loop_pos;     /* 0x08: SOTN loop_pos */
    /* 0x0C: BB2-only; used instead of next_sep_pos under flag 0x400 */
    u8 *unk0C;
    /* 0x10: BB2-only; compared with read_pos under flags 0x401 */
    u8 *unk10;
    /* 0x14: SOTN unk2b (1 = playing: set by replay, cleared by pause) */
    u8 unk14;
    u8 unk15;        /* 0x15: SOTN unk10 */
    u8 unk16;        /* 0x16: SOTN unk11 (MIDI running status) */
    u8 channel;      /* 0x17: SOTN channel */
    u8 unk18;        /* 0x18: SOTN unk13 */
    u8 unk19;        /* 0x19: SOTN unk14 */
    u8 unk1A;        /* 0x1A: SOTN unk15 */
    u8 unk1B;        /* 0x1B: SOTN unk16 */
    u8 unk1C;        /* 0x1C: SOTN unk27 */
    u8 unk1D;        /* 0x1D: SOTN unk28 */
    u8 unk1E;        /* 0x1E: SOTN unk29 */
    u8 unk1F;        /* 0x1F: SOTN unk2a */
    u8 unk20;        /* 0x20: SOTN unk46 (loop count) */
    u8 unk21;        /* 0x21: SOTN unk48 (loops played) */
    u8 unk22;        /* 0x22: SOTN unk3C (next SEP) */
    u8 unk23;        /* 0x23: SOTN unk0 (next SEQ) */
    u8 unk24[3];     /* 0x24: not accessed by BB2 */
    u8 panpot[16];   /* 0x27: SOTN panpot */
    u8 programs[16]; /* 0x37: SOTN programs */
    s16 unk48;       /* 0x48: SOTN unk3E */
    s16 unk4A;       /* 0x4A: SOTN unk40 */
    s16 unk4C;       /* 0x4C: SOTN unk42 */
    s16 unk4E;       /* 0x4E: SOTN unk44 */
    s16 unk50;       /* 0x50: SOTN unk4a */
    s16 unk52;       /* 0x52: SOTN unk6E */
    s16 unk54;       /* 0x54: SOTN unk70 */
    s16 unk56;       /* 0x56: SOTN unk72 */
    u16 unk58;       /* 0x58: SOTN unk74 (sequence L volume) */
    u16 unk5A;       /* 0x5A: SOTN unk76 (sequence R volume) */
    s16 unk5C;       /* 0x5C: SOTN unk78 */
    s16 unk5E;       /* 0x5E: SOTN unk7A */
    s16 vol[16];     /* 0x60: SOTN vol */
    s32 unk80;       /* 0x80: not accessed by BB2 */
    s32 unk84;       /* 0x84: SOTN unk7c */
    s32 unk88;       /* 0x88: SOTN unk80 */
    s32 unk8C;       /* 0x8C: SOTN unk84 */
    s32 delta_value; /* 0x90: SOTN delta_value */
    u32 unk94;       /* 0x94: SOTN unk8c (tempo) */
    s32 unk98;       /* 0x98: SOTN unk90 (play-state flags) */
    s32 unk9C;       /* 0x9C: SOTN unk94 */
    s32 unkA0;       /* 0xA0: SOTN unk98 */
    s32 unkA4;       /* 0xA4: not accessed by BB2 */
    s32 unkA8;       /* 0xA8: SOTN unkA0 */
    u32 unkAC;       /* 0xAC: SOTN unkA4 (target tempo) */
};
extern struct SeqStruct *_ss_score[32]; /* _ss_score */

/* Sony LIBSND `_SsFCALL`, verbatim from the PsyQ 4.0 LIBSND.H: the
   sequencer's MIDI-event dispatch table. In BB2 it is the object at
   0x800F3340 (148 bytes; the next object starts at 0x800F33D8). The 4.0
   SSINIT object defines the 148-byte common `SsFCALL`, and the 4.0 MIDIREAD
   _SsGetSeqData, which lines up with func_80084CC0 word for word except one
   hoisted store, reaches it through SsFCALL+0/+4/+8/+0xC/+0x10 relocations
   (docs/naming/sweep-2026-09-29/held.csv). That sweep holds the NAME at
   MEDIUM, so the object keeps its splat name D_800F3340. It is one object,
   not five scalars. func_80084CC0's target code keeps each handler load
   behind the score-block store before it, and GCC 2.7.2's scheduler only
   draws that dependence when the handler load is an in-struct access too.
   Replaces the per-word splat scalars D_800F3340/44/48/4C/50. */
typedef struct {
    void (*noteon)();
    void (*programchange)();
    void (*pitchbend)();
    void (*metaevent)();
    void (*control[13])();
    void (*ccentry[20])();
} _SsFCALL;

extern _SsFCALL D_800F3340; /* SsFCALL */

/* Sony LIBSND `_snd_seq_tick_env` (ssstart.c SndSeqTickEnv), base
   0x800A26CC: tick mode / tick-mode flag / tick handler / saved interrupt
   callback / VSync-hooked flag / 1-per-2 flag / interrupt slot / pad, then
   the 1-per-2 toggle at +0x14 (asm/data/91C98.data.s initializer: 0x3C, 1,
   SsSeqCalledTbyT, 0, 0, 0, 0x7F, 0, 0). Layout per SOTN
   src/main/psxsdk/libsnd/libsnd_i.h:284. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unk12;
    /* 0x10 */ u8 unk16;
    /* 0x11 */ u8 unk17;
    /* 0x12 */ u8 unk18;
    /* 0x13 */ u8 unk19;
    /* 0x14 */ u32 unk20;
} SndSeqTickEnv;

extern SndSeqTickEnv _snd_seq_tick_env;

/* Voice-manager and sequencer state (one declaration per object; splat names
 * kept). */
extern u8 _SsVmMaxVoice;
extern s16 kMaxPrograms;
extern s32 VBLANK_MINUS;
extern s32 _snd_ev_flag;
extern s32 _snd_openflag;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;
extern s16 _svm_damper;
extern s16 _svm_stereo_mono;
extern u8 _svm_auto_kof_mode;
extern s16 _svm_sreg_buf[];
extern u8 _svm_sreg_dirty[];
extern s32 _svm_envx_hist[];
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 D_800F1B14; /* psyz _svm_orev1 */
extern u16 D_800F2B68; /* psyz _svm_orev2 */
extern u16 _svm_vab_count;
extern u8 _svm_vab_used[];
extern s32 _svm_vab_start[];
extern s32 _svm_vab_total[];
extern VabHdr *_svm_vab_vh[];
extern ProgAtr *_svm_vab_pg[];
extern VagAtr *_svm_vab_tn[];
/* the voice manager's reverb attribute block */
extern SpuReverbAttr _svm_rattr;

extern void _SsInit(void);
extern void _SsVmInit(s32);
extern void _SsVmFlush(void);
extern void _SsVmKeyOnNow(s32, u16);
extern void _SsVmKeyOffNow(s32);
extern void _SsVmSeqKeyOff(s16);
extern void _SsVmDoAllocate(void);
extern s32 _SsVmVSetUp(s32, s32);
extern void _SsVmDamperOff(void);
extern s16 _SsVmGetSeqVol(s32, s16 *, s16 *);
extern s16 func_80087770(s16, u16, u16, s16);
extern void vmNoiseOn(u8);
extern s32 note2pitch2(u16, u16);
extern s32 _SsReadDeltaValue(s16, s16);
extern void _SsSeqPlay(s16, s16);
extern s32 func_80084CC0(s16, s16);
extern void _SsSndNextSep(s16, s16);
extern void _SsSndPlay(s16, s16);
extern void _SsSndCrescendo(s16, s16);
extern void _SsSndDecrescendo(s16, s16);
extern void _SsSndTempo(s16, s16);
extern void _SsSndPause(s16, s16);
extern void _SsSndReplay(s16, s16);
extern void _SsSndStop(s16, s16);

#endif /* LIBSND_I_H */
