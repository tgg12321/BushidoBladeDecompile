#!/usr/bin/env python3
"""Show the SCORED diff (engine masking) between the cheat-invisible sandbox
object and the canonical reference object, for one function.

Usage: pairdiff.py <stem> <func> [--unmasked]
  needs: tmp/sandbox/<func>/<stem>.o   (engine sandbox <func> --disable all)
         build/src/<stem>.o            (canonical reference)
"""
import sys, difflib
sys.path.insert(0, ".")
from engine import score

stem, func = sys.argv[1], sys.argv[2]
mask = "--unmasked" not in sys.argv

ours = score.normalized_insns(f"tmp/sandbox/{func}/{stem}.o", func, mask=mask)
tgt = score.normalized_insns(f"build/src/{stem}.o", func, mask=mask)

print(f"{func}: ours {len(ours)} insns, target {len(tgt)} insns "
      f"(mask={mask})")
sm = difflib.SequenceMatcher(a=ours, b=tgt, autojunk=False)
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == "equal":
        continue
    n += max(i2 - i1, j2 - j1)
    print(f"\n@@ ours[{i1}:{i2}] -> target[{j1}:{j2}]  ({tag})")
    for k in range(max(0, i1 - 3), i1):
        print(f"    {k:4d}  {ours[k]}")
    for k in range(i1, i2):
        print(f"  - {k:4d}  {ours[k]}")
    for k in range(j1, j2):
        print(f"  + {k:4d}  {tgt[k]}")
    for k in range(i2, min(len(ours), i2 + 3)):
        print(f"    {k:4d}  {ours[k]}")
print(f"\n=== {n} differing instructions ===")
