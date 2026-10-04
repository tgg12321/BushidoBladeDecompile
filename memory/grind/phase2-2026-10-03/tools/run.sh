#!/bin/bash
# run.sh SCRIPT [args...] : run a python script in WSL from the repo root with the venv active.
#   wsl bash memory/grind/phase2-2026-10-03/tools/run.sh memory/grind/phase2-2026-10-03/tools/cmp.py base new
cd "$(dirname "$0")/../../../.." && source .venv/bin/activate && python3 "$@"
