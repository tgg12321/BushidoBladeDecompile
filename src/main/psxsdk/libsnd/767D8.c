/* LIBSND code between verbatim modules: func_80085FD8, SsUtSetDetVVol, func_80086080, func_80086130
 * and _SsVmDoAllocate. .text 0x80085FD8..0x800863CC: an unidentified region between verbatim
 * LIBSCAN modules (docs/naming/libscan/matches.json; memory/closer/libsnd-hunt-report.md lists the
 * probable newer-build modules), one file per gap (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "libsnd_i.h"

s32 func_80085FD8(s16 a0) {
    if ((u16)a0 < 0x18) {
        SpuGetVoiceVolume(a0);
        return 0;
    }
    return -1;
}

s32 SsUtSetDetVVol(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        _svm_sreg_buf[idx * 8 + 1] = y;
        _svm_sreg_buf[idx * 8] = x;
        _svm_sreg_dirty[idx] |= 3;
        return 0;
    }
    return -1;
}

s32 func_80086080(s16 a0, s16 *a1, s16 *a2) {
    u16 raw1, raw2;

    if ((u16)a0 < 0x18) {
        SpuGetVoiceVolume(a0, &raw1, &raw2);
        *a1 = (s16)raw1 / 129;
        *a2 = (s16)raw2 / 129;
        return 0;
    }
    return -1;
}
s32 func_80086130(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        s16 vx = x * 129;
        s16 vy = y * 129;

        _svm_sreg_buf[idx * 8 + 1] = vy;
        _svm_sreg_buf[idx * 8] = vx;
        _svm_sreg_dirty[idx] |= 3;
        return 0;
    }
    return -1;
}
/* Sony LIBSND `_SsVmDoAllocate` (psyz vm_aloc2.c analog): set up the
   allocated voice's SPU shadow registers (start address, ADSR) and mark the
   voice's dirty bits. BB2 deltas vs psyz: _svm_voice stride 54, ADSR indexed
   by voiceOffset through the flat s16 shadow view D_80102A78[]. */
static inline void vmSetStartAddr(u16 addr) {
    _svm_sreg_buf[_svm_cur.voiceOffset + 3] = addr;
    _svm_sreg_dirty[_svm_cur.voice] |= 8;
}

void _SsVmDoAllocate(void) {
    int i;
    int progIdx;

    _svm_cur.voiceOffset = _svm_cur.voice * 8;
    _svm_cur.field_0x1e = _svm_cur.field_7_fake_program * 16 + _svm_cur.tone;
    _svm_voice[_svm_cur.voice].unk6 = 0x7FFF;
    for (i = 0; i < 16; i++) {
        _svm_envx_hist[i] &= ~(1 << _svm_cur.voice);
    }
    if ((_svm_cur.tone_vag_idx & 1) > 0) {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved2);
    } else {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved3);
    }
    _svm_sreg_buf[_svm_cur.voiceOffset + 4] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr1;
    _svm_sreg_buf[_svm_cur.voiceOffset + 5] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr2 + _svm_damper;
    _svm_sreg_dirty[_svm_cur.voice] |= 0x30;
}
