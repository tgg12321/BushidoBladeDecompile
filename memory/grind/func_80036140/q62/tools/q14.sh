#!/bin/bash
# Q14 re-check on the landed tree shape: expected_pos s32 vs u32 -> every consumer object byte-identical
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
T=tmp/func_80036140/q62
O=memory/grind/func_80036140/q62/q14_s32_vs_u32.txt
mkdir -p tmp/func_80036140/q14/inc
sed 's/    s32 expected_pos; \/\* 0x80101EA0 \*\//    u32 expected_pos; \/* 0x80101EA0 *\//' $T/include/code6cac.h > tmp/func_80036140/q14/inc/code6cac.h
{
echo "# expected_pos s32 (landed) vs u32: objects built from the scratch landed tree $T, header override for u32"
grep -c "u32 expected_pos" tmp/func_80036140/q14/inc/code6cac.h
for s in code6cac_b4_post code6cac_b5 code6cac_b5_post; do
  python3 tmp/func_80036140/xb.py $s $T/src/$s.c --inc $T/include --keep tmp/func_80036140/q14/$s.s32.o >/dev/null
  python3 tmp/func_80036140/xb.py $s $T/src/$s.c --inc tmp/func_80036140/q14/inc --keep tmp/func_80036140/q14/$s.u32.o >/dev/null
  if cmp -s tmp/func_80036140/q14/$s.s32.o tmp/func_80036140/q14/$s.u32.o; then echo "$s: identical"; else echo "$s: DIFFER"; fi
done
} > $O
cat $O
