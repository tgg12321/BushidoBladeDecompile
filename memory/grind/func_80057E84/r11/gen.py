#!/usr/bin/env python3
"""gen.py <reuse.c> <outdir>: Ruling 11 twins of func_80057E84's reuse body.

Values (each a group of writes reaching a common read):
  vtx:   vtx_a (edge start, poly->vtx[i]), vtx_b (edge end, poly->vtx[next]),
         vtx_dn (down corner, poly->vtx[idx_dn]), vtx_up (up corner, poly->vtx[idx_up])
  node:  node_dn (&route->node[c] in the down block), node_up (same in the up block)
  route: route_dn (&path[0] in the down block), route_up (&path[1] in the up block),
         route_pick (the two tail writes, both read by the copy loop)

Comments are stripped first (they do not reach cc1's output). Outputs:
  reuse.c                 the reuse body, comments stripped
  pv_<var>.c / pv_all.c   every value of <var> (of all three) in its own fresh local, declared
                          where the reuse variable is declared (fs)
  pvbs_<var>.c / pvbs_all.c  same, each value declared at the innermost block enclosing it (bs)
  r_<value>.c             only that one value split (fs)
"""
import re
import sys
from pathlib import Path

VALUES = {
    "vtx": ["vtx_a", "vtx_b", "vtx_dn", "vtx_up"],
    "node": ["node_dn", "node_up"],
    "route": ["route_dn", "route_up", "route_pick"],
}
TYPES = {"vtx": "s16 *", "node": "CpuWaypoint *", "route": "CpuRoute *"}

# region anchors (in the comment-stripped body)
A_EDGE = "        for (i = 0; i < nedges; i++) {\n"
A_EDGE_B = "            vtx = poly->vtx[next];\n"
A_DN = "        if (go_dn) {\n"
A_UP = "        if (go_up) {\n"
A_END = "        if (!go_dn && !go_up) {\n"
A_TAIL = "    if (dist_dn < dist_up) {\n"

# where each value lives: (region, innermost-block anchor for bs, indent)
REGION = {
    "vtx_a": "EA", "vtx_b": "EB", "vtx_dn": "D", "vtx_up": "U",
    "node_dn": "D", "node_up": "U",
    "route_dn": "D", "route_up": "U", "route_pick": "T",
}
BS_ANCHOR = {
    "EA": (A_EDGE, "            "), "EB": (A_EDGE, "            "),
    "D": ("            if (hit_dn) {\n                s32 c;\n", "                "),
    "U": ("            if (hit_up) {\n                s32 c;\n", "                "),
    "T": (None, None),
}
DECL = {
    "vtx": "        s16 *vtx;\n",
    "node": "        CpuWaypoint *node;\n",
    "route": "    CpuRoute *route;\n",
}


def strip_comments(s):
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    s = re.sub(r"[ \t]+\n", "\n", s)
    s = re.sub(r"\n\s*\n(\s*\n)+", "\n\n", s)
    return s


def regions(s):
    e = s.index(A_EDGE)
    eb = s.index(A_EDGE_B, e)
    d = s.index(A_DN, eb)
    u = s.index(A_UP, d)
    end = s.index(A_END, u)
    t = s.index(A_TAIL, end)
    return {"PRE": (0, e), "EA": (e, eb), "EB": (eb, d), "D": (d, u), "U": (u, end),
            "MID": (end, t), "T": (t, len(s))}


def var_of(value):
    return value.split("_")[0]


def split(s, values, mode):
    """Rename each value in `values` to its own local; mode fs|bs decides where it is declared."""
    reg = regions(s)
    # rename back to front so offsets stay valid
    order = sorted({REGION[v] for v in values}, key=lambda r: reg[r][0], reverse=True)
    for r in order:
        a, b = reg[r]
        chunk = s[a:b]
        for v in values:
            if REGION[v] == r:
                chunk = re.sub(r"(?<!->)\b%s\b" % var_of(v), v, chunk)
        s = s[:a] + chunk + s[b:]
    # declarations
    for var, vals in VALUES.items():
        mine = [v for v in vals if v in values]
        if not mine:
            continue
        keep = len(mine) < len(vals)
        if mode == "fs" or var == "route" and mine == ["route_pick"]:
            ind = DECL[var][: len(DECL[var]) - len(DECL[var].lstrip())]
            new = "".join(f"{ind}{TYPES[var]}{v};\n" for v in mine)
            s = s.replace(DECL[var], (DECL[var] if keep else "") + new, 1)
        else:
            fs_vals = [v for v in mine if BS_ANCHOR[REGION[v]][0] is None]
            ind = DECL[var][: len(DECL[var]) - len(DECL[var].lstrip())]
            s = s.replace(DECL[var], (DECL[var] if keep else "") +
                          "".join(f"{ind}{TYPES[var]}{v};\n" for v in fs_vals), 1)
            for v in reversed(mine):
                anchor, bind = BS_ANCHOR[REGION[v]]
                if anchor is None:
                    continue
                i = s.index(anchor) + len(anchor)
                s = s[:i] + f"{bind}{TYPES[var]}{v};\n" + s[i:]
    return s


def main():
    src = strip_comments(Path(sys.argv[1]).read_text())
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    (out / "reuse.c").write_text(src)
    allv = [v for vals in VALUES.values() for v in vals]
    for mode, pre in (("fs", "pv"), ("bs", "pvbs")):
        for var, vals in VALUES.items():
            (out / f"{pre}_{var}.c").write_text(split(src, vals, mode))
        (out / f"{pre}_all.c").write_text(split(src, allv, mode))
    for v in allv:
        (out / f"r_{v}.c").write_text(split(src, [v], "fs"))


if __name__ == "__main__":
    main()
