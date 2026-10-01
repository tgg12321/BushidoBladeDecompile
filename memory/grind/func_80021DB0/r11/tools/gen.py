#!/usr/bin/env python3
"""Generate one-variable-per-value spellings of func_80021DB0's `j` (Ruling 11 (D)(4) ablations)."""
from pathlib import Path

d = Path(__file__).resolve().parent
L = (d / "base.c").read_text().split("\n")
# 1-based line numbers in base.c
LOOP1 = [35, 37, 38]
LOOP2 = [54, 55]
SEL = [64, 68, 74, 78]
FOR_I = next(n for n, s in enumerate(L, 1) if s.strip() == "for (i = 0; i < 8; i++) {")


def sub_j(line, new):
    import re
    return re.sub(r"\bj\b", new, line)


def make(name, loop1=None, loop2=None, sel=None, keep_j=True):
    out = list(L)
    blockdecl = []
    if loop1:
        for n in LOOP1:
            out[n - 1] = sub_j(out[n - 1], loop1)
        blockdecl.append(f"        s32 {loop1};")
    if loop2:
        for n in LOOP2:
            out[n - 1] = sub_j(out[n - 1], loop2)
        blockdecl.append(f"        s32 {loop2};")
    if sel:
        for n in SEL:
            out[n - 1] = sub_j(out[n - 1], sel)
        out[11 - 1] = out[11 - 1] + f"\n    s32 {sel};"
    if not keep_j:
        out[11 - 1] = out[11 - 1].replace("    s32 j;\n", "").replace("    s32 j;", "")
    if blockdecl:
        out[FOR_I - 1] = out[FOR_I - 1] + "\n" + "\n".join(blockdecl)
    (d / f"{name}.c").write_bytes(("\n".join(out)).encode())
    print(name)


make("split_loop1", loop1="step")
make("split_loop2", loop2="rise")
make("split_sel", sel="sel")
make("split_all", loop1="step", loop2="rise", sel="sel", keep_j=False)
