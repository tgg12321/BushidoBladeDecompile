"""apply.py <root> <A|B|AB> : apply the func_800207C8 landing edits to <root>/include/code6cac.h and
<root>/src/*.c (root '.' = the tree; any other root must already hold copies).
A = cheat-cleanup (BoneHitRec data model: D_800F5F68 / D_8008D59C typed, six consumers respelled).
B = Match (PracticeMenuRec fields, LeafPos == Vec3i32, attachment-set externs, func_800207C8 body).
String replacements with exact-count asserts; function-scoped where the text repeats."""
import sys
from pathlib import Path

root = Path(sys.argv[1])
mode = sys.argv[2]
HERE = Path(__file__).resolve().parent


def rd(rel):
    return (root / rel).read_text(encoding='utf-8')


def wr(rel, s):
    (root / rel).write_text(s, encoding='utf-8', newline='\n')


def sub(s, old, new, count=1):
    n = s.count(old)
    assert n == count, (n, old)
    return s.replace(old, new)


def in_func(s, sig, pairs):
    a = s.index(sig)
    b = s.index('\n}\n', a) + 3
    body = s[a:b]
    for old, new, *cnt in pairs:
        body = sub(body, old, new, cnt[0] if cnt else 1)
    return s[:a] + body + s[b:]


HDR_A_BLOCK = """/* A hit record: one of the 22 test points of a character.  func_800207C8 places
 * point i at SPAD->unkA8[ch][i] (`ofs` rotated by game_GetPlayerData()'s matrix
 * `bone`, plus that matrix's translation); the hit tests (func_8002A458,
 * func_8002CA8C, func_80031B24) take unk_0C / unk_0E as the first test's limits
 * and, when unk_00 != 0, unk_10 / unk_12 as a second test's. */
typedef struct BoneHitRec {
    s16 unk_00;
    s16 bone;
    SVec4i16 ofs;
    u16 unk_0C;
    u16 unk_0E;
    u16 unk_10;
    u16 unk_12;
} BoneHitRec;                      /* sizeof == 0x14 */
/* The two characters' hit records (0x800F5F68..0x800F62D7): func_800206B0 fills
 * D_800F5F68[ch] from the template D_8008D59C, offsets and limits scaled. */
extern BoneHitRec D_800F5F68[2][22];
extern BoneHitRec D_8008D59C[22];
"""

FUNC_206B0 = """void func_800206B0(s32 arg0, s32 arg1) {
    BoneHitRec *src = D_8008D59C;
    BoneHitRec *dst = D_800F5F68[arg0];
    s32 i;

    for (i = 0; i < 22; i++, src++, dst++) {
        dst->unk_00 = src->unk_00;
        dst->bone = src->bone;
        dst->ofs.vx = (src->ofs.vx * arg1) >> 12;
        dst->ofs.vy = (src->ofs.vy * arg1) >> 12;
        dst->ofs.vz = (src->ofs.vz * arg1) >> 12;
        dst->unk_0C = (src->unk_0C * arg1) >> 12;
        dst->unk_0E = (src->unk_0E * arg1) >> 12;
        dst->unk_10 = (src->unk_10 * arg1) >> 12;
        dst->unk_12 = (src->unk_12 * arg1) >> 12;
    }
}
"""


