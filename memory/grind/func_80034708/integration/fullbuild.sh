#!/bin/bash
# Scratch full link: pristine build/ objects + the integ TUs, linked with integ/bb2.ld.
# Never touches build/ or any tracked file.
set -e
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tmp/func_80034708/fb
rm -rf $D; mkdir -p $D
cp -r build $D/build
rm -f $D/build/bb2.elf $D/build/bb2.bin $D/build/bb2.exe
cp tmp/func_80034708/integ/build/*.o $D/build/src/
if [ -f tmp/func_80034708/integ/asm/data/91C98.data.s ]; then
  mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 \
    tmp/func_80034708/integ/asm/data/91C98.data.s -o $D/build/asm/data/91C98.data.o
fi
python3 tmp/func_80034708/mk_ld.py
cp tmp/func_80034708/integ/bb2.ld $D/bb2.ld
for f in undefined_funcs_auto.txt undefined_syms_auto.txt named_syms.txt; do
  if [ -f tmp/func_80034708/integ/$f ]; then cp tmp/func_80034708/integ/$f $D/$f; else cp $f $D/$f; fi
done
cd $D
mipsel-linux-gnu-ld -nostdlib --no-check-sections -Map build/bb2.map -T bb2.ld \
  -T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt -o build/bb2.elf
mipsel-linux-gnu-objcopy -O binary -j .main build/bb2.elf build/bb2.bin
cd - >/dev/null
python3 tools/make_psexe.py disc/SLUS_006.63 $D/build/bb2.bin $D/build/bb2.exe
sha1sum $D/build/bb2.exe
