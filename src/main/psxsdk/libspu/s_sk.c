/* PsyQ 4.0 LIBSPU S_SK: SpuSetKey. .text 0x8008AAD4..0x8008ACD0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_sk: SpuSetKey — verbatim-linked Sony object;
 * C ref: sotn-decomp src/psxsdk/libspu/s_sk.c shape + PsyQ 4.0
 * S_SK object relocs (_spu_RQ = one u16[4]). Volatile decls are Ruling-4
 * ground-truth-codegen grants (volatile_extern_allowlist.txt:40-44). */

void SpuSetKey(s32 on_off, u32 voice_bit) {
    u16 lo;
    u16 hi;
    u32 hi2;

    voice_bit &= 0xFFFFFF;
    lo = voice_bit;
    hi2 = voice_bit >> 16;
    hi = hi2;

    switch (on_off) {
    case 1:
        if (_spu_env & 1) {
            _spu_RQ[0] = lo;
            _spu_RQ[1] = hi;
            _spu_RQmask |= 1;
            _spu_RQvoice |= voice_bit;
            if (_spu_RQ[2] & voice_bit) {
                _spu_RQ[2] &= ~voice_bit;
            }
            if (_spu_RQ[3] & hi2) {
                _spu_RQ[3] &= ~hi2;
            }
        } else {
            u32 stat = _spu_keystat | voice_bit;
            _spu_RXX->rxx.key_on[0] = lo;
            _spu_RXX->rxx.key_on[1] = hi;
            _spu_keystat = stat;
        }
        break;
    case 0:
        if (_spu_env & 1) {
            _spu_RQ[2] = lo;
            _spu_RQ[3] = hi;
            _spu_RQmask |= 1;
            _spu_RQvoice &= ~voice_bit;
            if (_spu_RQ[0] & voice_bit) {
                _spu_RQ[0] &= ~voice_bit;
            }
            if (_spu_RQ[1] & hi2) {
                _spu_RQ[1] &= ~hi2;
            }
        } else {
            _spu_RXX->rxx.key_off[0] = lo;
            _spu_RXX->rxx.key_off[1] = hi;
            _spu_keystat &= ~voice_bit;
        }
        break;
    }
}
