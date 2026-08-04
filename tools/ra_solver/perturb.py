#!/usr/bin/env python3
"""ra_solver.perturb — search minimal model perturbations that make the
simulated allocation match a TARGET assignment.

Usage:
  python3 tools/ra_solver/perturb.py <model.json> '<spec-json>' [--pairs] [--greedy]

spec: {"<pseudo>": <hardreg>, ...} — the constrained subset (from the
function's fdiff role mapping); unlisted pseudos are free.

Atoms (each maps to a C-level phenomenon):
  refs ±1/±2   a real extra/removed use
  live ±N      live range shortened/lengthened (statement span)
  birth p<->q  first-definition order swap (tie-break control)
  conf +q      a conflict edge (live ranges made to overlap)
  pref +r      a copy relationship toward hard reg r (arg/return/copy flow)

Search: singles; --pairs adds unordered pairs; --greedy locks one spec entry
at a time using an accumulating atom stack (finds k-atom solutions greedily).
"""
import itertools, json, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from simulate import Sim


def matches(assigned, spec):
    return all(assigned.get(p) == r for p, r in spec.items())


LEVERS = {
    "refs": "add/remove a REAL reference (use) of the value",
    "live": "shorten/extend the live range (def placement / block-local split)",
    "birth": "swap first-definition order (statement/decl order)",
    "conf": "make the two live ranges genuinely overlap",
    "pref": "create a copy relationship toward that hard reg "
            "(init from / feed into the value living there)",
}


def gen_atoms(s: Sim, spec):
    focus = sorted(set(spec)
                   | {q for p in spec for q in s.conf.get(p, set())
                      if q in s.pseudos})
    atoms = []
    for p in focus:
        f = s.flow.get(p, {})
        nr = f.get("nrefs_flow", 1)
        ll = f.get("livelen_flow", 1)
        for d in (1, 2, -1):
            if nr + d >= 1:
                atoms.append(({p: {"nrefs": nr + d}}, None,
                              f"pseudo {p}: refs{d:+d}", LEVERS["refs"]))
        for d in (-2, -4, -8, 2, 4):
            if ll + d >= 1:
                atoms.append(({p: {"livelen": ll + d}}, None,
                              f"pseudo {p}: live{d:+d}", LEVERS["live"]))
        for q in focus:
            if q > p:
                base = s.sort_order()
                swapped = sorted(
                    s.pseudos,
                    key=lambda x: (-s.my_pri(x),
                                   q if x == p else (p if x == q else x)))
                if swapped != base:
                    atoms.append(({}, swapped,
                                  f"birth {p}<->{q}", LEVERS["birth"]))
        for q in focus:
            if q != p and q not in s.conf.get(p, set()):
                atoms.append(({p: {"conf_add": [q]}}, None,
                              f"pseudo {p}: conf+{q}", LEVERS["conf"]))
        for tp in sorted(set(spec.values())):
            atoms.append(({p: {"prefs": sorted(set(s.prefs.get(p, set()))
                                               | {tp})}}, None,
                          f"pseudo {p}: pref+r{tp}", LEVERS["pref"]))
            atoms.append(({p: {"prefs": [tp]}}, None,
                          f"pseudo {p}: pref=r{tp} (reroute: REPLACE the "
                          f"existing copy relationship)", LEVERS["pref"]))
        if s.prefs.get(p):
            atoms.append(({p: {"prefs": []}}, None,
                          f"pseudo {p}: pref-clear (remove the copy "
                          f"relationship)", LEVERS["pref"]))
    return atoms


def merge_ov(*ovs):
    out = {}
    for ov in ovs:
        for k, v in ov.items():
            out.setdefault(k, {}).update(v)
    return out


def main():
    model = json.loads(Path(sys.argv[1]).read_text())
    spec = {int(k): v for k, v in json.loads(sys.argv[2]).items()}
    s = Sim(model)
    _, base = s.simulate()
    print("baseline sim:", {p: base.get(p) for p in spec})
    print("target spec :", spec)
    if matches(base, spec):
        print("baseline already matches")
        return

    atoms = gen_atoms(s, spec)
    print(f"searching {len(atoms)} single atoms...")
    hits = []
    for ov, order, dsc, lev in atoms:
        try:
            _, a = s.simulate(order=order, overrides=ov or None)
        except Exception:
            continue
        if matches(a, spec):
            hits.append((1, dsc))
            print(f"  HIT(1): {dsc}\n         lever: {lev}")

    if not hits and "--pairs" in sys.argv:
        print("searching pairs...")
        for (o1, r1, d1, l1), (o2, r2, d2, l2) in \
                itertools.combinations(atoms, 2):
            if r1 is not None and r2 is not None:
                continue
            order = r1 if r1 is not None else r2
            try:
                _, a = s.simulate(order=order, overrides=merge_ov(o1, o2) or None)
            except Exception:
                continue
            if matches(a, spec):
                hits.append((2, f"{d1} AND {d2}"))
                print(f"  HIT(2): {d1}  AND  {d2}")

    if not hits and "--greedy" in sys.argv:
        locked, stack, cur_order = {}, [], None
        remaining = dict(spec)
        progress = True
        while remaining and progress:
            progress = False
            for p in sorted(remaining, key=lambda x: -s.my_pri(x)):
                sub = dict(locked)
                sub[p] = remaining[p]
                for ov, order, dsc, lev in atoms:
                    if order is not None and cur_order is not None:
                        continue
                    o = order if order is not None else cur_order
                    try:
                        _, a = s.simulate(order=o,
                                          overrides=merge_ov(*stack, ov) or None)
                    except Exception:
                        continue
                    if matches(a, sub):
                        stack.append(ov)
                        if order is not None:
                            cur_order = order
                        locked = sub
                        del remaining[p]
                        progress = True
                        print(f"  GREEDY lock {p}->{sub[p]} via: {dsc}")
                        break
                if progress:
                    break
        if not remaining:
            print(f"GREEDY SUCCESS with {len(stack)} atoms"
                  + (" + 1 order swap" if cur_order else ""))
            hits.append((len(stack), "greedy"))
        else:
            print(f"greedy stalled; unresolved: {remaining}; "
                  f"locked so far: {locked}")

    if not hits:
        print("NO sufficient vector in searched space — likely needs "
              "class/seed/retry-level change or an out-of-space mechanism")


if __name__ == "__main__":
    main()
