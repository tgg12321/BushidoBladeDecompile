#!/bin/bash
# Regenerate every variant from cand.c, score all (header model, scratch TU), dump the four
# bodies the ledger cites, run the FINDREG traces and the statement-list receipts.
# usage (WSL): bash tmp/func_80055138/r11/all.sh > tmp/func_80055138/r11/all.log 2>&1
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
R=tmp/func_80055138/r11; V=$R/v
rm -f $V/*.c
python3 $R/mk.py && python3 $R/mk2.py
echo "== scores (model.py score: header model, scratch TU, vs build/src/text1b.o)"
bash $R/score.sh $V/cand.c $V/pv.c $V/pv_fs.c $V/abl_lvl5.c $V/abl_lvl3.c $V/abl_row_idx.c \
  $V/abl_move_mask.c $V/abl_stat1.c $V/abl_stat2.c $V/ctr_split.c $V/s_u8.c $V/s_decl_init.c \
  $V/s_tests.c $V/part_case_loop.c $V/part_mask_rest.c $V/part_casemask_stats.c \
  $V/nostage_fs.c $V/nostage_blk.c $V/nostage_inline.c
echo "== whole-file"
python3 $R/model.py score $V/cand.c --all
echo "== statement lists"
python3 $R/stmtcheck.py $V/cand.c $V/pv.c temp lvl5 lvl3 row_idx move_mask stat1 stat2
python3 $R/stmtcheck.py $V/cand.c $V/ctr_split.c idx zero_i
diff $R/cand.c $V/cand.c && echo "v/cand.c == cand.c"
echo "== dumps"
for spec in cand:cand pv:pv ctr:ctr_split nostage:nostage_fs; do
  bash $R/dump.sh ${spec%%:*} $V/${spec##*:}.c | grep -E "IDENTITY|score"
done
echo "== findreg"
bash $R/findreg.sh pv:188 pv:106 ctr:90 cand:99 cand:89
