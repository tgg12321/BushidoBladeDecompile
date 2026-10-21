/* PsyQ 4.0 LIBSPU S_INI: _SpuInit and SpuStart. .text 0x800885CC..0x80088740, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libetc.h>
#include "libspu_internal.h"
#include <psxsdk/libapi.h>

/* S_INI .data (0x800A2868..0x800A2CDC, the module's 1140-byte .data in
 * LIBSPU.LIB, XDEFs from _spu_EVdma at +8, no relocation). The first 8 bytes
 * are read by no code; the library object holds other values there
 * (0x15117350, 0x0040B49C). */
u32 D_800A2868[2] = {0x17117350, 0x30404D7C};
s32 _spu_EVdma = 0;
s32 _spu_keystat = 0;
s32 _spu_trans_mode = 0;
s32 _spu_rev_flag = 0;
s32 _spu_rev_reserve_wa = 0;
s32 _spu_rev_offsetaddr = 0;
SpuRevAttr _spu_rev_attr = {0};
volatile s32 _spu_RQvoice = 0;
volatile s32 _spu_RQmask = 0;
/* each voice's sample center note (SpuVoiceAttr.sample_note, 7BC88.c); 0xC000
 * at start and after _SpuInit(0) */
u16 _spu_voice_centerNote[24] = {
    0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000,
    0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000,
    0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000};
/* 1 KB of zeroes, DMA'd to SPU RAM to clear the reverb work area */
s32 _spu_zerobuf[256] = {0};
volatile s32 _spu_env = 0;
s32 _spu_isCalled = 0;

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
