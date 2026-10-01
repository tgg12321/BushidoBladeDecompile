#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/rev58580r11c"
python3 - <<'PY'
s=open('../func_80058580/r11b/dumps/reuse/t.i').read()
i=s.index('work4 = wi;'); j=s.index('work4--;',i)+len('work4--;')
seg=s[i:j].replace('work4 = wi;','').replace('work4','wi')
open('wi.i','w').write(s[:i]+seg+s[j:])
PY
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
../../tools/gcc-2.7.2/build/cc1 $FLAGS wi.i -o wi.s
echo "wi-direct: $(diff base.s wi.s | grep -c '^[<>]') diff lines; insns $(grep -c '^\s[a-z]' wi.s) vs $(grep -c '^\s[a-z]' base.s)"
