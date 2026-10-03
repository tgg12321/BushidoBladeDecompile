"""hoist.py: game declarations that 2+ game TUs (src/main/*.c) declare at file scope, spelled the same
(type-wise) everywhere they are declared (file or block scope, any TU), absent from every header,
not Sony-defined and not static anywhere. Prints the candidates with their spelling, files and any
attached comment; --json writes tmp/s5/hoist.json."""
import collections, glob, json, re, sys
sys.path.insert(0, "tmp/s5")
from citems import items, strip_cs
from dedupe import proto_key, ws

C = json.load(open("tmp/s5/census.json"))
game = sorted(f.replace("\\", "/") for f in glob.glob("src/main/*.c"))
hdr_text = "".join(open(h, encoding="utf-8").read() for h in glob.glob("include/**/*.h", recursive=True) + glob.glob("src/**/*.h", recursive=True))
hdr_names = set(re.findall(r"\b\w+\b", strip_cs(hdr_text)))
sony = {n for n, t in C["defs"].items() if t.startswith("main/psxsdk/")}
for f in glob.glob("src/main/psxsdk/**/*.c", recursive=True):
    sony |= set(re.findall(r"BIOS_[ABC]_FUNCTION\((\w+)", open(f, encoding="utf-8").read()))
alltext = {f.replace("\\", "/"): open(f, encoding="utf-8").read() for f in glob.glob("src/**/*.c", recursive=True)}

decl = collections.defaultdict(list)   # name -> [(file, key, text, comment)]
for f in game:
    its, lines = items(alltext[f])
    for it in its:
        if it["kind"] != "decl":
            continue
        own = "\n".join(lines[it["start"] - 1:it["end"]])
        c = ws(strip_cs(own))
        if c.startswith(("typedef", "static", "struct", "union", "enum")) or "{" in c:
            continue
        pk = proto_key(own)
        if pk:
            name, key = pk[1], pk
        elif c.startswith("extern"):
            b = re.sub(r"\[[^\]]*\]", "", c.split("=")[0])
            m2 = re.search(r"\(\s*\*\s*(\w+)\s*\)", b)
            name = m2.group(1) if m2 else re.findall(r"\w+", b)[-1]
            key = ("O", c)
            if "," in c:
                continue  # multi-declarator lines: leave alone
        else:
            continue
        j = it["start"] - 1
        while j >= it["lead"] and lines[j - 1].strip() != "":
            j -= 1
        run = "\n".join(lines[j:it["start"] - 1])
        blocks = list(re.finditer(r"/\*.*?\*/", run, re.S))
        com = blocks[-1].group(0) if blocks and run[blocks[-1].end():].strip() == "" and run[:blocks[-1].start()].split("\n")[-1].strip() == "" else ""
        decl[name].append((f, key, own.strip(), com))


def aux_norm(t, n):
    t = t.replace("extern ", "")
    t = re.sub(r"\s+", " ", t)
    return t


cands, why, rej = [], collections.Counter(), []
for name, ds in sorted(decl.items()):
    files = {d[0] for d in ds}
    if len(files) < 2:
        continue
    keys = {d[1] for d in ds}
    if len(keys) != 1:
        why["file-scope spellings differ"] += 1; rej.append(("file-scope spellings differ", name, sorted({d[2] for d in ds}))); continue
    if name in hdr_names:
        why["name in a header"] += 1; continue
    if name in sony:
        why["Sony function"] += 1; continue
    if any(re.search(r"^static\b[^;{(]*\b" + re.escape(name) + r"\b", t, re.M) for t in alltext.values()):
        why["static somewhere"] += 1; continue
    key = next(iter(keys))
    if key[0] == "F":
        sp = set()
        for r in C["funcs"]:
            if r["name"] != name or not r["file"].startswith("src/"):
                continue
            if r["kind"] in ("NC", "OC"):
                sp.add(aux_norm(r["text"], name))
            elif r["kind"] in ("NF", "OF") and r["file"].startswith("src/"):
                m = re.match(r"(.*?\()(.*)(\);?)$", r["text"])
                ps = []
                for p in m.group(2).split(","):
                    p = p.strip()
                    if p in ("void", "...", "") or "(" in p:
                        ps.append(p); continue
                    ids = re.findall(r"\w+", p)
                    ps.append(re.sub(r"\s*\b\w+$", "", p) if len(ids) > 1 and re.search(r"[\w*]\s*\b\w+$", p) else p)
                sp.add(aux_norm(m.group(1) + ", ".join(ps) + m.group(3), name).replace(" (", " ("))
        sp = {re.sub(r"\s*\(\s*", " (", s).replace("( ", "(") for s in sp}
        if len(sp) > 1:
            why["prototype/definition spellings differ"] += 1; rej.append(("prototype vs definition", name, sorted(sp))); continue
    else:
        sp = {ws(o["text"]) for o in C["objs"] if o["name"] == name}
        if len(sp) > 1:
            why["object spellings differ (incl. block scope)"] += 1; rej.append(("object spellings differ", name, sorted(sp))); continue
    coms = {d[3] for d in ds if d[3]}
    cands.append(dict(name=name, kind=key[0], text=ds[0][2], files=sorted(files), comments=sorted(coms)))
    why["candidate"] += 1
print(why)
for c in cands:
    print(f"{c['kind']} {c['name']:32s} {len(c['files'])} files  {c['text'][:90]}" + (f"  [comments: {len(c['comments'])}]" if c["comments"] else ""))
if "--json" in sys.argv:
    json.dump(cands, open("tmp/s5/hoist.json", "w"), indent=1)

if "--rejects" in sys.argv:
    NL = chr(10)
    with open("tmp/s5/hoist_rejects.txt", "w", encoding="utf-8", newline=NL) as fh:
        for r, n, sp in rej:
            fh.write(r + chr(9) + n + chr(9) + " | ".join(s.replace(NL, " ") for s in sp) + NL)
