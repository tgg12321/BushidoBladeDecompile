#!/usr/bin/env python3
"""Step 8: M3 reconciliation (Q65/Q67, per-file-gp-model.md "Merge": declarations reconciled FIRST, own byte-identical
commit). text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b are one original file (owner ruling
Q67); every symbol they declare differently gets its one truthful type, chosen from the target bytes (the
width / signedness / argument registers of the accesses and calls) and from each consumer's use. Every
part stays a separate file here; each object is byte-identical (the step's oracle build + objcompare).

Functions (the definition's own prototype wins unless the call bytes contradict it):
  func_800466C0   defined (text1a_b) `void (s32, s32)`; text1a_c2 only takes its address.
  func_80044100   defined (text1a_c) `void (s32 a0, s32 a1)`; text1a_b's call `func_80044100(8)` leaves $a1
                  unset in the target, so no prototype was in scope there: unprototyped in both parts (owner ruling Q88,
                  rules: a17a2c144, per-file-gp-model.md A5).
  func_80045694   defined (text1a_c_tu2) `void (s32, s32)`: the word is stored in a callback table
                  (sw to D_800EED1C + i); sound passes its three callbacks as that word.
  func_800460E4   defined (text1a_c2) `void (s32 stage_id, s32 arg1)`, arg1 used throughout. Its caller
                  game_StageCleanup (sound) forwards its own $a1 untouched (text1b calls game_StageCleanup
                  with two arguments, $a1 set): game_StageCleanup is `(s32 a0, s32 a1)` and passes both.
  func_80044010   callee bytes: $a0 is dereferenced (lw 0($a0), the table stores $a0 + 4), $a1 is sign-
                  truncated to 16 bits (sll/sra 16) -> `void (s32 *, s16)` (as text1a_pre already declares).
  GetClut         a full word: sound's caller (func_800477E8) keeps the result unmasked (move s1,v0 - a
                  u16 declaration makes the caller andi 0xFFFF), as gpu.c's definition `u32 GetClut(s32,
                  s32)` (the callee does the masking). Not the PsyQ header's u_short: that header was not
                  in scope in this file. text1b's sh of it is the same either way.
  func_800486FC   returns a full word: sound's caller tests all 32 bits (an s16 declaration adds sll 16),
                  and text1b's two callers (no declaration before them, implicit int) do the same; the body
                  returns the lh of the s16 global, already sign-extended -> `s32 (void)`.
  func_800468B0   defined (sound) `void (s32)`.
  func_8004153C   the step-6 declaration `extern s32 *func_8004153C();` (config's zero-argument calls).
  game_GetPlayerData  every caller passes the player index (text1b, code6cac, code6cac_b_tu2, code6cac_tu2);
                  the body forwards $a0 untouched to func_8004153C (defined `(s32 a0)`): `(s32 a0)`.
  ApplyMatrix / gte_MulMatrix0ClearTrans / MulMatrix0  the PsyQ libgte prototypes (MATRIX / SVECTOR / VECTOR),
                  as text1b. MulMatrix0: sound's func_800475A4 multiplies two 0x20-byte matrices the two
                  function-pointer calls fill (s32[8] buffers with no s32 use): they become `MATRIX buf1, buf2`
                  (the callees' parameter is `MATRIX *`), MulMatrix0(&buf2, &buf1, base + 0x18).
  func_80046020   defined (text1a_c_tu2) `void (void)`.
  func_800418D0   defined (text1a_post) `void (s32 *)`.
  func_80044FA0   defined (text1a_c_tu2) `s32 (s32, s32)`: $a1 is subtracted from (an integer).
  stage_GetDataPtr  defined (sound) `void *(void)`.
  ratan2          defined (display) `s32 (s32, s32)`, the PsyQ `long ratan2(long, long)`.
  func_8004A1FC   text1b's call `func_8004A1FC()` (no argument set) and its K&R definition need no
                  prototype in scope: unprototyped in sound.
Objects:
  D_800A3708      a pointer to the Unk80101DF0Record (step 6's type); sound reads xf.rot (+0x10) through it.
  D_800F62E0      eight 0x60-byte records (text1b's loop: 8 x 0x60; sound's +0x60 / +0x180; text1a_c's
                  `u8 [8][0x60]`).
  D_80102C00      only its address is ever taken (lui/addiu in all four users): the width is undecided by
                  the bytes; s32 (sound's and text1a_c's declaration). [flagged: evidence does not decide]
  D_800153F0      22 halfwords (func_8004A09C walks it as u16), copied whole by func_80049F4C. The copy's
                  bytes carry a RUN-TIME alignment test (or/andi 3/beqz, lwl/lwr + swl/swr loop), i.e. the
                  copy's compile-time alignment is < 4. In one file whose definition precedes the copy,
                  varasm's DATA_ALIGNMENT raises DECL_ALIGN of any aggregate definition to 32, so a copy
                  through the object's address would be aligned; a struct ASSIGNMENT uses the TYPE's
                  alignment instead. So the object is a halfword record and func_80049F4C assigns it.
                  [flagged: alternative reading - a local initializer of func_80049F4C - needs the
                  pre_rodata items placed at their functions; not a verbatim move]
usage: s08_apply.py <tree>"""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from adoptlib import *
os.chdir(sys.argv[1])


