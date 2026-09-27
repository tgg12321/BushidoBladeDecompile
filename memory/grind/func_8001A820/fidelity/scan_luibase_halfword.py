import json, re, glob, os
os.chdir(r"C:\Users\Trenton\desktop\Bushido Blade 2 Decompile")
q = json.load(open("engine/queue.json", encoding="utf-8"))["items"]
queued = {it["func"] for it in (q if isinstance(q, list) else q.values()) if isinstance(it, dict)}
canon = set(l.split()[0] for l in open("inline_asm_canonical.txt") if l.strip() and not l.startswith("#"))
DEST = re.compile(r"^(\w+)\s+\$(\w+),")
# Linear scan (ignores control flow): track registers currently holding a lui-only constant (low16 == 0).
for f in sorted(glob.glob("asm/funcs/*.s")):
    name = os.path.basename(f)[:-2]
    ins = [l.split("*/", 1)[-1].strip() for l in open(f).read().splitlines() if "*/" in l]
    hi = {}
    lh = lhu = 0
    sites = []
    for i, l in enumerate(ins):
        m = re.match(r"(lh|lhu)\s+\$(\w+), (.*)\(\$(\w+)\)$", l)
        if m and m.group(4) in hi and m.group(4) not in ("at",) and "%lo" not in m.group(3):
            r = m.group(2)
            if m.group(1) == "lh":
                lh += 1; sites.append(f"lh@{i}")
            else:
                for j in range(i + 1, min(len(ins), i + 6)):
                    if re.match(rf"sll\s+\$\w+, \${r}, 16", ins[j]):
                        lhu += 1; sites.append(f"lhu+sext@{i}")
                        break
        mm = re.match(r"lui\s+\$(\w+), (.*)$", l)
        if mm:
            hi[mm.group(1)] = i
            continue
        d = DEST.match(l)
        if d and not l.startswith(("sw", "sh", "sb", "b", "j", "mult", "div")):
            hi.pop(d.group(2), None)
    # require the lui register to be followed by at least one use as a base with an offset; skip $at
    if lh or lhu:
        st = "Q" if name in queued else ("CANON" if name in canon else "DONE")
        print(f"{name:26s} {st:5s} lh={lh} lhu+sext={lhu} {' '.join(sites[:6])}")
