#!/usr/bin/env python3
"""Work item D / Q108: split asm/funcs/FlushCache.s at 0x80079000 (LIBAPI C68 | SENDPAD) and
src/main/psxsdk/libapi/c68.c into c68.c (FlushCache) + sendpad.c (_SendPAD, _send_pad,
func_800790A4). Instruction lines move verbatim; only glabel/endlabel lines are added."""
import os, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
def rd(p): return open(os.path.join(ROOT, p), encoding="utf-8", newline="").read()
def wr(p, s):
    open(os.path.join(ROOT, p), "w", encoding="utf-8", newline="\n").write(s)
L = rd("asm/funcs/FlushCache.s").split("\n")
assert L[-1] == "" and L[0] == "glabel FlushCache" and L[-2] == "endlabel FlushCache", L[:2]
body = L[1:-2]
assert len(body) == 14
assert "80078FFC 00000000" in body[3] and "80079000 0A80093C" in body[4], body[3:5]
flush = ["glabel FlushCache"] + body[:4] + ["endlabel FlushCache", ""]
sendpad = ["glabel _SendPAD"] + body[4:] + ["endlabel _SendPAD", ""]
HDR = '#define INCLUDE_ASM_USE_MACRO_INC 1\n#include "include_asm.h"\n\n'
c68 = ("/* PsyQ 4.0 LIBAPI C68: FlushCache, the BIOS A(0x44) trampoline (4 words incl. the module's\n"
       " * trailing nop). .text 0x80078FF0..0x80079000, a verbatim LIBSCAN module span\n"
       " * (docs/naming/libscan/matches.json), Q106 D3. */\n"
       + HDR + 'INCLUDE_ASM("asm/funcs", FlushCache);\n')
old = rd("src/main/psxsdk/libapi/c68.c")
tail = old[old.index('INCLUDE_ASM("asm/funcs", _send_pad);'):]
assert tail == 'INCLUDE_ASM("asm/funcs", _send_pad);\nINCLUDE_ASM("asm/funcs", func_800790A4);\n', tail
sp = ("/* PsyQ 4.0 LIBAPI SENDPAD: _SendPAD, _send_pad and the BIOS patch stub func_800790A4 (data-as-code:\n"
      " * the 4 patch words, up to D_800790B4, that _send_pad writes into the BIOS pad code at B0[0x5B] +0x3D8\n"
      " * and +0x4E0; then the module's 3 pad words).\n"
      " * .text 0x80079000..0x800790C0, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json),\n"
      " * Q106 D3; split from C68 by owner ruling Q108 (docs/naming/libscan/boundary_fixes.md). */\n"
      + HDR + 'INCLUDE_ASM("asm/funcs", _SendPAD);\n' + tail)
if "--apply" in sys.argv:
    assert not os.path.exists(os.path.join(ROOT, "asm/funcs/_SendPAD.s"))
    assert not os.path.exists(os.path.join(ROOT, "src/main/psxsdk/libapi/sendpad.c"))
    wr("asm/funcs/FlushCache.s", "\n".join(flush))
    wr("asm/funcs/_SendPAD.s", "\n".join(sendpad))
    wr("src/main/psxsdk/libapi/c68.c", c68)
    wr("src/main/psxsdk/libapi/sendpad.c", sp)
print("\n".join(flush)); print("\n".join(sendpad)); print(c68); print(sp)