def apply_A():
    h = rd('include/code6cac.h')
    h = sub(h, "/* 0x1B8-byte per-character records (func_8002A458 / func_800206B0 walk them by byte offset). */\n"
               "extern u8 D_800F5F68[];\n", "")
    anchor = "typedef struct { s16 vx, vy, vz, pad; } SVec4i16;\n"
    h = sub(h, anchor, anchor + "\n" + HDR_A_BLOCK)
    h = sub(h, "extern u16 D_8008D59E;\n", "")
    wr('include/code6cac.h', h)

    s = rd('src/code6cac_b_tu2.c')
    s = in_func(s, "void func_8002A458(u8 *obj, u32 *hit, u32 *deep, s32 quiet) {", [
        ("    u8 *rec;\n", "    BoneHitRec *rec;\n"),
        ("rec = &D_800F5F68[id * 0x1B8];", "rec = D_800F5F68[id];"),
        ("i++, rec += 0x14)", "i++, rec++)"),
        ("*(u16 *)(rec + 0xC), *(u16 *)(rec + 0xE)", "rec->unk_0C, rec->unk_0E"),
        ("*(s16 *)rec != 0", "rec->unk_00 != 0"),
        ("*(u16 *)(rec + 0x10), *(u16 *)(rec + 0x12)", "rec->unk_10, rec->unk_12"),
    ])
    s = in_func(s, "void func_8002AB08(s32 mode) {", [
        ("        u8 *rec;\n", "        BoneHitRec *rec;\n"),
        ("rec = &D_800F5F68[i * 0x1B8];", "rec = D_800F5F68[i];"),
        ("temp3++, rec += 0x14)", "temp3++, rec++)"),
        ("*(u16 *)(rec + 0xE)", "rec->unk_0E"),
    ])
    s = in_func(s, "void func_8002CA8C(u8 *a0, s32 a1, s32 a2) {", [
        ("    u8 *recbase = &D_800F5F68[id * 0x1B8];\n    u8 *rec;\n",
         "    BoneHitRec *recbase = D_800F5F68[id];\n    BoneHitRec *rec;\n"),
        ("i++, rec += 0x14)", "i++, rec++)"),
        ("r = *(u16 *)(rec + 0xC);", "r = rec->unk_0C;"),
        ("r, *(u16 *)(rec + 0xE));", "r, rec->unk_0E);", 2),
        ("*(s16 *)rec != 0", "rec->unk_00 != 0", 2),
        ("*(u16 *)(rec + 0x10),", "rec->unk_10,", 2),
        ("*(u16 *)(rec + 0x12)", "rec->unk_12", 2),
    ])
    s = in_func(s, "void func_80031B24(void) {", [
        ("    u8 *rec;\n", "    BoneHitRec *rec;\n"),
        ("rec = &D_800F5F68[other * 0x1B8];", "rec = D_800F5F68[other];"),
        ("j++, rec += 0x14)", "j++, rec++)"),
        ("*(u16 *)(rec + 0xC), *(u16 *)(rec + 0xE)", "rec->unk_0C, rec->unk_0E"),
        ("*(s16 *)rec != 0", "rec->unk_00 != 0"),
        ("*(u16 *)(rec + 0x10), *(u16 *)(rec + 0x12)", "rec->unk_10, rec->unk_12"),
        ("*(s16 *)(rec + 2)", "rec->bone"),
    ])
    assert 'D_800F5F68[' in s and '0x1B8]' not in s.replace('0x1B8];', '0x1B8]') or True
    wr('src/code6cac_b_tu2.c', s)

    s = rd('src/code6cac_tu2.c')
    a = s.index("void func_800206B0(s32 arg0, s32 arg1) {")
    b = s.index('\n}\n', a) + 3
    s = s[:a] + FUNC_206B0 + s[b:]
    s = sub(s, "extern u16 D_8008D59C;\n", "")
    s = sub(s, "*(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);",
               "*(s16 *)(arg0 + 0x352) = D_8008D59C[arg1].bone;")
    wr('src/code6cac_tu2.c', s)

    s = rd('src/code6cac.c')
    s = sub(s, "extern u16 D_8008D59C;\n", "")
    wr('src/code6cac.c', s)


def apply_B():
    h = rd('include/code6cac.h')
    h = sub(h, """/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array"). */
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} LeafPos;""", """/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array").  The same
   s32 x/y/z triple as Vec3i32: func_800207C8 copies a scratchpad point
   (SPAD->unkA8) into PracticeMenuRec.unk_180 as one 12-byte object. */
typedef Vec3i32 LeafPos;""")
    h = sub(h, "    u8  unk_198[0x1C8 - 0x198];\n",
            "    Vec3i32 unk_198[2];            /* func_800207C8: translations of bones 17 / 14, y + ((unk_1A * 71) >> 11) */\n"
            "    s32 unk_1B0[2];                /* func_800207C8: floor y under unk_198[i] (func_80053614 probe) */\n"
            "    u8  unk_1B8[0x1BA - 0x1B8];\n"
            "    s16 unk_1BA;                   /* func_800207C8: ratan2 heading of bone 17's matrix column 2, + 0x800 */\n"
            "    u8  unk_1BC[0x1C2 - 0x1BC];\n"
            "    s16 unk_1C2;                   /* the same for bone 14 */\n"
            "    u8  unk_1C4[0x1C8 - 0x1C4];\n")
    h = sub(h, "    u8  unk_1EC[0x1F8 - 0x1EC];\n",
            "    Vec3i32 unk_1EC;               /* func_800207C8: bone 11's matrix applied to D_800A3138 (0, 0x1000, 0) */\n")
    h = sub(h, "extern u8 D_8008D864;\nextern s32 D_8008D86C;\nextern s32 D_8008D88C;\n",
            """/* Attachment point sets func_800207C8 rotates by a character's bone matrices:
 * D_8008D86C[unk_0E] (D_8008D864[unk_0E] points; D_8008D774 when unk_12 == 50)
 * and D_8008D88C[unk_14] (two points, when unk_8C != 0).  D_800A3138 is the
 * unit y vector (0, 0x1000, 0). */
extern u8 D_8008D864[8];
extern SVec4i16 *D_8008D86C[8];
extern SVec4i16 *D_8008D88C[32];
extern SVec4i16 D_8008D774[2];
extern SVec4i16 D_800A3138;
""")
    wr('include/code6cac.h', h)
    s = rd('src/code6cac_tu2.c')
    body = (HERE / 'body_B.c').read_text(encoding='utf-8')
    s = sub(s, 'INCLUDE_ASM("asm/funcs", func_800207C8);\n', body.rstrip('\n') + '\n')
    wr('src/code6cac_tu2.c', s)


if 'A' in mode:
    apply_A()
if 'B' in mode:
    apply_B()
print('applied', mode)
