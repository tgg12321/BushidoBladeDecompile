#!/bin/bash
# fix_symlinks.sh: scratch clone only — keep the toolchain symlinks out of the scratch commits, re-export 01.
set -e
A="/tmp/q56/adopt tree"; OUT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
cd "$A"
printf '/.venv\n/disc\n/tools/gcc-2.7.2\n/tmp\n' >> .git/info/exclude
git rm -q --cached .venv disc tools/gcc-2.7.2
git -c core.hooksPath=/dev/null commit -q --amend --no-edit
git tag -f step01 >/dev/null
git format-patch -1 --stdout > "$OUT/01-maspsx-indexed-operand-gp.patch"
git show --stat --format= HEAD
cat /tmp/q56/step01.maspsx.log | grep -E "^(FAIL|ERROR)"
# baseline: same unit tests on step0's maspsx
git stash -q 2>/dev/null || true
git checkout -q step0 -- tools/maspsx
(cd tools/maspsx && python3 -m unittest discover -s tests -t . 2>&1 | grep -E "^(FAIL|ERROR)|^Ran|^FAILED|^OK")
git checkout -q HEAD -- tools/maspsx
git status --short | head
