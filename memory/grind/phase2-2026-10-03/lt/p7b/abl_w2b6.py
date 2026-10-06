#!/usr/bin/env python3
# Ablations for w2b6 / P7b (run from the repo root with the batch applied): every FAKE in the 13 moved
# bodies (carried) and the in-place sweep of their holders / aliases / staged reads. Writes
# tmp/w2/abl6/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl6/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, inline, drop_fake

F2B, F9F, F25, F368, F6C = ("src/main/2B344.c", "src/main/9F9C.c", "src/main/25C38.c", "src/main/368E4.c",
                            "src/main/6CF8.c")
A = []


def ret_inline(call):
    """`ret = CALL; D_800A38B4 += ret / 4;` -> `D_800A38B4 += CALL / 4;` (or `+= ret` -> `+= CALL`)"""
    def t(b):
        m = re.search(r"( *)ret = (%s\([^;]*\));\n\1D_800A38B4 \+= ret( / 4)?;\n" % re.escape(call), b)
        if not m:
            raise SystemExit("no ret site for " + call)
        b = b[:m.start()] + "%sD_800A38B4 += %s%s;\n" % (m.group(1), m.group(2), m.group(3) or "") + b[m.end():]
        if not re.search(r"\bret\b", b.split("{", 1)[1].replace("s32 ret;", "")):
            b = b.replace("    s32 ret;\n", "", 1)
        return b
    return t


A.append(("b8e4_ret", "func_8003B8E4", F2B, ret_inline("func_80060544")))
A.append(("b8e4_tmp", "func_8003B8E4", F2B, lambda b: sub(sub(sub(b, "    tmp = D_800A37B8 + 1;\n    D_800A37B8 = tmp;\n    if (tmp < 3) {\n",
    "    D_800A37B8 = D_800A37B8 + 1;\n    if (D_800A37B8 < 3) {\n"), "    s32 tmp;\n", ""), "", "") if False else
    sub(sub(b, "    tmp = D_800A37B8 + 1;\n    D_800A37B8 = tmp;\n    if (tmp < 3) {\n",
        "    D_800A37B8 = D_800A37B8 + 1;\n    if (D_800A37B8 < 3) {\n"), "    s32 tmp;\n", "")))
A.append(("c2c0_ret", "func_8003C2C0", F2B, ret_inline("func_8005FC9C")))
A.append(("c2c0_newval", "func_8003C2C0", F2B, lambda b: sub(b, "            s32 newval = 8;\n            if (D_8008D9EC[stage] != 0) {\n                newval = 9;\n            }\n            D_800A38A4 = newval;\n",
    "            D_800A38A4 = 8;\n            if (D_8008D9EC[stage] != 0) {\n                D_800A38A4 = 9;\n            }\n")))
A.append(("c2c0_stage", "func_8003C2C0", F2B, lambda b: sub(sub(b, "            s32 stage = (s16)D_80101EC8[0].unk_0A;\n", ""),
    "D_8008D9EC[stage]", "D_8008D9EC[(s16)D_80101EC8[0].unk_0A]")))
A.append(("c2c0_a4val", "func_8003C2C0", F2B, lambda b: inline(b, "            u8 a4val = D_800A38A4;\n", "a4val", "D_800A38A4")))
A.append(("c2c0_state", "func_8003C2C0", F2B, lambda b: sub(b, """    {
        s16 state = D_800A3834;
        if (state != 0x13) {
            func_800372C0();
            state = D_800A3834;
            if (state != 0x12) {
                func_800548DC();
            }
        }
    }
""", """    if (D_800A3834 != 0x13) {
        func_800372C0();
        if (D_800A3834 != 0x12) {
            func_800548DC();
        }
    }
""")))
A.append(("c560_ret", "func_8003C560", F2B, ret_inline("func_8005E54C")))
A.append(("c560_counter", "func_8003C560", F2B, lambda b: inline(b, "    s32 counter;\n", "counter", "D_800A37B8",
    "    counter = D_800A37B8 + 1;\n    D_800A37B8 = counter;\n").replace("    if (D_800A382D == 2) {", "    D_800A37B8 = D_800A37B8 + 1;\n    if (D_800A382D == 2) {", 1)))
A.append(("c560_id", "func_8003C560", F2B, lambda b: sub(sub(b, "            id = 0xA7;\n            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {\n                id = 0xA8;\n            }\n            func_8005C650(id, 0x7F, 0x7F);\n",
    "            func_8005C650(D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0 ? 0xA8 : 0xA7, 0x7F, 0x7F);\n"), "    s32 id;\n", "")))
A.append(("c560_a4val", "func_8003C560", F2B, lambda b: sub(sub(b, "            a4val = 4;\n            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {\n                a4val = 5;\n            }\n            D_800A38A4 = a4val;\n",
    "            D_800A38A4 = 4;\n            if (D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0) {\n                D_800A38A4 = 5;\n            }\n"), "    u8 a4val;\n", "")))
A.append(("c8b4_ret", "func_8003C8B4", F2B, ret_inline("func_80060768")))


