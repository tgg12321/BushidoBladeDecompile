#!/usr/bin/env python3
# Recompute tools/canonical_asm_regions.json's sha256 lists for the three Q115 bodies with
# engine/completion.py region_hashes (run from the repo root after w2b13.py apply).
import json, sys

sys.path.insert(0, ".")
from engine import completion  # noqa: E402

P = "tools/canonical_asm_regions.json"
FUNCS = ("func_8002CD58", "func_8002DAD0", "func_8002D780")
raw = open(P, encoding="utf-8").read()
data = json.loads(raw)
text = open("src/main/17AFC.c", encoding="utf-8").read()
for f in FUNCS:
    ent = data["functions"][f]
    assert ent["file"] == "main/17AFC", ent
    new = completion.region_hashes(text, f)
    old = ent["sha256"]
    assert len(new) == len(old), (f, len(old), len(new))
    print("%s: %d regions, %d changed" % (f, len(new), sum(a != b for a, b in zip(old, new))))
    ent["sha256"] = new
out = json.dumps(data, indent=2) + ("\n" if raw.endswith("\n") else "")
if json.dumps(json.loads(raw), indent=2) + ("\n" if raw.endswith("\n") else "") != raw:
    sys.exit("the file is not in json.dumps(indent=2) form; refusing to rewrite it")
open(P, "w", encoding="utf-8", newline="\n").write(out)
