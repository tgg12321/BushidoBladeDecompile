#!/usr/bin/env python3
"""sched_solver.perturb — search for the input change that yields TARGET's order.

The simulator reproduces our schedule exactly (README: 6978/6978 blocks), so
the question can be asked in reverse: what minimal perturbation of a block's
SCHEDULER INPUTS produces the order the target binary has?

Each atom is chosen to be a thing C can actually control:

| atom | scheduler input | what it is in C |
|---|---|---|
| `add_dep A<-B kind0` | a true dependence edge | a value produced by one statement and consumed by the other -- variable identity, or a load that must follow a store |
| `add_dep A<-B kind!=0` | an anti/output edge | a write ordered against a read/write of the same location: aliasing, taking an address, a volatile access |
| `del_dep A<-B` | drop an edge | breaking that aliasing / splitting the value |
| `luid A B` | INSN_LUID order | the source order of the two generating statements -- the final tie-break, and the cheapest atom to spell |
| `cost N c` | INSN_COST | instruction selection: a load (ready 2) vs a move (1), a multiply (12) vs a shift |
| `unit N u` | function unit | same, seen from the hazard side |

Priorities are NOT perturbed directly: `priority()` is a pure function of the
dependence graph and the costs, so it is RECOMPUTED after every perturbation
(`recompute_priorities`).  Perturbing priority on its own would be modelling a
change no C edit can make in isolation.  `--self-check` validates the
recomputation against every dumped INSN_PRIORITY.

Goals:
  --goal-order u1,u2,...   the output must equal this exact sequence
  --goal-before A:B        (repeatable) A must appear before B in the output

Usage:
  python3 tools/sched_solver/perturb.py <model.json> --func F --pass N --block B
          [--goal-before 55:16] [--goal-order ...] [--depth 1|2] [--max N]
  python3 tools/sched_solver/perturb.py <model.json> --self-check
"""
import argparse, copy, itertools, json, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import simulate as S  # noqa: E402

TAIL_FLOOR = 0x7f000000   # LOW_PRIORITY_P: pinned tail insns sit above this


# --- priority() -------------------------------------------------------------
def recompute_priorities(blk):
    """sched.c priority(): max over LOG_LINKS of (priority(pred) + insn_cost - 1),
    floor 1.  Insns the tail loop pinned (TAIL_PRIORITY - i, always above
    0x7f000000) keep their dumped value -- that pin is positional, not derived.
    Returns {uid: priority}."""
    B = S.Block(blk)
    pri, stack = {}, []

    def cost(prev, kind, used):
        return B.insn_cost(prev, kind, used)

    def calc(uid, seen):
        if uid in pri:
            return pri[uid]
        n = B.nodes.get(uid)
        if n is None or n.get("extern"):
            return 0
        if n["pri"] >= TAIL_FLOOR:
            pri[uid] = n["pri"]
            return pri[uid]
        if uid in seen:          # cycles cannot occur in LOG_LINKS; be safe
            return 1
        seen = seen | {uid}
        m = 1
        for prev, kind in B.deps.get(uid, []):
            v = calc(prev, seen) + cost(prev, kind, uid) - 1
            if v > m:
                m = v
        pri[uid] = m
        return m

    sys.setrecursionlimit(10000)
    for uid in B.nodes:
        calc(uid, frozenset())
    return pri


def apply_priorities(blk):
    """Return a copy of BLK with every node's `pri` recomputed from its graph."""
    out = copy.deepcopy(blk)
    pri = recompute_priorities(out)
    for uid, p in pri.items():
        k = str(uid)
        if k in out["nodes"]:
            out["nodes"][k]["pri"] = p
    return out


def self_check(model):
    """Recomputed priorities must equal the dumped INSN_PRIORITY everywhere."""
    tot = ok = 0
    bad = []
    for f in model["funcs"]:
        for blk in f["blocks"]:
            pri = recompute_priorities(blk)
            for k, n in blk["nodes"].items():
                if n.get("extern"):
                    continue
                tot += 1
                if pri.get(int(k)) == n["pri"]:
                    ok += 1
                elif len(bad) < 8:
                    bad.append((f["func"], f["pass"], blk["b"], k,
                                n["pri"], pri.get(int(k))))
    pct = 100.0 * ok / tot if tot else 0.0
    print(f"priority recomputation: {ok}/{tot} exact ({pct:.2f}%)")
    for row in bad:
        print(f"  {row[0]} pass{row[1]} blk{row[2]} insn {row[3]}: "
              f"dumped {row[4]} recomputed {row[5]}")
    return ok == tot


# --- atoms ------------------------------------------------------------------
def uids(blk):
    return sorted(int(k) for k, n in blk["nodes"].items()
                  if not n.get("extern"))


def reachable(deps, src, dst, seen=None):
    """Is DST reachable from SRC along LOG_LINKS (i.e. does SRC depend on DST)?"""
    if seen is None:
        seen = set()
    for p, _ in deps.get(str(src), []):
        if p == dst:
            return True
        if p not in seen:
            seen.add(p)
            if reachable(deps, p, dst, seen):
                return True
    return False


