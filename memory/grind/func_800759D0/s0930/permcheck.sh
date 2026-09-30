#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tools/decomp-permuter/nonmatchings/${1:-func_800759D0}
grep -c "" $D/base.c
mipsel-linux-gnu-objdump -d $D/base.o | grep -c "^ .*:"
bash $D/compile.sh $D/base.c -o /tmp/f759_chk.o 2>&1 | head -5
timeout 60 python3 tools/decomp-permuter/permuter.py $D --stack-diffs -j 1 2>&1 | head -8
