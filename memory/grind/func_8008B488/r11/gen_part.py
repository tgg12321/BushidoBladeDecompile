#!/usr/bin/env python3
"""gen_part.py -- generate func_8008B488 variants from candidate.c where the five
ADSR clamp values (AR, DR, SR, RR, SL) are grouped into shared locals.

Singleton values keep their own local, declared at block scope (the
one-variable-per-value spelling, Ruling 11 (C)(1)). A group of >=2 values shares
one local declared at the top of the for-loop body (the innermost scope that
encloses all its writes, Ruling 11 (A)).

usage: gen_part.py OUTDIR            -> all 52 partitions, writes list.txt
       gen_part.py OUTDIR "AR,DR|SR"  -> one partition (unlisted values are singletons)
"""
import itertools, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# the s3 landable candidate (git show 9b95aa8aa:memory/grind/func_8008B488/candidate.c)
SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "base-s3-candidate.c")
VALS = ["AR", "DR", "SR", "RR", "SL"]
VAR = {v: v.lower() + "_rate" for v in VALS}
# the line that opens each value's block
OPEN = {"AR": "(mask & 0x800)) {", "DR": "(mask & 0x1000)) {", "SR": "(mask & 0x2000)) {",
        "RR": "(mask & 0x4000)) {", "SL": "(mask & 0x8000)) {"}


def partitions(s):
    if not s:
        yield []
        return
    first, rest = s[0], s[1:]
    for p in partitions(rest):
        for i in range(len(p)):
            yield p[:i] + [[first] + p[i]] + p[i + 1:]
        yield [[first]] + p


def build(groups, scope="loop"):
    text = open(SRC, encoding="utf-8").read()
    body = text[text.index("void func_8008B488"):]
    head = text[:text.index("void func_8008B488")]
    head = re.sub(r"/\*.*?\*/\n", "", head, count=1, flags=re.S)  # drop banner comment
    for v in VALS:
        body = body.replace("    u16 %s;\n" % VAR[v], "", 1)
    shared = [g for g in groups if len(g) > 1]
    names = {}
    for k, g in enumerate(shared):
        nm = "temp" if len(shared) == 1 else "temp%d" % (k + 1)
        for v in g:
            names[v] = nm
    lines = body.split("\n")
    out = []
    for i, ln in enumerate(lines):
        out.append(ln)
        for v in VALS:
            if ln.endswith(OPEN[v]) and v not in names:
                out.append("            u16 %s;" % VAR[v])
        if ln.startswith("    for (voice = 0;") and scope == "loop":
            for k, g in enumerate(shared):
                out.append("        u16 %s;" % names[g[0]])
    body = "\n".join(out)
    if scope == "func":
        decl = "".join("    u16 %s;\n" % names[g[0]] for g in shared)
        body = body.replace("    s32 bSetAll;\n", "    s32 bSetAll;\n" + decl, 1)
    for v, nm in names.items():
        body = re.sub(r"\b%s\b" % VAR[v], nm, body)
    return head + body


def tag(groups):
    return "_".join("".join(x[:2] for x in g) for g in sorted(groups, key=lambda g: VALS.index(g[0])))


if __name__ == "__main__":
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    scope = os.environ.get("SCOPE", "loop")
    if len(sys.argv) > 2:
        spec = [g.split(",") for g in sys.argv[2].split("|")]
        used = {v for g in spec for v in g}
        groups = spec + [[v] for v in VALS if v not in used]
        todo = [groups]
    else:
        todo = list(partitions(VALS))
    names = []
    for g in todo:
        g = [sorted(x, key=VALS.index) for x in g]
        fn = os.path.join(outdir, "p_%s.c" % tag(g))
        open(fn, "w", encoding="utf-8", newline="\n").write(build(g, scope))
        names.append(fn)
    open(os.path.join(outdir, "list.txt"), "w", newline="\n").write("\n".join(names) + "\n")
    print(len(names), "variants")
