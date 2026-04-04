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

# Items to symlink (gitignored, needed for builds)
ITEMS=(
    "tools/gcc-2.7.2"
    "tools/decomp-permuter"
    "tools/m2c"
    ".venv"
    "disc"
)

for item in "${ITEMS[@]}"; do
    src="$MAIN_REPO/$item"
    dst="$CURRENT/$item"
    if [ -e "$dst" ] || [ -L "$dst" ]; then
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
