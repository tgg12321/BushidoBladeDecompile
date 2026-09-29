#!/usr/bin/env python3
"""Private whole-TU check (no tracked edits): splice the candidate into a tmp copy of
src/code6cac_b.c, compile with the Makefile recipe, and compare every function's
disassembly (relocs included) against build/src/code6cac_b.o."""
import os, re, subprocess
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
os.chdir(ROOT)
W = "tmp/func_80029454/tu"
os.makedirs(W, exist_ok=True)
text = open("src/code6cac_b.c", newline="").read()
line = 'INCLUDE_ASM("asm/funcs", func_80029454);\n'
assert text.count(line) == 1
body = open("memory/grind/func_80029454/candidate.c", newline="").read()
open(f"{W}/code6cac_b.c", "w", newline="").write(text.replace(line, body))
MASPSX = "--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --expand-lb"
DEFS = "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
cmd = (f"mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin {DEFS} {W}/code6cac_b.c | "
       "tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | "
       f"python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py {MASPSX} | sed 's/\\.align\\t3/.align\\t2/' | "
       f"python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o {W}/code6cac_b.o")
r = subprocess.run(cmd, shell=True, capture_output=True, text=True)
print("build rc", r.returncode, r.stderr[-2000:])

def funcs(o):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-r", "--no-show-raw-insn", o], capture_output=True, text=True).stdout
    res, cur = {}, None
    for l in out.split("\n"):
        m = re.match(r"^[0-9a-f]+ <(\S+)>:$", l)
        if m:
            cur = m.group(1); res[cur] = []; start = None; continue
        if cur is None or not l.strip():
            continue
        m = re.match(r"\s+([0-9a-f]+):\s+(.*)", l)
        if m:
            s = m.group(2)
            s = re.sub(r"<[^>]*>", "", s)
            s = re.sub(r"\b[0-9a-f]+ (?=$)", "", s)
            # branch targets relative to function start
            parts = s.split()
            res[cur].append(re.sub(r"\s+", " ", s.strip()))
    return res

a = funcs("build/src/code6cac_b.o")
b = funcs(f"{W}/code6cac_b.o")
print("functions: ref", len(a), "ours", len(b))
print("only in ref:", sorted(set(a) - set(b)))
print("only in ours:", sorted(set(b) - set(a)))

def norm(lines):
    out = []
    for s in lines:
        s = re.sub(r"(b\w*|j)\s+([\w,$]*?)([0-9a-f]+)$", lambda m: m.group(1) + " " + m.group(2) + "T", s)
        out.append(s)
    return out
diff = [f for f in sorted(set(a) & set(b)) if norm(a[f]) != norm(b[f])]
print("differing functions:", diff)
