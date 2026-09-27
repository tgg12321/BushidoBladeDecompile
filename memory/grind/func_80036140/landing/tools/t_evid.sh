#!/bin/bash
# Evidence refresh for the five-commit sequence (scratch trees stB..stE from t_steps.sh).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; L=memory/grind/func_80036140/landing; C=$H/calib.py
mkdir -p $L/steps $L/q15 $L/calib2
# --- move records
REV=$(cat $H/stA/.rev 2>/dev/null || git rev-parse HEAD)
{
echo "== commit B: code6cac_b2_post.c @ rev -> b2_post | b4 | b4_post"
git show HEAD:src/code6cac_b2_post.c > $H/pre_B.c
python3 $H/movecheck2.py $H/pre_B.c $H/stB/src/code6cac_b2_post.c $H/stB/src/code6cac_b4.c $H/stB/src/code6cac_b4_post.c
echo; echo "== commit D: code6cac_b4_post.c (commit C state) -> b4_post | b5 | b5_post"
python3 $H/movecheck2.py $H/stC/src/code6cac_b4_post.c $H/stD/src/code6cac_b4_post.c $H/stD/src/code6cac_b5.c $H/stD/src/code6cac_b5_post.c
} > $L/steps/movecheck.txt 2>&1
grep "RESULT\|head block" $L/steps/movecheck.txt
# --- -G8 screening at every state that has a -G8 TU
for st in stB stC stD stE; do for tu in code6cac_b4 code6cac_b5; do
  [ -f $H/$st/src/$tu.c ] && python3 $H/screen.py $st $tu $L/steps/screen_${st}_$tu.txt | grep "sdata_syms.txt:\|RESULT"
done; done
# --- calibration: commit-B TU (per-byte handles; bases as tentative defs) and final b5 (stE)
python3 $C stB code6cac_b4 B4split_comm-G8 comm -G8 cdrom_SetMix func_80035F78 -- g_cd_atv D_800A36B8 D_800A3840 D_800A3854 | grep -v "target gp"
python3 $C stB code6cac_b4 B4split_comm-G0 comm -G0 cdrom_SetMix func_80035F78 -- g_cd_atv D_800A36B8 D_800A3840 D_800A3854 | grep -v "target gp"
for cls in comm extern static init; do for G in -G8 -G0; do
  python3 $C stE code6cac_b5 E5_${cls}$G $cls $G func_80036140 func_80036940 -- g_cd_result g_cd_atv D_800A36B8 D_800A3840 D_800A3854
done; done > $L/calib2/E5_all.txt 2>&1
python3 $C stE code6cac_b5 OURS_E5comm comm -G8 func_80036140 func_80036940 -- g_cd_result g_cd_atv D_800A36B8 D_800A3840 D_800A3854 >> $L/calib2/E5_all.txt 2>&1
grep -v "target gp\|ours   gp" $L/calib2/E5_all.txt
python3 $H/classify2.py $H/calib/E5_comm-G8/code6cac_b5-G8.obj func_80036140 g_cd_atv D_800A36B8 g_cd_result > $L/q15/func_80036140.cc1psx-G8.txt
python3 $H/classify2.py $H/calib/OURS_E5comm/code6cac_b5-G8.obj func_80036140 g_cd_atv D_800A36B8 g_cd_result > $L/q15/func_80036140.ourcc1-G8.txt
for f in $L/q15/func_80036140.*.txt; do echo "$f: $(grep '^# totals' $f) gp/sym+N OK=$(grep -c ' OK$' $f) MISMATCH=$(grep -c MISMATCH $f)"; done
cp $H/calib/*/*.result.txt $L/calib2/ 2>/dev/null; true
