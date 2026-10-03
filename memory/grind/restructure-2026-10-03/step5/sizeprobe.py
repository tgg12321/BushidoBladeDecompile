"""sizeprobe.py: for each hoisted data symbol C: its nearest lower-address declared neighbour P and
higher-address neighbour S; sizeof(P), sizeof(C) measured by cc1 in a TU that declares them;
flags P covering C or C covering S (second views of the same bytes). Run in WSL."""
import glob, json, re, subprocess, os
CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-lang-c", "-fno-builtin", "-Dmips", "-D__GNUC__=2",
       "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-w", "-mel", "-msoft-float", "-o", "/tmp/szp.s"]
addr = {}
for l in open("build/bb2.map", encoding="utf-8", errors="replace"):
    m = re.match(r"^\s+0x0*([0-9a-fA-F]{8})\s+([A-Za-z_]\w*)\s*$", l)
    if m:
        addr.setdefault(m.group(2), int(m.group(1), 16))
def ad(n):
    if n in addr: return addr[n]
    m = re.match(r"D_([0-9A-F]{8})$", n)
    return int(m.group(1), 16) if m else None
where = {}
for f in sorted(glob.glob("src/main/**/*.c", recursive=True)):
    t = open(f, encoding="utf-8").read()
    for m in re.finditer(r"^(?:extern\s+|static\s+)?(?:volatile\s+)?[A-Za-z_][\w ]*?[\s\*]+(\w+)\s*(?:\[[^\]]*\])*\s*(?:=[^;]*)?;", t, re.M):
        where.setdefault(m.group(1), [])
        if f not in where[m.group(1)]: where[m.group(1)].append(f)
for h in ["include/bb2.h"]:
    t = open(h, encoding="utf-8").read()
    for m in re.finditer(r"^extern\s+[\w ]*?[\s\*]+(\w+)\s*(?:\[[^\]]*\])*\s*;", t, re.M):
        where.setdefault(m.group(1), []).append("src/main/9F9C.c")  # any bb2.h includer
def size1(n, f):
    pre = subprocess.run(CPP + [f], capture_output=True).stdout + f"\nint __szp = sizeof({n});\n".encode()
    r = subprocess.run(CC1, input=pre, capture_output=True)
    if r.returncode: return None
    s = open("/tmp/szp.s").read()
    m = re.search(r"__szp:\s*\n\s*\.word\s+(\w+)", s)
    return int(m.group(1), 0) if m else None
def size(n):
    sz = [size1(n, f) for f in sorted(set(where.get(n, [])))]
    if not sz or any(s is None for s in sz): return None
    return max(sz)
names = sorted((n for n in where if ad(n) is not None), key=ad)
C = json.load(open("tmp/s5/hoist.json"))
for c in C:
    if c["kind"] != "O": continue
    n = c["name"]; a = ad(n)
    if a is None:
        print("??", n, "no address"); continue
    lower = [x for x in names if ad(x) < a]
    higher = [x for x in names if ad(x) > a]
    p = lower[-1] if lower else None
    s = higher[0] if higher else None
    sp = size(p) if p else None
    sc = size(n)
    flag = []
    if p and sp is not None and ad(p) + sp > a: flag.append(f"inside {p}")
    if s and sc is not None and a + sc > ad(s): flag.append(f"covers {s}")
    if sp is None and p: flag.append(f"size({p}) unknown")
    if sc is None: flag.append("own size unknown")
    print(f"{'FLAG' if any(x.startswith(('inside','covers')) for x in flag) else 'ok  '} {n}@{a:08X} size={sc} prev={p}@{ad(p) if p else 0:08X}/{sp} next={s} {' ; '.join(flag)}")
