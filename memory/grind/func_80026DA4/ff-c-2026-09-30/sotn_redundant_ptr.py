"""Scan SOTN PS1-build C files for a local pointer assigned the SAME address
expression twice with no other write to it in between (textually), where the
second assignment is not inside a loop body that writes it. Candidates only:
every hit must be read by hand."""
import re, sys, os, glob
ROOT = "C:/Users/Trenton/Desktop/sotn-decomp"
ASSIGN = re.compile(r"^\s*(\w+)\s*=\s*(&[\w\.\[\]\->]+|\([\w\s\*]+\)\s*&?[\w\.\[\]\->]+|g_\w+|PLAYER)\s*;")
WRITE = lambda v: re.compile(r"(^|[^\w.>])" + re.escape(v) + r"\s*(=[^=]|\+=|-=|\+\+|--)|(\+\+|--)\s*" + re.escape(v) + r"\b")
hits = []
for path in glob.glob(ROOT + "/src/**/*.c", recursive=True):
    rel = os.path.relpath(path, ROOT).replace("\\", "/")
    if any(s in rel for s in ("/pc/", "/saturn/", "/maria/", "/psp/")):
        continue
    try:
        lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
    except OSError:
        continue
    # crude function segmentation: top-level '{' at col 0 after a line with '('
    start = None
    for i, l in enumerate(lines):
        if l.startswith("{") or (l.rstrip().endswith("{") and "(" in l and not l.startswith((" ", "\t", "#"))):
            start = i
        if l.startswith("}") and start is not None:
            body = lines[start:i + 1]
            seen = {}
            for k, bl in enumerate(body):
                m = ASSIGN.match(bl)
                if m:
                    v, rhs = m.group(1), m.group(2).replace(" ", "")
                    if v in seen and seen[v][1] == rhs:
                        j = seen[v][0]
                        between = body[j + 1:k]
                        w = WRITE(v)
                        if not any(w.search(b) for b in between) and not any(
                                re.search(r"\b(for|while|do)\b", b) and re.search(r"\b" + re.escape(v) + r"\b", b)
                                for b in between):
                            if any(re.search(r"\b" + re.escape(v) + r"\b", b) for b in between):
                                hits.append(f"{rel}:{start + j + 1}->{start + k + 1}  {v} = {rhs}")
                    seen[v] = (k, rhs)
                else:
                    pass
            start = None
print(len(hits))
print("\n".join(hits[:400]))
