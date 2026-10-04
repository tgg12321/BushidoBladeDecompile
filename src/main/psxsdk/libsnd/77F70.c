/* LIBSND code between VM_NOWON and VM_VSU: func_80087770, _SsVmGetSeqVol, func_80087D10, func_80087D58
 * and _SsVmSeqKeyOff. .text 0x80087770..0x80087E3C. By layout this is LIBSND VM_SEQ (PsyQ 4.0
 * LIBSND.LIB XDEFs _SsVmSetSeqVol +0x0, _SsVmGetSeqVol +0x538, _SsVmGetSeqLVol +0x59C, _SsVmGetSeqRVol
 * +0x5E4, _SsVmSeqKeyOff +0x62C; here +0x53C, +0x5A0, +0x5E8, +0x630), but its first function
 * func_80087770 is not identified at the libscan xref or near tier, so the region stays one gap file
 * (owner ruling Q109), named by its ROM offset. */
#include "common.h"
#include "libsnd_i.h"

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
