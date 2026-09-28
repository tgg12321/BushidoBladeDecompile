#!/bin/bash
# launch the two admission-record permuter campaigns (2 jobs each)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tools/permuter_campaign.py launch --func func_8006C21C --dir tmp/c21c/perm_lvl10 --label r11-level-split -j 2
python3 tools/permuter_campaign.py launch --func func_8006C21C --dir tmp/c21c/perm_nocells10 --label r9-cells-free -j 2
