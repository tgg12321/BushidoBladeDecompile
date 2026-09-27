#!/bin/bash
# Each committed state A..E == the scratch state proven at the oracle (build-input files of all five steps).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
H=tmp/func_80036140
FILES=$(cat $H/snap/{A,B,C,D,E}.files | sort -u)
set -- 014401021 stA 90e325bba stB cbc69187b stC a6daf00b6 stD f601fc2ae stE
while [ $# -gt 0 ]; do
  c=$1; st=$2; shift 2; bad=0
  for f in $FILES; do
    a=$(git show $c:$f 2>/dev/null | sha1sum | cut -c1-40)
    if [ -f $H/$st/$f ]; then b=$(sha1sum < $H/$st/$f | cut -c1-40); else b=$(printf '' | sha1sum | cut -c1-40); fi
    [ "$a" = "$b" ] || { bad=$((bad+1)); echo "  $c $f differs from $st"; }
  done
  echo "$c vs $st: $bad differing files"
done
