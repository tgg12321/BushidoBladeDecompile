#!/usr/bin/env python3
"""Restructure step 4b: split main_post.c (as main/d_7D870.c after the rename) into
psxsdk/libapi/a71.c (AddDrv), psxsdk/libapi/a72.c (DelDrv) and the game data tail.
Source lines move verbatim; the one __asm__ block that held DelDrv and the data is
cut in two, each part wrapped in the block's own opening/closing directives."""
import os, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC = os.path.join(ROOT, sys.argv[1])
L = open(SRC, encoding="utf-8", newline="").read().split("\n")
assert L[-1] == ""; L = L[:-1]
i_add = L.index("BIOS_B_FUNCTION(AddDrv, 0x47);")
i_asm = L.index("__asm__(")
assert L[i_asm + 1:i_asm + 4] == [r'    ".set noreorder\n"', r'    ".set noat\n"', r'    "glabel DelDrv\n"'], L[i_asm+1:i_asm+4]
i_data = L.index(r'    ".global g_data_start\n"')
i_end = L.index(r'    "endlabel DelDrv\n"')
assert L[i_end + 1:i_end + 4] == [r'    ".set reorder\n"', r'    ".set at\n"', ');'] and i_end + 4 == len(L), L[i_end:]
delbody = L[i_asm + 4:i_data]
assert delbody == [r'    "    addiu $t2, $zero, 0xB0\n"', r'    "    jr    $t2\n"', r'    "    addiu $t1, $zero, 0x48\n"', r'    "    nop\n"'], delbody
data = L[i_data:i_end]
open_ = L[i_asm:i_asm + 3]          # __asm__( .set noreorder .set noat
close = L[i_end + 1:i_end + 4]      # .set reorder .set at );
HDR = '#define INCLUDE_ASM_USE_MACRO_INC 1\n#include "include_asm.h"\n'
a71 = ("/* PsyQ 4.0 LIBAPI A71: AddDrv, the BIOS B(0x47) trampoline. .text 0x8008D050..0x8008D060, a\n"
       " * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */\n"
       + HDR + '#include "bios.h"\n\n' + L[i_add] + "\n")
a72 = ("/* PsyQ 4.0 LIBAPI A72: DelDrv, the BIOS B(0x48) trampoline. .text 0x8008D060..0x8008D070, a\n"
       " * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */\n"
       + HDR + "\n" + "\n".join(open_ + [L[i_asm + 3]] + delbody + [L[i_end]] + close) + "\n")
d = ("/* Game data at the end of .text (0x8008D070..0x8008D120, ROM 0x7D870), after the last PsyQ module\n"
     " * (LIBAPI A72 ends at 0x8008D070; LIBSCAN, Q106 D3): g_data_start (8 zero words), g_module_func_tbl\n"
     " * (34 game function pointers) and the first 8 bytes of g_sqrt_table_u8, whose rest opens\n"
     " * asm/data/7D920.data.s. The labels keep the .aent form they had inside DelDrv's asm block. */\n"
     + "\n".join(open_ + data + close) + "\n")
if "--apply" in sys.argv:
    base = os.path.join(ROOT, "src/main/psxsdk/libapi/")
    for n, s in (("a71.c", a71), ("a72.c", a72)):
        assert not os.path.exists(base + n)
        open(base + n, "w", encoding="utf-8", newline="\n").write(s)
    open(SRC, "w", encoding="utf-8", newline="\n").write(d)
print(a71); print(a72); print(d[:1200])
