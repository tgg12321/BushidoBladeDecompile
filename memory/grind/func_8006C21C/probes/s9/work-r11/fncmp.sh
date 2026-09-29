#!/bin/bash
# compare func_8006C21C's .s between build cc1 and diagnostic cc1 outputs of alloc9.sh; list differing functions
for n in "$@"; do
  a=/tmp/c21c9_$n.s; b=/tmp/c21c9_dbg_$n.s
  ex() { awk '/^func_8006C21C:/{p=1} p{print} /\.end\tfunc_8006C21C/{p=0}' "$1"; }
  if cmp -s <(ex $a) <(ex $b); then echo "$n: func_8006C21C identical ($(ex $a | wc -l) lines)"; else echo "$n: func_8006C21C DIFFERS"; fi
  diff $a $b | grep -c '^[<>]'
  diff <(grep -n '^[a-zA-Z_0-9]*:$' $a) <(grep -n '^[a-zA-Z_0-9]*:$' $b) | head -3
  diff $a $b | head -8
done