def enumerate_atoms(blk, kinds=(0, 14)):
    """All single-atom perturbations that keep the graph acyclic."""
    out = []
    us = uids(blk)
    for a in us:
        have = {p for p, _ in blk["deps"].get(str(a), [])}
        for b in us:
            if a == b:
                continue
            if b in have:
                out.append(("del_dep", a, b, None))
                continue
            # adding a<-b requires b not already depending on a
            if reachable(blk["deps"], b, a):
                continue
            for kd in kinds:
                out.append(("add_dep", a, b, kd))
    for a, b in itertools.combinations(us, 2):
        out.append(("luid", a, b, None))
    for a in us:
        n = blk["nodes"][str(a)]
        for c in (1, 2, 3, 12):
            if c != n["icost"]:
                out.append(("cost", a, c, None))
    return out


def apply_atom(blk, atom):
    kind, a, b, extra = atom
    out = copy.deepcopy(blk)
    if kind == "add_dep":
        out["deps"].setdefault(str(a), []).append([b, extra])
        out["nodes"][str(b)]["ref"] += 1
    elif kind == "del_dep":
        lst = [d for d in out["deps"].get(str(a), []) if d[0] != b]
        if len(lst) == len(out["deps"].get(str(a), [])):
            return None
        out["deps"][str(a)] = lst
        out["nodes"][str(b)]["ref"] -= 1
    elif kind == "luid":
        na, nb = out["nodes"][str(a)], out["nodes"][str(b)]
        na["luid"], nb["luid"] = nb["luid"], na["luid"]
    elif kind == "cost":
        out["nodes"][str(a)]["icost"] = b
    return apply_priorities(out)


def describe(atom):
    kind, a, b, extra = atom
    if kind == "add_dep":
        w = "true/data" if extra == 0 else f"anti-output(kind {extra})"
        return f"add_dep {a} <- {b} ({w})"
    if kind == "del_dep":
        return f"del_dep {a} <- {b}"
    if kind == "luid":
        return f"luid swap {a} <-> {b} (source statement order)"
    return f"cost {a} := {b} (instruction selection)"


# --- goals ------------------------------------------------------------------
def make_goal(order, befores):
    def goal(out):
        if order is not None and out != order:
            return False
        pos = {u: i for i, u in enumerate(out)}
        for a, b in befores:
            if a not in pos or b not in pos or pos[a] > pos[b]:
                return False
        return True
    return goal


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("model")
    ap.add_argument("--func")
    ap.add_argument("--pass", dest="passno", type=int, default=2)
    ap.add_argument("--block", type=int)
    ap.add_argument("--goal-order")
    ap.add_argument("--goal-before", action="append", default=[])
    ap.add_argument("--depth", type=int, default=1)
    ap.add_argument("--max", type=int, default=40)
    ap.add_argument("--self-check", action="store_true")
    a = ap.parse_args()

    model = json.loads(Path(a.model).read_text())
    if a.self_check:
        return 0 if self_check(model) else 1

    blk = fn = None
    for f in model["funcs"]:
        if f["func"] == a.func and f["pass"] == a.passno:
            for b in f["blocks"]:
                if a.block is None or b["b"] == a.block:
                    blk, fn = b, f
                    break
    if blk is None:
        print(f"no block: func={a.func} pass={a.passno} block={a.block}")
        return 2

    base = S.sim_block(blk, a.passno == 2)
    print(f"{a.func} pass{a.passno} block{blk['b']}: {blk['n_insns']} insns, "
          f"baseline {'exact' if base['match'] else 'MISMATCH'}")
    print(f"  our order: {base['sim']}")

    order = ([int(x) for x in a.goal_order.split(",")] if a.goal_order
             else None)
    befores = [tuple(int(y) for y in x.split(":")) for x in a.goal_before]
    if not order and not befores:
        print("  (no goal given -- nothing to search; pass --goal-order or "
              "--goal-before)")
        return 0
    goal = make_goal(order, befores)
    if goal(base["sim"]):
        print("  baseline ALREADY satisfies the goal")
        return 0

    atoms = enumerate_atoms(blk)
    print(f"  searching {len(atoms)} single atoms"
          + (f" + pairs (depth {a.depth})" if a.depth > 1 else ""))
    hits = []
    for at in atoms:
        pb = apply_atom(blk, at)
        if pb is None:
            continue
        r = S.sim_block(pb, a.passno == 2)
        if r["err"] is None and goal(r["sim"]):
            hits.append((at, r["sim"]))
            if len(hits) >= a.max:
                break
    if a.depth > 1 and not hits:
        for a1, a2 in itertools.combinations(atoms, 2):
            p1 = apply_atom(blk, a1)
            if p1 is None:
                continue
            p2 = apply_atom(p1, a2)
            if p2 is None:
                continue
            r = S.sim_block(p2, a.passno == 2)
            if r["err"] is None and goal(r["sim"]):
                hits.append(((a1, a2), r["sim"]))
                if len(hits) >= a.max:
                    break

    if not hits:
        print("  NO single-atom perturbation reaches the goal "
              "(the order is not reachable by one input change)")
        return 1
    print(f"  {len(hits)} vector(s) reach the goal:")
    for at, out in hits:
        d = (describe(at) if isinstance(at[0], str)
             else " + ".join(describe(x) for x in at))
        print(f"    {d}\n      -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
