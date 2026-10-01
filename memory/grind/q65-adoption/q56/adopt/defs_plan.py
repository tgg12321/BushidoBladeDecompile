#!/usr/bin/env python3
"""defs_plan.py [tree]: Q65 step 4 planning. For every C object and every small-data address it touches
(0x800A30CC.. the gp reach; the COMMON block runs past the image end at 0x800A3800), from the object's own relocations (C and INCLUDE_ASM alike):
  kind = gp (R_MIPS_GPREL16: the file DEFINES it under the model) | ref (lui/%lo or la only);
  region = sdata (< 0x800A3308: initialized, .sdata) | static (< 0x800A3618: per-file .lcomm blocks) |
           common (>= 0x800A3618: the COMMON block);
the C names used, and each name's C declaration (file scope or block scope, from the preprocessed file).
Writes /tmp/q56/defs_plan.json."""
import json, os, re, subprocess, sys
T = sys.argv[1] if len(sys.argv) > 1 else "/tmp/q56/adopt tree"
LO, SD_END, ST_END, HI = 0x800A30CC, 0x800A3308, 0x800A3618, 0x800A30CC + 0x8000  # gp reach
CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ "
       "-D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")

def sh(c, cwd=None):
    return subprocess.run(c, shell=True, capture_output=True, text=True, cwd=cwd).stdout

addr = {}
for l in sh(f"mipsel-linux-gnu-nm '{T}/build/bb2.elf'").splitlines():
    p = l.split()
    if len(p) == 3:
        addr.setdefault(p[2], int(p[0], 16))

def region(a):
    return "sdata" if a < SD_END else ("static" if a < ST_END else "common")

plan = {}
for o in sorted(os.listdir(f"{T}/build/src")):
    if not o.endswith(".o"):
        continue
    f = o[:-2]
    rel = sh(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn '{T}/build/src/{o}'").splitlines()
    uses = {}
    for i, l in enumerate(rel):
        m = re.match(r"^\s*[0-9a-f]+:\s+R_MIPS_(HI16|LO16|GPREL16|32)\s+(\S+)$", l)
        if not m:
            continue
        nm = re.sub(r"\+0x[0-9a-f]+$", "", m.group(2))
        a = addr.get(nm)
        if a is None:
            mm = re.fullmatch(r"D_([0-9A-F]{8})", nm)
            a = int(mm.group(1), 16) if mm else None
        if a is None or not (LO <= a < HI):
            continue
        # REL: the offset of a gp access is in the instruction immediate
        off = 0
        if m.group(1) == "GPREL16":
            im = re.search(r"(-?\d+)\(gp\)", rel[i - 1])
            off = int(im.group(1)) if im else 0
        u = uses.setdefault(nm, {"addr": a, "gp": False, "gp_offsets": set(), "ref": False})
        if m.group(1) == "GPREL16":
            u["gp"] = True; u["gp_offsets"].add(off)
        else:
            u["ref"] = True
    if not uses:
        continue
    pre = sh(f"{CPP} src/{f}.c 2>/dev/null", cwd=T)
    for nm, u in uses.items():
        decls = re.findall(r"^\s*(extern\s+[^;{}()]*?\b%s\b(?:\s*\[[^\]]*\])*\s*;)" % re.escape(nm), pre, re.M)
        defs = re.findall(r"^\s*((?!extern)[A-Za-z_][^;{}()]*?\b%s\b(?:\s*\[[^\]]*\])*\s*(?:=[^;]*)?;)" % re.escape(nm), pre, re.M)
        u["decls"] = sorted(set(" ".join(d.split()) for d in decls))
        u["defs"] = sorted(set(" ".join(d.split()) for d in defs if not d.lstrip().startswith(("return", "if", "goto"))))
        u["region"] = region(u["addr"])
        u["gp_offsets"] = sorted(u["gp_offsets"])
    plan[f] = uses
json.dump(plan, open("/tmp/q56/defs_plan.json", "w"), indent=1, default=list)
# summary
for reg in ("sdata", "static", "common"):
    n = sum(1 for f in plan for u in plan[f].values() if u["region"] == reg and u["gp"])
    nod = [(f, nm) for f in plan for nm, u in plan[f].items() if u["region"] == reg and u["gp"] and not u["decls"] and not u["defs"]]
    print(f"{reg}: gp (file, name) pairs {n}; without a C declaration {len(nod)}: {nod[:12]}")
