#!/usr/bin/env python3
"""Ruling 11 twins for func_8002AB08 (adapted from memory/grind/func_80058580/r11/roles.py).

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
    "dx": {"dx_seg": [1, 2, 3], "dx_hit": [4, 5, 6], "dx_off0": [7, 8, 9, 14, 15, 16, 21, 22, 23], "dx_off1": [10, 11, 12, 17, 18, 19, 24, 25, 26], "dx_dir": [13, 20, 27, 28, 29, 30, 32], "dx_push": [31, 33, 34]},
    "dy": {"dy_seg": [1, 2, 3], "dy_hit": [4, 5, 6]},
    "dz": {"dz_seg": [1, 2, 3], "dz_hit": [4, 5, 6], "dz_off0": [7, 8, 9, 14, 15, 16, 21, 22, 23], "dz_off1": [10, 11, 12, 17, 18, 19, 24, 25, 26], "dz_dir": [13, 20, 27, 28, 29, 30, 32], "dz_push": [31, 33, 34]},
    "temp1": {"pt0": [1, 2, 3, 4, 5, 6, 7], "dsq0": [8, 9, 10, 11]},
    "temp2": {"pt1": [1, 2, 3, 4, 5, 6, 7], "dsq1": [8, 9, 10, 11]},
    "temp3": {"hn": list(range(1, 9)), "side": [9, 10, 11, 12]},
    "idx": {"seg": [1, 2, 3, 4, 5], "nearest": list(range(6, 14))},
    "alt": {"blade": list(range(1, 9)), "hitalt": list(range(9, 15))},
    "work": {"lensq": [1, 2], "dist": [3, 4, 5], "ang": [6, 7, 8, 9, 11], "weight": [10, 12, 13, 14, 15]},
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
        anchor = "    s32 strong;\n"
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
