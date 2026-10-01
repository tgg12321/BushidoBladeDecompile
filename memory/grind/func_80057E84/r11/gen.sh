#!/bin/bash
# gen.sh: regenerate every Ruling 11 twin of candidate.c into tmp/func_80057E84/r11/b and the
# (C)(2) statement-list receipt (stmtcheck.txt). Run from anywhere.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=memory/grind/func_80057E84/r11
B=tmp/func_80057E84/r11/b
rm -rf $B; mkdir -p $B
python3 $R/gen.py memory/grind/func_80057E84/candidate.c $B
for f in $B/*.c; do echo "$(basename $f) $(python3 $R/stmtcheck.py memory/grind/func_80057E84/candidate.c $f)"; done > $R/stmtcheck.txt
cat $R/stmtcheck.txt
