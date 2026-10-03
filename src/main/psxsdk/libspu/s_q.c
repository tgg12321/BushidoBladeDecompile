/* PsyQ 4.0 LIBSPU S_Q: SpuQuit. .text 0x800892F8..0x80089374, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
/* PsyQ LIBSPU: Sony's own header types the SPU transfer callback as a
   volatile function pointer; volatile_extern_allowlist.txt grant.
   SOTN: src/main/psxsdk/libspu/libspu_internal.h:39 @db41b28 (PS1 use:
   src/main/psxsdk/libspu/s_r.c:10) */
extern void (* volatile _spu_transferCallback)();
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern s32 _spu_IRQCallback;
extern s32 _spu_isCalled;
extern s32 _spu_EVdma;
void _SpuDataCallback(s32 a0);

/* PsyQ LIBSPU s_q.c: SpuQuit — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libspu/s_q.c. */


void SpuQuit(void) {
    if (_spu_isCalled == 1) {
        _spu_isCalled = 0;
        EnterCriticalSection();
        _spu_transferCallback = NULL;
        _spu_IRQCallback = 0;
        _SpuDataCallback(0);
        CloseEvent(_spu_EVdma);
        DisableEvent(_spu_EVdma);
        ExitCriticalSection();
    }
}
