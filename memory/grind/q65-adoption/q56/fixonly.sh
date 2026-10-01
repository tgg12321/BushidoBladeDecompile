#!/bin/bash
# (1) baseline list model + only the maspsx indexed-operand _uses_gp fix: byte-neutral?
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=/tmp/q56/full/fixonly; rm -rf $D; mkdir -p $D
cd "$REPO" && git archive "$(cat /tmp/q56/commit.txt)" | tar -x -C $D
cd $D; ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
cp /tmp/q56/model/tools/maspsx/maspsx/__init__.py tools/maspsx/maspsx/__init__.py
# that copy also carries the poc_noncomm hook, which is inert without the flag
source .venv/bin/activate
make -j8 build/bb2.exe > build.log 2>&1; echo "fixonly make_rc=$? exe_sha1=$(sha1sum build/bb2.exe | cut -d' ' -f1)"
