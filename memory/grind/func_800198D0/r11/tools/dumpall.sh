#!/bin/bash
# dumpall.sh VARIANT_DIR OUT_DIR name...  -> per-variant alloc dumps + named tables
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
VD="$1"; OD="$2"; shift 2
mkdir -p "$OD"
for n in "$@"; do
  bash tmp/func_800198D0/alloc.sh "$VD/$n.c" "$OD/$n" > /dev/null 2>&1 || true
  {
    echo "### $n  (instrumented cc1 BB2_ALLOC_DEBUG=1 -dr -dl -dg; tmp/func_800198D0/alloc.sh $VD/$n.c)"
    python3 tmp/func_800198D0/r11table.py "$OD/$n" 2>&1 | grep -v -E "^(hi|need|left|from|to|k|top|bit)(#[0-9]+)? "
    echo
  } > "$OD/$n.table.txt"
  echo "$n done"
done
