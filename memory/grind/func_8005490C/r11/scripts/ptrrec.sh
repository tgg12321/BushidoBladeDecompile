#!/bin/bash
# record: ctrl-block members typed u8 * (mk.py --ptr) vs s32 (minimal A), every text1b / code6cac_c2 function
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
echo "# mk.py <body> <dir> --ptr : include/game.h Unk800EFAE8Ctrl unk2C/unk30/unk34[2]/unk3C[2] -> u8 *,"
echo "# func_80054604 a6 / func_80054884 a7 / code6cac_c2.c magic -> u8 *, func_80054FDC p -> u8 **, interface casts (s32) added."
echo "# xbuild.py compiles the TU copies with the build pipeline and compares every function with build/src/<stem>.o."
bash tmp/func_8005490C/xb.sh - ptr --ptr --c2 2>&1
echo
echo "## func_80054FDC (ref < vs u8 * typing >)"
bash tmp/func_8005490C/dis.sh func_80054FDC tmp/func_8005490C/xb/text1b.o
echo "## func_80054604"
bash tmp/func_8005490C/dis.sh func_80054604 tmp/func_8005490C/xb/text1b.o
echo
echo "# minimal (A) only (func_8003FFC4 extern (s32 *), func_80054604 v -> s32 *, its two (s32) casts dropped):"
bash tmp/func_8005490C/xb.sh - A1 --c2 2>&1
