#ifndef SOUND_H
#define SOUND_H

/* Sound/SPU subsystem - BGM, SE, SPU control */

#include "common.h"

/* Named globals */
extern s32 g_snd_bgm_id;
extern s32 g_snd_se_id;
extern s32 g_snd_volume;

/* Sony LIBSND `_svm_cur` (vmanager current-voice state; psyz
   libsnd_private.h `struct struct_svm`), base 0x801027F0. One object: the
   original binary addresses its fields off a single base (func_800861BC:
   `addiu $t1, $v1, -2` from &voiceOffset to &voice). Replaces the splat
   per-word scalars D_801027F1/F6/F7/FC and D_80102806/08/0A/0C/0E
   (per-word splat symbol -> aggregate merge family, owner ruling 2026-08-17).
   Field names follow psyz; `char` fields are u8 under -funsigned-char. */
struct struct_svm {
    u8 prog_tones; u8 vabId; u8 note; u8 fine; u8 volume; u8 pan;
    u8 prog; u8 field_7_fake_program; u8 field_8_unknown; u8 field_0x9;
    u8 mvol; u8 mpan; u8 tone; u8 tone_vol; u8 tone_pan;
    u8 tone_prior; u8 tone_center; u8 tone_shift; u8 tone_min;
    u8 tone_max; u8 tone_mode; u8 pad; short seq_sep_no; short tone_vag_idx;
    short voice; short voiceOffset; short field_0x1e;
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
   symbol -> aggregate merge family, owner ruling 2026-08-17). */
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
extern VabHdr *_svm_vh; /* _svm_vh: current VAB header */

/* Functions */
extern void SsSetSerialAttr(s32, s32, s32);

#endif /* SOUND_H */
