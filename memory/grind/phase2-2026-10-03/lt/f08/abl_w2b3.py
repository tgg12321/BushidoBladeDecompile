#!/usr/bin/env python3
# FAKE ablations for w2b3 (run from the repo root with the batch applied): every FAKE in every moved body,
# new and carried, each removed (its plain alternative) in a copy of the applied body. Writes
# tmp/w2/abl3/<name>.c + tmp/w2/abl3/list.txt ("<name> <func> <file>") for run_abl_w2b3.ps1
# (engine sandbox <func> --disable all --candidate <file>).
import os, re


def body(path, fn):
    s = open(path, encoding="utf-8").read()
    m = re.search(r"\n[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{" % fn, s)
    i = m.start() + 1
    return s[i:s.index("\n}\n", i) + 3]


def sub(b, a, c, n=1):
    if b.count(a) != n:
        raise SystemExit("expected %d x %r, found %d" % (n, a[:70], b.count(a)))
    return b.replace(a, c)


def resub(b, pat, rep):
    b2, k = re.subn(pat, rep, b)
    if not k:
        raise SystemExit("no match " + pat)
    return b2


def drop_comment(b, start):
    i = b.index(start)
    j = b.index("*/", i) + 2
    if b[j:j + 1] == "\n":
        j += 1
    return b[:i] + b[j:]


A = []   # (name, func, path, transform)
F2B, F9C, F48, FD3, F68, FAB = ("src/main/2B344.c", "src/main/309CC.c", "src/main/31548.c",
                                 "src/main/31D3C.c", "src/main/368E4.c", "src/main/3AB48.c")
A.append(("fa24_arms", "func_8003FA24", F2B, lambda b: sub(b, """                if (((s16)flags >> 3) & 2) {
                    value = ((u32)src[7] << 16) | src[6];
                } else {
                    value = ((u32)src[7] << 16) | src[6];
                }""", """                value = ((u32)src[7] << 16) | src[6];""")))
A.append(("f8_cmp", "func_800400F8", F2B, lambda b: sub(b, "if (s2->count > s0) {", "if (s2->count > 0) {")))
# f8_s16: dropped by the rev-w2b3 fixes
# 510_wrap: ablated to 0, removed from the landed form
A.append(("8f8_neg1", "func_800408F8", F9C, lambda b: sub(sub(sub(b, "        s32 neg1 = -1;\n", ""),
                                                              "p->node.unk2 = neg1;", "p->node.unk2 = -1;"),
                                                          "if (v1 != neg1) {", "if (v1 != -1) {")))
A.append(("cb8_consts", "func_80040CB8", F9C, lambda b: sub(sub(sub(sub(sub(b, "    do {\n        none = -1;\n        kind = 3;\n        one = 1;\n    } while (0);\n", ""),
                                                                    "if (id != none) {", "if (id != -1) {"),
                                                                "*slot = kind;", "*slot = 3;"),
                                                            "= one;", "= 1;"),
                                                        "    s32 none;\n    s32 kind;\n    s32 one;\n", "")))
A.append(("cb8_wrap1", "func_80040CB8", F9C, lambda b: sub(b, "    do {\n        none = -1;\n        kind = 3;\n        one = 1;\n    } while (0);\n",
                                                           "    none = -1;\n    kind = 3;\n    one = 1;\n")))
A.append(("cb8_wrap2", "func_80040CB8", F9C, lambda b: sub(sub(b, "    do {\n        ent = (s32)&arg0->unk_8B4[0].unk58;\n", "    ent = (s32)&arg0->unk_8B4[0].unk58;\n    {\n"),
                                                           "        if (i < 0x12) goto loop;\n    } while (0);\n", "        if (i < 0x12) goto loop;\n    }\n")))
A.append(("d48_s4", "func_80040D48", F48, lambda b: re.sub(r"\bs4\b", "ent", sub(sub(b, "    Unk80045878Obj *s4;\n", ""), "    s4 = ent;\n", ""))))
A.append(("d48_s5first", "func_80040D48", F48, lambda b: sub(sub(b, "    s5 = s4->unk_2C;\n", ""), "        s5->node.unk6 = 0;\n", "        s4->unk_2C[0].node.unk6 = 0;\n")))
A.append(("d48_s1", "func_80040D48", F48, lambda b: re.sub(r"\bs1\b(?!p)", "arg4", sub(sub(b, "    s16 *s1;\n", ""), "        s1 = arg4;\n", ""))))
A.append(("d48_p58", "func_80040D48", F48, lambda b: sub(b, "    {\n        s32 *p58 = &s3->unk58;\n        *p58 = (s32)s4->unk_18F4;\n    }\n",
                                                          "    s3->unk58 = (s32)s4->unk_18F4;\n")))


