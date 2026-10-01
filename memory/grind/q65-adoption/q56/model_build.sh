#!/bin/bash
# model_build.sh: fresh scratch copy at /tmp/q56/model, apply model_setup.py, clean full build.
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
M=/tmp/q56/model
rm -rf $M && mkdir -p $M
cd "$REPO" && git archive "$(cat /tmp/q56/commit.txt)" | tar -x -C $M
cd $M
ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
source .venv/bin/activate
python3 "$REPO/tmp/q56/model_setup.py" || exit 1
make -k -j16 build/bb2.exe > build.log 2>&1; rc=$?
echo "make_rc=$rc exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)"
grep -E "Error|error:" build.log | head -20
