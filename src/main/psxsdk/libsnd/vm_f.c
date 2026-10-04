/* PsyQ LIBSND VM_F: _SsVmFlush. .text 0x800863DC..0x80086818. Not a verbatim LIBSCAN span: BB2 links
 * an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived release holds
 * (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan xref tier: the
 * verbatim SSCALL module's REL26 at +0x44 names _SsVmFlush -> EXE jal 0x800863DC
 * (docs/naming/libscan/near_manifest.csv), VM_F's only .text XDEF (+0x0, PsyQ 4.0 LIBSND.LIB). */
#include "common.h"
#include "libsnd_i.h"

extern s32 D_80103604;              /* psyz _svm_envx_ptr */
extern void (*D_80102BF8)(s32);     /* psyz _autovol */
extern void (*D_801027E8)(s32);     /* psyz _autopan */

/* Sony LIBSND `_SsVmFlush` (VM_F): sample every voice's envelope into
   _svm_voice[].unk6 and the 16-slot silence history ring, release the
   noise state of voices silent in history slots 0-14, run the auto-volume / auto-pan
   callbacks, flush the dirty shadow registers through SpuSetVoiceAttr
   (func_8008B488), then write key-off / key-on / reverb masks and clear the
   pending key masks. Shape follows sotn-decomp
   src/main/psxsdk/libsnd/vmanager.c SpuVmFlush (US main build, matched);
   BB2's build reads the envelope with SpuGetVoiceEnvelope and writes the SPU
   through LIBSPU calls where SOTN pokes the registers directly. */
void _SsVmFlush(void)
{
    s32 i;
    u32 env_mask;
    SpuVoiceAttr attr;

    D_80103604 = (D_80103604 + 1) & 0xF;
    _svm_envx_hist[D_80103604] = 0;

    for (i = 0; i < _SsVmMaxVoice; i++) {
        SpuGetVoiceEnvelope(i, &_svm_voice[i].unk6);
        if (_svm_voice[i].unk6 == 0) {
            _svm_envx_hist[D_80103604] |= 1 << i;
        }
    }
    if (_svm_auto_kof_mode == 0) {
        env_mask = 0xFFFFFFFF;
        for (i = 0; i < 0xF; i++) {
            env_mask &= _svm_envx_hist[i];
        }
        for (i = 0; i < _SsVmMaxVoice; i++) {
            if (env_mask & (1 << i)) {
                if (_svm_voice[i].unk1b == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                _svm_voice[i].unk1b = 0;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
    for (i = 0; i < 24; i++) {
        if (_svm_voice[i].auto_vol != 0) {
            D_80102BF8(i);
        }
        if (_svm_voice[i].auto_pan != 0) {
            D_801027E8(i);
        }
    }

    for (i = 0; i < 24; i++) {
        attr.mask = 0;
        attr.voice = 1 << i;
        if (_svm_sreg_dirty[i] & 1) {
            attr.mask = 3;
            attr.volume.left = _svm_sreg_buf[i * 8 + 0];
            attr.volume.right = _svm_sreg_buf[i * 8 + 1];
        }
        if (_svm_sreg_dirty[i] & 4) {
            attr.mask |= 0x10;
            attr.pitch = _svm_sreg_buf[i * 8 + 2];
        }
        if (_svm_sreg_dirty[i] & 8) {
            attr.mask |= 0x80;
            attr.addr = (u16)_svm_sreg_buf[i * 8 + 3] << 3;
        }
        if (_svm_sreg_dirty[i] & 0x10) {
            attr.mask |= 0x60000;
            attr.adsr1 = _svm_sreg_buf[i * 8 + 4];
            attr.adsr2 = _svm_sreg_buf[i * 8 + 5];
        }
        if (attr.mask != 0) {
            func_8008B488(&attr);
        }
        _svm_sreg_dirty[i] = 0;
    }

    SpuSetKey(0, ((_svm_okof2 & 0xFF) << 16) | _svm_okof1);
    SpuSetKey(1, ((_svm_okon2 & 0xFF) << 16) | _svm_okon1);
    SpuSetReverbVoice(8, ((D_800F2B68 & 0xFF) << 16) | D_800F1B14);

    _svm_okof1 = 0;
    _svm_okof2 = 0;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
}
