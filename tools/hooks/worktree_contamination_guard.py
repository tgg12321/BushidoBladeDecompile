#!/usr/bin/env python3
"""PreToolUse hook — blocks the worktree CROSS-CONTAMINATION footgun that
mangled work on 2026-06-14.

Root cause: a subagent worker's shell cwd is ALWAYS the MAIN repo (the
`cd ../bb2-work-<id>` it ran earlier does NOT persist across tool calls), so any
cwd-relative engine/build/git/edit op silently builds/scores/MUTATES MAIN's src
instead of the worker's worktree — corrupting the worker's own results AND
leaving stray edits / staged files / commits on shared main that mangle other
agents' work.

Covered tools / vectors:
  Bash|PowerShell:
    - relative `eng.ps1` INVOCATION  (not wteng, not absolute)        -> block
    - unpinned `make`                (no absolute `cd '/mnt/...'`, no wteng) -> block
    - unpinned sweep_variants / prologue_fix (file or `-m` module form)  -> block
    - unpinned raw `python3 -m engine.cli` (tolerant spacing) / engine import -> block
  Edit|Write|MultiEdit:
    - editing a MAIN-repo BUILD INPUT (src/*.c, *.h, regfix/asmfix/*.ld,
      prologue_config.json, inline_asm_canonical.txt, ...) while worker
      worktrees are live (a worker editing main instead of its worktree) -> block

Pinned/explicit forms are ALLOWED: `tools/wteng.ps1 <target> ...`, an
ABSOLUTE-path eng.ps1, an absolute WSL `cd '/mnt/.../<repo>' && ... make`, and
edits whose path is inside a bb2-work-* worktree. Reader commands that merely
MENTION a path (`git show HEAD:tools/eng.ps1`, `git commit -m '...eng.ps1...'`,
`cat`/`grep`) are NOT invocations and are allowed.

Exit codes:  0 — allow   2 — block.  Fails OPEN (exit 0) on any error, but logs
the failure to stderr so a swallowed bypass is visible.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys

# ── command-string detectors (Bash|PowerShell) ──────────────────────────────
# Tokens ending in eng.ps1 (quote-aware so spaced paths capture whole).
ENG_TOKEN_RE = re.compile(
    r'"([^"]*eng\.ps1)"'
    r"|'([^']*eng\.ps1)'"
    r"|([^\s\"'&|;()]*eng\.ps1)",
    re.IGNORECASE,
)
# A `make` shell-word after ANY command separator / start / subshell.
MAKE_RE = re.compile(r"(?:^|&&|\|\||[;&|(\n]|\$\()\s*(?:[A-Za-z_]+=\S+\s+)*make\b")
# sweep_variants / prologue_fix, as a .py file OR a python module (tools.x or x).
SWEEP_RE = re.compile(
    r"\b(?:sweep_variants|prologue_fix)\.py\b"
    r"|\b(?:tools\.)?(?:sweep_variants|prologue_fix)\b(?=\s|$)",
    re.IGNORECASE,
)
# raw engine.cli: tolerate `-m engine.cli`, `-mengine.cli`, and `from engine`.
RAW_ENGINE_RE = re.compile(
    r"\bpython3?\s+-m\s*engine\.cli\b"
    r"|\bfrom\s+engine(?:\.\w+)?\s+import\b"
    r"|\bimport\s+engine\.cli\b",
    re.IGNORECASE,
)
WTENG_RE = re.compile(r"\bwteng\.ps1\b", re.IGNORECASE)
# A pin that is the actual TARGET: an absolute WSL `cd` (for make).
CD_ABS_PIN_RE = re.compile(r"\bcd\s+['\"]?/mnt/[a-z]/", re.IGNORECASE)
# Reader/inspection commands that merely MENTION a path (not invoke it).
READER_RE = re.compile(r"\b(?:git|cat|grep|egrep|rg|less|head|tail|sed|awk|echo|printf|type|Get-Content|Select-String)\b")
FLAG_MENTION_RE = re.compile(r"-[mFc]\s*$")  # token follows a -m/-F/-c message flag


def is_absolute(token: str) -> bool:
    return bool(re.match(r"^[A-Za-z]:[\\/]", token)) or token.startswith("/") or token.startswith("\\\\")


def eng_invocation(cmd: str) -> str | None:
    """Return the offending relative eng.ps1 INVOCATION token, or None.
    A token is a mention (allowed) if a reader command precedes it, or it
    follows a -m/-F/-c message flag — those don't execute eng.ps1."""
    for m in ENG_TOKEN_RE.finditer(cmd):
        token = next((g for g in m.groups() if g), "")
        if not token:
            continue
        base = re.split(r"[\\/]", token)[-1].lower()
        if base == "wteng.ps1" or base != "eng.ps1":
            continue
        if is_absolute(token):
            continue
        before = cmd[: m.start()]
        # Mention, not invocation: a reader command precedes it, or it's a flag arg.
        if READER_RE.search(before) or FLAG_MENTION_RE.search(before):
            continue
        return token
    return None


