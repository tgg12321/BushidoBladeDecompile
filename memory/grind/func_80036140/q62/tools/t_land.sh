#!/bin/bash
# scratch tree at HEAD + land.py + full clean build
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
N=${1:-q62}; B=${2:-tmp/func_80036140/body_q62.c}
bash memory/grind/func_80036140/landing/tools/mktree.sh $N
python3 tmp/func_80036140/land.py tmp/func_80036140/$N $B
bash memory/grind/func_80036140/landing/tools/fullbuild.sh $N
