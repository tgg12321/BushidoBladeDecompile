#!/bin/bash
# regdiff_fn.sh FUNC : objdump FUNC in base snapshot vs tmp/p2/wk ablated object
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
d() { mipsel-linux-gnu-objdump -d --no-show-raw-insn "$1" | awk -v f="<$2>:" '$0 ~ f {p=1} p&&/^$/{exit} p' | sed 's/^ *[0-9a-f]*:\t//'; }
diff <(d tmp/p2/snap/base/obj/main/17AFC.o $1) <(d tmp/p2/wk/obj/main_17AFC.o $1)
