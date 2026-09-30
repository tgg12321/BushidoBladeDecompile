#!/bin/bash
# jtsurvey.sh: build with every `.align 3` marked (layout identical to the normal build for
# RODATA_ALIGN2 files), list each marker's absolute address and phase, then restore the build.
cd /home/user/BushidoBladeDecompile || exit 1
sed -e 's/^rodata_align_fix = .*/rodata_align_fix = python3 tmp\/jtmark.py $(if $(filter $1,$(RODATA_ALIGN2_FILES)),flat,keep) |/' Makefile > tmp/Makefile.jt
rm -rf build
make -f tmp/Makefile.jt -j16 build/bb2.bin > tmp/jtsurvey.log 2>&1
echo "make rc=$?"
cmp build/bb2.bin /tmp/claude-0/ref_main.bin && echo "marked build byte-identical (markers are layout-neutral)"
mipsel-linux-gnu-nm -n build/bb2.elf | grep JTMARK > tmp/jtmarks.txt
wc -l < tmp/jtmarks.txt
