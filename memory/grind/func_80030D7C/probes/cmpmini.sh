#!/bin/bash
# compare function code from mini TU (base.c) vs full TU (base_full.c) in a workspace
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
WS="$1"
bash "$WS/compile.sh" "$WS/base_full.c" -o "$WS/full.o"
mipsel-linux-gnu-objdump -d --no-show-raw-insn "$WS/full.o" | grep '^ ' | cut -f2- > "$WS/full.dis"
mipsel-linux-gnu-objdump -d --no-show-raw-insn "$WS/base.o" | grep '^ ' | cut -f2- > "$WS/base.dis"
if cmp -s "$WS/full.dis" "$WS/base.dis"; then echo SAME; else echo DIFFER; diff "$WS/full.dis" "$WS/base.dis" | head; fi
