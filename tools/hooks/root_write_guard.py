#!/usr/bin/env python3
"""PreToolUse hook on Write|Edit|MultiEdit. Blocks writes that would
create or modify suspicious-pattern files at the repo root.

Convention: one-off tools, audits, scratch outputs MUST write to
tmp/ (gitignored). Long-lived logs (if genuinely needed) go to
logs/ (gitignored, create on demand). Don't crowd the root.

Patterns blocked at root (when path has no directory part):
  *.log, *.csv (except allowlist), gccdump.*, *.bak, *.orig,
  *.dump, *.stackdump, _tmp_*, triage_*

For new path-creation cases (Write to a brand-new root file), the
hook also blocks if the filename matches a suspicious pattern even
if it's not yet in the allowlist.

Exit codes:
  0 — allow (path not at root, or matches allowed pattern)
  2 — block (suspicious root write; reason on stderr)
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

# Pull the allowlist from the cleanliness checker so they stay in sync
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
try:
    from check_root_cleanliness import ALLOWED_FILES, ALLOWED_STATE_RE
except ImportError:
    ALLOWED_FILES = set()
    ALLOWED_STATE_RE = re.compile(r"^\.bb2_.*$|^bb2\.d$|^Makefile\.bak$")

SUSPICIOUS_RE = re.compile(
    r"^("
    r"gccdump\.|"
    r".*\.log$|"
    r".*\.csv$|"
    r".*_results\..*|"
    r".*_run\..*|"
    r"triage_.*|"
    r"_tmp_.*|"
    r".*\.bak$|"
    r".*\.orig$|"
    r".*\.dump$|"
    r".*\.stackdump$|"
    r"overnight_.*|"
    r"rescore_.*|"
    r"smart_match_.*"
    r")$"
)


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    tool_input = payload.get("tool_input", {})
    file_path = tool_input.get("file_path", "")
    if not file_path:
        return 0

    p = Path(file_path)
    # Only care about writes to the repo root (no subdirectory part)
    # — i.e., the parent dir IS the repo root
    repo_root_names = {"Bushido Blade 2 Decompile"}  # the project dir name
    if p.parent.name not in repo_root_names:
        return 0

    name = p.name

    # Allowlisted file → permit
    if name in ALLOWED_FILES or ALLOWED_STATE_RE.match(name):
        return 0

    # Suspicious pattern → block
    if SUSPICIOUS_RE.match(name):
        print(f"BLOCKED by root_write_guard.py: writing {name!r} at repo root.", file=sys.stderr)
        print("", file=sys.stderr)
        print("Convention: one-off tool outputs, audit results, scratch files MUST go to:", file=sys.stderr)
        print("  - tmp/                — for one-off / per-session / regenerable outputs (gitignored)", file=sys.stderr)
        print("  - logs/               — for long-lived logs (create on demand, gitignored)", file=sys.stderr)
        print("  - docs/               — for permanent documentation", file=sys.stderr)
        print("  - tools/              — for reusable tools", file=sys.stderr)
        print("", file=sys.stderr)
        print(f"  Replace `{name}` with `tmp/{name}` (or similar) in the file_path.", file=sys.stderr)
        print("", file=sys.stderr)
        print("If this file genuinely belongs at the repo root, add it to the allowlist:", file=sys.stderr)
        print("  tools/check_root_cleanliness.py  (ALLOWED_* sets)", file=sys.stderr)
        return 2

    # New file at root that's NOT on allowlist AND NOT suspicious → warn
    # (Not block — there could be a legit reason; just flag.)
    if not p.exists():
        print(f"WARN from root_write_guard.py: creating new file {name!r} at repo root.", file=sys.stderr)
        print("  If this is a one-off / scratch / log: write to tmp/ instead.", file=sys.stderr)
        print("  If this is permanent: add to tools/check_root_cleanliness.py allowlist.", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
