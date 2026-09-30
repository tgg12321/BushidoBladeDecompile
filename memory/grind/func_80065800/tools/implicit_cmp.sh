#!/bin/bash
# implicit_cmp.sh <before-root> <after-root> <out-dir>: implicit function declarations of
# text1b.c / text1b_tu1c.c before and after the TU-boundary move, per file and as unions.
T=memory/grind/func_80065800/tools/implicit.sh
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mkdir -p "$3"
bash "$R/$T" "$1" src/text1b.c src/text1b_tu1c.c > "$3/before.txt"
bash "$R/$T" "$2" src/text1b.c src/text1b_tu1c.c > "$3/after.txt"
cut -d' ' -f2 "$3/before.txt" | sort -u > "$3/before.union"
cut -d' ' -f2 "$3/after.txt" | sort -u > "$3/after.union"
echo "before: $(wc -l < "$3/before.txt") file/name pairs, $(wc -l < "$3/before.union") names"
echo "after:  $(wc -l < "$3/after.txt") file/name pairs, $(wc -l < "$3/after.union") names"
echo "--- per-file diff (before < > after)"; diff "$3/before.txt" "$3/after.txt"
echo "--- union diff"; diff "$3/before.union" "$3/after.union" && echo "unions equal"
