#!/bin/bash
# usage: MODEL=model3 bash tmp/func_80036940/many.sh [xb flags --] name1 name2 ...   (files tmp/func_80036940/v/<name>.c)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
M=tmp/func_80036940/${MODEL:-model3}
FL=()
if [[ " $* " == *" -- "* ]]; then while [[ "$1" != "--" ]]; do FL+=("$1"); shift; done; shift; fi
for f in "$@"; do printf '%-16s ' "$f"; python3 tmp/func_80036940/xb.py tmp/func_80036940/v/$f.c --base $M/src/code6cac_b2_post.c --inc $M/include --tag v_$f "${FL[@]}"; done
