"""Item-5 evidence for func_80031B24 (Q117): apply the one-object s16[3] spelling to the CURRENT tree (bb2.h +
17AFC.c, all four users), for measuring only; `restore` puts the saved copies back."""
import re, shutil, sys

FILES = ["include/bb2.h", "src/main/17AFC.c"]
if sys.argv[1:] == ["restore"]:
    for f in FILES:
        shutil.copyfile("tmp/w2/q117_save/" + f.replace("/", "_"), f)
    print("restored")
    sys.exit()
import os
os.makedirs("tmp/w2/q117_save", exist_ok=True)
for f in FILES:
    shutil.copyfile(f, "tmp/w2/q117_save/" + f.replace("/", "_"))
h = open(FILES[0], encoding="utf-8").read()
h = h.replace("extern s16 D_800A37E8;\nextern s16 D_800A37EA;\nextern s16 D_800A37EC;\n", "extern s16 D_800A37E8[3];\n")
open(FILES[0], "w", encoding="utf-8", newline="\n").write(h)
s = open(FILES[1], encoding="utf-8").read()
s = re.sub(r"\bD_800A37EA\b", "D_800A37E8[1]", s)
s = re.sub(r"\bD_800A37EC\b", "D_800A37E8[2]", s)
s = re.sub(r"(?<![&\w])D_800A37E8\b(?!\[)(?=\s*[-=*+])", "D_800A37E8[0]", s)
s = re.sub(r"&D_800A37E8\b(?!\[)", "D_800A37E8", s)
open(FILES[1], "w", encoding="utf-8", newline="\n").write(s)
print("applied s16[3]")
