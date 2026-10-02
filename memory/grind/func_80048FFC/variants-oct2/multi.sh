#!/bin/bash
# multi.sh <G> <variant.c>... : score each func_80048FFC variant (diff lines vs reference); -v prints hunks
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
G=$1; shift
V=0; if [ "$1" = "-v" ]; then V=1; shift; fi
for f in "$@"; do
  out=$(bash tmp/camera_CalcAngles/g8/g8fn.sh $G "$f" - 2>/dev/null)
  echo "$f: $(echo "$out" | grep '^func_80048FFC')"
  [ $V = 1 ] && echo "$out" | sed -n '2,40p'
done