def subn(p, a, b, n):
    t = rd(p)
    assert t.count(a) == n, (p, a[:120], t.count(a), n)
    wr(p, t.replace(a, b))


C2, TB, PR, SD, T1 = "src/text1a_c2.c", "src/text1a_b.c", "src/text1a_b_pre_rodata.c", "src/sound.c", "src/text1b.c"
# text1a_c2
sub1(C2, "extern void func_80044010(s32, s32);", "extern void func_80044010(s32 *, s16);")
subn(C2, "func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);", "func_80044010((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);", 2)
sub1(C2, "func_80044010((s32)s6, 7);", "func_80044010(s6, 7);")
sub1(C2, "extern void func_800466C0(void);", "extern void func_800466C0(s32, s32);")
# text1a_b
sub1(TB, "extern void func_80044100(s32, ...);", "extern void func_80044100();")
# sound
sub1(SD, "extern void *func_8004153C(void);", "extern s32 *func_8004153C();")
sub1(SD, "extern void func_80044100(s32, s32);", "extern void func_80044100();")
sub1(SD, "extern void func_80045694(s32, void (*)(void));", "extern void func_80045694(s32, s32);")
sub1(SD, "func_80045694(chan, func_800468DC);", "func_80045694(chan, (s32)func_800468DC);")
sub1(SD, "func_80045694(9, snd_SeNullCallback);", "func_80045694(9, (s32)snd_SeNullCallback);")
sub1(SD, "func_80045694(0xA, func_80046A80);", "func_80045694(0xA, (s32)func_80046A80);")
sub1(SD, "extern void func_800460E4(s32);", "extern void func_800460E4(s32, s32);")
sub1(SD, "void game_StageCleanup(s32 a0) {\n    func_800460E4(a0);",
     "void game_StageCleanup(s32 a0, s32 a1) {\n    func_800460E4(a0, a1);")
sub1(SD, "extern void func_80044010(s32 *, s32);", "extern void func_80044010(s32 *, s16);")
sub1(SD, "extern void ApplyMatrix(s32 *, s16 *, s32 *);", "extern VECTOR *ApplyMatrix(MATRIX *, SVECTOR *, VECTOR *);")
sub1(SD, "ApplyMatrix(pos, rot, sp18);", "ApplyMatrix((MATRIX *)pos, (SVECTOR *)rot, (VECTOR *)sp18);")
sub1(SD, "ApplyMatrix((s32 *)&D_80101DF0.xf.mat, rot, result);",
     "ApplyMatrix((MATRIX *)&D_80101DF0.xf.mat, (SVECTOR *)rot, (VECTOR *)result);")
sub1(SD, "extern s16 ratan2(s32, s32);", "extern s32 ratan2(s32, s32);")
sub1(SD, "extern s32 D_800A3708;", "extern Unk80101DF0Record *D_800A3708;")
sub1(SD, "math_RotMatrixYXZ((s32 *)((u8 *)*(s32 **)&D_800A3708 + 0x10), pos);",
     "math_RotMatrixYXZ((s32 *)&D_800A3708->xf.rot, pos);")
sub1(SD, "extern void gte_MulMatrix0ClearTrans(void *, void *, void *);",
     "extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);")
