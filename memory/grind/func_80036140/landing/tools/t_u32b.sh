#!/bin/bash
# Q14 byte identity on the final state (stE): expected_pos s32 (as landed) vs u32.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140
for n in fin_s32 fin_u32; do rm -rf $H/$n; cp -r $H/stE $H/$n; rm -rf $H/$n/build; done
sed -i 's|    s32 expected_pos; /\* 0x80101EA0 \*/|    u32 expected_pos; /* 0x80101EA0 */|' $H/fin_u32/include/code6cac.h
grep -n "expected_pos; /\*" $H/fin_s32/include/code6cac.h $H/fin_u32/include/code6cac.h
bash $H/fullbuild.sh fin_s32 -j8; bash $H/fullbuild.sh fin_u32 -j8
n=0; d=0; for o in $H/fin_s32/build/src/*.o; do n=$((n+1)); cmp -s $o $H/fin_u32/build/src/$(basename $o) || { d=$((d+1)); echo "DIFF $(basename $o)"; }; done
echo "final state (stE): $n C objects compared (s32 vs u32 expected_pos), $d differ"
python3 $H/q14_accessors.py > memory/grind/func_80036140/landing/q14_accessors.txt
grep "sha1" memory/grind/func_80036140/landing/q14_accessors.txt
