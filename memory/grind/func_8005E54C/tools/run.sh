#!/bin/bash
# run.sh <candidate> <outdir> [patch.py] [dump]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
export PATCH=${3:-}
export DUMP=${4:-}
bash tmp/func_8005E54C/dump.sh "$1" "$2"
echo "rc=$?"
