#!/usr/bin/env python3
"""PreToolUse hook on Bash|PowerShell. Targeted BLOCK of the shell-nesting
footguns that repeatedly waste agent turns on this project.

The decomp toolchain only runs under WSL, so engine/build commands are really
`wsl bash -c 'cd <root> && source .venv && python3 -m engine.cli ...'`. Authoring
that through the Bash tool nests THREE shells (Git Bash -> wsl -> bash); every
`$`, quote and backslash is parsed three times. Inline awk/sed/heredocs and
hand-escaped quotes break in ways that look like tool bugs but are quoting.

This hook blocks ONLY the clear footguns and points to the safe path. Simple
one-command `wsl bash -c 'git status'`, pipes, and POSIX one-liners are allowed.

Blocks (exit 2, reason on stderr):
  1. Nested single-quote escaping  '"'"'  or  '\''   (always a footgun)
  2. Hand-rolled engine invocation `python3 -m engine.cli ...` (use tools/eng.ps1)
  3. Inside a `wsl bash -c '...'`: a heredoc (<<), a shell function def `() {`,
     or inline awk/sed touching `$` -> write a .py/.sh/.ps1 file in tmp/ and run it.

Exit codes:
  0 — allow
  2 — block
"""
from __future__ import annotations

import json
import re
import sys

NESTED_QUOTE_RE = re.compile(r"""'"'"'|'\\''""")
ENGINE_DIRECT_RE = re.compile(r"\bpython3?\s+-m\s+engine\.cli\b")
WSL_NEST_RE = re.compile(r"\bwsl\s+bash\s+-l?c\b")
HEREDOC_RE = re.compile(r"<<-?\s*[\"']?\w")
FUNCDEF_RE = re.compile(r"\b[A-Za-z_]\w*\s*\(\)\s*\{")
AWK_SED_DOLLAR_RE = re.compile(r"\b(?:awk|sed)\b[^|;&]*\\?\$")

SAFE = ("\nThe safe path:\n"
        "  - engine/build commands  ->  the PowerShell tool with  tools/eng.ps1\n"
        "        & tools/eng.ps1 queue next\n"
        "        & tools/eng.ps1 sandbox func_X --disable all\n"
        "  - anything beyond ONE simple command (awk/sed/heredoc/multi-statement)\n"
        "        ->  Write a .py/.sh/.ps1 file to tmp/ and run THAT (zero nested quoting)\n"
        "  - a multi-line commit message  ->  Write tmp/msg.txt, then  git commit -F tmp/msg.txt\n"
        "See the 'PowerShell-first scripting' section of CLAUDE.md / AGENTS.md.")


def reasons_for(cmd: str) -> list[str]:
    out: list[str] = []

    if NESTED_QUOTE_RE.search(cmd):
        out.append(
            "Nested single-quote escaping ('\"'\"' or '\\'') detected — the classic "
            "three-shell-nesting footgun. Don't hand-escape quotes through wsl."
        )

    if ENGINE_DIRECT_RE.search(cmd) and "eng.ps1" not in cmd:
        out.append(
            "Hand-rolled `python3 -m engine.cli ...`. Engine commands go through the "
            "PowerShell wrapper so there is zero quoting and metrics get attributed: "
            "`& tools/eng.ps1 <subcommand> ...`."
        )

    if WSL_NEST_RE.search(cmd):
        if HEREDOC_RE.search(cmd):
            out.append(
                "Heredoc (<<) inside `wsl bash -c '...'`. Heredocs through nested shells "
                "are fragile. For commit messages use `git commit -F tmp/msg.txt`; "
                "otherwise write a script file to tmp/ and run it."
            )
        if FUNCDEF_RE.search(cmd):
            out.append(
                "Shell function definition `() { ... }` inside `wsl bash -c '...'`. "
                "Put the logic in a .py/.sh file in tmp/ and run that file instead."
            )
        if AWK_SED_DOLLAR_RE.search(cmd):
            out.append(
                "Inline awk/sed touching `$` inside `wsl bash -c '...'`. The `$` is parsed "
                "by three shells before awk sees it. Write a .py file to tmp/ and run it "
                "(a Python normalizer is both more robust and easier to read)."
            )
    return out


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0
    if payload.get("tool_name") not in ("Bash", "PowerShell"):
        return 0
    cmd = payload.get("tool_input", {}).get("command", "")
    if not cmd:
        return 0

    reasons = reasons_for(cmd)
    if reasons:
        print("BLOCKED by shell_footgun_guard.py — shell-nesting footgun:", file=sys.stderr)
        for r in reasons:
            print(f"  - {r}", file=sys.stderr)
        print(SAFE, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
