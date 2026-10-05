# rmdecl.py FILE SYM [SYM...] : delete one-line declarations (function prototypes / object externs,
# file or block scope, no definition) of each SYM from FILE. Prints every removed line.
# Byte IO, LF preserved.
import re, sys

path, syms = sys.argv[1], sys.argv[2:]
data = open(path, "rb").read().decode("utf-8")
lines = data.split("\n")
out = []
removed = 0
for i, ln in enumerate(lines):
    hit = None
    for s in syms:
        pat = (r"^\s*(extern\s+)?(const\s+)?[A-Za-z_][A-Za-z0-9_]*(\s+[A-Za-z_][A-Za-z0-9_]*)*[\s\*]+\(?\s*\*?\s*"
               + re.escape(s) + r"\s*(\([^;{}]*\)|\[[^\]]*\])*\s*\)?\s*(\([^;{}]*\))?\s*;\s*(/\*\s*extern\s*\*/)?\s*$")
        if re.match(pat, ln) and not re.match(r"^\s*(return|static)\b", ln):
            hit = s
            break
    if hit:
        print(f"{path}:{i+1}: -{ln.strip()}")
        removed += 1
        continue
    out.append(ln)
open(path, "wb").write("\n".join(out).encode("utf-8"))
print(f"{path}: removed {removed}")
