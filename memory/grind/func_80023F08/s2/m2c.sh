#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
python3 tools/m2c/m2c.py --valid-syntax asm/funcs/func_80023F08.s > tmp/func_80023F08/m2c.c 2> tmp/func_80023F08/m2c.err
echo rc=$?
