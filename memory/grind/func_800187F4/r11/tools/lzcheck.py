import glob, re, sys
files = sorted(glob.glob("memory/grind/func_800187F4/r11/variants/*.c") + glob.glob("memory/grind/func_800187F4/r11/variants_v2/*.c")
               + glob.glob("memory/grind/func_800187F4/r11/variants_v3/*.c") + glob.glob("memory/grind/func_800187F4/rejected/*.template.c")
               + ["tmp/func_800187F4/rv_wd_end.c", "tmp/func_800187F4/rv_wd_after2.c", "tmp/func_800187F4/rv_both.c"])
decl = re.compile(r"\n    s32 lz\[(\d+)\];\n")
from collections import Counter
c = Counter(); odd = []
for f in files:
    m = decl.findall(open(f, encoding="utf-8").read())
    k = m[0] if len(m) == 1 else ("none" if not m else "many")
    c[k] += 1
    if k != "6": odd.append((f.split("/")[-1], k))
print(c); print(odd)
