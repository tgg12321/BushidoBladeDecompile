#!/usr/bin/env python3
"""Regenerate the s5 work5 per-value ablation variants from memory/grind/func_80058580/candidate.c
(run from the repo root; writes tmp/func_80058580/ab_*.c, then score them with tools/sandbox_sweep.ps1).
Values of work5: c2 = case-2 `work1 >> 27` test, adj = the (0x1000 - lv) * 625 >> 10 - 400 offset,
et = the `work5 = et` copy feeding the et < 5 / switch tests. Each listed value gets its own local."""
import itertools, os
s = open("memory/grind/func_80058580/candidate.c").read()
C2 = [("work5 = work1 >> 27;", "top = work1 >> 27;"), ("if (work5) {", "if (top) {")]
ADJ = [("work5 = (((0x1000 - lv) * 625) >> 10) - 400;\n                                work1 += work5 + CPU_S16(0x40A);\n                                hi += work5 + CPU_S16(0x40A);",
        "adj = (((0x1000 - lv) * 625) >> 10) - 400;\n                                work1 += adj + CPU_S16(0x40A);\n                                hi += adj + CPU_S16(0x40A);")]
ETC = [("work5 = et;\n                            if (work5 < 5) {", "kind = et;\n                            if (kind < 5) {"), ("switch (work5) {", "switch (kind) {")]
names = {"c2": (C2, "top"), "adj": (ADJ, "adj"), "et": (ETC, "kind")}
os.makedirs("tmp/func_80058580", exist_ok=True)
for r in (1, 2, 3):
    for combo in itertools.combinations(names, r):
        t = s
        for n in combo:
            for a, b in names[n][0]:
                assert a in t, (n, a)
                t = t.replace(a, b)
        t = t.replace("    s32 hi;\n", "    s32 hi;\n" + "".join(f"    s32 {names[n][1]};\n" for n in combo))
        open("tmp/func_80058580/ab_" + "_".join(combo) + ".c", "w", newline="\n").write(t)
