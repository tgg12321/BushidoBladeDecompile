#!/bin/bash
# model.diff: every change the POC makes to the pinned commit (build files, maspsx, src)
B=/tmp/q56/tree; M=/tmp/q56/model; O="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/model.diff"
cd /tmp/q56
{ for f in Makefile bb2.ld tools/maspsx/maspsx.py tools/maspsx/maspsx/__init__.py; do diff -u tree/$f model/$f; done
  for f in $(cd $M/src && ls *.c); do diff -u tree/src/$f model/src/$f; done
  echo "--- new file model/poc_noncomm_syms.txt"; cat $M/poc_noncomm_syms.txt; } > "$O"
wc -l "$O"; grep -c "^+__typeof__\|^+[a-z0-9 ]* D_.*POC" "$O"
