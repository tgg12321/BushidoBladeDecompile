#!/bin/bash
# All storage-class x -G calibration runs (original toolchain) for the two new -G8 TUs of tree p3.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
C=tmp/func_80036140/calib.py
B5="func_80036140 func_80036940 -- g_cd_result g_cd_atv D_800A36B8 D_800A3840 D_800A3854"
B4="cdrom_SetMix func_80035F78 -- g_cd_atv D_800A36B8 D_800A3840 D_800A3854"
for cls in comm extern static init; do
  for G in -G8 -G0; do
    python3 $C p3 code6cac_b5 b5_${cls}$G $cls $G $B5
    python3 $C p3 code6cac_b4 b4_${cls}$G $cls $G $B4
  done
done 2>&1 | tee tmp/func_80036140/calib/ALL.txt
