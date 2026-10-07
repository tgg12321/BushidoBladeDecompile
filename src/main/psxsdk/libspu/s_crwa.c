/* PsyQ 4.0 LIBSPU S_CRWA: SpuClearReverbWorkArea. .text 0x8008A928..0x8008AAC4,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3.
 */
#include "common.h"
#include "libspu_internal.h"

/* Declarations from the file this module was split from
 * (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_zerobuf;

/* PsyQ 4.0 LIBSPU s_crwa: SpuClearReverbWorkArea — verbatim-linked Sony
   object; C ref: sotn-decomp
   src/main/psxsdk/libspu/s_crwa.c */
s32 SpuClearReverbWorkArea(u32 rev_mode) {
    /* FAKE: volatile local admitted on SOTN precedent (owner rulings Q50
       route A, Q53) -- the saved transfer callback is stored to / reloaded
       from its $sp slot around the WaitEvent loop, as in the target; the
       plain-local spelling keeps it in a register (score 36). SOTN holds it
       in a `volatile s32`; typed here as the callback it stores. */
    /* SOTN: src/main/psxsdk/libspu/s_crwa.c:10 @db41b28 */
    void (* volatile callback)();
    s32 oldTransmode;
    s32 var_s2;
    s32 var_s3;
    s32 transmodeCleared;
    u32 var_s0;
    u32 var_s1;

    callback = 0;
    transmodeCleared = 0;
    if (rev_mode >= 10 || _SpuIsInAllocateArea_(_spu_rev_startaddr[rev_mode])) {
        return -1;
    }
    if (rev_mode == 0) {
        var_s1 = 0x10 << _spu_mem_mode_plus;
        var_s2 = 0xFFF0 << _spu_mem_mode_plus;
    } else {
        var_s1 = (0x10000 - _spu_rev_startaddr[rev_mode]) << _spu_mem_mode_plus;
        var_s2 = _spu_rev_startaddr[rev_mode] << _spu_mem_mode_plus;
    }
    oldTransmode = _spu_transMode;
    if (_spu_transMode == 1) {
        _spu_transMode = 0;
        transmodeCleared = 1;
    }
    var_s3 = 1;
    if (_spu_transferCallback != 0) {
        callback = _spu_transferCallback;
        _spu_transferCallback = 0;
    }
    while (var_s3 != 0) {
        var_s0 = var_s1;
        if (var_s1 > 0x400) {
            var_s0 = 0x400;
        } else {
            var_s3 = 0;
        }

        _spu_t(2, var_s2);
        _spu_t(1);
        _spu_t(3, &_spu_zerobuf, var_s0);
        WaitEvent(_spu_EVdma);
        var_s1 -= 0x400;
        var_s2 += 0x400;
    }
    if (transmodeCleared != 0) {
        _spu_transMode = oldTransmode;
    }
    if (callback != 0) {
        _spu_transferCallback = callback;
    }
    return 0;
}
