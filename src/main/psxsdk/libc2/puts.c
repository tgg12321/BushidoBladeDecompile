/* PsyQ 4.0 LIBC2 PUTS: puts. .text 0x80082000..0x80082050, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* PsyQ 4.0 LIBC2 puts: puts — verbatim-linked Sony object (census
   2026-07-09); no public C ref (absent from sotn psxsdk tree); transcribed
   from the ground-truth object: putchar loop with "<NULL>" fallback. */
extern s32 D_800162CC;
extern void putchar();
void puts(void *a0) {
    char *s = a0;
    char c;

    if (s == NULL) {
        s = (char *)&D_800162CC;
    }
    while ((c = *s++) != 0) {
        putchar(c);
    }
}
