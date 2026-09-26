#!/bin/bash
# usage: t_model.sh <name> [apply_model args...]  -> tree = HEAD + gate + model; full build; per-func scores
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
N=$1; shift
bash tmp/func_80036140/mktree.sh $N >/dev/null
python3 tmp/func_80036140/gate.py tmp/func_80036140/$N
python3 tmp/func_80036140/apply_model.py tmp/func_80036140/$N "$@"
bash tmp/func_80036140/fullbuild.sh $N -j8 || true
python3 tmp/func_80036140/scoretree.py $N
