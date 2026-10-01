"""Apply func_80023F08's data-model edits to a copy of include/code6cac.h.
usage: python hdr_edit.py <in.h> <out.h>"""
import sys

src, dst = sys.argv[1:3]
s = open(src, encoding="utf-8").read()


def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (n, old[:80])
    s = s.replace(old, new)


# --- new record types ahead of PracticeMenuRec -------------------------------
rep("""typedef struct PracticeMenuRec {
""", """/* One decoded motion frame (0x84 bytes, 33 words): func_800198D0 decodes
 * frame `frame` of a character's motion into one, and func_80023F08 keeps the
 * current and next frames of its record (PracticeMenuRec.unk_290 holds the
 * frame it last used).  Only 16-bit channels, so a whole-frame assignment is
 * the halfword-aligned block copy func_80023F08 shows.  unk_00 is the root
 * offset it negates, unk_02 / unk_04 the root heading and distance, unk_06..
 * unk_0A the three rotation angles it hands math_RotMatrixZYXAngles. */
typedef struct MotionFrame {
    s16 unk_00;
    u16 unk_02;
    u16 unk_04;
    u16 unk_06;
    u16 unk_08;
    u16 unk_0A;
    u16 unk_0C[0x3C];
} MotionFrame;                     /* sizeof == 0x84 */

/* Header of one move record of a character's move script (the u16 stream
 * func_80021424 returns pointers into; PracticeMenuRec.unk_50 is the current
 * move, unk_7C a buffered one).  unk_00 / unk_02 are follow-up move ids,
 * unk_04 the motion id func_80021A98 reads, unk_07 / unk_08 frame bounds
 * against PracticeMenuRec.unk_40, unk_09 flag bits; unk_0A starts the
 * command list func_80023F08 walks: (flags, move id[, mask lo, mask hi])
 * entries of 2 or 4 halfwords, ended by a zero flags word. */
typedef struct MoveScript {
    u16 unk_00;
    u16 unk_02;
    u16 unk_04;
    u8  unk_06;
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;
    u16 unk_0A[1];
} MoveScript;

typedef struct PracticeMenuRec {
""")

rep("""    u8  unk_22[0x2C - 0x22];
    s32 unk_2C;
    s32 unk_30;
    u8  unk_34[0x3C - 0x34];
    s32 unk_3C;
    s16 unk_40;
    u8  unk_42[0x50 - 0x42];
    u8 *unk_50;                    /* entry whose byte 8 bounds unk_40 (func_80058580) */
    u8  unk_54[0x58 - 0x54];
""", """    u8  unk_22[0x24 - 0x22];
    PadState unk_24;               /* this frame's pad input (func_80023F08 copies it whole) */
    s32 unk_3C;                    /* frames since the record started (func_80023F08) */
    s16 unk_40;                    /* current frame of the move */
    s16 unk_42;                    /* frame fraction, 0x1000 = one frame */
    s16 unk_44;                    /* frame-fraction step */
    s16 unk_46;
    u8  unk_48[0x4A - 0x48];
    s16 unk_4A;
    s16 unk_4C;
    u8  unk_4E[0x50 - 0x4E];
    MoveScript *unk_50;            /* current move; unk_08 bounds unk_40 (func_80058580) */
    u16 *unk_54;
""")

rep("""    u8  unk_60[0x6A - 0x60];
    u16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */
    s16 unk_6C;
    u8  unk_6E[0x72 - 0x6E];
    s16 unk_72;
    u8  unk_74[0x7C - 0x74];
    s32 unk_7C;
    u8  unk_80[0x84 - 0x80];
""", """    u8  unk_60[0x62 - 0x60];
    u8  unk_62;
    u8  unk_63;
    u16 unk_64;
    u16 unk_66;
    s16 unk_68;
    u16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */
    s16 unk_6C;
    u8  unk_6E[0x70 - 0x6E];
    s16 unk_70;
    s16 unk_72;
    s32 unk_74;
    s16 unk_78;
    s16 unk_7A;
    MoveScript *unk_7C;            /* buffered next move (func_80023F08) */
    s16 unk_80;
    u16 unk_82;
""")

