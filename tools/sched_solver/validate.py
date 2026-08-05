#!/usr/bin/env python3
"""sched_solver.validate — batch ground-truth check of the scheduler model.

Two independent checks:

  1. --blockage   the fitted function-unit blockage function vs EVERY
                  BB2_SCHED_DEBUG BLOCKAGE hook observation (the hook prints
                  raw_tick / adj_tick / max_blockage, so the compiler's own
                  blockage value is recoverable exactly).  This validates the
                  machine model in isolation, without the scheduler loop.
  2. (default)    simulate.py replayed over every basic block of every TU
                  given, scored as order-exact / clock-exact, split by pass,
                  with a per-named-function table for the banked residual
                  cases.

Usage (snapshot root):
  python3 tools/sched_solver/validate.py [stems...] [--funcs f1,f2] [--blockage]
                                         [--extract] [--json out.json]
"""
import argparse, collections, json, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
WORK = ROOT / "tmp" / "sched_solver_work"
sys.path.insert(0, str(HERE))
import simulate as S  # noqa: E402

DEFAULT_STEMS = ["config", "main", "code6cac", "display", "text1a", "text1b"]


def load(stem, do_extract):
    p = WORK / f"{stem}.sched.json"
    if do_extract or not p.exists():
        subprocess.run([sys.executable, str(HERE / "extract.py"), stem],
                       cwd=ROOT, check=True)
    return json.loads(p.read_text())


def check_blockage(models):
    """The compiler's own blockage value is adj_tick + max_blockage - raw_tick."""
    tot = ok = 0
    misses = collections.Counter()
    for m in models:
        for f in m["funcs"]:
            for blk in f["blocks"]:
                if not blk["blockage"]:
                    continue
                B = S.Block(blk)
                sim = S.Sim(B)
                for r in blk["blockage"]:
                    truth = r["adj_tick"] + r["maxb"] - r["raw_tick"]
                    got = sim.blockage(r["unit"], r["exec"], r["last"])
                    tot += 1
                    if got == truth:
                        ok += 1
                    else:
                        misses[(r["unit"], truth, got)] += 1
    pct = 100.0 * ok / tot if tot else 0.0
    print(f"blockage model: {ok}/{tot} exact ({pct:.2f}%)")
    for (u, t, g), n in misses.most_common(10):
        print(f"  unit {u}: truth {t} got {g}  x{n}")
    return ok == tot


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stems", nargs="*", default=None)
    ap.add_argument("--funcs", help="comma-separated function names to detail")
    ap.add_argument("--blockage", action="store_true")
    ap.add_argument("--extract", action="store_true")
    ap.add_argument("--json")
    a = ap.parse_args()

    stems = a.stems or DEFAULT_STEMS
    models = []
    for s in stems:
        try:
            models.append(load(s, a.extract))
        except Exception as e:
            print(f"  (skip {s}: {e})", file=sys.stderr)
    bad_parity = [m["stem"] for m in models if not m.get("parity", True)]
    if bad_parity:
        print(f"WARNING: no instrumented/build cc1 parity on: {bad_parity}")

    if a.blockage:
        return 0 if check_blockage(models) else 1

    detail = set(a.funcs.split(",")) if a.funcs else set()
    rows, per_tu, per_func = [], {}, {}
    for m in models:
        t = per_tu.setdefault(m["stem"], collections.Counter())
        for f in m["funcs"]:
            key = (m["stem"], f["func"], f["pass"])
            for blk in f["blocks"]:
                r = S.sim_block(blk, f["pass"] == 2)
                r.update(stem=m["stem"], func=f["func"], pass_=f["pass"],
                         b=blk["b"])
                rows.append(r)
                t["blocks"] += 1
                t["order"] += bool(r["match"])
                t["clock"] += bool(r["clock_match"])
                t[f"p{f['pass']}"] += 1
                t[f"p{f['pass']}ok"] += bool(r["match"])
                pf = per_func.setdefault(key, [0, 0])
                pf[0] += 1
                pf[1] += bool(r["match"])

    print(f"{'TU':<12} {'blocks':>7} {'order-exact':>14} {'clock-exact':>13} "
          f"{'sched1':>12} {'sched2':>12}")
    for stem, t in per_tu.items():
        n = t["blocks"] or 1
        print(f"{stem:<12} {t['blocks']:>7} "
              f"{t['order']:>6} ({100.0*t['order']/n:5.1f}%) "
              f"{t['clock']:>5} ({100.0*t['clock']/n:5.1f}%) "
              f"{t['p1ok']:>4}/{t['p1']:<6} {t['p2ok']:>4}/{t['p2']:<6}")
    tb = sum(t["blocks"] for t in per_tu.values())
    to = sum(t["order"] for t in per_tu.values())
    tc = sum(t["clock"] for t in per_tu.values())
    print(f"{'TOTAL':<12} {tb:>7} {to:>6} ({100.0*to/max(tb,1):5.1f}%) "
          f"{tc:>5} ({100.0*tc/max(tb,1):5.1f}%)")
    fx = sum(1 for v in per_func.values() if v[0] == v[1])
    print(f"function-passes fully exact: {fx}/{len(per_func)}")

    if detail:
        print("\nnamed functions:")
        for (stem, fn, ps), (n, k) in sorted(per_func.items()):
            if fn in detail:
                print(f"  {fn:<28} {stem:<10} pass{ps}  {k}/{n} blocks exact")

    if a.json:
        Path(a.json).write_text(json.dumps(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
