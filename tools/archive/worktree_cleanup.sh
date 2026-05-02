#!/bin/bash
# tools/worktree_cleanup.sh
#
# Remove an agent worktree (and optionally its branch) safely.
#
# Must be run via WSL, from the main repo, because worktree_setup.sh
# normalizes the worktree's .git file to the /mnt/c/... format for agent use
# and Git-for-Windows cannot parse that format during `git worktree remove`.
#
# Usage (from the Windows side, main repo):
#   wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && \
#                bash tools/worktree_cleanup.sh <worktree-name-or-path>'
#
# Accepts either a bare worktree name (agent-XXXX) or a full path.
# If the corresponding branch (worktree-<name>) is fully merged into main
# or empty, it is deleted. If not merged, a warning is printed and the
# branch is left in place.

set -e

# Guard: this script MUST run from WSL, never from Git Bash.
# Git-for-Windows cannot parse WSL-format gitdir pointers (`/mnt/c/...`) and
# its `git worktree prune` — which this script calls on the happy path AND
# the manual-fallback path — treats every WSL-format worktree as "non-existent
# location" and silently deletes the reverse-pointer metadata for ALL
# currently-active agent worktrees. This has wiped concurrent agents mid-run.
# See memory/feedback_worktree_path_format.md.
if [ ! -d /mnt/c ]; then
    echo "ERROR: worktree_cleanup.sh must run from WSL, not Git Bash." >&2
    echo "  Git-for-Windows prune will destroy all WSL-format worktrees." >&2
    echo "  Use: wsl bash -c 'cd /mnt/c/.../main-repo && bash tools/worktree_cleanup.sh <name>'" >&2
    exit 3
fi

if [ $# -lt 1 ]; then
    echo "usage: $0 <worktree-name-or-path>" >&2
    exit 1
fi

TARGET="$1"

# Resolve the worktree path
if [ -d "$TARGET" ]; then
    WT_PATH="$TARGET"
elif [ -d ".claude/worktrees/$TARGET" ]; then
    WT_PATH=".claude/worktrees/$TARGET"
else
    echo "ERROR: worktree not found: $TARGET (checked '$TARGET' and '.claude/worktrees/$TARGET')" >&2
    exit 1
fi

WT_NAME=$(basename "$WT_PATH")
BRANCH="worktree-${WT_NAME}"

echo "Worktree: $WT_PATH"
echo "Branch:   $BRANCH"

# Try git worktree remove first; if it refuses (e.g. .git file mangled or
# path format mismatch), fall back to manual filesystem + metadata cleanup.
if git worktree remove --force "$WT_PATH" 2>/dev/null; then
    echo "removed via git worktree remove"
else
    echo "git worktree remove failed — falling back to manual cleanup"
    rm -rf "$WT_PATH"
    rm -rf ".git/worktrees/$WT_NAME"
fi

git worktree prune -v

# Delete the branch if it exists. Use -D (force) only when the branch is
# fully merged into main or has no commits ahead; otherwise warn and leave it.
if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
    AHEAD=$(git rev-list --count "main..$BRANCH" 2>/dev/null || echo "?")
    if [ "$AHEAD" = "0" ]; then
        git branch -D "$BRANCH"
        echo "deleted branch $BRANCH (no commits ahead of main)"
    elif git merge-base --is-ancestor "$BRANCH" main 2>/dev/null; then
        git branch -D "$BRANCH"
        echo "deleted branch $BRANCH (fully merged into main)"
    else
        echo "WARNING: branch $BRANCH has $AHEAD unmerged commits — left in place" >&2
        echo "         merge it first, or delete manually with: git branch -D $BRANCH" >&2
    fi
fi

echo "done."
