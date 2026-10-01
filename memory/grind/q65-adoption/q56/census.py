#!/usr/bin/env python3
"""census.py — Q56 step-2 prep: shipped-byte evidence for every sdata_exclude.txt (func, sym).

For each (func, sym) it reads the SHIPPED bytes (asm/funcs/*.s, the split original EXE) and records:
  - func's own references to sym (address, mnemonic, form: gp_rel / lo-load-store / lo-addiu(la));
  - whether func uses gp at all (any symbol);
  - every OTHER function that reaches sym's address gp-relatively, with the C object (our TU)
    it is built into — and in particular same-TU gp users (the per-file ASPSX consistency test:
    Sony ASPSX 2.34 gp's a symbol iff the file DEFINES it, research-common-gp.md s0, so within one
    file a symbol's direct base access is gp in every function or in none).
Symbols are compared by resolved ADDRESS (reference ELF symtab), so aliases/renames are handled.
Output: /tmp/q56/census.json
"""
import json, os, re, subprocess
T = "/tmp/q56/tree"
REF = "/tmp/q56/refobj"

def sh(c):
    return subprocess.run(c, shell=True, capture_output=True, text=True).stdout

addr = {}
for ln in sh(f"mipsel-linux-gnu-nm {T}/build/bb2.elf").splitlines():
    p = ln.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))

by_addr = {}
for ln in sh(f"mipsel-linux-gnu-nm {T}/build/bb2.elf").splitlines():
    p = ln.split()
    if len(p) == 3 and p[1] in "TtA":
        by_addr.setdefault(int(p[0], 16), []).append(p[2])
func_tu = {}
for o in sorted(os.listdir(REF)):
    for ln in sh(f"mipsel-linux-gnu-nm --defined-only {REF}/{o}").splitlines():
        p = ln.split()
        if len(p) == 3 and p[1] in "Tt":
            func_tu.setdefault(p[2], o[:-2])

REL = re.compile(r"%(gp_rel|hi|lo)\(([A-Za-z_.$][\w.$]*)\s*(?:([+-])\s*(0x[0-9A-Fa-f]+|\d+))?\)")
LINE = re.compile(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/\s+(\S+)\s+(.*)")

# per function: list of (insn_addr, mnem, kind, target_addr, text)
refs = {}
unresolved = set()
for fn in os.listdir(T + "/asm/funcs"):
    if not fn.endswith(".s"):
        continue
    f = fn[:-2]
    out = []
    for ln in open(f"{T}/asm/funcs/{fn}", errors="replace"):
        m = LINE.search(ln)
        if not m:
            continue
        ia, mn, ops = m.groups()
        for r in REL.finditer(ops):
            kind, sym, sgn, off = r.groups()
            base = addr.get(sym)
            if base is None:
                mm = re.fullmatch(r"(?:D|jtbl)_([0-9A-Fa-f]{8})", sym)
                mp = re.fullmatch(r"(\w+?)_plus_(0x[0-9A-Fa-f]+)", sym)
                if mm:
                    base = int(mm.group(1), 16)
                elif mp and mp.group(1) in addr:
                    base = addr[mp.group(1)] + int(mp.group(2), 16)
            if base is None:
                unresolved.add(sym); continue
            a = base + ((int(off, 0) * (1 if sgn == "+" else -1)) if off else 0)
            out.append((int(ia, 16), mn, kind, a, ops.strip()))
    # key by the C-object name defined at this function's address (asm files are named by address
    # for renamed functions, e.g. func_80046B44.s == game_Init)
    key = f
    if f not in func_tu:
        fa = None
        for ln2 in open(f"{T}/asm/funcs/{fn}", errors="replace"):
            m2 = LINE.search(ln2)
            if m2:
                fa = int(m2.group(1), 16); break
        for nm in by_addr.get(fa, ()):
            if nm in func_tu:
                key = nm; break
    refs[key] = out

rows = []
for i, l in enumerate(open(T + "/sdata_exclude.txt").read().split("\n"), 1):
    s = l.strip()
    if s and not s.startswith("#") and ":" in s:
        f, syms = s.split(":", 1)
        rows.append((i, f.strip(), [x.strip() for x in syms.split(",") if x.strip()]))

# gp users per address
gp_users = {}
for f, rr in refs.items():
    for (ia, mn, kind, a, txt) in rr:
        if kind == "gp_rel":
            gp_users.setdefault(a, set()).add(f)

res = {}
for (n, f, syms) in rows:
    tu = func_tu.get(f)
    fr = refs.get(f, [])
    uses_gp = sum(1 for x in fr if x[2] == "gp_rel")
    for s in syms:
        a = addr.get(s)
        if a is None:
            mm = re.fullmatch(r"D_([0-9A-Fa-f]{8})", s)
            mp = re.fullmatch(r"(\w+?)_plus_(0x[0-9A-Fa-f]+)", s)
            if mm:
                a = int(mm.group(1), 16)
            elif mp and mp.group(1) in addr:
                a = addr[mp.group(1)] + int(mp.group(2), 16)
        rec = {"line": n, "func": f, "sym": s, "addr": (hex(a) if a is not None else None), "tu": tu,
               "func_gp_refs_any_sym": uses_gp}
        if a is None:
            rec["note"] = "symbol not in reference ELF"
            res[f"{n}:{s}"] = rec; continue
        own = [(hex(ia), mn, kind, txt) for (ia, mn, kind, ta, txt) in fr if ta == a and kind != "hi"]
        rec["own_refs"] = own
        rec["own_gp"] = sum(1 for x in own if x[2] == "gp_rel")
        rec["own_lo_access"] = sum(1 for x in own if x[2] == "lo" and x[1] != "addiu")
        rec["own_la"] = sum(1 for x in own if x[2] == "lo" and x[1] == "addiu")
        users = sorted(gp_users.get(a, ()))
        rec["gp_users_total"] = len(users)
        rec["gp_users"] = [f"{u} ({func_tu.get(u, '?')})" for u in users]
        rec["gp_user_tus"] = sorted(set(func_tu.get(u, "?asm") for u in users))
        rec["same_tu_gp_users"] = [u for u in users if tu and func_tu.get(u) == tu and u != f]
        gtus = set(func_tu.get(u) for u in users)
        rec["gp_tu_nongp_direct"] = sorted(g for g, rr in refs.items() if func_tu.get(g) in gtus and g not in users
                                          and any(ta == a and k == "lo" and mn != "addiu" for (ia, mn, k, ta, tx) in rr))
        res[f"{n}:{s}"] = rec
json.dump(res, open("/tmp/q56/census.json", "w"), indent=1)
print("records", len(res), "unresolved syms seen in asm:", len(unresolved))
