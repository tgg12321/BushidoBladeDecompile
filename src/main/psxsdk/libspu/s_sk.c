/* PsyQ 4.0 LIBSPU S_SK: SpuSetKey. .text 0x8008AAD4..0x8008ACD0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_keystat;
extern volatile s32 _spu_RQvoice; /* _spu_RQvoice — Ruling-4 grant (volatile_extern_allowlist.txt:44) */
extern volatile s32 _spu_RQmask;
extern volatile s32 _spu_env;
extern s32 _spu_RXX;
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;
/* Sony _spu_RQ: ONE u16[4] object (PsyQ 4.0 LIBSPU S_SK relocs: addends 0/2/4/6 —
 * key-on pending [0..1], key-off pending [2..3]); splat split it into two D_
 * symbols. Ruling-4 grant, volatile_extern_allowlist.txt:40-41. */
extern volatile u16 _spu_RQ[10]; /* _spu_RQ; _spu_init clears all 10 (PsyQ 4.0 spu.c) */

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
            ((SpuRXX *)_spu_RXX)->key_on[0] = lo;
            ((SpuRXX *)_spu_RXX)->key_on[1] = hi;
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
            ((SpuRXX *)_spu_RXX)->key_off[0] = lo;
            ((SpuRXX *)_spu_RXX)->key_off[1] = hi;
            _spu_keystat &= ~voice_bit;
        }
        break;
    }
}
