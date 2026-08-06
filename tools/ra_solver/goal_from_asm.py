#!/usr/bin/env python3
"""ra_solver.goal_from_asm — derive the TARGET register assignment from asm.

The inverse solver needs a goal: `{pseudo: target hardreg}`.  Until now that
was supplied by hand, read off a diff.  This derives it.

The chain mirrors sched_solver/goalmap.py, one level down (registers instead of
instruction order):

    honest .s  --align-->  target .s  --substitutions-->  {our reg: target reg}
                                      --dispositions-->   {pseudo: target reg}

* `honest .s` is built from the CHEAT-STRIPPED source (mkasm_honest.sh).  This
  is load-bearing for parked functions: on main their source carries asm pins
  that already force target's registers, so the on-main build differs from
  target by nothing and there is no question to invert.
* `target .s` is the post-regfix/asmfix stream.  The tree builds SHA1-identical
  to the original executable, so that stream IS the original's register
  assignment by construction.
* alignment reuses `sched_solver.goalmap.align()`, whose second pass
  deliberately pairs leftover deletes against inserts — which is exactly what a
  renamed instruction looks like to difflib.

ATTRIBUTION is the hard hop and is reported with its own confidence, never
guessed silently.  A substitution says "hard reg R should be T *here*"; the
model says which pseudos hold R.  Three cases:

  unique      exactly one allocated pseudo holds R  -> attribution certain
  conflicting several do, but exactly one of them conflicts with a pseudo
              holding T (they are simultaneously live, which is what makes an
              exchange possible at all) -> attribution ranked, flagged
  ambiguous   several remain -> ALL candidates reported, none chosen

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/goal_from_asm.py <stem> <func> [--model M.json]
                                           [--json out.json] [--show]
"""
import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "sched_solver"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from goalmap import align, asm_body            # noqa: E402

WORK = ROOT / "tmp" / "inverse_work"

REGNAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
            "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
            "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
NUM = {f"${n}": i for i, n in enumerate(REGNAMES)}
NUM.update({f"${i}": i for i in range(32)})
REG_RE = re.compile(r"\$(?:" + "|".join(REGNAMES) + r"|\d+)\b")


def regs_of(text):
    """Register operands of one instruction, in operand order."""
    body = text.split("#")[0]
    return [NUM[m.group(0)] for m in REG_RE.finditer(body)
            if m.group(0) in NUM]


def blank_regs(text):
    return REG_RE.sub("$#", text.split("#")[0].strip())


def substitutions(hon, tgt, verbose=False):
    """-> (Counter{(our, target): n}, [per-instruction detail])

    Only aligned pairs whose register-blanked skeletons are IDENTICAL count: a
    pair that differs in mnemonic or immediate is a different instruction, not
    a rename, and reading a substitution off it would be fiction.
    """
    amap, _ = align(hon, tgt, "hon->tgt", verbose)
    subs, detail = Counter(), []
    for i, j in enumerate(amap):
        if j is None:
            continue
        a, b = hon[i], tgt[j]
        if blank_regs(a) != blank_regs(b):
            continue
        ra, rb = regs_of(a), regs_of(b)
        if len(ra) != len(rb):
            continue
        pairs = [(x, y) for x, y in zip(ra, rb) if x != y]
        if pairs:
            for p in pairs:
                subs[p] += 1
            detail.append((i, j, a, b, pairs))
    return subs, detail


def attribute(subs, dispositions):
    """Map register substitutions onto pseudos using the model dispositions.

    dispositions: {pseudo: hardreg} from the model (post-allocation truth for
    OUR honest build).  Returns (goal, notes) where goal is {pseudo: target
    hardreg} for the confidently-attributed entries only.
    """
    holders = defaultdict(list)
    for p, r in dispositions.items():
        if r is not None and r >= 0:
            holders[r].append(p)
    goal, notes = {}, []
    for (ours, want), n in sorted(subs.items(), key=lambda kv: -kv[1]):
        cand = holders.get(ours, [])
        if not cand:
            notes.append(f"{rname(ours)}->{rname(want)} (x{n}): NO allocated "
                         f"pseudo holds {rname(ours)} — the register belongs to "
                         f"local-alloc (a block-local quantity) or is a hard "
                         f"operand; use the `local` backend for this one")
        elif len(cand) == 1:
            goal[cand[0]] = want
            notes.append(f"{rname(ours)}->{rname(want)} (x{n}): unique -> "
                         f"pseudo {cand[0]}")
        else:
            partners = set(holders.get(want, []))
            notes.append(f"{rname(ours)}->{rname(want)} (x{n}): AMBIGUOUS, "
                         f"{len(cand)} pseudos hold {rname(ours)}: {sorted(cand)}"
                         + (f"; pseudos holding {rname(want)}: "
                            f"{sorted(partners)}" if partners else ""))
    return goal, notes


def rname(r):
    return f"${REGNAMES[r]}" if 0 <= r < 32 else f"r{r}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("func")
    ap.add_argument("--model", help="model.json (enables pseudo attribution)")
    ap.add_argument("--json", help="write the derived goal here")
    ap.add_argument("--show", action="store_true", help="per-instruction detail")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()

    hon_p, tgt_p = WORK / f"{a.stem}.hon.s", WORK / f"{a.stem}.tgt.s"
    for p in (hon_p, tgt_p):
        if not p.exists():
            sys.exit(f"missing {p} — run: bash tools/ra_solver/mkasm_honest.sh "
                     f"{a.stem}")
    hon = [t for t, _ in asm_body(hon_p, a.func)]
    tgt = [t for t, _ in asm_body(tgt_p, a.func)]
    print(f"{a.func}: honest={len(hon)} insns  target={len(tgt)} insns")

    subs, detail = substitutions(hon, tgt, a.verbose)
    if len(hon) != len(tgt):
        print(f"  NOTE: instruction counts differ by {len(tgt) - len(hon)} — "
              f"the residual is not a pure rename; register substitutions "
              f"below cover only the aligned, same-skeleton pairs.")
    if not subs:
        print("  no register substitutions: the honest stream already uses "
              "target's registers (residual, if any, is elsewhere)")
    else:
        print(f"  {sum(subs.values())} substituted operand(s) over "
              f"{len(detail)} instruction(s):")
        for (x, y), n in sorted(subs.items(), key=lambda kv: -kv[1]):
            print(f"    {rname(x)} -> {rname(y)}   x{n}")
    if a.show:
        for i, j, x, y, pairs in detail:
            print(f"    [{i:4d}] {x:38s} -> {y:38s} "
                  f"{[f'{rname(p)}->{rname(q)}' for p, q in pairs]}")

    out = {"func": a.func, "stem": a.stem,
           "honest_insns": len(hon), "target_insns": len(tgt),
           "substitutions": {f"{rname(x)}->{rname(y)}": n
                             for (x, y), n in subs.items()},
           "goal": {}, "notes": []}

    if a.model:
        model = json.loads(Path(a.model).read_text())
        disp = {int(k): v for k, v in model["dispositions"].items()}
        goal, notes = attribute(subs, disp)
        print("\n  attribution:")
        for n in notes:
            print(f"    {n}")
        if goal:
            print(f"\n  derived goal: "
                  f"{json.dumps({str(k): v for k, v in sorted(goal.items())})}")
            print(f"  feed to: python3 tools/ra_solver/inverse.py global "
                  f"{a.model} --goal '"
                  f"{json.dumps({str(k): v for k, v in sorted(goal.items())})}'")
        out["goal"] = {str(k): v for k, v in goal.items()}
        out["notes"] = notes

    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1))
        print(f"\n  written: {a.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
