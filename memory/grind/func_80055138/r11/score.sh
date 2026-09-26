#!/bin/bash
# usage: bash tmp/func_80055138/r11/score.sh <file.c>...   (header-model whole-file score, --only)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for f in "$@"; do
  printf '%s ' "$(basename "$f")"
  python3 tmp/func_80055138/r11/model.py score "$f" | tail -1
done
