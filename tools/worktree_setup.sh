#!/bin/bash
# tools/worktree_setup.sh
#
# Run this at the start of any worktree agent session to symlink
# gitignored tools and data from the main repo. Without this,
# builds will silently succeed against missing binaries and produce
# false SHA1 matches.
#
# Usage (from within a worktree):
#   bash tools/worktree_setup.sh

set -e

# Get the current worktree's WSL path
CURRENT=$(git rev-parse --show-toplevel)

# Get the main (primary) worktree path from git.
# 'git worktree list --porcelain' outputs "worktree <path>" as the first line.
# Use sed to grab the full path (handles spaces), then convert Windows path to WSL if needed.
MAIN_REPO_RAW=$(git worktree list --porcelain | sed -n '1s/^worktree //p')

# Convert Windows path (C:/...) to WSL path (/mnt/c/...) if needed
if [[ "$MAIN_REPO_RAW" == [A-Za-z]:/* ]]; then
    DRIVE=$(echo "$MAIN_REPO_RAW" | cut -c1 | tr '[:upper:]' '[:lower:]')
    REST=$(echo "$MAIN_REPO_RAW" | cut -c3-)
    MAIN_REPO="/mnt/${DRIVE}${REST}"
else
    MAIN_REPO="$MAIN_REPO_RAW"
fi

if [ -z "$MAIN_REPO" ]; then
    echo "ERROR: could not determine main repo path" >&2
    exit 1
fi

# If we're already in the main worktree, nothing to do
if [ "$CURRENT" = "$MAIN_REPO" ]; then
    echo "Already in the main worktree — setup not needed."
    exit 0
fi

echo "Main repo:     $MAIN_REPO"
echo "This worktree: $CURRENT"
echo ""

# === FRESHNESS CHECK (MANDATORY) ===
# A worktree's base must be reachable from main's current HEAD. If main has
# moved on since this worktree was created, the agent will produce work
# against an outdated base — GCC 2.7.2's codegen is sensitive to surrounding
# declarations, so context drift causes silent merge failures.
MAIN_HEAD=$(git -C "$MAIN_REPO" rev-parse main 2>/dev/null)
WORKTREE_BASE=$(git rev-parse HEAD 2>/dev/null)

if [ -z "$MAIN_HEAD" ] || [ -z "$WORKTREE_BASE" ]; then
    echo "ERROR: could not determine HEAD commits" >&2
    exit 1
fi

if [ "$MAIN_HEAD" != "$WORKTREE_BASE" ]; then
    # Worktree is not exactly at main HEAD — check whether it is an ancestor
    if git -C "$MAIN_REPO" merge-base --is-ancestor "$WORKTREE_BASE" "$MAIN_HEAD" 2>/dev/null; then
        # WORKTREE_BASE is reachable from MAIN_HEAD — count how far behind
        BEHIND=$(git -C "$MAIN_REPO" rev-list --count "${WORKTREE_BASE}..${MAIN_HEAD}" 2>/dev/null)
        echo "ERROR: WORKTREE IS STALE — $BEHIND commits behind main HEAD" >&2
        echo "  worktree base:  $WORKTREE_BASE" >&2
        echo "  main HEAD:      $MAIN_HEAD" >&2
        echo "" >&2
        echo "GCC 2.7.2 codegen is sensitive to surrounding declarations." >&2
        echo "Working from a stale base risks silent merge failure." >&2
        echo "" >&2
        echo "STOP and report this error to the orchestrator." >&2
        echo "DO NOT try to git pull, fetch, or rebase — the orchestrator" >&2
        echo "must spawn a fresh worktree." >&2
        exit 2
    else
        # Disjoint history — completely wrong branch
        echo "ERROR: worktree base is not reachable from main HEAD" >&2
        echo "  worktree base:  $WORKTREE_BASE" >&2
        echo "  main HEAD:      $MAIN_HEAD" >&2
        echo "STOP and report to the orchestrator." >&2
        exit 2
    fi
fi
echo "Freshness OK: worktree at main HEAD ($MAIN_HEAD)"
echo ""

# Items to symlink (gitignored, needed for builds)
ITEMS=(
    "tools/gcc-2.7.2"
    "tools/maspsx"
    "tools/decomp-permuter"
    "tools/m2c"
    ".venv"
    "disc"
)

for item in "${ITEMS[@]}"; do
    src="$MAIN_REPO/$item"
    dst="$CURRENT/$item"
    # If dst is a symlink already, skip
    if [ -L "$dst" ]; then
        echo "  skip (linked):  $item"
    # If dst is an empty directory (git placeholder for nested repo), replace it
    elif [ -d "$dst" ] && [ -z "$(ls -A "$dst" 2>/dev/null)" ]; then
        rmdir "$dst"
        ln -sf "$src" "$dst"
        echo "  linked (replaced empty dir): $item -> $src"
    elif [ -e "$dst" ]; then
        echo "  skip (exists):  $item"
    elif [ -e "$src" ]; then
        ln -sf "$src" "$dst"
        echo "  linked:         $item -> $src"
    else
        echo "  WARNING: not found in main repo: $src"
    fi
done

echo ""
echo "Setup complete. Now run: source .venv/bin/activate"
