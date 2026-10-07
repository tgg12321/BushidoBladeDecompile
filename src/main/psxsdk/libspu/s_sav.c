/* PsyQ LIBSPU S_SAV: _SpuSetAnyVoice. .text 0x80089A48..0x80089D10, the whole
 * region between S_SNV and S_SNC (a newer build than PsyQ 4.0's 0x208-byte
 * S_SAV; memory/closer/libsnd-hunt-report.md). Module start (owner ruling
 * Q109), libscan xref tier: the verbatim S_SNV and S_SRV modules' REL26 at +0xC
 * name _SpuSetAnyVoice -> EXE jal 0x80089A48
 * (docs/naming/libscan/near_manifest.csv), S_SAV's only XDEF (+0x0, PsyQ 4.0
 * LIBSPU.LIB). */
#include "common.h"
#include "libspu_internal.h"

s32 _SpuSetAnyVoice(s32 on_off, u32 bits, s32 addr1, s32 addr2) {
    u32 var_t0;

    if (_spu_env & 1) {
        var_t0 = ((_spu_RQ[addr2 - 0xC4] & 0xFF) << 16) | _spu_RQ[addr1 - 0xC4];
    } else {
        var_t0 = ((_spu_RXX->raw[addr2] & 0xFF) << 16) | _spu_RXX->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (_spu_env & 1) {
            _spu_RQ[addr1 - 0xC4] |= bits;
            _spu_RQ[addr2 - 0xC4] |= (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            _spu_RXX->raw[addr1] |= bits;
            _spu_RXX->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (_spu_env & 1) {
            _spu_RQ[addr1 - 0xC4] &= ~bits;
            _spu_RQ[addr2 - 0xC4] &= ~((bits >> 16) & 0xFF);
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            _spu_RXX->raw[addr1] &= ~bits;
            _spu_RXX->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (_spu_env & 1) {
            _spu_RQ[addr1 - 0xC4] = bits;
            _spu_RQ[addr2 - 0xC4] = (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            _spu_RXX->raw[addr1] = bits;
            _spu_RXX->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
