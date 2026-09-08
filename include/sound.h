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

/* Functions */
extern void SsSetSerialAttr(s32, s32, s32);

#endif /* SOUND_H */
