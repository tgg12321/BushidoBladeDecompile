"""Apply the func_80020E74 data-model edits to a tree.

usage: python tmp/func_80020E74/apply_dm.py <out_dir> [--inplace] [--c4 scalar|array|struct]
  --c4 scalar (default): D_800A38C4 / D_800A38C6 stay two u16 scalars (the proposed model).
  --c4 array / struct: the single-object forms, measured failing (func_80020CDC / func_80020D38).
  default: read include/code6cac.h, src/{code6cac_tu2,code6cac,ings}.c from the repo,
           write edited copies flat into <out_dir> (harness use).
  --inplace: edit the repo files themselves (landing use; hold the landing lock)."""
import sys
from pathlib import Path

ROOT = next(q for q in Path(__file__).resolve().parents if (q / "bb2.ld").exists())
inplace = "--inplace" in sys.argv
c4 = sys.argv[sys.argv.index("--c4") + 1] if "--c4" in sys.argv else "scalar"
out = Path(sys.argv[1])

HDR = [
("extern s32 menuDat;\n",
 "/* menuDat: model id -> BBM file name, ended by a zero id (0x8008DCCC..0x8008DD5B,\n"
 " * asm/data/7D920.data.s dlabel menuDat). func_80020E74 loads the model of entry n\n"
 " * from CD file n + 2. */\n"
 "typedef struct {\n"
 "    s32 id;\n"
 "    char *name;\n"
 "} MenuDatEntry;\n"
 "extern MenuDatEntry menuDat[18];\n"),
("typedef struct {\n    u8 pad00[0x14];\n    s16 f14;\n",
 "typedef struct {\n"
 "    u8 pad00[3];\n"
 "    u8 unk_03;                     /* D_801027B0[ch][0] = record + 0x6C + (unk_03 - 1) * 6 (func_80020E74) */\n"
 "    s32 unk_04[4];                 /* D_801027B0[ch][1 + k] = record + unk_04[k] (func_80020E74) */\n"
 "    s16 f14;\n"),
("    u8  unk_48[0x4A - 0x48];\n",
 "    u16 unk_48;                    /* model id (func_80020E74); func_80021280 finds it in D_800A38C4 */\n"),
("extern u8 D_800A38C0;\nextern u8 D_800A38C1;\n",
 "extern u8 D_800A38C0[2];          /* per player: character of the D_800A3888 motion set (0xFF = none) */\n"),
("extern u8 D_8008DB1C;\n",
 "extern u16 D_8008DB1C[27][8];      /* [unk_0A][unk_0E] -> PracticeMenuRec.unk_48 model id (func_80020E74) */\n"),
("extern s16 D_80101F10;\n", ""),
("extern s16 D_8010235C;\n", ""),
]
TU2 = [
("""    D_800A38C1 = 0xFF;
    D_800A38C0 = 0xFF;
""", """    D_800A38C0[1] = 0xFF;
    D_800A38C0[0] = 0xFF;
"""),
("""        func_8001979C(1, D_801027C0);
""", """        func_8001979C(1, D_801027B0[0][4]);
"""),
("""        func_8001979C(2, D_801027D4);
""", """        func_8001979C(2, D_801027B0[1][4]);
"""),
("""    db1c = &D_8008DB1C;
    base = db1c + (*(s16 *)((u8 *)ptr + 0xA) * 16);
    val = *(u16 *)(base + *(s16 *)((u8 *)ptr + 0xE) * 2);
""", """    val = D_8008DB1C[*(s16 *)((u8 *)ptr + 0xA)][*(s16 *)((u8 *)ptr + 0xE)];
"""),
("""    u8 *base;
    s32 *ptr;
    s32 val;
    s32 i;
    u8 *db1c;
""", """    s32 *ptr;
    s32 val;
    s32 i;
"""),
]
EDITS = {"include/code6cac.h": HDR, "src/code6cac_tu2.c": TU2, "src/code6cac.c": [], "src/ings.c": []}

if c4 != "scalar":
    if c4 == "array":
        decl, e0, e1, ptr = ("extern u16 D_800A38C4[2];\n", "D_800A38C4[0]", "D_800A38C4[1]",
                             "u16 *v1 = D_800A38C4;")
    else:
        decl, e0, e1, ptr = ("typedef struct { u16 slot0; u16 slot1; } LoadedIds;\nextern LoadedIds D_800A38C4;\n",
                             "D_800A38C4.slot0", "D_800A38C4.slot1", "u16 *v1 = &D_800A38C4.slot0;")
    HDR.append(("extern u16 D_800A38C6;\n", decl))
    EDITS["src/code6cac.c"].append(("extern u16 D_800A38C4;\n", ""))
    EDITS["src/ings.c"].append(("    D_800A38C6 = 0;\n", f"    {e1} = 0;\n"))
    TU2[:0] = [
        ("extern u16 D_800A38C4;\n", ""),
        ("    D_800A38C6 = (u16)0xFFFF;\n", f"    {e1} = (u16)0xFFFF;\n"),
        ("    u16 *v1 = (u16 *)&D_800A38C4;\n", f"    {ptr}\n"),
    ]
    TU2.append(("""    D_800A3880 = 0;
    D_800A38C6 = 0;
    D_800A38C4 = 0;
""", f"""    D_800A3880 = 0;
    {e1} = 0;
    {e0} = 0;
"""))
    TU2.append(("    if (D_800A38C4) {\n", f"    if ({e0}) {{\n"))
    TU2.append(("    if (D_800A38C6) {\n", f"    if ({e1}) {{\n"))
    TU2.append(("    if (D_800A38C6 == 0xFFFF) {\n", f"    if ({e1} == 0xFFFF) {{\n", 2))
    TU2.append(("    D_800A38C6 = 0;\n}\n", f"    {e1} = 0;\n}}\n"))

for rel, edits in EDITS.items():
    p = ROOT / rel
    t = p.read_text(encoding="utf-8")
    for e in edits:
        old, new = e[0], e[1]
        want = e[2] if len(e) > 2 else 1
        n = t.count(old)
        assert n == want, f"{rel}: expected {want} match(es), got {n}: {old[:60]!r}"
        t = t.replace(old, new)
    dst = p if inplace else out / Path(rel).name
    dst.parent.mkdir(parents=True, exist_ok=True)
    with open(dst, "w", encoding="utf-8", newline="\n") as f:
        f.write(t)
    print("wrote", dst)
