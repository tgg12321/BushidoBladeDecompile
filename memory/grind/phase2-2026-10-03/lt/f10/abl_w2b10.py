#!/usr/bin/env python3
# Ablations for w2b10 (run from the repo root with the batch applied): every FAKE in func_80040D48, undone in
# place (the w2b3 / w2b3fix forms, the frame reads now by element). Writes tmp/w2/abl10/<name>.c + list.txt for
# ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl10/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub

F, FN = "src/main/31548.c", "func_80040D48"
A = []
A.append(("d48_s4", lambda b: re.sub(r"\bs4\b", "ent", sub(sub(b, "    Unk80045878Obj *s4;\n", ""), "    s4 = ent;\n", ""))))
A.append(("d48_s5first", lambda b: sub(sub(b, "    s5 = s4->unk_2C;\n", ""), "        s5->node.unk6 = 0;\n",
                                       "        s4->unk_2C[0].node.unk6 = 0;\n")))
A.append(("d48_s1", lambda b: re.sub(r"\bs1\b(?!p)", "arg4", sub(sub(b, "    s16 *s1;\n", ""), "        s1 = arg4;\n", ""))))
A.append(("d48_p58", lambda b: sub(b, "    {\n        s32 *p58 = &s3->unk58;\n        *p58 = (s32)s4->unk_18F4;\n    }\n",
                                   "    s3->unk58 = (s32)s4->unk_18F4;\n")))
A.append(("d48_idx", lambda b: sub(sub(b, "            idx = *tbl;\n            a4p->node.xf.rot.vy", "            a4p->node.xf.rot.vy"),
                                   "            idx = *tbl;\n            a4p->node.xf.rot.vz", "            a4p->node.xf.rot.vz")))


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


A.append(("d48_s0", d48_counters))


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl10", exist_ok=True)
    rows = []
    for name, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl10/%s.c" % name
        b0 = body(F, FN)
        try:
            b1 = t(b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, FN, p))
    open("tmp/w2/abl10/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
