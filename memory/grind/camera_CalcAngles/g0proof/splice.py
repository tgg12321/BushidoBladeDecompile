"""splice.py <candidate.c> <out.c>: copy src/text1b.c with camera_CalcAngles' INCLUDE_ASM line replaced."""
import sys
src = open("src/text1b.c", encoding="utf-8", newline="").read()
cand = open(sys.argv[1], encoding="utf-8", newline="").read()
line = 'INCLUDE_ASM("asm/funcs", camera_CalcAngles);\n'
assert src.count(line) == 1
open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(src.replace(line, cand))
