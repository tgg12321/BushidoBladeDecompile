#!/usr/bin/env python3
"""cc.py <candidate.c> [outdir] [extra cc1 flags...]
Substitute the candidate for INCLUDE_ASM(func_800290B8) in code6cac_b.c, run cpp|cc1,
write outdir/t.s (+ any dumps) and outdir/f.s (the function only)."""
import os, re, subprocess, sys

ROOT = "/home/user/BushidoBladeDecompile"
FUNC = "func_800290B8"
cand, out = sys.argv[1], os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else ROOT + "/tmp/w290/out")
extra = sys.argv[3:]
os.makedirs(out, exist_ok=True)
src = open(ROOT + "/src/code6cac_b.c").read()
body = open(cand).read()
inc = 'INCLUDE_ASM("asm/funcs", %s);' % FUNC
if inc in src:
    src = src.replace(inc, body, 1)
else:  # landed tree: swap the committed body (its leading comment .. closing brace) instead
    a = src.index("/* Marker list returned by func_8004678C")
    b = src.index("\n}\n", src.index("s32 func_800290B8(", a)) + 3
    src = src[:a] + body + src[b:]
open(out + "/t.c", "w", newline="\n").write(src)
cpp = ("mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 "
       "-D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ "
       "-D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C").split()
i = subprocess.run(cpp + [out + "/t.c"], capture_output=True, text=True, cwd=ROOT)
open(out + "/t.i", "w", newline="\n").write(i.stdout)
flags = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float".split()
r = subprocess.run([ROOT + "/tools/gcc-2.7.2/build/cc1"] + flags + extra + ["t.i", "-o", "t.s"],
                   capture_output=True, text=True, cwd=out)
if r.returncode:
    print(r.stderr[-3000:]); sys.exit(1)
s = open(out + "/t.s").read()
m = re.search(r"\n%s:\n(.*?)\t\.end\t%s" % (FUNC, FUNC), s, re.S)
open(out + "/f.s", "w", newline="\n").write(m.group(1))
print("ok", out + "/f.s")
