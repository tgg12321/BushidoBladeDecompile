"""mk.py <variant.c> <out.c> [--gpu]: src/text1b.c with the region from `extern u8 D_800EF848[];` through
`INCLUDE_ASM("asm/funcs", func_80048FFC);` replaced by the variant (func_80048F58 + func_80048FFC + decls)."""
import sys
src = open("src/text1b.c", encoding="utf-8").read()
a = src.index("extern u8 D_800EF848[];\n")
end_marker = 'INCLUDE_ASM("asm/funcs", func_80048FFC);\n'
b = src.index(end_marker) + len(end_marker)
var = open(sys.argv[1], encoding="utf-8").read()
out = src[:a] + var + src[b:]
if "--gpu" in sys.argv:
    out = out.replace('#include "gte.h"\n', '#include "gte.h"\n#include "gpu.h"\n', 1)
open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(out)
