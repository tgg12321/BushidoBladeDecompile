#!/bin/bash
# usage: bash tmp/func_800620B8/s3/psx.sh <variant-name>   (after dump.sh produced d/<name>/t.i)
# Compile the SAME preprocessed TU with the ORIGINAL PsyQ cc1psx (GCC 2.7.2.SN.1, dosemu2) -- calibration only.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=tmp/func_800620B8/s3/d/$1
FL="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
echo "cc1psx: bash tools/cc1psx_wrapper.sh $FL < t.i > psx.s" >> $D/cmd.txt
bash tools/cc1psx_wrapper.sh $FL < $D/t.i > $D/psx.s 2> $D/psx.err || { tail -5 $D/psx.err; exit 1; }
python3 - $D <<'PY'
import re,sys
t=open(sys.argv[1]+'/psx.s').read()
m=re.search(r'\nfunc_800620B8:\n(.*?)\n\t\.end\tfunc_800620B8', t, re.S)
f=m.group(1)
open(sys.argv[1]+'/psx_f.s','w').write(f)
print('lines', f.count('\n'))
for l in f.split('\n'):
    if 'D_8009BA' in l or '$fp' in l: print(l)
PY
rm -f $D/psx.s
