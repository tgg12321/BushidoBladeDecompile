/* PsyQ 4.0 LIBSPU S_INI: _SpuInit and SpuStart. .text 0x800885CC..0x80088740, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "system.h"
#include "libspu_internal.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void _SpuInit(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    ResetCallback();
    _spu_init(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = &_spu_voice_centerNote[23];
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    SpuStart();
    _spu_rev_flag = 0;
    _spu_rev_reserve_wa = 0;
    _spu_rev_attr.mode = 0;
    _spu_rev_attr.depth.left = 0;
    _spu_rev_attr.depth.right = 0;
    _spu_rev_attr.delay = 0;
    _spu_rev_attr.feedback = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr[0];
    _spu_FsetRXX(0xD1, _spu_rev_startaddr[0], 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
}

void SpuStart(void) {
    s32 v0;
    if (_spu_isCalled == 0) {
        _spu_isCalled = 1;
        EnterCriticalSection();
        _SpuDataCallback((s32)_spu_FiDMA);
        v0 = OpenEvent((s32)0xF0000009, 0x20, 0x2000, 0);
        _spu_EVdma = v0;
        EnableEvent(v0);
        ExitCriticalSection();
    }
}
