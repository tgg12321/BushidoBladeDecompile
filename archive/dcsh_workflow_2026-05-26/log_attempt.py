#!/usr/bin/env python3
"""Append an attempt entry to a function's .bb2_attempts/<func>.jsonl log.

Usage:
    log_attempt.py <func> <category> <variant> <duration_min> "<outcome>"

The escape-valve gate (tools/check_attempts.py) requires the log to
show ≥4 distinct categories, ≥6 attempts, ≥30 cumulative minutes
before allowing commits of INLINE_MOVE_ALIASING / asmfix bridge /
inline_asm_canonical patterns.

Category must be one of (see check_attempts.CATEGORIES for descriptions):
    C_structure, C_types, C_hints, permuter_seed, permuter_long,
    m2c_restart, manual_steer, sibling_match

OVERRIDE is a special category used only to bypass the gate when the
user explicitly approves an escape valve after observing exhausted
effort. Format:
    log_attempt.py <func> OVERRIDE override "<reason>"
"""
from __future__ import annotations

import argparse
import datetime
import json
import sys
from pathlib import Path

# Reuse the canonical category list from the validator.
sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_attempts import ATTEMPTS_DIR, CATEGORIES, attempts_path  # noqa: E402


def next_id(existing: list[dict]) -> str:
    n = 1
    used = {a.get("id", "") for a in existing}
    while f"a{n}" in used:
        n += 1
    return f"a{n}"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("category", help=f"one of: {', '.join(sorted(CATEGORIES))}, or OVERRIDE")
    ap.add_argument("variant", help="short label, e.g. 'drop_cached'")
    ap.add_argument("duration_minutes", type=int, help="wall-clock effort estimate")
    ap.add_argument("outcome", help="≥20 chars; reference a concrete observation")
    args = ap.parse_args()

    if args.category != "OVERRIDE" and args.category not in CATEGORIES:
        print(f"ERROR: unknown category '{args.category}'", file=sys.stderr)
        print(f"  valid: {sorted(CATEGORIES)} or OVERRIDE", file=sys.stderr)
        return 2

    if args.category != "OVERRIDE" and len(args.outcome) < 20:
        print(f"ERROR: outcome too short ({len(args.outcome)} chars; need ≥20).", file=sys.stderr)
        print( "  Reference a concrete observation: diff count, score, "
               "specific GCC behavior, error text. Vague outcomes get caught "
               "by the LLM auditor.", file=sys.stderr)
        return 2

    ATTEMPTS_DIR.mkdir(parents=True, exist_ok=True)
    p = attempts_path(args.func)

    # Load existing to compute the next id.
    existing: list[dict] = []
    if p.exists():
        for line in p.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            try:
                existing.append(json.loads(line))
            except json.JSONDecodeError:
                pass

    entry = {
        "id": next_id(existing),
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
        "category": args.category,
        "variant": args.variant,
        "duration_minutes": args.duration_minutes,
        "outcome": args.outcome,
    }
    if args.category == "OVERRIDE":
        entry["reason"] = args.outcome  # alias for clarity

    with p.open("a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")

    print(f"logged: {entry['id']} {args.category}/{args.variant} ({args.duration_minutes} min)")
    print(f"  file: {p.relative_to(ATTEMPTS_DIR.parent)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
