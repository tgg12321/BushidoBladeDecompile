#!/bin/bash
# receipts.sh: regenerate memory/grind/camera_CalcAngles/types/receipts.txt
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=memory/grind/camera_CalcAngles/types/receipts.txt
{
echo "# func_800475A4 / transform-node typing: order-free per-function compare of a full text1b variant"
echo "# (cc1 -G0 and -G8, maspsx -G8 as built) against build/src/text1b.o, relocations resolved."
echo "# Generators: memory/grind/camera_CalcAngles/types/{mk.py,final.py,cmpall.sh,cmpall.py} (2026-10-02)."
for v in final va va_tb vb_td_ir_plain vb_te_ir_plain vb_tf_ir_plain vb_th_ir_plain vb_tp_ir_plain vb_tq_ir_plain vb_tn_ir_plain vb_tg_ir_plain vb_tk_ir_plain vb_tl_ir_plain vb_tl_ir_plain_bp; do
  echo "=== $v -G0"; bash tmp/camera_CalcAngles/types/cmpall.sh 0 tmp/camera_CalcAngles/types/$v/text1b.c tmp/camera_CalcAngles/types/$v/inc
done
echo "=== final -G8 (head part func_800460E4..func_80048F58 must be identical)"
bash tmp/camera_CalcAngles/types/cmpall.sh 8 tmp/camera_CalcAngles/types/final/text1b.c tmp/camera_CalcAngles/types/final/inc
} > $R 2>&1
