#!/bin/bash
# Confirms the dumped reuse TU (rtl/tcand.i) is the preprocessed landed tree: cpp src/text1b.c
# (tree, model applied) and compare with rtl/tcand.i.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin $(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)') src/text1b.c 2>/dev/null \
  | grep -v '^# ' > /tmp/tree55138.i
grep -v '^# ' tmp/func_80055138/r11/rtl/tcand.i > /tmp/cand55138.i
if cmp -s /tmp/tree55138.i /tmp/cand55138.i; then echo "TREE == rtl/tcand.i (preprocessed, line markers stripped)"; else echo "DIFFER"; diff /tmp/tree55138.i /tmp/cand55138.i | head; fi
