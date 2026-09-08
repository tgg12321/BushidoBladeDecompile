#!/usr/bin/env python3
"""Data web (libscan-xref for GLOBALS): for every ACCEPTED verbatim module placement, resolve
each HI16/LO16 relocation pair that references an external symbol to the EXE address the
linker actually encoded. Pairing: a LO16 pairs with the most recent HI16 for the SAME BASE
SYMBOL (GCC reuses one lui for several lo offsets), and the base address is
(hi<<16 + signext(lo)) - offset_in_expr. A name is accepted only if every module agrees on
ONE base address. Output: tmp/near_audit/data_web.json + data_candidates.md with, per name,
our current symbols at the base and at every referenced offset, and reference counts."""
import csv, json, os, re, struct, sys, glob
from collections import defaultdict
REPO = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
sys.path.insert(0, os.path.join(REPO, "tmp", "closer"))
from psyq_lib import lib_modules, parse_obj  # noqa
exeb = open(os.path.join(REPO, "disc", "SLUS_006.63"), "rb").read()
taddr = struct.unpack_from("<I", exeb, 0x18)[0]
body = exeb[2048:]
def word(va): return struct.unpack_from("<I", body, va - taddr)[0]
LO, HI = 0x80078948, 0x8008D070

placements = {}
ACCEPT = {"CONFIRM", "FILL", "CONTRADICTED", "CONTRADICTED_ALIAS", "MODULE_LOCAL_STATIC", "IN_SPAN_NO_SYMBOL"}
for r in csv.DictReader(open(os.path.join(REPO, "docs/naming/libscan/rename_manifest.csv"), encoding="utf-8", errors="replace")):
    if r.get("classification") in ACCEPT and r.get("lib") != "LIBGS" and LO <= int(r["mod_start"], 16) < HI:
        placements[(r["lib"], r["module"])] = ("40J", int(r["mod_start"], 16))
placements[("LIBSND", "SSSTOP")] = ("40U", 0x80085270)
M = json.load(open(os.path.join(REPO, "tmp", "libscan", "matches.json")))
for r in M["results"]:
    if r.get("status") == "verbatim" and r["lib"] != "LIBGS" and r.get("words", 0) >= 24:
        vas = [int(v, 16) for v in r["vaddrs"] if LO <= int(v, 16) < HI]
        if len(vas) == 1 and (r["lib"], r["mod"]) not in placements:
            placements[(r["lib"], r["mod"])] = ("40J", vas[0])
LIBDIR = {"40J": os.path.join(REPO, "tmp", "libscan", "psyq40"), "40U": os.path.join(REPO, "tmp", "libsnd_hunt", "psyq40u")}

EXPR = re.compile(r"^\(?(?:(0x[0-9a-f]+)\+)?sym:(\w+)\)?$")
refs = defaultdict(lambda: defaultdict(set))    # name -> base addr -> {module tags}
offs = defaultdict(set)                          # name -> {offsets referenced}
libcache = {}
for (lib, mod), (ver, base) in sorted(placements.items()):
    p = os.path.join(LIBDIR[ver], lib + ".LIB")
    if p not in libcache:
        libcache[p] = dict(lib_modules(open(p, "rb").read()))
    if mod not in libcache[p]:
        continue
    o = parse_obj(libcache[p][mod])
    tsid = next((sid for sid, s in o.sections.items() if s["name"] == ".text"), None)
    if tsid is None:
        continue
    lasthi = {}   # base symbol -> hi16 value
    for rsec, t, roff, e in sorted([x for x in o.relocs if x[0] == tsid], key=lambda x: x[2]):
        e = re.sub(r"sym:(\d+)", lambda m: "sym:" + str(o.xrefs.get(int(m.group(1)), m.group(1))), e)
        m = EXPR.match(e)
        if not m:
            continue
        off = int(m.group(1), 16) if m.group(1) else 0
        name = m.group(2)
        w = word(base + roff)
        if t == 82:
            lasthi[name] = w & 0xFFFF
        elif t == 84 and name in lasthi:
            lo = w & 0xFFFF
            lo = lo - 0x10000 if lo >= 0x8000 else lo
            addr = ((lasthi[name] << 16) + lo - off) & 0xFFFFFFFF
            refs[name][addr].add(f"{lib}/{mod}@0x{base:08X}")
            offs[name].add(off)

# our symbols
SYMS = defaultdict(list)   # addr -> [(name, file)]
for f in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt"):
    for ln in open(os.path.join(REPO, f), encoding="utf-8", errors="replace"):
        mm = re.match(r"\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)", ln)
        if mm:
            SYMS[int(mm.group(2), 16)].append((mm.group(1), f))
def refcount(name):
    n = 0
    for pat in ("src/*.c", "include/*.h", "asm/funcs/*.s"):
        for fp in glob.glob(os.path.join(REPO, pat)):
            n += len(re.findall(r"\b" + re.escape(name) + r"\b", open(fp, encoding="utf-8", errors="replace").read()))
    return n

out = {}
lines = ["# Data candidates (libscan-xref for globals)", "",
         "| Sony global | base addr | modules | our names at base (refs) | referenced offsets -> our names |", "|---|---|---|---|---|"]
for name in sorted(refs):
    addrs = refs[name]
    if len(addrs) != 1:
        lines.append(f"| {name} | **AMBIGUOUS** {', '.join('0x%08X' % a for a in sorted(addrs))} | | | |")
        out[name] = dict(ambiguous={"0x%08X" % a: sorted(v) for a, v in addrs.items()})
        continue
    addr = next(iter(addrs))
    mods = sorted(addrs[addr])
    ours = [(n, f) for n, f in SYMS.get(addr, [])]
    oursx = ", ".join(f"{n}({refcount(n)})" for n, f in ours) or "?"
    offtxt = []
    for off in sorted(offs[name]):
        if off == 0:
            continue
        names = [n for n, f in SYMS.get(addr + off, [])]
        offtxt.append(f"+0x{off:X}: {', '.join(names) or '-'}")
    lines.append(f"| {name} | 0x{addr:08X} | {len(mods)} | {oursx} | {'; '.join(offtxt)[:200]} |")
    out[name] = dict(addr="0x%08X" % addr, modules=mods, ours=[n for n, f in ours],
                     offsets={"0x%X" % o: [n for n, f in SYMS.get(addr + o, [])] for o in sorted(offs[name]) if o})
open(os.path.join(REPO, "tmp/near_audit/data_candidates.md"), "w").write("\n".join(lines) + "\n")
json.dump(out, open(os.path.join(REPO, "tmp/near_audit/data_web.json"), "w"), indent=1)
print(len(placements), "placements;", len(refs), "names;", sum(1 for n in refs if len(refs[n]) != 1), "ambiguous")
