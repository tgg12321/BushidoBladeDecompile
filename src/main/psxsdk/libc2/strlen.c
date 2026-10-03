/* PsyQ 4.0 LIBC2 STRLEN: strlen. .text 0x800791D8..0x80079208, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

s32 strlen(u8 *a0) {
    s32 v1 = 0;
    if (!a0) {
        return 0;
    }
    while (*a0++ != 0) {
        v1++;
    }
    return v1;
}
