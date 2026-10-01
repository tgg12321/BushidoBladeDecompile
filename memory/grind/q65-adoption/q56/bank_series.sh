#!/bin/bash
# bank_series.sh: record the scratch adoption series (commits + clean-build SHA1 at its head) for citation.
O="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt_series_oracle.txt"
cd "/tmp/q56/adopt tree" || exit 1
source .venv/bin/activate
{
  echo "# Q65 adoption scratch series, banked $(date -u +%Y-%m-%dT%H:%MZ)"
  echo "# base main commit: $(git rev-parse step0)"
  echo "# scratch clone: /tmp/q56/adopt tree (git clone --shared, not a worktree), branch q56-adopt"
  for t in $(git tag -l 'step[0-9][0-9]' | sort); do
    echo "$t $(git rev-parse --short "$t") $(git log -1 --format=%s "$t" | cut -c1-110)"
  done
  rm -rf build
  make -j16 build/bb2.exe > /tmp/q56/bank_build.log 2>&1
  echo "full clean build at $(git rev-parse --short HEAD) (real definitions, no sdata lists): exe $(sha1sum build/bb2.exe | cut -d' ' -f1)"
  echo "bb2.bin $(sha1sum build/bb2.bin | cut -d' ' -f1)"
  echo "oracle  62efab4f73f992798c43e8c730aa43baa10bb4fa"
} > "$O"
cat "$O"
