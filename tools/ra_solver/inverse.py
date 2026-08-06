#!/usr/bin/env python3
"""ra_solver.inverse — the INVERSE register-allocation solver (lever synthesizer).

The forward models (simulate.py = global.c, local_alloc.py = local-alloc.c,
reload_sim.py = reload's retry) answer *why our allocation diverged*.  This
answers the next question: **which perturbation of the solver's INPUTS would
flip our allocation to target's, and what C lever produces that perturbation.**

    (our model inputs, target assignment)
        -> minimal input perturbation sets that reach the target
        -> ranked by size then plausibility cost
        -> each mapped to candidate C techniques from .claude/rules/
           (levers.py; PLAIN / SANCTIONED-with-prereqs; FORBIDDEN never
           suggested, only listed as "do not reach for this")

A NEGATIVE result is a first-class answer: "no perturbation of any modelled
input reaches target within depth N" means the mechanism is outside the model
(the local-alloc suggested-register pass, qty_size, reload retry) and the next
move is instrumentation, not another spelling search.

This module ADDS to the solver suite; it does not modify simulate.py,
local_alloc.py or perturb.py.  (perturb.py is the earlier single-model
prototype: untyped atoms, no cost model, no policy layer, global-only.)

Usage (WSL, repo root, venv active):

  # GLOBAL alloc (global.c) — model.json from extract.py
  python3 tools/ra_solver/inverse.py global tmp/ra_solver_work/F.model.json \\
        --goal '{"12": 4, "13": 5}' [--depth 2] [--top 8]

  # LOCAL alloc (local-alloc.c block_alloc) — <stem>.local.json
  python3 tools/ra_solver/inverse.py local tmp/ra_solver_work/code6cac.local.json \\
        --func camera_set_zoom --block 41 --swap 0,1 [--depth 2]

  --goal  '{"<pseudo|qty>": <hardreg>, ...}'   the constrained subset
  --swap  A,B                                  shorthand: exchange A's and B's
                                               current assignments
"""
import argparse
import itertools
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import levers as L                                   # noqa: E402
from simulate import Sim, pri as global_pri          # noqa: E402

FIRST_PSEUDO = 68
FIXED = {0, 1, 26, 27, 28, 29, 31, 67}
GR_REGS = set(range(0, 32))

REGNAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
            "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
            "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]


def rname(r):
    if r is None:
        return "-"
    return f"${REGNAMES[r]}" if 0 <= r < 32 else f"r{r}"


# --------------------------------------------------------------------------
# An atom is one perturbation of one model input.
# --------------------------------------------------------------------------
class Atom:
    __slots__ = ("cls", "desc", "cost", "payload")

    def __init__(self, cls, desc, cost, payload):
        self.cls, self.desc, self.cost, self.payload = cls, desc, cost, payload

    def __repr__(self):
        return f"<{self.cls}: {self.desc}>"


# Plausibility cost: how big a C change the perturbation implies.  Ranking is
# (number of atoms, total cost) — a 1-atom answer always beats a 2-atom one,
# and among equals the cheaper C change wins.
COST = {
    L.BIRTH_ORDER: 1,
    L.LIVE_SHRINK: 2,
    L.LIVE_EXTEND: 2,
    L.REFS_UP: 2,
    L.REFS_DOWN: 2,
    L.CONFLICT_ADD: 2,
    L.CONFLICT_DROP: 2,
    L.PREF_REROUTE: 2,
    L.PREF_CLEAR: 2,
    L.PREF_ADD: 3,
    L.HARDREG_LIVE: 3,
    L.EXTRA_QTY: 4,
    L.CALLS_CROSSED: 4,
    L.CLASS_CHANGE: 6,
    L.ALLOC_ORDER: 9,          # deliberately worst: it explains nothing in C
}


