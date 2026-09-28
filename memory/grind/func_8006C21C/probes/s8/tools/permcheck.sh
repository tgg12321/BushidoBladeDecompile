#!/bin/bash
# usage: permcheck.sh <name> : standalone base.c cc1 output vs full-TU cc1 output (function only)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
n=$1; W=tmp/c21c/perm_$n
mv $W/base.c $W/base_src.c 2>/dev/null; [ -f $W/base_src.c ] && mipsel-linux-gnu-cpp -P -undef -lang-c $W/base_src.c > $W/base.c
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
tools/gcc-2.7.2/build/cc1 $F < $W/base.c 2>$W/cc1.err > $W/standalone.s || { cat $W/cc1.err | head; exit 1; }
awk '/^func_8006C21C:/{p=1} p{print} /\.end\tfunc_8006C21C/{p=0}' $W/standalone.s | grep -v '^\s*\.' > $W/sa.fn
grep -v '^\s*\.' tmp/c21c/out/$n.fn.s > $W/tu.fn
if cmp -s $W/sa.fn $W/tu.fn; then echo "[$n] standalone == full TU"; else echo "[$n] DIFFERS: $(diff $W/sa.fn $W/tu.fn | grep -c '^[<>]') lines"; fi
bash $W/compile.sh $W/base.c -o $W/base.o && ls -la $W/base.o
