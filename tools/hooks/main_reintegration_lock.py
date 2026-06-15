#!/usr/bin/env python3
"""PreToolUse hook — MAIN-REINTEGRATION MUTEX.

Prevents the 2026-06-14 ORCHESTRATOR-vs-ORCHESTRATOR collision: two orchestrator
sessions concurrently mutating the MAIN repo's working tree / refs, where one
session's `git merge` / `git reset` (clean-the-tree-then-merge) CLOBBERS the
other session's still-uncommitted reintegration. (That is exactly what happened:
orch0614b merged its batch onto main while orch3's 8-match reintegration sat
uncommitted in main's working tree, wiping it.)

This is the COMPLEMENT to worktree_contamination_guard.py:
  - contamination_guard  -> WORKER edits/builds main instead of its worktree
  - THIS guard           -> two ORCHESTRATORS both reintegrate to main at once

## Model

While a batch is active (worker `bb2-work-*` worktrees are live) OR a lock is
already held, any MAIN-MUTATING operation requires the caller to HOLD the
main-reintegration lock for THIS session:

    tmp/.main_reintegration.lock   (gitignored; JSON {session, ts, head, label})

  - lock absent        -> BLOCK: acquire it first (& tools/reintegrate_lock.ps1 acquire)
  - lock held by ME    -> ALLOW
  - lock held by OTHER -> BLOCK: another orchestrator is reintegrating; wait for release
  - lock held by OTHER but STALE (> STALE_SECS) -> BLOCK + suggest `... steal`

Solo work (no worker worktrees, no lock) -> the guard is a NO-OP (zero friction).

## Identity

The Claude session id. The hook reads it from the stdin payload `session_id`;
the lock WRITER (reintegrate_lock.ps1) stamps `$env:CLAUDE_CODE_SESSION_ID`.
Both resolve to the same Claude Code session UUID.

## Guarded MAIN-mutating vectors

  Bash|PowerShell (when NOT cd-ed / pinned into a bb2-work-* worktree):
    git (merge|rebase|cherry-pick|revert|reset|am|clean)
    git stash (apply|pop|push|drop|clear)
    git (checkout|restore|switch)        # can overwrite the working tree
    git apply  (unless --check / --stat / --numstat / --summary — those are read-only)
    git commit
  Edit|Write|MultiEdit:
    a MAIN-repo BUILD INPUT (src/*.c, *.h, regfix/asmfix, *.ld, prologue_config.json,
    inline_asm_canonical.txt, ...) — exclusivity on top of the contamination guard.

Read-only git (show/log/diff/status/rev-parse/cat-file/worktree/grep/ls-files) is
NOT guarded. Exit 0 = allow, 2 = block. Fails OPEN (exit 0) on any error.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time

LOCK_REL = os.path.join("tmp", ".main_reintegration.lock")
STALE_SECS = 5400  # 90 min — a lock older than this from a (presumed dead) session is reclaimable via `steal`

# ── main-mutating git / wteng detectors ─────────────────────────────────────
# A git verb that mutates the working tree or refs, as a shell word after a `git`.
GIT_MUTATE_RE = re.compile(
    r"\bgit\b[^\n;|&]*?\b("
    r"merge|rebase|cherry-pick|revert|reset|am|clean|commit|checkout|restore|switch"
    r")\b",
    re.IGNORECASE,
)
GIT_STASH_MUTATE_RE = re.compile(r"\bgit\b[^\n;|&]*?\bstash\b\s*(apply|pop|push|drop|clear|save)\b", re.IGNORECASE)
# `git apply` that WRITES (not --check / --stat / --numstat / --summary).
GIT_APPLY_RE = re.compile(r"\bgit\b[^\n;|&]*?\bapply\b", re.IGNORECASE)
GIT_APPLY_READONLY_RE = re.compile(r"--(check|stat|numstat|summary)\b", re.IGNORECASE)
# wteng main <queue done|regen|park> — mutates main's engine/queue.json.
WTENG_MAIN_MUTATE_RE = re.compile(r"\bwteng\.ps1\s+main\s+queue\s+(done|regen|park)\b", re.IGNORECASE)
# A cd / pin INTO a worker worktree -> the op targets that worktree, not main.
WORKTREE_PIN_RE = re.compile(r"bb2-work-[A-Za-z0-9._-]+|wteng\.ps1\s+(?!main\b)\S", re.IGNORECASE)

# Build inputs (mirror of worktree_contamination_guard.BUILD_INPUT_RE).
BUILD_INPUT_RE = re.compile(
    r"(?:^|[\\/])(?:"
    r"src[\\/].*\.c"
    r"|include[\\/].*\.h|.*\.h"
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


def main_repo_root(start: str) -> str | None:
    """Return the MAIN repo toplevel, or None if `start` is inside a worktree."""
    toplevel = _git(start, "rev-parse", "--show-toplevel").strip()
    if not toplevel:
        return None
    tl = os.path.normcase(os.path.abspath(toplevel.replace("/", os.sep)))
    leaf = os.path.basename(tl)
    if leaf.startswith("bb2-work-") or (os.sep + "worktrees" + os.sep) in tl:
        return None  # inside a worker worktree — not main
    return tl


def workers_live(root: str) -> bool:
    wt = _git(root, "worktree", "list", "--porcelain")
    return bool(re.search(r"^worktree .*[\\/]bb2-work-", wt, re.MULTILINE))


def read_lock(root: str) -> dict | None:
    try:
        with open(os.path.join(root, LOCK_REL), "r", encoding="utf-8") as fh:
            return json.load(fh)
    except FileNotFoundError:
        return None
    except Exception:
        return None


def is_main_mutating_cmd(cmd: str) -> bool:
    # If the command cd's / pins INTO a worker worktree, it targets that worktree.
    if WORKTREE_PIN_RE.search(cmd):
        return False
    if GIT_MUTATE_RE.search(cmd):
        return True
    if GIT_STASH_MUTATE_RE.search(cmd):
        return True
    if GIT_APPLY_RE.search(cmd) and not GIT_APPLY_READONLY_RE.search(cmd):
        return True
    if WTENG_MAIN_MUTATE_RE.search(cmd):
        return True
    return False


def block(root: str, sid: str, lock: dict | None, what: str) -> int:
    acquire = "& tools/reintegrate_lock.ps1 acquire"
    if lock is None:
        msg = [
            f"BLOCKED by main_reintegration_lock.py — {what} on MAIN while a batch is active,",
            "but you do NOT hold the main-reintegration lock.",
            f"  Acquire it first (only ONE orchestrator may mutate main at a time):  {acquire}",
            "  Release when done:  & tools/reintegrate_lock.ps1 release",
            "Why: the 2026-06-14 orch3/orch0614b collision — a concurrent `git merge`/`reset`",
            "on main wiped another session's uncommitted reintegration.",
        ]
    else:
        age = ""
        try:
            age = f" (held {int((time.time() - float(lock.get('ts', 0))) / 60)} min)"
        except Exception:
            pass
        holder = lock.get("session", "?")
        stale = False
        try:
            stale = (time.time() - float(lock.get("ts", 0))) > STALE_SECS
        except Exception:
            pass
        msg = [
            f"BLOCKED by main_reintegration_lock.py — {what} on MAIN, but the",
            f"main-reintegration lock is held by ANOTHER session{age}:",
            f"    holder session_id = {holder}",
            "Do NOT mutate main concurrently — wait for that orchestrator to release",
            "(& tools/reintegrate_lock.ps1 status). This is the exact collision the lock prevents.",
        ]
        if stale:
            msg.append(
                f"  The lock looks STALE (> {STALE_SECS // 60} min). If that session is dead, reclaim it:")
            msg.append("    & tools/reintegrate_lock.ps1 steal --reason '<why>'")
    print("\n".join(msg), file=sys.stderr)
    return 2


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception as e:
        print(f"main_reintegration_lock: fail-open (bad payload: {e})", file=sys.stderr)
        return 0
    try:
        tool = payload.get("tool_name")
        ti = payload.get("tool_input", {}) or {}
        sid = str(payload.get("session_id") or "")
        cwd = payload.get("cwd") or os.getcwd()

        # Resolve the MAIN repo root from cwd; if we're inside a worktree, never our concern.
        root = main_repo_root(cwd)
        if not root:
            return 0

        # Identify whether THIS op is a guarded main-mutation.
        what = None
        if tool in ("Bash", "PowerShell"):
            if is_main_mutating_cmd(ti.get("command", "") or ""):
                what = "a git/queue working-tree mutation"
        elif tool in ("Edit", "Write", "MultiEdit"):
            fp = ti.get("file_path", "") or ""
            if fp and BUILD_INPUT_RE.search(os.path.abspath(fp).replace("\\", "/")):
                # Only if the edit targets the MAIN repo (not a worktree).
                if main_repo_root(os.path.dirname(os.path.abspath(fp)) or root):
                    what = f"an edit of a build input ({os.path.basename(fp)})"
        if not what:
            return 0

        lock = read_lock(root)

        # Engage only when a batch is active OR a lock already exists (contention possible).
        if not workers_live(root) and lock is None:
            return 0  # solo, uncontended — no friction

        # Held by me -> allow.
        if lock is not None and sid and lock.get("session") == sid:
            return 0

        return block(root, sid, lock, what)
    except Exception as e:
        print(f"main_reintegration_lock: fail-open (error: {e})", file=sys.stderr)
        return 0


if __name__ == "__main__":
    sys.exit(main())
