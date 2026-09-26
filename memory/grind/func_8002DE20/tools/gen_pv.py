"""Derive one-variable-per-value spellings of a Ruling 11 reuse body.

usage: python3 gen_pv.py <reuse.c> <out.c> [--only a3,b7,...]
Values are labelled by TEST number T = 1..12 (the T-th `(cross_a ^ ...)` test
in source order): value aT is the `cross_a = ...;` write whose next test is
test T, read only by that test (bT likewise for cross_b). Without --only,
EVERY value of the shared cross_a / cross_b gets its own fresh local
(`cross_aT` / `cross_bT`) declared at the innermost scope enclosing its single
write; with --only, just the listed values are split out (the Ruling 11 (D)(4)
ablation). Only declarations and identifiers change; the caller checks that
with a token diff.
"""
import argparse
import re

ap = argparse.ArgumentParser()
ap.add_argument("inp")
ap.add_argument("out")
ap.add_argument("--only", default="")
a = ap.parse_args()
lines = open(a.inp, newline="").read().split("\n")
only = set(x for x in a.only.split(",") if x)

tests = [i for i, ln in enumerate(lines) if "(cross_a ^ " in ln]
assert len(tests) == 12, len(tests)

func_decls = []
block_decls = {}
for i, ln in enumerate(list(lines)):
    m = re.match(r"^( *)cross_([ab]) = ", ln)
    if not m:
        continue
    v = m.group(2)
    t = next(k for k, ti in enumerate(tests, 1) if ti > i)
    tag = f"{v}{t}"
    if only and tag not in only:
        continue
    name = f"cross_{v}{t}"
    ti = tests[t - 1]
    lines[i] = lines[i].replace(f"cross_{v} = ", f"{name} = ", 1)
    old = "(cross_a ^ " if v == "a" else "^ cross_b)"
    assert old in lines[ti], (tag, lines[ti])
    lines[ti] = lines[ti].replace(old, old.replace(f"cross_{v}", name), 1)
    ind = len(m.group(1))
    if ind == 4:
        func_decls.append(name)
    else:
        j = i - 1
        while not (lines[j].endswith("{") and
                   len(lines[j]) - len(lines[j].lstrip()) == ind - 4):
            j -= 1
        block_decls.setdefault(j, []).append((ind, name))

for j in sorted(block_decls, reverse=True):
    lines[j + 1:j + 1] = [" " * ind + f"s32 {name};" for ind, name in block_decls[j]]
if func_decls:
    k = next(i for i, ln in enumerate(lines) if ln.strip() == "s32 cross_b;")
    lines[k + 1:k + 1] = ["    s32 " + n + ";" for n in sorted(func_decls, key=lambda n: (n[6], int(n[7:])))]
body = "\n".join(lines)
code = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
for v in "ab":
    if len(re.findall(rf"\bcross_{v}\b", code)) == 1:
        body = body.replace(f"    s32 cross_{v};\n", "", 1)
if not re.search(r"\bs32 cross_a;", body) and not re.search(r"\bs32 cross_b;", body):
    body = re.sub(r"    /\* cross_a / cross_b (?:each )?hold.*?\*/\n", "", body, count=1, flags=re.S)
open(a.out, "w", newline="\n").write(body)
print(a.out, "split:", sorted(only) if only else "all")
