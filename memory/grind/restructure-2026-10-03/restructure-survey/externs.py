"""Census of extern / prototype declarations in src/*.c and include/*.h.

Heuristic (regex, not a C parser): a declaration is a line-start `extern ...;` (single line) or a
file-scope prototype `type name(args);`. Reports: per-file counts (file-scope vs block-scope),
symbols declared in >1 file, conflicting spellings, FAKE-labelled block-scope externs, and whether
a header already declares the symbol.
"""
import re, glob, collections, os, json

def norm(s):
    s = re.sub(r"/\*.*?\*/", "", s)
    s = re.sub(r"//.*", "", s)
    return " ".join(s.split())

DECL = re.compile(r"^(?P<ind>\s*)extern\s+(?P<body>[^;{]*);")
PROTO = re.compile(r"^(?P<ind>)(?!extern|static|typedef|return|if|else|while|for|#)(?P<body>[A-Za-z_][\w\s\*]*?\b(?P<name>\w+)\s*\([^;{]*\))\s*;")

def name_of(body):
    b = re.sub(r"\[[^\]]*\]", "", body)
    m = re.search(r"\(\s*\*\s*(\w+)\s*\)", b)  # function pointer
    if m and "(" in b.split(m.group(0))[0] + "(":
        pass
    # function prototype: name before first '('
    m = re.match(r"^[^()]*?\b(\w+)\s*\(", b)
    if m and not re.search(r"\(\s*\*", b[:b.index("(") + 3]):
        return m.group(1), "func"
    if m:
        m2 = re.search(r"\(\s*\*\s*(\w+)", b)
        if m2:
            return m2.group(1), "obj"
    b = b.split("=")[0]
    toks = re.findall(r"\w+", b)
    return (toks[-1] if toks else "?"), "obj"

def scan(path):
    out = []
    lines = open(path, encoding="utf-8", errors="replace").read().split("\n")
    depth = 0
    for i, ln in enumerate(lines, 1):
        s = ln
        m = DECL.match(s)
        kind = None
        if m:
            body = m.group("body")
            nm, k = name_of(body)
            out.append(dict(file=path, line=i, name=nm, kind=k, body=norm(body),
                            block=depth > 0, fake=("FAKE" in ln or (i > 1 and "FAKE" in lines[i - 2])),
                            extern=True))
        elif depth == 0:
            m = PROTO.match(s)
            if m and not s.rstrip().endswith("{"):
                body = m.group("body")
                out.append(dict(file=path, line=i, name=m.group("name"), kind="func", body=norm(body),
                                block=False, fake=False, extern=False))
        depth += ln.count("{") - ln.count("}")
        if depth < 0:
            depth = 0
    return out

src = sorted(glob.glob("src/*.c"))
hdr = sorted(h for h in glob.glob("include/*.h") if "m2c" not in h)
decls = []
for f in src + hdr:
    decls += scan(f)
by = collections.defaultdict(list)
for d in decls:
    by[d["name"]].append(d)

srcd = [d for d in decls if d["file"].startswith("src")]
print("total src decls", len(srcd), "extern", sum(d["extern"] for d in srcd),
      "block-scope", sum(d["block"] for d in srcd), "fake-labelled", sum(d["fake"] for d in srcd))
print("header decls", len([d for d in decls if d["file"].startswith("include")]))
# per file
print("\n# per file: file-scope extern / prototypes / block-scope / FAKE / already-in-header")
for f in src:
    ds = [d for d in srcd if d["file"] == f]
    inh = sum(1 for d in ds if any(x["file"].startswith("include") for x in by[d["name"]]))
    print(f"{os.path.basename(f):28s} fs_ext={sum(1 for d in ds if d['extern'] and not d['block']):4d} "
          f"proto={sum(1 for d in ds if not d['extern']):3d} block={sum(d['block'] for d in ds):3d} "
          f"fake={sum(d['fake'] for d in ds):3d} dup_hdr={inh:3d}")
# multi-file symbols
multi = {n: v for n, v in by.items() if len({d['file'] for d in v if d['file'].startswith('src')}) > 1}
print("\nsymbols declared in >1 src file:", len(multi))
conf = {}
for n, v in by.items():
    sp = set()
    for d in v:
        b = d["body"]
        b = re.sub(r"\b" + re.escape(n) + r"\b", "@", b)
        b = b.replace(" ", "")
        sp.add(b)
    if len(sp) > 1:
        conf[n] = v
print("symbols with >1 distinct spelling (incl. headers):", len(conf))
hdrconf = [n for n, v in conf.items() if any(d['file'].startswith('include') for d in v)]
print("  of which also declared in a header:", len(hdrconf))
inhdr_dup = [d for d in srcd if any(x["file"].startswith("include") for x in by[d["name"]])]
print("src decls that duplicate a header declaration:", len(inhdr_dup))
# top conflicts
print("\n# sample conflicts (name: spellings @file:line)")
for n in sorted(conf, key=lambda n: -len(conf[n]))[:40]:
    sp = collections.defaultdict(list)
    for d in conf[n]:
        sp[d["body"].replace(" ", "")].append(f"{os.path.basename(d['file'])}:{d['line']}{'B' if d['block'] else ''}")
    print(n + ": " + " | ".join(f"{k} @{','.join(v[:4])}{'..' if len(v) > 4 else ''}" for k, v in sp.items()))
json.dump(decls, open("tmp/restructure-survey/decls.json", "w"), indent=0)
# fake block-scope
print("\n# FAKE-labelled declarations")
for d in srcd:
    if d["fake"]:
        print(f"  {d['file']}:{d['line']} {'block' if d['block'] else 'file'} {d['body'][:100]}")
