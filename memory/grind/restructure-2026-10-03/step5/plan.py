"""plan.py LIB... : header candidates for Sony library LIB (dir name under src/main/psxsdk).
Functions defined in LIB: every declaration spelling (param names / `extern` dropped) across src and
include, plus the definition's; ELIGIBLE when one spelling. Objects: every file-scope extern of a
name declared in some TU of LIB, its spellings and declaring files."""
import json, re, glob, collections, sys
C = json.load(open("tmp/s5/census.json"))
defs = dict(C["defs"])
for f in glob.glob("src/**/*.c", recursive=True):
    for m in re.finditer(r"BIOS_[ABC]_FUNCTION\((\w+)", open(f, encoding="utf-8").read()):
        defs[m.group(1)] = f[4:-2].replace("\\", "/")
libs = sys.argv[1:]
def lib_of(tu):
    p = tu.split("/")
    return p[2] if len(p) > 3 and p[1] == "psxsdk" else "GAME"
def norm_proto(t, name, names=()):
    t = t.replace("extern ", "")
    for n in names:
        t = re.sub(r"\b" + re.escape(n) + r"\b", "", t)
    t = re.sub(r"\s+", " ", t).replace(" ,", ",").replace(" )", ")").replace("( ", "(")
    return t.replace(" " + name + " (", " @(").replace("*" + name + " (", "*@(")
F = collections.defaultdict(dict)
for r in C["funcs"]:
    n = r["name"]
    if n not in defs or lib_of(defs[n]) not in libs:
        continue
    if r["kind"] in ("NC", "OC"):
        F[n][(r["file"], r["line"])] = norm_proto(r["text"], n)
    elif r["kind"] in ("NF", "OF") and r["file"].startswith("src/"):
        m = re.search(r"/\* \(([^)]*)\)", r.get("raw", "")) if False else None
        F[n][(r["file"], r["line"], "DEF")] = r["text"]
# definitions: strip param names using aux NF comment (not kept) -> derive names from text
out = []
for n in sorted(F, key=lambda n: (defs[n], n)):
    sp = collections.defaultdict(list)
    for k, t in F[n].items():
        if len(k) == 3:
            # NF text: 'extern void f (s32 arg0, u8 *p);' -> drop last identifier of each param
            m = re.match(r"(.*?\()(.*)(\);?)$", t)
            ps = []
            for p in m.group(2).split(","):
                p = p.strip()
                if p in ("void", "...", "") or "(" in p:
                    ps.append(p); continue
                ps.append(re.sub(r"\s*\b\w+$", "", p) if re.search(r"[\w*]\s*\b\w+$", p) and len(re.findall(r"\w+", p)) > 1 else p)
            t = m.group(1) + ", ".join(ps) + m.group(3)
            t = norm_proto(t, n)
            sp[t].append(f"{k[0][9:]}:{k[1]}DEF")
        else:
            sp[t].append(f"{k[0].replace('src/main/','').replace('include/','I:')}:{k[1]}")
    ok = len(sp) == 1
    print(f"F {'OK ' if ok else 'DIFF'} {defs[n]:32s} {n}")
    for t, fl in sp.items():
        print(f"        {t}   <- {' '.join(fl)}")
objs = collections.defaultdict(list)
for o in C["objs"]:
    if o["block"]:
        continue
    objs[o["name"]].append(o)
for n in sorted(objs):
    tus = {o["file"][4:-2].replace("\\", "/") for o in objs[n] if o["file"].startswith("src")}
    if not any(lib_of(t) in libs for t in tus):
        continue
    sp = collections.defaultdict(list)
    for o in objs[n]:
        t = re.sub(r"\s+", " ", o["text"])
        sp[t].append(f"{o['file'].replace('src/main/','').replace('include/','I:')}:{o['line']}")
    print(f"O {'OK ' if len(sp)==1 else 'DIFF'} {n}")
    for t, fl in sp.items():
        print(f"        {t}   <- {' '.join(fl)}")
