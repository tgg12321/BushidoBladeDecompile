/* PsyQ 4.0 LIBSPU S_Q: SpuQuit. .text 0x800892F8..0x80089374, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"
#include <psxsdk/libapi.h>

/* Declarations from the file this module was split from
 * (src/main/psxsdk/libspu/spu.c, ex main.c). */

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
