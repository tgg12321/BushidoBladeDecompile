#!/bin/bash
# fdiff.sh <variant> <func> [stem] : objdump diff of <func> between tmp/prc/<variant>/<stem>.o and build/src/<stem>.o
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
v=$1; f=$2; st=${3:-code6cac_tu2}
dis() { mipsel-linux-gnu-objdump -d -r --no-show-raw-insn "$1" | awk -v F="<$f>:" '$2==F{p=1;next} p&&/^$/{exit} p' | sed 's/^ *[0-9a-f]*:\t//; s/[0-9a-f]* <[^>]*>//g'; }
diff <(dis build/src/$st.o) <(dis tmp/prc/$v/$st.o)
