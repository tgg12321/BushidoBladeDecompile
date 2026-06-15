#!/usr/bin/env python3
"""PreToolUse hook on Bash|PowerShell. Blocks the worktree CROSS-CONTAMINATION
footgun that mangled work on 2026-06-14.

Root cause: tools/eng.ps1 resolves its repo root from $PSScriptRoot, so a
RELATIVE `& tools/eng.ps1 ...` runs the engine against whatever the *shell cwd*
points at. A worker subagent's PowerShell cwd is ALWAYS the MAIN repo (the
`cd ../bb2-work-<id>` it ran earlier does NOT persist across tool calls), so a
relative eng.ps1 silently builds/scores/MUTATES MAIN's src instead of the
worker's worktree — corrupting the worker's own results AND leaving stray edits
/ staged files on shared main that mangle other agents' work.

The fix is `tools/wteng.ps1 <target> ...`, which takes the target repo
EXPLICITLY (cwd-independent). This guard ENFORCES it: any engine/build/sweep
command whose target depends on the ambiguous cwd is blocked, with a pointer to
the pinned form. Explicitly-pinned commands are allowed:
  - via `tools/wteng.ps1 <id|main|path> ...`         (the canonical form)
  - via an ABSOLUTE-path invocation (`& "C:\\...\\tools\\eng.ps1"`)
  - via an ABSOLUTE WSL `cd` (`wsl bash -c "cd '/mnt/.../<repo>' && ... make"`)

Exit codes:  0 — allow   2 — block
Fails OPEN (exit 0) on any parsing error: a guard must never wedge real work.
"""
from __future__ import annotations

import json
import re
import sys

# Path tokens that end in eng.ps1 (captures the invocation path, incl. wteng.ps1).
# Three alternatives so a QUOTED path with spaces (e.g. "C:\Bushido Blade 2\...\
# eng.ps1") is captured WHOLE — otherwise the absolute-path test below sees only
# a relative-looking fragment after the last space.
ENG_TOKEN_RE = re.compile(
    r'"([^"]*eng\.ps1)"'           # double-quoted path
    r"|'([^']*eng\.ps1)'"          # single-quoted path
    r"|(?:^|[\s&(])([^\s\"'&|;()]*eng\.ps1)",  # bare token
    re.IGNORECASE,
)
# A `make` command word inside a shell (start / after && ; | or a `&& source ... &&`).
MAKE_RE = re.compile(r"(?:^|&&|;|\|)\s*(?:[A-Za-z_]+=\S+\s+)*make\b")
SWEEP_RE = re.compile(r"\bsweep_variants\.py\b|\bprologue_fix\.py\b")
RAW_ENGINE_RE = re.compile(r"\bpython3?\s+-m\s+engine\.cli\b")

# "This command explicitly pins its target repo" signals.
WTENG_RE = re.compile(r"\bwteng\.ps1\b", re.IGNORECASE)
WSL_ABS_RE = re.compile(r"/mnt/[a-z]/", re.IGNORECASE)          # absolute WSL cd
WIN_ABS_RE = re.compile(r"[A-Za-z]:[\\/]")                       # absolute Windows path

SAFE = (
    "\nUse the WORKTREE-PINNED wrapper so the engine targets the right repo "
    "regardless of shell cwd:\n"
    "  - in a worker worktree:   & tools/wteng.ps1 <your-worktree-id> <subcmd ...>\n"
    "        & tools/wteng.ps1 orch0614b-3 sandbox func_X --disable all\n"
    "        & tools/wteng.ps1 orch0614b-3 make\n"
    "  - on the main repo:       & tools/wteng.ps1 main <subcmd ...>\n"
    "        & tools/wteng.ps1 main queue status\n"
    "  - (or invoke eng.ps1 / make by an ABSOLUTE path / absolute `cd '/mnt/...'`.)\n"
    "Why: a RELATIVE `& tools/eng.ps1` resolves to whatever the shell cwd is — "
    "for a subagent that is ALWAYS main, so it builds/mutates MAIN, not your "
    "worktree (the 2026-06-14 contamination incident). See tools/wteng.ps1.")


def is_absolute(token: str) -> bool:
    return bool(re.match(r"^[A-Za-z]:[\\/]", token)) or token.startswith("/") or token.startswith("\\\\")


def has_explicit_pin(cmd: str) -> bool:
    """True if the command explicitly pins its target repo (wteng or any absolute path)."""
    return bool(WTENG_RE.search(cmd) or WSL_ABS_RE.search(cmd) or WIN_ABS_RE.search(cmd))


def reasons_for(cmd: str) -> list[str]:
    out: list[str] = []

    # 1. Relative eng.ps1 (the primary, confirmed vector). Precise: inspect each
    #    path token ending in eng.ps1; allow wteng.ps1 and absolute paths.
    for groups in ENG_TOKEN_RE.findall(cmd):
        token = next((g for g in groups if g), "")  # the non-empty alternative
        if not token:
            continue
        base = re.split(r"[\\/]", token)[-1].lower()
        if base == "wteng.ps1":
            continue
        if base == "eng.ps1" and not is_absolute(token):
            out.append(
                f"Relative engine invocation `{token}` — resolves to the shell's cwd "
                f"(for a subagent that is ALWAYS main), so it runs the engine against "
                f"MAIN, not your worktree. This is the cross-contamination footgun.")
            break

    pinned = has_explicit_pin(cmd)

    # 2. `make` build with no explicit pin (would build/corrupt main's build/).
    if not pinned and MAKE_RE.search(cmd) and ("wsl" in cmd.lower() or "source .venv" in cmd):
        out.append(
            "`make` with no pinned target (no `wteng.ps1` and no absolute `cd '/mnt/...'`). "
            "A cwd-relative build runs against MAIN.")

    # 3. sweep_variants / prologue_fix (edit src in place) with no explicit pin.
    if not pinned and SWEEP_RE.search(cmd):
        out.append(
            "An in-place src-mutating tool (sweep_variants/prologue_fix) with no pinned "
            "target — would edit MAIN's src. Pin it via wteng or an absolute path.")

    # 4. Raw `python3 -m engine.cli` with no pin (also caught by shell_footgun_guard,
    #    but double-cover the contamination angle).
    if not pinned and RAW_ENGINE_RE.search(cmd):
        out.append(
            "Hand-rolled `python3 -m engine.cli` with no pinned target. Use "
            "`tools/wteng.ps1 <target> <subcmd>`.")

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
    try:
        reasons = reasons_for(cmd)
    except Exception:
        return 0  # fail open
    if reasons:
        print("BLOCKED by worktree_contamination_guard.py — would run the engine/build "
              "against the wrong repo:", file=sys.stderr)
        for r in reasons:
            print(f"  - {r}", file=sys.stderr)
        print(SAFE, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
