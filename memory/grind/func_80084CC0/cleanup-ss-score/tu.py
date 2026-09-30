"""laneF: build a whole modified copy of a TU with ALL cheat rules disabled and
compare EVERY function's exact bytes + masked score to build/src/<stem>.o.

usage: python3 tmp/laneF/tu.py <stem> <src-copy.c> [func ...]
Prints only functions whose exact bytes differ (plus a summary line).
"""
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import cheats, pipeline, score  # noqa: E402

stem, src = sys.argv[1], sys.argv[2]
only = sys.argv[3:]
wd = Path("tmp/laneF/wd") / stem
ov = cheats.empty_overrides(str(wd / "cfg"))
ov["src_override"] = src
out_o = str(wd / f"{stem}.o")
pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
ref = f"build/src/{stem}.o"
tbl = score._o_func_table(ref)
names = only or sorted(tbl, key=lambda n: tbl[n][0])
built_tbl = score._o_func_table(out_o)
bad = 0
for n in names:
    if n not in built_tbl:
        print(f"MISSING {n}")
        bad += 1
        continue
    same = score.func_byte_signature(out_o, n) == score.func_byte_signature(ref, n)
    if not same:
        r = score.score_func(out_o, ref, n)
        print(f"DIFF {n}: score {r['score']} insns {r['build_insns']}/{r['target_insns']}")
        bad += 1
print(f"checked {len(names)} funcs, {bad} differ")

import os
if os.environ.get("DIFF"):
    for n in os.environ["DIFF"].split(","):
        d = score.insn_diff(out_o, ref, n)
        print(f"== {n}")
        for h in d["hunks"]:
            if h["class"] == "not-scored":
                continue
            print(f"-- {h['tag']} {h['class']} t@{h['target_at']} b@{h['build_at']}")
            for k in ("target", "built"):
                for line in h.get(k, []):
                    print(f"   {k[0]} {line}")
