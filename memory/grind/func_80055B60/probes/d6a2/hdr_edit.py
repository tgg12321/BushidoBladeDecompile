# hdr_edit.py <code6cac.h>: in-place edits on top of the rejected-p1 patch's header (MATRIX member, D_8008E338 table,
# per-word externs deleted).
import sys
p = sys.argv[1]
h = open(p, encoding='utf-8', newline='').read()

def one(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)

h = one(h, "/* The twelve 0x64-byte object records at 0x80106A78.",
        "/* PsyQ MATRIX layout (include/gte.h), spelled with a local tag for the same\n"
        " * reason as Unk80101DF0Mat below (several TUs typedef MATRIX themselves). */\n"
        "typedef struct { s16 m[3][3]; u16 pad; s32 t[3]; } Obj80106A78Mat;\n"
        "/* The twelve 0x64-byte object records at 0x80106A78.")
h = one(h, "    u8  unk_0C[0x2C - 0x0C];       /* matrix read by func_800300B4 */\n",
        "    Obj80106A78Mat unk_0C;         /* func_8002FF20 builds it (identity, RotMatrixX/Y/Z,\n"
        "                                      MulMatrix0); func_800300B4 reads it */\n")
h = one(h, "extern u8 D_8008E338;\n",
        "extern s8 D_8008E338[27][5];        /* [unk_0A][i] -> PracticeMenuRec.unk_332[i] (func_8003047C);\n"
        "                                       0x8008E338..0x8008E3BE, then one alignment byte */\n")
h = one(h, "extern s16 D_80106A7A;\nextern u8 D_80106A80;\nextern u8 D_80106A82;\n", "")
open(p, 'w', encoding='utf-8', newline='\n').write(h)
print('ok')
