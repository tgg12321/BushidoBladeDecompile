#!/bin/bash
# objd_b.sh FUNC [base|wk] : objdump FUNC (51268), relocations inline
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
o=tmp/p2/wk/obj/main_51268.o; [ "$2" = base ] && o=tmp/p2/snap/base/obj/main/51268.o
mipsel-linux-gnu-objdump -d -r --no-show-raw-insn "$o" | awk -v f="<$1>:" '$0 ~ f {p=1} p&&/^$/{exit} p' | sed 's/^ *[0-9a-f]*:\t//'
