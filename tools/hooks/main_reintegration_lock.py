#!/usr/bin/env python3
"""PreToolUse hook — MAIN-REINTEGRATION MUTEX  (hardened v2 after adversarial review).

Prevents the 2026-06-14 ORCHESTRATOR-vs-ORCHESTRATOR collision: two orchestrator
sessions concurrently mutating the MAIN repo's working tree / refs, where one
session's `git merge` / `git reset` CLOBBERS the other's still-uncommitted
reintegration. (orch0614b merged its batch onto main while orch3's 8-match
reintegration sat uncommitted in main's working tree, wiping it.)

Complement to worktree_contamination_guard.py:
  - contamination_guard  -> a WORKER edits/builds main instead of its worktree
  - THIS guard           -> two ORCHESTRATORS both reintegrate to main at once

## Model
While a batch is active (worker `bb2-work-*` worktrees live) OR a lock exists,
any MAIN-MUTATING op requires the caller to HOLD the lock for THIS session:
    tmp/.main_reintegration.lock   (gitignored; {session, ts, head, label})
  absent -> BLOCK (acquire first) ; mine -> ALLOW ; other -> BLOCK ; other+stale -> BLOCK+steal hint.
Solo work (no worktrees, no lock) -> NO-OP.

## Hardening (v2, from the adversarial review)
  - verb/pin matched at the git SUBCOMMAND position (not anywhere in the string),
    so `git log --grep=merge` / a `merge` inside a message is NOT a false mutate.
  - commit/merge MESSAGES (-m/-F/--message/--file) and `#` comments are STRIPPED
    before matching, so a `bb2-work-` substring in a message can't fake a pin.
  - the mutation TARGET repo is resolved from `git -C` / `--git-dir` / a leading
    `cd` / `wteng <id>` (else cwd), so `git -C <main> merge` from a worktree cwd
    is still gated, and `git -C <worktree> ...` from main cwd is not.
  - broadened verbs: push|pull|fetch(:refspec)|update-ref|symbolic-ref|branch -f/-D|
    bare `git stash`|`patch -pN`.
Read-only git (show/log/diff/status/rev-parse/worktree) is never gated. Fails OPEN.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time

LOCK_REL = os.path.join("tmp", ".main_reintegration.lock")
STALE_SECS = 5400  # 90 min

# git GLOBAL options that may precede the subcommand (so the verb stays anchored).
# _ARG is quote-aware so a -C target with SPACES (e.g. "Bushido Blade 2 Decompile")
# is consumed whole and the verb after it is still recognised.
_ARG = r"(?:\"[^\"]*\"|'[^']*'|\S+)"
_GLOBALS = (
    rf"(?:-C\s+{_ARG}|--git-dir[=\s]{_ARG}|--work-tree[=\s]{_ARG}|-c\s+{_ARG}"
    r"|--no-pager|-p|--paginate|-P)\s+"
)
# A mutate verb in SUBCOMMAND position.
GIT_MUTATE_RE = re.compile(
    rf"\bgit\s+(?:{_GLOBALS})*"
    r"(merge|rebase|cherry-pick|revert|reset|am|clean|commit|checkout|restore|switch|push|pull|update-ref|symbolic-ref)\b",
    re.IGNORECASE,
)
# `git ... stash` (subcommand position) — bare = implicit push; exclude read-only list/show.
GIT_STASH_RE = re.compile(rf"\bgit\s+(?:{_GLOBALS})*stash\b(?!\s+(?:list|show)\b)", re.IGNORECASE)
# `git ... branch -f|-D|--delete --force|--force` — force-moves/deletes a ref.
GIT_BRANCH_FORCE_RE = re.compile(
    rf"\bgit\s+(?:{_GLOBALS})*branch\b[^\n;|&]*?(-D|-f|--force|--delete\s+--force)\b", re.IGNORECASE)
# `git ... fetch <...> SRC:DST` — a refspec with `:` can update a LOCAL ref.
GIT_FETCH_REFSPEC_RE = re.compile(rf"\bgit\s+(?:{_GLOBALS})*fetch\b[^\n;|&]*?\S+:\S+", re.IGNORECASE)
# `git ... apply` that WRITES (not --check / --stat / --numstat / --summary).
GIT_APPLY_RE = re.compile(rf"\bgit\s+(?:{_GLOBALS})*apply\b", re.IGNORECASE)
GIT_APPLY_READONLY_RE = re.compile(r"--(check|stat|numstat|summary)\b", re.IGNORECASE)
# wteng main <queue done|regen|park> — mutates main's engine/queue.json.
WTENG_MAIN_MUTATE_RE = re.compile(r"\bwteng\.ps1\s+main\s+queue\s+(done|regen|park)\b", re.IGNORECASE)
# non-git `patch -pN < ...` applying a reintegration diff to the cwd.
PATCH_RE = re.compile(r"\bpatch\b[^\n;|&]*?-p\d", re.IGNORECASE)

# Target-pin extractors (run on the noise-stripped command).
GIT_CHDIR_RE = re.compile(r"\bgit\s+(?:-C\s+|--git-dir[=\s]|--work-tree[=\s])(\"[^\"]+\"|'[^']+'|\S+)", re.IGNORECASE)
CD_RE = re.compile(r"(?:^|&&|\|\||[;&|(\n])\s*cd\s+(\"[^\"]+\"|'[^']+'|\S+)", re.IGNORECASE)
WTENG_TGT_RE = re.compile(r"\bwteng\.ps1\s+(\S+)", re.IGNORECASE)

BUILD_INPUT_RE = re.compile(
    r"(?:^|[\\/])(?:"
    r"src[\\/].*\.c|include[\\/].*\.h|.*\.h"
    r"|regfix(?:_stage2)?\.txt|asmfix\.txt|.*\.ld"
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


def norm(p: str) -> str:
    """Usable, canonical path for both FS ops and comparison. Mirrors
    worktree_contamination_guard's idiom (proven in production)."""
    return os.path.normcase(os.path.abspath(p.replace("/", os.sep)))


