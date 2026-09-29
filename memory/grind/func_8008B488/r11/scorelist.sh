#!/bin/bash
# usage: scorelist.sh LISTFILE [-v]  -> standalone scores, sorted
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
mapfile -t F < "$1"
python3 tmp/f8b488s2/score.py "${F[@]}" ${2:-} | sort -t'(' -k2 -n
