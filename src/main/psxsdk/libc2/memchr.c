/* PsyQ 4.0 LIBC2 MEMCHR: memchr. .text 0x8007992C..0x8007997C,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3.
 */
#include "common.h"
#include <psxsdk/libc.h>

u8 *memchr(u8 *buf, s32 ch, s32 len) {
    if (buf == 0)
        return 0;
    if (len <= 0)
        return 0;
    len--;
    goto check;
found:
    return buf - 1;
check:
    if (len < 0)
        return 0;
    ch &= 0xFF;
loop:
    if (*buf++ == ch)
        goto found;
    --len;
    if (len >= 0)
        goto loop;
    return 0;
}