def main_repo_path(anchor: str) -> str | None:
    out = _git(anchor, "worktree", "list", "--porcelain")
    m = re.search(r"^worktree (.+)$", out, re.MULTILINE)
    return norm(m.group(1).strip()) if m else None


def resolve_root(p: str, cwd: str) -> str | None:
    if not p:
        return None
    p = p.strip("\"'")
    ap = p if os.path.isabs(p) else os.path.join(cwd, p)
    d = ap if os.path.isdir(ap) else (os.path.dirname(ap) or cwd)
    top = _git(d, "rev-parse", "--show-toplevel").strip()
    return norm(top) if top else None


def strip_noise(cmd: str) -> str:
    # Drop -m/-F/--message/--file quoted args and `#` comments so their TEXT
    # (a `bb2-work-` substring, a `merge` word) can't create a false match.
    s = re.sub(r"(?:-m|--message|-F|--file)\s*(\"[^\"]*\"|'[^']*'|\S+)", " ", cmd, flags=re.IGNORECASE)
    s = re.sub(r"#.*", " ", s)
    return s


def is_mutating(cmd: str) -> bool:
    if GIT_MUTATE_RE.search(cmd) or GIT_STASH_RE.search(cmd) or GIT_BRANCH_FORCE_RE.search(cmd):
        return True
    if GIT_FETCH_REFSPEC_RE.search(cmd):
        return True
    if GIT_APPLY_RE.search(cmd) and not GIT_APPLY_READONLY_RE.search(cmd):
        return True
    if WTENG_MAIN_MUTATE_RE.search(cmd) or PATCH_RE.search(cmd):
        return True
    return False


