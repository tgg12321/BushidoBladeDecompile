#!/bin/bash
# engine test in a scratch tree = HEAD (full tools/ + engine/ + .claude/ + docs/) + gate + registration.
# Compares against the same suite on a stock tree of the same rev.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
REV=$(git rev-parse HEAD)
for N in ${ONLY:-eng_stock eng_reg}; do
  T=tmp/func_80036140/$N
  bash tmp/func_80036140/mktree.sh $N $REV >/dev/null
  git archive $REV tools .claude docs | tar -x -C $T; mkdir -p $T/tmp $T/memory
  if [ $N = eng_reg ]; then python3 tmp/func_80036140/gate.py $T && python3 tmp/func_80036140/register.py $T || exit 1; fi
  bash tmp/func_80036140/fullbuild.sh $N -j8
  (cd $T && timeout 1500 python3 -m engine.cli test > engtest.log 2>&1; echo "$N rc=$?"; tail -3 engtest.log)
done
diff <(grep -E "FAIL|passed|failed" tmp/func_80036140/eng_stock/engtest.log) <(grep -E "FAIL|passed|failed" tmp/func_80036140/eng_reg/engtest.log) && echo "SAME RESULT SET"
