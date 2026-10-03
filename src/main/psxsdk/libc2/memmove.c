/* PsyQ 4.0 LIBC2 MEMMOVE: memmove (LIBC and LIBC2 MEMMOVE are byte-identical; libc2/ as for
 * sprintf.c). .text 0x8007A28C..0x8007A2F8, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

/* PsyQ 4.0 LIBC2 MEMMOVE: memmove — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libc/memmove.c */
u8 *memmove(u8 *dst, u8 *src, s32 n) {
    if (dst >= src) {
        while (n-- > 0) {
            dst[n] = src[n];
        }
    } else {
        while (n-- > 0) {
            *dst++ = *src++;
        }
    }
    return dst;
}
