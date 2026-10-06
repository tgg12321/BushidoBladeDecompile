#!/usr/bin/env python3
# warn_b.py TU... : cc1 (no -w) diagnostics, HEAD copies (tmp/p2/lt/f01/b/headinc: 51268.c, game.h, bb2.h) vs the tree
# (the HEAD side puts headinc/include ahead of include/). Prints gained / lost (function, message), line-free.
import collections, os, re, subprocess, sys
HI = "tmp/p2/lt/f01/b/headinc/"
for _p in ("src/main/51268.c", "include/game.h", "include/bb2.h"):   # HEAD 7b5212f70's copies
    if not os.path.exists(HI + _p):
        os.makedirs(os.path.dirname(HI + _p), exist_ok=True)
        open(HI + _p, "wb").write(subprocess.run(["git", "show", "7b5212f70:" + _p], capture_output=True, check=True).stdout)
CPPF = ["-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__",
        "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C",
        "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-mel", "-msoft-float", "-o", "/dev/null"]
def diags(inc, path, d):
    pre = subprocess.run(["mipsel-linux-gnu-cpp"] + inc + ["-I" + d, "-Iinclude"] + CPPF + [path], capture_output=True).stdout
    err = subprocess.run(CC1, input=pre, capture_output=True).stderr.decode("utf-8", "replace")
    fn, out = "<file>", collections.Counter()
    for line in err.splitlines():
        m = re.search(r"In function `([^']*)'", line)
        if m: fn = m.group(1); continue
        if "At top level" in line: fn = "<file>"; continue
        out[(fn, re.sub(r"^[^:]*:\d+:\s*", "", line))] += 1
    return out
for tu in sys.argv[1:]:
    d = os.path.dirname("src/" + tu + ".c")
    wk = "tmp/p2/wk/src/" + tu + ".c"
    if not os.path.exists(wk): wk = "src/" + tu + ".c"
    a = diags(["-Itmp/p2/lt/f01/b/headinc/include"], (lambda h: h if os.path.exists(h) else "src/" + tu + ".c")("tmp/p2/lt/f01/b/headinc/src/" + tu + ".c"), d)
    b = diags([], "src/" + tu + ".c", d)
    if a != b or len(sys.argv) < 6: print("== %s  %d -> %d" % (tu, sum(a.values()), sum(b.values())))
    for k in sorted(set(a) | set(b)):
        if a[k] != b[k]: print("   %s %d -> %d  %s: %s" % ("+" if b[k] > a[k] else "-", a[k], b[k], k[0], k[1][:150]))
