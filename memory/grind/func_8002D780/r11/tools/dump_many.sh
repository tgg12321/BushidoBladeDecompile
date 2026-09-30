#!/bin/bash
# dump_many.sh <variant-dir> <name>... : dump_sched.sh each <variant-dir>/<name>.c into tmp/func_8002D780/d_<name>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
dir="$1"; shift
for v in "$@"; do
  bash tmp/func_8002D780/dump_sched.sh "$dir/$v.c" "tmp/func_8002D780/d_$v" > /dev/null || echo "FAIL $v"
done
echo done
