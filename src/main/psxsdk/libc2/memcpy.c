/* PsyQ 4.0 LIBC2 MEMCPY: memcpy. .text 0x80079120..0x80079154, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

u8 *memcpy(u8 *dst, u8 *src, s32 len) {
    u8 *ret;
    if (!dst) {
        return 0;
    }
    ret = dst;
    while (len > 0) {
        *dst = *src;
        src++;
        len--;
        dst++;
    }
    return ret;
}
