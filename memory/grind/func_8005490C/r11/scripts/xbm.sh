#!/bin/bash
# usage: xbm.sh body1.c body2.c ...  -> FOCUS score per body (minimal A applied); nonzero-score other diffs flagged
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
for b in "$@"; do
  t=$(basename $b .c)
  r=$(bash tmp/func_8005490C/xb.sh $b m_$t 2>&1)
  f=$(echo "$r" | grep FOCUS | sed 's/FOCUS func_8005490C //')
  o=$(echo "$r" | grep "DIFF" | grep -v "func_8005490C\|'score': 0," | tr '\n' ' ')
  e=$(echo "$r" | grep -i "fail\|error" | head -3 | tr '\n' ' ')
  echo "$t: $f $o $e"
done
