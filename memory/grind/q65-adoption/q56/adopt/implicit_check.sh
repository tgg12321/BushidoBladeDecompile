#!/bin/bash
# implicit_check.sh: for each move/split/merge step, the implicit function declarations (cc1 -Wimplicit) of
# the C files it touches, before vs after, per file and as a union (the rodata-align section 7/9 check).
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IMPL="$R/memory/grind/func_80065800/tools/implicit.sh"
A="/tmp/q56/adopt tree"
for pair in "step01 step02" "step02 step03" "step03 step04" "step04 step05" "step05 step06" "step06 step07" "step09 step10"; do
  set -- $pair
  files=$(cd "$A" && git diff --name-only "$1" "$2" -- 'src/*.c' | tr '\n' ' ')
  for t in "$1" "$2"; do
    d=/tmp/q56/impl_$t; rm -rf $d; mkdir -p $d
    (cd "$A" && git archive "$t" src include) | tar -x -C $d
    ln -sfn "$A/tools" $d/tools
  done
  ex=""; for f in $files; do [ -f /tmp/q56/impl_$1/$f ] && ex="$ex $f"; done
  bash "$IMPL" /tmp/q56/impl_$1 $ex | cut -d' ' -f2 | sort -u > /tmp/q56/impl_$1.u
  ex=""; for f in $files; do [ -f /tmp/q56/impl_$2/$f ] && ex="$ex $f"; done
  bash "$IMPL" /tmp/q56/impl_$2 $ex | cut -d' ' -f2 | sort -u > /tmp/q56/impl_$2.u
  if diff -q /tmp/q56/impl_$1.u /tmp/q56/impl_$2.u >/dev/null; then r="union equal ($(wc -l < /tmp/q56/impl_$2.u) names)"; else r="UNION DIFFERS: $(diff /tmp/q56/impl_$1.u /tmp/q56/impl_$2.u | tr '\n' ' ')"; fi
  echo "$1 -> $2 [$files]: $r"
done
