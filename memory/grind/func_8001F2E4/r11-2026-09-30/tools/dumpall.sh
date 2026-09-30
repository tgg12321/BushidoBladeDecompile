#!/bin/bash
# dumpall.sh <variant>... : alloc.sh each tmp/ff-worker/f1f2e4/<variant>.c into r11/dumps/<variant>, then the naming table
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/ff-worker/f1f2e4
for v in "$@"; do
  tmp/ff-worker/f1f2e4/r11/tools/alloc.sh $T/$v.c $T/r11/dumps/$v > /dev/null 2>&1
  echo "=== $v ($(cat $T/r11/dumps/$v/instcheck.txt))"
  python3 $T/r11/tools/conf.py $T/r11/dumps/$v 2>&1 | grep -v "^obj\|^a \|^b \|^lzcr\|^shift\|^tbl\|^t "
done
