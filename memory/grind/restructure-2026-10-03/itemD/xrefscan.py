"""xref channel: for every single-placement verbatim LIBSND/LIBSPU module, each REL26 (type 74)
reloc naming an XREF symbol -> EXE word at placement+offset -> jal/j target. Report targets
that land in the gap ranges, with the target's function start (nm)."""
import json, struct, sys
sys.path.insert(0, "tools/libscan")
import psyq_lib as P
GAPS = [(0x800841E0, 0x800848AC), (0x80084974, 0x80085064), (0x800858D0, 0x800859F0),
        (0x80085A40, 0x80085E4C), (0x80085FD8, 0x800863CC), (0x800863DC, 0x80086B38),
        (0x80086CF8, 0x80087E3C), (0x80089A48, 0x80089D10), (0x8008B488, 0x8008BA94)]
exe = open("disc/SLUS_006.63", "rb").read()
def word(va): return struct.unpack_from("<I", exe, va - 0x80010000 + 0x800)[0]
m = json.load(open("docs/naming/libscan/matches.json"))
def walk(x):
    if isinstance(x, dict):
        if "mod" in x and "lib" in x: yield x
        else:
            for v in x.values(): yield from walk(v)
    elif isinstance(x, list):
        for v in x: yield from walk(v)
libs = {}
for L in ("LIBSND", "LIBSPU"):
    b = open(f"tmp/libscan/psyq40/LIB/{L}.LIB", "rb").read()
    libs[L] = {n: P.parse_obj(o) for n, o in P.lib_modules(b)}
seen = {}
for r in walk(m):
    if r["lib"] not in libs or r.get("status") != "verbatim" or len(r.get("vaddrs", [])) != 1:
        continue
    base = int(r["vaddrs"][0], 16)
    o = libs[r["lib"]].get(r["mod"])
    if not o: continue
    tsec = [sid for sid, s in o.sections.items() if s["name"] == ".text"][0]
    for sec, t, off, e in o.relocs:
        if sec != tsec or t != 74 or not e.startswith("sym:"): continue
        w = word(base + off)
        tgt = 0x80000000 | ((w & 0x03FFFFFF) << 2)
        if any(a <= tgt < z for a, z in GAPS):
            seen.setdefault((tgt, e[4:]), []).append(f"{r['mod']}+0x{off:x}")
for (tgt, nm), who in sorted(seen.items()):
    print(f"{tgt:08X} {nm:24s} <- {', '.join(who)}")
