#!/bin/bash
# usage: r11all.sh <landing body.c> : regenerate every Ruling 11 spelling from the landing body and score each
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_8005490C" || exit 1
rm -rf g6 && mkdir -p g6
python3 prep6.py $1 g6/base.c
python3 gen.py g6/base.c g6/v >/dev/null
python3 gens.py g6/v g6/st >/dev/null
python3 genz4.py g6/v/P_all_Zsplit.c g6/z4
cd ../.. && bash tmp/func_8005490C/xbm.sh tmp/func_8005490C/g6/base.c tmp/func_8005490C/g6/v/*.c tmp/func_8005490C/g6/st/*.c tmp/func_8005490C/g6/z4/*.c
