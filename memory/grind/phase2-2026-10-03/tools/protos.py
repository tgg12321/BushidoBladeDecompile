#!/usr/bin/env python3
"""protos.py OUTDIR : census of untyped pointer parameters (run in WSL, repo root, after census.py).
For every function parameter declared u8 * / s8 * / s16 * / u16 * / s32 * / u32 * / void * / char * or
a pointer-sized integer (s32 / u32 / int) -- C definitions (their own header) and asm-only functions
(their most common C prototype, tmp/p2/census.json) -- the struct each C call site passes
(cmodel.resolve on the argument), how many call sites cast a typed pointer down to fit
(`(u8 *)rec`), and for C definitions the casts the body applies to the parameter (struct casts
`(S *)p`, raw-offset sites whose root is p). Writes OUTDIR/protos.tsv:
  func def param idx decl callers resolved caller_structs callers_downcast body_struct_casts
  body_offset_sites verdict
verdict: typed-callers (every resolving call site passes the same struct S, at least one does) |
body-casts (the body casts the parameter to S, no caller contradicts) | conflict (two structs) |
mixed-scratchpad (S at some call sites, scratchpad workspace at others) | scratchpad | none."""
import collections, json, os, re, sys
sys.path.insert(0, ".")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmodel import units, calls, resolve, allstructs, strip_casts
from refs import split_params, drop_name

OUT = sys.argv[1]
UNTYPED = re.compile(r"^(const )?(u8|s8|s16|u16|s32|u32|void|char|unsigned char|int|unsigned int) ?\*?$")
sites = collections.defaultdict(int)
for l in open(f"{OUT}/casts.tsv", encoding="utf-8"):
    if l.startswith("#"):
        continue
    r = l.rstrip("\n").split("\t")
    if r[8].endswith("(param)"):
        sites[(r[0], r[2], r[7])] += 1

rows = []
seen = set()


def caller_info(name, i):
    n = k = down = 0
    ev = collections.Counter()
    for cu, cf, args in calls.get(name, []):
        if i >= len(args):
            continue
        n += 1
        a = args[i]
        if len(a) > 3 and a[0] == "(" and a[2] in ("*",) or (len(a) > 4 and a[0] == "(" and a[3] == "*"):
            down += 1
        s, how, _, _ = resolve(cu, cf, a)
        if s:
            k += 1
            ev[s] += 1
        elif how.startswith("scratchpad"):
            ev["@scratchpad"] += 1
    return n, k, ev, down


for u in units.values():
    T = u.T
    for f in u.funcs:
        if f["name"] in seen:
            continue
        seen.add(f["name"])
        for i, p in enumerate(f["params"]):
            v = f["vars"].get(p, [("int", 0, False)])[-1]
            decl = v[0] + (" " + "*" * v[1] if v[1] else "")
            if not UNTYPED.match(decl) or v[1] > 1:
                continue
            n, k, ev, down = caller_info(f["name"], i)
            q, e = f["body"]
            bc = collections.Counter()
            for j in range(q, e - 3):
                if T.t[j] == "(" and T.t[j + 1] in allstructs and T.t[j + 2] == "*" and T.t[j + 3] == ")" \
                        and j + 4 < e and T.t[j + 4] == p:
                    bc[T.t[j + 1]] += 1
            ns = sites.get((u.path, f["name"], p), 0)
            rows.append((f["name"], f"{u.path}:{f['start']}", p, i, decl, n, k, ev, down, bc, ns))

# asm-only functions: their C prototypes
C = json.load(open("tmp/p2/census.json"))
protos = collections.defaultdict(collections.Counter)
for r in C["funcs"]:
    if r["kind"] in ("NC", "OC") and r["name"] not in seen:
        protos[r["name"]][r["text"]] += 1
for name, spell in protos.items():
    if name not in C["defs"] or not calls.get(name):
        continue
    text = spell.most_common(1)[0][0]
    m = re.match(r"^(?:extern\s+)?(.*?)\b" + re.escape(name) + r"\s*\((.*)\);?\s*$", " ".join(text.split()))
    if not m:
        continue
    for i, p in enumerate(split_params(m.group(2))):
        decl = drop_name(p).replace(" *", " *").strip()
        if not UNTYPED.match(decl):
            continue
        n, k, ev, down = caller_info(name, i)
        rows.append((name, "asm", "", i, decl, n, k, ev, down, collections.Counter(), 0))

out, nint = [], 0
for name, d, p, i, decl, n, k, ev, down, bc, ns in rows:
    structs = {s for s in ev if s != "@scratchpad"}
    if len(structs | set(bc)) > 1:
        verdict = "conflict"
    elif structs and ev.get("@scratchpad"):
        verdict = "mixed-scratchpad"
    elif structs and k >= 1:
        verdict = "typed-callers"
    elif bc:
        verdict = "body-casts"
    elif ev.get("@scratchpad"):
        verdict = "scratchpad"
    else:
        verdict = "none"
    if verdict == "none" and "*" not in decl:
        nint += 1
        continue   # a plain integer parameter with no struct evidence
    out.append([name, d, p, str(i), decl, str(n), str(k), ",".join(f"{s}:{c}" for s, c in ev.most_common()) or "-",
                str(down), ",".join(f"{s}:{c}" for s, c in bc.most_common()) or "-", str(ns), verdict])
out.sort(key=lambda r: ({"typed-callers": 0, "body-casts": 1, "conflict": 2, "mixed-scratchpad": 3, "scratchpad": 4, "none": 5}[r[11]],
                        r[1], r[0], int(r[3])))
with open(f"{OUT}/protos.tsv", "w", encoding="utf-8", newline="\n") as fh:
    fh.write("# func\tdef\tparam\tidx\tdecl\tcallers\tresolved\tcaller_structs\tcallers_downcast\tbody_struct_casts"
             "\tbody_offset_sites\tverdict   (memory/grind/phase2-2026-10-03/tools/protos.py)\n")
    for r in out:
        fh.write("\t".join(r) + "\n")
print(len(out) + nint, "candidate parameters,", nint, "integer ones without evidence dropped;", collections.Counter(r[11] for r in out))
print("typed-callers by struct:", collections.Counter(r[7].split(":")[0] for r in out if r[11] == "typed-callers"))
