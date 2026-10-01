import json, re
E = json.load(open("/tmp/q56/model_edits.json")); A = json.load(open("/tmp/q56/model_analysis.json"))
rows = []
for tu, syms in E["edits"].items():
    rows += [(tu, s) for s in syms]
for tu, ds in E.get("block_scope", {}).items():
    rows += [(tu, re.search(r"(\w+)(\[.*\])?;$", d).group(1)) for d in ds]
def cat(s):
    a = int(A["symbols"][s]["addr"], 16)
    if A["symbols"][s]["offset_gp"]:
        return "initialized+offset (.sdata)" if a < 0x800A3308 else "static (.lcomm)"
    if a < 0x800A3308:
        return "initialized-region (" + ("nonzero" if A["symbols"][s]["orig_zero4"] is False else "zero") + ")"
    return "bss tentative (COMMON)"
from collections import Counter
c = Counter(cat(s) for _, s in rows)
print("declarations:", len(rows), "files:", len({t for t, _ in rows}), "distinct symbols:", len({s for _, s in rows}))
for k, v in sorted(c.items()): print(" ", k, v)
print("asm-only (no declaration needed):", {k: v for k, v in E.get("asm_only", {}).items() if v})
