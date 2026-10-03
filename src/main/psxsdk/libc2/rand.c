/* PsyQ 4.0 LIBC2 RAND: rand and srand. .text 0x80079154..0x80079194, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libc.h>

extern u32 D_800F1848;
s32 rand(void) {
    D_800F1848 = D_800F1848 * 0x41C64E6D + 0x3039;
    return (D_800F1848 >> 16) & 0x7FFF;
}

void srand(s32 a0) {
    D_800F1848 = a0;
}
