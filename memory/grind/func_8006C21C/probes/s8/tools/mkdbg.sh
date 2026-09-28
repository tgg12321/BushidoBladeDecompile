#!/bin/bash
# Build a PRIVATE diagnostic cc1 in /tmp/gccdbg with combine 3->2 / orphan logging.
# Never a build path. Verifies output == build cc1 on the candidate TU.
set -e
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
rm -rf /tmp/gccdbg
cp -r "$R/tools/gcc-2.7.2" /tmp/gccdbg
python3 "$R/tmp/c21c/patch_comb.py"
cd /tmp/gccdbg
touch combine.c
rm -f combine.o; (TMPDIR=/dev/shm make combine.o CFLAGS="-O -g -fgnu89-inline" && TMPDIR=/dev/shm make cc1 CFLAGS="-g -fgnu89-inline") > /tmp/gccdbg_make.log 2>&1 || { tail -30 /tmp/gccdbg_make.log; exit 1; }
ls -la /tmp/gccdbg/cc1
