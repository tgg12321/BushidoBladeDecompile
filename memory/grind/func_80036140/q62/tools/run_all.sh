#!/bin/bash
# usage: run_all.sh <variant> <body>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
V=$1; B=$2
python3 tmp/func_80036140/mk.py $V $B
D=tmp/func_80036140/v/$V
for s in code6cac_b5 code6cac_b4_post code6cac_b5_post; do
  python3 tmp/func_80036140/xb.py $s $D/$s.c --inc $D/inc --keep $D/$s.o | grep -v ': 0$'
done
