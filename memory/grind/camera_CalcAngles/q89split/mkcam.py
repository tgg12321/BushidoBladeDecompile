"""mkcam.py <split_dir> <out_dir> <cam.c> : head with camera_CalcAngles's C body and the typed
math_RotMatrixYXZ extern; tail copied unchanged."""
import os, shutil, sys
sd, od, cam = sys.argv[1:4]
os.makedirs(od, exist_ok=True)
h = open(os.path.join(sd, "text1b.c"), encoding="utf-8").read()
line = 'INCLUDE_ASM("asm/funcs", camera_CalcAngles);\n'
assert h.count(line) == 1
h = h.replace(line, open(cam, encoding="utf-8").read())
old = "extern void math_RotMatrixYXZ(s32 *, s32 *);\n"
assert h.count(old) == 1
h = h.replace(old, "extern void math_RotMatrixYXZ(Unk80101DF0Rot *, MATRIX *);\n")
open(os.path.join(od, "text1b.c"), "w", encoding="utf-8", newline="\n").write(h)
shutil.copy(os.path.join(sd, "text1b_tu1b.c"), os.path.join(od, "text1b_tu1b.c"))
