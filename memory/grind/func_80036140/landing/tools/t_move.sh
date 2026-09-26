#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
REV=$(cd tmp/func_80036140/fin && git -C ../../.. rev-parse HEAD)
bash tmp/func_80036140/mktree.sh inplace ${REV:-HEAD} >/dev/null
python3 tmp/func_80036140/apply_model.py tmp/func_80036140/inplace --split none --merge --ext rec
python3 tmp/func_80036140/movecheck.py tmp/func_80036140/fin tmp/func_80036140/inplace ${REV:-HEAD}
