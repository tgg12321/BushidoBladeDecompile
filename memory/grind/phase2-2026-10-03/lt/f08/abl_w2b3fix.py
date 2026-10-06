#!/usr/bin/env python3
# rev-w2b3 fixes: the in-place B4 sweep of batch 3's 40 moved bodies (run from the repo root with the
# fixed batch applied). Each candidate undoes one construct in place (a holder's expression substituted
# where the holder was read; no store or call moves). Writes tmp/w2/abl3f/<name>.c + list.txt for
# run_abl_w2b3.ps1 -List tmp/w2/abl3f/list.txt.
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

F2B, F309, F315, F31D, F350, F368, F3AB = ("src/main/2B344.c", "src/main/309CC.c", "src/main/31548.c",
                                          "src/main/31D3C.c", "src/main/35000.c", "src/main/368E4.c",
                                          "src/main/3AB48.c")


def drop_fake(b, start):
    """remove the FAKE comment that begins with `start` (it may span lines)"""
    i = b.index(start)
    j = b.index("*/", i) + 2
    k = b.rindex("\n", 0, i) + 1
    return b[:k] + b[j + 1:]


A = []
A.append(("wh", "func_80041AC8", F31D, lambda b: sub(sub(sub(drop_fake(b, "/* FAKE: constant holders for the 16 x 1"),
    "    s32 w = 0x10;\n    s32 h = 1;\n", ""), "      rect[2] = w;\n", "      rect[2] = 0x10;\n"), "      rect[3] = h;\n", "      rect[3] = 1;\n")))
A.append(("v0val", "func_80041AC8", F31D, lambda b: sub(sub(sub(drop_fake(b, "/* FAKE: V0VAL") if "V0VAL" in b else b,
    "      u16 v0_val;\n", ""), "      v0_val = (u16) var_s0[1];\n", ""), "      rect[1] = v0_val + var_s2;\n",
    "      rect[1] = (u16) var_s0[1] + var_s2;\n")))
A.append(("base", "func_800408F8", F309, lambda b: sub(sub(b, "        Unk80045878Node *base = p;\n", ""),
    "                p->node.unkC = &base[v1].node;\n", "                p->node.unkC = &a0->unk_2C[v1].node;\n")))
A.append(("a1", "func_80040B44", F309, lambda b: sub(b, "            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);\n", "            a1 = &seen[a3];\n")))
A.append(("out3", "func_80041188", F315, lambda b: sub(sub(sub(sub(b, "    MATRIX *out3;\n", ""), "    out3 = a4 + 1;\n", ""),
    "math_RotMatrixZYX(&buf, out3);", "math_RotMatrixZYX(&buf, out2);"),
    "func_800523E0(a4, out3, a3, &stptr2->node.work);", "func_800523E0(a4, out2, a3, &stptr2->node.work);")))
A.append(("off", "func_80041188", F315, lambda b: sub(b, "        offset = offset + (s32) a2;\n        p = (u16 *) offset;\n",
    "        p = (u16 *) (offset + (s32) a2);\n")))
A.append(("idx", "func_80045AA4", F350, lambda b: sub(b, "        idx = 3 * ptr->unk_04 + 1;\n        snd_VabFakeOpen(a1, idx);\n",
    "        s32 idx2 = 3 * ptr->unk_04 + 1;\n        snd_VabFakeOpen(a1, idx2);\n")))
A.append(("n", "func_8003FA24", F2B, lambda b: sub(sub(sub(b,
    "    for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {\n", "    count = *src++;\n    while (count != 0) {\n"),
    "        *packet++ = 0;\n    }\n", "        *packet++ = 0;\n        count = *src++;\n    }\n"), "    s32 n;\n", "")))
# the sweep
A.append(("594_player", "func_80040594", F309, lambda b: sub(b, "        if (func_8003E2A0() != player) goto case_1_else;\n",
    "        if (func_8003E2A0() != 1) goto case_1_else;\n")))
