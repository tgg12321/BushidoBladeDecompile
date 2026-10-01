#!/usr/bin/env python3
"""splice.py <candidate.c> <out.c>: text1b.c with the jtbl transcriptions removed and the candidate in place of the INCLUDE_ASM."""
import sys
src = open("src/text1b.c", encoding="utf-8").read()
cand = open(sys.argv[1], encoding="utf-8").read()
a = src.index("/* func_80058580's three switch tables")
b = src.index('INCLUDE_ASM("asm/funcs", func_80058580);')
e = b + len('INCLUDE_ASM("asm/funcs", func_80058580);')
out = src[:a] + cand + src[e:]
open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(out)
