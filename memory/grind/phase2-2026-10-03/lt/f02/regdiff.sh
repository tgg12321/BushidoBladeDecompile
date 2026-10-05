#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
d() { mipsel-linux-gnu-objdump -d --no-show-raw-insn "$1" | awk '/<func_8002A458>:/{f=1} f&&/^$/{exit} f' | sed 's/^ *[0-9a-f]*:\t//'; }
diff <(d tmp/p2/snap/base/obj/main/17AFC.o) <(d tmp/p2/wk/obj/main_17AFC.o)
