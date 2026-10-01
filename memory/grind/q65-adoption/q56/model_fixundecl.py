#!/usr/bin/env python3
"""model_fixundecl.py: drop POC declarations of symbols the C never declares (they are reached gp-relative
only by INCLUDE_ASM / whole-body asm text in that object). Reads build.log errors; records them."""
import json, re
M = "/tmp/q56/model"
log = open(M + "/build.log").read()
bad = {}
for f, s in re.findall(r"src/(\w+)\.c:\d+: `(\w+)' undeclared here", log):
    bad.setdefault(f, set()).add(s)
E = json.load(open("/tmp/q56/model_edits.json"))
E.setdefault("asm_only", {})
for f, ss in bad.items():
    p = f"{M}/src/{f}.c"
    t = open(p).read()
    for s in ss:
        t = t.replace(f"__typeof__({s}) {s};\n", "")
        E["edits"][f].remove(s)
        E["asm_only"].setdefault(f, []).append(s)
    open(p, "w", newline="\n").write(t)
json.dump(E, open("/tmp/q56/model_edits.json", "w"), indent=1)
print({f: sorted(s) for f, s in bad.items()})
