#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash tmp/c8dc2/mktree.sh >/dev/null || exit 1
python3 tmp/c8dc2/apply_array.py tmp/c8dc2/tree "$@" || exit 1
bash tmp/c8dc2/buildtree.sh 2>&1 | tail -3
python3 tmp/c8dc2/bindiff.py | head -8
