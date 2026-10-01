#!/bin/bash
# filelists.sh: per step, the files it touches (name-status) in the scratch clone.
cd "/tmp/q56/adopt tree"
prev=step0
for n in 01 02 03 04 05 06 07 08 09 10 11 12; do
  echo "== step$n: $(git log -1 --format=%s step$n | cut -c1-100)"
  git diff --name-status $prev step$n | awk '{printf "%s %s  ", $1, $2} END {print ""}'
  git diff --shortstat $prev step$n
  prev=step$n
done
