#!/bin/bash
# usage: xb1.sh <body.c> [tag]  -> builds code6cac_tu2 with the body + header override, prints diffs + focus
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
tag=${2:-x}
d=tmp/func_80023F08/srcx_$tag
python3 tmp/func_80023F08/tu_edit.py $1 $d >/dev/null && python3 tmp/func_80023F08/xbuild.py code6cac_tu2 $d/code6cac_tu2.c tmp/func_80023F08/inc func_80023F08 | grep -v "DIFF func_80023F08\|DIFF func_80020D70"