def reasons_for_cmd(cmd: str) -> list[str]:
    out: list[str] = []

    tok = eng_invocation(cmd)
    if tok:
        out.append(
            f"Relative engine invocation `{tok}` — resolves to the shell cwd (for a "
            f"subagent that is ALWAYS main), so it runs the engine against MAIN, not "
            f"your worktree. The cross-contamination footgun.")

    has_wteng = bool(WTENG_RE.search(cmd))

    if not has_wteng and MAKE_RE.search(cmd) and not CD_ABS_PIN_RE.search(cmd):
        out.append(
            "`make` with no pinned target — no `wteng.ps1` and no absolute "
            "`cd '/mnt/.../<repo>'` in the same command. A cwd-relative build runs "
            "against MAIN (corrupts its build/oracle).")

    if not has_wteng and SWEEP_RE.search(cmd):
        out.append(
            "An in-place src-mutating tool (sweep_variants/prologue_fix) with no "
            "`wteng.ps1` pin — would edit MAIN's src. Run it via wteng.")

    if not has_wteng and RAW_ENGINE_RE.search(cmd):
        out.append(
            "Hand-rolled engine invocation (`engine.cli` / `from engine import`) with "
            "no `wteng.ps1` pin — runs against the shell cwd (= MAIN).")

    return out


# ── Edit|Write|MultiEdit detector ───────────────────────────────────────────
BUILD_INPUT_RE = re.compile(
    r"(?:^|[\\/])(?:"
    r"src[\\/].*\.c"
    r"|include[\\/].*\.h"
    r"|.*\.h"
    r"|regfix(?:_stage2)?\.txt|asmfix\.txt"
    r"|.*\.ld"
    r"|prologue_config\.json|inline_asm_canonical\.txt|maspsx_label_nop_funcs\.txt"
    r"|sdata\w*\.txt|named_syms\.txt|expand_lb_funcs\.txt"
    r")$",
    re.IGNORECASE,
)


def _git(cwd: str, *args: str) -> str:
    try:
        return subprocess.run(["git", "-C", cwd, *args], capture_output=True,
                              text=True, timeout=8).stdout
    except Exception:
        return ""


def reasons_for_edit(file_path: str) -> list[str]:
    if not file_path:
        return []
    fp = os.path.abspath(file_path)
    if not BUILD_INPUT_RE.search(fp.replace("\\", "/")):
        return []  # not a decomp build input — edits to docs/tools/etc. are fine
    d = os.path.dirname(fp)
    if not os.path.isdir(d):
        return []
    toplevel = _git(d, "rev-parse", "--show-toplevel").strip()
    if not toplevel:
        return []
    tl_norm = os.path.normcase(os.path.abspath(toplevel.replace("/", os.sep)))
    # Editing a file INSIDE a worker worktree is correct — allow.
    leaf = os.path.basename(tl_norm)
    if leaf.startswith("bb2-work-") or (os.sep + os.path.normcase("worktrees") + os.sep) in tl_norm:
        return []
    # Else the edit targets a NON-worktree repo (the main repo). Block ONLY if
    # worker worktrees are currently live (a batch is active) — otherwise normal
    # solo/orchestrator edits to main are fine.
    # Orchestrator override: a persistent sentinel the orchestrator drops when it
    # legitimately needs to edit main's build inputs during a batch (workers never
    # create it). Persists on disk across tool calls (unlike env/cwd).
    if os.path.exists(os.path.join(tl_norm, "tmp", ".allow_main_edits")):
        return []
    wt = _git(d, "worktree", "list", "--porcelain")
    has_workers = bool(re.search(r"^worktree .*[\\/]bb2-work-", wt, re.MULTILINE))
    if has_workers:
        return [
            f"Editing a MAIN-repo build input ({os.path.basename(fp)}) while worker "
            f"worktrees are live. If you are a worker, you almost certainly meant your "
            f"OWN worktree — edit  ..\\bb2-work-<id>\\... , not the main repo. (If you "
            f"are the orchestrator and this is intentional, edit via git or remove the "
            f"worktrees first.)"]
    return []


SAFE = (
    "\nPin the target so it can't hit the wrong repo regardless of shell cwd:\n"
    "  - worker worktree:  & tools/wteng.ps1 <your-id> <subcmd>   (e.g. sandbox <func> --disable all ; make)\n"
    "  - main repo:        & tools/wteng.ps1 main <subcmd>\n"
    "  - edits: use your worktree's ABSOLUTE path  ..\\bb2-work-<id>\\src\\... — never the main repo path.\n"
    "Why: a subagent's cwd is ALWAYS main; a relative engine/make/edit hits MAIN, "
    "not your worktree (the 2026-06-14 contamination incident). See tools/wteng.ps1.")


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception as e:
        print(f"worktree_contamination_guard: fail-open (bad payload: {e})", file=sys.stderr)
        return 0
    tool = payload.get("tool_name")
    ti = payload.get("tool_input", {}) or {}
    try:
        if tool in ("Bash", "PowerShell"):
            reasons = reasons_for_cmd(ti.get("command", "") or "")
        elif tool in ("Edit", "Write", "MultiEdit"):
            reasons = reasons_for_edit(ti.get("file_path", "") or "")
        else:
            return 0
    except Exception as e:
        print(f"worktree_contamination_guard: fail-open (error: {e})", file=sys.stderr)
        return 0
    if reasons:
        print("BLOCKED by worktree_contamination_guard.py — would touch the wrong repo:", file=sys.stderr)
        for r in reasons:
            print(f"  - {r}", file=sys.stderr)
        print(SAFE, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
