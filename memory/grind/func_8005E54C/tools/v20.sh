#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tmp/func_8005E54C/mkprobes.py
V=tmp/func_8005E54C/v
for f in fin fin_plain fin_nodigit fin_union loc_wins_after loc_vals_after prod1_guard prod2_narrow prod3_named; do
  a=$(python3 tmp/func_8005E54C/sbxp.py $V/$f.c memory/grind/func_8005E54C/match0/tu_patch.py | tail -1)
  b=$(python3 tmp/func_8005E54C/sbxp.py $V/$f.c memory/grind/func_8005E54C/match0/tu_patch.py --nostrip | tail -1)
  cp tmp/func_8005E54C/sbxp/text1b.o tmp/func_8005E54C/obj_$f.o
  PATCH=tmp/func_8005E54C/applytp.py bash tmp/func_8005E54C/dump.sh $V/$f.c tmp/func_8005E54C/df_$f > /dev/null
  fr=$(grep '\.frame' tmp/func_8005E54C/df_$f/f.s | sed 's/.*# //')
  echo "$f | strip $a | nostrip $b | $fr"
  rm -f tmp/func_8005E54C/df_$f/t.i tmp/func_8005E54C/df_$f/t.c tmp/func_8005E54C/df_$f/t.s
done
echo "--- fin (volatile) vs fin_plain objects:"
python3 tools/objdiff.py tmp/func_8005E54C/obj_fin.o tmp/func_8005E54C/obj_fin_plain.o --summary
sha1sum tmp/func_8005E54C/obj_fin.o tmp/func_8005E54C/obj_fin_plain.o
diff tmp/func_8005E54C/df_fin/f.s tmp/func_8005E54C/df_fin_plain/f.s && echo "cc1 listings identical"
