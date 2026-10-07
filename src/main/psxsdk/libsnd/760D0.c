/* LIBSND code between TEMPO and UT_GVBA: func_800858D0. .text
 * 0x800858D0..0x800859F0. By link order and size probably LIBSND UT_AKO
 * (SsUtAllKeyOff), but no libscan evidence identifies it, so it stays a gap
 * file named by its ROM offset (Q106 D3, Q109). */
#include "common.h"
#include "libsnd_i.h"

/* Reset the per-voice state record (_svm_voice) of every
 * voice up to _SsVmMaxVoice and key each one off (func_8008B488 with a
 * one-voice mask, then _SsVmKeyOffNow). */
void func_800858D0(void) {
    SpuVoiceAttr attr;
    s16 var_s0;

    attr.mask = 0x60093;
    var_s0 = 0;
    attr.pitch = 0x1000;
    attr.addr = 0x1000;
    attr.adsr1 = 0x80FF;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.adsr2 = 0x4000;
    if (_SsVmMaxVoice != 0) {
        do {
            _svm_voice[var_s0].unk2 = 0x18;
            _svm_voice[var_s0].unk6 = 0;
            _svm_voice[var_s0].unke = 0xFF;
            _svm_voice[var_s0].unk10 = 0;
            _svm_voice[var_s0].prog = 0;
            _svm_voice[var_s0].tone = 0xFF;
            attr.voice = 1 << var_s0;
            func_8008B488(&attr);
            _svm_cur.voice = var_s0;
            _SsVmKeyOffNow(1);
            var_s0 = var_s0 + 1;
        } while (var_s0 < _SsVmMaxVoice);
    }
}
