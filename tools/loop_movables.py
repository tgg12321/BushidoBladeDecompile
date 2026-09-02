#!/usr/bin/env python3
"""loop.c movable-decision report — every term of move_movables' inequality
for every movable in every loop of one function, from the -da dumps.

WHY (post-mortem 2026-09-01): func_80041188 foreclosed its winning chassis
on an incomplete reading of scan_loop's movable test (it never enumerated
`n_times_set == 1`); func_8003C714 spent eight sessions on the single
inequality at loop.c:1631. The dump already carries most terms — this tool
lines them up and names the C-level lever for each.

    threshold * savings * lifetime >= insn_count   (loop.c:1631, moved if true)
    threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)    (loop.c:532)

Usage (WSL, venv active, repo root):
  pwsh tools/grinder/dump.ps1 <func>        # produces tmp/grind/<func>/dumps/<stem>.loop and .cse
  python3 tools/loop_movables.py --func <func> --file <stem> [--soft-float] [--json]

Derived (not printed by cc1) and labelled as such in the output:
  * loop_has_call — a call_insn whose UID lies inside the loop's [start,end]
    range in the .cse (fallback: .loop) dump RTL.
  * n_non_fixed_regs — 60 on the shipped hard-float chassis (ledger
    func_8003C714 s1/s8), 28 with --soft-float. CC_FLAGS is not a lever
    (no-compiler-divergence); the flag exists only to reproduce a ledger
    measurement.
  * n_times_set[regno] — count of `(set (reg:M R) ...)`/`(clobber (reg:M R))`
    in the .cse dump within the loop range (count_loop_regs_set also counts
    SUBREG/STRICT_LOW_PART destinations; those are folded in here).
  * threshold decrement — loop.c:1719/1904 subtract 3 after each moved
    movable; the report shows the RUNNING threshold each movable faced.

Dump-format notes (verified against tools/gcc-2.7.2/loop.c AND against real
dumps under tmp/grind/*/dumps/):
  * A movable line is `Insn %d: regno %d (life %d), ...` (loop.c:1556). Other
    `Insn %d:` lines in the same dump come from strength_reduce ("possible
    biv", "giv reg", "dest address") and are NOT movables — the leading
    `regno %d (life %d),` is what distinguishes them.
  * Real RTL insns terminate `) <code> {<template>} (nil)`, not `) -1`, and
    span multiple lines — insn blocks are therefore delimited by the next
    top-of-line `(insn|call_insn|jump_insn|note|code_label|barrier ...`.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

LOOP_HDR = re.compile(r"^Loop from (\d+) to (\d+): (\d+) real insns\.", re.M)
MOVABLE = re.compile(
    r"^Insn (?P<insn>\d+): regno (?P<regno>\d+) \(life (?P<life>\d+)\), "
    r"(?:consec (?P<consec>\d+), )?(?P<flags>(?:(?:cond|force|global|done|move-insn) )*)"
    r"(?:matches (?P<matches>\d+) )?(?:forces (?P<forces>\d+) )?"
    r"(?:savings (?P<savings>\d+) )?(?P<halved>halved since already moved )?"
    r"[ \t]*(?P<decision>moved to (?P<moved_to>\d+)|not desirable|not safe)?", re.M)

# Top-of-line start of an RTL expression in a cc1 dump.
RTL_HEAD = re.compile(r"^\((insn|call_insn|jump_insn|note|code_label|barrier)\s+(\d+)\b", re.M)
# A REG destination of a (set ...) / (clobber ...), through optional
# subreg / strict_low_part wrappers (mirrors count_loop_regs_set's dest walk).
DEST_RE = re.compile(
    r"\((?:set|clobber)\s+(?:\((?:subreg|strict_low_part)\S*\s+)*\(reg\S*\s+(\d+)")

LEVERS = {
    "savings": "loop.c:1596-1604 — 1 per insn moved, +1 for consec chains; a CONST_INT src is 1",
    "lifetime": "life = uid_luid span of the reg's uses; a second in-loop USE of the pseudo raises it",
    "insn_count": "count_loop_regs_set counts insns BEFORE loop_optimize deletes them: loop-carried "
                  "arithmetic that biv elimination later removes still counts (func_8003C714 s6)",
    "threshold": "loop.c:532 (loop_has_call halves it; a call in the loop is the only C-level input) "
                 "then -3 per movable already moved (loop.c:1719/1904)",
    "n_times_set": "scan_loop admits a movable only if n_times_set[dest]==1 (loop.c:705/740): a second "
                   "in-loop store to the same variable (even a dead one) removes the movable entirely "
                   "(func_80041188 s37 closer)",
    "cond": "conditionally executed — only movable if maybe_never allows; loop.c:760",
    "force/global": "REG_EQUIV/global effects — see loop.c:849-898",
}


def function_segment(dump: str, func: str) -> str:
    """The slice of a cc1 -da dump between `;; Function <func>` and the next banner."""
    m = re.search(r"^;; Function " + re.escape(func) + r"\b.*?$", dump, re.M)
    if not m:
        return dump
    rest = dump[m.end():]
    n = re.search(r"^;; Function ", rest, re.M)
    return rest[:n.start()] if n else rest


def parse_loops(seg: str) -> list:
    """Every `Loop from A to B: N real insns.` block and its movable lines.

    Phony loops (loop.c:573) print no insn_count and carry no movables, so they
    are not reported.
    """
    loops = []
    heads = list(LOOP_HDR.finditer(seg))
    for k, h in enumerate(heads):
        end = heads[k + 1].start() if k + 1 < len(heads) else len(seg)
        block = seg[h.end():end]
        movables = []
        for m in MOVABLE.finditer(block):
            flags = m.group("flags") or ""
            dec = m.group("decision") or ""
            movables.append({
                "insn": int(m.group("insn")),
                "regno": int(m.group("regno")),
                "life": int(m.group("life")),
                "consec": int(m.group("consec") or 0),
                "cond": "cond " in flags,
                "force": "force " in flags,
                "global": "global " in flags,
                "done": "done " in flags,
                "move_insn": "move-insn " in flags,
                "matches": int(m.group("matches")) if m.group("matches") else None,
                "forces": int(m.group("forces")) if m.group("forces") else None,
                "savings": int(m.group("savings")) if m.group("savings") is not None else None,
                "halved": bool(m.group("halved")),
                "decision": "moved" if dec.startswith("moved") else (dec or "(no decision line)"),
                "moved_to": int(m.group("moved_to")) if dec.startswith("moved") else None,
            })
        loops.append({"start": int(h.group(1)), "end": int(h.group(2)),
                      "insn_count": int(h.group(3)), "movables": movables})
    return loops


def count_sets_in_range(rtl: str, start: int, end: int):
    """(n_times_set approximation per regno, loop_has_call) over insns whose UID
    lies within [start, end].

    Insn blocks are delimited by the next top-of-line RTL head, so real
    multi-line dump insns yield exactly one block each.
    """
    sets = {}
    has_call = False
    heads = list(RTL_HEAD.finditer(rtl))
    for k, h in enumerate(heads):
        kind = h.group(1)
        uid = int(h.group(2))
        if kind in ("note", "code_label", "barrier"):
            continue
        if uid < start or uid > end:
            continue
        block = rtl[h.start():heads[k + 1].start() if k + 1 < len(heads) else len(rtl)]
        if kind == "call_insn":
            has_call = True
        for d in DEST_RE.finditer(block):
            r = int(d.group(1))
            sets[r] = sets.get(r, 0) + 1
    return sets, has_call


def report(func: str, stem: str, soft_float: bool) -> dict:
    dumps = ROOT / "tmp" / "grind" / func / "dumps"
    loop_p = dumps / f"{stem}.loop"
    cse_p = dumps / f"{stem}.cse"
    if not loop_p.exists():
        raise SystemExit(f"{loop_p} missing — run: pwsh tools/grinder/dump.ps1 {func}")
    seg = function_segment(loop_p.read_text(errors="replace"), func)
    cse_seg = function_segment(cse_p.read_text(errors="replace"), func) if cse_p.exists() else ""
    n_non_fixed = 28 if soft_float else 60
    out = {"func": func, "stem": stem, "n_non_fixed_regs": n_non_fixed, "loops": []}
    for lp in parse_loops(seg):
        sets, has_call = count_sets_in_range(cse_seg or seg, lp["start"], lp["end"])
        thr0 = (1 if has_call else 2) * (1 + n_non_fixed)
        thr = thr0
        rows = []
        for m in lp["movables"]:
            ic = lp["insn_count"] * (2 if m["halved"] else 1)
            lhs = (thr * m["savings"] * m["life"]) if m["savings"] is not None else None
            rows.append({**m, "n_times_set": sets.get(m["regno"]), "threshold_faced": thr,
                         "insn_count_faced": ic, "lhs": lhs,
                         "inequality": (f"{thr}*{m['savings']}*{m['life']}={lhs} >= {ic}"
                                        if lhs is not None else "(no savings line)")})
            if m["decision"] == "moved":
                thr -= 3
        out["loops"].append({**lp, "loop_has_call": has_call, "threshold_initial": thr0,
                             "movables": rows, "in_loop_sets": sets})
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--soft-float", action="store_true",
                    help="reproduce a -msoft-float measurement (diagnostic only)")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    r = report(a.func, a.file, a.soft_float)
    if a.json:
        print(json.dumps(r, indent=2))
        return 0
    print(f"loop.c movable report — {a.func} (n_non_fixed_regs={r['n_non_fixed_regs']} [derived])")
    if not r["loops"]:
        print("  (no non-phony loops in this function)")
    for lp in r["loops"]:
        print(f"\nLoop insns {lp['start']}..{lp['end']}: insn_count={lp['insn_count']} "
              f"loop_has_call={lp['loop_has_call']} [derived] "
              f"threshold={lp['threshold_initial']} [derived]")
        print("  insn regno life consec nset  sav  thr  cnt   inequality                 decision")
        for m in lp["movables"]:
            print(f"  {m['insn']:>4} {m['regno']:>5} {m['life']:>4} {m['consec']:>6} "
                  f"{str(m['n_times_set']):>4} {str(m['savings']):>4} {m['threshold_faced']:>4} "
                  f"{m['insn_count_faced']:>4}   {m['inequality']:<26} {m['decision']}"
                  + (" [cond]" if m["cond"] else "") + (" [move-insn]" if m["move_insn"] else ""))
        extra = {reg: n for reg, n in sorted(lp["in_loop_sets"].items()) if n > 1}
        if extra:
            print(f"  pseudos set >1 time in loop (NOT admissible as movables): {extra}")
    print("\nLevers per term:")
    for k, v in LEVERS.items():
        print(f"  {k:12s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
