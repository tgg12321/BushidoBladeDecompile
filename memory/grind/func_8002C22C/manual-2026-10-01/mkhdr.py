# Build the header edit for func_8002C22C (+ func_8002C61C cleanup).  Usage: mkhdr.py <in> <out>
import sys
s = open(sys.argv[1], 'rb').read().decode()
def rep(old, new):
    global s
    assert s.count(old) == 1, old
    s = s.replace(old, new)
LEAF = ("/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,\n"
        "   \"12-byte stride per leaf, 6 entries = 72-byte position array\"). */\n"
        "typedef struct {\n    s32 x;\n    s32 y;\n    s32 z;\n} LeafPos;\n")
rep(LEAF, "")
rep("typedef struct { s32 x, y, z; } Vec3i32;\n",
    "typedef struct { s32 x, y, z; } Vec3i32;\n" + LEAF)
rep("    u8  unk_A1[0xB1 - 0xA1];\n",
    "    u8  unk_A1[0xAD - 0xA1];\n"
    "    u8  unk_AD;                    /* != 0: func_8002C61C re-runs func_800283D0 for both records */\n"
    "    u8  unk_AE[0xB1 - 0xAE];\n")
rep("    u8  unk_204[0x24C - 0x204];\n",
    "    u8  unk_204[0x210 - 0x204];\n"
    "    LeafPos unk_210[3];            /* func_8002C61C: copy of scratchpad points 0x1F800000 + idx * 0x24 */\n"
    "    LeafPos unk_234[2];            /* func_8002C61C: copy of scratchpad points 0x1F800048 + idx * 0x18 */\n")
for sym in ("D_801020D8", "D_801020DC", "D_801020E0", "D_801020E4", "D_801020E8", "D_801020EC",
            "D_801020FC", "D_80102100", "D_80102104", "D_80102108", "D_8010210C", "D_80102110"):
    rep("extern s32 %s;\n" % sym, "")
rep("extern u8 D_80101F75;\n", "")
rep("extern u8 D_801023C1;\n", "")
open(sys.argv[2], 'wb').write(s.encode())
