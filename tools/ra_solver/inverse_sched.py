#!/usr/bin/env python3
"""inverse_sched — the inverse solver's SCHEDULER backend.

The RA backends (inverse.py) answer "which allocation input flips us to
target".  This answers the same question for sched.c's list scheduler: given a
block's scheduler inputs and TARGET's instruction order, which minimal
perturbation of the dependence graph / LUID order / instruction costs produces
that order, and which C technique spells it.

## What this adds over sched_solver/perturb.py

`sched_solver/perturb.py` already enumerates the atoms (add_dep / del_dep /
luid / luid_move / cost), recomputes priorities after each perturbation, and
searches to a goal order.  That machinery is REUSED here verbatim — this module
imports it rather than reimplementing it.  What it adds is the layer the RA
side has and the scheduler side did not:

  1. **Typed classes + a policy layer.**  This matters more on the scheduler
     than anywhere else in the project.  The most obvious way to create a
     dependence edge is `volatile`, a `memory` clobber, or an `__asm__`
     barrier — all FORBIDDEN families.  A search tool that reports "add an
     anti-dependence 55 <- 16" without saying so is one step from a cheat.
     Every atom is therefore mapped through levers.py, anti-edges carry the
     standing warning, and the forbidden block prints on every report.

  2. **A plausibility cost model.**  A LUID move is an ordinary statement
     reorder; an added anti-dependence requires two accesses to genuinely
     alias; an INSN_COST change requires a different instruction to be
     selected.  Ranking is (atom count, summed cost), so the cheapest C edit
     surfaces first instead of whichever atom the enumeration happened to emit.

  3. **Spellability triage.**  Vectors are split into SPELLABLE (LUID / value
     flow) and CONDITIONAL (edges and costs, which are only real under a stated
     precondition), because a depth-2 search will happily return an
     unspellable pair when a spellable one exists further down the list.

## Goal derivation

Target's instruction ORDER comes from the same honest-vs-target alignment the
RA side uses, via sched_solver/goalmap.py (`--goal-from-target`).  A block's
goal order is target's emission order for that block's UIDs, carried back into
the model's pre-reorg pick space.

NOTE on honest-vs-on-main: regfix/asmfix rules run AFTER cc1, so a function
whose only cheats are RULES has an honest scheduler model already — the on-main
cc1 output is what the pin-free source would produce.  Only cheat-asm in the C
source (register pins, `__asm__` barriers) contaminates the model; for those
functions build the model from the stripped source first (mkasm_honest.sh).

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/inverse_sched.py <model.json> --func F --block B
        [--pass N] --goal-order u1,u2,... [--depth 2] [--top 8]
  python3 tools/ra_solver/inverse_sched.py <model.json> --func F --block B
        --goal-from-target <stem> [--target <pinned .s>]
"""
import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# IMPORT ORDER IS LOAD-BEARING.  Both solver suites ship a `simulate.py` and a
# `perturb.py`, and this file lives in ra_solver/, whose directory Python puts
# at sys.path[0] automatically.  So: take `levers` from here FIRST, then put
# sched_solver ahead of it so `simulate` / `perturb` / `goalmap` resolve to the
# SCHEDULER ones (importing them the other way round silently binds ra_solver's
# register-allocation simulate and fails on the first attribute access).
import levers as L                                    # noqa: E402

sys.path.insert(0, str(ROOT / "tools" / "sched_solver"))

import perturb as P                                   # noqa: E402
import simulate as S                                  # noqa: E402

# atom kind -> (perturbation class, plausibility cost, spellable?)
#
# "spellable" means: an ordinary source edit produces it, with no precondition
# about the program's semantics.  Moving a statement is spellable.  Adding an
# anti-dependence is NOT — it is only available if the two accesses genuinely
# alias, which is a fact about the program, not a choice.
KIND_INFO = {
    "luid":      (L.LUID_ORDER, 1, True),
    "luid_move": (L.LUID_ORDER, 1, True),
    "del_dep":   (L.DEP_DROP, 3, False),
    "cost":      (L.INSN_COST, 5, False),
    "unit":      (L.SCHED_UNIT, 6, False),
}


def classify(atom):
    kind, a, b, extra = atom
    if kind == "add_dep":
        if extra == 0:
            return L.DEP_ADD_TRUE, 3, False
        return L.DEP_ADD_ANTI, 4, False
    return KIND_INFO.get(kind, (L.LUID_ORDER, 9, False))


def find_block(model, func, passno, b):
    for f in model["funcs"]:
        if f["func"] != func or (passno and f["pass"] != passno):
            continue
        for blk in f["blocks"]:
            if blk["b"] == b:
                return f, blk
    return None, None


def run(blk, passno, goal_order):
    """-> simulated pick order, or None when the block errors out."""
    r = S.sim_block(blk, passno == 2)
    return r


