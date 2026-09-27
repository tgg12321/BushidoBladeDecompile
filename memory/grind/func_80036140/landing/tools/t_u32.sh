#!/bin/bash
# Q14 byte-identity: the final landing with expected_pos declared s32 (as landed) vs u32; every object compared.
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140
REV=$(git rev-parse HEAD)
REV=$REV bash $H/t_final.sh fin_s32 > $H/fin_s32.log 2>&1
REV=$REV bash $H/t_final.sh fin_u32 > /dev/null 2>&1 || true
sed -i 's|    s32 expected_pos; /\* 0x80101EA0 \*/|    u32 expected_pos; /* 0x80101EA0 */|' $H/fin_u32/include/code6cac.h
grep -n "expected_pos; /\*" $H/fin_s32/include/code6cac.h $H/fin_u32/include/code6cac.h
bash $H/fullbuild.sh fin_s32 -j8
bash $H/fullbuild.sh fin_u32 -j8
n=0; d=0
for o in $H/fin_s32/build/src/*.o; do n=$((n+1)); cmp -s $o $H/fin_u32/build/src/$(basename $o) || { d=$((d+1)); echo "DIFF $(basename $o)"; }; done
echo "rev $REV: $n C objects compared (s32 vs u32 expected_pos), $d differ"
for f in code6cac_b4_post code6cac_b5; do
  mipsel-linux-gnu-objdump -dr $H/fin_s32/build/src/$f.o | grep -n "D_80101E58+0x48\|expected" | head -20
done