sub1(SD, "gte_MulMatrix0ClearTrans(&D_800EEDB0, a0, a1);",
     "gte_MulMatrix0ClearTrans((MATRIX *)&D_800EEDB0, (MATRIX *)a0, (MATRIX *)a1);")
sub1(SD, "gte_MulMatrix0ClearTrans(&sp10, var_s0, arg2);",
     "gte_MulMatrix0ClearTrans(&sp10, (MATRIX *)var_s0, (MATRIX *)arg2);")
sub1(SD, "extern void func_80044FA0(s32, s32 *);", "extern s32 func_80044FA0(s32, s32);")
sub1(SD, "func_80044FA0(a0, v0);", "func_80044FA0(a0, (s32)v0);")
sub1(SD, "extern void func_8004A1FC(void *);", "extern void func_8004A1FC();")
sub1(SD, "extern void MulMatrix0(s32 *, s32 *, s32 *);", "extern MATRIX *MulMatrix0(MATRIX *, MATRIX *, MATRIX *);")
sub1(SD, "    s32 buf1[8];\n    s32 buf2[8];\n", "    MATRIX buf1;\n    MATRIX buf2;\n")
sub1(SD, "((void (*)(u8 *, s32 *))D_800F66B0)(base + 0x10, buf1);",
     "((void (*)(u8 *, MATRIX *))D_800F66B0)(base + 0x10, &buf1);")
sub1(SD, "((void (*)(u8 *, s32 *))g_anim_func_table[0])((u8 *)&D_80101DF0.xf.rot, buf2);",
     "((void (*)(u8 *, MATRIX *))g_anim_func_table[0])((u8 *)&D_80101DF0.xf.rot, &buf2);")
sub1(SD, "MulMatrix0(buf2, buf1, (s32 *)(base + 0x18));", "MulMatrix0(&buf2, &buf1, (MATRIX *)(base + 0x18));")
sub1(SD, "extern s32 D_800F62E0;", "extern u8 D_800F62E0[8][0x60];")
sub1(SD, "base = (u8 *)&D_800F62E0;", "base = D_800F62E0[0];")
sub1(SD, "void *game_GetPlayerData(void) {\n    void *v0 = func_8004153C();",
     "void *game_GetPlayerData(s32 a0) {\n    void *v0 = func_8004153C(a0);")
# text1b
sub1(T1, "extern u16 GetClut(s32, s32);", "extern u32 GetClut(s32, s32);")
sub1(T1, "s16 func_800486FC(void) {", "s32 func_800486FC(void) {")
sub1(T1, "extern s32 func_800468B0(s32);", "extern void func_800468B0(s32);")
sub1(T1, "extern s32 func_8004153C(s32);", "extern s32 *func_8004153C();")
sub1(T1, "temp_v0 = func_8004153C(arg0);", "temp_v0 = (s32)func_8004153C(arg0);")
sub1(T1, "v = func_8004153C(0);", "v = (s32)func_8004153C(0);")
sub1(T1, "v = func_8004153C(1);", "v = (s32)func_8004153C(1);")
sub1(T1, "extern s32 func_80046020();", "extern void func_80046020(void);")
sub1(T1, "extern s32 func_800418D0();", "extern void func_800418D0(s32 *);")
sub1(T1, "func_800418D0(p1);", "func_800418D0((s32 *)p1);")
sub1(T1, "func_800418D0(p2);", "func_800418D0((s32 *)p2);")
sub1(T1, "extern void *D_800A3708;", "extern Unk80101DF0Record *D_800A3708;")
sub1(T1, "extern s16 *stage_GetDataPtr(void);", "extern void *stage_GetDataPtr(void);")
sub1(T1, "extern u32 D_80102C00;", "extern s32 D_80102C00;")
sub1(T1, "D_800A3820 = &D_80102C00;", "D_800A3820 = (s32)&D_80102C00;")
sub1(T1, "extern u8 D_800F62E0;", "extern u8 D_800F62E0[8][0x60];")
sub1(T1, "base = &D_800F62E0;", "base = D_800F62E0[0];")
REC = ("/* 0x800153F0: the 22-halfword record func_8004A09C unpacks (it walks it as u16). func_80049F4C copies\n"
       "   it whole by assignment: the copy's run-time alignment test in the target bytes is the halfword\n"
       "   type's alignment. */\n"
       "typedef struct {\n    u16 v[22];\n} Unk800153F0Record;\n")
