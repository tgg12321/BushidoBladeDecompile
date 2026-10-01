#!/bin/bash
# t234.sh: re-run s02/s03/s04 on the fullchain clone's f01..f03 states (scratch copy), print their bodies.
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=/tmp/q56r2/t234; rm -rf $T; git clone -q --shared /tmp/q56r2/f $T || exit 1
cd $T; printf '/.venv
/disc
/tools/gcc-2.7.2
' >> .git/info/exclude; ln -s "$R/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$R/.venv" .venv; ln -s "$R/disc" disc
for n in 02 03 04; do
  p=$(printf "%02d" $((10#$n - 1)))
  c=$(git -C /tmp/q56r2/f log --format=%H --grep="^f$p$" f | head -1)
  git switch -q --detach $c && git clean -qfd -e .venv -e disc -e tools/gcc-2.7.2
  python3 "$R/tmp/q56/adopt/s${n}_apply.py" $T > /tmp/q56r2/t$n.log 2>&1 || { echo "s$n FAILED"; tail -5 /tmp/q56r2/t$n.log; }
  git diff --stat | tail -1
  git -C /tmp/q56r2/f diff --stat $c $(git -C /tmp/q56r2/f log --format=%H --grep="^f$n$" f | head -1) | tail -1
  git stash -q -u 2>/dev/null; git stash drop -q 2>/dev/null
done
