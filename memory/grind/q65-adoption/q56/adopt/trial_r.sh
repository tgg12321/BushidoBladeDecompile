#!/bin/bash
# trial_r.sh <tag> <apply.py> <merge-group>: scratch clone only, nothing committed. At <tag>: build (objects kept),
# apply the reconciliation script, clean build (oracle?), compare every object; then merge the group verbatim
# (M3 or M4) and compile the merged file, listing what the compiler still reports. Restores the branch head.
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
A="/tmp/q56/adopt tree"; TAG="$1"; AP="$2"; GRP="$3"
cd "$A" && HEAD_REF=$(git rev-parse HEAD) && git checkout -q "$TAG" && source .venv/bin/activate
rm -rf build && make -j16 build/bb2.exe >/tmp/q56/tr0.log 2>&1
rm -rf /tmp/q56/tr_before && cp -r build/src /tmp/q56/tr_before
python3 "$H/$AP" "$A" || { echo APPLY FAILED; git checkout -q -- . ; git clean -qfd -e tmp -e build; git checkout -q q56-adopt; git reset -q --hard "$HEAD_REF"; exit 1; }
rm -rf build && make -j16 build/bb2.exe >/tmp/q56/tr1.log 2>&1
echo "reconciled build: $(sha1sum build/bb2.exe 2>/dev/null | cut -c1-40)"
grep -E "error|Error" /tmp/q56/tr1.log | head -20
for o in /tmp/q56/tr_before/*.o; do b=$(basename $o); [ -f build/src/$b ] || { echo "  missing $b"; continue; }
  cmp -s $o build/src/$b || { echo "  differs: $b"; python3 "$H/objdiff.py" $o build/src/$b 2>&1 | head -12; }; done
if [ "$GRP" = "M3" ]; then
  python3 "$H/mergec.py" src/text1b.c src/text1a_c2.c src/text1a_b.c src/text1a_b_pre_rodata.c src/sound.c src/text1b.c; F=text1b
elif [ "$GRP" = "M4" ]; then
  python3 "$H/mergec.py" src/text1b_b.c src/text1b_tu2.c src/text1a_b_mid_rodata.c src/text1b_b.c; F=text1b_b
fi
if [ -n "$GRP" ]; then
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C src/$F.c 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/null 2>&1 | head -40
  cp src/$F.c /tmp/q56/merged_$GRP.c
fi
git checkout -q -- . && git clean -qfd -e tmp -e build && git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
