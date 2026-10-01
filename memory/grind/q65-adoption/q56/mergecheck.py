#!/usr/bin/env python3
"""mergecheck.py: the initialized symbols gp'd from several objects force those objects into one original
file. Does merging them create a per-file contradiction (a symbol gp in one part, reached by a direct
non-gp load/store in another)?  Uses the oracle objects."""
import re, subprocess
GROUPS = [["code6cac_b2_pre", "replay_camera_rob_back_loose2", "code6cac_b2_post"], ["code6cac_c2", "config"]]
LOADSTORE = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh", "sw", "swl", "swr"}

def scan(o):
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases /tmp/q56/refobj/{o}.o",
                         shell=True, capture_output=True, text=True).stdout.splitlines()
    gp, direct, lui, fn, last = {}, {}, {}, None, None
    for ln in out:
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:$", ln)
        if m:
            fn = m.group(1); continue
        r = re.match(r"^\s*[0-9a-f]+:\s+(R_MIPS_\w+)\s+(\S+)$", ln)
        if r and last:
            kind, sym = r.group(1), re.sub(r"\+0x[0-9a-f]+$", "", r.group(2))
            mn, ops = last
            if kind == "R_MIPS_GPREL16":
                gp.setdefault(sym, set()).add(fn)
            elif kind == "R_MIPS_HI16":
                lui[ops.split(",")[0]] = sym
            elif kind == "R_MIPS_LO16" and mn in LOADSTORE:
                b = re.search(r"\((\w+)\)", ops)
                if b and lui.get(b.group(1)) == sym:
                    direct.setdefault(sym, set()).add(fn)
            continue
        i = re.match(r"^\s*[0-9a-f]+:\s+(\S+)\s*(.*)$", ln)
        if i:
            last = (i.group(1), i.group(2))
            regs = re.findall(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", last[1])
            if regs and last[0] != "lui" and last[0] not in ("sb", "sh", "sw", "swl", "swr") and not last[0].startswith(("b", "j")):
                lui.pop(regs[0], None)
    return gp, direct

for g in GROUPS:
    data = {o: scan(o) for o in g}
    bad = []
    for a in g:
        for s in data[a][0]:
            for b in g:
                if b != a and s in data[b][1]:
                    bad.append((s, f"gp in {a}", f"direct non-gp in {b}: {sorted(data[b][1][s])}"))
    print(" + ".join(g), "->", bad or "no contradiction")
