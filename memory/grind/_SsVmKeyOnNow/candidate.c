/* Sony LIBSND `_SsVmKeyOnNow` (VM_NOWON): compute the current voice's L/R
   volume (VAB master x volume x program/tone volume, score channel volume,
   three pan stages, optional mono fold, square-law curve), queue the SPU
   register shadow + pitch, and set the key-on/reverb bits. Shape follows the
   SOTN psxsdk SpuVmKeyOnNow (sotn-decomp src/main/psxsdk/libsnd/vmanager.c)
   and the ps2sdk libsnd2 vm_nowon.c port; BB2's build squares the volumes
   only for sequence voices (seq_sep_no != 0x21) and has no unk1b store.
   Symbol map: D_80102A78 <- _svm_sreg_buf (s16 view); D_800F65E0 <-
   _svm_sreg_dirty; D_800F4E1C <- _svm_voice[].unk04 (BB2 stride 54);
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
    D_80102A78[pos + 2] = pitch;
    D_80102A78[pos + 0] = voll;
    D_80102A78[pos + 1] = volr;
    D_800F65E0[_svm_cur.voice] |= 7;
    *(s16 *)((u8 *)&D_800F4E1C + _svm_cur.voice * 54) = pitch;
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
    D_800F1B10 |= bitsLower;
    D_800F1B12 |= bitsUpper;
    D_801078D8 &= ~D_800F1B10;
    D_801078DA &= ~D_800F1B12;
}
