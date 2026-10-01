#!/bin/bash
# post_run.sh: after series.sh - the dated inventory (pre-switch tag step14), SDATA_FILES vs GP_FILES, then the
# M3/M4 merge evidence at step07 (built there). Scratch clone only; restores the branch head.
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
A="/tmp/q56/adopt tree"
cd "$A" && source .venv/bin/activate && HEAD_REF=$(git rev-parse HEAD)
python3 "$H/inventory.py" "$A" step14 "$H/inventory.md"
echo "SDATA_FILES: $(git show step15:Makefile | grep '^SDATA_FILES :=' | cut -d= -f2)"
echo "GP_FILES:    $(git show step15:Makefile | grep '^GP_FILES :=' | cut -d= -f2)"
git checkout -q step07 && rm -rf build && make -j16 build/bb2.exe >/dev/null 2>&1
python3 "$H/m34_evidence.py" "$A"
git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
