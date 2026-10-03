/* PsyQ 4.0 LIBSPU S_M_F: SpuFree. .text 0x800899A8..0x80089A24, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* Self-referential on purpose: the object `_spu_memList` (Sony's SPU_MALLOC list pointer,
   declared s32 in libspu_internal.h) is viewed as SpuMemRec* through this macro; a macro
   name inside its own replacement list is not re-expanded (C90 6.8.3.4). */
#define _spu_memList ((SpuMemRec *)_spu_memList)

/* PsyQ 4.0 LIBSPU s_m_f: SpuFree — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_m_f.c */
void SpuFree(u32 arg0) {
    s32 i;

    for (i = 0; i < _spu_AllocBlockNum; i++) {
        if (((SpuMemRec *)_spu_memList)[i].addr & 0x40000000) {
            break;
        }
        if (((SpuMemRec *)_spu_memList)[i].addr == arg0) {
            ((SpuMemRec *)_spu_memList)[i].addr |= 0x80000000;
            break;
        }
    }
    _spu_gcSPU();
}
