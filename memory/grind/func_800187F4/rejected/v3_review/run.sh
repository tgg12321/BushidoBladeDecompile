#!/bin/bash
# run.sh <tagsfile> <outfile> : measure tags in 6 parallel lanes
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=$1; O=$2
split -n l/6 -d $T tmp/rv3/chunk_
for f in tmp/rv3/chunk_0*; do (bash tmp/func_800187F4/fast3.sh $(cat $f) > $f.out 2>&1) & done
wait
cat tmp/rv3/chunk_0*.out > $O
rm -f tmp/rv3/chunk_0*
sort -t= -k2 -n $O | awk '{print $2}' | sort | uniq -c | sort -k2 -t= -n | head -5
