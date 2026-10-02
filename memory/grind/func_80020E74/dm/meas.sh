#!/bin/bash
# usage: meas.sh <dir...>  -- per-function compare + private full link for each data-model dir
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for d in "$@"; do
  echo "== $d"
  rm -rf tmp/func_80020E74/$d/obj
  python3 tmp/func_80020E74/dm.py tmp/func_80020E74/$d code6cac_tu2 code6cac ings 2>&1 | grep -v "funcs compared"
  python3 tmp/func_80020E74/plink.py tmp/func_80020E74/$d 2>&1 | tail -2
done
