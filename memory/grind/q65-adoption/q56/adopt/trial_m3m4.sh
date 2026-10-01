#!/bin/bash
# trial_m3m4.sh [tag]: scratch clone only - at <tag> (default step07), merge the Q67 groups verbatim WITHOUT
# committing and list every declaration conflict the compiler reports (both declarations), then restore.
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
A="/tmp/q56/adopt tree"; TAG="${1:-step07}"
cd "$A" && HEAD_REF=$(git rev-parse HEAD) && git checkout -q "$TAG" && source .venv/bin/activate
python3 "$H/mergec.py" src/text1b.c src/text1a_c2.c src/text1a_b.c src/text1a_b_pre_rodata.c src/sound.c src/text1b.c
python3 "$H/mergec.py" src/text1b_b.c src/text1b_tu2.c src/text1a_b_mid_rodata.c src/text1b_b.c
for f in text1b text1b_b; do
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C src/$f.c 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/null 2>&1 \
   | sed "s#^#src/$f.c:#" | sed -E 's#^src/[a-z0-9_]+\.c:(src/)#\1#'
done > /tmp/q56/trial_m3m4.log
grep -c "" /tmp/q56/trial_m3m4.log
python3 "$H/conflicts.py" /tmp/q56/trial_m3m4.log > "$H/conflicts_M3M4.txt"
grep -vE "conflicting types|previous declaration" /tmp/q56/trial_m3m4.log | head -20
git checkout -q -- . && git clean -qfd -e tmp -e build && git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
grep -c "prev L" "$H/conflicts_M3M4.txt"
