/* SN Systems runtime: PCopen, PCclose, PClseek (the PC file server),
 * __SN_ENTRY_POINT, __main and __do_global_dtors. .text 0x80083698..0x8008386C:
 * an unidentified gap between verbatim LIBSCAN modules
 * (docs/naming/libscan/matches.json), one file per gap (Q106 D3), named by its
 * ROM offset. */
#include "common.h"
#include "include_asm.h"
#include <psxsdk/libsn.h>

INCLUDE_ASM("asm/funcs", PCopen);
INCLUDE_ASM("asm/funcs", PCclose);
INCLUDE_ASM("asm/funcs", PClseek);
INCLUDE_ASM("asm/funcs", __SN_ENTRY_POINT);
/* __main: libgcc / crt0 constructor walker, and __do_global_dtors its
   destructor-side twin; prebuilt PsyQ object whose 16-byte frame our cc1
   cannot produce from C. */
INCLUDE_ASM("asm/funcs", __main);
INCLUDE_ASM("asm/funcs", __do_global_dtors);
