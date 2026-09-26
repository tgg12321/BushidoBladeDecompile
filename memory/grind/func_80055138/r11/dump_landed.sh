#!/bin/bash
# Dump the LANDED tree's src/text1b.c itself (lock held, model applied) as tag `landed`.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
SRC=src/text1b.c bash tmp/func_80055138/r11/dump_tree.sh landed
bash tmp/func_80055138/r11/findreg.sh landed:99 landed:89 > /dev/null
python3 tmp/func_80055138/r11/cmp_dumps.py cand landed
for P in 99 89; do cmp -s tmp/func_80055138/r11/rtl/cand.findreg.$P tmp/func_80055138/r11/rtl/landed.findreg.$P && echo "findreg $P IDENTICAL" || echo "findreg $P DIFF"; done
