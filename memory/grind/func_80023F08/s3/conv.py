"""Convert the scaffold (candidate-local mirror types) into the header-typed body.
usage: python conv.py <scaffold.c> <body_out.c>"""
import re
import sys

src, dst = sys.argv[1:3]
s = open(src).read()
# drop the scaffold-local type block and externs (everything before the first extern of D_8008E0BC)
s = s[s.index("extern s16 D_8008E0BC"):]


def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (n, old[:90])
    s = s.replace(old, new)


s = re.sub(r"(pose\[[01]\]|unk_290)\.unk([0-9A])\b", lambda m: f"{m.group(1)}.unk_0{m.group(2)}", s)
s = re.sub(r"unk_50->unk([0-9A])\b", lambda m: f"unk_50->unk_0{m.group(1)}", s)
s = s.replace("SPAD23", "SPAD").replace("Pose23", "MotionFrame").replace("Move23", "MoveScript")

rep("rec = (R23 *)&g_practice_menu_table[arg0];", "rec = &g_practice_menu_table[arg0];")
rep("    R23 *rec;\n", "    PracticeMenuRec *rec;\n")
rep("rec->unk_24 = *(PadState *)arg1;", "rec->unk_24 = *pad;")
rep("void func_80023F08(s32 arg0, s32 arg1) {", "void func_80023F08(s32 arg0, PadState *pad) {")
rep("((R23 *)g_practice_menu_table)[D_800A38AE].unk_4A", "g_practice_menu_table[D_800A38AE].unk_4A")
rep("((MotionFrame **)&D_800A3888)[arg0][rec->unk_40]", "D_800A3888[arg0][rec->unk_40]")
for t in ("DA94", "DA50", "DAD8"):
    s = s.replace(f"(&D_8008{t})[rec->unk_0A]", f"D_8008{t}[rec->unk_0A]")
rep("D_800A36D8 = (s32)rec->unk_7C;", "D_800A36D8 = rec->unk_7C;")
rep("D_800A36D8 = (s32)mr;", "D_800A36D8 = mr;")
rep("(PracticeMenuRec *)rec", "rec", 4)
rep("MATRIX **pd = (MATRIX **)game_GetPlayerData(arg0);", "MATRIX **pd = game_GetPlayerData(arg0);")
rep("func_800204C0((u8 *)rec);", "func_800204C0(rec);")
rep("func_800207C8((u8 *)rec, ", "func_800207C8(rec, ")

# dispatch tables: one block-local u16 pointer per site
rep("""        rec->unk_31A = 0;
        func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E))[rec->unk_286], (u8 *)&rec->unk_5E), rec->unk_5E);""",
    """        rec->unk_31A = 0;
        table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
        func_80021A98(arg0, func_80021424((u8 *)rec, table[rec->unk_286], (u8 *)&rec->unk_5E), rec->unk_5E);""")
rep("""            rec->unk_31A = 0;
            func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E))[0xD], (u8 *)&rec->unk_5E), rec->unk_5E);""",
    """            rec->unk_31A = 0;
            table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
            func_80021A98(arg0, func_80021424((u8 *)rec, table[0xD], (u8 *)&rec->unk_5E), rec->unk_5E);""")
rep("""        func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E))[0x18], (u8 *)&rec->unk_5E), rec->unk_5E);""",
    """        table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
        func_80021A98(arg0, func_80021424((u8 *)rec, table[0x18], (u8 *)&rec->unk_5E), rec->unk_5E);""")
rep("tbl = func_80021424(", "table = func_80021424(")
rep("tbl[rec->unk_94 != 0 ? 0x17 : 0x18]", "table[rec->unk_94 != 0 ? 0x17 : 0x18]")
rep("    u16 *tbl;\n", "    u16 *table;\n")

open(dst, "w", newline="\n").write(s)
