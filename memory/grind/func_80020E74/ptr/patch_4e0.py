"""patch_4e0.py <dir>: typed func_800224E0 (PracticeMenuRec *, D_8008EB1C[][2]) in <dir> copies."""
import sys
from pathlib import Path
d = Path(sys.argv[1])
E = {
"code6cac_tu2.c": [
("""s32 func_800224E0(s32 *arg0) {
    u8 *p;
    u8 *end;
    s32 *ptr;
    s32 val;
    s32 i;

    p = (&D_8008EB1C) + (D_800A384C * 2);
    end = p + 2;
    ptr = (s32 *)(*arg0);
    val = D_8008DB1C[*(s16 *)((u8 *)ptr + 0xA)][*(s16 *)((u8 *)ptr + 0xE)];
""", """s32 func_800224E0(PracticeMenuRec *arg0) {
    u8 *p;
    u8 *end;
    s32 val;
    s32 i;

    p = D_8008EB1C[D_800A384C];
    end = p + 2;
    val = D_8008DB1C[arg0->unk_00->unk_0A][arg0->unk_00->unk_0E];
"""),
("        p->unk_84 = func_800224E0((s32 *)p);\n", "        p->unk_84 = func_800224E0(p);\n"),
],
"code6cac.h": [("extern u8 D_8008EB1C;\n", "extern u8 D_8008EB1C[][2];\n")],
}
for f, edits in E.items():
    p = d / f
    t = p.read_text(encoding="utf-8")
    for o, n in edits:
        assert t.count(o) == 1, (f, o[:50])
        t = t.replace(o, n)
    open(p, "w", newline="\n", encoding="utf-8").write(t)
print("ok")
