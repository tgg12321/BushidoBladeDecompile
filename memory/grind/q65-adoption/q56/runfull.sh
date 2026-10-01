#!/bin/bash
# runfull.sh: the five full clean builds in parallel.
rm -rf /tmp/q56/full
for t in all_removed cleaned cleaned_trim r5 r18; do
  bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/fullbuild.sh" "$t" "/tmp/q56/variants/$t.txt" > "/tmp/q56/full_$t.out" 2>&1 &
done
wait
cat /tmp/q56/full_*.out
