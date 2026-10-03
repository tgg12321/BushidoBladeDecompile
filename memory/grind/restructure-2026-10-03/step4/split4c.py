#!/usr/bin/env python3
"""Restructure step 4c: split LIBC2 CTYPE, MEMCHR and PUTCHAR out of src/main/psxsdk/libc2/prnt.c
(ex text1b_b_tu2.c, as committed at 8a7c613e3). Source lines move verbatim; only declarations a part
needs to compile are added."""
import os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRC = os.path.join(ROOT, "src/main/psxsdk/libc2/prnt.c")
L = open(SRC, encoding="utf-8", newline="").read().split("\n")
assert L[-1] == ""
L = L[:-1]


def ln(a, b=None):
    b = a if b is None else b
    return L[a - 1:b]


assert L[10 - 1] == "extern s32 column;"
assert L[306 - 1] == "}" and L[307 - 1] == "extern u8 _ctype__plus_0x1;"
assert L[308 - 1].startswith("u8 toupper(") and L[316 - 1].startswith("u8 tolower(")
assert L[323 - 1].startswith("u8 *memchr(") and L[338 - 1] == "}"
assert L[339 - 1] == "void write(s32, u8 *, s32);" and L[340 - 1].startswith("void putchar(")
assert L[363 - 1] == "}" and len(L) == 363
assert L[51 - 1] == "extern u8 _ctype__plus_0x1;"
SPAN = "a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3"
DECL = "/* Declarations from the file this module was split from (src/main/psxsdk/libc2/prnt.c). */\n"


def head(mod, st, en, what):
    return (f"/* PsyQ 4.0 LIBC2 {mod}: {what}. .text 0x{st:08X}..0x{en:08X},\n"
            f" * {SPAN}. */\n#include \"common.h\"\n\n")


ctype = head("CTYPE", 0x800798CC, 0x8007992C, "toupper and tolower") + "\n".join(ln(307, 322)) + "\n"
memchr = head("MEMCHR", 0x8007992C, 0x8007997C, "memchr") + "\n".join(ln(323, 338)) + "\n"
putchar = (head("PUTCHAR", 0x8007997C, 0x80079A30, "putchar") + DECL + L[10 - 1] + "\n" + L[51 - 1] + "\n\n"
           + "\n".join(ln(339, 363)) + "\n")
TOP = ("/* PsyQ 4.0 LIBC2 PRNT: prnt, the printf formatter. .text 0x80079244..0x800798CC and its .rodata\n"
       " * 0x80015A68..0x80015C7C (the digit and \"(null)\" strings, then the format switch's jump table),\n"
       " * " + SPAN + ". */")
prnt = [TOP] + [l for i, l in enumerate(L[:306], 1) if i != 10]
if "--apply" in sys.argv:
    base = os.path.join(ROOT, "src/main/psxsdk/libc2/")
    for n, s in (("ctype.c", ctype), ("memchr.c", memchr), ("putchar.c", putchar)):
        assert not os.path.exists(base + n)
        open(base + n, "w", encoding="utf-8", newline="\n").write(s)
    open(SRC, "w", encoding="utf-8", newline="\n").write("\n".join(prnt) + "\n")
print(ctype); print(memchr); print(putchar); print("\n".join(prnt[:14]))
