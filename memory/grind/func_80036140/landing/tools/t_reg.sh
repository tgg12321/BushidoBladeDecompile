#!/bin/bash
# tree 'reg' = HEAD + gate + registration; full build; objects vs base; maspsx tests; engine test in-tree
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
T=tmp/func_80036140/reg
REV=$(git rev-parse HEAD); bash tmp/func_80036140/mktree.sh reg $REV >/dev/null; bash tmp/func_80036140/mktree.sh base $REV >/dev/null; bash tmp/func_80036140/fullbuild.sh base -j8
git archive HEAD .claude/rules/integration-handoff-self-serve.md .claude/rules/maspsx-gate-lists.md tools/check_root_cleanliness.py tools/desync_audit.py tools/naming_wave.py tools/libscan tools/grinder/grindlib.py tools/hooks | tar -x -C $T
python3 tmp/func_80036140/gate.py $T && python3 tmp/func_80036140/register.py $T || exit 1
bash tmp/func_80036140/fullbuild.sh reg -j8
d=0; for o in tmp/func_80036140/base/build/src/*.o; do cmp -s "$o" "$T/build/src/$(basename $o)" || { d=$((d+1)); echo "DIFF $(basename $o)"; }; done; echo "objects differing vs stock: $d"
(cd $T/tools/maspsx && python3 -m unittest discover -s tests -t . 2>&1 | tail -1)
for f in engine/cheats.py engine/oracle.py engine/dossier.py engine/buildstamp.py engine/queue.py engine/buildconfig.py tools/check_root_cleanliness.py tools/desync_audit.py tools/naming_wave.py tools/libscan/manifest.py tools/grinder/grindlib.py tools/hooks/park_src_guard.py tools/hooks/main_reintegration_lock.py tools/hooks/worktree_contamination_guard.py; do python3 -m py_compile $T/$f || echo "COMPILE FAIL $f"; done
echo "py_compile done"
