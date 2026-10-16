/* SN Systems runtime: PCopen, PCclose, PClseek (the PC file server),
 * __SN_ENTRY_POINT, __main and __do_global_dtors. .text 0x80083698..0x8008386C:
 * PsyQ 4.0 LIBSN modules OPEN, CLOSE, LSEEK and SNMAIN (LIBSN.LIB of
 * sozud/psy-q@2266d676 4.0/PSX/LIB: each module's .text equals the EXE word
 * for word outside relocations; all four are assembler-built, no FILENAME
 * record). LIBSCAN missed them (docs/naming/libscan/matches.json); kept as one
 * file for the region (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "include_asm.h"
#include <psxsdk/libsn.h>

INCLUDE_ASM("asm/funcs", PCopen);
INCLUDE_ASM("asm/funcs", PCclose);
INCLUDE_ASM("asm/funcs", PClseek);
INCLUDE_ASM("asm/funcs", __SN_ENTRY_POINT);
/* __main: SN's constructor walker (LIBSN module SNMAIN), and
   __do_global_dtors its destructor-side twin; prebuilt PsyQ object whose
   16-byte frame our cc1 cannot produce from C. It calls each entry of the
   .ctors table (D_8008D070, the section start); the count is a link-time
   constant, (.ctors size) / 4, patched into its lui/addiu pair: 0 here, as
   the game has no constructors. */
INCLUDE_ASM("asm/funcs", __main);
INCLUDE_ASM("asm/funcs", __do_global_dtors);
