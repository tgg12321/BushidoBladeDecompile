#!/bin/bash
# Bank dump excerpts for the (a1) record-extension proof into the ledger.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
X=tmp/func_80036140/excerpt.py; D=tmp/func_80036140/dumps; O=memory/grind/func_80036140/landing/dumps
mkdir -p $O
python3 $X $D/rec_G8 b5 func_80036140 $O/rec_G8.func_80036140.txt 4 'D_80101E58"\)\)\s+\(const_int (68|76)\)'
python3 $X $D/split_G8 b5 func_80036140 $O/split_G8.func_80036140.txt 4 'D_80101E9C|D_80101EA4'
python3 $X $D/ptr_G8 b5 func_80036140 $O/ptrrmw_G8.func_80036140.txt 4 'D_80101E9C|D_80101EA4'
python3 $X $D/ptr_G0 b5 func_80036140 $O/ptrrmw_G0.func_80036140.txt 4 'D_80101E9C|D_80101EA4'
python3 $X $D/b4p_rec b4p cdrom_ReadyCallback $O/rec_G0.cdrom_ReadyCallback.txt 3 'D_80101E58"\)\)\s+\(const_int (64|40|44|72)\)'
python3 $X $D/b4p_sep b4p cdrom_ReadyCallback $O/sep12_G0.cdrom_ReadyCallback.txt 3 'D_80101E9C|D_80101E58"\)\)\s+\(const_int (64|40|44)\)'
du -sh $O
