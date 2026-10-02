"""splice2.py <ffc.c|-> <cam.c|-> <out.c>: src/text1b.c with the two INCLUDE_ASM lines replaced."""
import sys
src = open("src/text1b.c", encoding="utf-8", newline="").read()
for path, fn in ((sys.argv[1], "func_80048FFC"), (sys.argv[2], "camera_CalcAngles")):
    if path == "-":
        continue
    line = 'INCLUDE_ASM("asm/funcs", %s);\n' % fn
    assert src.count(line) == 1, fn
    src = src.replace(line, open(path, encoding="utf-8", newline="").read())
open(sys.argv[3], "w", encoding="utf-8", newline="\n").write(src)
