/* PsyQ 4.0 LIBSND VM_DOFF: _SsVmDamperOff. .text 0x800863CC..0x800863DC, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s16 _svm_damper;

void _SsVmDamperOff(void) {
    _svm_damper = 0;
}
