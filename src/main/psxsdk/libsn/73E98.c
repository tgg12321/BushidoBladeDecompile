/* SN Systems runtime: PCopen, PCclose, PClseek (the PC file server),
 * __SN_ENTRY_POINT, __main and __do_global_dtors. .text 0x80083698..0x8008386C:
 * PsyQ 4.0 LIBSN modules OPEN, CLOSE, LSEEK and SNMAIN; .data
 * 0x800A2668..0x800A2694: SNMAIN's and the data-only module SNDEF's (LIBSN.LIB
 * of sozud/psy-q@2266d676 4.0/PSX/LIB: each module's .text equals the EXE word
 * for word outside relocations; all four are assembler-built, no FILENAME
 * record). LIBSCAN missed them (docs/naming/libscan/matches.json); kept as one
 * file for the region (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "include_asm.h"
#include <psxsdk/libsn.h>
#include <psxsdk/libgpu.h>

extern void func_800164F8(void);
extern u8 D_8008D070[];
extern SPRT_8 g_font_sprites[2][32];

/* SNMAIN .data (0x800A2668, 36 bytes; base from SNMAIN's own relocations,
 * docs/naming/sweep-2026-09-25b/data_manifest.csv). */
s32 D_800A2668 = 0; /* __main's once flag; no Sony name (no XDEF) */
u32 __heapbase = 0; /* set by __SN_ENTRY_POINT, passed to InitHeap */
u32 __heapsize = 0;
/* SNMAIN's six XDEF'd words __text, __textlen, __data, __datalen, __bss,
 * __bsslen (linker-filled in the library object): .text start and length,
 * .data start (the empty .ctors table D_8008D070) and length up to .sdata
 * (_gp, 0x800A30CC), .bss start (its first object, g_font_sprites) and
 * length up to D_801078E0. The lengths are literal link-time values, as they
 * were in the asm: a layout change must recompute them. */
u32 D_800A2674[6] = {
    (u32)func_800164F8,  0x00076B78, /* __text, __textlen */
    (u32)D_8008D070,     0x0001605C, /* __data, __datalen */
    (u32)g_font_sprites, 0x00063FB0, /* __bss, __bsslen */
};

/* LIBSN SNDEF .data (0x800A268C, a data-only module directly after
 * SNMAIN's): the default stack and RAM sizes __SN_ENTRY_POINT reads. */
u32 _stacksize = 0x00008000;
u32 _ramsize = 0x00800000;

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
