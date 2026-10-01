#!/bin/bash
# mkmain.sh: private clone of MAIN's HEAD (/tmp/l770m/tree; git clone --shared, read-only use of main's objects)
# with the SelWork f1C/f20 union word views and text1b_tu2.c's other f1C/f20 element reads respelled .half[...].
set -e
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
A=/tmp/l770m/tree
rm -rf /tmp/l770m; mkdir -p /tmp/l770m
git clone -q --shared --no-checkout "$REPO" "$A"
cd "$A"
git checkout -q -b l770m "$(git -C "$REPO" rev-parse HEAD)"
git config user.email l770@scratch && git config user.name l770-scratch
mkdir -p tmp; ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
printf '/.venv\n/disc\n/tools/gcc-2.7.2\n/tmp\n' >> .git/info/exclude
source .venv/bin/activate
python3 "$REPO/tmp/func_800770B8/unions.py" "$A"
make -j16 build/bb2.exe > /tmp/l770m/build.log 2>&1
sha1sum build/bb2.exe
git add -A include src && git commit -qm "l770m: SelWork f1C/f20 unions" && git log -1 --format=%h
