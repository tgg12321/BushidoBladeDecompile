#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash tmp/c8dc/mktree.sh >/dev/null || exit 1
python3 tmp/f75f80/apply.py tmp/c8dc/tree "$1" || exit 1
python3 tmp/f75f80/apply_arr.py tmp/c8dc/tree || exit 1
python3 tmp/f759d0/apply2d.py tmp/c8dc/tree "$2" || exit 1
bash tmp/c8dc/buildtree.sh 2>&1 | tail -3
