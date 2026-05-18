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

# --- Auto-heal main's .venv if broken ------------------------------------
# Before linking, check that main's .venv exists AND has all packages from
# requirements.txt. If not, invoke `dc.sh setup-venv` IN MAIN to create/
# repair it. This eliminates the "agent hits ModuleNotFoundError 30s into
# a permuter run" failure mode that recurred across multiple sessions.
#
# Idempotent — when main's venv is healthy, this is one cheap python
# import-check per requirements package. When broken, the heal step is
# a one-time ~30s pip install (cached afterward).
REQ_FILE="$MAIN_REPO/requirements.txt"
if [ -f "$REQ_FILE" ]; then
    VENV_PY="$MAIN_REPO/.venv/bin/python3"
    NEED_HEAL=0
    if [ ! -x "$VENV_PY" ]; then
        NEED_HEAL=1
    else
        # Check only critical modules (must match setup-venv's
        # VALIDATE_IMPORTS list). Iterating requirements.txt would drag
        # in splat's broken N64 import chain.
        _miss=""
        for mod in toml; do
            if ! "$VENV_PY" -c "import $mod" 2>/dev/null; then
                _miss="$_miss $mod"
            fi
        done
        if [ -n "$_miss" ]; then
            NEED_HEAL=1
            echo "Worktree: main .venv missing critical module(s):$_miss — auto-healing" >&2
        fi
    fi
    if [ "$NEED_HEAL" = "1" ]; then
        if (cd "$MAIN_REPO" && bash tools/dc.sh setup-venv) >&2; then
            # Force re-link of THIS worktree's .venv below: any prior
            # cp -as tree predates the new packages and is now stale.
            rm -rf .venv 2>/dev/null
        else
            echo "Worktree: main .venv auto-heal FAILED — see error above" >&2
            echo "  manual fix: cd $MAIN_REPO && bash tools/dc.sh setup-venv" >&2
            # Don't bail — the other deps might still link OK and a
            # partial worktree is better than zero.
        fi
    fi
fi

# Trees: check for a specific marker file inside (not just the parent dir)
# so partial cp -as failures self-heal — if the marker is missing, blow
# away the tree and re-link. Format: "<dep>:<marker_relative_to_dep>"
TREE_DEPS="tools/gcc-2.7.2:build/cc1 tools/decomp-permuter:permuter.py .venv:bin/python3 disc:SLUS_006.63"
# Single files: just check the file itself.
FILE_DEPS="tools/cc1psx.exe"

LINKED=""
FAILED=""
EMPTY_IN_MAIN=""

for entry in $TREE_DEPS; do
    dep="${entry%%:*}"
    marker="${entry#*:}"
    # Skip if marker already present locally.
    [ -e "$dep/$marker" ] && continue
    # If main repo doesn't have the marker either, we can't help. But
    # distinguish two sub-cases for visibility:
    #   - Main is missing the dep entirely → silent skip (might be optional)
    #   - Main HAS the directory but it's empty/broken (marker missing) →
    #     loud warning; this is the "another agent will hit this in 90s"
    #     failure mode that the silent skip used to mask.
    if [ ! -e "$MAIN_REPO/$dep/$marker" ]; then
        if [ -d "$MAIN_REPO/$dep" ]; then
            EMPTY_IN_MAIN="$EMPTY_IN_MAIN $dep"
        fi
        continue
    fi
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

if [ -n "$EMPTY_IN_MAIN" ]; then
    echo "Worktree: main repo has empty dir(s) (clone missing):$EMPTY_IN_MAIN" >&2
    echo "  Will silently skip linking until main is fixed. To install:" >&2
    for dep in $EMPTY_IN_MAIN; do
        case "$dep" in
            tools/decomp-permuter)
                echo "    cd \"$MAIN_REPO\" && rmdir tools/decomp-permuter && git clone https://github.com/simonlindholm/decomp-permuter.git tools/decomp-permuter" >&2
                ;;
            tools/gcc-2.7.2)
                echo "    cd \"$MAIN_REPO\" && bash tools/setup_wsl.sh" >&2
                ;;
            .venv)
                echo "    cd \"$MAIN_REPO\" && python3 -m venv .venv && .venv/bin/pip install -r requirements.txt" >&2
                ;;
            disc)
                echo "    cd \"$MAIN_REPO\" && python3 tools/extract_iso.py <path-to-bb2.cue>" >&2
                ;;
            *)
                echo "    (unknown — check $MAIN_REPO/$dep)" >&2
                ;;
        esac
    done
fi

if [ -n "$FAILED" ]; then
    echo "Worktree: failed to link from main repo:$FAILED" >&2
    echo "  main repo: $MAIN_REPO" >&2
    echo "  fix: cd $MAIN_REPO && bash tools/setup_wsl.sh" >&2
    exit 1
fi

exit 0
