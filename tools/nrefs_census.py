#!/usr/bin/env python3
"""reg_n_refs / allocno-order census with a what-if lift calculator.

WHY (post-mortem 2026-09-01): func_800324D0 named the mechanism in s6
(duplicated tail statements raise the walker's reg_n_refs before
allocno_compare ranks it; cross-jump re-merges them for free) and priced the
threshold in s20 (nrefs ~50), but nobody joined the two until s22. This
tool prints the allocation order with every input, and answers "how many
extra references does P need to rank above Q".

Usage (WSL, venv active, repo root):
  python3 tools/nrefs_census.py --func <func> --file <stem>                 # census
  python3 tools/nrefs_census.py --func <func> --file <stem> --above 73:76   # min lift for 73 to beat 76
  python3 tools/nrefs_census.py --func <func> --file <stem> --lift 73=+72   # re-sort under a hypothetical lift

Inputs come from the INSTRUMENTED cc1 (BB2_ALLOC_DEBUG, tools/gcc-2.7.2/cc1)
via tools/ra_solver/extract.run_dumps — read-only, work files under
tmp/ra_solver_work/. Priority formula is global.c:615 exactly:
    floor(log2(nrefs)) * nrefs / livelen * 10000 * size
`size` is 1 for SImode pseudos and 2 for DImode; the census assumes 1 unless
the pseudo's mode in the RTL is DI/DF. pri(calc) == pri(hook) on every row is
the validation that the size guess (and the whole model) is right — the
census flags any row where they disagree.

RTL comes from the .lreg dump, NOT .greg: the .greg dump is printed AFTER
global allocation, so every pseudo has already been renumbered to a hard reg
and pseudo numbers do not appear in it at all (measured on
tmp/closer/ings2dbg/ings2.i.greg: zero 2+-digit pseudo regs). .lreg still
carries `(reg:SI 73)` and is banner-split by `;; Function <name>`, so it is
name-keyed rather than index-matched.

Free reference lifts (the sanctioned duplicated-statement-into-arms family,
2026-07-01) raise nrefs by (arms-1) * refs-per-statement with zero emitted
bytes IF cross-jump re-merges the tails; measure with sweep_variants after
using this calculator — the calculator says how many, not whether it is free.
"""
from __future__ import annotations

import argparse
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from tools.ra_solver import extract as X  # noqa: E402

# top-of-line RTL block starters in a cc1 -da dump
_BLOCK_RE = re.compile(
    r"^\((insn|call_insn|jump_insn|note|code_label|barrier)\b", re.M)


def priority(nrefs: int, livelen: int, size: int = 1) -> int:
    if nrefs <= 0 or livelen <= 0:
        return 0
    return int(((math.floor(math.log2(nrefs)) * nrefs) / livelen) * 10000 * size)


def reorder(rows: list) -> list:
    """global.c allocno_compare: higher priority first; ties keep input order
    (qsort is not stable, so ties are reported as ties, not resolved)."""
    keyed = [(priority(r["nrefs"], r["livelen"], r.get("size", 1)), -i, r)
             for i, r in enumerate(rows)]
    keyed.sort(key=lambda t: (t[0], t[1]), reverse=True)
    return [dict(r, pri_calc=p) for p, _, r in keyed]


def min_lift_to_beat(rows: list, p: int, q: int, cap: int = 4096) -> int:
    rp = next(r for r in rows if r["pseudo"] == p)
    rq = next(r for r in rows if r["pseudo"] == q)
    target = priority(rq["nrefs"], rq["livelen"], rq.get("size", 1))
    for k in range(0, cap):
        if priority(rp["nrefs"] + k, rp["livelen"], rp.get("size", 1)) > target:
            return k
    return -1


def rtl_blocks(rtl: str):
    """Split a cc1 -da RTL dump into whole insn blocks.

    Real dump insns span multiple lines and end with `<code> {template} (nil)`
    + a REG_* note list, so a line-oriented or `) -1`-anchored regex misses
    them. Blocks run from one top-of-line `(insn|call_insn|jump_insn|note|
    code_label|barrier` to the next. The single-line `) -1 (nil) (nil)` form
    is just a one-line block, so both forms work.
    """
    starts = [m.start() for m in _BLOCK_RE.finditer(rtl)]
    for k, s in enumerate(starts):
        e = starts[k + 1] if k + 1 < len(starts) else len(rtl)
        yield rtl[s:e].rstrip()


def first_def_insn(rtl: str, pseudo: int) -> str:
    """The first insn block that SETs/CLOBBERs `pseudo`, collapsed to one line.

    `(reg:SI 73)` and flagged forms `(reg/v:SI 73)` both count; the closing
    paren straight after the number is what distinguishes a pseudo from a
    hard reg (`(reg:SI 4 a0)`).
    """
    dest = re.compile(r"\((?:set|clobber) \(reg[/:][^ ]*\s+%d\)" % pseudo)
    for blk in rtl_blocks(rtl):
        if not blk.startswith(("(insn", "(call_insn", "(jump_insn")):
            continue
        flat = " ".join(blk.split())
        if dest.search(flat):
            return flat[:160]
    return ""