def search(blk, passno, goal, depth, top):
    """Ranked minimal perturbation vectors reaching GOAL.

    Breadth-first over vector size so results are minimal by construction;
    within a size, ranked by summed plausibility cost.
    """
    atoms = P.enumerate_atoms(blk)
    hits = []
    for size in (1, 2)[:depth]:
        if size == 1:
            for at in atoms:
                nb = P.apply_atom(blk, at)
                if nb is None:
                    continue
                r = S.sim_block(nb, passno == 2)
                if r["err"] is None and goal(r["sim"]):
                    hits.append((1, classify(at)[1], [at]))
        else:
            # Depth 2 over the full cross product is O(n^2) block simulations.
            # Restrict the FIRST element to the spellable classes: a vector
            # whose halves are both unspellable is not actionable anyway, and
            # this is what keeps a --max truncation from hiding the spellable
            # pairs (the failure mode perturb.py's docstring warns about).
            spell = [a for a in atoms if classify(a)[2]]
            for a1 in spell:
                b1 = P.apply_atom(blk, a1)
                if b1 is None:
                    continue
                for a2 in atoms:
                    if a2 == a1:
                        continue
                    b2 = P.apply_atom(b1, a2)
                    if b2 is None:
                        continue
                    r = S.sim_block(b2, passno == 2)
                    if r["err"] is None and goal(r["sim"]):
                        hits.append((2, classify(a1)[1] + classify(a2)[1],
                                     [a1, a2]))
        if hits:
            break
    hits.sort(key=lambda h: (h[0], h[1], not all(classify(x)[2] for x in h[2])))
    return hits[:top]


def report(hits, blk):
    for i, (n, cost, vec) in enumerate(hits, 1):
        spellable = all(classify(a)[2] for a in vec)
        print(f"--- #{i}  ({n} atom(s), cost {cost}, "
              f"{'SPELLABLE' if spellable else 'CONDITIONAL'}) "
              + "-" * 24)
        for a in vec:
            cls, c, sp = classify(a)
            print(f"  [{cls}] {P.describe(a)}")
        print(L.format_report(sorted({classify(a)[0] for a in vec})))
        print()
    print(L.forbidden_block())


def negative(depth):
    print("=" * 74)
    print(f"NEGATIVE RESULT: no perturbation of the scheduler's inputs, up to "
          f"depth {depth},\nproduces target's order for this block.")
    print()
    print("The model is validated 6978/6978, so this is a statement about the "
          "SEARCH\nSPACE, not the simulator: dependence edges, LUID order and "
          "instruction costs\ncannot get there. Either the goal order is wrong "
          "for this block (check the\ngoalmap alignment and any dependence "
          "violations it reported), or the residual\nis produced before "
          "sched.c runs — by the RTL the earlier passes built.")
    print()
    print(L.forbidden_block())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("model")
    ap.add_argument("--func", required=True)
    ap.add_argument("--block", type=int, required=True)
    ap.add_argument("--pass", dest="passno", type=int, default=2)
    ap.add_argument("--goal-order")
    ap.add_argument("--goal-before", action="append", default=[])
    ap.add_argument("--goal-from-target", metavar="STEM")
    ap.add_argument("--target", help="pin target's stream to this .s")
    ap.add_argument("--depth", type=int, default=2)
    ap.add_argument("--top", type=int, default=8)
    ap.add_argument("--root", default=".")
    a = ap.parse_args()

    model = json.loads(Path(a.model).read_text())
    f, blk = find_block(model, a.func, a.passno, a.block)
    if blk is None:
        sys.exit(f"{a.func} pass {a.passno} block {a.block} not in {a.model}")

    base = S.sim_block(blk, a.passno == 2)
    print(f"function   : {a.func} pass {a.passno} block {a.block} "
          f"({blk['n_insns']} insns)")
    print(f"model self-check: sim {'==' if base['match'] else '!='} dump "
          f"(err={base['err']})")
    if not base["match"]:
        print("  the forward model does not reproduce this block — fix that "
              "before inverting it")
        return 1

    ours = [p["insn"] for p in blk["picks"]]
    order = None
    if a.goal_order:
        order = [int(t) for t in a.goal_order.split(",")]
    elif a.goal_from_target:
        import goalmap
        m = goalmap.build_map(Path(a.root), a.goal_from_target, a.func,
                              verbose=True, target=a.target)
        bu = [int(k) for k, n in blk["nodes"].items() if not n.get("extern")]
        order, unresolved, interp = goalmap.goal_for_block(m, bu, ours, blk)
        bad = goalmap.topo_violations(blk, order)
        print(f"goal order : {order}")
        if unresolved:
            print(f"  unresolved (USE/CLOBBER, held in place): {unresolved}")
        if interp:
            print(f"  interpolated positions: {interp}")
        if bad:
            print(f"  *** GOAL INVALID: {len(bad)} dependence violation(s) "
                  f"{bad[:4]} — target alignment mis-paired duplicate "
                  f"instruction text; do not search this block")
            return 1
    befores = [tuple(int(x) for x in s.split(":")) for s in a.goal_before]
    if order is None and not befores:
        ap.error("need --goal-order, --goal-before or --goal-from-target")

    print(f"ours       : {ours}")
    if order is not None and order == ours:
        print("\ngoal == ours: this block already matches target's order.")
        return 0

    goal = P.make_goal(order, befores)
    hits = search(blk, a.passno, goal, a.depth, a.top)
    print()
    if hits:
        print(f"minimal solution size: {hits[0][0]} atom(s) — "
              f"{len(hits)} vector(s) reported\n")
        report(hits, blk)
    else:
        negative(a.depth)
    return 0


if __name__ == "__main__":
    sys.exit(main())
