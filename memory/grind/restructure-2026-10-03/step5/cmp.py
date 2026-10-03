#!/usr/bin/env python3
"""cmp.py A B : compare two snapshots. Per TU: token-stream changed?, object byte-identical?
(if not: per-section bytes, relocations, symbol table compared and the differences printed), and the
implicit-declaration set difference. Exit 1 on any object difference."""
import re, subprocess, sys
A, B = (f"tmp/s5/snap/{x}" for x in sys.argv[1:3])
def kv(p):
    return dict(l.split() for l in open(p) if l.strip())
ta, tb = kv(f"{A}/tokens.txt"), kv(f"{B}/tokens.txt")
changed = sorted(t for t in tb if ta.get(t) != tb[t])
print("TUs whose token stream changed:", len(changed))
bad = 0
def run(*a):
    return subprocess.run(a, capture_output=True, text=True).stdout
def detail(o):
    secs = {}
    for l in run("mipsel-linux-gnu-readelf", "-SW", o).splitlines():
        m = re.match(r"\s*\[\s*(\d+)\]\s+(\S+)", l)
        if m and m.group(2).startswith("."):
            secs[m.group(2)] = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", m.group(2), o, "/dev/stdout"], capture_output=True).stdout
    rel = run("mipsel-linux-gnu-readelf", "-rW", o)
    sym = sorted(" ".join(l.split()[1:]) for l in run("mipsel-linux-gnu-readelf", "-sW", o).splitlines() if re.match(r"\s*\d+:", l))
    return secs, rel, sym
for t in sorted(tb):
    oa, ob = f"{A}/{t}.o", f"{B}/{t}.o"
    if open(oa, "rb").read() == open(ob, "rb").read():
        continue
    sa, ra, ya = detail(oa); sb, rb, yb = detail(ob)
    diffs = [s for s in set(sa) | set(sb) if sa.get(s) != sb.get(s) and s not in (".symtab", ".strtab", ".shstrtab")]
    if ra != rb: diffs.append("relocs")
    if ya != yb: diffs.append("symbols")
    if diffs:
        bad += 1
        print("OBJECT DIFF", t, diffs)
        if "symbols" in diffs:
            for s in sorted(set(ya) ^ set(yb))[:20]:
                print("   ", "-" if s in ya else "+", s)
    else:
        print("bytes differ only in symtab/strtab layout:", t)
def impl(p):
    return set(l.rstrip("\n") for l in open(p) if l.strip())
ia, ib = impl(f"{A}/implicit.txt"), impl(f"{B}/implicit.txt")
for x in sorted(ia - ib): print("implicit -> declared:", x)
for x in sorted(ib - ia): print("declared -> implicit:", x)
print("CMP", "OK" if not bad else f"FAIL ({bad} objects)", f"; implicit pairs {len(ia)} -> {len(ib)}")
sys.exit(1 if bad else 0)
