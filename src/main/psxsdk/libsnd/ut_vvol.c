/* PsyQ LIBSND UT_VVOL: SsUtGetDetVVol, SsUtSetDetVVol, SsUtGetVVol and
 * func_80086130 (at SsUtSetVVol's place in the module). .text 0x80085FD8..0x800861BC. Not a verbatim
 * LIBSCAN span: BB2 links an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived release
 * holds (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan near tier:
 * the SsUtGetDetVVol sub-function of UT_VVOL (PsyQ 4.1-4.4) places 0/14 at 0x80085FD8, XDEF +0x0
 * (docs/naming/libscan/near_manifest.csv, CONFIRM; the SsUtSetDetVVol and SsUtGetVVol rows there are
 * near too). */
#include "common.h"
#include "libsnd_i.h"

s32 SsUtGetDetVVol(s16 voice, s16 *voll, s16 *volr) {
    if ((u16)voice < 0x18) {
        SpuGetVoiceVolume(voice, voll, volr);
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

s32 SsUtGetVVol(s16 a0, s16 *a1, s16 *a2) {
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
