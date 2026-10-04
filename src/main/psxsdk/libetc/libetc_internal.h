#ifndef LIBETC_INTERNAL_H
#define LIBETC_INTERNAL_H

/* PsyQ LIBETC library-internal entry points shared by the modules in this directory: the
 * INTR_VB / INTR_DMA start hooks that INTR's startIntr calls (SOTN declares them in
 * src/main/psxsdk/libapi/libapi_internal.h). */

#include <psxsdk/libetc.h>

extern s32 startIntrVSync(void);
extern s32 startIntrDMA(void);

#endif /* LIBETC_INTERNAL_H */
