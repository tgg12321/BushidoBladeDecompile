/* PsyQ 4.0 LIBSPU S_M: data only (no .text), the SPU memory allocator state.
 * .data 0x800A2D38..0x800A2D44, the module's 12-byte .data in LIBSPU.LIB
 * (XDEFs _spu_AllocBlockNum +0, _spu_AllocLastNum +4, _spu_memList +8, no
 * relocation; .sdata empty), equal to the EXE's zero bytes. Read by
 * s_ini.c, s_m_init.c, s_m_m.c, s_m_f.c, s_m_int.c and s_m_util.c.
 * Owner ruling Q122 (data-only-no-sdata): three 4-byte objects the library
 * object keeps in .data, sized by its XDEFs; in one run between LIBSPU SPU's
 * .data and S_RMP's, both Sony library modules with no .text between them, so
 * the file builds without maspsx -G8. */
#include "common.h"
#include "libspu_internal.h"

s32 _spu_AllocBlockNum = 0;
s32 _spu_AllocLastNum = 0;
SpuMemRec *_spu_memList = NULL;
