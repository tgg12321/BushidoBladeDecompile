#!/usr/bin/env python3
"""Assemble verbatim library regions in MOVOVL.EXE, resolve Sony symbol names
via XDEFs + local symbols, compute coverage and the game-code remainder.

Adapted from tmp/libscan/analyze.py (main-EXE version); queue cross-reference
dropped (the overlay has no decomp pipeline).

Writes: placements.json, symbols.txt, gaps.txt
"""
import struct, sys, os, json, re, glob, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from psyq_lib import lib_modules, parse_obj

M = json.load(open(os.path.join(HERE, "matches.json")))
taddr = int(M["taddr"], 16)
tsize = M["tsize"]
tend = taddr + tsize

objs = {}
for libpath in sorted(glob.glob(os.path.join(os.path.dirname(HERE), "libscan",
                                             "psyq40", "*.LIB"))):
    libname = os.path.basename(libpath)[:-4]
    for modname, objdata in lib_modules(open(libpath, "rb").read()):
        try:
            objs[(libname, modname)] = parse_obj(objdata)
        except Exception:
            pass


def text_sid(o):
    for sid, s in o.sections.items():
        if s["name"] == ".text" and len(s["bytes"]) >= 4:
            return sid
    return None


verb = [r for r in M["results"] if r["status"] == "verbatim"]

# Reject placements whose entire verified content is the generic empty-leaf
# pattern (`jr $ra` / `nop` only) — those match anywhere and carry no evidence.
GENERIC = {0x03E00008, 0x00000000}
rejected_generic = []


def is_generic(r):
    o = objs.get((r["lib"], r["mod"]))
    if not o:
        return False
    sid = text_sid(o)
    if sid is None:
        return False
    data = o.sections[sid]["bytes"]
    n = len(data) // 4
    ws = struct.unpack_from("<%dI" % n, data, 0)
    return all(x in GENERIC for x in ws)


keep = []
for r in verb:
    if is_generic(r):
        rejected_generic.append((r["lib"], r["mod"], r["words"], r["vaddrs"]))
    else:
        keep.append(r)
verb = keep

unique = [r for r in verb if len(r["vaddrs"]) == 1]
multi = [r for r in verb if len(r["vaddrs"]) > 1]

ivs = [(int(r["vaddrs"][0], 16),
        int(r["vaddrs"][0], 16) + r["words"] * 4, r["lib"], r["mod"], r["words"])
       for r in unique]


def merge(intervals, gap=0):
    out = []
    for t in sorted(intervals):
        a, b = t[0], t[1]
        if out and a <= out[-1][1] + gap:
            out[-1][1] = max(out[-1][1], b)
        else:
            out.append([a, b])
    return out


def overlaps_unique(a, b):
    return any(a < ub and ua < b for ua, ub, *_ in ivs)


accepted = list(ivs)
placed_multi, ambiguous, subsumed = [], [], []
for _pass in range(3):
    regions_now = merge(accepted, 0x40)
    for r in multi:
        good = []
        for va in r["vaddrs"]:
            a = int(va, 16); b = a + r["words"] * 4
            if any(ra - 0x10 <= a <= rb + 0x10 or ra - 0x10 <= b <= rb + 0x10
                   for ra, rb in regions_now):
                good.append((a, b))
        fresh = [(a, b) for a, b in good if not overlaps_unique(a, b)
                 and not any(a == pa and b == pb and r["lib"] == pl and r["mod"] == pm
                             for pa, pb, pl, pm, _w in accepted)]
        if fresh:
            for a, b in fresh:
                accepted.append((a, b, r["lib"], r["mod"], r["words"]))
                placed_multi.append((r["lib"], r["mod"], "0x%08X" % a, r["words"],
                                     len(r["vaddrs"])))
        elif _pass == 2:
            if good:
                subsumed.append((r["lib"], r["mod"], r["words"],
                                 ["0x%08X" % a for a, b in good]))
            elif not any(pm[0] == r["lib"] and pm[1] == r["mod"]
                         for pm in placed_multi):
                ambiguous.append((r["lib"], r["mod"], r["words"], r["vaddrs"]))

regions2 = merge(accepted)