def _mode_size(rtl: str, pseudo: int) -> int:
    m = re.search(r"\(reg[/:][^ ]*?:(\w+)\s+%d\)" % pseudo, rtl)
    return 2 if m and m.group(1) in ("DI", "DF") else 1


def _lreg_segment(lreg: str, func: str) -> str:
    """Name-keyed .lreg segment (banner `;; Function <name>`)."""
    banners = [(m.start(), m.group(1))
               for m in re.finditer(r"^;; Function (\S+)", lreg, re.M)]
    for k, (s, name) in enumerate(banners):
        if name == func:
            e = banners[k + 1][0] if k + 1 < len(banners) else len(lreg)
            return lreg[s:e]
    return ""


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--above", help="P:Q — minimum nrefs lift for pseudo P to rank above Q")
    ap.add_argument("--lift", action="append", default=[],
                    help="P=+K — re-sort with pseudo P lifted by K (repeatable)")
    a = ap.parse_args()

    d = X.run_dumps(a.file)
    dbg = X.split_allocdbg(d["stderr"])
    rows = dbg["rows"].get(a.func)
    if not rows:
        print(f"no ALLOCDBG rows for {a.func} — is the instrumented cc1 at tools/gcc-2.7.2/cc1 and "
              "the function a C body in src/ (not INCLUDE_ASM)?", file=sys.stderr)
        return 1
    ents = X.ent_order(d["asm"])
    fi = ents.index(a.func) if a.func in ents else -1
    flow_segs = X.split_flow(d["flow"])
    flow_seg = flow_segs[fi] if 0 <= fi < len(flow_segs) else ""
    flow_regs = X.parse_flow_regs(flow_seg) if flow_seg else {}
    # .lreg keeps pseudo numbers (.greg is post-renumber); name-keyed banner split.
    rtl = _lreg_segment(d["lreg"], a.func)
    if not rtl:
        greg_segs = X.split_greg(d["greg"])
        rtl = greg_segs[fi] if 0 <= fi < len(greg_segs) else d["greg"]
    for r in rows:
        r["size"] = _mode_size(rtl, r["pseudo"])
        r["calls_crossed"] = flow_regs.get(r["pseudo"], {}).get("calls_crossed")
        r["first_def"] = first_def_insn(rtl, r["pseudo"])

    print(f"allocno census — {a.func}  (order = global.c allocno_compare; pri = global.c:615)")
    print("  ord pseudo hard nrefs livelen  pri(hook) pri(calc) xcalls  first def")
    mismatches = 0
    for r in reorder(rows):
        flag = "" if r["pri_calc"] == r["pri"] else "  <-- pri MISMATCH (size/mode?)"
        mismatches += 1 if flag else 0
        print(f"  {r['ord']:>3} {r['pseudo']:>6} {r['hardreg']:>4} {r['nrefs']:>5} {r['livelen']:>7} "
              f"{r['pri']:>10} {r['pri_calc']:>9} {str(r['calls_crossed']):>6}  "
              f"{r['first_def'][:70]}{flag}")
    if mismatches:
        print(f"\nWARNING: {mismatches} row(s) where pri(calc) != pri(hook) — the size/mode model "
              "is wrong for those pseudos; the what-if answers below are unreliable.")

    if a.above:
        p, q = (int(x) for x in a.above.split(":"))
        k = min_lift_to_beat(rows, p, q)
        rp = next(r for r in rows if r["pseudo"] == p)
        print(f"\nlift needed: pseudo {p} (nrefs {rp['nrefs']}) must gain +{k} refs to rank above pseudo {q}"
              if k >= 0 else f"\npseudo {p} cannot out-rank {q} by nrefs alone within 4096 refs")
        if k > 0:
            print("  free-lift arithmetic: each arm that duplicates a statement with R refs to the pseudo adds R;\n"
                  f"  N arms x R refs per statement >= {k}  (e.g. 12 arms x 6 refs = 72)")
    if a.lift:
        lifted = [dict(r) for r in rows]
        for spec in a.lift:
            p, k = spec.split("=")
            for r in lifted:
                if r["pseudo"] == int(p):
                    r["nrefs"] += int(k.replace("+", ""))
        print("\nre-sorted under lift " + ", ".join(a.lift) + ":")
        for r in reorder(lifted):
            print(f"  pseudo {r['pseudo']:>4}  nrefs {r['nrefs']:>4}  pri {r['pri_calc']:>9}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
