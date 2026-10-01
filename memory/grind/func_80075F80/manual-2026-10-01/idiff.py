#!/usr/bin/env python3
"""Instruction diff of <func> between tmp/j759/work/<stem>/<stem>.o (last harness build) and build/src/<stem>.o.
usage: idiff.py <stem> <func>"""
import difflib
import sys
sys.path.insert(0, ".")
from engine import score  # noqa: E402

stem, func = sys.argv[1], sys.argv[2]
ours = score.normalized_insns(f"tmp/j759/work/{stem}/{stem}.o", func)
tgt = score.normalized_insns(f"build/src/{stem}.o", func)
sm = difflib.SequenceMatcher(None, tgt, ours, autojunk=False)
for op, a1, a2, b1, b2 in sm.get_opcodes():
    if op == "equal":
        continue
    print(f"@@ {op} target[{a1}:{a2}] ours[{b1}:{b2}]")
    for x in tgt[a1:a2]:
        print("  T", x)
    for x in ours[b1:b2]:
        print("  O", x)
