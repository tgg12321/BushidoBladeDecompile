#!/usr/bin/env python3
"""loop.c movable-decision report -- every term of move_movables' inequality
for every movable in every loop of one function, from the -da dumps.

WHY (post-mortem 2026-09-01): func_80041188 foreclosed its winning chassis
on an incomplete reading of scan_loop's movable test (it never enumerated
`n_times_set`); func_8003C714 spent eight sessions on the single inequality
at loop.c:1631. The dump already carries most terms -- this tool lines them
up and names the C-level lever for each.

    threshold * savings * lifetime >= insn_count   (loop.c:1631)
    threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)    (loop.c:532)

but that inequality is only the MIDDLE disjunct of the real test at
loop.c:1626-1631:

    if (already_moved[regno]
        || (threshold * savings * m->lifetime) >= insn_count
        || (m->forces && m->forces->done && n_times_used[m->forces->regno] == 1))

so a `moved` row whose arithmetic fails is not a bug in this tool -- it was
moved by the first or third disjunct, and is flagged [forced: ...].

SCOPE: move_movables (LICM of loop invariants) only. strength_reduce runs its
own, DIFFERENT threshold -- `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)`
at loop.c:3241, tested inverted at loop.c:3823 (`v->lifetime * threshold *
benefit < insn_count` => "not worth while") -- so givs/bivs are NOT covered by
this report.

Usage (WSL, venv active, repo root):
  pwsh tools/grinder/dump.ps1 <func>        # produces tmp/grind/<func>/dumps/<stem>.loop and .cse
  python3 tools/loop_movables.py --func <func> --file <stem> [--soft-float] [--json]

Derived (not printed by cc1) and labelled as such in the output:
  * loop_has_call -- a call_insn whose UID lies inside the loop's [start,end]
    range in the .cse RTL. If the .cse dump is absent the report WARNS and
    does not substitute the .loop RTL (which is POST-loop and misleading).
  * n_non_fixed_regs -- 60 on the shipped hard-float chassis (ledger
    func_8003C714 s1/s8), 28 with --soft-float. CC_FLAGS is not a lever
    (no-compiler-divergence); the flag exists only to reproduce a ledger
    measurement.
  * n_times_set[regno] -- count of `(set (reg:M R) ...)`/`(clobber (reg:M R))`
    in the .cse dump within the loop range, PSEUDOS ONLY: loop.c:596
    force-sets `n_times_set[i] = 1` for every i < FIRST_PSEUDO_REGISTER, so a
    hard reg is never what gates a movable.
  * threshold decrement -- loop.c:1719 (move_insn branch) and loop.c:1904
    (general branch) subtract 3 after a move, but the partial&&match branch at
    loop.c:1646-1668 prints the same " moved to" and does NOT decrement. A
    moved row that also printed `matches N` may therefore be that branch, so
    every LATER row in the loop is marked threshold_uncertain (`thr?`).

Dump-format notes (verified against tools/gcc-2.7.2/loop.c AND against real
dumps under tmp/grind/*/dumps/):
  * A movable line is `Insn %d: regno %d (life %d), ...` (loop.c:1556). Other
    `Insn %d:` lines in the same dump come from strength_reduce ("possible
    biv", "giv reg", "dest address") and are NOT movables -- the leading
    `regno %d (life %d),` is what distinguishes them.
  * Real RTL insns terminate `) <code> {<template>} (nil)`, not `) -1`, and
    span multiple lines -- insn blocks are therefore delimited by the next
    top-of-line `(insn|call_insn|jump_insn|note|code_label|barrier ...`.
  * `insn_count *= 2` at loop.c:1611 mutates move_movables' LOCAL insn_count,
    so the doubling PERSISTS for every later movable in the same loop
    (D_80083418 in ings2: 31 -> 62 -> 124 -> 248 across three halved rows).
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
# A PSEUDO destination of a (set ...) / (clobber ...), through the wrappers
# count_loop_regs_set walks. The trailing `\)` is load-bearing: a hard reg
# prints its name after the number (`(reg:SI 31 ra)`) and loop.c:596 pins
# n_times_set to 1 for all of them, so they must not appear here.
DEST_RE = re.compile(
    r"\((?:set|clobber)\s+"
    r"(?:\((?:subreg|strict_low_part|zero_extract|sign_extract)\S*\s+)*"
    r"\(reg\S*\s+(\d+)\)")

LEVERS = {
    "savings": "savings = n_times_used of the invariant pseudo inside the loop (loop.c:793, plus "
               "libcall_benefit at :795) -- a second in-loop USE of the invariant raises savings AND "
               "lifetime; loop.c:897 sets savings=1 on the partial/zero-extend path; force_movables "
               "doubles it (:1223) and combine_movables adds matched movables' (:1283, :1331)",
    "lifetime": "life = uid_luid span of the reg's uses; a second in-loop USE of the pseudo raises it",
    "insn_count": "count_loop_regs_set counts insns BEFORE loop_optimize deletes them: loop-carried "
                  "arithmetic that biv elimination later removes still counts (func_8003C714 s6). "
                  "Doubled per `halved` row and the doubling PERSISTS (loop.c:1611)",
    "threshold": "loop.c:532 (loop_has_call halves it; a call in the loop is the only C-level input) "
                 "then -3 per move (loop.c:1719/1904, but NOT the partial&&match branch :1646). "
                 "The inequality is one of three disjuncts (loop.c:1626-1631): already_moved[regno] "
                 "(another movable loading the same reg was moved, :1909) and a done `forces` "
                 "predecessor with n_times_used==1 both move it regardless of the arithmetic",
    "n_times_set": "scan_loop admits a movable only if n_times_set[dest]==1 OR "
                   "consec_sets_invariant_p succeeds (loop.c:705-709 -- it is a DISJUNCT, not a "
                   "hard gate): a second in-loop store to the same variable removes the movable "
                   "unless the whole consecutive set-chain is itself invariant (func_80041188 s37)",
    "cond": "conditionally executed -- m->cond set at loop.c:787 when invariant_p returned 2 "
            "(only conditionally invariant); re-tested at :1634 before the move",
    "force/global": "m->global at loop.c:788 (ordinary path) and :881-887 (partial path); "
                    "force_movables chains a movable to its predecessor at :1220-1223",
}


def function_segment(dump: str, func: str):
    """The slice of a cc1 -da dump between `;; Function <func>` and the next banner.

    Returns None when the banner is absent. There is deliberately NO whole-dump
    fallback: insn UIDs restart per function, so aggregating across banners
    would silently mix another function's insns into the loop range.
    """
    m = re.search(r"^;; Function " + re.escape(func) + r"\b.*?$", dump, re.M)
    if not m:
        return None
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
    """(n_times_set approximation per PSEUDO regno, loop_has_call) over insns
    whose UID lies within [start, end].

    Insn blocks are delimited by the next top-of-line RTL head, so real
    multi-line dump insns yield exactly one block each. Hard regs are excluded
    (loop.c:596 pins their n_times_set to 1).
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


