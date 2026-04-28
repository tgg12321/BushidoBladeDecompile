#!/usr/bin/env python3
"""Run the attempt-first pipeline across the whole asm backlog.

For each candidate function (the inline `__asm__()` blocks in src/*.c plus
any remaining INCLUDE_ASM stubs), runs attempt_func.py and records the
result. Cheap verdicts (SKIPPED / HARD / NOT_FOUND) cost ~1s; expensive
ones (smart_match + permute_capped) hit the configured timeouts.

Output:
    tmp/batch_attempt.csv     -- per-function row: result, score, stage, elapsed
    tmp/batch_attempt.log     -- raw stdout per attempt (for debugging)

The --classify-only mode skips the heavy pipeline and just runs classify_func
on every candidate. Useful for an upper-bound census in <5 minutes.

Usage:
    python3 tools/batch_attempt.py --classify-only
    python3 tools/batch_attempt.py --time 60 --max-flat 20
    python3 tools/batch_attempt.py --limit 50          # only first 50
    python3 tools/batch_attempt.py --resume            # skip already-seen funcs
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
CSV_PATH = TMP / "batch_attempt.csv"
LOG_PATH = TMP / "batch_attempt.log"


def collect_candidates() -> list[tuple[str, str, str]]:
    """Return [(func, kind, src_relpath), ...] for every function that is
    currently asm-disguised: inline `__asm__()` block OR INCLUDE_ASM stub.
    Sorted: inline_asm first by src file then by asm/funcs/.s size, then
    INCLUDE_ASM."""
    candidates: list[tuple[str, str, str]] = []
    seen: set[str] = set()

    inline_re = re.compile(r"glabel\s+(\w+)")
    include_re = re.compile(r'INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*(\w+)\s*\)')

    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        for m in inline_re.finditer(text):
            name = m.group(1)
            if name in seen:
                continue
            seen.add(name)
            candidates.append((name, "inline_asm", str(src.relative_to(ROOT))))
        for m in include_re.finditer(text):
            name = m.group(1)
            if name in seen:
                continue
            seen.add(name)
            candidates.append((name, "include_asm", str(src.relative_to(ROOT))))

    # Sort by asm size (small first -- they're cheaper to attempt)
    def asm_size(name):
        p = ASM_FUNCS / f"{name}.s"
        if not p.exists():
            return 9999
        return sum(1 for _ in p.read_text(encoding="utf-8").splitlines())

    candidates.sort(key=lambda c: (c[2], asm_size(c[0])))
    return candidates


# ---- Result parsing ------------------------------------------------------

ATTEMPT_LINE_RE = re.compile(
    r"attempt:\s+(\S+)\s+result=(\w+)\s+score=(\S+)\s+stage=(\w+)\s+"
    r"reason=(\S+)\s+elapsed=(\d+)s(?:\s+attempts=(\d+))?"
)
CLASSIFY_LINE_RE = re.compile(r"^=== (\S+) -- (\S+) ===")


def parse_attempt(text: str, func: str) -> dict:
    for line in text.splitlines():
        m = ATTEMPT_LINE_RE.search(line)
        if m:
            return {
                "func": m.group(1),
                "result": m.group(2),
                "score": m.group(3),
                "stage": m.group(4),
                "reason": m.group(5),
                "elapsed": int(m.group(6)),
                "attempts": int(m.group(7)) if m.group(7) else 0,
            }
    return {"func": func, "result": "NO_OUTPUT", "score": "?", "stage": "?",
            "reason": "no_summary_line", "elapsed": 0, "attempts": 0}


def parse_classify(text: str, func: str) -> dict:
    rec_m = CLASSIFY_LINE_RE.search(text)
    if not rec_m:
        return {"func": func, "result": "NOT_FOUND", "recommendation": "?"}
    return {"func": func, "result": "CLASSIFY", "recommendation": rec_m.group(2)}


# ---- Per-function runners ------------------------------------------------

def run_classify(func: str, timeout: int = 30) -> dict:
    try:
        r = subprocess.run(
            [sys.executable, str(TOOLS / "classify_func.py"), func, "--json"],
            capture_output=True, text=True, cwd=str(ROOT), timeout=timeout,
        )
        if r.returncode != 0 and not r.stdout:
            return {"func": func, "result": "NOT_FOUND", "recommendation": "?"}
        try:
            d = json.loads(r.stdout)
            return {
                "func": func, "result": "CLASSIFY",
                "recommendation": d.get("recommendation", "?"),
                "size_insns": d.get("size", {}).get("insns", 0),
                "blocker_tags": ",".join(d.get("blocker_tags", []) or []),
                "src_kind": d.get("src", {}).get("kind", "?"),
            }
        except json.JSONDecodeError:
            return {"func": func, "result": "PARSE_ERROR", "recommendation": "?"}
    except subprocess.TimeoutExpired:
        return {"func": func, "result": "TIMEOUT", "recommendation": "?"}


def run_attempt(func: str, opts: dict, timeout: int) -> tuple[dict, str]:
    cmd = [sys.executable, str(TOOLS / "attempt_func.py"), func,
           "--time", str(opts["time"]),
           "--max-flat", str(opts["max_flat"]),
           "--threads", str(opts["threads"]),
           "--smart-time", str(opts["smart_time"])]
    try:
        r = subprocess.run(
            cmd, capture_output=True, text=True, cwd=str(ROOT), timeout=timeout,
        )
        out = r.stdout + r.stderr
        return parse_attempt(out, func), out
    except subprocess.TimeoutExpired:
        return ({"func": func, "result": "PROC_TIMEOUT", "score": "?",
                 "stage": "harness", "reason": "subprocess_timeout",
                 "elapsed": timeout, "attempts": 0}, "")


# ---- CSV utilities -------------------------------------------------------

CSV_FIELDS = [
    "func", "src", "kind", "result", "score", "stage", "reason",
    "elapsed", "attempts", "recommendation", "size_insns", "blocker_tags",
]


def load_existing(csv_path: Path) -> set[str]:
    seen = set()
    if not csv_path.exists():
        return seen
    with csv_path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            seen.add(row.get("func", ""))
    return seen


def append_row(csv_path: Path, row: dict, write_header: bool):
    mode = "w" if write_header else "a"
    with csv_path.open(mode, newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CSV_FIELDS)
        if write_header:
            w.writeheader()
        w.writerow({k: row.get(k, "") for k in CSV_FIELDS})


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--classify-only", action="store_true",
                    help="Run only classify_func -- fast census, no permuter.")
    ap.add_argument("--time", type=int, default=60,
                    help="attempt_func --time (default: 60)")
    ap.add_argument("--max-flat", type=int, default=20,
                    help="attempt_func --max-flat (default: 20)")
    ap.add_argument("--threads", type=int, default=2,
                    help="permuter parallelism per function (default: 2)")
    ap.add_argument("--smart-time", type=int, default=45,
                    help="smart_match cap per function (default: 45)")
    ap.add_argument("--limit", type=int, default=None,
                    help="Only attempt the first N candidates")
    ap.add_argument("--resume", action="store_true",
                    help="Skip funcs already in the CSV")
    ap.add_argument("--csv", default=str(CSV_PATH),
                    help=f"Output CSV path (default: {CSV_PATH.relative_to(ROOT)})")
    ap.add_argument("--no-log", action="store_true",
                    help="Don't write per-attempt stdout to a log file")
    ap.add_argument("--funcs-file", default=None,
                    help="Read candidate list from a file (one func per line). "
                    "When set, overrides the auto-discovery scan.")
    args = ap.parse_args()

    csv_path = Path(args.csv)
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    log_path = LOG_PATH

    if args.funcs_file:
        wanted = []
        with open(args.funcs_file, encoding="utf-8") as f:
            for line in f:
                name = line.strip()
                if not name or name.startswith("#"):
                    continue
                wanted.append(name)
        all_cands = collect_candidates()
        by_name = {c[0]: c for c in all_cands}
        candidates = []
        for name in wanted:
            if name in by_name:
                candidates.append(by_name[name])
            else:
                # Synthesize a minimal record so we still attempt
                candidates.append((name, "unknown", ""))
    else:
        candidates = collect_candidates()

    if args.limit:
        candidates = candidates[:args.limit]
    seen = load_existing(csv_path) if args.resume else set()
    write_header = not args.resume or not csv_path.exists()
    if write_header:
        csv_path.write_text("")  # truncate

    pending = [c for c in candidates if c[0] not in seen]
    print(f"# candidates: {len(candidates)} ({len(pending)} pending)",
          file=sys.stderr)

    if not args.no_log:
        log_f = log_path.open("a", encoding="utf-8")
    else:
        log_f = None

    # The per-function harness timeout is conservative: time + smart_time + 30s
    # buffer (setup, m2c, gen_regfix). Subprocess timeout above that aborts a
    # runaway attempt and continues with the next.
    proc_timeout = args.time + args.smart_time + 30

    started = time.monotonic()
    counts: dict[str, int] = {}
    for i, (func, kind, src) in enumerate(candidates, 1):
        if func in seen:
            continue

        if args.classify_only:
            row = run_classify(func)
        else:
            row, raw = run_attempt(func, vars(args), proc_timeout)
            row["recommendation"] = ""  # filled by attempt's classify gate
            if log_f:
                log_f.write(f"\n=== {func} ({kind}, {src}) ===\n{raw}\n")
                log_f.flush()

        row["src"] = src
        row["kind"] = kind
        # Update counts dict
        bucket = row.get("result") or row.get("recommendation") or "?"
        counts[bucket] = counts.get(bucket, 0) + 1

        append_row(csv_path, row, write_header)
        write_header = False

        elapsed = int(time.monotonic() - started)
        # One-line progress (stderr so it doesn't pollute the CSV)
        line = (f"[{i}/{len(candidates)}] {func:30s} "
                f"{row.get('result',''):14s} "
                f"score={row.get('score','-'):>6s} "
                f"rec={row.get('recommendation',''):20s} "
                f"({elapsed}s elapsed)")
        print(line, file=sys.stderr, flush=True)

    if log_f:
        log_f.close()

    elapsed = int(time.monotonic() - started)
    print(f"\n# Done in {elapsed}s. Results -> {csv_path.relative_to(ROOT)}",
          file=sys.stderr)
    print("# Tally:", file=sys.stderr)
    for k, v in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"#   {k:24s} {v:>5d}", file=sys.stderr)


if __name__ == "__main__":
    sys.exit(main())
