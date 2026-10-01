#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80058580/r11b"
C=../../../memory/grind/func_80058580/candidate.c
rm -rf b; mkdir -p b
cp $C b/reuse.c
for v in work1 work2 work3 work4 work5; do
  python3 roles.py $C var b/pv_$v.c $v
  python3 roles.py $C var b/pvbs_$v.c $v bs
  python3 typed.py b/pv_$v.c b/pvt_$v.c
done
python3 roles.py $C all b/pv_all.c
python3 roles.py $C all b/pvbs_all.c bs
python3 typed.py b/pv_all.c b/pvt_all.c
python3 roles.py $C one b
python3 mk_struct.py $C b/pv_work4.c b
for f in b/*.c; do echo "$(basename $f) $(python3 stmtcheck.py $C $f)"; done > stmtcheck.txt
ls b | wc -l
