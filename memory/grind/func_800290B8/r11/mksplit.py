#!/usr/bin/env python3
"""Derive the one-variable-per-value spelling (split.c) from final.c: declarations/identifiers only."""
from pathlib import Path
d = Path(__file__).resolve().parent
s = (d / "final.c").read_text()
decl = s[s.index("    s32 temp;  /*"):s.index("    *(LeafPos *)(scr + 0x84)")]
t = s.replace(decl, "    s32 temp;\n\n")
a = "    for (i = 1; i < 4; i++) {\n        temp = i / 2;\n        temp2 = i & 1;\n        n = idx * 4 + temp * 2 + temp2;\n"
assert t.count(a) == 1
t = t.replace(a, "    for (i = 1; i < 4; i++) {\n        s32 row;\n        s32 col;\n\n        row = i / 2;\n        col = i & 1;\n        n = idx * 4 + row * 2 + col;\n")
b = "    for (temp = 0; rec->type != 0; temp++, rec++) {\n        if (rec->used != 0) continue;\n"
assert t.count(b) == 1
t = t.replace(b, "    for (temp = 0; rec->type != 0; temp++, rec++) {\n        s32 temp2;\n\n        if (rec->used != 0) continue;\n")
(d / "split.c").write_text(t, newline="\n") if False else open(d / "split.c", "w", newline="\n").write(t)
