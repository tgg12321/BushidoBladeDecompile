"""mksplit.py <src_text1b.c> <outdir> : Q89 cut of text1b.c immediately before INCLUDE_ASM math_RotMatrixZYX.
Head keeps the name text1b.c; the tail becomes text1b_tu1b.c. Moves only (per-file gp model: a definition
lives in the file whose accesses reach it gp-relative):
  - the tail-only statics (D_800A33F0 .. D_800A3418, the second half of the Q65 static block) move verbatim
    to the tail, under a copy of the block's comment line;
  - the head-only K3 definitions D_800A3248/D_800A324A/D_800A324C (first lines of the .sdata block at the end of
    the file) and the K1 tentative definition g_gpu_ot256_ptr move verbatim to the end of the head, each under
    a copy of its block's comment line (the .sdata comment line stays in the tail too);
  - the PAD_NOPS #define block (used only after the cut) moves verbatim into the tail's declaration block
    (splitc.py copies it there as an active #define the tail needs; the head copy is then removed);
  - splitc.py (memory/grind/func_80058580/aspsx-align-check/poc/splitc.py) does the rest.
"""
import os, subprocess, sys

src, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
L = open(src, encoding="utf-8", newline="").read().split("\n")
cut_text = 'INCLUDE_ASM("asm/funcs", math_RotMatrixZYX);'
assert L.count(cut_text) == 1

# 1. tail statics
first, last = L.index("static s32 D_800A33F0;"), L.index("static s32 D_800A3418;")
statics = L[first:last + 1]
assert all(l.startswith("static ") for l in statics), statics
st_comment = "/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */"
assert L[first - 17] == st_comment, L[first - 17]
L = L[:first] + L[last + 1:]

# 2. head K3 + K1 definitions from the end of the file
sd_comment = "/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */"
k3 = ["s16 D_800A3248 = -1;", "s16 D_800A324A = -1;", "s32 D_800A324C = -1;"]
i = L.index(sd_comment)
assert L[i + 1:i + 4] == k3, L[i + 1:i + 4]
del L[i + 1:i + 4]
cm_comment = "/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */"
j = L.index(cm_comment)
assert L[j + 1] == "s32 g_gpu_ot256_ptr;" and all(not x.strip() for x in L[j + 2:]), L[j:]
k1 = L[j:j + 2]
del L[j:j + 2]
while L and L[-1] == "" and len(L) > 1 and L[-2] == "":
    L.pop()
c = L.index(cut_text)
L[c:c] = ["", sd_comment] + k3 + [k1[0], k1[1]]

# 2b. (--cluster tail|own) the merged text1a_b_pre_rodata block's data definitions leave the head.
# The typedef Unk800153F0Record (+ its comment) stays: func_80049F4C (head) uses it.
cluster, mode = [], None
if "--cluster" in sys.argv:
    mode = sys.argv[sys.argv.index("--cluster") + 1]
    a = L.index("/* ---- merged from text1a_b_pre_rodata.c (owner ruling Q67: one original file) ---- */")
    b = L.index("/* ---- merged from sound.c (owner ruling Q67: one original file) ---- */")
    td = L.index("typedef struct {", a)
    assert L[td + 1] == "    u16 v[22];" and L[td + 2] == "} Unk800153F0Record;" and td < b
    keep_c = L.index("/* 0x800153F0: the 22-halfword record func_8004A09C unpacks (it walks it as u16). func_80049F4C copies", a)
    assert keep_c == td - 3
    cluster = L[a:keep_c] + L[td + 3:b]
    L = L[:a] + L[keep_c:td + 3] + [""] + L[b:]

if mode == "tail":
    c = L.index(cut_text)
    L[c:c] = cluster + [""]
    cut = c + 1
else:
    cut = L.index(cut_text) + 1
tmp = os.path.join(out, "text1b.c")
open(tmp, "w", encoding="utf-8", newline="\n").write("\n".join(L))
splitc = "memory/grind/func_80058580/aspsx-align-check/poc/splitc.py"
r = subprocess.run([sys.executable, os.path.abspath(splitc), "text1b.c", str(cut), "text1b_tu1b.c"], cwd=out,
                   capture_output=True, text=True)
print(r.stdout.strip(), r.stderr.strip())
assert r.returncode == 0

# 3. head: drop the PAD_NOPS block (now copied into the tail's declaration block)
pad = ['#define PAD_NOPS_1 __asm__(".section .text\\n    nop\\n")',
       '#define PAD_NOPS_2 __asm__(".section .text\\n    nop\\n    nop\\n")',
       '#define PAD_NOPS_3 __asm__(".section .text\\n    nop\\n    nop\\n    nop\\n")']
h = open(tmp, encoding="utf-8").read().split("\n")
ip = h.index(pad[0])
assert h[ip:ip + 3] == pad and h[ip - 1] == "/* Padding NOP macro */"
end = ip + 3
if h[end] == "":
    end += 1
del h[ip - 1:end]
open(tmp, "w", encoding="utf-8", newline="\n").write("\n".join(h))

# 4. tail: the moved statics after the declaration block, before the moved text
tp = os.path.join(out, "text1b_tu1b.c")
t = open(tp, encoding="utf-8").read().split("\n")
assert pad[0] in t
k = t.index(cut_text)
k = t.index(cluster[0]) if mode == "tail" else k
t[k:k] = [st_comment] + statics + [""]
open(tp, "w", encoding="utf-8", newline="\n").write("\n".join(t))
if mode == "own":
    hdr = ['#include "common.h"', "", "typedef struct {", "    u16 v[22];", "} Unk800153F0Record;", ""]
    open(os.path.join(out, "text1b_ro.c"), "w", encoding="utf-8", newline="\n").write("\n".join(hdr + cluster) + "\n")
print("head %d lines, tail %d lines" % (len(h), len(t)))
