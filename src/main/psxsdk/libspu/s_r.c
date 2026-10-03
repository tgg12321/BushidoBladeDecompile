/* PsyQ 4.0 LIBSPU S_R: SpuRead. .text 0x8008AD64..0x8008ADC4, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
/* PsyQ LIBSPU: Sony's own header types the SPU transfer callback as a
   volatile function pointer; volatile_extern_allowlist.txt grant.
   SOTN: src/main/psxsdk/libspu/libspu_internal.h:39 @db41b28 (PS1 use:
   src/main/psxsdk/libspu/s_r.c:10) */
extern void (* volatile _spu_transferCallback)();
extern s32 _spu_inTransfer;
s32 _spu_Fr(s32 a0, s32 a1);

s32 SpuRead(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    _spu_Fr(a0, a1);
    if (_spu_transferCallback == NULL) {
        _spu_inTransfer = 0;
    }
    return a1;
}
