/* PsyQ 4.0 LIBC2 CTYPE: toupper and tolower. .text 0x800798CC..0x8007992C,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3.
 */
#include "common.h"
#include <psxsdk/libc.h>

extern u8 _ctype__plus_0x1;

char toupper(char a0) {
    char c = a0;
    if ((&_ctype__plus_0x1)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}

extern u8 _ctype__plus_0x1;

char tolower(char a0) {
    char c = a0;
    if ((&_ctype__plus_0x1)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
