#!/usr/bin/env python3
"""model_analyze.py — Q56 follow-up (A): per-file gp model, static analysis of the REFERENCE build.

For every C object of the reference (oracle) build:
  G(O)   = symbols the object reaches gp-relative (R_MIPS_GPREL16), with addends;
  N(O,S) = direct non-gp accesses (lui %hi(S) ; load/store %lo(S)(same reg)) to a symbol S in G(O).
Per symbol: objects that gp it, whether any gp access has a nonzero addend (offset), the address, and
whether the ORIGINAL EXE bytes over [addr, addr+4) are zero (initialized vs zero data; bss is zero).
Writes /tmp/q56/model_analysis.json and prints a summary.
"""
import json, os, re, subprocess
Q = "/tmp/q56"; T = Q + "/tree"; REF = Q + "/refobj"
EXE = open(T + "/disc/SLUS_006.63", "rb").read()

def sh(c):
    return subprocess.run(c, shell=True, capture_output=True, text=True).stdout

addr = {}
for ln in sh(f"mipsel-linux-gnu-nm {T}/build/bb2.elf").splitlines():
    p = ln.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))

def orig_bytes(a, n=4):
    off = a - 0x80010000 + 0x800
    if 0 <= off < len(EXE):
        return EXE[off:off + n]
    return b"\0" * n  # beyond the image: bss

LOADSTORE = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh", "sw", "swl", "swr"}
INS = re.compile(r"^\s*([0-9a-f]+):\s+(\S+)\s*(.*)$")
REL = re.compile(r"^\s*([0-9a-f]+):\s+(R_MIPS_\w+)\s+(\S+)$")
FN = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")

def parse(o):
    out = sh(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases {o}").splitlines()
    insns = []  # (func, mnem, ops, [relocs])
    fn = None
    for ln in out:
        m = FN.match(ln)
        if m:
            fn = m.group(1); continue
        r = REL.match(ln)
        if r and insns:
            sym = r.group(3)
            add = 0
            mm = re.match(r"(.+?)\+0x([0-9a-f]+)$", sym)
            if mm:
                sym, add = mm.group(1), int(mm.group(2), 16)
            insns[-1][3].append((r.group(2), sym, add)); continue
        i = INS.match(ln)
        if i and fn:
            insns.append([fn, i.group(2), i.group(3), []])
    return insns

res = {"objects": {}, "symbols": {}}
for o in sorted(os.listdir(REF)):
    tu = o[:-2]
    insns = parse(f"{REF}/{o}")
    gp = {}   # sym -> list of (func, mnem, addend)
    lui = {}  # reg -> (sym, addend)
    direct = []  # (func, mnem, sym, addend)
    for fn, mn, ops, rels in insns:
        regs = re.findall(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", ops)
        imm = re.search(r"(-?\d+)\((\w+)\)", ops)
        for kind, sym, add in rels:
            if kind == "R_MIPS_GPREL16":
                # REL relocations: the addend is the instruction's own offset field
                gp.setdefault(sym, []).append((fn, mn, add + (int(imm.group(1)) if imm else 0)))
        if mn == "lui" and rels and rels[0][0] == "R_MIPS_HI16":
            lui[regs[0]] = (rels[0][1], rels[0][2]); continue
        lo = [x for x in rels if x[0] == "R_MIPS_LO16"]
        if lo and mn in LOADSTORE:
            base = re.search(r"\((\w+)\)", ops)
            b = base.group(1) if base else None
            if b in lui and lui[b][0] == lo[0][1]:
                direct.append((fn, mn, lo[0][1], int(base_off.group(1)) if (base_off := re.search(r"(-?\d+)\(", ops)) else 0))
        # any write to a register kills its lui tracking (dest = first operand for non-stores)
        if regs and mn not in ("sb", "sh", "sw", "swl", "swr") and not mn.startswith("b") and mn not in ("jr", "j", "jal", "jalr"):
            lui.pop(regs[0], None)
    rec = {"gp": {s: v for s, v in gp.items()}, "direct_nongp_to_gp_syms": [d for d in direct if d[2] in gp]}
    res["objects"][tu] = rec
    for s, v in gp.items():
        S = res["symbols"].setdefault(s, {"objects": [], "offset_gp": False})
        S["objects"].append(tu)
        if any(a for (_, _, a) in v):
            S["offset_gp"] = True
for s, S in res["symbols"].items():
    a = addr.get(s)
    S["addr"] = hex(a) if a is not None else None
    S["orig_zero4"] = (orig_bytes(a) == b"\0\0\0\0") if a is not None else None
json.dump(res, open(Q + "/model_analysis.json", "w"), indent=1)

syms = res["symbols"]
print("objects with gp:", sum(1 for r in res["objects"].values() if r["gp"]), "/", len(res["objects"]))
print("gp symbols:", len(syms))
print("  in >1 object:", sorted((s, v["objects"]) for s, v in syms.items() if len(v["objects"]) > 1))
print("  offset gp:", sorted((s, v["objects"]) for s, v in syms.items() if v["offset_gp"]))
print("  nonzero orig bytes:", sorted((s, v["addr"], v["objects"]) for s, v in syms.items() if v["orig_zero4"] is False))
print("same-object gp + direct non-gp (contradiction candidates):")
for tu, r in res["objects"].items():
    for d in r["direct_nongp_to_gp_syms"]:
        print("  ", tu, d, "gp users:", sorted(set(f for f, _, _ in r["gp"][d[2]])))
