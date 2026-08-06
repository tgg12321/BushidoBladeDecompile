#!/usr/bin/env python3
"""ra_solver.pseudo_scope — narrow pseudo attribution by SCOPE.

THE PROBLEM
-----------
`goal_from_asm.attribute()` maps a register substitution ("$v0 should be $v1")
onto pseudos by asking "which allocated pseudos hold $v0". That question is
position-blind, so on a function of any size it returns everything: on
DispPracticeMenuTex_A, 85 pseudos hold $v0, and attribute() correctly refuses to
choose, returning an empty goal.

WHAT THIS ADDS
--------------
A substitution happens at a specific INSTRUCTION, which sits in a specific basic
block. A pseudo that lives entirely inside a DIFFERENT block cannot be the one
being renamed there. That single observation does most of the work.

cc1's `.lreg` dump (post-local-alloc) states each pseudo's scope directly:

    Register 87 used 5 times across 8 insns in block 0; GR_REGS or none.
    Register 85 used 4 times across 10 insns; GR_REGS or none; pointer.

The `in block N` clause appears only for pseudos confined to ONE block. Its
ABSENCE is equally informative: that pseudo spans blocks, so it is a global
allocno and could be live at the site.

So for a site in block B, the candidates are:
    pseudos scoped to block B   +   pseudos with no block (multi-block)
and everything scoped to another block is excluded outright.

MEASURED (DispPracticeMenuTex_A, block 3): 85 $v0 candidates -> 5
(two block-local, three multi-block), and the $v1 side -> exactly ONE (pseudo
100). That was enough to derive a real goal and get a decisive answer from
inverse.py global.

WHY NOT FULL INSN-LEVEL LIVENESS
--------------------------------
Exact liveness needs the pre-allocation RTL insn stream aligned to the emitted
asm — a four-hop chain (object -> hon.s -> cc1.s -> dbr uids -> .lreg pseudos),
each hop its own alignment with its own failure mode. Block scope is ONE hop,
reads a fact cc1 states in plain text, and already narrows enough to act. Build
the RTL chain only if a case survives this filter ambiguous AND matters.

HONESTY
-------
This NARROWS a candidate set; it never picks. When more than one candidate
survives, that is reported and no goal entry is emitted — same contract as
attribute(). A confident wrong attribution produces a goal that looks
authoritative and is arbitrary, which is worse than no goal.

Usage (WSL, repo root, venv active):
    python3 tools/ra_solver/pseudo_scope.py <stem> <func> [--work DIR]
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# "Register 87 used 5 times across 8 insns in block 0; ..."   -> block-local
# "Register 85 used 4 times across 10 insns; ..."             -> multi-block
_REG = re.compile(r"^Register (\d+) used (\d+) times across (\d+) insns"
                  r"(?: in block (\d+))?", re.M)


def lreg_segment(stem, func, work="tmp/ra_solver_work"):
    p = ROOT / work / f"{stem}.i.lreg"
    if not p.exists():
        raise SystemExit(
            f"missing {p} — produced by `tools/ra_solver/extract.py {func} {stem}` "
            f"(it runs cc1 -da).")
    parts = re.split(r"^;; Function (\w+)\s*$", p.read_text(errors="replace"),
                     flags=re.M)
    for i in range(1, len(parts), 2):
        if parts[i] == func:
            return parts[i + 1]
    raise SystemExit(f"{func} not found in {p.name}")


def scopes(stem, func, work="tmp/ra_solver_work"):
    """-> (block_of {pseudo: block}, multi_block {pseudo}, refs {pseudo: n})."""
    seg = lreg_segment(stem, func, work)
    block_of, multi, refs = {}, set(), {}
    for m in _REG.finditer(seg):
        p = int(m.group(1))
        refs[p] = int(m.group(2))
        if m.group(4) is not None:
            block_of[p] = int(m.group(4))
        else:
            multi.add(p)
    return block_of, multi, refs


def candidates(holders, blk, block_of, multi):
    """Pseudos from HOLDERS that could be live in block BLK."""
    return sorted(p for p in holders
                  if block_of.get(p) == blk or p in multi)


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("stem")
    ap.add_argument("func")
    ap.add_argument("--work", default="tmp/ra_solver_work")
    a = ap.parse_args()
    block_of, multi, refs = scopes(a.stem, a.func, a.work)
    print(f"{a.func} ({a.stem}): {len(block_of)} block-local pseudo(s), "
          f"{len(multi)} multi-block (global) pseudo(s)")
    byblk = {}
    for p, b in block_of.items():
        byblk.setdefault(b, []).append(p)
    for b in sorted(byblk):
        print(f"  block {b:3d}: {sorted(byblk[b])}")
    print(f"  multi-block: {sorted(multi)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
