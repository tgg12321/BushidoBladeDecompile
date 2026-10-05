# s5d: one D_80103608 declaration (32D04's s32 *[]) in bb2.h; 2B344 converts the slot it reads.
import sys
W = sys.argv[1]
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
H = "include/bb2.h"; F32 = "src/main/32D04.c"; F2B = "src/main/2B344.c"
rep(H, "extern s32 D_801027B0[][5];\n",
    "extern s32 D_801027B0[][5];\n"
    "/* [i]: the first slot (after the header word) of block i, set by func_80044010.  Each slot is a\n"
    " * block-relative offset that func_80044010 turns into an address; func_80044098 / func_80044100\n"
    " * adjust the slots as ints, func_800432A0 / func_800433E4 store a slot to the scratchpad and\n"
    " * func_8003FA24 reads u16 data through one.  32D04's D_80103658[i] holds the slot count. */\n"
    "extern s32 *D_80103608[];\n")
rep(F32, "extern s32 *D_80103608[];\n", "")
rep(F2B, "extern u16 **D_80103608[];\n", "")
rep(F2B, "    src = D_80103608[*(s16 *)(obj + 4)][*(s16 *)(obj + 2)];\n",
    "    src = (u16 *)D_80103608[*(s16 *)(obj + 4)][*(s16 *)(obj + 2)];\n")
for p, s in files.items():
    open(W + p, "w", encoding="utf-8", newline="\n").write(s)
