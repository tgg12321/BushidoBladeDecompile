#!/bin/bash
# setup.sh <commit>: scratch copy of <commit> at /tmp/q56/tree, toolchain symlinked, full baseline build.
set -e
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C="${1:-HEAD}"
rm -rf /tmp/q56/tree && mkdir -p /tmp/q56/tree
cd "$REPO"
git archive "$C" | tar -x -C /tmp/q56/tree
git rev-parse "$C" > /tmp/q56/commit.txt
cd /tmp/q56/tree
ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2
ln -s "$REPO/.venv" .venv
ln -s "$REPO/disc" disc
source .venv/bin/activate
make -j16 build/bb2.exe > /tmp/q56/baseline.log 2>&1 || { tail -20 /tmp/q56/baseline.log; exit 1; }
sha1sum build/bb2.exe build/bb2.bin | tee /tmp/q56/baseline.sha1
cp build/bb2.bin /tmp/q56/ref.bin
mkdir -p /tmp/q56/refobj && cp build/src/*.o /tmp/q56/refobj/