# ==========================================================================
# Backend: GLOBAL allocation (global.c) — wraps simulate.Sim
# ==========================================================================
class GlobalBackend:
    name = "global (global.c allocno priority + find_reg)"

    def __init__(self, model_path):
        self.model = json.loads(Path(model_path).read_text())
        self.sim = Sim(self.model)
        self.units = "pseudo"

    def baseline(self):
        _, a = self.sim.simulate()
        return a

    def run(self, atoms):
        ov, order = {}, None
        for at in atoms:
            kind = at.payload[0]
            if kind == "ov":
                for p, d in at.payload[1].items():
                    ov.setdefault(p, {}).update(d)
            elif kind == "order":
                if order is not None:
                    return None            # two order atoms are not composable
                order = at.payload[1]
        try:
            _, a = self.sim.simulate(order=order, overrides=ov or None)
        except Exception:
            return None
        return a

    def atoms(self, goal):
        s = self.sim
        # Focus: the constrained pseudos plus everyone they conflict with —
        # a flip is produced either by moving the pseudo itself or by moving
        # whoever is sitting in the register it wants.
        focus = sorted(set(goal) | {q for p in goal
                                    for q in s.conf.get(p, set())
                                    if q in s.pseudos})
        out = []
        for p in focus:
            f = s.flow.get(p, {})
            nr, ll = f.get("nrefs_flow", 1), f.get("livelen_flow", 1)
            for d in (1, 2, 3):
                out.append(Atom(L.REFS_UP, f"pseudo {p}: refs {nr}->{nr + d}",
                                COST[L.REFS_UP] + d - 1,
                                ("ov", {p: {"nrefs": nr + d}})))
            for d in (-1, -2):
                if nr + d >= 1:
                    out.append(Atom(L.REFS_DOWN,
                                    f"pseudo {p}: refs {nr}->{nr + d}",
                                    COST[L.REFS_DOWN] + abs(d) - 1,
                                    ("ov", {p: {"nrefs": nr + d}})))
            for d in (-2, -4, -8):
                if ll + d >= 1:
                    out.append(Atom(L.LIVE_SHRINK,
                                    f"pseudo {p}: live length {ll}->{ll + d}",
                                    COST[L.LIVE_SHRINK],
                                    ("ov", {p: {"livelen": ll + d}})))
            for d in (2, 4, 8):
                out.append(Atom(L.LIVE_EXTEND,
                                f"pseudo {p}: live length {ll}->{ll + d}",
                                COST[L.LIVE_EXTEND],
                                ("ov", {p: {"livelen": ll + d}})))
            cc = f.get("calls_crossed", 0)
            out.append(Atom(L.CALLS_CROSSED,
                            f"pseudo {p}: calls_crossed {cc}->"
                            f"{0 if cc else 1}",
                            COST[L.CALLS_CROSSED],
                            ("ov", {p: {"calls": 0 if cc else 1}})))
            for q in focus:
                if q != p and q not in s.conf.get(p, set()):
                    out.append(Atom(L.CONFLICT_ADD,
                                    f"pseudo {p}: conflict +{q}",
                                    COST[L.CONFLICT_ADD],
                                    ("ov", {p: {"conf_add": [q]}})))
            for tp in sorted(set(goal.values())):
                cur = sorted(s.prefs.get(p, set()))
                # With no existing preference this is a genuine ADD (create a
                # copy relationship).  With one, it is a REROUTE — and the
                # distinction matters, because find_reg takes the LOWEST
                # preferred register, so adding above an existing pref is inert.
                if cur:
                    out.append(Atom(L.PREF_REROUTE,
                                    f"pseudo {p}: preference "
                                    f"{[rname(c) for c in cur]} -> "
                                    f"[{rname(tp)}] (REPLACE the copy "
                                    f"relationship)",
                                    COST[L.PREF_REROUTE],
                                    ("ov", {p: {"prefs": [tp]}})))
                else:
                    out.append(Atom(L.PREF_ADD,
                                    f"pseudo {p}: acquire a copy preference "
                                    f"for {rname(tp)} (has none today)",
                                    COST[L.PREF_ADD],
                                    ("ov", {p: {"prefs": [tp]}})))
                if cur:
                    out.append(Atom(L.PREF_ADD,
                                    f"pseudo {p}: preference += {rname(tp)}",
                                    COST[L.PREF_ADD],
                                    ("ov", {p: {"prefs": sorted(set(cur) | {tp})}})))
            if s.prefs.get(p):
                out.append(Atom(L.PREF_CLEAR, f"pseudo {p}: drop preferences",
                                COST[L.PREF_CLEAR], ("ov", {p: {"prefs": []}})))

        # Birth-order (allocno tie-break) swaps, expressed as a forced order.
        base = s.sort_order()
        for p, q in itertools.combinations(focus, 2):
            swapped = sorted(s.pseudos,
                             key=lambda x: (-s.my_pri(x),
                                            q if x == p else (p if x == q else x)))
            if swapped != base:
                out.append(Atom(L.BIRTH_ORDER,
                                f"birth order {p} <-> {q}", COST[L.BIRTH_ORDER],
                                ("order", swapped)))
        return out

    def describe(self, goal, assigned):
        return ", ".join(f"{self.units} {p}: {rname(assigned.get(p))} -> "
                         f"{rname(r)}" for p, r in sorted(goal.items()))


