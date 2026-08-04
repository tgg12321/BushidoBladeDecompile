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


def run(stem):
    WORK.mkdir(parents=True, exist_ok=True)
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{CPP} "{ROOT / "src" / (stem + ".c")}" > "{ifile}"',
                   shell=True, cwd=ROOT, check=True, stderr=subprocess.DEVNULL)
    env = dict(os.environ, BB2_QTY_DEBUG="1", BB2_ALLOC_DEBUG="1")
    r = subprocess.run(
        f'"{CC1}" {CC1_FLAGS} -da "{ifile}" -o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
    return r.stderr


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
    a = ap.parse_args()
    per_func = segment(run(a.stem))
    path = WORK / f"{a.stem}.local.json"
    path.write_text(json.dumps(per_func, indent=1))
    print("local model written: %s  (%d functions, %d qty rows)"
          % (path, len(per_func), sum(len(v) for v in per_func.values())))
    if a.func:
        rows = per_func.get(a.func, [])
        print("\n%s: %d qty rows" % (a.func, len(rows)))
        for r in rows:
            print("  blk=%-3d %-4s ord=%-3d qty=%-3d reg1=%-4d "
                  "birth=%-4d death=%-4d refs=%-3d got=%d"
                  % (r["blk"], r["pass"], r["ord"], r["qty"], r["first_reg"],
                     r["birth"], r["death"], r["refs"], r["got"]))


main()
