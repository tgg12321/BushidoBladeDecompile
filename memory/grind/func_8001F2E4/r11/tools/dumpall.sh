#!/bin/bash
# dumpall.sh <variantdir> <dumpdir> <variant>[:pseudo,pseudo...]... : dump.sh each <variantdir>/<v>.c into
# <dumpdir>/<v> (FINDREG for the listed pseudos), then write and print the allocation table (table.txt).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
VD="$1"; DD="$2"; shift 2
T=tmp/func_8001F2E4/tools
for spec in "$@"; do
  v="${spec%%:*}"
  ps=""
  [ "$spec" != "$v" ] && ps="${spec#*:}"
  $T/dump.sh "$VD/$v.c" "$DD/$v" ${ps//,/ } > /dev/null 2>&1 || echo "dump FAILED: $v"
  echo "=== $v ($(cat $DD/$v/instcheck.txt 2>/dev/null))"
  python3 $T/conf.py "$DD/$v" > "$DD/$v/table.txt" 2>&1
  grep -v -E "^(obj|a|b|lzcr|shift|tbl|t|dist|dist_sq)(#[0-9])? " "$DD/$v/table.txt"
done
