#!/usr/bin/env python3
"""Opcode-level aligned diff of func between target (build/src/code6cac.o) and
sandbox object. Usage: sdiff.py [ours.o] [--regs] [--ctx N]"""
import re, subprocess, sys, difflib

FUNC = "func_800198D0"
ours_o = "tmp/sandbox/func_800198D0/code6cac.o"
tgt_o = "build/src/code6cac.o"
args = [a for a in sys.argv[1:]]
regs = "--regs" in args
args = [a for a in args if not a.startswith("--")]
if args:
    ours_o = args[0]


def dis(o):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "-r", "--no-show-raw-insn", "-M", "no-aliases" if False else "gpr-names=32", o],
                         capture_output=True, text=True).stdout
    lines = out.splitlines()
    res = []
    on = False
    for ln in lines:
        if re.match(r"^[0-9a-f]+ <%s>:" % FUNC, ln):
            on = True
            continue
        if on and re.match(r"^[0-9a-f]+ <", ln):
            break
        if not on:
            continue
        m = re.match(r"^\s*([0-9a-f]+):\s+(\S+)\s*(.*)$", ln)
        if m:
            res.append([m.group(2), m.group(3).strip(), int(m.group(1), 16)])
        elif res and "R_MIPS" in ln:
            r = ln.split()
            res[-1][1] += "  <" + r[-2] + " " + r[-1] + ">"
    base = res[0][2] if res else 0
    for r in res:
        r[2] -= base
    return res


def norm(ins):
    op, a, addr = ins
    if not regs:
        a2 = re.sub(r"\$?\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", "R", a)
    else:
        a2 = a
    a2 = re.sub(r"\b[0-9a-f]+ <[^>]*>", "L", a2)
    if op in ("b", "beq", "bne", "beqz", "bnez", "bgtz", "blez", "bltz", "bgez", "j", "jal"):
        a2 = re.sub(r"\b[0-9a-f]{2,}\b", "L", a2)
    return op + " " + a2


t = dis(tgt_o)
o = dis(ours_o)
tn = [norm(x) for x in t]
on = [norm(x) for x in o]
sm = difflib.SequenceMatcher(None, tn, on, autojunk=False)
print(f"target {len(t)} ours {len(o)}")
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == "equal":
        continue
    print(f"@@ {tag} t[{i1}:{i2}] o[{j1}:{j2}]")
    for i in range(i1, i2):
        print(f"  -T {i:4d} {t[i][0]:8s} {t[i][1]}")
    for j in range(j1, j2):
        print(f"  +O {j:4d} {o[j][0]:8s} {o[j][1]}")
