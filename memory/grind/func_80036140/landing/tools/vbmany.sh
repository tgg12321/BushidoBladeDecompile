#!/bin/bash
# usage: vbmany.sh <tree> <G8|G0|both> name1 name2 ...   (variants tmp/func_80036140/v/<name>.c)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=$1; G=$2; shift 2
for v in "$@"; do
  for g in $( [ "$G" = both ] && echo G8 G0 || echo $G ); do
    printf '%-22s %s  ' "$v" "$g"; bash tmp/func_80036140/vb.sh $T tmp/func_80036140/v/$v.c $g | tail -1
  done
done
