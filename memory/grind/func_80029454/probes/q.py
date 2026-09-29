#!/usr/bin/env python3
"""Fast local harness for func_80029454 (WSL): prefix of src/code6cac_b.c + candidate,
compiled with the Makefile recipe for code6cac_b; compares against the target
listing taken from build/src/code6cac_b.o (INCLUDE_ASM = original bytes).
usage: q.py cand.c [-d] [-k] [-r]   (-d: print diff, -r: keep register names in score)"""
import sys, os, subprocess, re, difflib, hashlib

ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
os.chdir(ROOT)
FUNC = "func_80029454"
args = [a for a in sys.argv[1:] if not a.startswith('-')]
flags = [a for a in sys.argv[1:] if a.startswith('-')]
cand = args[0]
work = "tmp/func_80029454/q"
os.makedirs(work, exist_ok=True)
tag = hashlib.md5(cand.encode()).hexdigest()[:8]
src_lines = open("src/code6cac_b.c").read().split("\n")
idx = next(i for i, l in enumerate(src_lines) if l.startswith('INCLUDE_ASM("asm/funcs", %s)' % FUNC))
body = open(cand).read()
tu = "\n".join(src_lines[:idx]) + "\n" + body + "\n"
cfile = f"{work}/tu_{tag}.c"
open(cfile, "w").write(tu)
obj = f"{work}/tu_{tag}.o"
MASPSX_FLAGS = "--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --expand-lb"
CPP_DEFS = "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
cc1 = os.environ.get("CC1", "tools/gcc-2.7.2/build/cc1")
extra = os.environ.get("CC1X", "")
cmd = (f"mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin {CPP_DEFS} {cfile} | "
       f"{cc1} -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float {extra} | "
       f"python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py {MASPSX_FLAGS} | "
       f"sed 's/\\.align\\t3/.align\\t2/' | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | "
       f"mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o {obj}")
r = subprocess.run(cmd, shell=True, capture_output=True, text=True)
if r.returncode != 0 or not os.path.exists(obj):
    print("BUILD FAILED\n", r.stderr[-3000:])
    sys.exit(1)

def dis(o):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-r", "--no-show-raw-insn", o],
                         capture_output=True, text=True).stdout
    lines = out.split("\n")
    res = []
    on = False
    start = None
    for l in lines:
        if l.endswith(f"<{FUNC}>:"):
            on = True
            continue
        if on:
            if not l.strip():
                break
            m2 = re.match(r"\s+[0-9a-f]+:\s+(R_MIPS_\S+)\s+(\S+)", l)
            if m2:
                if res:
                    if res[-1][0] == 'jal':
                        res[-1][1] = m2.group(2)
                    else:
                        res[-1][1] += " @" + m2.group(2)
                continue
            m = re.match(r"\s+([0-9a-f]+):\s+(\S+)\s*(.*)", l)
            if m:
                addr = int(m.group(1), 16)
                if start is None:
                    start = addr
                ins = m.group(2); ops = m.group(3)
                ops = re.sub(r"\s*<[^>]*>", "", ops)
                if ins.startswith('b') or ins in ('j', 'jal'):
                    parts = ops.split(',')
                    try:
                        t = int(parts[-1], 16)
                        parts[-1] = "L%x" % (t - start)
                        ops = ','.join(parts)
                    except ValueError:
                        pass
                res.append([ins, ops])
            else:
                m2 = re.match(r"\s+[0-9a-f]+:\s+(R_MIPS_\S+)\s+(\S+)", l)
                if m2 and res:
                    res[-1][1] += " @" + m2.group(2)
    return [f"{a} {b}" for a, b in res]

tgt = dis("build/src/code6cac_b.o")
ours = dis(obj)
def norm(x):
    if '-r' in flags:
        return x
    return re.sub(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", "R", x)
sm = difflib.SequenceMatcher(None, [norm(x) for x in tgt], [norm(x) for x in ours], autojunk=False)
diffcnt = 0
for op, i1, i2, j1, j2 in sm.get_opcodes():
    if op != 'equal':
        diffcnt += max(i2 - i1, j2 - j1)
smr = difflib.SequenceMatcher(None, tgt, ours, autojunk=False)
rawcnt = sum(max(i2 - i1, j2 - j1) for op, i1, i2, j1, j2 in smr.get_opcodes() if op != 'equal')
print(f"target {len(tgt)}  ours {len(ours)}  structdiff {diffcnt}  rawdiff {rawcnt}")
if '-d' in flags:
    use = smr if '-r' in flags else sm
    for op, i1, i2, j1, j2 in use.get_opcodes():
        if op == 'equal':
            continue
        print(f"@@ {op} t[{i1}:{i2}] o[{j1}:{j2}]")
        for k in range(i1, i2):
            print(f"  T {k:4d} {tgt[k]}")
        for k in range(j1, j2):
            print(f"  O {k:4d} {ours[k]}")
if '-k' in flags:
    open(f"{work}/ours.s", "w").write("\n".join(ours))
    open(f"{work}/tgt.s", "w").write("\n".join(tgt))
