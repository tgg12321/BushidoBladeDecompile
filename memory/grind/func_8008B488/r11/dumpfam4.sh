#!/bin/bash
# Dumps + scored hunks for representative fam4 (cross-block FAKE write + chain-extender read) variants.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate
for x in sr_early_x_pos_c:f4pos sr_early_c_srrate_c:f4srcopy sr_early_x_load_c:f4load sr_to_rr_x_load_n:f4tork rr_early_c_rrrate_c:f4rrcopy; do
  v="tmp/f8b488s4/fam4/${x%%:*}.c"
  echo "######## ${x%%:*}"
  python3 tmp/f8b488s2/score.py -v "$v" | sed 's/^/  /'
  bash tmp/f8b488s4/dumpvar.sh "$v" "${x##*:}" | grep -vE "used_so_far|pass0_used|pass1_used|used2_noconflict|own_copy|own_full"
done
