#!/usr/bin/env python3
"""Validate a function's decomp attempt log against the escape-valve gate.

`tools/hooks/escape_valve_guard.sh` invokes this at commit time when it
detects a commit adding an escape-valve pattern (INLINE_MOVE_ALIASING,
asmfix bridge, inline_asm_canonical entry). The gate REJECTS the commit
unless the function's `.bb2_attempts/<func>.jsonl` log shows the agent
has actually exhausted enough pure-C territory to justify the deviation.

Why this exists: the documented escape valves accept "≥2 attempts" as
sufficient. That bar is trivially met — an agent who tries 3 variations
of "different declaration order" and one short permuter run claims
"exhausted." This module forces the variation to span DISTINCT technique
categories, not just N rotations of the same idea.

Log format: `.bb2_attempts/<func>.jsonl` — one JSON object per line.
Required fields per attempt:
    id              str  — unique within file, e.g. "a1"
    timestamp       str  — ISO 8601
    category        str  — must be in CATEGORIES (see below)
    variant         str  — short label
    duration_minutes int — wall-clock effort, agent-honest estimate
    outcome         str  — what happened (≥20 chars, must reference a
                            concrete observation: diff count, score,
                            error message, etc.)

Override entry: a special entry with category="OVERRIDE" and a reason
counts as satisfying ALL gate criteria. Used when the user explicitly
approves an escape valve.

Exit codes:
    0  — gate passes, escape-valve commit is allowed
    1  — gate fails, escape-valve commit should be blocked
    2  — usage error / malformed log
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ATTEMPTS_DIR = ROOT / ".bb2_attempts"

# Distinct technique categories. The escape-valve gate requires attempts
# spanning ≥MIN_CATEGORIES of these. Adding a new category is fine;
# REMOVING one would lower the bar for existing functions, so don't.
CATEGORIES = {
    "C_structure":     "body structural change: decl reorder, intermediate vars, loop reshape, expression rearrangement",
    "C_types":         "signature type changes, pointer-vs-int, cast-style changes",
    "C_hints":         "volatile annotations, register-asm pin exploration (note: pins are a hint, not the escape valve itself)",
    "permuter_seed":   "permuter run with specific seed (count separate entries per distinct seed value)",
    "permuter_long":   "permuter with --max-time >=600s and --max-flat >=120s",
    "m2c_restart":     "fresh-start from m2c's auto-decompile of target.s",
    "manual_steer":    "single-instruction inline asm for codegen control, NOT the full INLINE_MOVE_ALIASING template",
    "sibling_match":   "compare to / borrow from an already-matched sibling function with similar shape",
}

# Gate thresholds. Tunable. Current bar:
#   - ≥4 distinct categories (catches "3 variations of decl reorder")
#   - ≥6 total attempts (catches "I tried 4 things")
#   - ≥30 cumulative minutes (catches "I spent 10 min on this")
MIN_CATEGORIES = 4
MIN_ATTEMPTS = 6
MIN_MINUTES = 30


def attempts_path(func: str) -> Path:
    return ATTEMPTS_DIR / f"{func}.jsonl"


def load_attempts(func: str) -> tuple[list[dict], list[str]]:
    """Returns (attempts, errors). Errors are non-fatal warnings."""
    p = attempts_path(func)
    if not p.exists():
        return [], []
    attempts: list[dict] = []
    errors: list[str] = []
    for lineno, raw in enumerate(p.read_text(encoding="utf-8").splitlines(), 1):
        raw = raw.strip()
        if not raw or raw.startswith("#"):
            continue
        try:
            ev = json.loads(raw)
        except json.JSONDecodeError as e:
            errors.append(f"{p}:{lineno}: invalid JSON: {e}")
            continue
        if not isinstance(ev, dict):
            errors.append(f"{p}:{lineno}: not a JSON object")
            continue
        attempts.append(ev)
    return attempts, errors


def evaluate(func: str) -> dict:
    """Compute the gate verdict and metrics. No I/O side effects."""
    attempts, errors = load_attempts(func)
    total_attempts = len(attempts)
    categories_seen: set[str] = set()
    unknown_categories: set[str] = set()
    total_minutes = 0
    has_override = False
    override_reason = ""
    for a in attempts:
        cat = a.get("category", "")
        if cat == "OVERRIDE":
            has_override = True
            override_reason = a.get("reason", a.get("outcome", "(no reason given)"))
            continue
        if cat not in CATEGORIES:
            unknown_categories.add(cat)
            continue
        categories_seen.add(cat)
        dur = a.get("duration_minutes", 0)
        if isinstance(dur, (int, float)):
            total_minutes += int(dur)
    missing_categories = sorted(set(CATEGORIES) - categories_seen)
    verdict = {
        "function": func,
        "log_exists": attempts_path(func).exists(),
        "total_attempts": total_attempts,
        "categories_seen": sorted(categories_seen),
        "categories_count": len(categories_seen),
        "missing_categories": missing_categories,
        "unknown_categories": sorted(unknown_categories),
        "total_minutes": total_minutes,
        "has_override": has_override,
        "override_reason": override_reason,
        "errors": errors,
        "thresholds": {
            "min_categories": MIN_CATEGORIES,
            "min_attempts": MIN_ATTEMPTS,
            "min_minutes": MIN_MINUTES,
        },
    }
    if has_override:
        verdict["pass"] = True
        verdict["pass_reason"] = "OVERRIDE entry present"
        return verdict
    fails = []
    if len(categories_seen) < MIN_CATEGORIES:
        fails.append(f"only {len(categories_seen)} distinct categories (need {MIN_CATEGORIES})")
    if total_attempts < MIN_ATTEMPTS:
        fails.append(f"only {total_attempts} attempts (need {MIN_ATTEMPTS})")
    if total_minutes < MIN_MINUTES:
        fails.append(f"only {total_minutes} cumulative minutes (need {MIN_MINUTES})")
    verdict["pass"] = not fails
    verdict["pass_reason"] = "ok" if not fails else "; ".join(fails)
    return verdict


def render_human(verdict: dict, *, color: bool = False) -> str:
    """Multi-line human-readable summary."""
    GREEN = "\033[32m" if color else ""
    RED = "\033[31m" if color else ""
    YELLOW = "\033[33m" if color else ""
    RESET = "\033[0m" if color else ""
    lines = []
    lines.append(f"=== attempts log: {verdict['function']} ===")
    if not verdict["log_exists"]:
        lines.append(f"  {RED}LOG MISSING{RESET} (no .bb2_attempts/{verdict['function']}.jsonl)")
    lines.append(f"  attempts:   {verdict['total_attempts']}")
    lines.append(f"  categories: {verdict['categories_count']} distinct  {verdict['categories_seen']}")
    lines.append(f"  minutes:    {verdict['total_minutes']}")
    if verdict["has_override"]:
        lines.append(f"  OVERRIDE:   {verdict['override_reason']}")
    if verdict["unknown_categories"]:
        lines.append(f"  {YELLOW}unknown categories used:{RESET} {verdict['unknown_categories']}")
        lines.append(f"    (typo? known categories: {sorted(CATEGORIES)})")
    if verdict["errors"]:
        lines.append(f"  {RED}log errors:{RESET}")
        for e in verdict["errors"]:
            lines.append(f"    {e}")
    t = verdict["thresholds"]
    lines.append("")
    lines.append(f"--- escape-valve gate (need ≥{t['min_categories']} categories, "
                 f"≥{t['min_attempts']} attempts, ≥{t['min_minutes']} min) ---")
    if verdict["pass"]:
        lines.append(f"  {GREEN}PASS{RESET}: {verdict['pass_reason']}")
    else:
        lines.append(f"  {RED}FAIL{RESET}: {verdict['pass_reason']}")
        if verdict["missing_categories"]:
            lines.append(f"  Try ≥1 attempt in one of these to qualify:")
            for cat in verdict["missing_categories"]:
                lines.append(f"    - {cat}: {CATEGORIES[cat]}")
        lines.append("")
        lines.append(f"  Log more attempts: bash tools/dc.sh log-attempt {verdict['function']} \\")
        lines.append("    <category> <variant_name> <duration_min> '<outcome description>'")
        lines.append("")
        lines.append("  User override (rare): add an OVERRIDE entry manually to")
        lines.append(f"    .bb2_attempts/{verdict['function']}.jsonl  — see tools/check_attempts.py docstring.")
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func", help="function name (e.g., func_800483DC)")
    ap.add_argument("--json", action="store_true",
                    help="emit JSON verdict to stdout instead of human-readable")
    ap.add_argument("--quiet", action="store_true",
                    help="emit nothing on PASS (for use in scripts)")
    args = ap.parse_args()

    if not args.func or args.func.startswith("-"):
        print("ERROR: function name required", file=sys.stderr)
        return 2

    verdict = evaluate(args.func)
    if args.json:
        print(json.dumps(verdict, indent=2))
    elif args.quiet and verdict["pass"]:
        pass
    else:
        # Use color if stderr is a tty (we print to stderr on FAIL so it
        # surfaces in hook block messages).
        stream = sys.stdout if verdict["pass"] else sys.stderr
        print(render_human(verdict, color=stream.isatty()), file=stream)
    return 0 if verdict["pass"] else 1


if __name__ == "__main__":
    sys.exit(main())
