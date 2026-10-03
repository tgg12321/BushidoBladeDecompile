/* PsyQ 4.0 LIBAPI C114: _96_remove, the BIOS A(0x72) trampoline, after the module's two leading
 * data words. .text 0x800831D0..0x800831F0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    /* The two data words below are the first 8 bytes of Sony's hand-written
       LIBAPI C114 object (.text+0x0; _96_remove entry is at .text+0x8 —
       matches PsyQ 4.0 LIBAPI.LIB C114, zero relocs). They are
       object data, not compiler output, and belong to this canonical
       trampoline's module. */
    "    .word 0x15007350\n"
    "    .word 0x0040809C\n"
    "glabel _96_remove\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr $t2\n"
    "    addiu $t1, $zero, 0x72\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "endlabel _96_remove\n"
    "    .set reorder\n"
    "    .set at\n"
);