def analyze_loop(lp: dict, sets: dict, has_call: bool, n_non_fixed: int) -> dict:
    """Replay move_movables' bookkeeping over one parsed loop.

    Stateful across the loop's movables, exactly as move_movables is:
      * insn_count doubles on each `halved` row and STAYS doubled (loop.c:1611)
      * threshold drops 3 after each move (loop.c:1719/1904)
    """
    thr0 = (1 if has_call else 2) * (1 + n_non_fixed)
    thr = thr0
    ic = lp["insn_count"]
    uncertain = False
    rows = []
    for m in lp["movables"]:
        if m["halved"]:
            ic *= 2
        lhs = (thr * m["savings"] * m["life"]) if m["savings"] is not None else None
        moved = m["decision"] == "moved"
        forced = bool(moved and lhs is not None and lhs < ic)
        rows.append({**m, "n_times_set": sets.get(m["regno"]), "threshold_faced": thr,
                     "threshold_uncertain": uncertain, "insn_count_faced": ic, "lhs": lhs,
                     "forced": forced,
                     "inequality": (f"{thr}*{m['savings']}*{m['life']}={lhs} >= {ic}"
                                    if lhs is not None else "(no savings line)")})
        if moved:
            # The partial&&match branch (loop.c:1646-1668) prints the same
            # " moved to" but skips the decrement; `matches` is the only hint
            # the dump gives, so assume the common path and flag the doubt.
            if m["matches"] is not None:
                uncertain = True
            thr -= 3
    return {**lp, "loop_has_call": has_call, "threshold_initial": thr0, "movables": rows,
            "in_loop_sets": sets,
            "multi_set": {r: n for r, n in sorted(sets.items()) if n > 1}}


