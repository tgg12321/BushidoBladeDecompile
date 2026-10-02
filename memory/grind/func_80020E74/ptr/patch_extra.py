"""patch_extra.py <dir>: D_800A38C4[2] comment; D_800A3864 -> D_800A3860[1]."""
import sys
from pathlib import Path
d = Path(sys.argv[1])
C4 = ("extern u16 D_800A38C4[2];         /* per slot: model id in the D_800A3860[i] buffer (func_80020E74);"
      " [1] = 0xFFFF: func_8001DB9C started a sequence at 0x80190800 (D_800A3860[1]'s buffer);"
      " func_80020D38 calls seq_Reset for it */\n")
E = {
"code6cac_tu2.c": [("    D_800A3864 = (s32)0x80190800;\n", "    D_800A3860[1] = (Tbl800A3860Entry *)0x80190800;\n")],
"code6cac.h": [("extern u16 D_800A38C4[2];\n", C4),
               ("extern s32 D_800A3864;\n", "")],
}
for f, edits in E.items():
    p = d / f
    t = p.read_text(encoding="utf-8")
    for o, n in edits:
        assert t.count(o) == 1, (f, o[:50])
        t = t.replace(o, n)
    open(p, "w", newline="\n", encoding="utf-8").write(t)
print("ok")
