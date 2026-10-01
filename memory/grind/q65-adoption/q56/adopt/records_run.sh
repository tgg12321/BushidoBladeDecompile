#!/bin/bash
# records_run.sh: scratch clone at step01 (pre-move layout, oracle build) -> cut-window records for the ledger.
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
A="/tmp/q56/adopt tree"
cd "$A" && HEAD_REF=$(git rev-parse HEAD) && git checkout -q step01 && rm -rf build && source .venv/bin/activate
make -j16 build/bb2.exe >/dev/null 2>&1; echo "step01 build: $(sha1sum build/bb2.exe)"
{
echo "## Cut-window records (pre-move layout at step01 = $(git rev-parse --short step01), base $(git rev-parse --short step0))"; echo
python3 "$H/window.py" "$A" code6cac_b_tu2 D_800A3140 func_80033498 func_800343F0
echo
echo "rodata-align conditions for the surviving positions: condition 2 - b_tu3 keeps its rodata start (INCLUDE_RODATA jtbl_8001084C at 0x8001081C; the functions after func_80033498 that could move own no compiled rodata before it); condition 3 - text order is unchanged and no gp symbol is shared across any surviving cut, so every surviving position gives identical bytes; the conventional position (immediately before func_800343F0, the function holding the access b_tu2's assembly cannot produce) is used, the others are recorded here; condition 4 - moves only (splitc.py + mergec.py, verbatim)."
echo
python3 "$H/window.py" "$A" text1a_c D_800A3820
echo
python3 "$H/cutsearch.py" "$A" text1b text1b_tu1c
} > "$H/cut_windows.md"
git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
grep -c survives "$H/cut_windows.md"
