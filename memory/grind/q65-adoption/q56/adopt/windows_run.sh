#!/bin/bash
# windows_run.sh: scratch clone only - back to step01, oracle build, then the cut-window records.
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
bash "$H/rewind.sh" step01 >/dev/null
cd "/tmp/q56/adopt tree" && rm -rf build && source .venv/bin/activate
make -j16 build/bb2.exe >/dev/null 2>&1; sha1sum build/bb2.exe
python3 "$H/window.py" "/tmp/q56/adopt tree" text1a_c D_800A3820
python3 "$H/window.py" "/tmp/q56/adopt tree" code6cac_b_tu2 D_800A3140 | head -30
