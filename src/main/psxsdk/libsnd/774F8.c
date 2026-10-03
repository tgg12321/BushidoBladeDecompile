/* LIBSND code between verbatim modules: vmNoiseOn, _SsVmKeyOffNow, _SsVmKeyOnNow, func_80087770,
 * _SsVmGetSeqVol, func_80087D10, func_80087D58 and _SsVmSeqKeyOff. .text 0x80086CF8..0x80087E3C: an
 * unidentified region between verbatim LIBSCAN modules (docs/naming/libscan/matches.json;
 * memory/closer/libsnd-hunt-report.md lists the probable newer-build modules), one file per gap
 * (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "sound.h"
#include <psxsdk/libspu.h>

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s16 _svm_stereo_mono;
extern u8 _SsVmMaxVoice;
typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag;
    s16 reserved[4];
} VagAtr;
extern ProgAtr *_svm_pg;
extern VagAtr *_svm_tn;
extern s16 _svm_sreg_buf[];
extern u8 _svm_sreg_dirty[];
extern u16 _svm_okon1;
extern u16 _svm_okof1;
extern u16 D_800F1B14;              /* psyz _svm_orev1 */
extern u16 D_800F2B68;              /* psyz _svm_orev2 */

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
    struct SeqStruct *score;
    s16 voice;
    s16 bitsLower;
    s16 bitsUpper;
    u32 voll_t, volr_t;
    u32 voll, volr;
    /* SOTN-verbatim (sotn-decomp src/main/psxsdk/libsnd/vmanager.c
       vmNoiseOn): temp holds the tone pan, then the program pan, then the
       voice pan, one per pan stage below. Owner Ruling 8
       (ordinary-c-judge-decidable.md), vmNoiseOn only. */
    u32 temp;
    u32 idx;

    score = &_ss_score[_svm_cur.seq_sep_no & 0xFF]
                       [(_svm_cur.seq_sep_no & 0xFF00) >> 8];

    voll_t = score->unk58 * 0x81;
    volr_t = score->unk5A * 0x81;

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
       after the jal: no dependence ties it to the call). Using vc at each use
       instead puts the index in $a0 and drops $s3 from the frame; idx also at
       the two _svm_voice[] uses differs too (the target zero-extends vc again
       there). */
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
    struct SeqStruct *score;
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
    score = &_ss_score[_svm_cur.seq_sep_no & 0xFF]
                       [(_svm_cur.seq_sep_no & 0xFF00) >> 8];
    if (_svm_cur.seq_sep_no != 0x21) {
        voll_t = (voll_t * score->unk58) / 127;
        volr_t = (volr_t * score->unk5A) / 127;
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
    struct SeqStruct *score;
    s16 voice;
    u32 voll_t;
    u32 volr_t;
    u16 left;
    u16 right;
    u8 tone_pan;
    u8 prog_pan;
    u8 voice_pan;

    score = &_ss_score[seq_sep_no & 0xFF][(seq_sep_no & 0xFF00) >> 8];
    score->unk58 = voll;
    score->unk5A = volr;
    if (score->unk58 >= 0x7F) {
        score->unk58 = 0x7F;
    }
    if (score->unk5A >= 0x7F) {
        score->unk5A = 0x7F;
    }

    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        if (_svm_voice[voice].unke == seq_sep_no) {
            _SsVmVSetUp(_svm_voice[voice].vabId, _svm_voice[voice].unk10);
            voll_t = _svm_vh->mvol * 0x3FFF *
                     (_svm_voice[voice].unk8 *
                      score->vol[_svm_voice[voice].unkc] / 0x7F) /
                     0x3F01;
            volr_t = voll_t * _svm_pg[_svm_voice[voice].prog].mvol *
                     _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].vol /
                     0x3F01;
            voll_t = voll_t * _svm_pg[_svm_voice[voice].prog].mvol *
                     _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].vol /
                     0x3F01;
            voll_t = voll_t * score->unk58 / 0x7F;
            volr_t = volr_t * score->unk5A / 0x7F;

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
    struct SeqStruct *score = &_ss_score[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    _svm_cur.seq_sep_no = a0;
    *a1 = score->unk58;
    *a2 = score->unk5A;
    return _svm_cur.seq_sep_no;
}

s16 func_80087D10(s32 a0) {
    struct SeqStruct *score = &_ss_score[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    _svm_cur.seq_sep_no = a0;
    return score->unk58;
}

s16 func_80087D58(s32 a0) {
    struct SeqStruct *score = &_ss_score[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    _svm_cur.seq_sep_no = a0;
    return score->unk5A;
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
