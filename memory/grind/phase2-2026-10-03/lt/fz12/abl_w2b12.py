#!/usr/bin/env python3
# Ablations for w2b12 (run from the repo root with the batch applied): the carried FAKE in the moved bodies and
# the sweep of their holders. Writes tmp/w2/abl12/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1.
import os, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, drop_fake

A = []
A.append(("ffc_h2", "func_80048FFC", "src/main/368E4.c", lambda b: sub(sub(drop_fake(b, "/* FAKE: strip two's height"),
    "        s16 h2 = phase;\n", ""), "        rect.h = h2;\n", "        rect.h = phase;\n")))
A.append(("1c8_val2", "func_800421C8", "src/main/31D3C.c", lambda b: sub(sub(sub(sub(b, "    s32 val;\n", "    s32 yaw;\n    s32 pitch;\n"),
    "    val = *p;\n", "    yaw = *p;\n"), "    val = *(s16 *)((u8 *)p + 2);\n", "    pitch = *(s16 *)((u8 *)p + 2);\n"),
    "yaw = val & 0xFFF;\n" * 0 + "    D_800F62E0[4].light[0].yaw = val & 0xFFF;\n    D_800F62E0[1].light[0].yaw = val & 0xFFF;\n    D_800F62E0[0].light[0].yaw = val & 0xFFF;\n",
    "    D_800F62E0[4].light[0].yaw = yaw & 0xFFF;\n    D_800F62E0[1].light[0].yaw = yaw & 0xFFF;\n    D_800F62E0[0].light[0].yaw = yaw & 0xFFF;\n").replace(
    "light[0].pitch = val & 0xFFF;", "light[0].pitch = pitch & 0xFFF;")))
A.append(("1c8_mask", "func_800421C8", "src/main/31D3C.c", lambda b: sub(sub(b, "    val = *p;\n", "    val = *p & 0xFFF;\n"),
    "light[0].yaw = val & 0xFFF;", "light[0].yaw = val;", 3)))


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl12", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl12/%s.c" % name
        b0 = body(path, fn)
        try:
            b1 = t(b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl12/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
