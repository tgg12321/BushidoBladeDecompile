#!/usr/bin/env python3
"""model_blockdecl.py: symbols the C declares only at BLOCK scope (`extern T S;` inside a function) get
their file-scope definition from that declaration's own type text (`T S;`)."""
import json, re
M = "/tmp/q56/model"
E = json.load(open("/tmp/q56/model_edits.json"))
E.setdefault("block_scope", {})
for tu, syms in E.get("asm_only", {}).items():
    p = f"{M}/src/{tu}.c"; t = open(p).read()
    for s in list(syms):
        m = re.search(r"^\s+extern\s+([^;]*?)\b%s\b([^;]*);" % re.escape(s), t, re.M)
        if not m:
            continue
        d = f"{m.group(1).strip()} {s}{m.group(2)};"
        t += d + "  /* Q56 POC: from the block-scope extern */\n"
        E["block_scope"].setdefault(tu, []).append(d)
        syms.remove(s)
    open(p, "w", newline="\n").write(t)
json.dump(E, open("/tmp/q56/model_edits.json", "w"), indent=1)
print(E["block_scope"]); print("still asm-only:", E["asm_only"])