def no_a0a1(b):
    b = sub(b, "    s32 *a0 = &D_800F6608.unk_00.x;\n", "")
    b = sub(b, "    s16 *a1 = &D_800F6608.h10;\n", "")
    b = sub(b, "    a0[0] = 0;\n", "    D_800F6608.unk_00.x = 0;\n")
    b = sub(b, "    *a1 = 0x20;\n", "    D_800F6608.h10 = 0x20;\n")
    return sub(b, "    func_80046BF4(a0, a1, 0x2710);\n", "    func_80046BF4(&D_800F6608.unk_00.x, &D_800F6608.h10, 0x2710);\n")


A.append(("c9a4_a0a1", "func_8003C9A4", F2B, no_a0a1))
A.append(("c9a4_ret", "func_8003C9A4", F2B, ret_inline("func_8005FA98")))
A.append(("cd10_a0a1", "func_8003CD10", F2B, no_a0a1))
A.append(("cd10_ret", "func_8003CD10", F2B, ret_inline("func_800600C8")))
A.append(("ea84_ret", "func_8001EA84", F9F, ret_inline("func_8005FA98")))
A.append(("ea84_base", "func_8001EA84", F9F, lambda b: sub(sub(b, "    base = &D_80101EC8[0];\n    if (D_800A3748 == 0) {\n        base++;\n    }\n    func_8001BC70(base, D_800A37B8 << 3);\n",
    "    func_8001BC70(&D_80101EC8[D_800A3748 == 0], D_800A37B8 << 3);\n"), "    Unk80101EC8Record *base;\n", "")))
A.append(("553c_q", "func_8003553C", F25, lambda b: sub(re.sub(r"    q = g;\n    g \+= 1;\n    AddPrim\(([^;]*), q\);\n", r"    AddPrim(\1, g);\n    g += 1;\n", b), "    POLY_G4 *q;\n", "")))
A.append(("553c_ot", "func_8003553C", F25, lambda b: sub(sub(b, "    ot = g_gpu_ot_ptr + 0x401C;\n", ""), "AddPrim(ot, q);", "AddPrim(g_gpu_ot_ptr + 0x401C, q);").replace("    u8 *ot;\n", "")))


# 6CF8 (granted for P7b): carried FAKEs and the sweep
A.append(("e60_otbase", "func_80016E60", F6C, lambda b: sub(sub(b, "    ot_base = arg0;\n", ""), "DrawOTag(&ot_base->ot[0x1007]);", "DrawOTag(&arg0->ot[0x1007]);")))
A.append(("e60_wrap", "func_80016E60", F6C, lambda b: sub(b, "        do {\n            PutDispEnv(&env->disp);\n            PutDrawEnv(&env->draw);\n        } while (0);\n",
    "        PutDispEnv(&env->disp);\n        PutDrawEnv(&env->draw);\n")))
A.append(("e60_padbits", "func_80016E60", F6C, lambda b: inline(b, "    u32 padbits;\n", "padbits", "g_pad_state.pressed", "        padbits = g_pad_state.pressed;\n")))


def e60_bits(b):
    for op, rhs in (("|=", "bits |= mask;"), ("&=", "bits &= ~mask;")):
        expr = "1 << (u8)(select - 3)" if op == "|=" else "~(1 << (u8)(select - 3))"
        old = ("                u8 shift;\n                s32 mask;\n                s32 bits;\n                func_8005C650(0, 0x7F, 0x7F);\n"
               "                shift = select - 3;\n                mask = 1;\n                mask <<= shift;\n                bits = D_800A3788;\n"
               "                %s\n                D_800A3788 = bits;\n" % rhs)
        new = "                func_8005C650(0, 0x7F, 0x7F);\n                D_800A3788 %s %s;\n" % (op, expr)
        b = sub(b, old, new)
    return b


A.append(("e60_bits", "func_80016E60", F6C, e60_bits))
A.append(("e60_special", "func_80016E60", F6C, lambda b: sub(b, "    limit = 3;\n    if (special != 0) {\n        limit = 6;\n    }\n",
    "    limit = special != 0 ? 6 : 3;\n")))
A.append(("main_tbl", "main", F6C, lambda b: sub(sub(sub(b, "    tbl = D_800A3770;\n", ""), "    D_800A38B4 = (u32 *)tbl[idx];\n", "    D_800A38B4 = (u32 *)D_800A3770[idx];\n"),
    "        s32 prim_base = (s32)tbl[idx];\n", "        s32 prim_base = (s32)D_800A3770[idx];\n")))
A.append(("main_prim", "main", F6C, lambda b: inline(b, "        s32 prim_base = (s32)tbl[idx];\n", "prim_base", "(s32)tbl[idx]")))
A.append(("main_adj", "main", F6C, lambda b: inline(b, "        s32 adj = (s32)D_800A38B4 + 0xFFFECC00u;\n", "adj", "(s32)D_800A38B4 + 0xFFFECC00u")))
A.append(("main_cnt", "main", F6C, lambda b: sub(b, "        s32 cnt = GetRCnt(0xF2000001u);\n        if (cnt >= ((D_800A36F1 - 1) << 8) + 0x80) break;\n",
    "        if (GetRCnt(0xF2000001u) >= ((D_800A36F1 - 1) << 8) + 0x80) break;\n")))


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl6", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl6/%s.c" % name
        b0 = body(path, fn)
        b1 = t(b0)
        if b1 == b0:
            raise SystemExit("no change for " + name)
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl6/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
