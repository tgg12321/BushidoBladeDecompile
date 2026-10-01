"""Wrap every `__asm__ ... ;` statement of a preprocessed C file as a decomp-permuter
`#pragma _permuter b64literal <b64>` line (the permuter emits it back verbatim), so its C
parser never sees GNU asm operand lists. Same method as tmp/func_8002DE20/mkperm.py.
usage: python3 b64asm.py in.c out.c"""
import sys
from base64 import b64encode

s = open(sys.argv[1]).read()
out, i = [], 0
while True:
    j = s.find("__asm__", i)
    if j == -1:
        out.append(s[i:])
        break
    ls = s.rfind("\n", 0, j) + 1
    out.append(s[i:ls])
    k = s.index("(", j)
    depth, p, instr = 0, k, False
    while True:
        c = s[p]
        if instr:
            if c == "\\":
                p += 2
                continue
            if c == '"':
                instr = False
        elif c == '"':
            instr = True
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                break
        p += 1
    end = s.index(";", p) + 1
    stmt = s[ls:end]
    out.append("#pragma _permuter b64literal %s\n" % b64encode(stmt.encode()).decode())
    i = end + 1 if end < len(s) and s[end] == "\n" else end
open(sys.argv[2], "w", newline="\n").write("".join(out))
