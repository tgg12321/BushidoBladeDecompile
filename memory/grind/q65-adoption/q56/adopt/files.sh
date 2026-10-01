#!/bin/bash
# files.sh: after series.sh - per-step touched files (files.md) and the series base/SHA record (series_base.txt).
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
cd "/tmp/q56/adopt tree" || exit 1
{
  echo "# Files touched per step (base $(git rev-parse --short step0), $(date -u +%Y-%m-%d))"; echo
  for n in $(seq -w 1 16); do
    t="step$n"; git rev-parse -q --verify "$t" >/dev/null || continue
    echo "- **$n** $(git log -1 --format=%s $t | cut -c1-90)"
    echo "  $(git show --name-only --format= $t | tr '\n' ' ')"
  done
} > "$H/files.md"
{ echo "base $(git rev-parse step0) ($(date -u +%Y-%m-%dT%H:%MZ))"
  for n in $(seq -w 1 16); do echo "step$n $(git rev-parse step$n)"; done
  echo "exe sha1 at step16: $(sha1sum build/bb2.exe | cut -c1-40)"; } > "$H/series_base.txt"
cat "$H/series_base.txt"
