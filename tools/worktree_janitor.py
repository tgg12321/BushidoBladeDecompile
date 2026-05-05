#!/usr/bin/env python3
"""worktree_janitor.py -- safely prune fully-matched worker worktrees.

SAFETY RULES (per user 2026-05-04):
  Worktrees are deleted ONLY when:
    1. The branch has 0 commits ahead of main (worker did nothing), OR
    2. ALL of the branch's commits ahead of main are reachable from main
       (i.e., already integrated via merge).

  Worktrees with unique commits NOT in main (preserved-stuck, WIP)
  are NEVER deleted. Even with --force.

Usage:
  python3 tools/worktree_janitor.py            # dry-run; print decisions
  python3 tools/worktree_janitor.py --apply    # actually remove safe ones

Output:
  KEEP   <wt>  reason  (preserved/active/etc.)
  PRUNE  <wt>  reason  (no commits / fully merged)
"""
from __future__ import annotations

import subprocess
import sys
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run(cmd: list[str]) -> tuple[int, str]:
    p = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, errors="replace")
    return p.returncode, (p.stdout + p.stderr).strip()


def list_worktrees() -> list[dict]:
    """Parse `git worktree list --porcelain` into structured records."""
    rc, out = run(["git", "worktree", "list", "--porcelain"])
    if rc != 0:
        return []
    records = []
    cur = {}
    for line in out.splitlines():
        if not line:
            if cur:
                records.append(cur)
                cur = {}
            continue
        if line.startswith("worktree "):
            cur["path"] = line[len("worktree "):]
        elif line.startswith("HEAD "):
            cur["head"] = line[len("HEAD "):]
        elif line.startswith("branch "):
            cur["branch"] = line[len("branch "):].replace("refs/heads/", "")
        elif line == "locked":
            cur["locked"] = True
    if cur:
        records.append(cur)
    return records


def commits_ahead(branch: str) -> int:
    rc, out = run(["git", "rev-list", "--count", f"main..{branch}"])
    if rc != 0:
        return -1
    try:
        return int(out)
    except ValueError:
        return -1


def is_branch_merged(branch: str) -> bool:
    """Return True if branch's HEAD is reachable from main (already merged)."""
    rc, _ = run(["git", "merge-base", "--is-ancestor", branch, "main"])
    return rc == 0


def main() -> int:
    apply = "--apply" in sys.argv

    worktrees = list_worktrees()
    if not worktrees:
        print("No worktrees found")
        return 0

    main_path = None
    for wt in worktrees:
        if wt.get("branch") == "main":
            main_path = wt.get("path")
            break

    decisions = []
    # Determine if a worktree's log shows recent activity (helps distinguish
    # actively running vs. lock-orphaned but otherwise dead).
    import time
    now = time.time()

    def recent_log_activity(branch_short: str) -> bool:
        """Check if there's a tmp/parallel_logs/ entry mtime within last 5 min."""
        wid = branch_short.replace("worktree-agent-", "")[:8]
        log_dir = ROOT / "tmp" / "parallel_logs"
        if not log_dir.is_dir():
            return False
        for log in log_dir.glob("*.log"):
            try:
                text = log.read_text(encoding="utf-8", errors="replace")
            except Exception:
                continue
            if wid in text and (now - log.stat().st_mtime) < 300:
                return True
        return False

    for wt in worktrees:
        path = wt.get("path", "")
        branch = wt.get("branch", "")
        locked = wt.get("locked", False)

        if path == main_path:
            decisions.append(("KEEP", path, "main worktree -- never delete"))
            continue
        if not branch:
            decisions.append(("KEEP", path, "no branch info -- conservative skip"))
            continue
        if not branch.startswith("worktree-agent-"):
            # Not one of ours; leave alone.
            decisions.append(("KEEP", path, f"non-agent branch ({branch})"))
            continue

        # SAFETY: locked + recent log activity = ACTIVE worker, never delete.
        if locked and recent_log_activity(branch):
            decisions.append(("KEEP", path, "ACTIVE: locked + log activity within 5 min"))
            continue

        ahead = commits_ahead(branch)
        if ahead == -1:
            decisions.append(("KEEP", path, "couldn't count commits -- conservative skip"))
            continue
        if ahead == 0:
            # Worker either did nothing or was already merged via cherry-pick.
            # Either way, no unique work to lose. (Stale lock without recent
            # activity = dead worker, safe to remove.)
            reason = "0 commits ahead of main"
            if locked:
                reason += " (stale lock; no recent log activity)"
            decisions.append(("PRUNE", path, reason))
            continue

        # ahead > 0: check if branch is fully merged
        if is_branch_merged(branch):
            decisions.append(("PRUNE", path, f"fully merged ({ahead} commits all in main)"))
        else:
            decisions.append(("KEEP", path, f"PRESERVED: {ahead} unique commits NOT in main"))

    print("=== Worktree janitor ===")
    print(f"  ({len(decisions)} worktrees scanned)")
    print()
    n_prune = sum(1 for d in decisions if d[0] == "PRUNE")
    n_keep = sum(1 for d in decisions if d[0] == "KEEP")
    for action, path, reason in decisions:
        wt_short = path.split("/")[-1] if path else "?"
        print(f"  {action:<6} {wt_short:40s} {reason}")
    print()
    print(f"Summary: {n_prune} prune, {n_keep} keep")

    if not apply:
        print()
        print("Dry run. Re-run with --apply to remove the PRUNE entries.")
        return 0

    if n_prune == 0:
        return 0

    print()
    print("Applying (force-removing pruned worktrees + branches)...")
    removed = 0
    for action, path, _ in decisions:
        if action != "PRUNE":
            continue
        # Try to remove the worktree
        rc, out = run(["git", "worktree", "remove", path, "-f", "-f"])
        # Determine the branch name
        branch_short = path.split("/")[-1].replace("agent-", "worktree-agent-")
        # Try to delete the branch (it may already be gone via worktree remove)
        run(["git", "branch", "-D", branch_short])
        if rc == 0:
            print(f"  REMOVED {path}")
            removed += 1
        else:
            print(f"  SKIP    {path}: {out}")
    # Final prune of stale worktree registry entries
    run(["git", "worktree", "prune"])
    print()
    print(f"Removed {removed} worktrees. PRESERVED branches and active worktrees untouched.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
