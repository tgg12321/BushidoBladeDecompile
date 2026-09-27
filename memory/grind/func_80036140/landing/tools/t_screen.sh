#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; L=memory/grind/func_80036140/landing
for st in stB stC stD stE; do for tu in code6cac_b4 code6cac_b5; do
  [ -f $H/$st/src/$tu.c ] && python3 $H/screen.py $st $tu $L/steps/screen_${st}_$tu.txt | grep "RESULT"
done; done
{
echo "== commit B"; git show HEAD:src/code6cac_b2_post.c > $H/pre_B.c
python3 $H/movecheck2.py $H/pre_B.c $H/stB/src/code6cac_b2_post.c $H/stB/src/code6cac_b4.c $H/stB/src/code6cac_b4_post.c
echo "== commit D"
python3 $H/movecheck2.py $H/stC/src/code6cac_b4_post.c $H/stD/src/code6cac_b4_post.c $H/stD/src/code6cac_b5.c $H/stD/src/code6cac_b5_post.c
} > $L/steps/movecheck.txt 2>&1; grep RESULT $L/steps/movecheck.txt
cp $H/t_steps.log $L/steps/t_steps.log; cp $H/t_u32b.log $L/steps/t_u32b.log
python3 $H/mkdiffs.py $(head -1 $H/t_steps.log | cut -d" " -f2)
