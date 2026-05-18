#!/bin/bash
# tools/worktree_bootstrap.sh
#
# Idempotently ensures the current git worktree has the gitignored binary
# deps (gcc-2.7.2 cc1, decomp-permuter, .venv, disc, cc1psx.exe) wired up
# from the main repo via cp -as symlink-trees. No-op when run in the
# main worktree or when deps are already present.
#
# Without this, the first build in a fresh worktree fails with
# "tools/gcc-2.7.2/build/cc1: not found" and the agent has to rediscover
# the symlink workflow every time.
#
# Called automatically by:
#   - tools/dc.sh start         (SessionStart hook entry point)
#   - tools/wsl.sh              (auto-fix when deps missing)
#   - tools/dc.sh bootstrap     (explicit invocation)
#
# Run from the worktree root. Prints one summary line per actioned dep
# (silent if nothing to do). Returns non-zero only if main has the dep
# but linking failed.
#
# History: extracted 2026-05-18 from inline logic in tools/dc.sh start.

set -u

# Only meaningful inside a git worktree — main repo has .git as a directory.
[ -f ".git" ] || exit 0

# Parse the gitdir line: `gitdir: <path>`
GITDIR_LINE=$(head -1 .git | sed 's|^gitdir: *||')

# Convert Windows-style C:/... to WSL /mnt/c/... when running under WSL.
# (The .git pointer file is written with Windows paths by `git worktree add`
# from Git Bash, which `git rev-parse --git-common-dir` can't resolve under
# WSL — see memory/workflow/parallel-harness-gotchas.md.)
if [ -d /mnt/c ] && [ "${GITDIR_LINE#?:/}" != "$GITDIR_LINE" ]; then
    DRIVE=$(echo "$GITDIR_LINE" | cut -c1 | tr 'A-Z' 'a-z')
    GITDIR_ABS="/mnt/$DRIVE${GITDIR_LINE#?:}"
else
    GITDIR_ABS="$GITDIR_LINE"
fi

# GITDIR_ABS is .../.git/worktrees/<name>; main repo is .git's parent
# (three dirname's up).
[ -d "$GITDIR_ABS" ] || exit 0
MAIN_REPO=$(dirname "$(dirname "$(dirname "$GITDIR_ABS")")")
[ -d "$MAIN_REPO" ] || exit 0
# Defensive: refuse to act if we somehow ARE the main repo.
[ "$MAIN_REPO" = "$(pwd)" ] && exit 0

# Trees: check for a specific marker file inside (not just the parent dir)
# so partial cp -as failures self-heal — if the marker is missing, blow
# away the tree and re-link. Format: "<dep>:<marker_relative_to_dep>"
TREE_DEPS="tools/gcc-2.7.2:build/cc1 tools/decomp-permuter:permuter.py .venv:bin/python3 disc:SLUS_006.63"
# Single files: just check the file itself.
FILE_DEPS="tools/cc1psx.exe"

LINKED=""
FAILED=""

for entry in $TREE_DEPS; do
    dep="${entry%%:*}"
    marker="${entry#*:}"
    # Skip if marker already present locally.
    [ -e "$dep/$marker" ] && continue
    # Skip if main repo doesn't have it either (nothing we can do).
    [ -e "$MAIN_REPO/$dep/$marker" ] || continue
    rm -rf "$dep" 2>/dev/null
    mkdir -p "$(dirname "$dep")"
    if cp -as "$MAIN_REPO/$dep" "$dep" 2>/dev/null && [ -e "$dep/$marker" ]; then
        LINKED="$LINKED $dep"
    else
        FAILED="$FAILED $dep"
    fi
done

for dep in $FILE_DEPS; do
    [ -e "$dep" ] && continue
    [ -e "$MAIN_REPO/$dep" ] || continue
    if cp "$MAIN_REPO/$dep" "$dep" 2>/dev/null; then
        LINKED="$LINKED $dep"
    else
        FAILED="$FAILED $dep"
    fi
done

if [ -n "$LINKED" ]; then
    echo "Worktree: auto-linked from main repo:$LINKED"
fi

if [ -n "$FAILED" ]; then
    echo "Worktree: failed to link from main repo:$FAILED" >&2
    echo "  main repo: $MAIN_REPO" >&2
    echo "  fix: cd $MAIN_REPO && bash tools/setup_wsl.sh" >&2
    exit 1
fi

exit 0
