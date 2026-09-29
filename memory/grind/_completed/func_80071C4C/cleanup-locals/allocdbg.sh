#!/bin/bash
# usage: allocdbg.sh <dumpdir> <func> [findreg pseudo...]
# Re-runs the instrumented cc1 on <dumpdir>/t.i (written by dump.sh) with
# BB2_ALLOC_DEBUG (global.c allocno order/priority) and optional BB2_FINDREG_DEBUG.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
DIR="$1"; FUNC="$2"; shift 2
CC_FLAGS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CC_FLAGS)")
echo "cmd: BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $CC_FLAGS t.i -o /dev/null"
BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $CC_FLAGS "$DIR/t.i" -o /dev/null 2>&1 | grep "func=$FUNC "
for p in "$@"; do
  echo "cmd: BB2_FINDREG_DEBUG=$p tools/gcc-2.7.2/cc1 $CC_FLAGS t.i -o /dev/null"
  BB2_FINDREG_DEBUG=$p tools/gcc-2.7.2/cc1 $CC_FLAGS "$DIR/t.i" -o /dev/null 2>&1 | awk -v f="func=$FUNC" '$0 ~ f {on=1} on && /FINDREGDBG/ {print} /FINDREGDBG func=/ && $0 !~ f {on=0}'
done
