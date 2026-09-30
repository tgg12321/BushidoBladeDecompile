#!/bin/bash
# tbltest.sh: scratch copy of the working tree with func_80058580's C candidate in its TU (no
# transcribed tables); full link; dump the rodata where its switch tables land.
set -e
R=/home/user/BushidoBladeDecompile
S=/tmp/claude-0/tbl
rm -rf $S && mkdir -p $S
cd $R && tar --exclude=./build --exclude=./.git --exclude=./tools/gcc-2.7.2 --exclude=./tools/maspsx --exclude=./metrics --exclude=./memory --exclude=./tmp --exclude=./disc -cf - . | tar -xf - -C $S
ln -s $R/tools/gcc-2.7.2 $S/tools/gcc-2.7.2; ln -s $R/tools/maspsx $S/tools/maspsx; ln -s $R/disc $S/disc
python3 - <<'PY'
S = "/tmp/claude-0/tbl/"
t = open(S + "src/text1b.c").read()
i = t.index("/* func_80058580's three switch tables")
j = t.index('INCLUDE_ASM("asm/funcs", func_80058580);')
cand = open("/home/user/BushidoBladeDecompile/memory/grind/func_80058580/candidate.c").read()
open(S + "src/text1b.c", "w", newline="\n").write(t[:i] + cand + t[j + len('INCLUDE_ASM("asm/funcs", func_80058580);'):])
PY
cd $S && make -j16 build/bb2.bin > $S/make.log 2>&1 || { tail -20 $S/make.log; exit 1; }
grep -E "^ \.rodata\s+0x" build/bb2.map | grep "text1b.o\|pre_rodata"
mipsel-linux-gnu-objdump -s -j .rodata build/src/text1b.o | tail -8
