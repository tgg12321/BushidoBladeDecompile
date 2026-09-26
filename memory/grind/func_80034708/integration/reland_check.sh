#!/bin/bash
# Regenerate the full landing from the live tree, build every TU with the re-landing memberships
# (b3: GP + EXPAND_LB + RODATA_ALIGN2; b3_post: EXPAND_LB + RODATA_ALIGN2), relink, score.
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tmp/func_80034708/mk_integ3.py >/dev/null && python3 tmp/func_80034708/mk_cfg.py >/dev/null || exit 1
rm -rf tmp/func_80034708/integ/build
RELAND=1 python3 tmp/func_80034708/integ.py $(cat tmp/func_80034708/allstems.txt) code6cac_b3 code6cac_b3_post 2>&1 \
  | grep -v "warning:\|^ *|\|^ *[0-9]* |\|Assembler\|unterminated\|^\s*$\|^\s\+CHANGED\|^\s\+functions=" | grep -v "identical=\([0-9]*\) changed=0"
bash tmp/func_80034708/fullbuild.sh 2>&1 | tail -1
python3 tmp/func_80034708/check3.py 2>&1 | head -1
