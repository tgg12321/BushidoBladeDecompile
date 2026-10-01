#!/bin/bash
# perm_run.sh [seconds]: the permuter campaign from the full split body (perm_setup.sh first), 2 workers
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
P=tmp/func_80057E84/r11/perm
# control: perm_control.sh (the reuse body through the same scorer: base score 0)
timeout ${1:-1200} python3 tools/decomp-permuter/permuter.py $P -j 2 --stop-on-zero > $P/perm.log 2>&1
echo PERM-DONE
grep -E "base score|new best|score" $P/perm.log | head -5
ls $P | grep output
