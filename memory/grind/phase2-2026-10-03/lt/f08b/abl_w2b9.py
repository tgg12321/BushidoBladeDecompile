#!/usr/bin/env python3
# Ablations for w2b9 (run from the repo root with the batch applied): every FAKE in the four moved 309CC
# bodies, undone in place, the new member cursor's alternatives, and the sweep of their holders.
# Writes tmp/w2/abl9/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl9/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, inline, drop_fake

F = "src/main/309CC.c"
A = []

# ---- carried FAKEs
A.append(("8f8_base", "func_800408F8", F, lambda b: sub(sub(b, "        Unk80045878Node *base = p;\n", ""),
    "                p->node.unkC = &base[v1].node;\n", "                p->node.unkC = &a0->unk_2C[v1].node;\n")))
A.append(("8f8_neg1", "func_800408F8", F, lambda b: sub(sub(sub(b, "        s32 neg1 = -1;\n", ""),
    "p->node.unk2 = neg1;", "p->node.unk2 = -1;"), "if (v1 != neg1) {", "if (v1 != -1) {")))
A.append(("b44_a1", "func_80040B44", F, lambda b: sub(b, "            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);\n",
    "            a1 = &seen[a3];\n")))
A.append(("b44_t2", "func_80040B44", F, lambda b: re.sub(r"\bt2\b", "a0_val",
    sub(sub(b, "            s32 t2;\n", ""), "            t2 = a0_val;\n", ""))))
A.append(("cb8_consts", "func_80040CB8", F, lambda b: sub(sub(sub(sub(sub(b,
    "    do {\n        none = -1;\n        kind = 3;\n        one = 1;\n    } while (0);\n", ""),
    "if (id != none) {", "if (id != -1) {"), "slot->node.unk0 = kind;", "slot->node.unk0 = 3;"),
    "= one;", "= 1;"), "    s32 none;\n    s32 kind;\n    s32 one;\n", "")))
A.append(("cb8_wrap1", "func_80040CB8", F, lambda b: sub(b,
    "    do {\n        none = -1;\n        kind = 3;\n        one = 1;\n    } while (0);\n",
    "    none = -1;\n    kind = 3;\n    one = 1;\n")))
A.append(("cb8_wrap2", "func_80040CB8", F, lambda b: sub(sub(b,
    "    do {\n        ent = (s32)&arg0->unk_8B4[0].unk58;\n", "    ent = (s32)&arg0->unk_8B4[0].unk58;\n    {\n"),
    "        if (i < 0x12) goto loop;\n    } while (0);\n", "        if (i < 0x12) goto loop;\n    }\n")))
A.append(("cb8_w", "func_80040CB8", F, lambda b: sub(sub(b, "                u16 w = arg0->unk_16;\n", ""),
    "                *(s16 *)(ent - 0x54) = w;\n", "                *(s16 *)(ent - 0x54) = arg0->unk_16;\n")))
# ---- the unk_08 member cursor: a record pointer, and the record index
A.append(("cb8_recptr", "func_80040CB8", F, lambda b: sub(sub(sub(sub(b, "    s16 *tbl;\n", "    Unk80094B96Rec *tbl;\n"),
    "    tbl = &D_80094B96[0].unk_08;\n", "    tbl = D_80094B96;\n"), "        id = *tbl;\n", "        id = tbl->unk_08;\n"),
    "        tbl = (s16 *)((u8 *)tbl + sizeof(Unk80094B96Rec));\n", "        tbl++;\n")))
A.append(("cb8_idx", "func_80040CB8", F, lambda b: sub(sub(sub(sub(b, "    s16 *tbl;\n", ""),
    "    tbl = &D_80094B96[0].unk_08;\n", ""), "        id = *tbl;\n", "        id = D_80094B96[i].unk_08;\n"),
    "        tbl = (s16 *)((u8 *)tbl + sizeof(Unk80094B96Rec));\n", "")))
# ---- sweep
A.append(("8f8_idx", "func_800408F8", F, lambda b: sub(sub(sub(b, "    s16 idx;\n", ""), "    idx = a0->unk_08;\n", ""),
    "    if (idx < count) {\n        a0->unk_12 = D_80094C68[idx];\n",
    "    if (a0->unk_08 < count) {\n        a0->unk_12 = D_80094C68[a0->unk_08];\n")))
A.append(("8f8_i", "func_800408F8", F, lambda b: sub(b, "            p++;\n        } while (++i < 21);\n",
    "            p++;\n            i++;\n        } while (i < 21);\n")))
A.append(("b44_v1", "func_80040B44", F, lambda b: sub(sub(sub(b, "    s32 *v1;\n", ""), "    v1 = (s32 *)arg0->unk_1C;\n", ""),
    "    t3 = (u16 *)((u8 *)v1 + v1[2]);\n", "    t3 = (u16 *)((u8 *)arg0->unk_1C + ((s32 *)arg0->unk_1C)[2]);\n")))


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl9", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl9/%s.c" % name
        b0 = body(path, fn)
        try:
            b1 = t(b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        if b1 == b0:
            print("SKIP %s: no change" % name)
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl9/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
