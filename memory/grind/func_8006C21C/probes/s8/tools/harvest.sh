#!/bin/bash
# usage: harvest.sh [--stop] : harvest both admission-record campaigns
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for d in perm_lvl10 perm_nocells10; do
  echo "== $d"
  python3 tools/permuter_campaign.py harvest --dir tmp/c21c/$d "$@" --reason "admission-record campaign (R11 D4 / R9 i), time-boxed" | grep -E '"elapsed_s"|"iterations"|"best_new_score"|"stopped"|"score"' | head -12
done
