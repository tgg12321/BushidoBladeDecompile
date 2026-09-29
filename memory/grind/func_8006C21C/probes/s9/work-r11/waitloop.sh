#!/bin/bash
# usage: waitloop.sh <dir> <seconds> : repeat permuter_campaign wait until <seconds> have elapsed
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
end=$(( $(date +%s) + $2 ))
while [ $(date +%s) -lt $end ]; do
  left=$(( end - $(date +%s) ))
  m=$(python3 -c "print(max(0.1, $left/60))")
  python3 tools/permuter_campaign.py wait --dir "$1" --timeout-min $m > /tmp/c21c9_wait.json
  grep -q '"reason": "dead"' /tmp/c21c9_wait.json && { echo dead; break; }
done
grep -E '"iterations"|pid_alive' /tmp/c21c9_wait.json
ls "$1" | grep output | sort -t- -k2 -n | head -3
