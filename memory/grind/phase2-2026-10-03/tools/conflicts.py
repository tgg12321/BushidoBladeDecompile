"""conflicts.py: Phase 2's declaration worklist after restructure step 5 (tab-separated, one symbol per
line: class, symbol, spellings with their sites). Classes:
  sony-func     a Sony library function declared with more than one spelling (incl. its definition)
  header-local  a symbol a header declares that some TU also declares with a different spelling
  game-local    a game symbol whose file-scope declarations differ between TUs (hoist.py)
  game-proto    a game function whose declarations differ from its C definition (hoist.py)
  note          a single named item (type-name conflicts, implicit calls a prototype would change)
Needs tmp/p2/census.json (census.py) and tmp/p2/hoist_rejects.txt (hoist.py --rejects)."""
import collections, glob, json, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dedupe import proto_key, ws

C = json.load(open("tmp/p2/census.json"))
defs = dict(C["defs"])
for f in glob.glob("src/**/*.c", recursive=True):
    for m in re.finditer(r"BIOS_[ABC]_FUNCTION\((\w+)", open(f, encoding="utf-8").read()):
        defs[m.group(1)] = f[4:-2].replace("\\", "/")
out = []


def norm(t):
    t = re.sub(r"\s+", " ", t.replace("extern ", "")).strip()
    return t


def strip_names(t):
    m = re.match(r"(.*?\()(.*)(\);?)$", t)
    if not m:
        return t
    ps = []
    for p in m.group(2).split(","):
        p = p.strip()
        ids = re.findall(r"\w+", p)
        if p in ("void", "...", "") or "(" in p or len(ids) < 2:
            ps.append(p)
        else:
            ps.append(re.sub(r"\s*\b\w+$", "", p))
    return m.group(1) + ", ".join(ps) + m.group(3)


# functions: every declaration / definition spelling per name (aux-info)
sp = collections.defaultdict(lambda: collections.defaultdict(set))
for r in C["funcs"]:
    if r["kind"] not in ("NC", "OC", "NF", "OF"):
        continue
    t = norm(r["text"] if r["kind"] in ("NC", "OC") else strip_names(r["text"]))
    t = t.replace(" " + r["name"] + " (", " @(")
    site = f"{r['file'].replace('src/main/', '')}:{r['line']}" + ("(def)" if r["kind"] in ("NF", "OF") else "")
    sp[r["name"]][t].add(site)
for n in sorted(sp):
    if len(sp[n]) < 2 or n not in defs:
        continue
    if defs[n].startswith("main/psxsdk/"):
        cls = "sony-func"
    elif any(s.startswith("include/") for v in sp[n].values() for s in v):
        cls = "header-local"
    else:
        continue  # game-only conflicts come from hoist.py below
    out.append((cls, n, " | ".join(f"{k} <- {','.join(sorted(v)[:6])}" for k, v in sorted(sp[n].items()))))
# objects declared in a header and differently in some TU
osp = collections.defaultdict(lambda: collections.defaultdict(set))
for o in C["objs"]:
    osp[o["name"]][re.sub(r"\s+", " ", o["text"])].add(f"{o['file'].replace('src/main/', '')}:{o['line']}")
for n in sorted(osp):
    if len(osp[n]) > 1 and any(s.startswith("include/") or s.endswith(".h") for v in osp[n].values() for s in v):
        out.append(("header-local", n, " | ".join(f"{k} <- {','.join(sorted(v)[:6])}" for k, v in sorted(osp[n].items()))))
sony_rows = {r[1] for r in out if r[0] == "sony-func"}
dup = 0
for l in open("tmp/p2/hoist_rejects.txt", encoding="utf-8"):
    r, n, s = l.rstrip("\n").split("\t")
    if n in sony_rows:   # hoist.py rejects on spelling before its Sony test; the sony-func row covers it
        dup += 1
        continue
    out.append(("game-local" if r.startswith("file-scope") else "game-proto", n, s))
print(dup, "hoist rejects dropped: Sony functions already listed as sony-func rows")
NOTES = [
    ("note", "func_8005C2A8", "main/309CC.c (func_80040594) calls it implicitly with an int second argument; the (s32 *, s16, s32) prototype changes 309CC's bytes (sll/sra), so its declarations stay local"),
    ("note", "SpuGetVoiceVolume", "libsnd/ut_vvol.c calls it implicitly twice (voice, s16 *, s16 *); the definition libspu/s_gvv.c is (s32, u16 *, u16 *) and libspu.h gives no prototype"),
    ("note", "D_800A3899/D_800A38AB", "Q21 per-file views: 175A4.c/17AFC.c declare single u8s D_800A3898/99/AA/AB, main/9F9C.c declares D_800A3898[2]/D_800A38AA[2]"),
    ("note", "D_800963EE", "second name for D_800963EC[0].length_sectors (main/35000.c x2, main/32D04.c); 35000.c also indexes it as (u8 *)&D_800963EE + a0 * 4"),
    ("note", "Vec3", "type name: 16-byte {vx,vy,vz,pad} at file scope in 3AB48.c/64FD8.c vs 12-byte block-scope Vec3 twice in main/9F9C.c"),
    ("note", "EnvA", "type: two layouts (s32 * / s8 * vs s32 members) in 51268.c/63D2C.c vs 64FD8.c"),
    ("note", "va_list", "type: char * (2B344.c, libspu/spu.c) vs void * (libc2/sprintf.c)"),
    ("note", "InterruptCallback", "declared (void) in libetc/intr*.c and cast at every call (SOTN: (int, void *))"),
    ("note", "Vec4i32/SVec4i16/Obj80106A78Mat/Unk80101DF0Rot/Unk80101DF0Mat", "game.h local names for VECTOR/SVECTOR/MATRIX layouts (libgte.h)"),
]
out += NOTES
with open("tmp/p2/phase2_conflicts.tsv", "w", encoding="utf-8", newline="\n") as fh:
    fh.write("# class\tsymbol\tspellings <- sites  (phase2 refresh, memory/grind/phase2-2026-10-03/tools/conflicts.py)\n")
    for r in out:
        fh.write("\t".join(r) + "\n")
print(collections.Counter(r[0] for r in out))
