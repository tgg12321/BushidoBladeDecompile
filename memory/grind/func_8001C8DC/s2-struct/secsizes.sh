#!/bin/bash
# secsizes.sh: section sizes of the four touched objects, array tree (saved) vs struct tree (rebuilt).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
rm -rf tmp/c8dc2/arr_o; mkdir -p tmp/c8dc2/arr_o
cp tmp/c8dc2/tree/build/src/*.o tmp/c8dc2/arr_o/ 2>/dev/null || find tmp/c8dc2/tree/build -name '*.o' | head
bash tmp/c8dc2/run.sh tmp/c8dc2/cand_s.c --tail
for o in code6cac_tu2 code6cac_b_tu2 code6cac_c2 text1b; do
  echo "== $o"
  diff <(mipsel-linux-gnu-objdump -h tmp/c8dc2/arr_o/$o.o | awk '/^ +[0-9]/{print $2, $3}') \
       <(mipsel-linux-gnu-objdump -h tmp/c8dc2/tree/build/src/$o.o | awk '/^ +[0-9]/{print $2, $3}')
done
