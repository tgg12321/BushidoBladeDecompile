/* PsyQ 4.0 LIBSPU S_M_UTIL: _SpuIsInAllocateArea and _SpuIsInAllocateArea_.
 * .text 0x80089E30..0x80089F3C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_m_util: _SpuIsInAllocateArea / _SpuIsInAllocateArea_ —
   verbatim-linked Sony object; C ref: sotn-decomp
   src/main/psxsdk/libspu/s_m_util.c (4.0 adds the NULL list guard) */
s32 _SpuIsInAllocateArea(u32 arg0) {
    s32 i;

    if (_spu_memList == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (_spu_memList[i].addr & 0x80000000) {
            continue;
        }
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (_spu_memList[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (_spu_memList[i].addr & 0x0FFFFFFF) + _spu_memList[i].size) {
            return 1;
        }
    }
    return 0;
}

s32 _SpuIsInAllocateArea_(u32 arg0) {
    s32 i;

    arg0 <<= _spu_mem_mode_plus;
    if (_spu_memList == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (_spu_memList[i].addr & 0x80000000) {
            continue;
        }
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (_spu_memList[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (_spu_memList[i].addr & 0x0FFFFFFF) + _spu_memList[i].size) {
            return 1;
        }
    }
    return 0;
}