# ==========================================================================
# Backend: LOCAL allocation (local-alloc.c block_alloc), one basic block
# ==========================================================================
class LocalBackend:
    """Replays block_alloc for ONE block: qty_compare ordering, then
    find_free_reg (ascending scan; MIPS defines no REG_ALLOC_ORDER here) with
    post_mark_life.  Same semantics as local_alloc.py's ASSIGN check, but
    driven forward from a perturbable state rather than scored against a dump.
    """
    name = "local (local-alloc.c block_alloc)"
    units = "qty"
    caveat = (
        "LOCAL-MODE CAVEAT (measured, camera_set_zoom 2026-08-05): a birth /\n"
        "span perturbation is a claim about ALLOC-TIME order, and alloc-time\n"
        "order is not known to equal final EMISSION order.  Variant B produced\n"
        "target's emission order in the contested block and the assignment did\n"
        "NOT flip.  Treat every birth/span vector below as NECESSARY, not\n"
        "SUFFICIENT, until re-derived from a QTYDBG dump of the candidate."
    )

    def __init__(self, path, func, block):
        data = json.loads(Path(path).read_text())
        if func not in data:
            raise SystemExit(f"{func} not in {path}; have "
                             f"{len(data)} functions")
        rows = [r for r in data[func]
                if r["blk"] == block and r["pass"] == "main"]
        if not rows:
            raise SystemExit(f"{func} block {block}: no main-pass qty rows")
        self.rows = rows
        self.state = {r["qty"]: {"refs": r["refs"], "birth": r["birth"],
                                 "death": r["death"], "size": 1}
                      for r in rows}
        self.dump = {r["qty"]: r["got"] for r in rows}
        self.hard_live = {}          # pos -> set(hard regs) held by NON-qtys
        self.extra = {}              # synthetic competing quantities

    # -- forward -----------------------------------------------------------
    @staticmethod
    def _pri(q):
        span = q["death"] - q["birth"]
        if span <= 0:
            return 0
        n = q["refs"]
        fl = n.bit_length() - 1 if n > 0 else 0
        return int((float(fl * n * q["size"]) / span) * 10000)

    def _alloc(self, state, hard_live, forced_order=None):
        order = forced_order or sorted(state,
                                       key=lambda k: (-self._pri(state[k]), k))
        live, got = {}, {}
        for q in order:
            e = state[q]
            used = set(FIXED)
            for i in range(e["birth"], e["death"]):
                used |= live.get(i, set())
                used |= hard_live.get(i, set())
            r = next((x for x in range(FIRST_PSEUDO)
                      if x in GR_REGS and x not in used), -1)
            got[q] = r
            if r >= 0:
                for i in range(e["birth"], e["death"] + 1):
                    live.setdefault(i, set()).add(r)
        return order, got

    def baseline(self):
        return self._alloc(self.state, self.hard_live)[1]

    def run(self, atoms):
        state = {k: dict(v) for k, v in self.state.items()}
        hard = {k: set(v) for k, v in self.hard_live.items()}
        forced = None
        for at in atoms:
            kind, arg = at.payload[0], at.payload[1]
            if kind == "qty":
                q, field, val = arg
                if q not in state:
                    return None
                state[q][field] = val
            elif kind == "hard":
                reg, lo, hi = arg
                for i in range(lo, hi + 1):
                    hard.setdefault(i, set()).add(reg)
            elif kind == "extra":
                qid, e = arg
                state[qid] = dict(e)
            elif kind == "order":
                if forced is not None:
                    return None
                forced = list(arg)
        if forced is not None:
            forced = [q for q in forced if q in state] + \
                     [q for q in state if q not in forced]
        try:
            return self._alloc(state, hard, forced)[1]
        except Exception:
            return None

    # -- perturbations -----------------------------------------------------
    def atoms(self, goal):
        out = []
        qs = sorted(self.state)
        for q in qs:
            e = self.state[q]
            for d in (1, 2, 3, 4):
                out.append(Atom(L.REFS_UP,
                                f"qty {q}: refs {e['refs']}->{e['refs'] + d}",
                                COST[L.REFS_UP] + d - 1,
                                ("qty", (q, "refs", e["refs"] + d))))
            for d in (-1, -2):
                if e["refs"] + d >= 1:
                    out.append(Atom(L.REFS_DOWN,
                                    f"qty {q}: refs {e['refs']}->{e['refs'] + d}",
                                    COST[L.REFS_DOWN],
                                    ("qty", (q, "refs", e["refs"] + d))))
            for nb in range(e["birth"] + 1, e["death"]):
                out.append(Atom(L.LIVE_SHRINK,
                                f"qty {q}: born later ({e['birth']}->{nb}); "
                                f"span {e['death'] - e['birth']}->"
                                f"{e['death'] - nb}",
                                COST[L.LIVE_SHRINK],
                                ("qty", (q, "birth", nb))))
            for nd in range(e["birth"] + 1, e["death"]):
                out.append(Atom(L.LIVE_SHRINK,
                                f"qty {q}: dies earlier ({e['death']}->{nd}); "
                                f"span {e['death'] - e['birth']}->"
                                f"{nd - e['birth']}",
                                COST[L.LIVE_SHRINK],
                                ("qty", (q, "death", nd))))
            for d in (1, 2, 4):
                out.append(Atom(L.LIVE_EXTEND,
                                f"qty {q}: dies later "
                                f"({e['death']}->{e['death'] + d})",
                                COST[L.LIVE_EXTEND],
                                ("qty", (q, "death", e["death"] + d))))
                if e["birth"] - d >= 0:
                    out.append(Atom(L.LIVE_EXTEND,
                                    f"qty {q}: born earlier "
                                    f"({e['birth']}->{e['birth'] - d})",
                                    COST[L.LIVE_EXTEND],
                                    ("qty", (q, "birth", e["birth"] - d))))

        # A hard register genuinely occupied across the contested span: the
        # measured second half of the title_mv_exec2 two-part requirement.
        lo = min(e["birth"] for e in self.state.values())
        hi = max(e["death"] for e in self.state.values())
        for reg in sorted(set(goal.values()) | {2, 3, 4, 5}):
            if reg in FIXED:
                continue
            out.append(Atom(L.HARDREG_LIVE,
                            f"hard {rname(reg)} live across [{lo},{hi}) "
                            f"(a real arg/return value spanning the block)",
                            COST[L.HARDREG_LIVE], ("hard", (reg, lo, hi))))

        # One more real competing quantity overlapping the region.
        nxt = max(self.state) + 100
        for pr_refs, span in ((2, 2), (4, 4), (2, hi - lo)):
            e = {"refs": pr_refs, "birth": lo, "death": min(lo + span, hi),
                 "size": 1}
            out.append(Atom(L.EXTRA_QTY,
                            f"one more temp: refs={pr_refs} span=[{e['birth']},"
                            f"{e['death']}) pri={self._pri(e)}",
                            COST[L.EXTRA_QTY], ("extra", (nxt, e))))
            nxt += 1

        # Forced allocation order — the "no modelled input explains it" probe.
        base = sorted(self.state, key=lambda k: (-self._pri(self.state[k]), k))
        for perm in itertools.permutations(base):
            if list(perm) != base and len(base) <= 6:
                out.append(Atom(L.ALLOC_ORDER,
                                f"allocation order forced to {list(perm)} "
                                f"(no refs/span/birth change)",
                                COST[L.ALLOC_ORDER], ("order", list(perm))))
        return out

    def describe(self, goal, assigned):
        return ", ".join(f"{self.units} {q}: {rname(assigned.get(q))} -> "
                         f"{rname(r)}" for q, r in sorted(goal.items()))


