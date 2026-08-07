#!/usr/bin/env python3
"""ra_solver.local_extract — pull LOCAL-alloc (local-alloc.c) ground truth.

Phase 5 companion to extract.py, which covers global.c only.

Mechanism: the instrumented cc1 (tools/gcc-2.7.2/cc1) carries a BB2_QTY_DEBUG
hook in block_alloc that prints one line per quantity per basic block, for both
allocation loops:

    QTYDBG-SUGG blk=B ord=I qty=Q reg1=R birth=X death=Y refs=N got=REG
    QTYDBG      blk=B ord=I qty=Q reg1=R birth=X death=Y refs=N got=REG

`ord` is the position in qty_order after the sort (qty_sugg_compare for the
suggested pass, qty_compare for the main pass), so the stream records the exact
allocation order local-alloc used. `got` is the ground truth to reproduce.

QTYDBG lines are NOT function-tagged (block numbers restart per function). They
are segmented here by the fact that cc1 runs local_alloc before global_alloc for
each function, so every QTYDBG line belongs to the function named by the NEXT
`ALLOCDBG func=` line in the stream.

Usage (WSL, repo root, venv active):
    python3 tools/ra_solver/local_extract.py <stem> [--func NAME]
writes tmp/ra_solver_work/<stem>.local.json
"""
import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "ra_solver_work"
CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin "
       "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ "
       "-D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = ROOT / "tools" / "gcc-2.7.2" / "cc1"
CC1_FLAGS = ("-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 "
             "-mno-abicalls -fno-builtin -w -mel")

QTY = re.compile(
    r"^QTYDBG(?P<sugg>-SUGG)? blk=(?P<blk>-?\d+) ord=(?P<ord>-?\d+) "
    r"qty=(?P<qty>-?\d+) reg1=(?P<reg1>-?\d+) birth=(?P<birth>-?\d+) "
    r"death=(?P<death>-?\d+) refs=(?P<refs>-?\d+) got=(?P<got>-?\d+)")
FUNC = re.compile(r"^ALLOCDBG func=(?P<func>\S+)")

# --- BB2_SUGG_DEBUG (--suggest only; see cc1_hooks.patch.md section 7) --------
# SUGGDBG-QTY is the complete per-qty input table, printed BEFORE the suggested-
# register pass runs (find_free_reg's retry clears qty_phys_num_copy_sugg, so a
# later dump would under-report).  SUGGDBG-FFR is one line per find_free_reg
# call carrying the two hard-reg sets it scans.
SUGG_QTY = re.compile(
    r"^SUGGDBG-QTY func=(?P<func>\S+) blk=(?P<blk>-?\d+) qty=(?P<qty>-?\d+) "
    r"reg1=(?P<reg1>-?\d+) birth=(?P<birth>-?\d+) death=(?P<death>-?\d+) "
    r"refs=(?P<refs>-?\d+) size=(?P<size>-?\d+) mode=(?P<mode>-?\d+) "
    r"minclass=(?P<minclass>-?\d+) altclass=(?P<altclass>-?\d+) "
    r"calls=(?P<calls>-?\d+) chgsize=(?P<chgsize>-?\d+) "
    r"ncopysugg=(?P<ncopysugg>-?\d+) nsugg=(?P<nsugg>-?\d+) "
    r"copysugg=(?P<copysugg>[\d,]*) sugg=(?P<sugg>[\d,]*)")
SUGG_FFR = re.compile(
    r"^SUGGDBG-FFR qty=(?P<qty>-?\d+) class=(?P<class>-?\d+) "
    r"mode=(?P<mode>-?\d+) jts=(?P<jts>-?\d+) acc=(?P<acc>-?\d+) "
    r"born=(?P<born>-?\d+) dead=(?P<dead>-?\d+) "
    r"used=(?P<used>[\d,]*) first_used=(?P<first_used>[\d,]*)")


# TUs where the instrumented cc1 and the BUILD compiler (build/cc1, Makefile:12)
# emit different code. EMPTY since 2026-08-07: the divergence was removed, not
# waived. It had never been about the BB2 hooks — the two binaries were simply
# different builds, and only the build compiler carried the combine.c PLUS->IOR
# removal. The instrumented cc1 is now rebuilt from the same hooked sources PLUS
# tools/cc1-no-plus-to-ior.patch (tools/build_diagnostic_cc1.sh), and the two
# agree on all 32 TUs — so every dump describes what the project actually
# builds. Re-verify with `bash tools/build_diagnostic_cc1.sh` (no --install):
# it reports the divergent stems, and "none" is the contract.
# Keep the machinery below: if the binaries ever drift again, refill this dict
# rather than discovering it downstream. See docs/ORACLE-COMPILER.md.
UNFAITHFUL_STEMS = {}


def _regs(s):
    return [int(x) for x in s.split(",") if x]


