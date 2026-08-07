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


# ---------------------------------------------------------------------------
# Suggested-register pass (local-alloc.c:1466-1490 + find_free_reg:2161-2223).
# Needs <stem>.sugg.json from `local_extract.py --suggest`.
# ---------------------------------------------------------------------------
def sugg_key(r):
    """qty_sugg_compare's primary key: copy suggestions rank ahead of plain
    ones, and within each group FEWER suggestions is higher priority
    (local-alloc.c:1660)."""
    return (r["ncopysugg"] if r["ncopysugg"]
            else r["nsugg"] * FIRST_PSEUDO)


def sugg_order(rows):
    """qty_sugg_compare_1: sugg key ascending, then priority descending, then
    qty number ascending."""
    return sorted(rows, key=lambda r: (
        sugg_key(r), -pri(r["refs"], r["birth"], r["death"], r["size"]),
        r["qty"]))


def check_sugg(blocks):
    """Two independent checks against the SUGGDBG ground truth.

    Recomputes find_free_reg's `first_used` from the dumped `used` plus the
    qty's suggestion sets (copy-suggested wins if non-empty, else
    arithmetic-suggested; local-alloc.c:2169-2175) and compares with the dumped
    `first_used`. This tests the suggestion preference logic itself; the
    resulting assignment is scored separately by check_sugg_assign.
    """
    pref_ok = pref_bad = 0
    detail = []
    for blk, b in sorted(blocks.items()):
        ffrs = b.get("_ffr", [])
        qtys = {k: v for k, v in b.items() if k != "_ffr"}
        seen = defaultdict(int)
        for f in ffrs:
            if not f["jts"]:
                continue                      # main pass, covered by check_assign
            r = qtys.get(str(f["qty"]))
            if r is None:
                continue
            # find_free_reg:2216 — when the copy-suggested scan fails and plain
            # suggestions also exist, it zeroes qty_phys_num_copy_sugg and
            # recurses. So the SECOND jts=1 call for a qty scans the plain set
            # even though the dumped (pre-pass) ncopysugg is non-zero.
            retry = seen[f["qty"]] > 0
            seen[f["qty"]] += 1
            used = set(f["used"])
            prefset = (r["copysugg"] if r["ncopysugg"] and not retry
                       else r["sugg"])
            # IOR_COMPL: everything OUTSIDE the suggestion set becomes unusable.
            want = used | {x for x in range(FIRST_PSEUDO) if x not in prefset}
            if want == set(f["first_used"]):
                pref_ok += 1
            else:
                pref_bad += 1
                detail.append((blk, f["qty"],
                               sorted(want ^ set(f["first_used"]))))
    return pref_ok, pref_bad, detail


def check_sugg_assign(blocks, got_rows):
    """Scan check scored against the real `got` from the QTYDBG-SUGG lines.

    One qty can produce several jts=1 find_free_reg calls: the copy-suggested
    scan, then (local-alloc.c:2216) the plain-suggested retry after it fails,
    then (2229) the caller-saves retry. Only the chain's outcome is an
    assignment, so the calls are folded into one result -- the first scan that
    finds a free register -- rather than scored individually.
    """
    ok = bad = 0
    detail = []
    bygot = {(r["blk"], r["qty"]): r["got"]
             for r in got_rows if r["pass"] == "sugg"}
    for blk, b in sorted(blocks.items()):
        chain = defaultdict(list)
        for f in b.get("_ffr", []):
            if f["jts"]:
                chain[f["qty"]].append(f)
        for q, calls in sorted(chain.items()):
            got = bygot.get((int(blk), q))
            if got is None or got < 0:
                continue
            best = -1
            for f in calls:
                free = set(f["first_used"])
                best = next((x for x in range(FIRST_PSEUDO) if x not in free), -1)
                if best >= 0:
                    break
            if best == got:
                ok += 1
            else:
                bad += 1
                detail.append((blk, q, got, best))
    return ok, bad, detail


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("--func")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--suggest", action="store_true",
                    help="also score the suggested-register pass from "
                         "<stem>.sugg.json (local_extract.py --suggest)")
    a = ap.parse_args()
    data = json.loads((WORK / f"{a.stem}.local.json").read_text())
    sugg = {}
    if a.suggest:
        spath = WORK / f"{a.stem}.sugg.json"
        if not spath.exists():
            raise SystemExit("%s missing - run: python3 tools/ra_solver/"
                             "local_extract.py %s --suggest" % (spath, a.stem))
        sugg = json.loads(spath.read_text())
    funcs = [a.func] if a.func else sorted(data)
    tot_o = tot_ob = tot_a = tot_ab = 0
    tot_p = tot_pb = tot_s = tot_sb = 0
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
        if a.suggest:
            blocks = sugg.get(f, {})
            p, pb, pd = check_sugg(blocks)
            s2, s2b, s2d = check_sugg_assign(blocks, rows)
            tot_p += p; tot_pb += pb; tot_s += s2; tot_sb += s2b
            if p + pb + s2 + s2b:
                print("%-34s   SUGG prefer %d/%-3d   assign %d/%-3d"
                      % ("", p, p + pb, s2, s2 + s2b))
            if a.verbose:
                for blk, q, sym in pd:
                    print("   SUGG-PREFER blk=%s qty=%-3d set-mismatch on %s"
                          % (blk, q, sym))
                for blk, q, got, sim in s2d:
                    print("   SUGG-ASSIGN blk=%s qty=%-3d dump=%-3d sim=%-3d"
                          % (blk, q, got, sim))
    print("\nTOTAL  order %d/%d blocks (%.1f%%)   assign %d/%d qtys (%.1f%%)"
          % (tot_o, tot_o + tot_ob, 100.0 * tot_o / max(1, tot_o + tot_ob),
             tot_a, tot_a + tot_ab, 100.0 * tot_a / max(1, tot_a + tot_ab)))
    if a.suggest:
        print("SUGG   prefer %d/%d (%.1f%%)   assign %d/%d (%.1f%%)"
              % (tot_p, tot_p + tot_pb,
                 100.0 * tot_p / max(1, tot_p + tot_pb),
                 tot_s, tot_s + tot_sb,
                 100.0 * tot_s / max(1, tot_s + tot_sb)))
    if tot_skip:
        print("out-of-model rows not scored: %s" % dict(tot_skip))


main()
