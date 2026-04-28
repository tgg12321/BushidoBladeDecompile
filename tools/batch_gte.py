#!/usr/bin/env python3
"""Parallel batch run for the 78 GTE-using inline-asm functions.

For each GTE function:
  1. Run `gte_migrate` to produce a candidate base.c that uses gte.h macros.
  2. Stage permuter/<func>/ via extract_inline_asm.py setup (target.s, etc).
  3. Try smart_match (cap default 30s) -- direct strategy may already match
     when the function is pure GTE atomic ops with trivial wrapping.
  4. Try permute_capped (short cap) for additional auto-match cycles.
  5. Record the verdict in tmp/batch_gte.csv.

Why bother running permuter at all? Most GTE functions need manual register-
transfer wiring, but a small minority -- functions whose only "GTE-ness" is
an atomic op like rtps/nclip/avsz3 with C around it -- can match outright.
The cost is 30s per function = ~40 min total. Worth a free fishing trip.

Usage:
    python3 tools/batch_gte.py                  # full run
    python3 tools/batch_gte.py --limit 10       # first 10 only
    python3 tools/batch_gte.py --scaffold-only  # just write base.c, no compile
"""
from __future__ import annotations

import argparse
import csv
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
SRC_DIR = ROOT / "src"
ASM_FUNCS = ROOT / "asm" / "funcs"
TMP = ROOT / "tmp"
TMP.mkdir(exist_ok=True)

CSV_PATH = TMP / "batch_gte.csv"
LOG_PATH = TMP / "batch_gte.log"

GTE_OP_RE = re.compile(
    r"\b(cop2|mtc2|mfc2|lwc2|swc2|ctc2|cfc2|nclip|rtps|rtpt|"
    r"avsz3|avsz4|gpf|gpl|mvmva|ncs|nct|ncds|ncdt|nccs|ncct|"
    r"cdp|cc|sqr|dpcs|dpct|intpl|dpcl)\b"
)

CSV_FIELDS = [
    "func", "src", "size_insns", "atomic_ops", "register_transfers",
    "scaffold_handled", "scaffold_leftover",
    "result", "score", "stage", "elapsed_s",
]


def collect_gte_functions() -> list[tuple[str, str]]:
    """Find inline-asm functions whose body uses GTE ops. Returns
    [(func, src_relpath), ...] sorted by asm size (small first)."""
    out = []
    seen: set[str] = set()
    inline_re = re.compile(r"glabel\s+(\w+)")

    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        # Walk each `__asm__()` block
        for blk in re.finditer(r"__asm__\s*\(([^;]*?)\)\s*;", text, re.DOTALL):
            body = blk.group(1)
            m = inline_re.search(body)
            if not m:
                continue
            func = m.group(1)
            if func in seen:
                continue
            if not GTE_OP_RE.search(body):
                continue
            seen.add(func)
            out.append((func, str(src.relative_to(ROOT))))

    def asm_size(name):
        p = ASM_FUNCS / f"{name}.s"
        if not p.exists():
            return 9999
        return sum(1 for _ in p.read_text(encoding="utf-8").splitlines())

    out.sort(key=lambda c: asm_size(c[0]))
    return out


def run(cmd: list[str], timeout: int = 60) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


def gte_migrate(func: str) -> tuple[bool, dict, str]:
    """Run gte_migrate.py with --setup. Returns (ok, stats, raw_log)."""
    try:
        r = run([sys.executable, str(TOOLS / "gte_migrate.py"), func, "--setup"],
                timeout=90)
    except subprocess.TimeoutExpired:
        return False, {"handled": 0, "leftover": 0}, "timeout"
    log = r.stderr + "\n" + r.stdout
    m = re.search(r"handled=(\d+).*?leftover=(\d+)", log)
    stats = {
        "handled": int(m.group(1)) if m else 0,
        "leftover": int(m.group(2)) if m else 0,
    }
    base = ROOT / "permuter" / func / "base.c"
    return base.exists() and base.stat().st_size > 0, stats, log


