#!/bin/bash
# mkclone.sh: private scratch clone for func_800770B8 work (/tmp/l770/tree), from the Q65 scratch clone's step16
# (read-only use of that clone: git clone --shared). Adds the SelWork f1C/f20 union word views, builds.
set -e
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
A=/tmp/l770/tree
rm -rf /tmp/l770; mkdir -p /tmp/l770
git clone -q --shared --no-checkout "/tmp/q56/adopt tree" "$A"
cd "$A"
git checkout -q -b l770 step16
git config user.email l770@scratch && git config user.name l770-scratch
mkdir -p tmp; ln -s "$REPO/tools/gcc-2.7.2" tools/gcc-2.7.2; ln -s "$REPO/.venv" .venv; ln -s "$REPO/disc" disc
printf '/.venv\n/disc\n/tools/gcc-2.7.2\n/tmp\n' >> .git/info/exclude
source .venv/bin/activate
python3 - <<'PY'
p = "include/game.h"
t = open(p).read()
a = "    s16 f1C[2];\n    s16 f20[2];\n"
b = ("    union {\n        s16 half[2];\n        s32 word;\n    } f1C;\n"
     "    union {\n        s16 half[2];\n        s32 word;\n    } f20;\n")
assert t.count(a) == 1
open(p, "w", newline="\n").write(t.replace(a, b))
PY
sed -i 's/->f1C\[/->f1C.half[/g; s/->f20\[/->f20.half[/g' src/text1b_b.c
make -j16 build/bb2.exe > /tmp/l770/build.log 2>&1
sha1sum build/bb2.exe
git add -A include src && git commit -qm "l770: SelWork f1C/f20 unions" && git log -1 --format=%h
