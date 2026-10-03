/* PsyQ 4.0 LIBC2 STRCPY: strcpy. .text 0x80079194..0x800791D8, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

u8 *strcpy(u8 *a0, u8 *a1) {
    u8 *v1;
    if (!a0) {
        return 0;
    }
    if (!a1) {
        return 0;
    }
    v1 = a0;
    while ((*a0++ = *a1++) != 0) {
    }
    return v1;
}
