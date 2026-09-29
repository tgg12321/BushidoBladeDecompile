#!/bin/bash
# Dumps for the escape variants cited in proof.md (D)(3).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
for x in fam/sr_chain_ior:srch fam2/sl2_sr_ior:sl2sr fam2/both2_sr_ior:both2sr fam/sl_chain_in_sr:slchsr fam/sl_dead_in_sr:sldeadsr fam/sl_self_clamp:slself; do
  bash tmp/f8b488s4/dumpvar.sh "tmp/f8b488s4/${x%%:*}.c" "${x##*:}"
done