# ==========================================================================
# Search
# ==========================================================================
def matches(assigned, goal):
    return assigned is not None and \
        all(assigned.get(k) == v for k, v in goal.items())


def search(backend, goal, depth, top):
    base = backend.baseline()
    print(f"backend    : {backend.name}")
    print(f"baseline   : "
          f"{ {k: rname(v) for k, v in sorted(base.items())} }")
    print(f"goal       : {backend.describe(goal, base)}")
    if matches(base, goal):
        print("\nbaseline ALREADY matches the goal — nothing to invert.")
        return []

    atoms = backend.atoms(goal)
    print(f"atom space : {len(atoms)} single perturbations "
          f"over {len(set(a.cls for a in atoms))} classes\n")
    if getattr(backend, "caveat", None):
        print(backend.caveat + "\n")

    hits = []
    seen_desc = set()
    for k in range(1, depth + 1):
        for combo in itertools.combinations(atoms, k):
            if len({a.cls for a in combo}) < len(combo) and k > 1:
                # allow same-class pairs only when they touch different units
                pass
            res = backend.run(list(combo))
            if matches(res, goal):
                key = " AND ".join(sorted(a.desc for a in combo))
                if key in seen_desc:
                    continue
                seen_desc.add(key)
                hits.append((k, sum(a.cost for a in combo), list(combo)))
        if hits:
            print(f"minimal solution size: {k} atom(s) — "
                  f"{len(hits)} distinct vector(s)\n")
            break

    if not hits:
        return []

    hits.sort(key=lambda h: (h[0], h[1]))
    return hits[:top]


