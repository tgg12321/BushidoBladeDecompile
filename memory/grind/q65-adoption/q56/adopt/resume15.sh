#!/bin/bash
# resume15.sh <main-commit>: scratch clone back to step14 (clean), then series from step 15 (run_from.sh).
cd "/tmp/q56/adopt tree" || exit 1
git reset -q --hard step14 && git clean -qfd || exit 1
git describe --tags --exact-match HEAD
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/run_from.sh" "$1" 15
