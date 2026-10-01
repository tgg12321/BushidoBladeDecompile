#!/usr/bin/env python3
"""Ruling 11 twins for func_80058580's work1..work5 (memory/grind/func_80058580/r11/README.md).

Occurrence indices count code occurrences of each workN in candidate.c (comments excluded,
index 0 = the declaration). A "value" is one role; its occurrences are renamed to a fresh
`s32 <value>_` local. Usage (from the repo root):
  roles.py <cand.c> list
  roles.py <cand.c> one <out_dir>                one file per value: r_<value>.c (that value split)
  roles.py <cand.c> var <out.c> <workN> [bs]     every value of workN split (bs: block-scoped decls)
  roles.py <cand.c> all <out.c> [bs]             every value of every workN split
"""
import re, sys, os

VALUES = {
    "work1": {
        "n449": [1, 2, 3], "base": [4, 5, 6], "kind": [7, 8, 9], "kind2": [10, 11, 12],
        "x1": [13, 14, 15], "m445": [16, 17, 18], "bsel": [19, 20],
        "w": [21, 22, 23, 24, 25, 26, 27], "lo": [28, 29, 30, 31, 32, 33],
    },
    "work2": {
        "n445": [1, 2, 3], "sum": [4, 5], "pace": [6, 7, 8], "y1": [9, 10, 11],
        "m449": [12, 13, 14], "best": [15, 16, 17], "cnt2": [18, 19, 20, 21, 22],
        "lvl": [23, 24, 25, 26, 27, 28, 29],
    },
    "work3": {
        "side": [1, 2, 3, 4], "n447": [5, 6], "ang": [7, 8, 9, 10, 11, 12],
        "prod": [13, 14, 15, 16], "force": [17, 18, 19, 20], "idx": [21, 22, 23, 24],
        "dist": [25, 26, 27, 28, 29, 30, 32], "farflag": [31, 33], "dang": [34, 35, 36, 37, 38, 39],
        "coin": [40, 41, 42], "sel3": [43, 44, 45, 46], "slot": [47, 48, 49, 50, 51, 52, 53, 55],
        "mask": [54, 56, 57, 58, 59, 60], "cmask": [61, 62], "ok": [63, 64, 65, 66, 67, 68, 69, 70],
        "script4": [71, 72, 73, 74, 75, 76],
    },
    "work4": {
        "i": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], "n": [11, 12, 13, 14, 15, 16], "k": [17, 18, 19, 20],
    },
    "work5": {
        "top": [1, 2], "adj": [3, 4, 5], "kindet": [6, 7, 8],
    },
}


def code_mask(s):
    m = [True] * len(s)
    i = 0
    while i < len(s):
        if s.startswith("/*", i):
            j = s.index("*/", i) + 2
            for k in range(i, j):
                m[k] = False
            i = j
        else:
            i += 1
    return m


def occurrences(s, var):
    m = code_mask(s)
    return [mo.start() for mo in re.finditer(r'\b%s\b' % var, s) if m[mo.start()]]


def innermost_block(s, positions, mask):
    """Offset just after the '{' (and its newline) of the innermost block enclosing all positions."""
    stack, enclosing = [], None
    lo, hi = min(positions), max(positions)
    for i, ch in enumerate(s):
        if not mask[i]:
            continue
        if ch == '{':
            stack.append(i)
        elif ch == '}':
            start = stack.pop()
            if start < lo and i > hi:
                if enclosing is None or start > enclosing:
                    enclosing = start
    nl = s.index('\n', enclosing) + 1
    return nl


def split(s, values, block_scoped=False):
    """values: list of (var, value). Rename and declare."""
    mask = code_mask(s)
    edits = []
    decls = []
    for var, val in values:
        occ = occurrences(s, var)
        pos = [occ[k] for k in VALUES[var][val]]
        for p in pos:
            edits.append((p, len(var), val + "_"))
        if block_scoped:
            at = innermost_block(s, pos, mask)
            # indentation of the first line in that block
            line_end = s.index('\n', at)
            indent = re.match(r'\s*', s[at:line_end]).group(0)
            decls.append((at, f"{indent}s32 {val}_;\n"))
        else:
            decls.append((None, f"    s32 {val}_;\n"))
    out = s
    allpos = sorted([(p, ln, new, 'r') for p, ln, new in edits] +
                    [(at, 0, d, 'd') for at, d in decls if at is not None], reverse=True)
    for p, ln, new, kind in allpos:
        out = out[:p] + new + out[p + ln:]
    fs = "".join(d for at, d in decls if at is None)
    if fs:
        anchor = "    s32 pick;\n"
        assert out.count(anchor) == 1
        out = out.replace(anchor, anchor + fs, 1)
    return out


if __name__ == "__main__":
    s = open(sys.argv[1]).read()
    cmd = sys.argv[2]
    if cmd == "list":
        for var, vals in VALUES.items():
            occ = occurrences(s, var)
            for val, idx in vals.items():
                lines = sorted({s.count('\n', 0, occ[k]) + 1 for k in idx})
                print(var, val, "lines", lines)
    elif cmd == "one":
        d = sys.argv[3]
        os.makedirs(d, exist_ok=True)
        for var, vals in VALUES.items():
            for val in vals:
                open(os.path.join(d, f"r_{val}.c"), "w", newline="\n").write(split(s, [(var, val)]))
    elif cmd == "var":
        var = sys.argv[4]
        bs = len(sys.argv) > 5 and sys.argv[5] == "bs"
        open(sys.argv[3], "w", newline="\n").write(split(s, [(var, v) for v in VALUES[var]], bs))
    elif cmd == "all":
        bs = len(sys.argv) > 4 and sys.argv[4] == "bs"
        vals = [(var, v) for var in VALUES for v in VALUES[var]]
        open(sys.argv[3], "w", newline="\n").write(split(s, vals, bs))
