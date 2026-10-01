#!/bin/bash
# mdumps.sh <variant>...: mdump.sh for each tmp/func_800770B8/<variant>.c
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
for v in "$@"; do bash tmp/func_800770B8/mdump.sh "$v" "tmp/func_800770B8/$v.c"; done
