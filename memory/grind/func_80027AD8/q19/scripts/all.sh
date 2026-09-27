#!/bin/bash
# Dump every proof body (tmp/func_80027AD8/r11/v/*.c) and print a per-tag summary.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for f in tmp/func_80027AD8/r11/v/*.c; do
  t=$(basename $f .c)
  echo "== $t"
  bash tmp/func_80027AD8/r11/dump.sh $t $f 2>&1 | grep -E "IDENTITY|Error|error" | head -3
done
