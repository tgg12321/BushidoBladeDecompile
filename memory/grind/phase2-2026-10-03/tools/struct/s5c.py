# s5c: D_800F1140 / 44 / 48 -> one Vec4i32 D_800F1140; func_80041E10 takes Vec4i32 *. usage: s5c.py PREFIX [cam]
import sys, re
W = sys.argv[1]
modes = set(sys.argv[2:])
files = {}
def get(p):
    if p not in files:
        files[p] = open(W + p, encoding="utf-8").read()
    return files[p]
def rep(p, a, b, cnt=1):
    s = get(p)
    n = s.count(a)
    assert n == cnt, (p, a, n)
    files[p] = s.replace(a, b)
H = "include/bb2.h"; F5 = "src/main/51268.c"; F31 = "src/main/31D3C.c"; U = "undefined_syms_auto.txt"

rep(H, "extern s32 D_800F1144;\nextern s32 D_800F1148;\n",
    "/* A 16-byte vector: thirteen 51268 functions store x / y / z, and func_80061064 passes its\n"
    " * address to func_80041E10, which copies all 16 bytes to D_800A9B28. */\n"
    "extern Vec4i32 D_800F1140;\n")
rep(H, "extern void func_800418D0(s32 *);\n", "extern void func_800418D0(s32 *);\nextern void func_80041E10(Vec4i32 *, s32);\n")
rep(F5, "extern s32 func_80041E10();\n", "")
rep(F5, "extern s32 D_800F1140;\n", "")
s = get(F5)
s, n1 = re.subn(r"D_800F1140 = \*(p|ap)\+\+;", r"D_800F1140.vx = *\1++;", s)
s, n2 = re.subn(r"D_800F1144 = \*(p|ap)\+\+;", r"D_800F1140.vy = *\1++;", s)
s, n3 = re.subn(r"D_800F1148 = \*(p|ap)(\+\+)?;", r"D_800F1140.vz = *\1\2;", s)
assert (n1, n2, n3) == (13, 13, 13), (n1, n2, n3)
assert "D_800F1144" not in s and "D_800F1148" not in s
files[F5] = s
rep(F31, "extern Block16 D_800A9B28;\nvoid func_80041E10(Block16 *a0, s32 a1) {", "extern Vec4i32 D_800A9B28;\nvoid func_80041E10(Vec4i32 *a0, s32 a1) {")
if "cam" in modes:
    rep(F31, "    s32 *cam;\n", "    Vec4i32 *cam;\n")
    rep(F31, "    cam = (s32 *)&D_800A9B28;\n", "    cam = &D_800A9B28;\n")
    rep(F31, "        dx = ptr[0] - cam[0];\n        dy = ptr[1] - cam[1];\n        dz = ptr[2] - cam[2];\n",
        "        dx = ptr[0] - cam->vx;\n        dy = ptr[1] - cam->vy;\n        dz = ptr[2] - cam->vz;\n")
for k in ("D_800F1144", "D_800F1148"):
    rep(U, "%s = 0x%s;\n" % (k, k[2:]), "")
for p, s in files.items():
    open(W + p, "w", encoding="utf-8", newline="\n").write(s)
