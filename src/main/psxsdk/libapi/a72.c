/* PsyQ 4.0 LIBAPI A72: DelDrv, the BIOS B(0x48) trampoline. .text 0x8008D060..0x8008D070, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel DelDrv\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x48\n"
    "    nop\n"
    "endlabel DelDrv\n"
    ".set reorder\n"
    ".set at\n"
);
