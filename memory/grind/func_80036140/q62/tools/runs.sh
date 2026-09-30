#!/bin/bash
# Re-run every necessity variant (Q62 model, HEAD headers + mk.py edits) and bank the scores.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
O=memory/grind/func_80036140/q62/runs.txt
{
echo "# func_80036140 necessity re-measurement under the Q62 global COMMON model ($(date -I), HEAD $(git rev-parse --short HEAD))"
echo "# scratch whole-file builds on each file's exact Makefile recipe (q62/tools/xb.py), scored per function vs build/src/<stem>.o"
echo "# body: q62/body_q62.c (= candidate.c) with the E9C/EA4 spelling each variant needs (q62/tools/mk.py, mkbodies.py)"
for vb in "ext body_q62" "ext48 body_q62" "ext4C body_q62" "sep body_q62" "sep body_ptr" "sep body_alias" "sep12 body_q62"; do
  set -- $vb
  echo "== variant $1, body $2"
  bash tmp/func_80036140/run_all.sh $1 tmp/func_80036140/$2.c 2>&1 | grep -v "^wrote"
done
} > $O 2>&1
cat $O
