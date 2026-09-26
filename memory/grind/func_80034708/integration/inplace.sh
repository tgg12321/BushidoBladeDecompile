#!/bin/bash
# Byte-neutrality proof BEFORE the split: both merges + consumer conversions + the ings.c
# reorder applied in place (func_80034708 still INCLUDE_ASM in code6cac_b.c, no -G8, no split,
# bb2.ld and every symbol file untouched). Every TU rebuilt, relinked, SHA1 printed.
set -e
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
rm -rf tmp/func_80034708/integ
NOSPLIT=1 python3 tmp/func_80034708/mk_integ3.py
grep -c 'INCLUDE_ASM("asm/funcs", func_80034708);' tmp/func_80034708/integ/src/code6cac_b.c
python3 tmp/func_80034708/integ.py $(cat tmp/func_80034708/allstems.txt) 2>&1 \
  | grep -v "warning:\|^ *|\|^ *[0-9]* |\|Assembler\|unterminated\|^\s*$" | grep "BUILD FAIL\|CHANGED\|functions=" | grep -v "changed=0" || true
D=tmp/func_80034708/fb_inplace
rm -rf $D; mkdir -p $D
cp -r build $D/build
rm -f $D/build/bb2.elf $D/build/bb2.bin $D/build/bb2.exe
cp tmp/func_80034708/integ/build/*.o $D/build/src/
cp bb2.ld undefined_funcs_auto.txt undefined_syms_auto.txt named_syms.txt $D/
cd $D
mipsel-linux-gnu-ld -nostdlib --no-check-sections -Map build/bb2.map -T bb2.ld \
  -T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt -o build/bb2.elf
mipsel-linux-gnu-objcopy -O binary -j .main build/bb2.elf build/bb2.bin
cd - >/dev/null
python3 tools/make_psexe.py disc/SLUS_006.63 $D/build/bb2.bin $D/build/bb2.exe >/dev/null
sha1sum $D/build/bb2.exe
