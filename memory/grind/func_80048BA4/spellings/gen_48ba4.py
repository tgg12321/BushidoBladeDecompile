"""Generate full-file spellings of func_80048BA4 (src/text1b.c) for score_full.py."""
import re, sys
src = open("src/text1b.c", encoding="utf-8").read()
out = "tmp/ffc/48ba4"
import os
os.makedirs(out, exist_ok=True)

SCALAR_DECLS = "".join(f"extern s16 D_800FF5{x};\n" for x in
                       ("58", "5A", "5C", "5E", "60", "62", "64", "66", "68")) + \
    "extern s32 D_800FF56C;\nextern s32 D_800FF570;\nextern s32 D_800FF574;\n"
assert SCALAR_DECLS in src

OLD_TOP = """    prim = (u8 *)D_800A33E4;
    rotp = &rot.vx;
    vec = &D_800FF56C;
    ApplyMatrix(*(s32 *)player, rotp, vec);
    vec[0] += *(s32 *)(*(u8 **)player + 0x14);
    D_800FF570 += *(s32 *)(*(u8 **)(player + 4) + 0x18);
    D_800FF574 += *(s32 *)(*(u8 **)(player + 8) + 0x1C);
"""
OLD_MID = """    math_RotMatrixZYX(rotp, mtx.m[0]);
    gte_MulMatrix0ClearTrans(*(s32 *)player, mtx.m[0], mtx.m[0]);
    D_800FF558 = mtx.m[0][0];
    D_800FF55A = mtx.m[1][0];
    D_800FF55C = mtx.m[2][0];
    D_800FF55E = mtx.m[0][1];
    D_800FF560 = mtx.m[1][1];
    D_800FF562 = mtx.m[2][1];
    D_800FF564 = mtx.m[0][2];
    D_800FF566 = mtx.m[1][2];
    D_800FF568 = mtx.m[2][2];
"""
OLD_LOCALS = "    s16 *rotp;\n    s32 *vec;\n"
OLD_PROTOS = """extern void ApplyMatrix(s32, s16 *, s32 *);
extern void math_RotMatrixZYX(s16 *, s16 *);
extern void gte_MulMatrix0ClearTrans(s32, s16 *, s16 *);
"""
OLD_957 = "extern void ApplyMatrix(s32, s16 *, s32 *);\n\nvoid func_80049718"
OLD_1012 = "ApplyMatrix((*((s32 *) (part + 0xC))) + 0x18, sp10, (s32 *) (obj + 0x2C));"
for s in (OLD_TOP, OLD_MID, OLD_LOCALS, OLD_PROTOS, OLD_957, OLD_1012):
    assert src.count(s) == 1, s

NEW_PROTOS = """extern void ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);
"""
MTX_DECL = "extern MATRIX D_800FF558;\n"
MID_MATRIX = """    math_RotMatrixZYX(&rot, &mtx);
    gte_MulMatrix0ClearTrans(*(MATRIX **)player, &mtx, &mtx);
    D_800FF558.m[0][0] = mtx.m[0][0];
    D_800FF558.m[0][1] = mtx.m[1][0];
    D_800FF558.m[0][2] = mtx.m[2][0];
    D_800FF558.m[1][0] = mtx.m[0][1];
    D_800FF558.m[1][1] = mtx.m[1][1];
    D_800FF558.m[1][2] = mtx.m[2][1];
    D_800FF558.m[2][0] = mtx.m[0][2];
    D_800FF558.m[2][1] = mtx.m[1][2];
    D_800FF558.m[2][2] = mtx.m[2][2];
"""


def emit(name, top, mid=None, locals_="", protos=None, decls=None, fix957=False):
    s = src.replace(OLD_TOP, top).replace(OLD_LOCALS, locals_)
    if mid is not None:
        s = s.replace(OLD_MID, mid)
    if protos is not None:
        s = s.replace(OLD_PROTOS, protos)
    if decls is not None:
        s = s.replace(SCALAR_DECLS, decls)
    if fix957:
        s = s.replace(OLD_957, "\nvoid func_80049718")
        s = s.replace(OLD_1012,
                      "ApplyMatrix((MATRIX *)(*((s32 *) (part + 0xC)) + 0x18), (SVECTOR *)sp10, (VECTOR *) (obj + 0x2C));")
    open(f"{out}/{name}.c", "w", encoding="utf-8", newline="\n").write(s)


# V0 control: unchanged file
emit("v0_control", OLD_TOP, locals_=OLD_LOCALS)

# V1: scalars, no alias, no rotp (audit's 70 form)
emit("v1_scalar_direct", """    prim = (u8 *)D_800A33E4;
    ApplyMatrix(*(s32 *)player, &rot.vx, &D_800FF56C);
    D_800FF56C += *(s32 *)(*(u8 **)player + 0x14);
    D_800FF570 += *(s32 *)(*(u8 **)(player + 4) + 0x18);
    D_800FF574 += *(s32 *)(*(u8 **)(player + 8) + 0x1C);
""", mid=OLD_MID.replace("rotp", "&rot.vx"))

