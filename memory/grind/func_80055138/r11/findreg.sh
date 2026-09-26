#!/bin/bash
# usage: bash tmp/func_80055138/r11/findreg.sh <tag>:<pseudo>...
# Re-runs the instrumented cc1 on rtl/<tag>.i with BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo>
# and writes the func_80055138 FINDREGDBG block to rtl/<tag>.findreg.<pseudo>.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80055138/r11/rtl"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for spec in "$@"; do
  T=${spec%%:*}; P=${spec##*:}
  BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$P ../../../../tools/gcc-2.7.2/cc1 $FLAGS $T.i -o /tmp/fr55138.s 2> /tmp/fr55138.log
  awk -v P="$P" '$0 ~ ("FINDREGDBG func=func_80055138 pseudo=" P " ") {f=1} $0 ~ /FINDREGDBG func=/ && $0 !~ ("func=func_80055138 pseudo=" P " ") {f=0} f && /FINDREGDBG/ {print}' /tmp/fr55138.log > $T.findreg.$P
  echo "== $spec"
  cat $T.findreg.$P
done
