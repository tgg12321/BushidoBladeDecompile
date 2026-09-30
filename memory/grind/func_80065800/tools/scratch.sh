#!/bin/bash
# Scratch full build of the func_80065800 landing (never touches the real tree).
# usage: scratch.sh <variant>   variant = pkg | move
#   pkg : land.py as banked (merge + body + jtbl deletion + symbol rows)
#   move: pkg + text1b|text1b_tu1c cut moved from snd_Init to func_80061064 (move.py)
set -e
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
S=/tmp/laneC/$1
rm -rf "$S" && mkdir -p "$S"
cd "$R"
tar -cf - Makefile bb2.ld bb2.sha1 *.txt asm src include engine | tar -xf - -C "$S"
ln -s "$R/tools" "$S/tools"; ln -s "$R/disc" "$S/disc"; ln -s "$R/.venv" "$S/.venv"
mkdir -p "$S/tmp/f65800"
cp "${CAND:-$R/memory/grind/func_80065800/candidate.c}" "$S/tmp/f65800/final.c"
cd "$S"
source .venv/bin/activate
python3 "$R/memory/grind/func_80065800/tools/land.py"
if [ "$1" = move ]; then python3 "$R/memory/grind/func_80065800/tools/move.py"; fi
make -j16 check > make.log 2>&1 || { tail -30 make.log; exit 1; }
tail -3 make.log
sha1sum build/*.exe build/SLUS* 2>/dev/null | head
grep -E "^ \.rodata\s+0x" build/bb2.map | grep -E "text1b|pre_rodata_b" || true
