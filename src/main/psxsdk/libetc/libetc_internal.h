#ifndef LIBETC_INTERNAL_H
#define LIBETC_INTERNAL_H

/* PsyQ LIBETC library-internal entry points shared by the modules in this directory: the
 * INTR_VB / INTR_DMA start hooks that INTR's startIntr calls (SOTN declares them in
 * src/main/psxsdk/libapi/libapi_internal.h), and INTR's interrupt / DMA handler setters, which
 * forward (irq, handler) to the callbacks table and return the previous handler (not in
 * LIBETC.H). */

#include <psxsdk/libetc.h>

extern s32 startIntrVSync(void);
extern s32 startIntrDMA(void);
extern void *InterruptCallback(s32, void (*)());
extern void *DMACallback(s32, void (*)());

#endif /* LIBETC_INTERNAL_H */
