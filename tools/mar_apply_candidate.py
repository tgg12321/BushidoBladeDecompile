#!/usr/bin/env python3
"""Replace marionation_Exec's whole body in src/system.c with
tmp/mar_candidate.c (marionation-only, partition-based). No restore —
caller manages backup."""
import re
from pathlib import Path

src = Path("src/system.c")
body = src.read_text()
cand = Path("tmp/mar_candidate.c").read_text()
head, sep, tail = body.partition("s32 marionation_Exec(s32 a0, u8 *a1)\n{")
assert sep, "function start not found"
# find the matching closing brace of the function in tail
depth = 1
i = 0
while depth > 0:
    c = tail[i]
    if c == "{":
        depth += 1
    elif c == "}":
        depth -= 1
    i += 1
rest = tail[i:]
newbody = head + cand.rstrip("\n") + rest
src.write_text(newbody)
print("candidate applied; func length", i)