# ---- symbol map ----
syms = {}
mod_of = []
for a, b, lib, mod, w in sorted(set(accepted)):
    mod_of.append((a, b, lib, mod))
    o = objs.get((lib, mod))
    if not o:
        continue
    sid = text_sid(o)
    for nm, sec, off in o.xdefs:
        if sec == sid and a + off < b:
            syms.setdefault(a + off, set()).add(nm)
    for nm, sec, off in o.locals:
        if sec == sid and a + off < b:
            syms.setdefault(a + off, set()).add(nm + " (static)")

with open(os.path.join(HERE, "symbols.txt"), "w") as f:
    f.write("# MOVOVL.EXE PsyQ library symbol map (verbatim-matched modules)\n")
    f.write("# vaddr           lib/module            symbol(s)\n")
    for a in sorted(syms):
        owner = next(("%s/%s" % (l, m) for aa, bb, l, m in mod_of if aa <= a < bb), "?")
        f.write("0x%08X %-22s %s\n" % (a, owner, " / ".join(sorted(syms[a]))))

# ---- coverage ----
lib_bytes = sum(b - a for a, b in regions2)
per_lib = collections.Counter()
per_lib_mods = collections.Counter()
for a, b, lib, mod in mod_of:
    per_lib[lib] += b - a
    per_lib_mods[lib] += 1

# gaps = game code (plus rodata/data trailing the text image)
gaps = []
cur = taddr
for a, b in regions2:
    if a > cur:
        gaps.append((cur, a))
    cur = max(cur, b)
if cur < tend:
    gaps.append((cur, tend))

with open(os.path.join(HERE, "gaps.txt"), "w") as f:
    f.write("# Non-library spans in MOVOVL.EXE text image (game code + rodata/data)\n")
    for a, b in gaps:
        f.write("0x%08X..0x%08X  %7d bytes\n" % (a, b, b - a))

json.dump(dict(taddr="0x%08X" % taddr, tsize=tsize,
               regions=[["0x%08X" % a, "0x%08X" % b, b - a] for a, b in regions2],
               modules=[["0x%08X" % a, "0x%08X" % b, lib, mod] for a, b, lib, mod in mod_of],
               placed_multi=placed_multi, subsumed=subsumed, ambiguous=ambiguous,
               gaps=[["0x%08X" % a, "0x%08X" % b, b - a] for a, b in gaps],
               rejected_generic=rejected_generic,
               lib_bytes=lib_bytes,
               per_lib={k: v for k, v in per_lib.most_common()},
               per_lib_modules={k: v for k, v in per_lib_mods.most_common()},
               symbol_count=len(syms)),
          open(os.path.join(HERE, "placements.json"), "w"), indent=1)

print("MOVOVL text: 0x%08X..0x%08X (%d bytes)" % (taddr, tend, tsize))
print("merged library regions:")
for a, b in regions2:
    print("  0x%08X..0x%08X (%d bytes)" % (a, b, b - a))
print("library bytes: %d  (%.1f%% of text image)" % (lib_bytes, 100.0 * lib_bytes / tsize))
print("per-lib bytes:")
for k, v in per_lib.most_common():
    print("  %-9s %7d  (%d modules)" % (k, v, per_lib_mods[k]))
print("unique modules:", len(ivs), " multi placements:", len(placed_multi),
      " subsumed:", len(subsumed), " ambiguous:", len(ambiguous))
for x in ambiguous:
    print("  AMBIG", x)
print("rejected generic-stub placements:", len(rejected_generic))
for x in rejected_generic:
    print("  GENERIC", x)
print("symbols:", len(syms))
# text-span coverage (text = first library byte's enclosing code span .. last)
tx_lo, tx_hi = 0x801D919C, regions2[-1][1]
print("text span 0x%08X..0x%08X = %d bytes; library %d (%.1f%%), game %d (%.1f%%)"
      % (tx_lo, tx_hi, tx_hi - tx_lo, lib_bytes,
         100.0 * lib_bytes / (tx_hi - tx_lo),
         (tx_hi - tx_lo) - lib_bytes,
         100.0 * ((tx_hi - tx_lo) - lib_bytes) / (tx_hi - tx_lo)))
print("gaps (non-library):")
for a, b in gaps:
    print("  0x%08X..0x%08X %7d" % (a, b, b - a))
