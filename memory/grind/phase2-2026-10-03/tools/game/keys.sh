#!/bin/bash
# keys.sh BASE FILE... : key.py over the given src/ .c files (no git calls from WSL)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
b=$1; shift
for f in "$@"; do python3 memory/grind/phase2-2026-10-03/tools/key.py "$b" "$f"; done
echo "keys done ($# files)"
