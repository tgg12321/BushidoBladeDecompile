"""fncmp.py <ref.dis> <new.dis> <fn...>: per-function instruction diff (branch targets masked)."""
import re, sys, difflib

def fns(p):
    d, cur = {}, None
    for l in open(p):
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", l)
        if m:
            cur = m.group(1); d[cur] = []; continue
        if cur is None or not l.strip() or l.startswith("Disassembly"):
            continue
        s = l.strip()
        if re.match(r"^[0-9a-f]+: R_MIPS", s):
            s = "    " + re.sub(r"^[0-9a-f]+:\s*", "", s)
            s = re.sub(r"\.L\w+|\.text\+0x[0-9a-f]+|\.text", "LBL", s)
        else:
            s = re.sub(r"^[0-9a-f]+:\s*", "", s)
            s = re.sub(r"[0-9a-f]+ <[^>]+>", "T", s)
        d[cur].append(s)
    return d

a, b = fns(sys.argv[1]), fns(sys.argv[2])
for f in sys.argv[3:]:
    x, y = a.get(f), b.get(f)
    if y is None:
        print(f, ": not in candidate output"); continue
    xi = [s for s in x if not s.startswith("    ")]
    yi = [s for s in y if not s.startswith("    ")]
    sm = difflib.SequenceMatcher(None, x, y, autojunk=False)
    nd = sum(max(i2 - i1, j2 - j1) for t, i1, i2, j1, j2 in sm.get_opcodes() if t != "equal")
    print("%s: ref %d insns, new %d insns, diff lines (incl relocs) %d" % (f, len(xi), len(yi), nd))
    for l in list(difflib.unified_diff(x, y, n=1, lineterm=""))[2:60]:
        print("   ", l)
