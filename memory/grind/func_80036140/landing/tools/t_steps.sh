#!/bin/bash
# The four-commit sequence on scratch trees of one rev: each state built from clean, SHA1 printed.
# usage: t_steps.sh [rev]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; REV=${1:-$(git rev-parse HEAD)}; echo "rev $REV"
mk() {  # name, steps...
  local N=$1; shift; local T=$H/$N
  bash $H/mktree.sh $N $REV >/dev/null
  git archive $REV tools .claude docs | tar -x -C $T; mkdir -p $T/tmp $T/memory
  python3 $H/gate.py $T && python3 $H/register.py $T || return 1
  for s in "$@"; do python3 $H/apply2.py $T $s || return 1; done
  printf '%-8s ' $N; bash $H/fullbuild.sh $N -j8
}
bash $H/mktree.sh st0 $REV >/dev/null; printf '%-8s ' st0; bash $H/fullbuild.sh st0 -j8
mk stA
d=0; for o in $H/st0/build/src/*.o; do cmp -s $o $H/stA/build/src/$(basename $o) || { d=$((d+1)); echo "  DIFF $(basename $o)"; }; done
echo "  stA vs stock: $(ls $H/st0/build/src/*.o | wc -l) objects, $d differ"
mk stB split
mk stC split merge
mk stD split merge match
mk stE split merge match cdres
python3 $H/scoretree.py stD; python3 $H/scoretree.py stE
