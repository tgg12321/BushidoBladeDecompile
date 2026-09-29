#!/bin/bash
# sscore.sh <permdir> : standalone base.o vs target.o, cmp.py line diff
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=$1
for o in base target; do
  mipsel-linux-gnu-objdump -d -z --no-show-raw-insn $D/$o.o | awk '/<func_800187F4>:/{f=1;next} f&&/^$/{exit} f' | sed -e 's/^ *\([0-9a-f]*\):\t/\1 /' -e 's/\t/ /g' -e 's/<[^>]*>//' > $D/$o.dis
done
python3 tmp/func_800187F4/cmp.py $D/target.dis $D/base.dis $D/sbs.txt $(basename $D)
