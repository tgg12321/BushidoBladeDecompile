/* PsyQ 4.0 LIBSPU S_M_F: SpuFree. .text 0x800899A8..0x80089A24, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

/* PsyQ 4.0 LIBSPU s_m_f: SpuFree — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_m_f.c */
void SpuFree(u32 arg0) {
    s32 i;

    for (i = 0; i < _spu_AllocBlockNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (_spu_memList[i].addr == arg0) {
            _spu_memList[i].addr |= 0x80000000;
            break;
        }
    }
    _spu_gcSPU();
}
