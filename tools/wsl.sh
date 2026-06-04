#!/bin/bash
# tools/wsl.sh
#
# Run a command in WSL with cwd set to the current worktree root.
# Cuts `wsl bash -c 'cd <path> && <cmd>'` boilerplate down to
# `bash tools/wsl.sh '<cmd>'`. Auto-detects the worktree root via
# `git rev-parse --show-toplevel`, so this works from the main repo
# OR any git worktree without manual configuration.
#
# Also auto-bootstraps the worktree (symlinks gitignored deps from
# main repo) the first time it's invoked in a freshly-created worktree
# that's missing the cc1 binary. Silent when no work needed.
#
# Usage:
#   bash tools/wsl.sh 'make 2>&1 | tail -5'
#   bash tools/wsl.sh 'source .venv/bin/activate && python3 -m splat split splat.yaml'
#   bash tools/wsl.sh 'bash tools/dc.sh verify --all'
#
# The command is passed verbatim to WSL bash -c, after `cd`-ing into the
# project directory. Quote your command with single quotes to prevent Git
# Bash from expanding $N (register names), $(), etc. before WSL sees them.

set -u

if [ "$#" -eq 0 ]; then
    cat >&2 <<EOF
usage: bash tools/wsl.sh '<command>'

Runs <command> inside WSL with cwd set to the current worktree root.
Auto-bootstraps the worktree if gitignored deps (cc1, .venv, etc.) are missing.

Examples:
  bash tools/wsl.sh 'make 2>&1 | tail -5'
  bash tools/wsl.sh 'bash tools/dc.sh verify --all'
  bash tools/wsl.sh 'python3 tools/regen_memory_index.py'
EOF
    exit 2
fi

# Detect project root via git. Works from Git Bash (returns C:/foo paths)
# and from inside WSL (returns /mnt/c/foo paths). We need a WSL-form path
# for the `cd` inside the wsl call.
PROJECT_DIR=$(git rev-parse --show-toplevel 2>/dev/null)
if [ -z "$PROJECT_DIR" ]; then
    echo "wsl.sh: not in a git repository (couldn't auto-detect project root)" >&2
    exit 1
fi

# Normalize: Git Bash returns C:/Users/... ; convert to WSL /mnt/c/...
case "$PROJECT_DIR" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_DIR" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_DIR#?:}"
        REST="${REST//\\//}"
        WSL_DIR="/mnt/$DRIVE$REST"
        ;;
    /[a-zA-Z]/*)
        # Already WSL-bash-style /c/... — prepend /mnt
        WSL_DIR="/mnt$PROJECT_DIR"
        ;;
    *)
        # Already a Linux/WSL path or unknown form — pass through.
        WSL_DIR="$PROJECT_DIR"
        ;;
esac

# Worktree auto-bootstrap removed 2026-05-26: worktrees are no longer part of
# the workflow (decomp runs directly on main, where the gitignored deps are
# real files, not symlinks). worktree_bootstrap.sh was retired to archive/.

# Auto-repair: if .venv exists but its python3 symlink is missing, restore it.
# This catches a venv-corruption mode where pyvenv-script shebangs (pip, etc.)
# fail with `.venv/bin/python3: not found` while explicit `python3 ...` still
# works via PATH fallback. The repair lives in tools/venv_repair.sh -- a script
# file rather than an inline multi-line string here, because inline multi-line
# bash variables through `wsl bash -c` had subtle expansion issues (newlines /
# variable scope) that silently dropped the repair. See
# tools/hooks/tooling_error_signatures.json id `venv-python-symlink-missing`
# (codified 2026-06-04).
wsl bash -c "
set -e
cd \"$WSL_DIR\"
bash tools/venv_repair.sh || true
$*
"