rep("""    s16 unk_92;
    u8  unk_94[0x96 - 0x94];
    s16 unk_96;
    u8  unk_98[0xA0 - 0x98];
""", """    s16 unk_92;
    s16 unk_94;
    s16 unk_96;
    SVec4i16 unk_98;
""")

rep("""    u8  unk_A3[2];
    u8  unk_A5[0xAD - 0xA5];
""", """    u8  unk_A3[2];
    u8  unk_A5;
    u8  unk_A6;
    u8  unk_A7;
    u8  unk_A8;
    u8  unk_A9;
    u8  unk_AA;
    u8  unk_AB;
    u8  unk_AC;
""")

rep("""    u8  unk_B2;
    u8  unk_B3[0xB8 - 0xB3];
""", """    u8  unk_B2;
    u8  unk_B3;
    u8  unk_B4;
    u8  unk_B5[0xB8 - 0xB5];
""")

rep("""    s16 unk_152;
    u8  unk_154[0x156 - 0x154];
""", """    s16 unk_152;
    s16 unk_154;
""")

rep("""    s16 unk_1D8;
    u8  unk_1DA[0x1DC - 0x1DA];
""", """    s16 unk_1D8;
    s16 unk_1DA;
""")

rep("""    Vec4i32 unk_24C;
    u8  unk_25C[0x268 - 0x25C];
""", """    Vec4i32 unk_24C;
    LeafPos unk_25C;               /* func_80023F08: copy of scratchpad point unk00[idx][0] */
""")

rep("""    s16 unk_286;
    u8  unk_288[0x28C - 0x288];
    s32 unk_28C;
    u8  unk_290[0x31A - 0x290];
    s16 unk_31A;
    u8  unk_31C[0x330 - 0x31C];
""", """    s16 unk_286;
    u16 unk_288[2];                /* per blade (unk_A1 / unk_A3 bounds), func_80023F08 */
    s32 unk_28C;
    MotionFrame unk_290;           /* the motion frame func_80023F08 last used */
    u16 unk_314;
    u16 unk_316;
    s16 unk_318;
    s16 unk_31A;
    s16 unk_31C;
    u8  unk_31E[0x320 - 0x31E];
    Vec3i32 unk_320;
    u8  unk_32C[0x330 - 0x32C];
""")

# --- data declarations --------------------------------------------------------
rep("""extern s16 D_8008DA50;
extern s16 D_8008DA94;
extern s16 D_8008DAD8;
""", """extern s16 D_8008DA50[];          /* [unk_0A] (func_80023F08) */
extern s16 D_8008DA94[];          /* [unk_0A] (func_80023F08) */
extern s16 D_8008DAD8[];          /* [unk_0A] (func_80023F08) */
""")
rep("extern s32 D_800A36D8;\n", "extern MoveScript *D_800A36D8;\n")
rep("""extern s32 D_800A3888;
extern s32 D_800A388C;
""", """extern MotionFrame *D_800A3888[2];  /* per player: preset motion frames (func_80020D70, func_80023F08) */
""")

# --- scratchpad point tables (moved from src/code6cac_b_tu2.c) ----------------
rep("""/* Per-character / practice-menu record table (base 0x80101EC8, stride 0x44C,""",
    """/* Scratchpad point tables at 0x1F800000.  unk00: three points per character
 * (func_8002C61C copies [0][0..2] and [1][0..2] to the two records' +0x210);
 * unk48: two more per character (copied to +0x234); unkA8: 22 points per
 * character (func_8002A458 reads 0x1F8000A8 + id * 0x108 + i * 0xC, i < 22).
 * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48. */
typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    u8 unk78[0xA8 - 0x78];
    LeafPos unkA8[2][22];
} ScrPad;
#define SPAD ((ScrPad *)0x1F800000)

/* Per-character / practice-menu record table (base 0x80101EC8, stride 0x44C,""")

open(dst, "w", encoding="utf-8", newline="\n").write(s)
