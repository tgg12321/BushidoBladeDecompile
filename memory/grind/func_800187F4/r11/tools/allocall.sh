#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for t in "$@"; do echo "== $t"; bash tmp/func_800187F4/alloc2.sh $t; done
