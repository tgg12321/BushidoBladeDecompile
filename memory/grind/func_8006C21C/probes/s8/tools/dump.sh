#!/bin/bash
# usage: dump.sh <name> <flags...>   (needs tmp/c21c/out/<name>.i from orph.py)
# runs project cc1 with extra dump flags; dumps land in tmp/c21c/out/<name>.i.<pass>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
n=$1; shift
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$@" tmp/c21c/out/$n.i -o /tmp/c21c_$n.s
