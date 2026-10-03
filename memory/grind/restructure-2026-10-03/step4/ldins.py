#!/usr/bin/env python3
"""ldins.py (--after|--before) ANCHOR_ID SECTIONS ID... : insert `build/src/<ID>.o(<sec>);` lines into
bb2.ld directly after/before ANCHOR_ID's line in each of SECTIONS (comma list), in the given order.
ldins.py --drop ID [SECTIONS]: remove ID's lines (all sections, or the listed ones).
ldins.py --move ID SEC --after|--before ANCHOR: move ID's SEC line next to ANCHOR's SEC line."""
import re, sys

p = "bb2.ld"
s = open(p, encoding="utf-8", newline="").read()


def line_re(tid, sec):
    return re.compile(r"(?m)^([ \t]*)build/src/" + re.escape(tid) + r"\.o\(" + re.escape(sec) + r"\);\n")


a = sys.argv[1:]
if a[0] == "--drop":
    secs = a[2].split(",") if len(a) > 2 else None
    n = 0
    for m in list(re.finditer(r"(?m)^[ \t]*build/src/" + re.escape(a[1]) + r"\.o\((\.\w+)\);\n", s))[::-1]:
        if secs is None or m.group(1) in secs:
            s = s[:m.start()] + s[m.end():]
            n += 1
    print("dropped", n)
elif a[0] == "--move":
    tid, sec, where, anchor = a[1], a[2], a[3], a[4]
    m = line_re(tid, sec).search(s); assert m, (tid, sec)
    line = m.group(0); s = s[:m.start()] + s[m.end():]
    m = line_re(anchor, sec).search(s); assert m, (anchor, sec)
    at = m.end() if where == "--after" else m.start()
    s = s[:at] + line + s[at:]
    print("moved", tid, sec, where, anchor)
else:
    where, anchor, secs, ids = a[0], a[1], a[2].split(","), a[3:]
    for sec in secs:
        m = line_re(anchor, sec).search(s); assert m, (anchor, sec)
        ins = "".join(f"{m.group(1)}build/src/{t}.o({sec});\n" for t in ids)
        at = m.end() if where == "--after" else m.start()
        s = s[:at] + ins + s[at:]
    print("inserted", len(ids), "ids x", secs)
open(p, "w", encoding="utf-8", newline="\n").write(s)
