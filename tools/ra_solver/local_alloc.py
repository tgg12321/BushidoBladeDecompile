#!/usr/bin/env python3
"""ra_solver.local_alloc — Python replication of GCC 2.7.2 local-alloc.c
block_alloc (qty ordering + find_free_reg), validated against the BB2_QTY_DEBUG
ground truth produced by local_extract.py.

Two independent checks, because they need different inputs:

  ORDER  — recompute qty_compare's priority
             floor_log2(refs) * refs * size / (death - birth) * 10000
           and the qsort tie-break (lower qty number first), then compare with
           the `ord` column. Needs nothing but the dumped rows.

  ASSIGN — replay find_free_reg: used = fixed_reg_set
             | union(regs_live_at[birth .. death))
           then scan hard regs ASCENDING (MIPS defines no REG_ALLOC_ORDER in
           this tree, so find_free_reg falls through to `regno = i`), take the
           first free, and post_mark_life it over [birth, death]. Compare with
           the `got` column.

The ASSIGN replay starts from an EMPTY hard-reg liveness map, i.e. it models
only qty-vs-qty interference. Blocks whose RTL has live hard registers (incoming
argument registers, call-clobbered sets around a CALL_INSN, the return value)
are expected to diverge — those divergences are the measurement that says what
extra input a full model needs, so they are reported per block rather than
hidden.

Usage (WSL, repo root, venv active):
    python3 tools/ra_solver/local_alloc.py <stem> [--func NAME] [--verbose]
"""
import argparse
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "ra_solver_work"

FIRST_PSEUDO = 68
# mips.h FIXED_REGISTERS: $0, $at, $k0, $k1, $gp, $sp, $ra-adjacent frame regs.
FIXED = {0, 1, 26, 27, 28, 29, 31, 67}
GR_REGS = set(range(0, 32))


def floor_log2(x):
    return x.bit_length() - 1 if x > 0 else 0


def pri(refs, birth, death, size=1):
    span = death - birth
    if span <= 0:
        return 0
    return int((float(floor_log2(refs) * refs * size) / span) * 10000)


def check_order(rows):
    """Does qty_compare's formula reproduce the dumped `ord` sequence?"""
    ok = bad = 0
    detail = []
    by_block = defaultdict(list)
    for r in rows:
        by_block[(r["blk"], r["pass"])].append(r)
    for key, group in sorted(by_block.items()):
        got = [r["qty"] for r in sorted(group, key=lambda r: r["ord"])]
        want = [r["qty"] for r in sorted(
            group, key=lambda r: (-pri(r["refs"], r["birth"], r["death"]), r["qty"]))]
        if got == want:
            ok += 1
        else:
            bad += 1
            detail.append((key, got, want))
    return ok, bad, detail


def check_assign(rows):
    """Replay find_free_reg per block from an empty hard-reg liveness map.

    Scored only over the rows this model claims to cover: the MAIN pass, landing
    in GR_REGS. Two categories are counted separately rather than as failures,
    because they need inputs the hook does not dump:
      sugg   — the suggested-register pass scans qty_phys_copy_sugg/qty_phys_sugg,
               not the ascending order, so an ascending replay is meaningless.
      mdreg  — landed in $hi/$lo (64-67), i.e. the qty's class was MD_REGS.
    """
    ok = bad = 0
    skipped = defaultdict(int)
    detail = []
    by_block = defaultdict(list)
    for r in rows:
        by_block[r["blk"]].append(r)
    for blk, group in sorted(by_block.items()):
        # sugg pass runs before the main pass; within a pass, `ord` is the order
        seq = sorted(group, key=lambda r: (0 if r["pass"] == "sugg" else 1, r["ord"]))
        live = defaultdict(set)
        for r in seq:
            got = r["got"]
            if got >= 0:
                cat = None
                if r["pass"] == "sugg":
                    cat = "sugg"
                elif got not in GR_REGS:
                    cat = "mdreg"
                if cat:
                    skipped[cat] += 1
                else:
                    used = set(FIXED)
                    for i in range(r["birth"], r["death"]):
                        used |= live[i]
                    best = next((x for x in range(FIRST_PSEUDO)
                                 if x in GR_REGS and x not in used), -1)
                    if best == got:
                        ok += 1
                    else:
                        bad += 1
                        detail.append((blk, r["qty"], got, best,
                                       r["birth"], r["death"], r["refs"]))
            # post_mark_life uses the REAL assignment so later qtys stay honest
            if got >= 0:
                for i in range(r["birth"], r["death"] + 1):
                    live[i].add(got)
    return ok, bad, detail, skipped


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("--func")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()
    data = json.loads((WORK / f"{a.stem}.local.json").read_text())
    funcs = [a.func] if a.func else sorted(data)
    tot_o = tot_ob = tot_a = tot_ab = 0
    tot_skip = defaultdict(int)
    for f in funcs:
        rows = data.get(f, [])
        if not rows:
            print("%-34s no qty rows" % f)
            continue
        o, ob, od = check_order(rows)
        s, sb, sd, sk = check_assign(rows)
        tot_o += o
        tot_ob += ob
        tot_a += s
        tot_ab += sb
        for k, v in sk.items():
            tot_skip[k] += v
        print("%-34s order %2d/%-2d blocks   assign %3d/%-3d qtys%s"
              % (f, o, o + ob, s, s + sb,
                 "   (skipped %s)" % dict(sk) if sk else ""))
        if a.verbose:
            for key, got, want in od:
                print("   ORDER blk=%s pass=%s dump=%s sim=%s" % (key[0], key[1], got, want))
            for blk, q, got, sim, b, d, n in sd:
                print("   ASSIGN blk=%-3d qty=%-3d dump=%-3d sim=%-3d "
                      "(birth=%d death=%d refs=%d)" % (blk, q, got, sim, b, d, n))
    print("\nTOTAL  order %d/%d blocks (%.1f%%)   assign %d/%d qtys (%.1f%%)"
          % (tot_o, tot_o + tot_ob, 100.0 * tot_o / max(1, tot_o + tot_ob),
             tot_a, tot_a + tot_ab, 100.0 * tot_a / max(1, tot_a + tot_ab)))
    if tot_skip:
        print("out-of-model rows not scored: %s" % dict(tot_skip))


main()
