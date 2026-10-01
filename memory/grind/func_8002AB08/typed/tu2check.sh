#!/bin/bash
# tu2check.sh : compile src/code6cac_tu2.c as-is (HEAD-ish include/) and the unk_114[2] respelling with the
# overlay header; compare the cc1 output text
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=tmp/func_8002AB08/typed
python3 - <<'PY'
s = open('src/code6cac_tu2.c').read()
s2 = s.replace('p->unk_114.vx = 0;', 'p->unk_114[0].vx = 0;').replace('p->unk_114.vy = 0;', 'p->unk_114[0].vy = 0;').replace('p->unk_114.vz = 0;', 'p->unk_114[0].vz = 0;')
s2 = s2.replace('p->unk_124.vx = 0;', 'p->unk_114[1].vx = 0;').replace('p->unk_124.vy = 0;', 'p->unk_114[1].vy = 0;').replace('p->unk_124.vz = 0;', 'p->unk_114[1].vz = 0;')
assert s2.count('unk_114[') == 6
open('tmp/func_8002AB08/typed/tu2_new.c', 'w').write(s2)
PY
CPP="mipsel-linux-gnu-cpp -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC="tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
mkdir -p $D/old; git show :include/code6cac.h > $D/old/code6cac.h
$CPP -I$D/old -Iinclude -Isrc src/code6cac_tu2.c 2>/dev/null | $CC -o $D/tu2_old.s
$CPP -I$D/inc/include -Iinclude -Isrc $D/tu2_new.c 2>/dev/null | $CC -o $D/tu2_new.s
if cmp -s $D/tu2_old.s $D/tu2_new.s; then echo "code6cac_tu2 cc1 output IDENTICAL"; else echo DIFFER; diff $D/tu2_old.s $D/tu2_new.s | head; fi
