#!/usr/bin/env python3
"""summ.py — quick summary of /tmp/q56/results.json."""
import json, re
R = json.load(open("/tmp/q56/results.json"))
rows, rt = R["rows"], R["row_tests"]
for n, info in rows.items():
    t = rt.get(n)
    if t is None:
        print(n, info["func"], "NOT TESTED tus=", info["tus"]); continue
    if not t.get("built"):
        print(n, info["func"], "BUILD FAIL", t.get("err", "")[-300:]); continue
    live = not t["obj_identical"]
    extra = ""
    if live:
        extra = f"link={t.get('linked')} bin_identical={t.get('bin_identical')} ref={t['funcdiff']['ref_lines']} var={t['funcdiff']['var_lines']}"
        if t.get("bin_identical"):
            extra += "  <<< OBJ DIFFERS BUT BIN SAME"
    flags = []
    if not info["in_sdata_funcs"]: flags.append("NOT-in-sdata_funcs")
    if info["include_asm"]: flags.append("INCLUDE_ASM")
    nos = [s for s, v in info["syms_in_sdata_syms"].items() if not v]
    if nos: flags.append("not-in-sdata_syms:" + ",".join(nos))
    print(n, info["func"], t["tu"], "LIVE" if live else "dead", extra, " ".join(flags))
print("func tests:")
for f, t in R["func_tests"].items():
    print(" ", f, "identical" if t.get("obj_identical") else "DIFF", t.get("bin_identical"))
print("sym tests:")
for n, d in R["sym_tests"].items():
    print(" ", n, rows[n]["func"], {s: ("dead" if v.get("obj_identical") else "LIVE") for s, v in d.items()})
