/* PsyQ 4.0 LIBSND VS_AUTO: SsSetAutoKeyOffMode. .text 0x80087F00..0x80087F10, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void SsSetAutoKeyOffMode(u8 a0) {
    _svm_auto_kof_mode = a0;
}
