#!/bin/bash
# usage: bash tmp/func_80055138/r11/dump_many.sh <tag>...   (dumps v/<tag>.c via dump.sh, scratch TU)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for t in "$@"; do
  bash tmp/func_80055138/r11/dump.sh "$t" "tmp/func_80055138/r11/v/$t.c" | grep -E "IDENTITY|score"
done
