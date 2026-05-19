#!/usr/bin/env python3
"""bb2_dead_rule_audit.py -- find regfix rules that the build doesn't need.

Some regfix rules accumulated over the project's history are no longer
load-bearing -- the underlying C source matches without them. This tool
finds those by:

  1. Reading regfix.txt
  2. For each non-comment rule line:
     a. Comment it out
     b. Run a full make
     c. If SHA1 still matches, mark the rule as DEAD
     d. Restore the rule
  3. Report all DEAD rules (and per-function dead-rule groups)

The tool is RESUMEABLE via a journal file (`tmp/dead_rule_audit_journal.json`)
so an interrupted run picks up where it left off rather than redoing the
full sweep. Each full-make takes 30-60 seconds; a full sweep of ~5000 rules
would take ~30 hours. Run with `--limit N` to bound the work, or
`--func <name>` to audit only rules for one function.

Usage:
  python3 tools/bb2_dead_rule_audit.py --limit 20         # 20 rules then stop
  python3 tools/bb2_dead_rule_audit.py --func cdrom_FramesToBcd
  python3 tools/bb2_dead_rule_audit.py --resume           # continue prior run
  python3 tools/bb2_dead_rule_audit.py --report           # show findings so far

Output: tmp/dead_rule_audit.json (results) + tmp/dead_rule_audit_journal.json (resume state).
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import time
from pathlib import Path


REGFIX_PATH = Path("regfix.txt")
JOURNAL_PATH = Path("tmp/dead_rule_audit_journal.json")
RESULTS_PATH = Path("tmp/dead_rule_audit.json")


def read_lf(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_lf(path: Path, content: str) -> None:
    if "\r\n" in content:
        raise SystemExit("REFUSING to write CRLF -- LF only for build files")
    path.write_bytes(content.encode("utf-8"))


def _is_wsl() -> bool:
    try:
        with open("/proc/version") as f:
            return "microsoft" in f.read().lower() or "wsl" in f.read().lower()
    except Exception:
        return False


def make_and_check() -> bool:
    """Run full make. Return True iff SHA1 matches."""
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'make 2>&1 | tail -5'
    )
    args = ["bash", "-c", cmd] if _is_wsl() else ["wsl", "bash", "-c", cmd]
    r = subprocess.run(args, capture_output=True, text=True, timeout=300)
    out = (r.stdout or "") + (r.stderr or "")
    return "bb2 matches!" in out


def load_journal() -> dict:
    if JOURNAL_PATH.exists():
        try:
            return json.loads(JOURNAL_PATH.read_text(encoding="utf-8"))
        except Exception:
            return {}
    return {}


def save_journal(j: dict) -> None:
    JOURNAL_PATH.parent.mkdir(parents=True, exist_ok=True)
    JOURNAL_PATH.write_text(json.dumps(j, indent=2), encoding="utf-8")


def parse_regfix_rules(filter_func: str | None = None) -> list[tuple[int, str, str]]:
    """Return [(line_idx, func_name, original_line)] for each non-comment rule."""
    text = read_lf(REGFIX_PATH)
    rules = []
    for i, line in enumerate(text.split("\n")):
        stripped = line.lstrip()
        if stripped.startswith("#") or not stripped:
            continue
        m = re.match(r"^([a-zA-Z_]\w+):\s*", stripped)
        if not m:
            continue
        func = m.group(1)
        if filter_func and func != filter_func:
            continue
        rules.append((i, func, line))
    return rules


def comment_line(line_idx: int) -> str:
    """Comment out line at index. Returns original text for restoration."""
    text = read_lf(REGFIX_PATH)
    lines = text.split("\n")
    if line_idx >= len(lines):
        raise IndexError(f"line {line_idx} out of range")
    original = lines[line_idx]
    if original.lstrip().startswith("#"):
        return original  # already commented
    lines[line_idx] = f"# DEAD-AUDIT-TEMP: {original}"
    write_lf(REGFIX_PATH, "\n".join(lines))
    return original


def restore_line(line_idx: int, original: str) -> None:
    text = read_lf(REGFIX_PATH)
    lines = text.split("\n")
    lines[line_idx] = original
    write_lf(REGFIX_PATH, "\n".join(lines))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--limit", type=int, default=0,
                   help="Stop after auditing N rules (0 = no limit)")
    p.add_argument("--func", default=None,
                   help="Only audit rules for this function")
    p.add_argument("--resume", action="store_true",
                   help="Resume from journal (skip already-audited rules)")
    p.add_argument("--report", action="store_true",
                   help="Just print existing results, don't audit")
    p.add_argument("--quiet", action="store_true")
    args = p.parse_args()

    if args.report:
        if not RESULTS_PATH.exists():
            print("(no results yet)")
            return
        results = json.loads(RESULTS_PATH.read_text(encoding="utf-8"))
        dead = [r for r in results if r["status"] == "dead"]
        live = [r for r in results if r["status"] == "live"]
        print(f"Audited: {len(results)}")
        print(f"  DEAD (build matches without rule): {len(dead)}")
        print(f"  LIVE (rule load-bearing):          {len(live)}")
        if dead:
            print("\nDEAD rules (safe to retire):")
            for r in dead:
                print(f"  L{r['line']:5d}  {r['rule']}")
        return

    # Sanity-check baseline before starting
    print("[audit] sanity: running baseline make to verify SHA1 matches at HEAD...")
    if not make_and_check():
        sys.exit("[audit] BASELINE FAIL: build doesn't match at HEAD. "
                 "Run the audit on a clean tree only.")
    print("[audit] baseline OK")

    journal = load_journal() if args.resume else {}
    rules = parse_regfix_rules(args.func)
    print(f"[audit] {len(rules)} rules in scope")

    results = []
    if RESULTS_PATH.exists():
        try:
            results = json.loads(RESULTS_PATH.read_text(encoding="utf-8"))
        except Exception:
            results = []
    audited_lines = {r["line"] for r in results}

    audited_count = 0
    for line_idx, func, original_line in rules:
        if args.resume and line_idx in audited_lines:
            continue
        if args.limit and audited_count >= args.limit:
            print(f"[audit] limit {args.limit} reached")
            break

        print(f"\n[audit] L{line_idx:5d} {func}:  {original_line.strip()[:80]}")
        original = comment_line(line_idx)
        try:
            matches = make_and_check()
        except Exception as e:
            print(f"[audit] ERROR during make: {e}")
            matches = False
        restore_line(line_idx, original)

        status = "dead" if matches else "live"
        print(f"[audit]   -> {status}")
        results.append({
            "line": line_idx,
            "func": func,
            "rule": original_line.strip(),
            "status": status,
            "ts": time.strftime("%Y-%m-%dT%H:%M:%S"),
        })
        # Persist after each rule (resumeable)
        RESULTS_PATH.parent.mkdir(parents=True, exist_ok=True)
        RESULTS_PATH.write_text(json.dumps(results, indent=2), encoding="utf-8")
        audited_count += 1

    print(f"\n[audit] done. {audited_count} rules audited this run, "
          f"{len(results)} total. See {RESULTS_PATH}")
    dead = [r for r in results if r["status"] == "dead"]
    if dead:
        print(f"[audit] DEAD rules found: {len(dead)}")
        for r in dead[:10]:
            print(f"  L{r['line']:5d}  {r['rule']}")
        if len(dead) > 10:
            print(f"  ... ({len(dead) - 10} more) -- see {RESULTS_PATH}")


if __name__ == "__main__":
    main()
