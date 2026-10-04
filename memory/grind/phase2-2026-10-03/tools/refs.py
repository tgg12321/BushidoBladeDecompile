#!/usr/bin/env python3
"""refs.py : the reference prototype of every function row in tmp/p2/phase2_conflicts.tsv (run in WSL,
repo root, after census.py / hoist.py / conflicts.py). Writes tmp/p2/targets.tsv:
  class  symbol  def(BB2 C definition)  bb2hdr(include/ spelling; a Sony reference only under include/psxsdk/)  sotn  psyz  target  target_src
Reference order for Sony rows: BB2 header -> SOTN include/psxsdk (SOTN env var, default
/mnt/c/Users/Trenton/Desktop/sotn-decomp) -> psyz psyz/include (PSYZ env var, default /tmp/psyz; a PsyQ
4.0 rewrite whose return types are not always Sony's) -> the BB2 definition. Game rows: the definition,
else the most common spelling. `target` is the chosen spelling with PsyQ scalar names mapped to BB2's
(u_long -> u32 ...), parameter names dropped; proto_trial.py compiles it into every caller."""
import collections, glob, json, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from citems import items, strip_cs

SOTN = os.environ.get("SOTN", "/mnt/c/Users/Trenton/Desktop/sotn-decomp") + "/include/psxsdk"
PSYZ = os.environ.get("PSYZ", "/tmp/psyz") + "/psyz/include"
MAP = [(r"\bunsigned\s+long\b", "u32"), (r"\bunsigned\s+short\b", "u16"), (r"\bunsigned\s+char\b", "u8"),
       (r"\bunsigned\s+int\b", "u32"), (r"\bu_long\b", "u32"), (r"\bu_short\b", "u16"), (r"\bu_char\b", "u8"),
       (r"\bu_int\b", "u32"), (r"\blong\b", "s32"), (r"\bshort\b", "s16"), (r"\bOT_TYPE\b", "u32"),
       (r"\bunsigned\b", "u32"), (r"\bCallback\b", "void (*)()")]


def ws(s):
    return " ".join(s.split())


def split_params(p):
    out, depth, cur = [], 0, ""
    for ch in p:
        if ch == "," and depth == 0:
            out.append(cur.strip()); cur = ""; continue
        depth += ch == "("; depth -= ch == ")"
        cur += ch
    out.append(cur.strip())
    return out


def drop_name(p):
    """parameter text without its name: `RECT* rect` -> `RECT *`, `void (*f)(int)` -> `void (*)(int)`."""
    p = ws(p)
    if p in ("void", "...", ""):
        return p
    m = re.match(r"(.*\(\s*\*)\s*\w+\s*(\).*)", p)
    if m:
        return m.group(1) + m.group(2)
    ids = re.findall(r"[A-Za-z_]\w*", p)
    kw = {"const", "volatile", "unsigned", "signed", "struct", "union", "enum", "int", "char", "short", "long", "void"}
    if len(ids) >= 2 and ids[-1] not in kw and re.search(r"\b" + ids[-1] + r"\s*(\[[^\]]*\])?$", p):
        p = re.sub(r"\b" + ids[-1] + r"\s*(?=(\[[^\]]*\])?$)", "", p).strip()
    return ws(p.replace("*", " * ")).replace("* *", "**")


def canon(text, name):
    """`RET name(PARAMS)` with names dropped, `@` for the name, BB2 scalar names; None if unparsable."""
    s = ws(strip_cs(text)).rstrip(";").strip()
    s = re.sub(r"^(extern|static)\s+", "", s)
    m = re.match(r"^(.*?)\b" + re.escape(name) + r"\s*\((.*)\)\s*$", s)
    if not m:
        return None
    ret, params = m.group(1).strip(), m.group(2)
    ps = [drop_name(p) for p in split_params(params)]
    out = f"{ws(ret.replace('*', ' * '))} @({', '.join(ps)})"
    for a, b in MAP:
        out = re.sub(a, b, out)
    return ws(out).replace("( ", "(").replace(" )", ")")


def header_protos(paths):
    res = collections.defaultdict(list)
    for h in paths:
        t = open(h, encoding="utf-8", errors="replace").read()
        its, lines = items(t)
        for it in its:
            if it["kind"] != "decl":
                continue
            src = "\n".join(lines[it["start"] - 1:it["end"]])
            c = ws(strip_cs(src))
            m = re.search(r"\b(\w+)\s*\([^()]*(\([^()]*\)[^()]*)*\)\s*;$", c)
            if m and not c.startswith("typedef") and "(*" not in c.split(m.group(1))[0]:
                res[m.group(1)].append((f"{h}:{it['start']}", c))
    return res


def main():
    C = json.load(open("tmp/p2/census.json"))
    sotn = header_protos(sorted(glob.glob(SOTN + "/*.h")))
    psyz = header_protos(sorted(glob.glob(PSYZ + "/*.h")))
    defs, hdr, ghdr, spell = {}, {}, {}, collections.defaultdict(collections.Counter)
    for r in C["funcs"]:
        if r["kind"] in ("NF", "OF") and r["file"].startswith("src/"):
            defs[r["name"]] = (f"{r['file']}:{r['line']}", r["text"])
        elif r["kind"] in ("NC", "OC"):
            if r["file"].startswith("include/psxsdk/") or (r["file"].endswith(".h") and r["file"].startswith("src/")):
                hdr[r["name"]] = (f"{r['file']}:{r['line']}", r["text"])
            elif r["file"].startswith("include/"):
                ghdr[r["name"]] = (f"{r['file']}:{r['line']}", r["text"])
            else:
                spell[r["name"]][r["text"]] += 1
    rows = []
    for l in open("tmp/p2/phase2_conflicts.tsv", encoding="utf-8"):
        if l.startswith("#"):
            continue
        cls, sym, _ = l.rstrip("\n").split("\t", 2)
        if cls == "note" or (cls in ("game-local", "header-local") and sym not in defs and sym not in spell):
            continue
        d = defs.get(sym)
        h = hdr.get(sym) or ghdr.get(sym)
        so = sotn.get(sym, [None])[0]
        pz = psyz.get(sym, [None])[0]
        cd = canon(d[1], sym) if d else None
        ch = canon(h[1], sym) if h else None
        chs = ch if sym in hdr else None   # only a Sony header's spelling is a Sony reference
        cs = canon(so[1], sym) if so else None
        cz = canon(pz[1], sym) if pz else None
        if cls == "sony-func":
            order = [(chs, "bb2hdr"), (cs, "sotn"), (cz, "psyz"), (cd, "def")]
        else:
            common = canon(spell[sym].most_common(1)[0][0], sym) if spell.get(sym) else None
            order = [(cd, "def"), (ch, "bb2hdr"), (common, "common")]
        tgt, src = next(((t, s) for t, s in order if t), ("", ""))
        rows.append((cls, sym, f"{cd} [{d[0]}]" if d else "", f"{ch} [{h[0]}]" if h else "",
                     f"{ws(so[1])} [{so[0].replace(SOTN, 'sotn:include/psxsdk')}]" if so else "",
                     f"{ws(pz[1])} [{pz[0].replace(PSYZ, 'psyz:psyz/include')}]" if pz else "", tgt, src))
    with open("tmp/p2/targets.tsv", "w", encoding="utf-8", newline="\n") as fh:
        fh.write("# class\tsymbol\tdef\tbb2hdr\tsotn\tpsyz\ttarget\ttarget_src\n")
        for r in rows:
            fh.write("\t".join(r) + "\n")
    print(len(rows), "function rows;", collections.Counter(r[7] for r in rows))


if __name__ == "__main__":
    main()
