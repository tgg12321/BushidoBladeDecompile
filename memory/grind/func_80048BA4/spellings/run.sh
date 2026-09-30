#!/bin/bash
# usage: run.sh <func> <dir> [variants...]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
export PYTHONPATH=.
f=$1; d=$2; shift 2
for v in "$@"; do echo "== $v"; python3 tmp/ffc/score_full.py "$f" "$d/$v.c" 2>&1 | tail -3; done
