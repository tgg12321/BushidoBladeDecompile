#!/usr/bin/env python3
"""cmp.py A B : compare snapshots tmp/p2/snap/A and tmp/p2/snap/B (run in WSL, repo root).
Per TU: token stream changed?; object byte-identical? If not, every section's bytes, the relocation
listing and the symbol table are compared and the differences printed (an object whose bytes differ
only in .symtab/.strtab/.shstrtab layout is reported but not failed). Also: TUs added/removed, exe
SHA1s, and the implicit-declaration set difference (implicit -> declared is a promotion that can move
codegen; it is listed for review). Exit 1 on any object difference."""
import re, subprocess, sys

A, B = (f"tmp/p2/snap/{x}" for x in sys.argv[1:3])


def kv(p):
    return dict(l.split(None, 1) for l in open(p) if l.strip())


def run(*a):
    return subprocess.run(a, capture_output=True, text=True).stdout


def detail(o):
    secs = {}
    for l in run("mipsel-linux-gnu-readelf", "-SW", o).splitlines():
        m = re.match(r"\s*\[\s*(\d+)\]\s+(\S+)", l)
        if m and m.group(2).startswith("."):
            secs[m.group(2)] = subprocess.run(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", m.group(2), o,
                                               "/dev/stdout"], capture_output=True).stdout
    rel = run("mipsel-linux-gnu-readelf", "-rW", o)
    sym = sorted(" ".join(l.split()[1:]) for l in run("mipsel-linux-gnu-readelf", "-sW", o).splitlines()
                 if re.match(r"\s*\d+:", l))
    return secs, rel, sym


ma, mb = kv(f"{A}/meta.txt"), kv(f"{B}/meta.txt")
print("exe", ma["exe_sha1"].strip()[:12], "->", mb["exe_sha1"].strip()[:12])
ta, tb = kv(f"{A}/tokens.txt"), kv(f"{B}/tokens.txt")
for t in sorted(set(ta) - set(tb)):
    print("TU removed:", t)
for t in sorted(set(tb) - set(ta)):
    print("TU added:", t)
changed = sorted(t for t in tb if t in ta and ta[t] != tb[t])
print(f"TUs whose token stream changed: {len(changed)}" + (" (" + " ".join(changed[:12]) +
                                                          (" ..." if len(changed) > 12 else "") + ")" if changed else ""))
bad = 0
for t in sorted(set(ta) & set(tb)):
    oa, ob = f"{A}/obj/{t}.o", f"{B}/obj/{t}.o"
    if open(oa, "rb").read() == open(ob, "rb").read():
        continue
    sa, ra, ya = detail(oa)
    sb, rb, yb = detail(ob)
    diffs = sorted(s for s in set(sa) | set(sb)
                   if sa.get(s) != sb.get(s) and s not in (".symtab", ".strtab", ".shstrtab"))
    if ra != rb:
        diffs.append("relocs")
    if ya != yb:
        diffs.append("symbols")
    if diffs:
        bad += 1
        print("OBJECT DIFF", t, diffs)
        if "symbols" in diffs:
            for s in sorted(set(ya) ^ set(yb))[:20]:
                print("   ", "-" if s in ya else "+", s)
        if "relocs" in diffs:
            la, lb = ra.splitlines(), rb.splitlines()
            for x in [l for l in la if l not in lb][:10]:
                print("    - reloc", " ".join(x.split()))
            for x in [l for l in lb if l not in la][:10]:
                print("    + reloc", " ".join(x.split()))
    else:
        print("bytes differ only in symtab/strtab layout:", t)


def impl(p):
    return set(l.rstrip("\n") for l in open(p) if l.strip())


ia, ib = impl(f"{A}/implicit.txt"), impl(f"{B}/implicit.txt")
for x in sorted(ia - ib):
    print("implicit -> declared:", x)
for x in sorted(ib - ia):
    print("declared -> implicit:", x)
print("CMP", "OK" if not bad else f"FAIL ({bad} objects)", f"; implicit pairs {len(ia)} -> {len(ib)}")
sys.exit(1 if bad else 0)
