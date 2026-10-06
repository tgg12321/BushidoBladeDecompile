#!/usr/bin/env python3
# warn_b.py TU... : cc1 (no -w) diagnostics, a base revision's copies of 51268.c / game.h / bb2.h vs the tree
# (the base side puts its include/ copies ahead of include/). Prints gained / lost (function, message),
# line-free. Base revision: $HEADREV (default 7b5212f70, F01b1's base; F01b2 used HEADREV=9126eb268).
import collections, os, re, subprocess, sys
REV = os.environ.get("HEADREV", "7b5212f70")
HI = "tmp/p2/lt/f01/b/headinc_%s/" % REV
for _p in ("src/main/51268.c", "include/game.h", "include/bb2.h"):   # the base revision's copies
    if not os.path.exists(HI + _p):
        os.makedirs(os.path.dirname(HI + _p), exist_ok=True)
        open(HI + _p, "wb").write(subprocess.run(["git", "show", REV + ":" + _p], capture_output=True, check=True).stdout)
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
    a = diags(["-I" + HI + "include"], (lambda h: h if os.path.exists(h) else "src/" + tu + ".c")(HI + "src/" + tu + ".c"), d)
    b = diags([], "src/" + tu + ".c", d)
    if a != b or len(sys.argv) < 6: print("== %s  %d -> %d" % (tu, sum(a.values()), sum(b.values())))
    for k in sorted(set(a) | set(b)):
        if a[k] != b[k]: print("   %s %d -> %d  %s: %s" % ("+" if b[k] > a[k] else "-", a[k], b[k], k[0], k[1][:150]))
