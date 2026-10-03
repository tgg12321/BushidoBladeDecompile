"""sony.py: Sony function declarations (definition in src/main/psxsdk/**) per spelling and file."""
import json, re, glob, collections, sys
C = json.load(open("tmp/s5/census.json"))
defs = dict(C["defs"])
for f in glob.glob("src/**/*.c", recursive=True):
    for m in re.finditer(r"BIOS_[ABC]_FUNCTION\((\w+)", open(f, encoding="utf-8").read()):
        defs[m.group(1)] = f[4:-2].replace("\\", "/")
sony = {n for n, t in defs.items() if t.startswith("main/psxsdk/")}
rows = collections.defaultdict(dict)
for r in C["funcs"]:
    if r["kind"] in ("NC", "OC") and r["name"] in sony:
        rows[r["name"]][(r["file"], r["line"])] = r["text"]
lib = lambda n: defs[n].split("/")[2]
only = sys.argv[1] if len(sys.argv) > 1 else None
stat = collections.Counter()
for n in sorted(rows, key=lambda n: (lib(n), n)):
    if only and lib(n) != only: continue
    sp = collections.defaultdict(list)
    for (f, l), t in rows[n].items():
        sp[t.replace(" " + n + " (", " @ (")].append(f"{f.replace('src/main/','').replace('include/','I:')}:{l}")
    stat["identical" if len(sp) == 1 else "differ"] += 1
    print(f"{lib(n):7s} {n}  [{defs[n].split('/')[-1]}]  {'OK' if len(sp)==1 else 'DIFFER'}")
    for t, fl in sp.items():
        print(f"        {t}   <- {len(fl)}: {' '.join(fl[:8])}{' ...' if len(fl)>8 else ''}")
print(stat, file=sys.stderr)
