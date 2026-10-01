#!/bin/bash
# score_many.sh <func> <variant>... : harness score of <func> in each tmp/prc/<variant>/
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
f=$1; shift
for v in "$@"; do
  printf '%-8s ' "$v"
  python3 tmp/prc/harness.py tmp/prc/$v "$f" | tail -1
done
