#!/bin/bash
# usage: mkperm.sh <tu_copy.c> <ws_name>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
ws=tools/decomp-permuter/nonmatchings/$2
mkdir -p $ws
cp tools/decomp-permuter/nonmatchings/func_80048FFC_v2/compile.sh $ws/compile.sh
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -P -DPERMUTER -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $1 > $ws/base.c
python3 tools/decomp-permuter/strip_other_fns.py $ws/base.c func_8005490C
awk 'NR==1,/glabel func_80048FFC/' tools/decomp-permuter/nonmatchings/func_80048FFC_v2/target.s | grep -v -e "glabel func_80048FFC" -e "gp=64" > $ws/target.s
cat asm/funcs/func_8005490C.s >> $ws/target.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 $ws/target.s -o $ws/target.o
sed 's/func_80048FFC/func_8005490C/' tools/decomp-permuter/nonmatchings/func_80048FFC_v2/settings.toml > $ws/settings.toml
bash $ws/compile.sh $ws/base.c -o $ws/base.o && echo compiled
python3 tools/decomp-permuter/permuter.py $ws --debug 2>&1 | tail -3
