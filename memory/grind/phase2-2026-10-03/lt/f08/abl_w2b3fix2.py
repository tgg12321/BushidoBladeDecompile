#!/usr/bin/env python3
# rev-w2b3 re-review: the second in-place sweep of batch 3's 40 bodies for named temps and staged reads
# computed ahead of a store or call (the delta / v0_val / texA class). Run from the repo root with the
# batch applied; writes tmp/w2/abl3g/<name>.c + list.txt for run_abl_w2b3.ps1 -List tmp/w2/abl3g/list.txt.
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
A = []
# the three the reviewer measured
A.append(("tex", "func_80040594", F309, lambda b: sub(sub(sub(b,
    "        s32 *texA = (s32 *)((s32)rmd + (((u32)rmd[1] >> 2) << 2));\n        s32 *texB;\n", ""),
    "        texB = (s32 *)((s32)rmd + (((u32)rmd[4] >> 2) << 2));\n", ""),
    "        func_80044010(texA, a0->unk_14);\n        func_80044010(texB, a0->unk_16);\n",
    "        func_80044010((s32 *)((s32)rmd + (((u32)rmd[1] >> 2) << 2)), a0->unk_14);\n"
    "        func_80044010((s32 *)((s32)rmd + (((u32)rmd[4] >> 2) << 2)), a0->unk_16);\n")))


def v1val(b):
    b = sub(b, "  v1_val = (u16) (*var_s0);\n\n  if ((*var_s0) >= 0)", "\n  if ((*var_s0) >= 0)")
    b = sub(b, "      rect[0] = v1_val + var_s3;\n", "      rect[0] = (u16) (*var_s0) + var_s3;\n")
    b = sub(b, "      v1_val = (u16) (*var_s0);\n", "")
    return sub(b, "  u16 v1_val;\n", "")


A.append(("v1val", "func_80041AC8", F31D, v1val))
A.append(("bf4_off", "func_80041BF4", F31D, lambda b: sub(sub(sub(b, "    s32 off = idx << 5;\n    idx++;\n", ""),
    "LoadImage((s32)rect, (s32)((u8 *)&g_gpu_store_buf + off));", "LoadImage((s32)rect, (s32)((u8 *)&g_gpu_store_buf + (idx << 5)));"),
    "    DrawSync(0);\n    tbl += 2;\n", "    DrawSync(0);\n    idx++;\n    tbl += 2;\n")))
# the re-sweep
A.append(("f824_obj", "func_8003F824", F2B, lambda b: sub(sub(sub(b, "            obj = arg0->unk_1A34[i];\n", ""),
    "            rec->obj = obj;\n", "            rec->obj = arg0->unk_1A34[i];\n"),
    "            obj->node.unk0 = 0xD;\n", "            arg0->unk_1A34[i]->node.unk0 = 0xD;\n")))
A.append(("fa24_mode", "func_8003FA24", F2B, lambda b: sub(b.replace("        mode = ((s16)flags >> 3) & 3;\n", ""),
    "D_80094AEC[mode]", "D_80094AEC[((s16)flags >> 3) & 3]", 2)))
A.append(("fa24_count", "func_8003FA24", F2B, lambda b: sub(b, "    count = *src;\n    init.count = count;\n",
    "    init.count = count = *src;\n")))
A.append(("594_player", "func_80040594", F309, lambda b: sub(b.replace("        s32 player = a0->unk_04;\n", ""),
    "        if (player == 1) goto case_1;\n        if (player >= 2) goto done_cases;\n        if (player != 0) goto done_cases;\n",
    "        if (a0->unk_04 == 1) goto case_1;\n        if (a0->unk_04 >= 2) goto done_cases;\n        if (a0->unk_04 != 0) goto done_cases;\n")))
A.append(("b44_t2", "func_80040B44", F309, lambda b: re.sub(r"\bt2\b", "a0_val", sub(sub(b, "            s32 t2;\n", ""), "            t2 = a0_val;\n", ""))))
A.append(("cb8_w", "func_80040CB8", F309, lambda b: sub(sub(b, "                u16 w = arg0->unk_16;\n", ""),
    "                *(s16 *)(ent - 0x54) = w;\n", "                *(s16 *)(ent - 0x54) = arg0->unk_16;\n")))
A.append(("688_else", "func_80041688", F31D, lambda b: sub(b, """        r = player->unk_18.byte[0];
        g = player->unk_18.byte[1];
        b = player->unk_18.byte[2];
        func_80041398(b | ((r << 16) | (g << 8)));
""", """        func_80041398(player->unk_18.byte[2] | ((player->unk_18.byte[0] << 16) | (player->unk_18.byte[1] << 8)));
""")))
A.append(("688_if", "func_80041688", F31D, lambda b: sub(b, """        r = player->unk_18.byte[0];
        g = player->unk_18.byte[1];
        b = player->unk_18.byte[2];
        v = math_Grayscale3(b, g, r);
""", """        v = math_Grayscale3(player->unk_18.byte[2], player->unk_18.byte[1], player->unk_18.byte[0]);
""")))
A.append(("604_n", "func_80054604", F3AB, lambda b: sub(sub(sub(b, "        n = (s->unk4 & 0x3F) - 1;\n", ""),
    "            game_StageCleanup(n, a6);\n", "            game_StageCleanup((s->unk4 & 0x3F) - 1, a6);\n"),
    "            game_StageCleanup(n, (s32)D_800A3770);\n", "            game_StageCleanup((s->unk4 & 0x3F) - 1, (s32)D_800A3770);\n")))
A.append(("604_id", "func_80054604", F3AB, lambda b: re.sub(r"\bid\b", "(a0 + 0x131)", sub(b, "    s32 id = a0 + 0x131;\n", ""))))


def main():
    os.makedirs("tmp/w2/abl3g", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        p = "tmp/w2/abl3g/%s.c" % name
        b0 = body(path, fn)
        b1 = t(b0)
        if b1 == b0:
            raise SystemExit("no change for " + name)
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl3g/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