TOP_M = """    prim = (u8 *)D_800A33E4;
    ApplyMatrix(*(MATRIX **)player, &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += (*(MATRIX **)player)->t[0];
    D_800FF558.t[1] += ((MATRIX **)player)[1]->t[1];
    D_800FF558.t[2] += ((MATRIX **)player)[2]->t[2];
"""
# V2: MATRIX object + honest prototypes, no alias (func_80049718 call cast)
emit("v2_matrix_direct", TOP_M, mid=MID_MATRIX, protos=NEW_PROTOS,
     decls=MTX_DECL, fix957=True)

# V2b: &D.t spelling
emit("v2b_matrix_addr_t", TOP_M.replace("(VECTOR *)D_800FF558.t", "(VECTOR *)&D_800FF558.t"),
     mid=MID_MATRIX, protos=NEW_PROTOS, decls=MTX_DECL, fix957=True)

# V2c: MATRIX + offsets into player unchanged (raw) to isolate
emit("v2c_matrix_rawplayer", """    prim = (u8 *)D_800A33E4;
    ApplyMatrix(*(MATRIX **)player, &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += *(s32 *)(*(u8 **)player + 0x14);
    D_800FF558.t[1] += *(s32 *)(*(u8 **)(player + 4) + 0x18);
    D_800FF558.t[2] += *(s32 *)(*(u8 **)(player + 8) + 0x1C);
""", mid=MID_MATRIX, protos=NEW_PROTOS, decls=MTX_DECL, fix957=True)

# V3: MATRIX + alias (FAKE fallback shape)
emit("v3_matrix_alias", """    prim = (u8 *)D_800A33E4;
    t = D_800FF558.t;
    ApplyMatrix(*(MATRIX **)player, &rot, (VECTOR *)t);
    t[0] += (*(MATRIX **)player)->t[0];
    D_800FF558.t[1] += ((MATRIX **)player)[1]->t[1];
    D_800FF558.t[2] += ((MATRIX **)player)[2]->t[2];
""", mid=MID_MATRIX, locals_="    s32 *t;\n", protos=NEW_PROTOS, decls=MTX_DECL, fix957=True)
print("ok")

# V4: player typed as the array of MATRIX pointers it is (ApplyMatrix arg 1,
# t[] at +0x14/+0x18/+0x1C, 0x20-byte copies == sizeof(MATRIX))
s = open(f"{out}/v2_matrix_direct.c", encoding="utf-8").read()
reps = [
    ("typedef struct { s32 f0, f1, f2, f3, f4, f5, f6, f7; } _struct_copy_func48BA4;\n", ""),
    ("    u8 *player;\n", "    MATRIX **player;\n"),
    ("""    ApplyMatrix(*(MATRIX **)player, &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += (*(MATRIX **)player)->t[0];
    D_800FF558.t[1] += ((MATRIX **)player)[1]->t[1];
    D_800FF558.t[2] += ((MATRIX **)player)[2]->t[2];
""", """    ApplyMatrix(player[0], &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += player[0]->t[0];
    D_800FF558.t[1] += player[1]->t[1];
    D_800FF558.t[2] += player[2]->t[2];
"""),
    ("gte_MulMatrix0ClearTrans(*(MATRIX **)player, &mtx, &mtx);", "gte_MulMatrix0ClearTrans(player[0], &mtx, &mtx);"),
    ("""        *((_struct_copy_func48BA4 *)(prim + 0x18)) =
            *((_struct_copy_func48BA4 *)((u8 **)player)[index]);""",
     """        *(MATRIX *)(prim + 0x18) = *player[index];"""),
    ("""        *((_struct_copy_func48BA4 *)(prim + 0x18)) =
            *((_struct_copy_func48BA4 *)*(u8 **)(player + 0x48));""",
     """        *(MATRIX *)(prim + 0x18) = *player[18];"""),
    ("""        *((_struct_copy_func48BA4 *)(prim + 0x18)) =
            *((_struct_copy_func48BA4 *)*(u8 **)(player + 0x4C));""",
     """        *(MATRIX *)(prim + 0x18) = *player[19];"""),
]
for a, b in reps:
    assert s.count(a) == 1, a
    s = s.replace(a, b)
assert "_struct_copy_func48BA4" not in s, "other users"
open(f"{out}/v4_player_matrix_ptrs.c", "w", encoding="utf-8", newline="\n").write(s)
print("v4 ok")

# V5 (final): V4 + PsyQ libgte.h return type; the func_80049718 redeclaration
# is kept (updated) rather than deleted.
s = open(f"{out}/v4_player_matrix_ptrs.c", encoding="utf-8").read()
a = "extern void ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);\n"
assert s.count(a) == 1
s = s.replace(a, "extern VECTOR *ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);\n")
a = "extern void MulMatrix0(s16 *, s16 *, s16 *);\n\nvoid func_80049718"
assert s.count(a) == 1
s = s.replace(a, "extern void MulMatrix0(s16 *, s16 *, s16 *);\n"
              "extern VECTOR *ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);\n\nvoid func_80049718")
open(f"{out}/v5_final.c", "w", encoding="utf-8", newline="\n").write(s)
print("v5 ok")
