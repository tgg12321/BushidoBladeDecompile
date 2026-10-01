#!/usr/bin/env python3
"""explicit_exclusions.py <tree>: per-file-gp-model.md's explicit-relocation clause, for the ledger.
For every function listed in inline_asm_canonical.txt (canonical hand-written asm), in the C file that now holds
it, list each DIRECT lui/%lo load/store in its shipped asm (asm/funcs/<func>.s) to a symbol that the same file
reaches gp-relative elsewhere (i.e. a symbol the file defines under the model). Those accesses are excluded from
E1 / E2 / the split test. Output: function, symbol, asm line (address + text)."""
import os, re, subprocess, sys
T = sys.argv[1]
canon = [l.split()[0] for l in open(f"{T}/inline_asm_canonical.txt") if l.strip() and not l.startswith("#")]
where = {}
for f in os.listdir(f"{T}/src"):
    if f.endswith(".c"):
        t = open(f"{T}/src/{f}", errors="replace").read()
        for m in re.finditer(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)|glabel\s+(\w+)|^[A-Za-z_][\w\s\*]*?\b(\w+)\s*\([^;{]*\)\s*\{', t, re.M):
            where.setdefault(m.group(1) or m.group(2) or m.group(3), f[:-2])
addr = {}
for l in subprocess.run(f"mipsel-linux-gnu-nm '{T}/build/bb2.elf'", shell=True, capture_output=True, text=True).stdout.splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))


def res(s):
    if s in addr:
        return addr[s]
    m = re.fullmatch(r"D_([0-9A-F]{8})", s)
    return int(m.group(1), 16) if m else None


# gp-reached addresses per object (the file's definitions under the model), from the build's objects
gp_by_obj = {}
for o in os.listdir(f"{T}/build/src"):
    if o.endswith(".o"):
        out = subprocess.run(f"mipsel-linux-gnu-objdump -dr '{T}/build/src/{o}'", shell=True, capture_output=True, text=True).stdout
        gp_by_obj[o[:-2]] = {res(re.sub(r"\+0x[0-9a-f]+$", "", m.group(1))) for m in re.finditer(r"R_MIPS_GPREL16\s+(\S+)", out)} - {None}
L = re.compile(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/\s+(\S+)\s+(.*)")
rows = []
for fn in canon:
    f = where.get(fn)
    p = f"{T}/asm/funcs/{fn}.s"
    if not f or not os.path.exists(p):
        continue
    lui = {}
    for ln in open(p, errors="replace"):
        m = L.search(ln)
        if not m:
            continue
        ia, mn, ops = m.groups()
        h = re.search(r"%hi\((\w+)\)", ops)
        if mn == "lui" and h:
            lui[ops.split(",")[0].strip()] = h.group(1)
            continue
        lo = re.search(r"%lo\((\w+)\)\((\$\w+)\)", ops)
        if lo and mn != "addiu" and lui.get(lo.group(2)) == lo.group(1):
            a = res(lo.group(1))
            if a is not None and a in gp_by_obj.get(f, set()):
                rows.append((f, fn, lo.group(1), ia, f"{mn} {ops.strip()}"))
print("| file | function | symbol | address | asm line |")
print("|---|---|---|---|---|")
for r in rows:
    print("| %s | %s | %s | 0x%s | `%s` |" % r)
print(f"\n{len(rows)} excluded access(es)")
