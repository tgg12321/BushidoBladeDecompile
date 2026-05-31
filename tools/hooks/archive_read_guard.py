#!/usr/bin/env python3
"""PreToolUse hook on Read|Grep|Glob|Bash|PowerShell. Blocks reads/searches
of the archive/ directory unless the user has explicitly asked for it.

Rationale: archive/ contains historical snapshots (dc.sh workflow,
old metrics dumps, retired scripts/recipes) that are no longer relevant
to current work. Agents that browse it pick up outdated terminology,
deprecated tools, and stale code paths — and end up confused.

The user's policy (2026-05-31): "Leave the archive but lets prevent agents
from looking in the archive without my permission. I dont want them
getting confused, i usually archive stuff that is no longer relevant at
all."

Exit codes:
  0 — allow (path is not inside archive/, or the user explicitly requested it)
  2 — block (reading archive/ without explicit user request)

What counts as "user explicitly requested":
- The user message in this turn mentions "archive" (case-insensitive). The
  hook detects this by looking at the most recent user turn in the project
  transcript and grepping for /\barchive\b/i. This is best-effort — if it
  can't determine intent it errs on the side of blocking.
"""
from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path


def _path_in_archive(p: str) -> bool:
    """True if `p` (a file path or command string) targets archive/."""
    if not p:
        return False
    # Normalize separators
    n = p.replace("\\", "/").lower()
    # Match `archive/...` at start of path or after `/` or whitespace
    return bool(re.search(r"(?:^|[/\s\"'])archive/", n))


def _user_asked_about_archive() -> bool:
    """Best-effort check: did the user's most recent message mention archive?
    Reads the project transcript and checks the last user turn."""
    sid = os.environ.get("CLAUDE_SESSION_ID")
    if not sid:
        return False
    # Project transcript lives at ~/.claude/projects/<munged>/<sid>.jsonl
    proj = os.environ.get("CLAUDE_PROJECT_DIR", "")
    if not proj:
        return False
    munged = proj.replace("\\", "-").replace("/", "-").replace(":", "")
    if munged.startswith("-"):
        munged = munged.lstrip("-")
    # Try both with and without leading C-- on Windows paths
    candidates = [
        Path.home() / ".claude" / "projects" / munged / f"{sid}.jsonl",
        Path.home() / ".claude" / "projects" / f"C--{munged}" / f"{sid}.jsonl",
    ]
    transcript = next((p for p in candidates if p.exists()), None)
    if not transcript:
        return False
    try:
        text = transcript.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return False
    # Walk lines in reverse, find the most recent user message, check for "archive"
    for line in reversed(text.splitlines()):
        if not line.strip():
            continue
        try:
            rec = json.loads(line)
        except Exception:
            continue
        if rec.get("type") == "user":
            # User message — check content for archive mention
            content = rec.get("message", {}).get("content", "")
            if isinstance(content, list):
                content = " ".join(
                    (c.get("text", "") if isinstance(c, dict) else str(c))
                    for c in content
                )
            return bool(re.search(r"\barchive\b", str(content), re.IGNORECASE))
    return False


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    tool_name = payload.get("tool_name", "")
    tool_input = payload.get("tool_input", {})

    # Determine which path/command to check based on tool
    targets: list[str] = []
    if tool_name == "Read":
        targets.append(tool_input.get("file_path", ""))
    elif tool_name == "Glob":
        targets.append(tool_input.get("pattern", ""))
        targets.append(tool_input.get("path", ""))
    elif tool_name == "Grep":
        targets.append(tool_input.get("path", ""))
    elif tool_name in ("Bash", "PowerShell"):
        targets.append(tool_input.get("command", ""))

    targets = [t for t in targets if t]
    if not any(_path_in_archive(t) for t in targets):
        return 0

    # Path targets archive/. Check if user explicitly asked for it.
    if _user_asked_about_archive():
        return 0

    # Block.
    print("BLOCKED by archive_read_guard.py:", file=sys.stderr)
    print("  This tool call targets the archive/ directory, which contains historical", file=sys.stderr)
    print("  snapshots (dc.sh workflow, retired scripts, old metrics dumps) that are no", file=sys.stderr)
    print("  longer relevant to current work. Reading it pulls in outdated terminology,", file=sys.stderr)
    print("  deprecated tool names, and stale code paths.", file=sys.stderr)
    print("", file=sys.stderr)
    print("  If you genuinely need to consult the archive, ask the user first — they", file=sys.stderr)
    print("  can mention 'archive' in their next message and this guard will allow the", file=sys.stderr)
    print("  access through.", file=sys.stderr)
    print("", file=sys.stderr)
    print("  For current workflow docs see: CLAUDE.md, AGENTS.md, memory/MEMORY.md,", file=sys.stderr)
    print("  .claude/rules/, and engine/.", file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
