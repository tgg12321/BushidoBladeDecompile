#!/usr/bin/env python3
"""PreToolUse hook on Edit|Write to CLAUDE.md and AGENTS.md.

Soft guard (warns, doesn't block) against:
- CLAUDE.md growing past ~200 lines or by >50 lines in one edit
- AGENTS.md growing past ~250 lines
- `@AGENTS.md` import being removed from CLAUDE.md
- Rule-shaped content (HARD RULE / FORBIDDEN / MUST / NEVER) appearing
  in CLAUDE.md — those belong in memory/rules/ atomic files
- Tool-agnostic content (toolchain facts, build commands) ending up
  in CLAUDE.md instead of AGENTS.md

Exit codes:
  0 — allow (always; warnings printed to stderr but don't block)
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

CLAUDE_MAX_LINES = 200
CLAUDE_GROWTH_WARN_LINES = 50
AGENTS_MAX_LINES = 250

# Patterns that look like atomic rule content — should live in memory/rules/
RULE_PATTERNS = [
    r"\*\*FORBIDDEN:\*\*",
    r"\*\*REQUIRED:\*\*",
    r"\*\*HARD RULE\*\*",
    r"^## THE HARD RULE",
    r"^## Inline asm rules",
    r"^## Escalation ladder",
    r"^## Communication — DO NOT ASK",
    r"^## Pre-dive analysis",
    r"^## Programmatic tools first",
    r"^## Integration discipline",
    r"^## Post-match retrospective",
    r"^## Tool error handling",
    r"^## Debugging discipline",
]

# Patterns that look like AGENTS.md content — toolchain/build facts
AGENTS_PATTERNS = [
    r"PsyQ SDK 3\.\d+",
    r"^## Toolchain",
    r"^## Build commands",
    r"^## Build pipeline",
    r"^## Splat configuration",
    r"^## Executable details",
    r"^## Disc structure",
    r"mipsel-linux-gnu-cpp \| cc1",
]


def simulate_edit(file_path: str, old_string: str, new_string: str) -> str | None:
    p = Path(file_path)
    if not p.exists():
        return None
    current = p.read_text(encoding="utf-8", errors="replace")
    if old_string not in current:
        return None
    return current.replace(old_string, new_string, 1)


def simulate_multiedit(file_path: str, edits: list[dict]) -> str | None:
    p = Path(file_path)
    if not p.exists():
        return None
    content = p.read_text(encoding="utf-8", errors="replace")
    for edit in edits:
        old = edit.get("old_string", "")
        new = edit.get("new_string", "")
        if old not in content:
            return None
        content = content.replace(old, new, 1)
    return content


def check_claude_md(content: str, was_size: int | None) -> list[str]:
    warnings: list[str] = []
    new_size = content.count("\n") + 1

    if new_size > CLAUDE_MAX_LINES:
        warnings.append(
            f"CLAUDE.md will be {new_size} lines (target: <={CLAUDE_MAX_LINES}). "
            f"Consider moving content to memory/rules/, memory/workflow/, or AGENTS.md."
        )
    if was_size is not None and (new_size - was_size) > CLAUDE_GROWTH_WARN_LINES:
        warnings.append(
            f"CLAUDE.md is growing by {new_size - was_size} lines in this edit "
            f"(was {was_size}, becomes {new_size}). Large additions usually belong "
            f"in memory/rules/ (durable rules) or memory/workflow/ (harness mechanics)."
        )

    if "@AGENTS.md" not in content:
        warnings.append(
            "CLAUDE.md no longer imports `@AGENTS.md`. The import is what loads "
            "tool-agnostic project facts (toolchain, build, disc structure) into "
            "Claude's context. Re-add `@AGENTS.md` near the top unless you've "
            "intentionally inlined those facts."
        )

    for pat in RULE_PATTERNS:
        m = re.search(pat, content, re.MULTILINE)
        if m:
            warnings.append(
                f"CLAUDE.md contains a rule-shaped pattern (`{m.group(0)[:50]}`) — "
                f"this belongs in memory/rules/<rule>.md as an atomic rule file, "
                f"not in CLAUDE.md."
            )
            break  # one warning is enough; don't spam

    for pat in AGENTS_PATTERNS:
        m = re.search(pat, content, re.MULTILINE)
        if m:
            warnings.append(
                f"CLAUDE.md contains tool-agnostic content (`{m.group(0)[:50]}`) — "
                f"this belongs in AGENTS.md so other AI agents (Codex/Cursor/etc.) "
                f"can read it via the agents.md standard."
            )
            break

    return warnings


def check_agents_md(content: str, was_size: int | None) -> list[str]:
    warnings: list[str] = []
    new_size = content.count("\n") + 1
    if new_size > AGENTS_MAX_LINES:
        warnings.append(
            f"AGENTS.md will be {new_size} lines (target: <={AGENTS_MAX_LINES}). "
            f"Consider whether the new content really belongs here vs. in "
            f"docs/ (deeper engineering reference)."
        )
    return warnings


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    tool = payload.get("tool_name", "")
    tool_input = payload.get("tool_input", {})
    file_path = tool_input.get("file_path", "")
    if not file_path:
        return 0

    name = Path(file_path).name
    if name not in ("CLAUDE.md", "AGENTS.md"):
        return 0

    p = Path(file_path)
    was_size = p.read_text(encoding="utf-8", errors="replace").count("\n") + 1 if p.exists() else None

    # Determine new content
    if tool == "Write":
        content = tool_input.get("content", "")
    elif tool == "Edit":
        content = simulate_edit(
            file_path,
            tool_input.get("old_string", ""),
            tool_input.get("new_string", ""),
        )
    elif tool == "MultiEdit":
        content = simulate_multiedit(file_path, tool_input.get("edits", []))
    else:
        return 0

    if content is None:
        return 0

    if name == "CLAUDE.md":
        warnings = check_claude_md(content, was_size)
    else:
        warnings = check_agents_md(content, was_size)

    if warnings:
        print(f"WARN from claude_md_guard.py on {name}:", file=sys.stderr)
        for w in warnings:
            print(f"  - {w}", file=sys.stderr)
        print(
            "\n(Warning only — write proceeds. Re-read this hint when planning the next edit.)",
            file=sys.stderr,
        )
    # Always allow
    return 0


if __name__ == "__main__":
    sys.exit(main())