def report(hits, backend):
    for i, (n, cost, combo) in enumerate(hits, 1):
        print(f"--- #{i}  ({n} atom(s), cost {cost}) "
              + "-" * max(0, 46 - len(str(i))))
        for a in combo:
            print(f"  [{a.cls}] {a.desc}")
        print(L.format_report(sorted({a.cls for a in combo})))
        print()
    print(L.forbidden_block())


def negative_report(backend, goal, depth):
    print("=" * 74)
    print(f"NEGATIVE RESULT: no perturbation of any modelled input, up to "
          f"depth {depth},")
    print("reaches the target assignment.")
    print()
    print("This is a validated answer, not a failure.  It means the flip is "
          "not\nproduced by refs / live span / birth order / conflicts / "
          "preferences /\ncalls-crossed at all, so no C spelling that only "
          "moves those will ever\nclose it.  The next move is INSTRUMENTATION, "
          "not another spelling search:")
    print()
    for slug, tier, how in L.LEVERS[L.ALLOC_ORDER]:
        for line in L._wrap(how, 70):
            print(f"  {line}")
    print()
    print(L.forbidden_block())


# ==========================================================================
def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="mode", required=True)

    g = sub.add_parser("global", help="global.c allocation (model.json)")
    g.add_argument("model")

    lo = sub.add_parser("local", help="local-alloc.c block_alloc (.local.json)")
    lo.add_argument("model")
    lo.add_argument("--func", required=True)
    lo.add_argument("--block", type=int, required=True)

    for p in (g, lo):
        p.add_argument("--goal", help='{"<unit>": <hardreg>, ...}')
        p.add_argument("--swap", help="A,B — exchange their current registers")
        p.add_argument("--depth", type=int, default=2)
        p.add_argument("--top", type=int, default=8)

    a = ap.parse_args()
    if a.mode == "global":
        backend = GlobalBackend(a.model)
    else:
        backend = LocalBackend(a.model, a.func, a.block)

    if a.swap:
        x, y = (int(t) for t in a.swap.split(","))
        base = backend.baseline()
        goal = {x: base.get(y), y: base.get(x)}
    elif a.goal:
        goal = {int(k): v for k, v in json.loads(a.goal).items()}
    else:
        ap.error("one of --goal / --swap is required")

    hits = search(backend, goal, a.depth, a.top)
    if hits:
        report(hits, backend)
    else:
        negative_report(backend, goal, a.depth)
    return 0


if __name__ == "__main__":
    sys.exit(main())
