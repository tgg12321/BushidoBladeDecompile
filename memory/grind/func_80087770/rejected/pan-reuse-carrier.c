/* REJECTED 2026-09-24 layer-2 FAIL: the single u8 `pan` reused across the
 * tone/program/voice pan stages is a multi-write carrier; Ruling 8 admits that
 * shape for vmNoiseOn only. Scored 0 but not landable. */
/* func_80087770: Sony LIBSND vmanager _SsVmSetSeqVol (SOTN SpuVmSetSeqVol,
 * ps2sdk libsnd2 vm_seq.c _SsVmSetSeqVol). Store a sequence's master volume
 * pair (clamped to 0x7F) in its score block, then recompute the shadow
 * volume registers of every voice that sequence owns: a shared base from the
 * VAB master volume and the voice's channel volume, scaled by the program and
 * tone volumes and by the sequence's per-side volume, panned by the tone,
 * program and voice pans, folded to mono if requested, then squared into
 * _svm_sreg_buf. The fourth argument is unused (callers pass 1). */
s16 func_80087770(s16 seq_sep_no, u16 voll, u16 volr, s16 arg3) {
    u8 *score;
    s16 voice;
    u32 voll_t;
    u32 volr_t;
    u16 left;
    u16 right;
    u8 pan;

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

            pan = _svm_tn[_svm_voice[voice].unk10 * 16 + _svm_voice[voice].tone].pan;
            if (pan < 0x40) {
                left = voll_t;
                right = volr_t * pan / 0x3F;
            } else {
                left = voll_t * (0x7F - pan) / 0x3F;
                right = volr_t;
            }
            pan = _svm_pg[_svm_voice[voice].prog].mpan;
            if (pan < 0x40) {
                right = right * pan / 0x3F;
            } else {
                left = left * (0x7F - pan) / 0x3F;
            }
            pan = _svm_voice[voice].unka;
            if (pan < 0x40) {
                right = right * pan / 0x3F;
            } else {
                left = left * (0x7F - pan) / 0x3F;
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
