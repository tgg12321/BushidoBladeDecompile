/* PsyQ 4.0 LIBSPU S_M_INIT: SpuInitMalloc. .text 0x80089384..0x800893D8, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s32 _spu_mem_mode_plus;

extern s32 _spu_AllocBlockNum;
extern s32 _spu_AllocLastNum;
extern s32 _spu_memList;

/* PsyQ LIBSPU s_m_init.c: SpuInitMalloc — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/
   s_m_init.c */
s32 SpuInitMalloc(s32 num, s32 *top) {
    s32 size;

    if (num > 0) {
        size = 0x10000 << _spu_mem_mode_plus;
        top[0] = 0x40001010;
        _spu_memList = (s32)top;
        _spu_AllocLastNum = 0;
        _spu_AllocBlockNum = num;
        top[1] = size - 0x1010;
        return num;
    }
    return 0;
}
