#!/usr/bin/env python3
"""fd.py <variant-dir> [func] [ctx]: diff ours (variant o/t.elf) vs build/bb2.elf disassembly for a function."""
import subprocess, sys, difflib, re
V = sys.argv[1]; F = sys.argv[2] if len(sys.argv) > 2 else "func_80058580"; C = int(sys.argv[3]) if len(sys.argv) > 3 else 2
BR = ("j", "jal", "b", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez")
def dis(elf):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", "--no-show-raw-insn", elf], capture_output=True, text=True).stdout
    nm = subprocess.run(["mipsel-linux-gnu-nm", elf], capture_output=True, text=True).stdout
    addr = {l.split()[2]: int(l.split()[0], 16) for l in nm.splitlines() if len(l.split()) == 3}
    a = addr[F]; nxt = min(v for v in addr.values() if v > a and v < a + 0x10000)
    lines = []
    for l in out.splitlines():
        m = re.match(r"\s*([0-9a-f]+):\s+(.*)", l)
        if m and a <= int(m.group(1), 16) < nxt:
            ins = re.sub(r"\s+", " ", m.group(2).split("<")[0]).strip()
            if ins.split()[0] in BR:
                ins = re.sub(r"\b[0-9a-f]{6,8}\b", "ADDR", ins)
            lines.append((int(m.group(1), 16), ins))
    return lines
o = dis(V + "/o/t.elf"); t = dis("build/bb2.elf")
sm = difflib.SequenceMatcher(None, [x[1] for x in o], [x[1] for x in t], autojunk=False)
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == "equal": continue
    n += max(i2 - i1, j2 - j1)
    print("---- %s ours %x / target %x" % (tag, o[min(i1, len(o)-1)][0], t[min(j1, len(t)-1)][0]))
    for k in range(max(0, i1 - C), i1): print("    ", o[k][1])
    for k in range(i1, i2): print("  - ", o[k][1])
    for k in range(j1, j2): print("  + ", t[k][1])
print("ours", len(o), "target", len(t), "diff-lines", n)
