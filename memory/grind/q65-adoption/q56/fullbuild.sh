#!/bin/bash
# fullbuild.sh <tag> <exclude-file>: fresh scratch copy of the pinned commit, exclude file swapped in,
# clean full build (make build/bb2.exe), SHA1 printed.
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
TAG="$1"; EX="$2"
D=/tmp/q56/full/$TAG
rm -rf "$D" && mkdir -p "$D"
cd "$REPO" && git archive "$(cat /tmp/q56/commit.txt)" | tar -x -C "$D"
cd "$D"
ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
cp "$EX" sdata_exclude.txt
source .venv/bin/activate
make -j8 build/bb2.exe > build.log 2>&1; rc=$?
echo "$TAG make_rc=$rc exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1) bin_sha1=$(sha1sum build/bb2.bin 2>/dev/null | cut -d' ' -f1)"
