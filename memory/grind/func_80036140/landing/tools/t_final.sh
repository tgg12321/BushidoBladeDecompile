#!/bin/bash
# The full landing on a scratch tree of HEAD (or $REV): gate + registration + model (final) + symbol rows.
# usage: t_final.sh <name>
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
N=$1; T=tmp/func_80036140/$N; REV=${REV:-HEAD}
bash tmp/func_80036140/mktree.sh $N $REV >/dev/null
git archive $REV tools .claude docs | tar -x -C $T; mkdir -p $T/tmp $T/memory
python3 tmp/func_80036140/gate.py $T
python3 tmp/func_80036140/register.py $T
python3 tmp/func_80036140/apply_model.py $T --split final --merge --ext rec
python3 tmp/func_80036140/symfiles.py $T
bash tmp/func_80036140/fullbuild.sh $N -j8 || true
python3 tmp/func_80036140/scoretree.py $N
