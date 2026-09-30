#!/bin/bash
# campaign 1 harvest + campaign 2 (fresh workspace = fresh seed, no --stop-on-zero) launch
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tools/permuter_campaign.py harvest --dir tmp/func_8002D780/perm_pv --stop --reason "stopped on zero (dead-local find, Q30 set-aside); campaign 2 continues" | tail -20
bash tmp/func_8002D780/mkperm.sh func_8002D780 tmp/func_8002D780/r11/pv_all.c tmp/func_8002D780/perm_pv2 | tail -3
python3 tools/permuter_campaign.py launch --func func_8002D780 --dir tmp/func_8002D780/perm_pv2 --label d780-pv-all-2 -j 2 | tail -6
