#!/usr/bin/env python3
"""Combined marionation probe: splice candidate -> sandbox (masked score + exact
callee-saved reg mapping vs target) + faithful allocno ledger (debug cc1).
Usage: python3 tmp/probe.py <candidate.c> [--noledger]
Leaves src in candidate state. Base for splice = tmp/system.c.orig (pristine)."""
import re, subprocess, sys
from pathlib import Path

SRC = Path("src/system.c"); BAK = Path("tmp/system.c.orig")
TGT = "build/src/system.o"; OUR = "tmp/sandbox/marionation_Exec/system.o"
REGNAME = {16:"s0",17:"s1",18:"s2",19:"s3",20:"s4",21:"s5",22:"s6",23:"s7"}

def extract_fn(text, name):
    m = re.search(r"(^|\n)(s32\s+"+name+r"\s*\([^;{]*\))\s*\{", text)
    if not m: raise SystemExit("fn start not found")
    start = m.start(2); i = text.index("{", m.end(2)-1); depth = 0
    while i < len(text):
        if text[i] == "{": depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0: return start, i+1
        i += 1
    raise SystemExit("fn end not found")

def splice(candpath):
    base = BAK.read_text(); cand = Path(candpath).read_text()
    s, e = extract_fn(base, "marionation_Exec"); cs, ce = extract_fn(cand, "marionation_Exec")
    SRC.write_text(base[:s] + cand[cs:ce] + base[e:])

def objdump_win(obj):
    r = subprocess.run(["mipsel-linux-gnu-objdump","-d",obj], capture_output=True, text=True)
    out, on = [], False
    for l in r.stdout.splitlines():
        m = re.match(r"^[0-9a-f]+ <(.+)>:$", l)
        if m: on = (m.group(1)=="marionation_Exec"); continue
        if on and re.match(r"^\s+[0-9a-f]+:", l):
            p = l.split("\t"); out.append("\t".join(p[2:]).strip() if len(p)>=3 else p[-1].strip())
    return out

def sandbox():
    r = subprocess.run(["bash","-c","python3 -m engine.cli sandbox marionation_Exec --disable all 2>&1"],
                       capture_output=True, text=True)
    for line in r.stdout.splitlines():
        if '"score"' in line: return line.split(":")[1].strip().rstrip(",")
    return "?"

CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips "
       "-D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx "
       "-D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C src/system.c > tmp/rtl/p.i 2>/dev/null")
CC1 = ("cd tmp/rtl && BB2_ALLOC_DEBUG=1 ../gccdbg/cc1 -O2 -G0 -funsigned-char -quiet "
       "-mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -da p.i -o p.s 2> alloc.log")

def ledger():
    Path("tmp/rtl").mkdir(exist_ok=True)
    subprocess.run(["bash","-c",CPP], check=True); subprocess.run(["bash","-c",CC1])
    greg = Path("tmp/rtl/p.i.greg").read_text()
    i = greg.index("Function marionation_Exec"); seg = greg[i:]
    di = seg.index("Register dispositions:"); disp = {}
    for m in re.finditer(r"(\d+) in (\d+)", seg[di:di+900]): disp[int(m.group(1))]=int(m.group(2))
    mar = {p:h for p,h in disp.items() if 16<=h<=23}
    log = Path("tmp/rtl/alloc.log").read_text(errors="replace").splitlines()
    blocks, cur = [], []
    for l in log:
        m = re.search(r"ord=(\d+) pseudo=(\d+) hardreg=(\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)", l)
        if not m: continue
        if int(m.group(1))==0 and cur: blocks.append(cur); cur=[]
        cur.append(tuple(int(m.group(k)) for k in range(2,7)))
    if cur: blocks.append(cur)
    best = None
    for b in blocks:
        bm = {p:h for (p,h,_,_,_) in b if 16<=h<=23}
        if all(bm.get(p)==h for p,h in mar.items()): best=b; break
    return mar, best

if __name__ == "__main__":
    cand = sys.argv[1]; splice(cand)
    score = sandbox()
    tgt, our = objdump_win(TGT), objdump_win(OUR)
    n = min(len(our), len(tgt))
    diffs = sum(1 for j in range(n) if our[j]!=tgt[j] and "<marionat" not in our[j])
    print(f"== {cand}: masked={score} raw_diffs={diffs} insns={len(our)}/{len(tgt)}")
    if "--noledger" not in sys.argv:
        mar, best = ledger()
        # value label by CURRENT reg (based on objdump of OUR); derive per-reg from names
        print("  reg  pseudo nrefs livelen  pri     (sorted by pri desc)")
        if best:
            rows = sorted([(p,h,nr,ll,pr) for (p,h,nr,ll,pr) in best if 16<=h<=23], key=lambda r:-r[4])
            for p,h,nr,ll,pr in rows:
                print(f"  {REGNAME[h]:4s} {p:<6d} {nr:<5d} {ll:<8d} {pr}")
        else:
            print("  (no matching ledger block)")
