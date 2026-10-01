# generate func_8002C22C variants from the banked body
import re, sys
R = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" if sys.platform != "win32" else "."
src = open("memory/grind/func_8002C22C/rejected/retro-audit-2026-09-30.c").read()
m = re.search(r"^void func_8002C22C\(void\) \{\n.*?^\}\n", src, re.S | re.M)
body = m.group(0)
F = {0x210: ("unk_210", 0), 0x21C: ("unk_210", 1), 0x234: ("unk_234", 0), 0x240: ("unk_234", 1)}
R0 = {}
for base, (fld, k) in F.items():
    for j, c in enumerate("xyz"):
        R0[0x80101EC8 + base + 4 * j] = (fld, k, c)
def rec0(expr):
    def f(mm):
        a = int(mm.group(1), 16)
        fld, k, c = R0[a]
        return expr % (fld, k, c)
    return f
def fld1(off):
    for base, (fld, k) in F.items():
        if base <= off < base + 12:
            return fld, k, "xyz"[(off - base) // 4]
def make(decl, r0, r1):
    b = body.replace("    s32 *d_tbl = &D_80102314;\n", decl)
    b = re.sub(r"\bD_(80102[0-9A-F]{3})\b", rec0(r0), b)
    b = re.sub(r"d_tbl\[(0x[0-9A-F]+)/4\]", lambda mm: r1 % fld1(int(mm.group(1), 16)), b)
    return b
V = {
  "p1": make("    PracticeMenuRec *rec1 = &g_practice_menu_table[1];\n",
             "g_practice_menu_table[0].%s[%d].%s", "rec1->%s[%d].%s"),
  "direct": make("", "g_practice_menu_table[0].%s[%d].%s", "g_practice_menu_table[1].%s[%d].%s"),
  "p0p1": make("    PracticeMenuRec *rec0 = &g_practice_menu_table[0];\n    PracticeMenuRec *rec1 = &g_practice_menu_table[1];\n",
             "rec0->%s[%d].%s", "rec1->%s[%d].%s"),
  "tblp": make("    PracticeMenuRec *tbl = g_practice_menu_table;\n",
             "tbl[0].%s[%d].%s", "tbl[1].%s[%d].%s"),
}
for k, b in V.items():
    open(f"tmp/func_8002C22C/{k}.c", "w", newline="\n").write(b)
print(list(V))

def spad(mm):
    off = int(mm.group(1), 16)
    if off < 0x48:
        i, r = divmod(off, 0x24); arr = "unk00"
    else:
        i, r = divmod(off - 0x48, 0x18); arr = "unk48"
    j, c = divmod(r, 12)
    return "SPAD->%s[%d][%d].%s" % (arr, i, j, "xyz"[c // 4])
for k in ("p1", "p0p1"):
    b = re.sub(r"\*\(s32 \*\)0x1F8000([0-9A-F]{2})", spad, V[k])
    open(f"tmp/func_8002C22C/{k}s.c", "w", newline="\n").write(b)
