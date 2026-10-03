/* LIBSPU code between verbatim modules: _SpuSetAnyVoice. .text 0x80089A48..0x80089D10: an
 * unidentified region between verbatim LIBSCAN modules (docs/naming/libscan/matches.json;
 * memory/closer/libsnd-hunt-report.md lists the probable newer-build modules), one file per gap
 * (Q106 D3), named by its ROM offset. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern volatile s32 _spu_RQmask;
extern volatile s32 _spu_env;
extern s32 _spu_RXX;
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;
typedef union {
    SpuRXX rxx;
    volatile u16 raw[0x100];
} SpuUnion;
extern SpuUnion D_800F7298;

s32 _SpuSetAnyVoice(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (_spu_env & 1) {
        var_t0 = ((D_800F7298.raw[addr2] & 0xFF) << 16) | D_800F7298.raw[addr1];
    } else {
        var_t0 = ((((SpuUnion *)_spu_RXX)->raw[addr2] & 0xFF) << 16) | ((SpuUnion *)_spu_RXX)->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] |= bits;
            D_800F7298.raw[addr2] |= (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] |= bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] &= ~bits;
            D_800F7298.raw[addr2] &= ~((bits >> 16) & 0xFF);
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] &= ~bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (_spu_env & 1) {
            D_800F7298.raw[addr1] = bits;
            D_800F7298.raw[addr2] = (bits >> 16) & 0xFF;
            _spu_RQmask |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)_spu_RXX)->raw[addr1] = bits;
            ((SpuUnion *)_spu_RXX)->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
