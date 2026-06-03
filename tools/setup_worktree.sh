#!/usr/bin/env bash
# tools/setup_worktree.sh — bootstrap a fresh git worktree for engine work.
#
# `git worktree add` brings tracked files only. The heavy artifacts the engine
# depends on (the GCC 2.7.2 build, the .venv, the pristine build/ cache, the
# permuter clone, the maspsx clone, the disc directory) are all gitignored, so
# a fresh worktree has none of them. Without those, `engine sandbox` /
# `engine verify-oracle` / cheat-asm strip / permuter runs all error out at
# step one.
#
# This script symlinks them from the main repository so a worker can spin up
# in a clean worktree and have full engine capability without any per-worktree
# bootstrap cost beyond the symlinks (~ms).
#
# Run from INSIDE the worktree (typically as the first step of any Workflow
# worker that uses `isolation: 'worktree'`).
#
# Why symlinks (not copies):
#   - .venv / tools/gcc-2.7.2 / tools/decomp-permuter / tools/maspsx / disc are
#     READ-ONLY for engine use (no worker mutates them) — symlinking is safe.
#   - build/ is the canonical oracle reference; symlinking ensures the worker
#     sees the same byte-identical reference the main repo verified.
#   - memory/wip/ symlinking lets workers READ the WIP entries directly; they
#     should not WRITE here (the synthesizer in main tree handles all WIP
#     updates from worker structured output).
#   - tools/cc1psx.exe is the diagnostic-only PsyQ binary (cc1psx-calibration).
#
# Idempotent: re-running is safe (existing symlinks/files are left alone).
#
# Safety: refuses to run in the main repository (sentinel: .git is a directory,
# not a file). A worktree's .git is a FILE pointing back to main's gitdir.

set -euo pipefail

WORKTREE_ROOT="$(git rev-parse --show-toplevel)"
cd "$WORKTREE_ROOT"

# Refuse to run in the main repo. Main has .git/ as a directory; worktrees have
# .git as a file. Mutating main via symlinks here would be catastrophic.
if [ -d "$WORKTREE_ROOT/.git" ]; then
    echo "setup_worktree.sh: refusing to run in main repo ($WORKTREE_ROOT/.git is a directory)"
    echo "  This script is for fresh worktrees only."
    exit 0
fi

# Discover the main repo path. `git worktree list --porcelain` lists all
# worktrees; the FIRST one is always the main repo. (POSIX awk is fine.)
MAIN_REPO="$(git worktree list --porcelain | awk '/^worktree /{print $2; exit}')"

if [ -z "${MAIN_REPO:-}" ] || [ "$MAIN_REPO" = "$WORKTREE_ROOT" ]; then
    echo "setup_worktree.sh: could not identify main repo (or already there). Bailing."
    exit 0
fi

if [ ! -d "$MAIN_REPO/.git" ]; then
    echo "setup_worktree.sh: main repo $MAIN_REPO is not a git checkout. Bailing."
    exit 1
fi

echo "setup_worktree.sh: bootstrapping $WORKTREE_ROOT from main $MAIN_REPO"

linked=0
skipped_exists=0
skipped_missing=0

# Heavy paths to symlink. Order: deps -> per-build cache -> diagnostic data.
# All are gitignored in main; worktree won't have them after `git worktree add`.
PATHS=(
    ".venv"
    "build"
    "tools/gcc-2.7.2"
    "tools/decomp-permuter"
    "tools/maspsx"
    "tools/cc1psx.exe"
    "disc"
)

for rel in "${PATHS[@]}"; do
    src="$MAIN_REPO/$rel"
    dst="$WORKTREE_ROOT/$rel"

    if [ ! -e "$src" ] && [ ! -L "$src" ]; then
        echo "  - skip $rel (not present in main)"
        skipped_missing=$((skipped_missing + 1))
        continue
    fi

    if [ -e "$dst" ] || [ -L "$dst" ]; then
        echo "  - skip $rel (already present in worktree)"
        skipped_exists=$((skipped_exists + 1))
        continue
    fi

    # Make sure the parent dir exists. (For e.g. tools/gcc-2.7.2 the
    # tools/ dir already does, but be defensive.)
    mkdir -p "$(dirname "$dst")"
    ln -s "$src" "$dst"
    echo "  + link $rel -> $src"
    linked=$((linked + 1))
done

# tmp/ — never symlink. Each worktree gets its own scratch space for sandbox
# builds, permuter workspaces, instrumented cc1 dumps, etc. Confirm it exists.
mkdir -p "$WORKTREE_ROOT/tmp"

echo "setup_worktree.sh: done. linked=$linked already-present=$skipped_exists missing-in-main=$skipped_missing"

# Sanity check: can we import engine? (Doesn't fail the script — just a useful
# diagnostic in the log if a worker's first engine import would fail.)
if [ -f "$WORKTREE_ROOT/.venv/bin/python3" ]; then
    if (
        cd "$WORKTREE_ROOT"
        ./.venv/bin/python3 -c "import engine.cli; import engine.wip; import engine.queue; import engine.sandbox" 2>&1
    ); then
        echo "setup_worktree.sh: engine import OK"
    else
        echo "setup_worktree.sh: WARNING engine import failed — worker should investigate"
    fi
fi
