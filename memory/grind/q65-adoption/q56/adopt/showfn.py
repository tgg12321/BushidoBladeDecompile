#!/usr/bin/env python3
"""showfn.py <tree> func...: print each function's C definition (first match in src/*.c, brace-balanced) or,
if it is INCLUDE_ASM, its asm/funcs/<func>.s body (instruction text only)."""
import os, re, sys
T = sys.argv[1]
srcs = {f: open(f"{T}/src/{f}", errors="replace").read() for f in sorted(os.listdir(f"{T}/src")) if f.endswith(".c")}
for fn in sys.argv[2:]:
    done = False
    for f, t in srcs.items():
        m = re.search(r"^[A-Za-z_][^\n;]*?\b%s\s*\([^;{]*\)\s*(?:[^;{()]*;\s*)*\{" % re.escape(fn), t, re.M)
        if not m:
            continue
        i, d = m.end() - 1, 0
        for j in range(i, len(t)):
            d += {"{": 1, "}": -1}.get(t[j], 0)
            if d == 0:
                break
        body = t[m.start():j + 1]
        print(f"==== {fn}  ({f})\n{body if len(body) < 4000 else body[:4000] + ' ...'}\n")
        done = True
        break
    if not done:
        p = f"{T}/asm/funcs/{fn}.s"
        if os.path.exists(p):
            ins = [re.sub(r"^\s*/\*.*?\*/\s*", "", l).rstrip() for l in open(p) if "/*" in l]
            print(f"==== {fn}  (asm, {len(ins)} insns)\n" + "\n".join(ins[:120]) + "\n")
        else:
            print(f"==== {fn}: not found\n")
