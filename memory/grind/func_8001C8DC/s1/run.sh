#!/bin/bash
# run.sh <body.c> [--tail]: fresh scratch tree from HEAD, apply merge + body, full build, SHA1.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash tmp/c8dc/mktree.sh >/dev/null || exit 1
python3 tmp/c8dc/apply.py tmp/c8dc/tree "$@" || exit 1
bash tmp/c8dc/buildtree.sh 2>&1 | tail -3