sub1(T1, "extern u8 D_800153F0;\n", REC + "extern const Unk800153F0Record D_800153F0;\n")
sub1(T1, "    s32 sp10[11];\n    s32 i;\n    u8 *base;\n    __builtin_memcpy(sp10, &D_800153F0, 44);",
     "    Unk800153F0Record sp10;\n    s32 i;\n    u8 *base;\n    sp10 = D_800153F0;")
sub1(T1, "func_8004A09C((s32)base, (u16 *)sp10);", "func_8004A09C((s32)base, sp10.v);")
# text1a_b_pre_rodata: the same record type, same bytes (each little-endian word = two halfwords, low first)
words = [0x0E000E00, 0x00000E00, 0, 0, 0x0E000000, 0x00010A00, 0, 0, 0, 0x00300030, 0x10000030]
old = "/* D_800153F0: 11 words (44B) @ 0x800153F0 */\nconst u32 D_800153F0[11] = {\n" + \
      "".join(f"    0x{w:08X},\n" for w in words) + "};\n"
hw = [h for w in words for h in (w & 0xFFFF, w >> 16)]
new = "/* D_800153F0: 22 halfwords (44B) @ 0x800153F0 */\n" + REC + "const Unk800153F0Record D_800153F0 = {{\n" + \
      "".join("    " + ", ".join(f"0x{h:04X}" for h in hw[i:i + 8]) + ",\n" for i in range(0, 22, 8)) + "}};\n"
sub1(PR, old, new)
# the libgte prototypes take SVECTOR / VECTOR / MATRIX: the locals handed to them are those types (layer-2 round 2,
# step 08: no (SVECTOR *) / (VECTOR *) / (MATRIX *) puns on s16[3] / s32[3..8] arrays); byte-identical (sound.o)
sub1(SD, """void func_800475A4(void) {
    s16 rot[3];
    s32 result[4];""", """void func_800475A4(void) {
    SVECTOR rot;
    VECTOR result;""")
sub1(SD, """    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x6590;
    ApplyMatrix((MATRIX *)&D_80101DF0.xf.mat, (SVECTOR *)rot, (VECTOR *)result);

    angle = ratan2(result[0], result[2]);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result[2] + (s32)Judge[angle & 0xFFF] * result[0]) >> 12;
    result[2] = computed;

    {
        s16 neg = -ratan2(result[1], computed);""", """    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x6590;
    ApplyMatrix((MATRIX *)&D_80101DF0.xf.mat, &rot, &result);

    angle = ratan2(result.vx, result.vz);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result.vz + (s32)Judge[angle & 0xFFF] * result.vx) >> 12;
    result.vz = computed;

    {
        s16 neg = -ratan2(result.vy, computed);""")
sub1(SD, """    s16 rot[3];
    s32 sp18[3];
    s32 pos[8];
    s16 s0;

    math_RotMatrixYXZ((s32 *)&D_800A3708->xf.rot, pos);
    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x1000;
    ApplyMatrix((MATRIX *)pos, (SVECTOR *)rot, (VECTOR *)sp18);
    s0 = ratan2(sp18[0], sp18[2]);
    sp18[2] = ((s32)Judge[((s16)s0 + 0x400) & 0xFFF] * sp18[2]
              + (s32)Judge[s0 & 0xFFF] * sp18[0]) >> 12;
    D_800A33C8 = -ratan2(sp18[1], sp18[2]);""", """    SVECTOR rot;
    VECTOR sp18;
    MATRIX pos;
    s16 s0;

    math_RotMatrixYXZ((s32 *)&D_800A3708->xf.rot, (s32 *)&pos);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&pos, &rot, &sp18);
    s0 = ratan2(sp18.vx, sp18.vz);
    sp18.vz = ((s32)Judge[((s16)s0 + 0x400) & 0xFFF] * sp18.vz
              + (s32)Judge[s0 & 0xFFF] * sp18.vx) >> 12;
    D_800A33C8 = -ratan2(sp18.vy, sp18.vz);""")
print("M3 reconciliation applied")
