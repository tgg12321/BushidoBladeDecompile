/* LIBSND code between verbatim modules: func_800858D0. .text 0x800858D0..0x800859F0: an
 * unidentified region between verbatim LIBSCAN modules (docs/naming/libscan/matches.json;
 * memory/closer/libsnd-hunt-report.md lists the probable newer-build modules), one file per gap
 * (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "sound.h"

extern u8 _SsVmMaxVoice;

/* func_800858D0: reset the per-voice state record (_svm_voice) of every
 * voice up to _SsVmMaxVoice and key each one off (func_8008B488 with a
 * one-voice mask, then _SsVmKeyOffNow). */
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
