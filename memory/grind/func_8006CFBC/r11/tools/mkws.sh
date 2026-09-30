#!/bin/bash
# mkws.sh <ws> <body.c>: permuter workspace; target.o from the original asm; verify a body against it
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
WS=$1; BODY=$2
mkdir -p $WS
python3 tmp/func_8006CFBC/mkperm.py $BODY $WS
cp tmp/func_8006CFBC/perm_compile.sh $WS/compile.sh; chmod +x $WS/compile.sh
printf 'func_name = "func_8006CFBC"\ncompiler_type = "gcc"\n' > $WS/settings.toml
{ echo '.include "include/macro.inc"'; echo '.set noat'; echo '.set noreorder'; echo '.section .text'; cat asm/funcs/func_8006CFBC.s; } > $WS/target.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $WS/target.o $WS/target.s
bash $WS/compile.sh $WS/base.c -o $WS/base.o
f() { mipsel-linux-gnu-objdump -dr "$1" | awk '/<func_8006CFBC>:/{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed 's/^ *[0-9a-f]*:\t[0-9a-f ]*\t//' | grep -v '^$'; }
f $WS/target.o > $WS/t.txt; f $WS/base.o > $WS/b.txt
wc -l $WS/t.txt $WS/b.txt; diff $WS/t.txt $WS/b.txt | head -20; diff $WS/t.txt $WS/b.txt | grep -c '^[<>]' || true
