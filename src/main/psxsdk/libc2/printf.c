/* PsyQ 4.0 LIBC2 PRINTF: printf (formats through prnt, LIBC2 PRNT). .text 0x80079208..0x80079244,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

void printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;
    ap[1] = a;
    ap[2] = b;
    ap[3] = c;
    prnt(1, fmt, ap + 1);
}
