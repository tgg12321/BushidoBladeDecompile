#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
for f in $(grep -l '#include "code6cac.h"' src/*.c); do
  stem=$(basename $f .c)
  if [ -f tmp/func_80023F08/srcx/$stem.c ]; then s=tmp/func_80023F08/srcx/$stem.c; else s=src/$stem.c; fi
  python3 tmp/func_80023F08/xbuild.py $stem $s tmp/func_80023F08/inc ${FOCUS:-}
done
