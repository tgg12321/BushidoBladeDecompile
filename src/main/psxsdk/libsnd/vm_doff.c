/* PsyQ 4.0 LIBSND VM_DOFF: _SsVmDamperOff. .text 0x800863CC..0x800863DC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void _SsVmDamperOff(void) { _svm_damper = 0; }
