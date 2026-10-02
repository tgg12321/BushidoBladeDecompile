#!/bin/bash
# usage: r11all7.sh <landing body.c> : every Ruling 11 spelling of the landing body (nocopy rotation form), scored
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_8005490C" || exit 1
rm -rf g7 && mkdir -p g7
python3 prep6.py $1 g7/base.c
python3 tocopy.py g7/base.c g7/base_copy.c
python3 gen.py g7/base_copy.c g7/v >/dev/null
python3 gens.py g7/v g7/st >/dev/null
python3 genz4.py g7/v/P_all_Zsplit.c g7/z4
rm g7/st/R2_no_xz_copies_*.c
cp g7/v/P_split_Zsplit.c g7/st/R2_xz_copies_split.c
cp g7/v/P_all_Zsplit.c g7/st/R2_xz_copies_Pall_Zsplit.c
cp g7/base_copy.c g7/st/R2_xz_copies_landing.c
python3 post.py $(ls g7/v/*.c g7/st/*.c g7/z4/*.c | grep -v R2_xz_copies)
cd ../.. && bash tmp/func_8005490C/xbm.sh tmp/func_8005490C/g7/base.c tmp/func_8005490C/g7/v/*.c tmp/func_8005490C/g7/st/*.c tmp/func_8005490C/g7/z4/*.c
