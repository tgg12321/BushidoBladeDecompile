#!/usr/bin/env python3
"""Scan MOVOVL.EXE (BB2 FMV/MDEC overlay) for verbatim-linked PsyQ library modules.

Adapted from tmp/libscan/scan.py (main-EXE scanner). Same method:
  parse each .LIB module -> masked .text word sequence (reloc-affected fields
  masked out) -> anchor-search the overlay text -> verify 100% of unmasked bits.

Adds: XDEF symbol extraction with overlay virtual addresses, and coverage
accounting over the overlay text region.

Usage: python3 scan_movovl.py [libdir] [outname]
"""
import struct, sys, os, json, glob, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from psyq_lib import lib_modules, parse_obj

EXE = os.path.join(REPO, "disc", "STR", "MOVOVL.EXE")
exeb = open(EXE, "rb").read()
pc, gp, taddr, tsize = struct.unpack_from("<IIII", exeb, 0x10)
body = exeb[2048:2048 + tsize] if tsize else exeb[2048:]
NW = len(body) // 4
exe_words = list(struct.unpack_from("<%dI" % NW, body, 0))
sys.stderr.write("MOVOVL: pc=0x%08X gp=0x%08X taddr=0x%08X tsize=0x%X words=%d\n"
                 % (pc, gp, taddr, tsize, NW))


def mask_for(t):
    if t in (82, 84):   # HI16 / LO16
        return 0xFFFF0000
    if t == 74:         # REL26
        return 0xFC000000
    if t == 16:         # FULL32
        return 0
    return None         # unknown -> fully mask, note it


def module_text(o):
    for sid, s in o.sections.items():
        if s["name"] == ".text" and len(s["bytes"]) >= 4:
            return sid, s["bytes"]
    return None, None


results = []
lib_errors = []

libdir = sys.argv[1] if len(sys.argv) > 1 else "psyq40"
outname = sys.argv[2] if len(sys.argv) > 2 else "matches.json"
srcdir = os.path.join(os.path.dirname(HERE), "libscan", libdir)

for libpath in sorted(glob.glob(os.path.join(srcdir, "*.LIB"))):
    libname = os.path.basename(libpath)[:-4]
    try:
        mods = list(lib_modules(open(libpath, "rb").read()))
    except Exception as e:
        lib_errors.append((libname, "container: %s" % e)); continue
    for modname, objdata in mods:
        try:
            o = parse_obj(objdata)
        except Exception as e:
            lib_errors.append(("%s/%s" % (libname, modname), str(e))); continue
        sid, data = module_text(o)
        if sid is None:
            results.append(dict(lib=libname, mod=modname, status="no-text"))
            continue
        n = len(data) // 4
        words = list(struct.unpack_from("<%dI" % n, data, 0))
        masks = [0xFFFFFFFF] * n
        unknown_relocs = set()
        for rsec, t, roff, e in o.relocs:
            if rsec != sid:
                continue
            wi = roff // 4
            if wi >= n:
                continue
            m = mask_for(t)
            if m is None:
                unknown_relocs.add(t); m = 0
            masks[wi] &= m
        runs = []
        i = 0
        while i < n:
            if masks[i] == 0xFFFFFFFF:
                j = i
                while j < n and masks[j] == 0xFFFFFFFF:
                    j += 1
                runs.append((j - i, i))
                i = j
            else:
                i += 1
        runs.sort(reverse=True)
        anchors = [r for r in runs if r[0] >= 4][:3]
        if not anchors:
            anchors = [r for r in runs if r[0] >= 2][:3]
        rec = dict(lib=libname, mod=modname, words=n,
                   masked_words=sum(1 for m in masks if m != 0xFFFFFFFF),
                   unmasked_words=sum(1 for m in masks if m == 0xFFFFFFFF),
                   unknown_reloc_types=sorted(unknown_relocs))
        if not anchors:
            rec["status"] = "unanchorable"
            results.append(rec); continue
        cands = set()
        for rl, ri in anchors:
            pat = struct.pack("<%dI" % rl, *words[ri:ri + rl])
            start = 0
            while True:
                p = body.find(pat, start)
                if p < 0:
                    break
                if p % 4 == 0:
                    base = p // 4 - ri
                    if 0 <= base and base + n <= NW:
                        cands.add(base)
                start = p + 4
        best = None
        matches = []
        for base in sorted(cands):
            mism = 0
            for i in range(n):
                if (exe_words[base + i] ^ words[i]) & masks[i]:
                    mism += 1
            if mism == 0:
                matches.append(base)
            if best is None or mism < best[1]:
                best = (base, mism)
        # XDEFs that live in this module's .text
        xd = [(nm, off) for nm, sec, off in o.xdefs if sec == sid]
        xd.sort(key=lambda x: x[1])
        rec["xdefs"] = xd
        rec["other_xdefs"] = [(nm, o.sections.get(sec, {}).get("name", str(sec)), off)
                              for nm, sec, off in o.xdefs if sec != sid]
        if matches:
            rec["status"] = "verbatim"
            rec["bases"] = matches
            rec["vaddrs"] = ["0x%08X" % (taddr + b * 4) for b in matches]
        elif best is not None and best[1] <= max(1, n // 20):
            rec["status"] = "near"
            rec["bases"] = [best[0]]
            rec["vaddr"] = "0x%08X" % (taddr + best[0] * 4)
            rec["mismatch_words"] = best[1]
        else:
            rec["status"] = "no-match"
            if best is not None:
                rec["best_mismatch"] = best[1]
                rec["best_vaddr"] = "0x%08X" % (taddr + best[0] * 4)
        results.append(rec)

json.dump(dict(exe="MOVOVL.EXE", taddr="0x%08X" % taddr, tsize=tsize,
               entry="0x%08X" % pc, results=results, lib_errors=lib_errors),
          open(os.path.join(HERE, outname), "w"), indent=1)

for r in results:
    if r["status"] in ("verbatim", "near"):
        print(r["lib"], r["mod"], r["status"],
              r.get("vaddrs") or r.get("vaddr"), r.get("words"),
              "mism=%s" % r.get("mismatch_words", 0),
              "unk=%s" % r.get("unknown_reloc_types") if r.get("unknown_reloc_types") else "")
print("--- lib errors:", len(lib_errors))
for e in lib_errors[:20]:
    print(e)
print(collections.Counter(r["status"] for r in results))