def d48_counters(b):
    # one counter per loop: c1 (case-0 rotations), c2 (unk6 clear), c3 (func_800417D0), c4 (list)
    b = sub(b, "    s32 s0;\n", "    s32 c1, c2, c3, c4;\n")
    b = sub(b, "        s0 = 1;\n        tbl = D_80094CFC;", "        c1 = 1;\n        tbl = D_80094CFC;")
    b = sub(b, "            a4p = &s3[s0];", "            a4p = &s3[c1];")
    b = sub(b, "            s0++;\n            tbl++;\n        } while (s0 < 0x12);", "            c1++;\n            tbl++;\n        } while (c1 < 0x12);")
    b = sub(b, "        s0 = 0x11;", "        c2 = 0x11;")
    b = sub(b, "            s0--;\n            p--;\n        } while (s0 >= 0);", "            c2--;\n            p--;\n        } while (c2 >= 0);")
    b = sub(b, "        s0 = 0;\n", "        c3 = 0;\n")
    b = sub(b, "            s0++;\n            s1p++;\n        } while (s0 < 0x12);", "            c3++;\n            s1p++;\n        } while (c3 < 0x12);")
    b = sub(b, "    s0 = 1;\n    s3->node.unk0 = 0xA;", "    c4 = 1;\n    s3->node.unk0 = 0xA;")
    b = sub(b, "            s0++;\n            a4p++;\n        } while (s0 < 0x12);", "            c4++;\n            a4p++;\n        } while (c4 < 0x12);")
    return b


A.append(("d48_s0", "func_80040D48", F48, d48_counters))
A.append(("688_pad", "func_80041688", FD3, lambda b: drop_comment(b, "    volatile u32 pre_pad[8];")))
A.append(("688_b", "func_80041688", FD3, lambda b: sub(b, "    b = p->node.unk2 >= 0;\n    if (b) {", "    if (p->node.unk2 >= 0) {")))
# ac8_wh: in abl_w2b3fix.py (wh)
A.append(("ac8_idptr", "func_80041AC8", FD3, lambda b: sub(sub(b, "  id_ptr = &arg0->unk_08;\n", ""), "D_80094E08[*id_ptr]", "D_80094E08[arg0->unk_08]")))
A.append(("bf4_one", "func_80041BF4", FD3, lambda b: sub(sub(b, "  one = 1;\n", ""), "== one)", "== 1)")))
A.append(("bf4_rect", "func_80041BF4", FD3, lambda b: sub(b, "  s16 rect[8];\n", "  s16 rect[4];\n")))
A.append(("bf4_wrap", "func_80041BF4", FD3, lambda b: sub(b, "    do { xoff = 0x80; yoff = 0; } while (0);\n", "    xoff = 0x80;\n    yoff = 0;\n")))
A.append(("ad0_sound", "func_80048AD0", F68, lambda b: re.sub(r"\(sound = 0; sound < 0x11; sound\+\+\)", "(k = 0; k < 0x11; k++)",
                                                              re.sub(r"sound \* 0x68", "k * 0x68", sub(sub(b, "    s32 sound;\n", "    s32 sound;\n    s32 k;\n"),
                                                                                                        "= sound;\n        *(s16 *)(q - 6", "= k;\n        *(s16 *)(q - 6")))))
A.append(("604_s", "func_80054604", FAB, lambda b: re.sub(r"\bs->", "D_800EFAE8.", sub(b, "    Unk800EFAE8Ctrl *s = &D_800EFAE8;\n", ""))))
A.append(("90c_s", "func_8005490C", FAB, lambda b: re.sub(r"\bs->", "D_800EFAE8.", sub(b, "    Unk800EFAE8Ctrl *s = &D_800EFAE8;\n", ""))))


def p90c_player(b):
    b = sub(b, "    Unk80045878Obj *player;\n", "    Unk80045878Obj *player;\n    Unk80045878Obj *p0;\n    Unk80045878Obj *p1;\n")
    b = sub(b, "        player = func_8004153C(0);\n        if (player != 0) {\n            func_8003FFC4(player);",
            "        p0 = func_8004153C(0);\n        if (p0 != 0) {\n            func_8003FFC4(p0);")
    b = sub(b, "        player = func_8004153C(1);\n        if (player != 0) {\n            func_8003FFC4(player);",
            "        p1 = func_8004153C(1);\n        if (p1 != 0) {\n            func_8003FFC4(p1);")
    return b


A.append(("90c_player", "func_8005490C", FAB, p90c_player))


def p90c_rotz(b):
    b = sub(b, "    s32 rot_z;\n", "    s32 rot_z;\n    s32 rot_z2;\n")
    i = b.rindex("rot_z = (vec.vz * c - vec.vx * sn) >> 12;")
    b = b[:i] + b[i:].replace("rot_z = (vec.vz", "rot_z2 = (vec.vz", 1).replace("vec.vz = rot_z;", "vec.vz = rot_z2;", 1)
    return b


A.append(("90c_rotz", "func_8005490C", FAB, p90c_rotz))

os.makedirs("tmp/w2/abl3", exist_ok=True)
rows = []
for name, fn, path, t in A:
    p = "tmp/w2/abl3/%s.c" % name
    open(p, "w", encoding="utf-8", newline="\n").write(t(body(path, fn)))
    rows.append("%s %s %s" % (name, fn, p))
open("tmp/w2/abl3/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
print(len(rows), "candidates")
