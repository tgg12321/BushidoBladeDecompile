/* PsyQ 4.0 LIBC2 PRINTF: printf (formats through prnt, LIBC2 PRNT). .text
 * 0x80079208..0x80079244, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

/* LIBC2 PRNT (libc2/prnt.c): s32 prnt(s32, u8 *, char *). Left unprototyped:
 * printf passes fmt as the s32 word it spilled. */
extern s32 prnt();

void printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;
    ap[1] = a;
    ap[2] = b;
    ap[3] = c;
    prnt(1, fmt, ap + 1);
}
