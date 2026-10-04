/* PsyQ LIBSND UT_KEYV: SsUtKeyOnV and SsUtKeyOffV. .text 0x80085A40..0x80085E4C, the whole region
 * between UT_GVBA and UT_RDEP. Module start (owner ruling Q109), libscan near tier: UT_KEYV of the
 * PsyQ 4.0 LIBSND.LIB matches 255/259 words at 0x80085A40, unique (the 4 differing words are the
 * _svm_voice stride edit), XDEFs SsUtKeyOnV +0x0 and SsUtKeyOffV +0x394
 * (docs/naming/libscan/near_manifest.csv). */
#include "common.h"
#include "libsnd_i.h"

/* PsyQ LIBSND UT_KEYV. BB2 keeps the 4.0 routine's source shape but uses the
   later 54-byte voice-state stride (rather than 4.0's 52-byte layout). */
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
   splat merged into SsUtKeyOnV (docs/naming/libscan/
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
