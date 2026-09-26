#!/bin/bash
# usage: bash tmp/func_8003993C/r11/dump_prefs.sh <tag> <pseudo> [<pseudo>...]
# Re-runs the instrumented tools/gcc-2.7.2/cc1 on rtl/<tag>.i (made by dump.sh) with
#   BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1   (local-alloc: every quantity's copy/arith hard-reg suggestions,
#                                       every find_free_reg call's used / first_used sets)
#   BB2_FINDREG_DEBUG=<pseudo>         (global.c find_reg: conflicts, someone_prefers, used_so_far,
#                                       pass0 set, own_copy_prefs, own_full_prefs)
# and keeps the func_8003993C part: rtl/<tag>.sugg and rtl/<tag>.findreg.<pseudo>.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; shift
D=tmp/func_8003993C/r11/rtl
FLAGS=$(python3 -c 'from engine import buildconfig as c; print(c.CC_FLAGS)')
cd $D
BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o /dev/null 2> $TAG.sugg.all || true
python3 - "$TAG" <<'EOF'
import sys
tag = sys.argv[1]
out, on = [], False
for l in open(tag + ".sugg.all"):
    if l.startswith("SUGGDBG-QTY func="):
        on = l.startswith("SUGGDBG-QTY func=func_8003993C ")
    if on:
        out.append(l)
open(tag + ".sugg", "w").write("".join(out))
EOF
rm -f $TAG.sugg.all
for P in "$@"; do
  BB2_FINDREG_DEBUG=$P ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o /dev/null 2>&1 | grep -A12 "FINDREGDBG func=func_8003993C pseudo=$P " > $TAG.findreg.$P || true
done
echo "prefs $TAG: $(wc -l < $TAG.sugg) sugg lines; findreg for $*"
