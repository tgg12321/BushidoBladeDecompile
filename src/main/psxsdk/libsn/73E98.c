/* SN Systems runtime: PCopen, PCclose, PClseek (the PC file server), __SN_ENTRY_POINT, __main and
 * __do_global_dtors. .text 0x80083698..0x8008386C: an unidentified region between verbatim LIBSCAN
 * modules (docs/naming/libscan/matches.json; memory/closer/psyq-library-census.md), one file per
 * gap (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "include_asm.h"
#include <psxsdk/libsn.h>

INCLUDE_ASM("asm/funcs", PCopen);
INCLUDE_ASM("asm/funcs", PCclose);
INCLUDE_ASM("asm/funcs", PClseek);
INCLUDE_ASM("asm/funcs", __SN_ENTRY_POINT);
/* 0x80083794 = libgcc __main / crt0 ctor-walker — COMPLETED-INLINE-ASM-CANONICAL
   (owner routing ruling: provably prebuilt PsyQ object; our cc1 cannot produce
   the 16-byte frame from any C — REG_PARM_STACK_SPACE; entry in
   inline_asm_canonical.txt). __do_global_dtors @0x80083804 (below) is its
   dtor-side twin, COMPLETED-INLINE-ASM-CANONICAL on the same grounds (same
   prebuilt crt0/libgcc object; entry in inline_asm_canonical.txt). */
INCLUDE_ASM("asm/funcs", __main);
INCLUDE_ASM("asm/funcs", __do_global_dtors);
