# objabl_b2.py NAME FUNC : compile tmp/p2/f01b2/51268.c (f01b2.py output) with tmp/p2/f01b2/abl/NAME.c (abl_b2.py)
# substituted, in tmp/p2/wk with the scratch headers, and print base-vs-variant objdumps side by side.
import re, shutil, os, subprocess, sys
name, f = sys.argv[1], sys.argv[2]
S = open("tmp/p2/f01b2/51268.c", encoding="utf-8").read()
b = open("tmp/p2/f01b2/abl/%s.c" % name, encoding="utf-8").read() if name != "base" else None
if b:
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, S); i = m.start() + 1; j = S.index("\n}\n", i) + 3
    S = S[:i] + b + S[j:]
shutil.rmtree("tmp/p2/wk", ignore_errors=True); os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
for p in ("game.h", "bb2.h"): shutil.copy("tmp/p2/f01b2/" + p, "tmp/p2/wk/include/" + p)
open("tmp/p2/wk/src/main/51268.c", "w", encoding="utf-8", newline="\n").write(S)
print(subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268", f], capture_output=True, text=True).stdout.strip())
print(subprocess.run(["wsl", "bash", "memory/grind/phase2-2026-10-03/lt/f01/sbs_b2.sh", f], capture_output=True, text=True).stdout)
