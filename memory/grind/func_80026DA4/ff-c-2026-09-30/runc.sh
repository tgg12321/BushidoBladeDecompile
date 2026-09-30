#!/bin/bash
# usage: runc.sh <func> <dir> [variants...]  -- official CLI, body candidates
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
f=$1; d=$2; shift 2
for v in "$@"; do printf "== %s: " "$v"; python3 -m engine.cli sandbox "$f" --disable all --candidate "$d/$v.c" 2>&1 | grep -E '"score"|"build_insns"|"error"|Error|error:' | tr -d '\n '; echo; done
