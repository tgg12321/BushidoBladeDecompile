#!/usr/bin/env python3
"""mkv.py <name> : build scratch variant tmp/prc/<name>/ = main's code6cac_tu2.c / code6cac_c2.c / code6cac.h with
round3_full.patch applied (round 2 + func_80021424 respell) plus the L1 extras (PracticeMenuRec members for
func_8001FBE8, its body tmp/prc/func_8001FBE8.<name>.c if present else .r3.c, retired externs)."""
import re
import shutil
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
name = sys.argv[1]
v = root / "tmp/prc" / name
tree = root / "tmp/prc" / f"{name}_tree"
shutil.rmtree(v, ignore_errors=True)
shutil.rmtree(tree, ignore_errors=True)
(tree / "src").mkdir(parents=True)
(tree / "include").mkdir(parents=True)
for p in ("src/code6cac_tu2.c", "src/code6cac_c2.c", "include/code6cac.h", "undefined_syms_auto.txt"):
    shutil.copy(root / p, tree / p)
r = subprocess.run(["git", "apply", "--directory=" + str(tree.relative_to(root)).replace("\\", "/"),
                    "--unsafe-paths", "tmp/func_80021424/round3_full.patch"], cwd=root, capture_output=True, text=True)
if r.returncode:
    sys.exit("patch failed: " + r.stderr)


def sub1(s, a, b, what):
    assert s.count(a) == 1, (what, s.count(a))
    return s.replace(a, b)


def span(s, func):
    m = re.search(rf"^(?!extern)[A-Za-z_][^\n;]*\b{func}\s*\([^;\n]*$", s, re.M)
    i = s.index("{", m.end() - 1 if s[m.end() - 1] == "{" else m.end())
    i += 1
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return m.start(), i


h = (tree / "include/code6cac.h").read_bytes().decode()
h = sub1(h, "    u8  unk_4E[0x5E - 0x4E];\n",
         "    u8  unk_4E[0x50 - 0x4E];\n    s32 *unk_50;                   /* func_8001FAE4's argument (func_8001FBE8) */\n"
         "    u8  unk_54[0x5E - 0x54];\n", "unk_4E")
h = sub1(h, "    u8  unk_60[0x72 - 0x60];\n",
         "    u8  unk_60[0x6A - 0x60];\n    u16 unk_6A;\n    u8  unk_6C[0x72 - 0x6C];\n", "unk_60")
h = sub1(h, "    u8  unk_74[0x78 - 0x74];\n", "    s32 unk_74;\n", "unk_74")
h = sub1(h, "    u8  unk_7A[0x7C - 0x7A];\n", "    s16 unk_7A;\n", "unk_7A")
h = sub1(h, "    u8  unk_92[0x96 - 0x92];\n", "    u8  unk_92[0x94 - 0x92];\n    s16 unk_94;\n", "unk_92")
h = sub1(h, "    u8  unk_26C[0x274 - 0x26C];\n", "    u8  unk_26C[0x272 - 0x26C];\n    u16 unk_272;   /* lhu/sh only (func_8001FBE8 0x8001FFA4/B0) */\n", "unk_26C")
for ext in ("extern s16 D_80101F14;\n", "extern s16 D_80101F42;\n", "extern s16 D_80101F08;\n", "extern s16 D_80101F10;\n"):
    h = sub1(h, ext, "", ext)
(tree / "include/code6cac.h").write_bytes(h.encode())

s = (tree / "src/code6cac_tu2.c").read_bytes().decode()
body = root / "tmp/prc" / f"func_8001FBE8.{name}.c"
if not body.exists():
    body = root / "tmp/prc/func_8001FBE8.r3.c"
a, b = span(s, "func_8001FBE8")
s = s[:a] + body.read_bytes().decode().rstrip("\n") + s[b:]
(tree / "src/code6cac_tu2.c").write_bytes(s.encode())

v.mkdir(parents=True)
for p in ("src/code6cac_tu2.c", "src/code6cac_c2.c", "include/code6cac.h"):
    shutil.copy(tree / p, v / Path(p).name)
print("built", v)
