#!/usr/bin/env python3
"""Callee web (libscan-xref): for every ACCEPTED bit-verbatim PsyQ module placement in the
main library block (tmp/libscan/libsyms.json = the 2026-08-07 accepted set, minus LIBGS; plus
the 4.0U-only SSSTOP placement proven 2026-07-10), resolve each external REL26 (jal)
relocation: the module's XREF names the callee, the EXE word encodes where the linker put
it. That address IS that symbol by construction of linking — byte-level evidence independent
of the callee's own body. Also resolves HI16/LO16 XREF pairs to data addresses (Sony's names
for globals) for a later data wave.
Outputs tmp/near_audit/web.md / web.json (functions) and web_data.md (globals)."""
import json, os, re, struct, sys
from collections import defaultdict
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # tools/libscan/x.py -> repo root
sys.path.insert(0, os.path.join(REPO, "tmp", "closer"))
from psyq_lib import lib_modules, parse_obj  # noqa

exeb = open(os.path.join(REPO, "disc", "SLUS_006.63"), "rb").read()
taddr = struct.unpack_from("<I", exeb, 0x18)[0]
body = exeb[2048:]
def word(va): return struct.unpack_from("<I", body, va - taddr)[0]
LO, HI = 0x80078948, 0x8008D070

# accepted placements = the 2026-08-07 libscan manifest rows (docs/naming/libscan/
# rename_manifest.csv) whose classification accepted the module at mod_start; ties/islands
# (AMBIGUOUS, REJECTED_*) are excluded so a same-shape stub cannot inject a false caller.
import csv
placements = {}
ACCEPT = {"CONFIRM", "FILL", "CONTRADICTED", "CONTRADICTED_ALIAS", "MODULE_LOCAL_STATIC", "IN_SPAN_NO_SYMBOL"}
for r in csv.DictReader(open(os.path.join(REPO, "docs/naming/libscan/rename_manifest.csv"), encoding="utf-8", errors="replace")):
    if r.get("classification") not in ACCEPT or r.get("lib") == "LIBGS":
        continue
    ms = int(r["mod_start"], 16)
    if not (LO <= ms < HI):
        continue
    placements[(r["lib"], r["module"])] = ("40J", ms)
placements[("LIBSND", "SSSTOP")] = ("40U", 0x80085270)   # proven 2026-07-10 (4.0 Jun-06 build)
# plus verbatim 4.0J placements the manifest left unclassified because their XDEF sits
# mid-function (boundary_fixes.md, e.g. SSCALL): single placement, main block, >= 24 words
M = json.load(open(os.path.join(REPO, "tmp", "libscan", "matches.json")))
for r in M["results"]:
    if r.get("status") != "verbatim" or r["lib"] == "LIBGS" or r.get("words", 0) < 24:
        continue
    vas = [int(v, 16) for v in r["vaddrs"] if LO <= int(v, 16) < HI]
    if len(vas) == 1 and (r["lib"], r["mod"]) not in placements:
        placements[(r["lib"], r["mod"])] = ("40J", vas[0])
LIBDIR = {"40J": os.path.join(REPO, "tmp", "libscan", "psyq40"),
          "40U": os.path.join(REPO, "tmp", "libsnd_hunt", "psyq40u")}

MAPDEF = re.compile(r"^\s+0x0*([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$")
ours = defaultdict(list)
for ln in open(os.path.join(REPO, "build", "bb2.map"), encoding="utf-8", errors="replace"):
    m = MAPDEF.match(ln)
    if m:
        ours[int(m.group(1), 16)].append(m.group(2))
DATA = defaultdict(list)
for f in ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt"):
    for ln in open(os.path.join(REPO, f), encoding="utf-8", errors="replace"):
        m = re.match(r"\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)", ln)
        if m:
            DATA[int(m.group(2), 16)].append(m.group(1))
func_starts = sorted(a for a, ns in ours.items())

