"""Relocate reviewed assembly-region grants for moved functions: change `file` only, and only when
the island hashes recomputed from the NEW file equal the reviewed hashes (text moved verbatim)."""
import json, sys
sys.path.insert(0, "/home/user/BushidoBladeDecompile")
from engine import completion, inlineasm
moved = json.load(open("/home/user/BushidoBladeDecompile/tmp/moved.json"))
g = json.loads(completion.REGIONS.read_text())
n = 0
for f, e in g["functions"].items():
    if f in moved and e["file"] != moved[f]:
        text = inlineasm._read_src_cached(moved[f])
        got = completion.region_hashes(text, f)
        assert got == e["sha256"], (f, "island text changed")
        e["file"] = moved[f]; n += 1
        print("relocated", f, "->", moved[f])
completion.REGIONS.write_text(json.dumps(g, indent=2, ensure_ascii=True) + "\n", newline="\n") if False else open(completion.REGIONS, "w", newline="\n").write(json.dumps(g, indent=2, ensure_ascii=True) + "\n")
print(n)
