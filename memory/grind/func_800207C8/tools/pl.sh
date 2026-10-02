#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
rm -rf tmp/func_800207C8/pl; mkdir -p tmp/func_800207C8/pl/obj
for s in code6cac_tu2 code6cac_b_tu2 code6cac; do cp tmp/func_800207C8/chk/$s/$s.o tmp/func_800207C8/pl/obj/; done
python3 tmp/func_800207C8/plink.py tmp/func_800207C8/pl
