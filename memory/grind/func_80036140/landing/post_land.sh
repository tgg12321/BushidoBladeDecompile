#!/bin/bash
# After land.sh: move check against the landing rev, maspsx tests, and the landed scores of the moved functions.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; REV=$(cat $H/land_rev.txt)
bash $H/mktree.sh inplace_land $REV >/dev/null
python3 $H/apply_model.py $H/inplace_land --split none --merge --ext rec
python3 $H/movecheck.py . $H/inplace_land $REV > $H/movecheck_landed.txt; tail -8 $H/movecheck_landed.txt
(cd tools/maspsx && python3 -m unittest discover -s tests -t . 2>&1 | tail -1)
python3 $H/screen.py fin_s32 code6cac_b5 /dev/null >/dev/null 2>&1; echo "landed rev $REV"
