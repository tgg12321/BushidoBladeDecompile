#!/usr/bin/env python3
# objd_d.py DIR NAME... : for each ablation candidate DIR/NAME.c (listed in DIR/list.txt), compile the tree's
# 51268.c with it substituted (tmp/p2/wk, the tree's headers) and print its instruction diff against the
# target object (relocation lines dropped): the objdump effect each FAKE comment states.
import difflib, os, re, shutil, subprocess, sys
d = sys.argv[1]
rows = dict(l.split() for l in open(d + "/list.txt", encoding="utf-8") if l.strip())
names = sys.argv[2:] or list(rows)
S = open("src/main/51268.c", encoding="utf-8").read()
def od(f, which):
    o = subprocess.run(["wsl", "bash", "memory/grind/phase2-2026-10-03/lt/f01/objd_b.sh", f] + ([which] if which else []),
                       capture_output=True, text=True).stdout
    return [re.sub(r"\s+", " ", l.strip()) for l in o.splitlines()[1:] if l.strip() and "R_MIPS" not in l]
for n in names:
    f = rows[n]
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, S); i = m.start() + 1; j = S.index("\n}\n", i) + 3
    s = S[:i] + open("%s/%s.c" % (d, n), encoding="utf-8").read() + S[j:]
    shutil.rmtree("tmp/p2/wk", ignore_errors=True); os.makedirs("tmp/p2/wk/src/main")
    open("tmp/p2/wk/src/main/51268.c", "w", encoding="utf-8", newline="\n").write(s)
    subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268"], capture_output=True)
    a, b = od(f, "base"), od(f, None)
    out = [l for l in difflib.unified_diff(a, b, lineterm="", n=0) if not l.startswith(("---", "+++"))]
    print("==", n, "(%d -> %d lines)" % (len(a), len(b)))
    print("   " + "\n   ".join(out[:40]))
