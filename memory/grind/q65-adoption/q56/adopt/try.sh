#!/bin/bash
# try.sh NN: scratch clone only - apply sNN_apply.py WITHOUT committing, build with -k, report.
NN="$1"
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; A="/tmp/q56/adopt tree"
cd "$A" && source .venv/bin/activate
python3 "$REPO/tmp/q56/adopt/s${NN}_apply.py" "$A" > /tmp/q56/try$NN.apply.log 2>&1; echo "apply rc=$?"; tail -5 /tmp/q56/try$NN.apply.log
rm -rf build
make -k -j16 build/bb2.exe > /tmp/q56/try$NN.build.log 2>&1; echo "make rc=$?"
echo "exe_sha1=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)"
grep -E "src/\w+\.c:[0-9]+: |Error|error:|undefined reference|multiple definition" /tmp/q56/try$NN.build.log | grep -v warning | head -30
