#!/bin/bash
# adopt_setup.sh <commit>: scratch CLONE of main (git clone --shared: main's object store is only read via
# alternates; NOT a worktree) at "/tmp/q56/adopt tree" (a path with a space, as the engine suite expects),
# checked out at <commit> on branch q56-adopt; toolchain symlinked; baseline clean build + engine test.
set -e
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C="$1"
A="/tmp/q56/adopt tree"
rm -rf "$A" /tmp/q56/adopt
git clone -q --shared --no-checkout "$REPO" "$A"
cd "$A"
git checkout -q -b q56-adopt "$C"
git config user.email q56@scratch && git config user.name q56-scratch
git tag -f step0 >/dev/null
echo "$C" > /tmp/q56/adopt_base.txt
mkdir -p tmp; ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
source .venv/bin/activate
make -j16 build/bb2.exe > /tmp/q56/adopt_base_build.log 2>&1 || { tail -5 /tmp/q56/adopt_base_build.log; exit 1; }
sha1sum build/bb2.exe
rm -rf /tmp/q56/objs_step00 && cp -r build/src /tmp/q56/objs_step00
