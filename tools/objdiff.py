#!/usr/bin/env python3
"""Normalized per-function object comparison.

Two MIPS .o files built from the same source differ cosmetically in ways that are
NOT codegen differences:
  * `jal 0 <SomeName>` — the jal target is 0 with an R_MIPS_26 reloc; objdump
    prints whatever symbol happens to sit at offset 0 in THAT object, so the name
    changes when function order/size changes.
  * branch/jump displacements printed as absolute offsets, which shift whenever an
    earlier function changes size.
Both are normalized away here so only real codegen differences survive.

Usage:
  objdiff.py <objA> <objB>             -> summary + per-function diffs
  objdiff.py <objA> <objB> --summary   -> just the changed-function list
"""
import subprocess, sys, re, difflib

SYMTGT = re.compile(r'\b[0-9a-f]+\s*<[^>]*>')
HEXIMM = re.compile(r'\b0x[0-9a-f]+\b')


def functions(obj):
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-d", obj],
                         capture_output=True, text=True).stdout
    funcs, cur = {}, None
    for line in out.splitlines():
        if line.endswith(">:") and "<" in line:
            cur = line.split("<")[1].rstrip(">:")
            funcs[cur] = []
            continue
        if cur is None:
            continue
        p = line.split("\t")
        if len(p) >= 3:
            ins = p[2].strip() + (" " + p[3].strip() if len(p) > 3 else "")
            ins = SYMTGT.sub("TGT", ins)
            funcs[cur].append(ins)
    return funcs


def compare(a_obj, b_obj):
    a, b = functions(a_obj), functions(b_obj)
    same, changed, missing = [], [], []
    for f in a:
        if f not in b:
            missing.append(f)
        elif a[f] == b[f]:
            same.append(f)
        else:
            changed.append(f)
    return a, b, same, changed, missing


if __name__ == "__main__":
    A, B = sys.argv[1], sys.argv[2]
    summary = "--summary" in sys.argv
    a, b, same, changed, missing = compare(A, B)
    print(f"functions={len(a)} identical={len(same)} changed={len(changed)} "
          f"missing={len(missing)}")
    for f in changed:
        print(f"  CHANGED {f}  ({len(a[f])} -> {len(b[f])} insns)")
    if summary:
        sys.exit(0)
    for f in changed:
        print(f"\n===== {f}  A={len(a[f])}  B={len(b[f])} =====")
        for l in difflib.unified_diff(a[f], b[f], lineterm="", n=1):
            if l.startswith(('+', '-')) and not l.startswith(('+++', '---')):
                print("  ", l)