def targeted_repo(cmd: str, cwd: str, main_path: str) -> str:
    """Return 'main' | 'worktree' | 'unknown' — which repo the mutation hits."""
    pins = [m.group(1) for m in GIT_CHDIR_RE.finditer(cmd)]
    pins += [m.group(1) for m in CD_RE.finditer(cmd)]
    for p in (x.strip("\"'") for x in pins):
        if "bb2-work-" in p or "worktrees" in p.replace("\\", "/").split("/"):
            return "worktree"
    mw = WTENG_TGT_RE.search(cmd)
    if mw:
        return "main" if mw.group(1).lower() == "main" else "worktree"
    for p in pins:
        rp = resolve_root(p, cwd)
        if rp:
            return "main" if rp == main_path else "worktree"
    if pins:
        return "main"  # an explicit, unresolved, non-worktree target -> guard conservatively
    rc = resolve_root(cwd, cwd)
    if rc is None:
        return "unknown"
    return "main" if rc == main_path else "worktree"


def workers_live(root: str) -> bool:
    return bool(re.search(r"^worktree .*[\\/]bb2-work-",
                          _git(root, "worktree", "list", "--porcelain"), re.MULTILINE))


def read_lock(root: str) -> dict | None:
    try:
        with open(os.path.join(root, LOCK_REL), "r", encoding="utf-8") as fh:
            return json.load(fh)
    except Exception:
        return None


def block(sid: str, lock: dict | None, what: str) -> int:
    if lock is None:
        msg = [
            f"BLOCKED by main_reintegration_lock.py — {what} on MAIN while a batch is active,",
            "but you do NOT hold the main-reintegration lock.",
            "  Acquire it first (only ONE orchestrator may mutate main at a time):",
            "    & tools/reintegrate_lock.ps1 acquire     (release with: … release)",
            "Why: the 2026-06-14 orch3/orch0614b collision — a concurrent `git merge`/`reset`",
            "on main wiped another session's uncommitted reintegration.",
        ]
    else:
        try:
            age = f" (held {int((time.time() - float(lock.get('ts', 0))) / 60)} min)"
            stale = (time.time() - float(lock.get("ts", 0))) > STALE_SECS
        except Exception:
            age, stale = "", False
        msg = [
            f"BLOCKED by main_reintegration_lock.py — {what} on MAIN, but the",
            f"main-reintegration lock is held by ANOTHER session{age}:",
            f"    holder session_id = {lock.get('session', '?')}",
            "Do NOT mutate main concurrently — wait for release (& tools/reintegrate_lock.ps1 status).",
            "This is the exact collision the lock prevents.",
        ]
        if stale:
            msg += [f"  The lock looks STALE (> {STALE_SECS // 60} min). If that session is dead, reclaim:",
                    "    & tools/reintegrate_lock.ps1 steal -Reason '<why>'"]
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
        main_path = main_repo_path(cwd)
        if not main_path:
            return 0

        what = None
        if tool in ("Bash", "PowerShell"):
            cmd = strip_noise(ti.get("command", "") or "")
            if is_mutating(cmd) and targeted_repo(cmd, cwd, main_path) == "main":
                what = "a git/queue working-tree mutation"
        elif tool in ("Edit", "Write", "MultiEdit"):
            fp = ti.get("file_path", "") or ""
            if fp and BUILD_INPUT_RE.search(norm(fp).replace("\\", "/")):
                if resolve_root(fp, cwd) == main_path:
                    what = f"an edit of a build input ({os.path.basename(fp)})"
        if not what:
            return 0

        lock = read_lock(main_path)
        if not workers_live(main_path) and lock is None:
            return 0  # solo, uncontended
        if lock is not None and sid and lock.get("session") == sid:
            return 0  # held by me
        return block(sid, lock, what)
    except Exception as e:
        print(f"main_reintegration_lock: fail-open (error: {e})", file=sys.stderr)
        return 0


if __name__ == "__main__":
    sys.exit(main())
