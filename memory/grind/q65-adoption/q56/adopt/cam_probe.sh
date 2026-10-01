#!/bin/bash
# cam_probe.sh: camera_CalcAngles' tail shape - a static s16[2] pair written [0], [1], then its address
# returned - under cc1psx -G8 (calibration only) and our cc1 -G0 / -G8.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=/tmp/q56r2/cam; mkdir -p $D
cat > $D/p.c <<'X'
extern int ratan2(int, int);
static short D_pair[2];
short *f(int a, int b, short s0) {
    D_pair[0] = -ratan2(a, b);
    D_pair[1] = s0;
    return D_pair;
}
X
echo "=== cc1psx -G8"; bash tools/cc1psx_wrapper.sh -O2 -G8 -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < $D/p.c | grep -vE '^\s*#|^$|^\s*\.(frame|mask|fmask|file|ent|end|text|align|globl|set|lcomm)' | head -30
for g in 0 8; do echo "=== our cc1 -G$g"; tools/gcc-2.7.2/build/cc1 -O2 -G$g -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < $D/p.c -o /dev/stdout | grep -vE '^\s*#|^$|^\s*\.(frame|mask|fmask|file|ent|end|text|align|globl|set|local|comm|type|size|ident|version)' | head -30; done
