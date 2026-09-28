#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for n in "$@"; do bash tmp/c21c/dump.sh "$n" -dl -dg; done
source .venv/bin/activate
python3 tmp/c21c/greg.py "$@"