A.append(("594_fb", "func_80040594", F309, lambda b: sub(b, """        s32 flags = a0->unk_00.word & (s32)0xFFE0FFFF;
        s32 bits = (g_player_char_ids[a0->unk_04] & 0x1F) << 16;
        a0->unk_00.word = flags | bits;
""", """        a0->unk_00.word = (a0->unk_00.word & (s32)0xFFE0FFFF) | ((g_player_char_ids[a0->unk_04] & 0x1F) << 16);
""")))
A.append(("594_cast", "func_80040594", F309, lambda b: sub(b, "& (s32)0xFFE0FFFF;", "& 0xFFE0FFFF;")))
A.append(("594_mid", "func_80040594", F309, lambda b: sub(sub(sub(sub(sub(b,
    "        s32 prev_off = ((u32)rmd[count - 1] >> 2) << 2;\n", ""), "        s32 next_off = ((u32)rmd[count + 1] >> 2) << 2;\n", ""),
    "        ptr = (s32 *)((s32)rmd + prev_off);\n", "        ptr = (s32 *)((s32)rmd + (((u32)rmd[count - 1] >> 2) << 2));\n"),
    "(s32 *)((s32)rmd + next_off)", "(s32 *)((s32)rmd + (((u32)rmd[count + 1] >> 2) << 2))"),
    "        idx = a0->unk_04 * 3 + 1;\n        ptr = (s32 *)((s32)ptr + func_8005C2A8(ptr, idx,",
    "        ptr = (s32 *)((s32)ptr + func_8005C2A8(ptr, a0->unk_04 * 3 + 1,")))
A.append(("d48_idx", "func_80040D48", F315, lambda b: sub(sub(b, "            idx = *tbl;\n            a4p->node.xf.rot.vy", "            a4p->node.xf.rot.vy"),
    "            idx = *tbl;\n            a4p->node.xf.rot.vz", "            a4p->node.xf.rot.vz")))
A.append(("d48_scaled", "func_80040D48", F315, lambda b: sub(sub(sub(b, "        s32 scaled;\n", ""),
    "        scaled = (s3->node.work.t[1] * s4->unk_12) >> 12;\n", ""),
    "        s3->node.work.t[1] = scaled;\n", "        s3->node.work.t[1] = (s3->node.work.t[1] * s4->unk_12) >> 12;\n")))
A.append(("sci_val", "player_SetCharId", F31D, lambda b: sub(sub(b, "        s32 val = ptr->unk_00.half[1];\n", ""),
    "        if ((val & 0x1F) != a1) {\n", "        if ((ptr->unk_00.half[1] & 0x1F) != a1) {\n")))
A.append(("650_val", "func_80041650", F31D, lambda b: sub(b, "        s32 val = ptr->unk_00.half[1];\n        return val & 0x1F;\n",
    "        return ptr->unk_00.half[1] & 0x1F;\n")))
A.append(("ad0_idx", "func_80048AD0", F368, lambda b: sub(sub(b, "    idx = temp_v0->unk_08;\n", ""),
    "    sound = (&D_80099BCC)[idx];\n", "    sound = (&D_80099BCC)[temp_v0->unk_08];\n")))
A.append(("ad0_delta", "func_80048AD0", F368, lambda b: sub(sub(b, "    delta = (s32)(p - base);\n", ""),
    "    func_800468B0(delta + 0x6E8);\n", "    func_800468B0((s32)(p - base) + 0x6E8);\n")))
A.append(("ad0_q", "func_80048AD0", F368, lambda b: b.replace("    q = p + 0xA;\n", "").replace("(q - 8 + ", "(p + 2 + ").replace(
    "(q - 6 + ", "(p + 4 + ").replace("(q - 9 + ", "(p + 1 + ").replace("*(s16 *)(q + sound", "*(s16 *)(p + 0xA + sound")))
A.append(("90c_vy", "func_8005490C", F3AB, lambda b: sub(b, "            vec.vy = frame[0];\n            vec.vy = (vec.vy * player->unk_12) >> 12;\n",
    "            vec.vy = (frame[0] * player->unk_12) >> 12;\n")))

os.makedirs("tmp/w2/abl3f", exist_ok=True)
rows = []
for name, fn, path, t in A:
    p = "tmp/w2/abl3f/%s.c" % name
    b0 = body(path, fn)
    b1 = t(b0)
    if b1 == b0:
        raise SystemExit("no change for " + name)
    open(p, "w", encoding="utf-8", newline="\n").write(b1)
    rows.append("%s %s %s" % (name, fn, p))
open("tmp/w2/abl3f/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
print(len(rows), "candidates")