web = defaultdict(lambda: defaultdict(set))
dweb = defaultdict(lambda: defaultdict(set))
libcache = {}
for (lib, mod), (ver, base) in sorted(placements.items()):
    p = os.path.join(LIBDIR[ver], lib + ".LIB")
    if not os.path.exists(p):
        continue
    if p not in libcache:
        libcache[p] = dict(lib_modules(open(p, "rb").read()))
    if mod not in libcache[p]:
        continue
    o = parse_obj(libcache[p][mod])
    tsid = next((sid for sid, s in o.sections.items() if s["name"] == ".text"), None)
    if tsid is None:
        continue
    hi = {}
    for rsec, t, roff, e in o.relocs:
        if rsec != tsid:
            continue
        e = re.sub(r"sym:(\d+)", lambda m: "sym:" + str(o.xrefs.get(int(m.group(1)), m.group(1))), e)
        a = base + roff
        w = word(a)
        tag = f"{lib}/{mod}@0x{base:08X}"
        if t == 74 and e.startswith("sym:"):
            if (w >> 26) in (2, 3):
                tgt = ((w & 0x3FFFFFF) << 2) | (a & 0xF0000000)
                web[e[4:]][tgt].add(tag)
        elif t == 82:
            hi[e] = w & 0xFFFF
        elif t == 84 and e in hi:
            lo = w & 0xFFFF
            lo = lo - 0x10000 if lo >= 0x8000 else lo
            addr = ((hi[e] << 16) + lo) & 0xFFFFFFFF
            m = re.match(r"^(?:\((0x[0-9a-f]+)\+)?sym:(\w+)\)?$", e)
            if m:
                off = int(m.group(1), 16) if m.group(1) else 0
                dweb[m.group(2)][addr - off].add(tag)
            elif e.startswith("sym:"):
                dweb[e[4:]][addr].add(tag)

lines = ["# libscan-xref callee web (accepted verbatim placements, main block)", "",
         "| Sony callee | EXE target | fn start? | our name(s) | called from |", "|---|---|---|---|---|"]
out = {}
for name in sorted(web):
    for tgt in sorted(web[name]):
        ns = ours.get(tgt, []) + [n for n in DATA.get(tgt, []) if not re.match(r"^(func|D)_[0-9A-Fa-f]{8}$", n) and n not in ours.get(tgt, [])]
        isstart = "yes" if tgt in ours else "NO (interior of %s)" % (ours[[f for f in func_starts if f <= tgt][-1]][0] if any(f <= tgt for f in func_starts) else "?")
        flag = "" if name in ns else " **<- differs**"
        lines.append(f"| {name} | 0x{tgt:08X} | {isstart} | {', '.join(ns) or '?'}{flag} | {'; '.join(sorted(web[name][tgt]))[:160]} |")
        out.setdefault(name, {})["0x%08X" % tgt] = dict(ours=ns, fn_start=tgt in ours, callers=sorted(web[name][tgt]))
open(os.path.join(REPO, "tmp", "near_audit", "web.md"), "w").write("\n".join(lines) + "\n")
json.dump(out, open(os.path.join(REPO, "tmp", "near_audit", "web.json"), "w"), indent=1)

dl = ["# libscan-xref data web (Sony global name -> EXE address, from accepted verbatim placements)", "",
      "| Sony global | EXE addr | our name(s) | referenced from |", "|---|---|---|---|"]
dout = {}
for name in sorted(dweb):
    for addr in sorted(dweb[name]):
        ns = DATA.get(addr, [])
        flag = "" if name in ns else " **<- differs**"
        dl.append(f"| {name} | 0x{addr:08X} | {', '.join(ns) or '?'}{flag} | {'; '.join(sorted(dweb[name][addr]))[:120]} |")
        dout.setdefault(name, {})["0x%08X" % addr] = dict(ours=ns, refs=sorted(dweb[name][addr]))
open(os.path.join(REPO, "tmp", "near_audit", "web_data.md"), "w").write("\n".join(dl) + "\n")
json.dump(dout, open(os.path.join(REPO, "tmp", "near_audit", "web_data.json"), "w"), indent=1)
print(len(placements), "placements;", len(web), "callee names;", len(dweb), "data names")
