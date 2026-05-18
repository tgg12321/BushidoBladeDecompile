#!/bin/bash
# tools/wsl.sh
#
# Run a command in WSL with the working directory set to the BB2 project root.
# Cuts the `wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && <cmd>'`
# boilerplate down to `bash tools/wsl.sh '<cmd>'`.
#
# Usage:
#   bash tools/wsl.sh 'make 2>&1 | tail -5'
#   bash tools/wsl.sh 'source .venv/bin/activate && python3 -m splat split splat.yaml'
#   bash tools/wsl.sh 'bash tools/dc.sh verify --all'
#
# The command is passed verbatim to WSL bash -c, after `cd`-ing into the project
# directory. Quote your command with single quotes to prevent Git Bash from
# expanding `$N` (register names), `$()`, etc. before WSL sees them.

set -u

PROJECT_DIR='/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'

if [ "$#" -eq 0 ]; then
    cat >&2 <<EOF
usage: bash tools/wsl.sh '<command>'

Runs <command> inside WSL with cwd = $PROJECT_DIR.

Examples:
  bash tools/wsl.sh 'make 2>&1 | tail -5'
  bash tools/wsl.sh 'bash tools/dc.sh verify --all'
  bash tools/wsl.sh 'python3 tools/regen_memory_index.py'
EOF
    exit 2
fi

# Pass the joined args as a single shell command to WSL.
# Escape the project dir for proper quoting inside the inner shell.
wsl bash -c "cd \"$PROJECT_DIR\" && $*"
