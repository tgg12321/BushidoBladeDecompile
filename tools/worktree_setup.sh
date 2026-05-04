#!/usr/bin/env bash
# tools/worktree_setup.sh — make a git worktree buildable.
#
# Worktrees inherit the tracked source tree but NOT gitignored
# directories. The build pipeline needs:
#   tools/gcc-2.7.2/        (compiler — gitignored, large)
#   tools/decomp-permuter/  (matching tool — gitignored, separate clone)
#   tools/m2c/              (decompiler — gitignored, separate clone)
#   .venv/                  (Python venv — gitignored)
#   disc/                   (extracted ISO — gitignored, large)
#
# Without these, the maspsx step fails silently, mipsel-as receives
# nothing, and every .o is a 788-byte empty ELF stub. Builds appear
# to succeed but every object is empty — guaranteed mismatch.
#
# This script symlinks them in from the main worktree. Idempotent:
# correct symlinks are left alone, empty placeholders are replaced,
# non-empty user dirs are skipped (don't trample work).
#
# Run as the FIRST action in any worker subagent's worktree.

set -euo pipefail

# Auto-relaunch under WSL if invoked from a Windows shell (git bash /
# MSYS / Cygwin). Those shells fake `ln -s` as empty dirs or shortcuts,
# unusable by the WSL build pipeline. Re-exec inside WSL so symlinks
# are real Unix symlinks.
to_wsl_path() {
    case "$1" in
        /[a-zA-Z]/*)
            # Already bash form: /c/foo -> /mnt/c/foo
            local drive
            drive=$(printf '%s' "$1" | cut -c2 | tr '[:upper:]' '[:lower:]')
            printf '/mnt/%s%s\n' "$drive" "${1:2}"
            ;;
        [a-zA-Z]:[/\\]*)
            # Windows form: C:/foo or C:\foo -> /mnt/c/foo
            local drive
            drive=$(printf '%s' "$1" | cut -c1 | tr '[:upper:]' '[:lower:]')
            local rest="${1:2}"
            rest="${rest//\\//}"
            printf '/mnt/%s%s\n' "$drive" "$rest"
            ;;
        *) printf '%s\n' "$1" ;;
    esac
}
case "${OSTYPE:-}" in
    msys*|cygwin*|win32*)
        if [ -z "${WORKTREE_SETUP_IN_WSL:-}" ]; then
            WT_WSL=$(to_wsl_path "$(pwd)")
            SCRIPT_WSL=$(to_wsl_path "$(realpath "$0")")
            export WORKTREE_SETUP_IN_WSL=1
            exec wsl bash -c "cd '$WT_WSL' && WORKTREE_SETUP_IN_WSL=1 bash '$SCRIPT_WSL'"
        fi
        ;;
esac

# Fix the .git file's gitdir reference if it points to a Windows-form path
# (C:/foo or C:\foo). When `git worktree add` runs from git-bash on Windows
# it writes the gitdir in Windows form, which WSL git can't follow. Rewrite
# to a relative path — works from both git-bash AND WSL.
if [ -f .git ] && [ ! -d .git ]; then
    GITDIR_LINE=$(head -1 .git)
    GITDIR_PATH="${GITDIR_LINE#gitdir: }"
    if [ "$GITDIR_PATH" != "$GITDIR_LINE" ]; then
        case "$GITDIR_PATH" in
            [a-zA-Z]:[/\\]*)
                # Convert Windows path to a real filesystem path WSL understands,
                # then to a path relative to the worktree (this .git file's dir).
                GITDIR_WSL=$(to_wsl_path "$GITDIR_PATH")
                if [ -d "$GITDIR_WSL" ]; then
                    RELATIVE=$(python3 -c "import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))" "$GITDIR_WSL" "$(pwd)")
                    echo "[worktree-setup] FIX:  .git gitdir (Windows -> relative: $RELATIVE)"
                    printf 'gitdir: %s\n' "$RELATIVE" > .git
                else
                    echo "[worktree-setup] WARN: .git gitdir target $GITDIR_WSL not found, leaving alone" >&2
                fi
                ;;
        esac
    fi
fi

# Discover main repo root from git common dir. From a worktree,
# `git rev-parse --git-common-dir` returns the main repo's .git/, so
# its parent is the main worktree root.
MAIN_GITDIR=$(git rev-parse --git-common-dir 2>/dev/null || echo "")
if [ -z "$MAIN_GITDIR" ]; then
    echo "[worktree-setup] ERROR: not in a git repo" >&2
    exit 1
fi

# Resolve to absolute and normalize through cd/pwd so both paths come out
# in the same shell form. On git-bash for Windows, the raw outputs can be
# /c/foo (from --git-common-dir) vs C:/foo (from --show-toplevel) for the
# same physical location — string-comparing them then misidentifies main
# as a worktree.
MAIN_GITDIR=$(cd "$MAIN_GITDIR" && pwd)
MAIN_ROOT=$(cd "$(dirname "$MAIN_GITDIR")" && pwd)
WT_ROOT=$(cd "$(git rev-parse --show-toplevel)" && pwd)

if [ "$MAIN_ROOT" = "$WT_ROOT" ]; then
    echo "[worktree-setup] running in main repo at $MAIN_ROOT — nothing to do"
    exit 0
fi

echo "[worktree-setup] main:     $MAIN_ROOT"
echo "[worktree-setup] worktree: $WT_ROOT"

cd "$WT_ROOT"

# relative-path-in-worktree -> absolute-source-in-main
LINKS=(
    "tools/gcc-2.7.2"
    "tools/decomp-permuter"
    "tools/m2c"
    ".venv"
    "disc"
)

for rel in "${LINKS[@]}"; do
    target="$MAIN_ROOT/$rel"
    if [ ! -e "$target" ]; then
        echo "[worktree-setup] WARN: source $target missing in main, skipping"
        continue
    fi

    if [ -L "$rel" ]; then
        cur=$(readlink "$rel")
        if [ "$cur" = "$target" ]; then
            echo "[worktree-setup] OK:   $rel"
            continue
        fi
        echo "[worktree-setup] FIX:  $rel (relink: $cur -> $target)"
        rm "$rel"
    elif [ -d "$rel" ]; then
        # Empty placeholder = git worktree's empty dir for a gitignored path.
        # Non-empty = something the worker (or splat) created — leave alone.
        if [ -z "$(ls -A "$rel" 2>/dev/null)" ]; then
            echo "[worktree-setup] FIX:  $rel (replace empty placeholder)"
            rmdir "$rel"
        else
            echo "[worktree-setup] SKIP: $rel (non-empty directory exists)"
            continue
        fi
    elif [ -e "$rel" ]; then
        echo "[worktree-setup] SKIP: $rel (file exists, not symlink/dir)"
        continue
    fi

    mkdir -p "$(dirname "$rel")"
    ln -s "$target" "$rel"
    echo "[worktree-setup] LINK: $rel -> $target"
done

echo "[worktree-setup] done"
