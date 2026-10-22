/* PsyQ 4.0 LIBC2 PUTS: puts. .text 0x80082000..0x80082050, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

/* .rodata 0x800162CC: puts's NULL-pointer text (here by Q106 D4: every reader
 * is in this file). */
const char D_800162CC[8] = "<NULL>\0\0";

/* No public C ref; transcribed from the object: putchar loop with "<NULL>"
   fallback. */
extern void putchar();

void puts(const char *s) {
    char c;

    if (s == NULL) {
        s = (char *)&D_800162CC;
    }
    while ((c = *s++) != 0) {
        putchar(c);
    }
}

/* .data 0x800A14C4..0x800A14CC: LIBC2.LIB PUTS's 8-byte .data, which no code
 * reads (equal to the library object's; like LIBCARD INIT's D_8009BE1C) */
u32 D_800A14C4[2] = {0x15027350, 0x0040849C};
