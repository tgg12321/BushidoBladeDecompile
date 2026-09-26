#!/bin/bash
# dumps2.sh : RTL dumps (cse/loop/lreg/greg) + BB2_ALLOC_DEBUG for split and merged forms,
# from the SAME preprocessed TUs psx.sh fed to cc1psx; function-extracted into the ledger.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
L=memory/grind/func_8005D814/dumps
FL="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for T in split merged; do
  W=/tmp/d814_$T; rm -rf $W; mkdir -p $W $L/$T
  cp tmp/func_8005D814/psx_$T/t.i $W/t.i
  ( cd $W && "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/gcc-2.7.2/cc1" $FL -ds -dL -dl -dg t.i -o t.s )
  ( cd $W && BB2_ALLOC_DEBUG=1 "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/gcc-2.7.2/cc1" $FL t.i -o /dev/null 2>&1 | grep "func=func_8005D814" ) > $L/$T/allocdbg.txt
  for ext in cse loop lreg greg; do
    awk '/^;; Function func_8005D814/{p=1} /^;; Function /&&!/func_8005D814/{p=0} p' $W/t.i.$ext > $L/$T/func_8005D814.$ext
  done
  echo "tools/gcc-2.7.2/cc1 $FL -ds -dL -dl -dg t.i   (t.i = tmp/func_8005D814/psx_$T/t.i, cpp of src/text1b.c with the candidate spliced; Makefile CPP_FLAGS+CPP_DEFS)" > $L/$T/COMMAND.txt
  echo "BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $FL t.i -o /dev/null" >> $L/$T/COMMAND.txt
  # cc1psx calibration outputs + diffs vs target
  cp tmp/func_8005D814/psx_$T/psx_fn.txt $L/$T/cc1psx_func.txt
  cp tmp/func_8005D814/psx_$T/ours_fn.txt $L/$T/cc1_func.txt
done
ls -la $L/*/ | head -30
