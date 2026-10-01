#!/usr/bin/env python3
"""mergecheck2.py: per-file contradiction check for proposed ORIGINAL files, each a list of our objects or
object slices ("obj@func" = from func to the object's end, "obj@@func" = from its start up to, not
including, func). A contradiction = a symbol reached gp-relative somewhere in the file and by a direct
lui/%lo load/store elsewhere in the same file (COMMON-offset sites, sym+N non-gp with base gp, are
reported separately: they are the Q62 signature, consistent). Uses the adopt clone's oracle objects."""
import re, subprocess, sys
B = "/tmp/q56/adopt tree/build/src"
LOADSTORE = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh", "sw", "swl", "swr"}
FILES = {
    "M3 text1a_c tail + text1a_c2 + text1a_b + sound + text1b": ["text1a_c@func_80044800", "text1a_c2", "text1a_b", "sound", "text1b"],
    "text1a_c head": ["text1a_c@@func_80044800"],
    "M1 code6cac_b2_pre + replay_camera_rob_back_loose2 + code6cac_b2_post": ["code6cac_b2_pre", "replay_camera_rob_back_loose2", "code6cac_b2_post"],
    "M2 code6cac_c2 + config": ["code6cac_c2", "config"],
    "M4 text1b_tu2 + text1b_b": ["text1b_tu2", "text1b_b"],
    "code6cac_b_tu2 up to func_800343F0": ["code6cac_b_tu2@@func_800343F0"],
    "func_800343F0 + code6cac_b_tu3": ["code6cac_b_tu2@func_800343F0", "code6cac_b_tu3"],
}

def scan(spec):
    o, lo, hi = spec, None, None
    if "@@" in spec: o, hi = spec.split("@@")
    elif "@" in spec: o, lo = spec.split("@")
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases '{B}/{o}.o'",
                         shell=True, capture_output=True, text=True).stdout.splitlines()
    gp, direct, lui, fn, last, on = {}, {}, {}, None, None, lo is None
    for ln in out:
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:$", ln)
        if m:
            fn = m.group(1)
            if lo and fn == lo: on = True
            if hi and fn == hi: on = False
            continue
        if not on: continue
        r = re.match(r"^\s*[0-9a-f]+:\s+(R_MIPS_\w+)\s+(\S+)$", ln)
        if r and last:
            kind, sym = r.group(1), re.sub(r"\+0x[0-9a-f]+$", "", r.group(2))
            mn, ops = last
            off = re.search(r"(-?\d+)\(", ops)
            off = int(off.group(1)) if off else 0
            if kind == "R_MIPS_GPREL16":
                gp.setdefault(sym, set()).add((fn, off))
            elif kind == "R_MIPS_HI16":
                lui[ops.split(",")[0]] = sym
            elif kind == "R_MIPS_LO16" and mn in LOADSTORE:
                b = re.search(r"\((\w+)\)", ops)
                if b and lui.get(b.group(1)) == sym:
                    direct.setdefault(sym, set()).add((fn, off))
            continue
        i = re.match(r"^\s*[0-9a-f]+:\s+(\S+)\s*(.*)$", ln)
        if i:
            last = (i.group(1), i.group(2))
            regs = re.findall(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", last[1])
            if regs and last[0] != "lui" and last[0] not in ("sb", "sh", "sw", "swl", "swr") and not last[0].startswith(("b", "j")):
                lui.pop(regs[0], None)
    return gp, direct

for name, parts in FILES.items():
    gp, direct = {}, {}
    for p in parts:
        g, d = scan(p)
        for s, v in g.items(): gp.setdefault(s, set()).update(v)
        for s, v in d.items(): direct.setdefault(s, set()).update(v)
    bad, common_sig = [], []
    for s in gp:
        if s in direct:
            base_gp = any(off == 0 for _, off in gp[s])
            nd = direct[s]
            if all(off != 0 for _, off in nd) and base_gp:
                common_sig.append((s, sorted(nd)[:3]))
            else:
                bad.append((s, sorted(f for f, _ in gp[s])[:3], sorted(f for f, _ in nd)[:3]))
    print(f"{name}: contradictions {bad or 'none'}; COMMON-offset signature {common_sig or 'none'}")
