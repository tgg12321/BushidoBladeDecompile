#!/bin/bash
# classify ASPSX-vs-shipped differences for the calibration objects
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
for tag in OURS_b5comm b5_comm-G8; do
  for f in func_80036140 func_80036940; do
    printf '%-14s ' $tag; python3 tmp/func_80036140/wdiff.py tmp/func_80036140/calib/$tag/code6cac_b5-G8.obj $f 0 --classify
  done
done
