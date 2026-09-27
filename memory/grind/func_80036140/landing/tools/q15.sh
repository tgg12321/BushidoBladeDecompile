#!/bin/bash
# Q15 classification tables for func_80036140's comm row (and the two exact rows for completeness).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; O=memory/grind/func_80036140/landing/q15
mkdir -p $O
python3 $H/classify2.py $H/calib/f5_comm-G8/code6cac_b5-G8.obj func_80036140 g_cd_atv D_800A36B8 g_cd_result > $O/func_80036140.cc1psx-G8.txt
python3 $H/classify2.py $H/calib/OURS_f5comm/code6cac_b5-G8.obj func_80036140 g_cd_atv D_800A36B8 g_cd_result > $O/func_80036140.ourcc1-G8.txt
python3 $H/classify2.py $H/calib/f4_comm-G8/code6cac_b4-G8.obj cdrom_SetMix g_cd_atv > $O/cdrom_SetMix.cc1psx-G8.txt
python3 $H/classify2.py $H/calib/f4_comm-G8/code6cac_b4-G8.obj func_80035F78 D_800A36B8 > $O/func_80035F78.cc1psx-G8.txt
for f in $O/*.txt; do echo "== $f"; grep "^# totals\|MISMATCH\|UNCLASSIFIED" $f; grep -c " OK$" $f; done
