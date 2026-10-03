/* PsyQ 4.0 LIBSPU S_M_UTIL: _SpuIsInAllocateArea and _SpuIsInAllocateArea_. .text
 * 0x80089E30..0x80089F3C, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"
#include "libspu_internal.h"

/* Self-referential on purpose: the object `_spu_memList` (Sony's SPU_MALLOC list pointer,
   declared s32 in libspu_internal.h) is viewed as SpuMemRec* through this macro; a macro
   name inside its own replacement list is not re-expanded (C90 6.8.3.4). */
#define _spu_memList ((SpuMemRec *)_spu_memList)

/* PsyQ 4.0 LIBSPU s_m_util: _SpuIsInAllocateArea / _SpuIsInAllocateArea_ —
   verbatim-linked Sony object; C ref: sotn-decomp
   src/main/psxsdk/libspu/s_m_util.c (4.0 adds the NULL list guard) */
s32 _SpuIsInAllocateArea(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)_spu_memList;
    s32 i;

    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}

s32 _SpuIsInAllocateArea_(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)_spu_memList;
    s32 i;

    arg0 <<= _spu_mem_mode_plus;
    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}
