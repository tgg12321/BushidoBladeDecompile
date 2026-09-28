#!/bin/bash
# usage: loops.sh name... -> prints loop sizes + movables for func_8006C21C
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for n in "$@"; do
  bash tmp/c21c/dump.sh "$n" -dL
  echo "== $n"
  awk '/^;; Function func_8006C21C/{p=1} /^;; Function /&&!/func_8006C21C/{p=0} p' tmp/c21c/out/$n.i.loop | grep "^Loop from\|^Insn .*: regno" | head -${LINES_MAX:-8}
done
