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
`size` is PSEUDO_REGNO_SIZE = ceil(mode bytes / 4), read from the pseudo's
mode in the RTL (SI/SF -> 1, DI/DF -> 2, TI/TF -> 4). pri(calc) == pri(hook)
on every row is the validation that the size model (and the whole model) is
right — the census flags any row where they disagree.

RTL comes from the .lreg dump, NOT .greg: the .greg dump is printed AFTER
global allocation, so every pseudo has already been renumbered to a hard reg
and pseudo numbers do not appear in it at all (measured on
tmp/closer/ings2dbg/ings2.i.greg: zero 2+-digit pseudo regs). .lreg still
carries `(reg:SI 73)` and is banner-split by `;; Function <name>`, so it is
name-keyed rather than index-matched. There is deliberately NO .greg
fallback: a whole-file search would happily match another function's RTL.

FREE REFERENCE LIFTS (the sanctioned duplicated-statement-into-arms family,
2026-07-01). flow.c adds `loop_depth` per reference, not 1
(flow.c:2081/2329/2515/2725), so: each duplicated copy adds
R = (occurrences of the pseudo in the statement) x (loop depth at that site);
sinking a statement into N arms and deleting the original adds (N-1)xR;
adding a fresh copy in each of N arms while the original survives adds NxR
(the post-mortem's 24->96 was the NxR form). Emitted bytes stay at zero only
if cross-jump re-merges the tails; measure with sweep_variants after using
this calculator — the calculator says how many, not whether it is free.

CAVEAT — shared allocnos. reg_may_share (global.c) can fold several pseudos
into ONE allocno, summing their n_refs and taking the max live_length. The
hook prints one row per allocno keyed by its representative pseudo, so a
one-row-per-pseudo census cannot express that sharing; a lift applied to a
shared pseudo moves the whole allocno.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from tools.ra_solver import extract as X  # noqa: E402

# top-of-line RTL block starters in a cc1 -da dump
_BLOCK_RE = re.compile(
    r"^\((insn|call_insn|jump_insn|note|code_label|barrier)\b", re.M)

# PSEUDO_REGNO_SIZE = ceil(GET_MODE_SIZE(mode) / UNITS_PER_WORD), UNITS_PER_WORD=4
_MODE_SIZE = {"QI": 1, "HI": 1, "SI": 1, "SF": 1, "PSI": 1,
              "DI": 2, "DF": 2, "SC": 2, "PDI": 2,
              "TI": 4, "TF": 4, "DC": 4, "XF": 4}


def priority(nrefs: int, livelen: int, size: int = 1) -> int:
    """global.c:615 / allocno_compare, exactly.

    global.c:569-572 rewrites a live length of 0 to -1 (a length of 0 means
    "make an allocno but don't allocate it"), which yields a large NEGATIVE
    priority — so only nrefs<=0 short-circuits to 0 here. floor_log2 is the
    bit-length form, matching gcc's floor_log2.
    """
    if nrefs <= 0:
        return 0
    if livelen == 0:
        livelen = -1
    return int((((nrefs.bit_length() - 1) * nrefs) / livelen) * 10000 * size)


def _pri(r: dict) -> int:
    return priority(r["nrefs"], r["livelen"], r.get("size", 1))


def reorder(rows: list) -> list:
    """global.c allocno_compare: higher priority first; equal priorities are
    resolved by ASCENDING allocno number (global.c:651-655, `return *v1 - *v2`
    — "so that the results of qsort leave nothing to chance"). Allocnos are
    created in ascending pseudo-regno order, so the pseudo number is the key."""
    keyed = sorted(rows, key=lambda r: (-_pri(r), r["pseudo"]))
    return [dict(r, pri_calc=_pri(r)) for r in keyed]


def _row(rows: list, p: int) -> dict:
    for r in rows:
        if r["pseudo"] == p:
            return r
    raise SystemExit(f"pseudo {p} has no allocno row; allocated pseudos: "
                     f"{sorted(r['pseudo'] for r in rows)}")


LIVELEN_CAVEAT = (
    "assumes livelen unchanged — true only if the duplicate sits strictly inside the "
    "pseudo's existing live range (flow.c:2087 reg_live_length++ per live insn); "
    "otherwise the printed k is a FLOOR — re-measure.")

LIFT_ARITHMETIC = (
    "  free-lift arithmetic: each duplicated copy adds R = (occurrences of the pseudo in the\n"
    "  statement) x (loop depth at that site) — flow.c adds loop_depth per reference, not 1.\n"
    "  Sinking a statement into N arms and deleting the original adds (N-1)xR; adding a fresh\n"
    "  copy in each of N arms while the original survives adds NxR (the post-mortem's 24->96\n"
    "  was the NxR form).")


def min_lift_to_beat(rows: list, p: int, q: int, cap: int = 4096) -> int:
    """Smallest k with priority(nrefs_p + k) strictly above q's priority.

    Assumes livelen unchanged — true only if the duplicate sits strictly
    inside the pseudo's existing live range (flow.c:2087 reg_live_length++ per
    live insn); otherwise the returned k is a FLOOR — re-measure. Returns -1
    if no k below `cap` suffices.
    """
    rp, rq = _row(rows, p), _row(rows, q)
    target = _pri(rq)
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


def mode_size(rtl: str, pseudo: int) -> int:
    """PSEUDO_REGNO_SIZE for `pseudo` from its mode in the RTL (default 1).

    Matches both plain `(reg:DI 76)` and flagged `(reg/v:SI 73)`.
    """
    m = re.search(r"\(reg(?:/\w+)*:(\w+)\s+%d\)" % pseudo, rtl)
    return _MODE_SIZE.get(m.group(1), 1) if m else 1


def banner_segment(dump: str, func: str) -> str:
    """Name-keyed dump segment (banner `;; Function <name>`); "" if absent."""
    banners = [(m.start(), m.group(1))
               for m in re.finditer(r"^;; Function (\S+)", dump, re.M)]
    for k, (s, name) in enumerate(banners):
        if name == func:
            e = banners[k + 1][0] if k + 1 < len(banners) else len(dump)
            return dump[s:e]
    return ""


def format_census(rows: list) -> tuple:
    """(lines, mismatched_pseudos) — the census table in allocation order.

    A row whose pri(hook) != pri(calc) carries a MISMATCH marker: the size /
    mode model is wrong for that pseudo and the what-if answers are unsound.
    """
    lines = ["  ord pseudo hard nrefs livelen  pri(hook) pri(calc) xcalls  first def"]
    bad = []
    for r in reorder(rows):
        flag = ""
        if r["pri_calc"] != r.get("pri", r["pri_calc"]):
            flag = "  <-- pri MISMATCH (size/mode?)"
            bad.append(r["pseudo"])
        lines.append(
            f"  {r.get('ord', -1):>3} {r['pseudo']:>6} {r.get('hardreg', -1):>4} "
            f"{r['nrefs']:>5} {r['livelen']:>7} {r.get('pri', 0):>10} {r['pri_calc']:>9} "
            f"{str(r.get('calls_crossed')):>6}  {r.get('first_def', '')[:70]}{flag}")
    return lines, bad


def parse_lift(spec: str) -> tuple:
    """'P=+K' -> (P, K). Raises SystemExit with a clean message on garbage."""
    m = re.match(r"^\s*(\d+)\s*=\s*([+-]?\d+)\s*$", spec)
    if not m:
        raise SystemExit(f"bad --lift {spec!r}: expected P=+K (e.g. 73=+72)")
    return int(m.group(1)), int(m.group(2))


CAP = 4096


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

    # .flow: prefer the name-keyed banner; .ent index only when there are no banners.
    flow_seg = banner_segment(d["flow"], a.func)
    if not flow_seg and ";; Function" not in d["flow"]:
        ents = X.ent_order(d["asm"])
        fi = ents.index(a.func) if a.func in ents else -1
        segs = X.split_flow(d["flow"])
        flow_seg = segs[fi] if 0 <= fi < len(segs) else ""
    flow_regs = X.parse_flow_regs(flow_seg) if flow_seg else {}

    # .lreg keeps pseudo numbers (.greg is post-renumber). No .greg fallback:
    # a whole-file search could match a different function's RTL.
    rtl = banner_segment(d["lreg"], a.func)
    if not rtl:
        print(f"no .lreg segment for {a.func} — first-def/size unavailable", file=sys.stderr)
    for r in rows:
        r["size"] = mode_size(rtl, r["pseudo"]) if rtl else 1
        r["calls_crossed"] = flow_regs.get(r["pseudo"], {}).get("calls_crossed")
        r["first_def"] = first_def_insn(rtl, r["pseudo"]) if rtl else ""

    print(f"allocno census — {a.func}  (order = global.c allocno_compare; pri = global.c:615)")
    lines, bad = format_census(rows)
    print("\n".join(lines))
    if bad:
        print(f"\nWARNING: pri(calc) != pri(hook) for pseudo(s) {bad} — the size/mode model is "
              "wrong there; the what-if answers below are unreliable.")

    if a.above:
        try:
            p, q = (int(x) for x in a.above.split(":"))
        except ValueError:
            raise SystemExit(f"bad --above {a.above!r}: expected P:Q (e.g. 73:76)")
        rp = _row(rows, p)
        _row(rows, q)
        k = min_lift_to_beat(rows, p, q, cap=CAP)
        if k < 0:
            print(f"\npseudo {p} cannot out-rank {q} by nrefs alone within {CAP} refs")
        else:
            print(f"\nlift needed: pseudo {p} (nrefs {rp['nrefs']}) must gain +{k} refs to rank "
                  f"above pseudo {q}")
            print(LIFT_ARITHMETIC)
            print(f"  caveat: {LIVELEN_CAVEAT}")
    if a.lift:
        lifted = [dict(r) for r in rows]
        for spec in a.lift:
            p, k = parse_lift(spec)
            _row(rows, p)
            for r in lifted:
                if r["pseudo"] == p:
                    r["nrefs"] += k
        print("\nre-sorted under lift " + ", ".join(a.lift) + ":")
        for r in reorder(lifted):
            print(f"  pseudo {r['pseudo']:>4}  nrefs {r['nrefs']:>4}  pri {r['pri_calc']:>9}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
