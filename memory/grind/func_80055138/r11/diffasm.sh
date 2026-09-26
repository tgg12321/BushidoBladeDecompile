#!/bin/bash
# usage: bash tmp/func_80055138/r11/diffasm.sh <body.c>   -> unified diff of func_80055138 (target vs ours), header model
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tmp/func_80055138/r11/model.py score "$1"
dis() { mipsel-linux-gnu-objdump -d --no-show-raw-insn -M no-aliases "$1" | awk '/<func_80055138>:/{f=1;next} f&&/^$/{exit} f' | sed -E 's/^ *[0-9a-f]+:\s*//; s/<[^>]*>//; s/\s+/ /g'; }
dis build/src/text1b.o > /tmp/t55138.s
dis tmp/func_80055138/wf3/text1b.o > /tmp/o55138.s
diff -U2 /tmp/t55138.s /tmp/o55138.s | sed -n 3,400p