def smart_match(func: str, timeout: int) -> tuple[str | None, str]:
    """Run smart_match.py. Returns (matched_strategy_or_None, log)."""
    try:
        r = run([sys.executable, str(TOOLS / "smart_match.py"),
                 "--func", func], timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, "smart_match: timeout"
    log = r.stdout + "\n" + r.stderr
    m = re.search(r"MATCH via (\w+)!", log)
    if m:
        return m.group(1), log
    return None, log


def permute(func: str, max_time: int, max_flat: int, threads: int) -> dict:
    try:
        r = run([sys.executable, str(TOOLS / "permute_capped.py"), func,
                 "--max-time", str(max_time),
                 "--max-flat-seconds", str(max_flat),
                 "-j", str(threads)],
                timeout=max_time + 30)
    except subprocess.TimeoutExpired:
        return {"matched": False, "best": None, "reason": "subprocess_timeout"}
    out = r.stdout + r.stderr
    m = re.search(r"matched=(\w+)\s+best=(\S+).*?reason=(\w+)", out)
    if not m:
        return {"matched": False, "best": None, "reason": "parse_failed"}
    return {
        "matched": m.group(1) == "Y",
        "best": int(m.group(2)) if m.group(2).isdigit() else None,
        "reason": m.group(3),
    }


def count_atomic_and_xfer(func: str) -> tuple[int, int]:
    """Quick fingerprint counts for the CSV."""
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return 0, 0
    text = p.read_text(encoding="utf-8")
    ATOMIC = {"rtps", "rtpt", "nclip", "avsz3", "avsz4", "dpcs", "dpct",
              "intpl", "sqr", "ncs", "nct", "ncds", "ncdt", "nccs", "ncct",
              "cdp", "cc", "dpcl"}
    XFER = {"mtc2", "mfc2", "lwc2", "swc2", "ctc2", "cfc2"}
    a = x = 0
    for line in text.splitlines():
        m = re.search(r"\*/\s+(\S+)", line)
        if not m:
            continue
        op = m.group(1).lower()
        if op in ATOMIC:
            a += 1
        elif op in XFER:
            x += 1
    return a, x


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--scaffold-only", action="store_true",
                    help="Just generate base.c via gte_migrate; skip compile/permute")
    ap.add_argument("--smart-time", type=int, default=30,
                    help="smart_match cap (default: 30)")
    ap.add_argument("--time", type=int, default=20,
                    help="permute_capped --max-time (default: 20)")
    ap.add_argument("--max-flat", type=int, default=10,
                    help="permute_capped --max-flat-seconds (default: 10)")
    ap.add_argument("--threads", type=int, default=2,
                    help="permuter parallelism (default: 2)")
    ap.add_argument("--limit", type=int, default=None,
                    help="Only process the first N candidates")
    ap.add_argument("--csv", default=str(CSV_PATH))
    args = ap.parse_args()

    candidates = collect_gte_functions()
    if args.limit:
        candidates = candidates[:args.limit]
    print(f"# {len(candidates)} GTE candidates", file=sys.stderr)

    csv_path = Path(args.csv)
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    csv_path.write_text("")  # truncate
    log_f = LOG_PATH.open("w", encoding="utf-8")

    write_header = True
    started = time.monotonic()
    counts: dict[str, int] = {}

    for i, (func, src) in enumerate(candidates, 1):
        atomic, xfer = count_atomic_and_xfer(func)
        size = sum(1 for _ in (ASM_FUNCS / f"{func}.s").read_text(
            encoding="utf-8").splitlines())

        ok, stats, mig_log = gte_migrate(func)
        log_f.write(f"\n=== {func} (gte_migrate) ===\n{mig_log}\n")
        log_f.flush()

        row = {
            "func": func, "src": src, "size_insns": size,
            "atomic_ops": atomic, "register_transfers": xfer,
            "scaffold_handled": stats["handled"],
            "scaffold_leftover": stats["leftover"],
            "result": "SCAFFOLD_ONLY" if args.scaffold_only else "?",
            "score": "", "stage": "scaffold", "elapsed_s": 0,
        }

        if not ok:
            row["result"] = "MIGRATE_FAILED"
        elif not args.scaffold_only:
            t0 = time.monotonic()
            strat, sm_log = smart_match(func, args.smart_time)
            log_f.write(f"\n=== {func} (smart_match) ===\n{sm_log}\n")
            log_f.flush()
            if strat:
                row["result"] = "MATCHED"
                row["score"] = "0"
                row["stage"] = f"smart:{strat}"
            else:
                # Try permuter
                pres = permute(func, args.time, args.max_flat, args.threads)
                row["score"] = pres["best"] if pres["best"] is not None else "?"
                if pres["matched"]:
                    row["result"] = "MATCHED"
                    row["stage"] = "permute"
                elif pres["best"] is not None and pres["best"] <= 200:
                    row["result"] = "NEAR_MISS"
                    row["stage"] = "permute"
                elif pres["best"] is not None:
                    row["result"] = "HARD"
                    row["stage"] = "permute"
                else:
                    row["result"] = "ERROR"
                    row["stage"] = "permute"
            row["elapsed_s"] = int(time.monotonic() - t0)

        # Write row
        with csv_path.open("a", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=CSV_FIELDS)
            if write_header:
                w.writeheader()
                write_header = False
            w.writerow({k: row.get(k, "") for k in CSV_FIELDS})

        counts[row["result"]] = counts.get(row["result"], 0) + 1
        elapsed = int(time.monotonic() - started)
        print(f"[{i}/{len(candidates)}] {func:30s} "
              f"atomic={atomic} xfer={xfer} size={size:3d} "
              f"-> {row['result']:14s} score={row.get('score','-')} "
              f"({elapsed}s)", file=sys.stderr, flush=True)

    log_f.close()
    elapsed = int(time.monotonic() - started)
    print(f"\n# Done in {elapsed}s. Results -> {csv_path.relative_to(ROOT)}",
          file=sys.stderr)
    for k, v in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"#   {k:20s} {v:>4d}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
