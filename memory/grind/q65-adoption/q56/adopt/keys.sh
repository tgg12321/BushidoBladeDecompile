#!/bin/bash
# keys.sh: layer-2 body keys of the round-3 step-08 / step-14 bodies on the test clone's chain commits
cd /tmp/q56r2/t || exit 1
source .venv/bin/activate
K="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/keys.py"
python3 "$K" t08 src/sound.c func_800475A4 camera_CalcAngles
python3 "$K" t14 src/text1b_tu1c.c func_80060A68 func_80060C60
python3 "$K" t14 src/code6cac_c_mid.c func_80038170
