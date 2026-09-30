#!/bin/bash
# run.sh [<body.c> [--tail]]: fresh scratch tree from HEAD; with a body, apply the struct model + body;
# full clean-driver build; print the EXE SHA1. No args = unmodified baseline (harness check).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash tmp/c8dc2/mktree.sh >/dev/null || exit 1
if [ "$#" -gt 0 ]; then python3 tmp/c8dc2/apply_struct.py tmp/c8dc2/tree "$@" || exit 1; fi
bash tmp/c8dc2/buildtree.sh 2>&1 | tail -3
