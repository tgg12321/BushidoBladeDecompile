#!/bin/bash
# Simulate laneH's unk_6A landing first, then land_l1.py; the result must equal the measured l1 header.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/prc/hfirst_tree
rm -rf $T && mkdir -p $T/src $T/include
cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
python3 - <<'EOF'
from pathlib import Path
p = Path("tmp/prc/hfirst_tree/include/code6cac.h")
s = p.read_bytes().decode()
a = "    u8  unk_60[0x72 - 0x60];\n"
assert s.count(a) == 1
s = s.replace(a, "    u8  unk_60[0x6A - 0x60];\n    u16 unk_6A;\n    u8  unk_6C[0x72 - 0x6C];\n")
p.write_bytes(s.encode())
EOF
git apply -C1 --directory=$T --unsafe-paths tmp/func_80021424/round3_full.patch
L1_ROOT=$T BODY=r3 python3 tmp/prc/land_l1.py
diff $T/include/code6cac.h tmp/prc/l1_tree/include/code6cac.h && echo "laneH-first rebase: header identical to the measured one"
