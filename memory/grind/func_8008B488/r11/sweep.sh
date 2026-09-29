#!/bin/bash
# usage: sweep.sh OUTDIR [partition-spec]  -> generate variants, score each (standalone harness)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
python3 tmp/f8b488s4/gen_part.py "$1" "${@:2}"
python3 tmp/f8b488s2/score.py $(cat "$1/list.txt") | sort -t'(' -k2 -n
