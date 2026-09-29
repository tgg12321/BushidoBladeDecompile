#!/bin/bash
# Dumps for the fam3 (same-iteration write + chain-extender read in another block) variants.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
for x in sr_w_load_n:f3load sr_w_srrate_n:f3srrate sr_w_rxx_n:f3rxx sr_wearly_load_n:f3early dr_w_load_n:f3dr; do
  bash tmp/f8b488s4/dumpvar.sh "tmp/f8b488s4/fam3/${x%%:*}.c" "${x##*:}"
done
