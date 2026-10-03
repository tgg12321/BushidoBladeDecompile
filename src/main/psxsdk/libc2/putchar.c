/* PsyQ 4.0 LIBC2 PUTCHAR: putchar. .text 0x8007997C..0x80079A30,
 * a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libc2/prnt.c). */
extern s32 column;
extern u8 _ctype__plus_0x1;

void write(s32, u8 *, s32);
void putchar(s8 arg0) {
    u8 sp10;
    s32 temp_a0;

    sp10 = arg0;
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 == 9) goto loop;
    if (temp_a0 == 0xA) {
        putchar(0xD);
        column = 0;
        goto tail;
    }
    goto def;
loop:
    putchar(0x20);
    if ((column & 7) == 0) return;
    goto loop;
def:
    if ((&_ctype__plus_0x1)[temp_a0] & 0x97) {
        column += 1;
    }
tail:
    write(1, &sp10, 1);
}
