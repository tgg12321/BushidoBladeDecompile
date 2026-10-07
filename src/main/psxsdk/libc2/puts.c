/* PsyQ 4.0 LIBC2 PUTS: puts. .text 0x80082000..0x80082050, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

/* .rodata 0x800162CC..0x800162D4: puts's NULL-pointer text (moved from
 * src/text1a_b_tail_rodata.c, Q106 D4: every reader is in this file, in link
 * order). */

/* D_800162CC: 1 string(s), 8B @ 0x800162CC */
const char D_800162CC[8] = "<NULL>\0\0";

/* PsyQ 4.0 LIBC2 puts: puts — verbatim-linked Sony object (census
   2026-07-09); no public C ref (absent from sotn psxsdk tree); transcribed
   from the ground-truth object: putchar loop with "<NULL>" fallback. */
extern void putchar();

void puts(char *s) {
    char c;

    if (s == NULL) {
        s = (char *)&D_800162CC;
    }
    while ((c = *s++) != 0) {
        putchar(c);
    }
}
