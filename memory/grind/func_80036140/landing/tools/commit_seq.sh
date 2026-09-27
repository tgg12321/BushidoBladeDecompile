#!/bin/bash
# AFTER a layer-2 PASS (lock held): commit the five snapshots in order, each with explicit pathspecs.
# The worktree ends in the final state (snapshot E == the landed tree).
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
H=tmp/func_80036140; S=$H/snap
git restore --staged -- $(cat $S/A.files $S/B.files $S/C.files $S/D.files $S/E.files | sort -u) 2>/dev/null || true
for st in A B C D E; do
  msg=$(ls $H/msg_${st}_*.txt)
  files=$(cat $S/$st.files)
  for f in $files; do cp "$S/$st/$f" "$f"; done
  git add -- $files
  git commit -q -F "$msg" -- $files
  echo "$st: $(git log -1 --format='%h %s')"
  git show --stat --format= HEAD | tail -1
done
git status --short | grep -v "^?? tmp/" || true
