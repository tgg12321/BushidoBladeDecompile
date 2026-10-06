#!/usr/bin/env python3
# Worker-2 batch 6: P7b -- D_800A38B4 is the draw-chunk builders' u32 * word cursor (plan:
# lt/item3_plan2.txt P7b). The builders take and return byte sizes as s32, so each call converts the
# cursor (s32)D_800A38B4 (a boundary conversion to the callee's parameter type) and each advance adds
# words: `+= ret / 4` where the callers rounded down to whole words, `+= ret` at the two callers whose
# targets add ret * 4 bytes (2B344 func_8003C2C0 after func_8005FC9C, func_8003CD10 after func_800600C8).
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/p7b/w2b6.py [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "c84492fba"   # batch 5 on p2/w2 (main 370450dcc)
rep = B2.rep
CNT = {}


def base(p):
    B2.BASE = BASE
    return B2.show(p)


def cnt(k, n=1):
    CNT[k] = CNT.get(k, 0) + n


def bb2_h(s):
    return rep(s, "extern u32 D_800A38B4;\n", "extern u32 *D_800A38B4; /* the draw-chunk builders' word cursor */\n")


def c2b344(s):
    for a, c in [("ret = func_80060544(D_800A38B4, 1);", "ret = func_80060544((s32)D_800A38B4, 1);"),
                 ("ret = func_8005FC9C(D_800A38B4, 1);", "ret = func_8005FC9C((s32)D_800A38B4, 1);"),
                 ("ret = func_8005E54C(D_800A3784, D_800A38B4, 1);", "ret = func_8005E54C(D_800A3784, (s32)D_800A38B4, 1);"),
                 ("ret = func_80060768(D_800A38B4, 1, D_800A38E9);", "ret = func_80060768((s32)D_800A38B4, 1, D_800A38E9);"),
                 ("ret = func_8005FA98(0, D_800A38B4, 1);", "ret = func_8005FA98(0, (s32)D_800A38B4, 1);"),
                 ("ret = func_800600C8(D_800A391F, D_800A38B4, 1);", "ret = func_800600C8(D_800A391F, (s32)D_800A38B4, 1);")]:
        s = rep(s, a, c)
        cnt("2B344 call")
    s = rep(s, "D_800A38B4 = D_800A38B4 + (func_8005C8A8(1, D_800A3817, D_800A38B4, 0) / 4) * 4;",
            "D_800A38B4 += func_8005C8A8(1, D_800A3817, (s32)D_800A38B4, 0) / 4;")
    cnt("2B344 call")
    cnt("2B344 adv/4")
    n = s.count("D_800A38B4 = D_800A38B4 + (ret / 4) * 4;")
    s = s.replace("D_800A38B4 = D_800A38B4 + (ret / 4) * 4;", "D_800A38B4 += ret / 4;")
    cnt("2B344 adv/4", n)
    n = s.count("D_800A38B4 = D_800A38B4 + ret * 4;")
    s = s.replace("D_800A38B4 = D_800A38B4 + ret * 4;", "D_800A38B4 += ret;")
    cnt("2B344 adv ret", n)
    assert "D_800A38B4 = D_800A38B4" not in s
    return s


def c9f9c(s):
    pat1 = re.compile(r"D_800A38B4 \+= (func_\w+\((?:[^;]*?)), D_800A38B4, (\d)\) / 4 \* 4;")
    s, n = pat1.subn(r"D_800A38B4 += \1, (s32)D_800A38B4, \2) / 4;", s)
    cnt("9F9C call", n)
    cnt("9F9C adv/4", n)
    pat2 = re.compile(r"D_800A38B4 = D_800A38B4 \+ \(\((func_\w+\((?:[^;]*?)), D_800A38B4, (\d)\) / 4\) \* 4\);")
    s, n = pat2.subn(r"D_800A38B4 += \1, (s32)D_800A38B4, \2) / 4;", s)
    cnt("9F9C call", n)
    cnt("9F9C adv/4", n)
    s = rep(s, "ret = func_8005FA98(0, D_800A38B4, 1);", "ret = func_8005FA98(0, (s32)D_800A38B4, 1);")
    cnt("9F9C call")
    s = rep(s, "D_800A38B4 = D_800A38B4 + ((ret / 4) * 4);", "D_800A38B4 += ret / 4;")
    cnt("9F9C adv/4")
    assert "(s32)(s32)" not in s
    return s


def c6cf8(s):
    s = rep(s, "        D_800A38B4 = prim_base + (idx * 0x9A00);", "        D_800A38B4 = (u32 *)(prim_base + (idx * 0x9A00));")
    s = rep(s, "            func_8005C8A8(2, select | (D_800A3788 << 16), D_800A38B4, 0);",
            "            func_8005C8A8(2, select | (D_800A3788 << 16), (s32)D_800A38B4, 0);")
    s = rep(s, "            func_8005C8A8(0, select, D_800A38B4, 0);", "            func_8005C8A8(0, select, (s32)D_800A38B4, 0);")
    s = rep(s, "    D_800A38B4 = tbl[idx];", "    D_800A38B4 = (u32 *)tbl[idx];")
    s = rep(s, "        s32 adj = D_800A38B4 + 0xFFFECC00u;", "        s32 adj = (s32)D_800A38B4 + 0xFFFECC00u;")
    s = rep(s, NL + "u32 D_800A38B4;" + NL, NL + "u32 *D_800A38B4;" + NL)
    return s


def c368e4(s):
    s = rep(s, "    D_800A38B4 = (u32)obj;\n", "    D_800A38B4 = (u32 *)obj;\n")
    s = rep(s, "    D_800A38B4 = (u32)(obj + 1);\n", "    D_800A38B4 = (u32 *)(obj + 1);\n")
    return s


def c25c38(s):
    return rep(s, "    D_800A38B4 = (u32)g;\n", "    D_800A38B4 = (u32 *)g;\n")


FILES = [("include/bb2.h", bb2_h), ("src/main/2B344.c", c2b344), ("src/main/9F9C.c", c9f9c),
         ("src/main/6CF8.c", c6cf8), ("src/main/368E4.c", c368e4), ("src/main/25C38.c", c25c38)]


# B4 sweep of the 13 moved bodies (abl_w2b6.py): the holders / staged temps that ablate to IDENTICAL go,
# the two that score are labelled.
sys.path.insert(0, HERE)
import abl_w2b6 as ABL
DROP = ["b8e4_ret", "b8e4_tmp", "c2c0_ret", "c2c0_stage", "c2c0_a4val", "c2c0_state", "c560_ret", "c560_counter",
        "c560_id", "c8b4_ret", "c9a4_a0a1", "c9a4_ret", "cd10_a0a1", "cd10_ret", "ea84_ret", "ea84_base", "553c_ot",
        "553c_q", "e60_padbits", "e60_bits", "main_cnt"]
LABELS = []   # rev-w2b6: the three holders the first sweep labelled have measured plain forms (fix6)


def fix6(p, s):
    """rev-w2b6 fixes (FAIL, measured): ternaries for the two D_800A38A4 choices, main's overflow
    check as one expression, func_80016E60's no-op (u8) and its OT words, the carried no-op casts"""
    if p == "src/main/2B344.c":
        s = rep(s, """            s32 newval = 8;
            if (D_8008D9EC[(s16)D_80101EC8[0].unk_0A] != 0) {
                newval = 9;
            }
            D_800A38A4 = newval;
""", "            D_800A38A4 = D_8008D9EC[D_80101EC8[0].unk_0A] != 0 ? 9 : 8;\n")
        s = rep(s, """            a4val = 4;
            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {
                a4val = 5;
            }
            D_800A38A4 = a4val;
""", "            D_800A38A4 = D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0 ? 5 : 4;\n")
        s = B2.fn(s, "func_8003C560", lambda b: rep(b, "    u8 a4val;\n", ""))
        s = B2.fn(s, "func_8003C9A4", lambda b: rep(rep(b, "    if ((u8)D_800A3929 < 0x3C) return;\n", "    if (D_800A3929 < 0x3C) return;\n"),
                  "        D_800A38DF = (u8)func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);\n",
                  "        D_800A38DF = func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);\n"))
    if p == "src/main/9F9C.c":
        s = B2.fn(s, "func_8001EA84", lambda b: rep(b, "    if (((u8)D_800A3929) < 0x3C) return;\n", "    if (D_800A3929 < 0x3C) return;\n"))
    if p == "src/main/6CF8.c":
        s = rep(s, """        s32 prim_base = (s32)tbl[idx];
        s32 adj = (s32)D_800A38B4 + 0xFFFECC00u;
        s32 remaining = prim_base - adj;
""", "        s32 remaining = tbl[idx] + 0x13400 - (s32)D_800A38B4;\n")
        s = B2.fn(s, "func_80016E60", lambda b: rep(rep(b, "1 << (u8)(select - 3)", "1 << (select - 3)", 2), "    u8 *ot[2];\n", "    u32 ot[2];\n"))
    return s


def sweep(p, s):
    byname = {a[0]: a for a in ABL.A}
    for d in DROP:
        name, fname, path, t = byname[d]
        if path == p:
            s = B2.fn(s, fname, t)
    for fname in ("func_8003B8E4", "func_8003C2C0", "func_8003C560", "func_8003C8B4", "func_8003C9A4",
                  "func_8003CD10", "func_8001EA84", "func_8003553C"):   # a dropped first declaration's blank line
        s = s.replace("void %s(void) {\n\n" % fname, "void %s(void) {\n" % fname)
    for f, anchor, text in LABELS:
        if f == p:
            ind = anchor[:len(anchor) - len(anchor.lstrip(" "))]
            s = rep(s, anchor, "%s/* FAKE: %s. */\n%s" % (ind, text, anchor))
    return s


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b6"])[0]
    for p, g in FILES:
        s = fix6(p, sweep(p, g(base(p))))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    for k, v in sorted(CNT.items()):
        print("%-14s %d" % (k, v))
    print("w2b6 wrote %d files to %s" % (len(FILES), "the tree" if apply else out))


if __name__ == "__main__":
    main()