def report(func: str, stem: str, soft_float: bool, dumps_dir=None) -> dict:
    # A dump covers the whole TU, so --dumps lets one dump serve every
    # function in it (the default dir is named for whichever func triggered it).
    dumps = Path(dumps_dir) if dumps_dir else ROOT / "tmp" / "grind" / func / "dumps"
    loop_p = dumps / f"{stem}.loop"
    cse_p = dumps / f"{stem}.cse"
    if not loop_p.exists():
        raise SystemExit(f"{loop_p} missing -- run: pwsh tools/grinder/dump.ps1 {func}")
    seg = function_segment(loop_p.read_text(errors="replace"), func)
    if seg is None:
        raise SystemExit(f"no ';; Function {func}' banner in {loop_p}")
    warnings = []
    cse_seg = None
    if cse_p.exists():
        cse_seg = function_segment(cse_p.read_text(errors="replace"), func)
        if cse_seg is None:
            warnings.append(f"no ';; Function {func}' banner in {cse_p}")
    else:
        warnings.append(f"{stem}.cse missing")
    nset_reliable = cse_seg is not None
    if not nset_reliable:
        warnings.append(
            f"WARNING: {stem}.cse missing -- n_times_set/loop_has_call derived from POST-loop "
            f"RTL and unreliable; re-run dump.ps1 {func}")
        print(warnings[-1], file=sys.stderr)
    n_non_fixed = 28 if soft_float else 60
    out = {"func": func, "stem": stem, "n_non_fixed_regs": n_non_fixed,
           "nset_reliable": nset_reliable, "warnings": warnings, "loops": []}
    rtl = cse_seg if cse_seg is not None else seg
    for lp in parse_loops(seg):
        sets, has_call = count_sets_in_range(rtl, lp["start"], lp["end"])
        out["loops"].append(analyze_loop(lp, sets, has_call, n_non_fixed))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--soft-float", action="store_true",
                    help="reproduce a -msoft-float measurement (diagnostic only)")
    ap.add_argument("--dumps", default=None,
                    help="dump dir override (default tmp/grind/<func>/dumps); a dump covers "
                         "the whole TU, so point at a sibling function's dir to reuse it")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    r = report(a.func, a.file, a.soft_float, a.dumps)
    if a.json:
        print(json.dumps(r, indent=2))
        return 0
    q = "" if r["nset_reliable"] else "?"
    print(f"loop.c movable report -- {a.func} "
          f"(n_non_fixed_regs={r['n_non_fixed_regs']} [derived])")
    if not r["loops"]:
        print("  (no non-phony loops in this function)")
    for lp in r["loops"]:
        print(f"\nLoop insns {lp['start']}..{lp['end']}: insn_count={lp['insn_count']} "
              f"loop_has_call={lp['loop_has_call']} [derived] "
              f"threshold={lp['threshold_initial']} [derived]")
        print("  insn regno life consec nset*  sav  thr  cnt   "
              "inequality                 decision")
        for m in lp["movables"]:
            thr = f"{m['threshold_faced']}" + ("?" if m["threshold_uncertain"] else "")
            note = ""
            if m["cond"]:
                note += " [cond]"
            if m["move_insn"]:
                note += " [move-insn]"
            if m["forced"]:
                note += " [forced: already_moved/forces]"
            if m["matches"] is not None and m["decision"] == "moved":
                note += " [matches: threshold decrement uncertain from here]"
            print(f"  {m['insn']:>4} {m['regno']:>5} {m['life']:>4} {m['consec']:>6} "
                  f"{str(m['n_times_set']) + q:>5} {str(m['savings']):>4} {thr:>4} "
                  f"{m['insn_count_faced']:>4}   {m['inequality']:<26} {m['decision']}{note}")
        if lp["multi_set"]:
            print("  pseudos set >1 time in loop (admissible only via "
                  f"consec_sets_invariant_p, loop.c:705): {lp['multi_set']}")
    print("\n  * nset derived from the .cse dump; approximate"
          + ("" if r["nset_reliable"] else " -- AND UNRELIABLE, see warning above"))
    print("\nLevers per term:")
    for k, v in LEVERS.items():
        print(f"  {k:12s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