def run(stem, suggest=False):
    WORK.mkdir(parents=True, exist_ok=True)
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{CPP} "{ROOT / "src" / (stem + ".c")}" > "{ifile}"',
                   shell=True, cwd=ROOT, check=True, stderr=subprocess.DEVNULL)
    env = dict(os.environ, BB2_QTY_DEBUG="1", BB2_ALLOC_DEBUG="1")
    if suggest:
        env["BB2_SUGG_DEBUG"] = "1"
    r = subprocess.run(
        f'"{CC1}" {CC1_FLAGS} -da "{ifile}" -o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
    return r.stderr


def segment_sugg(stderr):
    """SUGGDBG-QTY carries func= itself, so it needs no segmentation.  FFR lines
    do not, and are attributed to the block whose QTY table most recently
    preceded them -- which is exact, because block_alloc prints the whole table
    before making any find_free_reg call for that block."""
    out, cur = {}, None
    for line in stderr.splitlines():
        m = SUGG_QTY.match(line)
        if m:
            d = m.groupdict()
            cur = (d["func"], int(d["blk"]))
            row = {k: int(v) for k, v in d.items()
                   if k not in ("func", "copysugg", "sugg")}
            row["copysugg"] = _regs(d["copysugg"])
            row["sugg"] = _regs(d["sugg"])
            out.setdefault(d["func"], {}).setdefault(str(row["blk"]), {})[
                str(row["qty"])] = row
            continue
        f = SUGG_FFR.match(line)
        if f and cur:
            d = f.groupdict()
            row = {k: int(v) for k, v in d.items()
                   if k not in ("used", "first_used")}
            row["used"] = _regs(d["used"])
            row["first_used"] = _regs(d["first_used"])
            blk = out.setdefault(cur[0], {}).setdefault(str(cur[1]), {})
            blk.setdefault("_ffr", []).append(row)
    return out


def segment(stderr):
    """Group QTYDBG rows by the function named in the following ALLOCDBG line."""
    out, pending = {}, []
    for line in stderr.splitlines():
        m = QTY.match(line)
        if m:
            d = m.groupdict()
            pending.append({
                "pass": "sugg" if d["sugg"] else "main",
                "blk": int(d["blk"]), "ord": int(d["ord"]), "qty": int(d["qty"]),
                "first_reg": int(d["reg1"]), "birth": int(d["birth"]),
                "death": int(d["death"]), "refs": int(d["refs"]),
                "got": int(d["got"]),
            })
            continue
        f = FUNC.match(line)
        if f:
            out.setdefault(f.group("func"), []).extend(pending)
            pending = []
    if pending:
        out.setdefault("__trailing__", []).extend(pending)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("--func")
    ap.add_argument("--suggest", action="store_true",
                    help="also enable BB2_SUGG_DEBUG and write "
                         "<stem>.sugg.json (per-qty suggestion sets, qty_size, "
                         "class/mode, and find_free_reg's scanned hard-reg "
                         "sets). The .local.json output is unchanged either way.")
    a = ap.parse_args()
    if a.stem in UNFAITHFUL_STEMS:
        print("WARNING: the instrumented cc1 disagrees with the build compiler "
              "(build/cc1) on %s by %d instruction(s). This dump does NOT "
              "describe what the project builds — do not treat it as ground "
              "truth. See cc1_hooks.patch.md section 7."
              % (a.stem, UNFAITHFUL_STEMS[a.stem]), file=sys.stderr)
    stderr = run(a.stem, suggest=a.suggest)
    per_func = segment(stderr)
    path = WORK / f"{a.stem}.local.json"
    path.write_text(json.dumps(per_func, indent=1))
    print("local model written: %s  (%d functions, %d qty rows)"
          % (path, len(per_func), sum(len(v) for v in per_func.values())))
    if a.suggest:
        sg = segment_sugg(stderr)
        spath = WORK / f"{a.stem}.sugg.json"
        spath.write_text(json.dumps(sg, indent=1))
        nq = sum(len([k for k in b if k != "_ffr"])
                 for f in sg.values() for b in f.values())
        nsug = sum(1 for f in sg.values() for b in f.values()
                   for k, r in b.items()
                   if k != "_ffr" and (r["ncopysugg"] or r["nsugg"]))
        print("suggestion table written: %s  (%d functions, %d qtys, "
              "%d carrying a suggestion)" % (spath, len(sg), nq, nsug))
    if a.func:
        rows = per_func.get(a.func, [])
        print("\n%s: %d qty rows" % (a.func, len(rows)))
        for r in rows:
            print("  blk=%-3d %-4s ord=%-3d qty=%-3d reg1=%-4d "
                  "birth=%-4d death=%-4d refs=%-3d got=%d"
                  % (r["blk"], r["pass"], r["ord"], r["qty"], r["first_reg"],
                     r["birth"], r["death"], r["refs"], r["got"]))


main()
