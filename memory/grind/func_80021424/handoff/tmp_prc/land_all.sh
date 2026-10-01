#!/bin/bash
# land_all.sh : the whole L1 change as anchored edits (no patch context to go stale):
#   land.py --rows retire (merges, tail words) -> land2.py (unk_4A/unk_86 readers) -> land3.py (func_80021424 itself)
#   -> land_l1.py (func_8001FBE8 / func_80021A98 / func_80022F34, remaining members, extern/row retirements).
# Set L1_ROOT=<scratch tree> to run on a copy; unset = main (landing lock only).
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
python3 tmp/func_80021424/land.py --rows retire
python3 tmp/func_80021424/land2.py
python3 tmp/func_80021424/land3.py
BODY=${BODY:-r3} python3 tmp/prc/land_l1.py
