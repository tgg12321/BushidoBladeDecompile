#ifndef CODE6CAC_H
#define CODE6CAC_H

/* Shared declarations for the code6cac module family */

#include "common.h"
#include "libcd.h"

/* Named globals */
extern u8 cpu_practice_honmokuroku_data_tbl[][4];
/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF: 0x8008D118..0x8008D517
 * (0x400 bytes; the first 8 are the words after DelDrv in src/main_post.c,
 * the rest asm/data/7D920.data.s dlabel D_8008D120). */
extern u8 g_sqrt_table_u8[0x400];
/* menuDat: model id -> BBM file name, ended by a zero id (0x8008DCCC..0x8008DD5B,
 * asm/data/7D920.data.s dlabel menuDat). func_80020E74 loads the model of entry n
 * from CD file n + 2. */
typedef struct {
    s32 id;
    char *name;
} MenuDatEntry;
extern MenuDatEntry menuDat[18];
/* D_8008EB40: 3 rows x 3 s16 angle offsets, read as [row][col] with row, col
 * in 0..2 from the pad bits (func_800233AC, func_80023648); 0x8008EB40..0x8008EB51,
 * then 2 bytes of word-alignment padding before D_8008EB54 (dlabel 0x14 bytes,
 * asm/data/7D920.data.s). */
extern s16 D_8008EB40[3][3];
/* Judge: the sine table, one full turn in 0x1000 steps, 1.0 = 0x1000
 * (cos(a) = Judge[(a + 0x400) & 0xFFF]). 0x800973FC..0x800993FB = 0x1000 s16
 * (asm/data/7D920.data.s dlabel Judge, 0x2000 bytes; every reader indexes it
 * with a 12-bit angle). */
extern s16 Judge[0x1000];

/* Per-character record pointed to by D_800A3860[ch] (ch = rec+0x4A). f14 is
 * the modulus func_800213A0 / func_80021424 wrap rec+0x86 with. The u16
 * fields at +0x4E are indices into D_801027B0[ch][0], as func_80021424 reads
 * them: f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4),
 * f54[rec+0x86][t] (id 0x7FF3), and
 * f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). f16 (func_800219E4) and
 * f18[class] (func_80021A3C; class = Unk80101EC8Record.unk_0A, 0..26 as D_8008D538
 * holds and as the [27][6] class tables D_8008DE34 / D_8008DF78 are sized; 27
 * halfwords end exactly at f4E) are indices into D_80102760; both readers load
 * them with lhu. */
typedef struct {
    u8 pad00[3];
    u8 unk_03;                     /* D_801027B0[ch][0] = record + 0x6C + (unk_03 - 1) * 6 (func_80020E74) */
    s32 unk_04[4];                 /* D_801027B0[ch][1 + k] = record + unk_04[k] (func_80020E74) */
    s16 f14;
    u16 f16;
    u16 f18[27];
    u16 f4E[3];
    u16 f54[3][3];
    u16 f66[11][3];
} Tbl800A3860Entry;

/* s32 x/y/z triple.  Unk80101EC8Record's position-like triples are copied as
 * whole 12-byte objects (func_80022580), and func_80021DB0 writes one through
 * its out parameter. */
typedef struct { s32 x, y, z; } Vec3i32;
/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array").  The same
   s32 x/y/z triple as Vec3i32: func_800207C8 copies a scratchpad point
   (SPAD->unkA8) into Unk80101EC8Record.unk_180 as one 12-byte object. */
typedef Vec3i32 LeafPos;
/* The 16 slots func_800645B0 spawns and func_800646E8 draws (bit i of D_800A3444
 * live): slot i's position.  Both functions address it as base + i*12
 * (asm/funcs/func_800645B0.s:31-55, func_800646E8.s:93-109).  Replaces the splat
 * per-word scalars D_800F0D78 / D_800F0D7C / D_800F0D80 (the last was misnamed
 * "videoDec": it is slot 0's z). */
extern Vec3i32 D_800F0D78[16];
/* PsyQ VECTOR / SVECTOR layouts (include/gte.h), spelled with local tags for
 * the same reason as Unk80101DF0Rot below.  func_80022580 copies
 * Unk80101EC8Record's +0xB8 and +0x104 as whole 16-byte VECTORs (pad included)
 * and +0x1C8 as a whole 8-byte SVECTOR. */
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
typedef struct { s16 vx, vy, vz, pad; } SVec4i16;
/* The 10-entry block table over the 0x45000-byte buffer at D_800A9D10
 * (text1a_c_tu2.c func_800451D0 .. func_8004574C; D_800A33AC live entries).
 * func_800451D0 clears id in all 10 (offset 0x90 down to 0 in steps of 0x10,
 * asm/funcs/func_800451D0.s); the walkers index base + i*16.  func_80045294(a0,
 * a1) hands func_800520B8 entry a0's unk4, unk4 + a1 and the summed amt of
 * entries a0.., then adds a1 to their unk4 and calls each fn(id, a1).  Replaces the splat per-word scalars D_800EED14 /
 * D_800EED18 / D_800EED1C and D_800EED00 (entry -1, i.e. D_800EED10[j - 1]). */
typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} Unk800EED10Entry;
extern Unk800EED10Entry D_800EED10[10];

/* A hit record: one of the 22 test points of a character.  func_800207C8 places
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
/* The 6 slots func_8006288C spawns and func_8006295C draws (bit i of D_800A3460
 * live): slot i's position and its RotMatrixZYX angles.  Both functions
 * address them as base + i*12 / base + i*8 (asm/funcs/func_8006288C.s:20-44, the
 * 0xC / 0x8 induction steps at :49-53).  Replaces the splat per-word scalars
 * D_800F0FB8 / D_800F0FBC / D_800F0FC0 and D_800F10A0 / D_800F10A2 / D_800F10A4. */
extern Vec3i32 D_800F0FB8[6];
extern SVec4i16 D_800F10A0[6];
extern s16 D_800F0C04[6];

/* Pad input record (0x18 bytes) at 0x80102788.  func_80019568 fills it each
 * frame from the two pads (one u16 half per player in each word):
 * held = current bits, pressed = held & ~previous, released = ~held & previous,
 * unheld = ~held; pad_ResetState sets type[] and the four bit words to
 * (4, 4, 0, 0, 0, -1) and leaves valid[] alone.  func_8001BE20
 * copies the whole record to its caller's buffer, and pad_ClearStateBits clears the
 * four bit words of such a buffer. */
typedef struct PadState {
    s16 type[2];                   /* per pad: InitPAD buffer byte 1 >> 4, 5 and 7 folded
                                      to 4, 4 when the status byte != 0 (func_80019568,
                                      where func_8003A728 can first replace them with
                                      nibbles from the link-cable exchange words);
                                      4 from pad_ResetState (both) and func_80055B60
                                      (entry arg0 of its own record) */
    s16 valid[2];                  /* per pad: 1 iff the InitPAD buffer status byte == 0
                                      (func_80019568); 1, 1 from func_80019534 */
    u32 held;                      /* 0x08 */
    u32 pressed;                   /* 0x0C */
    u32 released;                  /* 0x10 */
    u32 unheld;                    /* 0x14 */
} PadState;                        /* sizeof == 0x18 */

/* The 24-entry pending-sound pool (text1b_tu1b.c): func_8005C650 queues a
 * request (an entry of D_8009AA70) with its volumes in the first free entry;
 * func_8005C6D0 keys each queued note on via SsUtKeyOnV(.., voll, volr) and
 * clears the entry; snd_Init / func_8005B5AC reset all 24.  Every user indexes
 * base + i*8 (asm/funcs/func_8005C650.s, snd_Init.s: 0xC0 / 8 entries).
 * Replaces the splat per-word scalars D_800EFB7C / D_800EFB7D. */
typedef struct {
    u16 *req;
    u8 volr;
    u8 voll;
} Unk800EFB78Entry;
extern Unk800EFB78Entry D_800EFB78[24];

/* Scratchpad point tables at 0x1F800000.  unk00: three points per character
 * (func_8002C61C copies [0][0..2] and [1][0..2] to the two records' +0x210);
 * unk48: two more per character (copied to +0x234); unk78: two body points
 * per character (func_800288C8 builds them from unkA8 joint 1 and the
 * midpoint of joints 15 and 19); unkA8: 22 points per
 * character (func_8002A458 reads 0x1F8000A8 + id * 0x108 + i * 0xC, i < 22).
 * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48. */
typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    LeafPos unk78[2][2];
    LeafPos unkA8[2][22];
} ScrPad;
#define SPAD ((ScrPad *)0x1F800000)

/* Per-character record table (base 0x80101EC8, stride 0x44C, 2 records); the
 * old "practice menu" name was RESET by owner ruling Q103.  Schema:
 * docs/naming/CHAR_STRUCT_SCHEMA.md; base symbol: named_syms.txt (D_80101EC8).  Only the fields reached by C so
 * far are named; the rest is reserved padding.  func_80022580 initializes
 * record [idx] (every field it writes is declared at its offset). */
/* One CPU path waypoint (CpuRoute.node[]): func_800571C0 writes x/z from the
 * opponent's position and kind 2; func_80058580 walks them back to front. */
typedef struct CpuWaypoint {
    s16 x;
    s16 z;
    u8 kind;
    u8 unk5;
} CpuWaypoint;

/* A CPU route: the polygon and vertex the walker stands at (func_80057ACC), the waypoint
 * count and the waypoints. Unk80101EC8Record carries one at +0x360; func_80057E84 builds two
 * candidates of the same layout on its stack and appends the cheaper one. */
typedef struct CpuRoute {
    u8 poly;                       /* index into the stage's NavPolySet.polys */
    u8 vtx;                        /* vertex index in that polygon */
    u8 count;                      /* number of waypoints in node[] */
    CpuWaypoint node[8];
} CpuRoute;                        /* sizeof == 0x34 */

/* One polygon of a stage's navigation set (8 bytes): flags (0x80 = open chain, the last vertex
 * does not close back to the first), a kind byte copied into the route waypoints built around
 * it, a corner margin (func_80057CC8 scales it by 40), the vertex count and the vertex table of
 * x/z pairs. */
typedef struct NavPoly {
    u8 flags;
    u8 kind;
    u8 margin;
    u8 nvtx;
    s16 (*vtx)[2];
} NavPoly;

/* A stage's navigation set: a D_8009A658 row (12 bytes: count word, polygon array, zero word). */
typedef struct NavPolySet {
    u8 npolys;
    u8 unk1[3];
    NavPoly *polys;
    u8 unk8[4];
} NavPolySet;

/* One decoded motion frame (0x84 bytes, 33 words): func_800198D0 decodes
 * frame `frame` of a character's motion into one, and func_80023F08 keeps the
 * current and next frames of its record (Unk80101EC8Record.unk_290 holds the
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
 * func_80021424 returns pointers into; Unk80101EC8Record.unk_50 is the current
 * move, unk_7C a buffered one).  unk_00 / unk_02 are follow-up move ids,
 * unk_07 / unk_08 frame bounds against Unk80101EC8Record.unk_40, unk_09 flag
 * bits (func_80023F08, func_80055B60, func_80058580); unk_0A starts the
 * command list func_80023F08 walks: (flags, move id[, mask lo, mask hi])
 * entries of 2 or 4 halfwords, ended by a zero flags word. */
typedef struct MoveScript {
    u16 unk_00;
    u16 unk_02;
    u16 unk_04;                    /* row of the bank's 4-byte entry table (func_80021A98) */
    u8  unk_06;                    /* first frame: Unk80101EC8Record.unk_40 starts here (func_80021A98) */
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;
    u16 unk_0A[1];
} MoveScript;

typedef struct Unk80101EC8Record {
    struct Unk80101EC8Record *other; /* the other record: [1] for record 0, else [0] (func_80022580) */
    s16 index;                     /* this record's own index (func_80022580) */
    s16 unk_06;                    /* != 0: func_8001BE20 hands pad input to func_80055B60 */
    s16 unk_08;
    s16 unk_0A;                    /* class idx: row of D_8008E5CC / D_8008E6A4, index of D_8008D9EC */
    s16 unk_0C;
    s16 unk_0E;                    /* column of D_8008E5CC / D_8008E6A4 */
    u8  unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14;                    /* -1 == none, else index of D_8008EB80 */
    u8  unk_16[0x1A - 0x16];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8  unk_22[0x24 - 0x22];
    PadState unk_24;               /* this frame's pad input (func_80023F08 copies it whole) */
    s32 unk_3C;                    /* frames since the record started (func_80023F08) */
    s16 unk_40;                    /* current frame of the move */
    s16 unk_42;                    /* frame fraction, 0x1000 = one frame */
    s16 unk_44;                    /* frame-fraction step */
    s16 unk_46;
    u16 unk_48;                    /* model id (func_80020E74); func_80021280 finds it in D_800A38C4 */
    s16 unk_4A;
    s16 unk_4C;
    u8  unk_4E[0x50 - 0x4E];
    MoveScript *unk_50;            /* current move; unk_08 bounds unk_40 (func_80058580) */
    u16 *unk_54;
    u8 *unk_58;                    /* byte 3 read by func_80056FE8 */
    u16 unk_5C;
    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */
    u8  unk_60;
    u8  unk_61;
    u8  unk_62;
    u8  unk_63;
    u16 unk_64;
    u16 unk_66;
    s16 unk_68;
    u16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */
    s16 unk_6C;
    s16 unk_6E;
    s16 unk_70;
    s16 unk_72;
    s32 unk_74;
    s16 unk_78;
    s16 unk_7A;
    MoveScript *unk_7C;            /* buffered next move (func_80023F08) */
    s16 unk_80;
    u16 unk_82;
    s16 unk_84;
    s16 unk_86;                    /* row of unk_3A8 / unk_3F8 / unk_3FE / unk_404 */
    s16 unk_88;
    s16 unk_8A;
    s16 unk_8C;                    /* != 0: the second blade (unk_234 / SPAD unk48 points) is live */
    s16 unk_8E;
    s16 unk_90;
    s16 unk_92;
    s16 unk_94;
    s16 unk_96;
    SVec4i16 unk_98;
    u8  unk_A0;
    u8  unk_A1[2];
    u8  unk_A3[2];
    u8  unk_A5;
    u8  unk_A6;
    u8  unk_A7;
    u8  unk_A8;
    u8  unk_A9;
    u8  unk_AA;
    u8  unk_AB;
    u8  unk_AC;
    u8  unk_AD;                    /* != 0: func_8002C61C re-runs func_800283D0 for both records */
    u8  unk_AE;
    u8  unk_AF;
    u8  unk_B0;
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3;
    u8  unk_B4;
    u8  unk_B5[0xB8 - 0xB5];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8  unk_E4[0xE8 - 0xE4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8  unk_100[0x104 - 0x100];
    Vec4i32 unk_104;
    Vec4i32 unk_114[2];            /* per blade; func_8002AB08 indexes it with its 0/1 blade flag */
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    s16 unk_154;
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8  unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8  unk_164[0x168 - 0x164];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    Vec3i32 unk_198[2];            /* func_800207C8: translations of bones 17 / 14, y + ((unk_1A * 71) >> 11) */
    s32 unk_1B0[2];                /* func_800207C8: floor y under unk_198[i] (func_80053614 probe) */
    u8  unk_1B8[0x1BA - 0x1B8];
    s16 unk_1BA;                   /* func_800207C8: ratan2 heading of bone 17's matrix column 2, + 0x800 */
    u8  unk_1BC[0x1C2 - 0x1BC];
    s16 unk_1C2;                   /* the same for bone 14 */
    u8  unk_1C4[0x1C8 - 0x1C4];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    s16 unk_1DA;
    s16 unk_1DC;
    u8  unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    Vec3i32 unk_1EC;               /* func_800207C8: bone 11's matrix applied to D_800A3138 (0, 0x1000, 0) */
    Vec3i32 unk_1F8;
    u8  unk_204[0x210 - 0x204];
    LeafPos unk_210[3];            /* func_8002C61C: copy of scratchpad points 0x1F800000 + idx * 0x24 */
    LeafPos unk_234[2];            /* func_8002C61C: copy of scratchpad points 0x1F800048 + idx * 0x18 */
    Vec4i32 unk_24C;
    LeafPos unk_25C;               /* func_80023F08: copy of scratchpad point unk00[idx][0] */
    s32 unk_268;
    s16 unk_26C;
    s16 unk_26E;
    s16 unk_270;
    s16 unk_272;
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
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
    s16 unk_330;
    s16 unk_332[12];               /* queued kinds, unk_330 of them (func_8003047C / func_80030B10) */
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E;                   /* written by func_8001BE20 for the OTHER record */
    u8  unk_34F[0x350 - 0x34F];
    s16 unk_350;
    s16 unk_352;                   /* index into game_GetPlayerData()'s MATRIX * table and SPAD->unkA8[] (func_800204C0) */
    Vec3i32 unk_354;               /* func_800203B4's gte_stlvnl output; func_800204C0's gte_ldlv0 input */
    CpuRoute cpu_route;            /* 0x360 */
    s32 unk_394;
    s16 unk_398;
    s16 unk_39A;
    u8  unk_39C;
    u8  unk_39D;
    s16 unk_39E;
    s16 unk_3A0;
    s16 unk_3A2;
    u16 *unk_3A4;                  /* script list: u16 byte offsets from its own start */
    u16 *unk_3A8[3];               /* per-row cursor into unk_3A4 (func_80055138) */
    u8 *unk_3B4;                   /* script pointer (func_80055B44 / func_80055948) */
    u8  unk_3B8;
    u8  unk_3B9[0x3BC - 0x3B9];
    u8  unk_3BC;
    u8  unk_3BD;
    u8  unk_3BE[0x3C0 - 0x3BE];
    u8  unk_3C0;
    u8  unk_3C1;
    s16 unk_3C2;
    s32 unk_3C4;
    s32 unk_3C8;
    s32 unk_3CC;
    PadState unk_3D0;              /* the pad record func_80055B60 builds for func_8001BE20 */
    u16 unk_3E8;
    u8  unk_3EA[0x3EE - 0x3EA];
    s16 unk_3EE;
    s16 unk_3F0;
    u8  unk_3F2;
    u8  unk_3F3;
    u8  unk_3F4;
    u8  unk_3F5;
    u8  unk_3F6;
    u8  unk_3F7;
    s16 unk_3F8[3];                /* [unk_86] distance bounds (func_80055138) */
    s16 unk_3FE[3];
    s16 unk_404[3];
    s16 unk_40A;
    s8  unk_40C;
    s8  unk_40D;
    s16 unk_40E;
    s16 unk_410;
    s16 unk_412;
    u8  unk_414[8][2];             /* func_80055B60: 8 (target id, count) pairs */
    u8  unk_424;
    u8  unk_425;
    u8  unk_426;
    u8  unk_427;
    s16 unk_428;
    s16 unk_42A;
    s16 unk_42C;
    s16 unk_42E;
    s32 unk_430;
    s32 unk_434;
    s16 unk_438;
    s16 unk_43A;
    s16 unk_43C;
    s16 unk_43E;
    u8  unk_440;
    u8  unk_441;
    u8  unk_442;
    u8  unk_443;
    u8  unk_444[8];                /* func_80056CB8's per-direction results */
} Unk80101EC8Record;                 /* sizeof == 0x44C */

extern Unk80101EC8Record D_80101EC8[];

/* Data symbols */
extern s32 D_800100A4;
extern s32 D_800109C8;
extern u8 D_8008D518;
extern u8 D_8008D538[];
extern u8 D_8008D55C;
extern u8 D_8008D578[];
/* Attachment point sets func_800207C8 rotates by a character's bone matrices:
 * D_8008D86C[unk_0E] (D_8008D864[unk_0E] points; D_8008D774 when unk_12 == 50)
 * and D_8008D88C[unk_14] (two points, when unk_8C != 0).  D_800A3138 is the
 * unit y vector (0, 0x1000, 0). */
extern u8 D_8008D864[8];
extern SVec4i16 *D_8008D86C[8];
extern SVec4i16 *D_8008D88C[32];
extern SVec4i16 D_8008D774[2];
extern SVec4i16 D_800A3138;
extern u8 D_8008D9EC[];
extern u8 D_8008DA08[0x48];         /* 0x8008DA08..0x8008DA4F (asm/data/7D920.data.s dlabel D_8008DA08) */
extern s16 D_8008DA50[];          /* [unk_0A] (func_80023F08) */
extern s16 D_8008DA94[];          /* [unk_0A] (func_80023F08) */
extern s16 D_8008DAD8[];          /* [unk_0A] (func_80023F08) */
extern u16 D_8008DB1C[27][8];      /* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_48 model id (func_80020E74) */
extern u8 D_8008DD5C[27][8];        /* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_84 */
extern u16 D_8008DE34[27][6];       /* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_1C */
extern u16 D_8008DF78[27][6];       /* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_1E / unk_20 */
/* 14-byte record table indexed by the kind field (+0x2) of the 0x64-byte
 * objects at D_80106A78: func_80030580 and func_80031B24 index it with
 * stride 14, func_80030D7C reads +0x0 / +0xA of a record. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    u16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
} Tbl8008E194;
extern Tbl8008E194 D_8008E194[];
/* 8 initialized byte pairs (asm/data/7D920.data.s, 0x8009BD58: {0,0} {1,0}
 * {2,0} {3,0} {4,1} {5,1} {6,1} {7,0}); func_80077904 returns [n][0] and
 * caches [n][1] in D_800A35E0, n = D_8009BD38.unk0 (base + n*2,
 * asm/funcs/func_80077904.s).  Replaces the splat per-byte scalar D_8009BD59. */
extern u8 D_8009BD58[8][2];
/* PsyQ MATRIX layout (include/gte.h), spelled with a local tag for the same
 * reason as Unk80101DF0Mat below (several TUs typedef MATRIX themselves). */
typedef struct { s16 m[3][3]; u16 pad; s32 t[3]; } Obj80106A78Mat;
/* The twelve 0x64-byte object records at 0x80106A78. func_80030580 spawns one
 * (its kind indexes D_8008E194 / D_8008EB80), func_80030D7C moves them,
 * func_80031B24 tests them against both fighters, func_80030208 hands them to
 * the effect calls; kind == -1 marks a free record. Field widths are the
 * consumers' loads and stores (asm/funcs/func_80030580.s, func_80030D7C.s). */
typedef struct {
    s16 unk_00;                    /* 0 at spawn; func_80030D7C adds 1 per step while unk_50 != 0 */
    s16 kind;                      /* -1 = free */
    u8  unk_04;
    u8  unk_05;
    u8  owner;                     /* .index of the spawning record (func_80030580) */
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;                    /* index into the owner's matrix table (func_800300B4) */
    u8  slot;                      /* own index in D_80106A78 once allocated; 0xFF = free (set by func_8003043C at init and by func_80030D7C once kind == -1) */
    u8  unk_0B;
    Obj80106A78Mat mtx;            /* func_8002FF20 builds it (identity, RotMatrixX/Y/Z,
                                      MulMatrix0); func_800300B4 reads it */
    Vec3i32 pos;                   /* += vel each func_80030D7C step */
    Vec3i32 prev_pos;              /* pos before the step */
    Vec3i32 vel;
    s32 unk_50;                    /* != 0: moving */
    s16 rot[3];                    /* RotMatrixX/Y/Z angles; += rot_vel each step */
    u8  unk_5A[0x5C - 0x5A];
    s16 rot_vel[3];
    u8  unk_62[0x64 - 0x62];
} Obj80106A78;                     /* sizeof == 0x64 */
extern Obj80106A78 D_80106A78[12];
extern s8 D_8008E338[27][5];        /* [unk_0A][i] -> Unk80101EC8Record.unk_332[i] (func_8003047C);
                                       0x8008E338..0x8008E3BE, then one alignment byte */
extern u16 D_8008E3C0[28];          /* [unk_0A] -> Unk80101EC8Record.unk_274 */
extern u16 D_8008E3F8[27][4];       /* [unk_0A][i] -> Unk80101EC8Record.unk_276[i] */
extern u16 D_8008E4D0[27][4];       /* [unk_0A][i] -> Unk80101EC8Record.unk_27E[i] */
extern u8 D_8008E5A8[];
extern u8 D_8008E5CC[][8];         /* [unk_0A][unk_0E] of D_80101EC8 */
extern u8 D_8008E6A4[][6];         /* [unk_0A][unk_0E] of D_80101EC8 */
extern u8 D_8008E748;
extern u8 D_8008E75C;
/* 2-byte {a,b} threshold pairs. D_8008EA44: indexed by (type - 2), 5 entries (types 2..6);
   D_8008EBFC: indexed by leaf category, 6 entries. The original binary indexes both at a
   2-byte stride and reads both bytes at the same index (asm/funcs/func_800335D8.s). */
typedef struct {
    u8 a;
    u8 b;
} LeafThreshold;

extern LeafThreshold D_8008EA44[5];
/* per-stage s16 table, 34 entries (0x8008EAC0..0x8008EB03, one data label) */
extern s16 D_8008EAC0[34];
extern s16 D_8008EB04;
extern s16 D_8008EB06;
extern s16 D_8008EB08;
extern s16 D_8008EB0A;
extern s16 D_8008EB0C;
extern s32 D_8008EB10;
extern s32 D_8008EB14;
extern s32 D_8008EB18;
extern u8 D_8008EB1C[][2];
extern u8 D_8008EB28[8][2];         /* [unk_0E][flag] -> Unk80101EC8Record.unk_12 */
extern u8 D_8008EB38[8];            /* [unk_0E] -> Unk80101EC8Record.unk_12 */
/* 6-row tables func_80026DA4 selects by D_80101EC8[0].unk_6A mode (row 0..5): unk0
 * scales the Judge sin/cos offset, unk2 is added to y; D_8008EB6C[row] is
 * passed as func_80032854's arg1. */
typedef struct {
    s16 unk0;
    s16 unk2;
} Tbl8008EB54Entry;
extern Tbl8008EB54Entry D_8008EB54[6];
extern u8 D_8008EB6C[6];
extern u8 D_8008EB80[];
extern u16 D_8008EBA0[22];          /* per-limb angle (func_80031890: [j], j < 22) */
extern s32 D_8008EBCC[];
extern s32 D_8008EBE0[];
extern u8 D_8008EBF4[6];
extern LeafThreshold D_8008EBFC[6];
extern u8 D_8008EC30;
extern s16 D_8008F12C;
extern u8 D_8008F13C;
extern u8 D_8008F19C[];
extern u8 D_8008F1A8[];
extern u8 D_8008F204[];
extern u8 D_800900EC;
extern u8 D_8009016C;
extern u32 D_80090178;
extern u32 D_800905F8;
extern s32 D_80090600;
extern s32 D_80090604;
extern s16 D_80090608;
/* Two per-stage tables (initialized data, asm/data/7D920.data.s), indexed by the
 * stage id func_80046EA0 passes to func_8003DA8C: 38 s32 at 0x8009060C, then
 * s16 pairs at 0x800906A4 (base + id*4, [0] tested, [1] passed to
 * func_8003DBE4; asm/funcs/func_8003DA8C.s), 39 to the next object at
 * 0x80090740 (the last one zero).  Replaces the splat per-halfword scalar
 * StatusUpBuf (pair 0's [1]). */
extern s32 D_8009060C[38];
extern s16 D_800906A4[39][2];
extern u16 D_80094C68[];
/* 0x18-byte per-character status record table (named_syms.txt:
 * g_status_flag_record_table_80099D88). The original binary indexes it by character id
 * with stride 0x18 (id*3<<3) in func_80055138, func_80055948, func_80055B60 and
 * func_80058580: flags halfword at +0, bytes at +3..+8, +0xC, +0xF, +0x14, +0x15. The
 * splat symbols D_80099D8B..D_80099D9D are fields of record 0 (alias rows in
 * undefined_syms_auto.txt, retired with func_80055B60 / func_80058580). */
typedef struct StatusFlagRec {
    u16 flags;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 pick_weight[7];   /* +0x8: func_80058580's 7-entry random pick, indexed by pick */
    u8 script_weight[8]; /* +0xF: func_80058580's script-entry types 0..7 (et), indexed by type */
    u8 unk17;
} StatusFlagRec; /* size 0x18 */
extern StatusFlagRec D_80099D88[];
/* Rows of eight 4-byte entries starting at 0x8009A8C8: each 0x20-byte row ends with a zero
 * entry (0x8009A8E4 / 0x8009A904 / 0x8009A924, asm/data/7D920.data.s). Both readers index the
 * column 1-based, [row][D_800A37A0 - 1]: func_80055138 reads unk0/unk1 (GCC folds the -1
 * into the address, %lo(0x8009A8C4)), func_80058580 the mask halfword (%lo(0x8009A8CA)). */
typedef struct CpuLevelEntry {
    u8 unk0;
    u8 unk1;
    u16 mask;
} CpuLevelEntry;
extern CpuLevelEntry D_8009A8C8[][8];
extern u8 D_8009A9B4[][2];         /* byte pairs (func_80055138) */
extern u8 D_800A3100[][4];         /* [D_8008D9EC flag] -> 3 bytes (func_80041BF4 args), stride 4 */
extern s16 D_800A310C[4];          /* 0x800A310C..0x800A3113 (asm/data/91C98.data.s dlabel D_800A310C) */
extern s32 D_800A3134;
extern s32 D_800A3140;
extern u8 D_800A31DA;
extern u8 D_800A3670;
extern u8 D_800A3671;
extern s16 D_800A367A;
extern s16 D_800A367C;
extern u8 D_800A3680;
extern s32 g_comb_recv_buf_plus_0x4;
extern u8 D_800A3690;
extern s32 g_comb_send_buf_plus_0x4;
extern s16 D_800A36A4;
extern s32 D_800A36AC;
extern s32 D_800A36B4;
extern CdlATV D_800A36B8;
extern s16 D_800A36C2;
extern s32 D_800A36C4;
extern s16 D_800A36C6;
extern u8 D_800A36C8;
extern u16 D_800A36CA;
extern u8 D_800A36CC;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern MoveScript *D_800A36D8;
extern u8 D_800A36E8;
extern u8 D_800A36F0;
extern u8 D_800A36F1;
extern u8 D_800A36F2[2];             /* per-player byte, [unk_04] (func_8003047C) */
extern u8 D_800A36F4;
extern s16 D_800A36F6;
extern u8 D_800A36F9;
extern u8 D_800A36FA;
extern s16 D_800A36FC;
extern u8 D_800A3712;
extern u8 D_800A3713;
extern CdlATV g_cd_atv;
extern s32 D_800A371C;
extern u8 D_800A3728;
extern s8 D_800A3748;
/* 4-entry s16 leaf random/scratch buffer (named_syms.txt: g_leaf_random_buffer). func_800335D8
   walks it from D_800A3750 to D_800A3750+8 at a 2-byte stride; func_80033510 clears it from the
   last element down (asm/funcs/func_80033510.s starts at 0x800A3756). */
extern s16 D_800A3750[4];
extern u8 D_800A3758;
/* libcd's 8-byte CD status/result buffer (u_char result[8]) that CdSync / CdReady
 * fill: func_80036940 and func_80036140 test result[0] (the status byte) and
 * func_80036140 reads result[4] and hands &result[3] / &result[5] (the reported
 * position) to CdPosToInt. A tentative definition in the CD module's file, like
 * the CdlATV blocks: result[0] is read gp-relative, result[4] with lui/%lo
 * (the owner Q62 global COMMON model; the tentative definition is in
 * src/code6cac_b5.c). */
extern u8 g_cd_result[8];
extern u8 D_800A3769;
extern u8 D_800A376A;
extern u8 D_800A376B;
extern u8 D_800A376C;
extern s16 D_800A376E;
extern s32 D_800A3778;
extern u8 D_800A377B;
extern u8 D_800A377C[]; /* round-result table: 0/1/2 per round, indexed by round */
/* Two buffer addresses selected by frame parity: sys_GameInit sets 0x801D8800 /
 * 0x801EBC00, func_80016E60 and main read [D_800A36AC & 1] (base + i*4).
 * Replaces the splat per-word scalar D_800A3774 ([1]). */
extern u32 D_800A3770[2];
extern u8 D_800A3781;
extern u8 D_800A3783;
extern s32 D_800A3784;
extern u8 D_800A3788;
extern s32 g_memcard_fd;
extern u8 D_800A37A0;
extern s32 D_800A37A4;
extern u8 D_800A37B0;
extern u8 D_800A37B4;
extern u8 D_800A37B5;
extern u8 D_800A37B6;
extern s32 D_800A37B8;
extern u8 D_800A37BC;
extern s32 D_800A37C0;
extern u8 D_800A37C6;
extern u8 D_800A37D2;
extern u8 D_800A37D3;
extern u8 D_800A37E0;
extern u8 D_800A37E1;
extern s16 D_800A37E8;
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern u8 D_800A37F8;
extern u8 D_800A3804;
extern s32 D_800A380C;
extern u8 D_800A3816;
extern u8 D_800A3817;
extern s16 D_800A381C;
extern u8 D_800A381E;
extern s32 D_800A3820;
extern s16 D_800A3824;
extern u8 D_800A382D;
extern s16 D_800A382E;
extern s32 D_800A3830;
extern s16 D_800A3834;
extern u8 D_800A3836;
extern s32 D_800A3844;
extern s32 D_800A3858;
extern u8 *D_800A385C;
extern Tbl800A3860Entry *D_800A3860[];
extern u8 D_800A3874;
extern s16 D_800A3876;
extern s32 D_800A3878;
extern s32 D_800A387C;
extern u8 D_800A3880;
extern MotionFrame *D_800A3888[2];  /* per player: preset motion frames (func_80020D70, func_80023F08) */
extern u8 D_800A389A;
extern u8 D_800A389B;
extern u16 D_800A389C;
extern s32 D_800A38A0;
extern u8 D_800A38A4;
extern s16 D_800A38A8;
extern s16 D_800A38AE;
extern u8 D_800A38B0;
extern u8 D_800A38B8;
extern s16 D_800A38BA;
extern u8 D_800A38C0[2];          /* per player: character of the D_800A3888 motion set (0xFF = none) */
extern u16 D_800A38C4[2];         /* per slot: model id in the D_800A3860[i] buffer (func_80020E74); [1] = 0xFFFF: func_8001DB9C started a sequence at 0x80190800 (D_800A3860[1]'s buffer); func_80020D38 calls seq_Reset for it */
extern u8 D_800A38D4;
extern s16 D_800A38DC;
extern u8 D_800A38DE;
extern u8 D_800A38DF;
extern u8 D_800A38E0;
extern u8 D_800A38E1;
extern u8 D_800A38E2;
extern s32 D_800A38E4;
extern u8 D_800A38E8;
extern u8 D_800A38E9;
extern u8 D_800A38EC;
extern u8 D_800A38ED;
extern u8 D_800A38EE;
extern s32 D_800A38F0;
extern s32 D_800A38F4;
extern u8 D_800A38F8;
extern u16 D_800A3904;
extern u8 D_800A3906;
extern u8 D_800A3907;
extern u8 D_800A390C;
extern u8 D_800A390D;
extern s8 D_800A390E;
extern u8 D_800A390F;
extern s16 D_800A3910;
extern u8 D_800A3912;
extern u8 D_800A3913;
extern u8 D_800A3914;
extern u8 D_800A3915;
/* 6-byte per-leaf slot state table (named_syms.txt: g_leaf_slot_state,
   "6-byte slot state table (per-leaf counter byte)"; D_800A391E is the
   separate end marker recorded at named_syms.txt:1555, not an element). */
extern u8 D_800A3918[6];
extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern u8 D_800A3928;
extern u8 D_800A3929;
extern s32 D_800A3D40;
/* The three stage lights' position / direction words: stage_SetLightPosDir
 * stores pos[a2] / dir[a2] (base + a2*4), stage_ClearLighting zeroes all six,
 * stage_ApplyLighting passes pair i to sys_StubEmpty3(pos, dir, i).  Replaces
 * the splat per-word scalars D_800A93B4 / D_800A93B8 / D_800A93C0 / D_800A93C4. */
extern s32 g_stage_light_pos[3];
extern s32 g_stage_light_dir[3];
/* 0x44-byte record shared by the two camera-target objects at 0x800F5328 and
 * 0x800F6608. Field span evidenced by func_8001B294 (initialises +0x00/04/08/10/12/14/
 * 18/1E/30/32/34/38/3A/3C on the 0x800F6608 object) and func_8001B3C0 (the same
 * treatment of 0x800F5328 at +0x00/04/08/30/32/34/38/3A/3C/40); +0x00/04/08 are the
 * Vec3 that func_8001BC70/func_8001BCF0 already copy through a struct assignment.
 * Replaces the per-word splat symbols D_800F532C..D_800F5368 / D_800F660C..D_800F6644. */
typedef struct Rec44 {
    s32 w0; s32 w4; s32 w8; s32 wC;
    s16 h10; s16 h12; s16 h14; s16 h16;
    s32 w18;
    s16 h1C; u8 b1E; u8 b1F;
    s32 w20; s32 w24; s32 w28; s32 w2C;
    s16 h30[2][4];                 /* two 4 x s16 limit vectors; func_8001A820 passes h30[p] to func_8001A67C */
    u8 b40; u8 b41; u8 b42; u8 b43;
} Rec44;
/* 0x1C-byte record: func_8003993C walks an array of these at D_800A36EC with element
 * stride 0x1C (base + i*0x38 + {0,0x1C} + D_800A3748*0x1C, asm/funcs/func_8003993C.s:57-89)
 * and passes two of them to func_8001BAE4 / func_8001BBD8. */
typedef struct Rec1C {
    s16 h0; s16 h2; s16 h4; s16 h6; s16 h8; s16 hA; s16 hC; s16 hE;
    s32 w10; s32 w14; s32 w18;
} Rec1C;
extern Rec44 D_800F5328;
extern Rec44 D_800F6608;
/* Per-round snapshot: func_800340A0 stores both players' counters
 * (D_800A3898 / D_800A3899) for round D_800A3874 at base + round*2 / +1
 * (asm/funcs/func_800340A0.s); func_80034200 packs the pairs into D_800A3784,
 * func_8003C42C sums them.  16 bytes up to D_800F6608.  Replaces the splat
 * per-byte scalar D_800F65F9 (entry [0][1]). */
extern u8 D_800F65F8[8][2];
extern s16 D_800F68E0[];
extern s32 g_pad_buf_plus_0x4;
extern s32 g_pad_buf_plus_0x24;
extern s32 g_pad_buf_plus_0x28;
extern s32 D_800FF5C8;
extern s32 D_800FF5CC;
extern s32 D_800FF5D0;
extern s16 D_800FF5D8;
extern s16 D_800FF5DA;
extern s16 D_800FF5DC;
extern s32 D_800FF5E0;
extern u8 D_80101BF0;
/* Two 0x58-byte records set up side by side by func_80049E4C: 0x80101DF0
 * (unk0 = 0x64, unk8 = 5; pointer stored to D_800A3708) and 0x800FF638
 * (unk0 = 0x65, unk8 = 2; pointer stored to D_800A370C). func_800418D0
 * passes a negated stack copy of xf.rot (+0x10..0x14; the record is not
 * written) and &work (+0x38) to g_anim_func_table[unk8], then copies work
 * to xf.mat (+0x18);
 * func_80046BF4 writes work.t (+0x4C/+0x50/+0x54) just before that call.
 * func_800475A4 passes &xf.rot to g_anim_func_table[0] and &xf.mat to
 * ApplyMatrix; func_8003E6D8 forms one base at &xf (0x80101E00) and passes
 * base+0x1C (xf.mat.t). g_cam_bone_data2 (0x800EEDF0) and D_800EF070 are
 * two more records of this layout: camera_InitRotation initialises the
 * first (u8/s8 stores at +0x00/+0x01, s16 stores at +0x02/+0x04/+0x08/+0x0A,
 * s32 at +0x0C, rot, work.t, then xf.mat = work; it does not write +0x06),
 * func_800477E8 sets up the second (+0x06 as s16) and passes it to
 * func_800417D0, which reads +0x06 as an s16 state.
 * Rot / Mat are the PsyQ SVECTOR / MATRIX layouts
 * (include/gte.h), spelled with local tags because several TUs typedef
 * SVECTOR/MATRIX themselves. */
typedef struct { s16 vx, vy, vz, pad; } Unk80101DF0Rot;
typedef struct { s16 m[3][3]; u16 pad; s32 t[3]; } Unk80101DF0Mat;
typedef struct {
    Unk80101DF0Rot rot; /* +0x00 */
    Unk80101DF0Mat mat; /* +0x08 */
} Unk80101DF0Xform;
typedef struct {
    u8 unk0;               /* +0x00 */
    s8 unk1;               /* +0x01 */
    s16 unk2;              /* +0x02 */
    s16 unk4;              /* +0x04 */
    s16 unk6;              /* +0x06 */
    s16 unk8;              /* +0x08 g_anim_func_table index */
    s16 unkA;              /* +0x0A */
    s32 unkC;              /* +0x0C */
    Unk80101DF0Xform xf;   /* +0x10 */
    Unk80101DF0Mat work;   /* +0x38 */
} Unk80101DF0Record;      /* 0x58 */
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
extern Unk80101DF0Record g_cam_bone_data2;
extern Unk80101DF0Record D_800EF070;
/* The 8 light-setup records at 0x800F62E0 (0x60 each; base + n*0x60 in
 * func_800470B0, func_80049F4C).  func_8004A09C fills one from a 22-halfword
 * template: the color matrix +0x38 (m[3][3]), the three lights' pitch / yaw /
 * on at +0x00 / +0x08 / +0x10, the u8 back color +0x58 and the s16 +0x5C;
 * func_8004A1FC then turns light i's pitch / yaw into the light matrix row i
 * (+0x18 + i*6, zero when off), scaled by +0x5C.  func_80049F4C hands record
 * 0's color matrix and back color to SetColorMatrix / SetBackColor; the light
 * matrix goes to gte_MulMatrix0ClearTrans (asm/funcs/func_8004A940.s:70-72);
 * func_80046F24 / func_800470B0 read its row 0.  Replaces the splat per-field
 * scalars D_800F62E2..D_800F64BA, g_gte_color_matrix_data and
 * g_gte_back_color_r/g/b. */
typedef struct {
    s16 pitch;
    s16 yaw;
    s16 on;
    s16 pad;
} Unk800F62E0Light;
typedef struct {
    Unk800F62E0Light light[3]; /* +0x00 */
    Unk80101DF0Mat lmat;       /* +0x18 */
    Unk80101DF0Mat cmat;       /* +0x38 */
    u8 back[3];                /* +0x58 */
    s16 unk5C;                 /* +0x5C */
} Unk800F62E0Rec;              /* 0x60 */
extern Unk800F62E0Rec D_800F62E0[8];
/* 0x800F66A0: the rotation-to-matrix handlers a transform node's unk8 selects
 * (rot -> matrix, PsyQ RotMatrix shape).  func_80042E90 fills [0] ZYX, [2] ZXY,
 * [4] YXZ, [5] XYZ ([1] and [3] are never written); the nodes set unk8 to 0, 2,
 * 4 and 5, and func_8003EDC0 / func_800417D0 / func_800418D0 / camera_InitRotation
 * index it by unk8 (4-byte stride).  _svm_vab_vh follows at 0x800F66B8. */
typedef void (*AnimRotFunc)(Unk80101DF0Rot *, Unk80101DF0Mat *);
extern AnimRotFunc g_anim_func_table[6];
/* The 0x68-byte records of the table game_GetCharData returns: a transform
 * node of the Unk80101DF0Record layout, then a byte flag. func_8003EDC0
 * fills them from a stream and calls g_anim_func_table[unk8] on &xf.rot /
 * &xf.mat (func_800418D0's call); func_8003E6D8 and func_8003EB84 queue each
 * one on the D_800A3820 list at most once, guarded by unk58. */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    u8 unk58;               /* +0x58 */
    u8 unk59[0xF];          /* +0x59 */
} Unk800A6690Rec;           /* 0x68 */
/* The 0x68-byte entries of the D_800A9CF8.unkC table (the buffer
 * func_80044670 is handed; it returns the end, base + count * 104). Each
 * is a transform node of the Unk80101DF0Record layout paired with one
 * game_GetCharData entry of the same index (D_800A9CF8.unk10):
 * func_8004473C initialises the node and copies the paired entry's
 * xf.mat.t into work.t; func_80044B30 / func_80044800 run the node from
 * the D_800A9CF8.unk8 key frames and queue it on the D_800A3820 list.
 * unk58 is a word frame counter (lw/sw: -1 idle, 0 started by
 * func_80044B30, -2 done), unk5C the word Y angle func_80044B30 stores,
 * unk60 an s16 fade level (lh/sh in func_80044800); +0x62..0x67 are not
 * accessed. Not the Unk800A6690Rec layout: that table's +0x58 is a byte
 * (lbu/sb in func_8003E6D8 / func_8003EB84). */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 unk5C;              /* +0x5C */
    s16 unk60;              /* +0x60 */
    s8 pad62[6];            /* +0x62 */
} Unk800A9CF8Entry;         /* 0x68 */
/* The 16-byte records func_8003EDC0 fills ahead of those (unk8 / unkC = the
 * grid cell's column / row * 2000 - 32000); func_8003E6D8 and func_8003EB84
 * set unk6 and the unk7 bits and queue them on the D_800A3820 list. */
typedef struct {
    s16 unk0;  /* +0x00 */
    s16 unk2;  /* +0x02 */
    u16 unk4;  /* +0x04 */
    u8 unk6;   /* +0x06 */
    u8 unk7;   /* +0x07 */
    s32 unk8;  /* +0x08 */
    s32 unkC;  /* +0x0C */
} Unk800A4750Rec;          /* 0x10 */
/* One entry of the CD file table (sweep-2026-09-24 data_manifest.csv:18-19, CONFIRM:
 * 159/159 entries equal the disc's ISO9660 directory records). */
typedef struct {
    CdlLOC loc;                    /* the file's start position */
    u32 size;                      /* the file's length in bytes */
} CdFileEntry;

/* The CD file table at 0x8008EC34: one 8-byte record per disc file, indexed by the
 * file numbers func_80036EA8 forms. `loc` is sought (cdrom_StartRead copies the record
 * into D_80101E58.rec.pair, whose position is sought; cdrom_LoadExec seeks to it
 * directly); `size` is cdrom_StartRead's sector count, cdrom_GetFileSize's result and
 * cdrom_StartAudio's end position. */
extern CdFileEntry g_cd_file_table[159]; /* 0x8008EC34..0x8008F12B */

/* The replay-camera / CD-read words at 0x80101E60..0x80101EA7: the tail of
 * CdState D_80101E58 below, where the evidence that they are one object with
 * its head is set out.  Member widths follow the original accesses;
 * 0x80101E91..93 is the compiler's alignment padding.  unk3A (0x80101E9A)
 * lies inside the proven span's object (its size is a multiple of 4) and is
 * func_80036140's halfword (sh 80036548, lhu 80036778 / sh 80036788).
 *
 * unk3C..unk44 (0x80101E9C..0x80101EA7) are in the object by compiler
 * necessity (aggregate-merge prong (a), (a1)/(a2)/(a4')): func_80036140 is
 * compiled -G8, and there its read-modify-writes of 0x80101E9C and
 * 0x80101EA4 keep the address in a register (la; lX 0(r); sX 0(r)) only for
 * a variable larger than 8 bytes -- a smaller one is small data, and cse
 * folds any pointer back to the symbol; the original compiler agrees.
 * Separate variables, block- or function-scope pointers, an object ending at
 * 0x80101E9F or 0x80101EA3, and a separate 12-byte record at 0x80101E9C all
 * miss func_80036140's bytes; only this object matches.  unk3E and expected_pos
 * lie inside that span but func_80036140 never touches them: they are typed
 * by their other users' original accesses (owner rulings Q13/Q14): unk3E by
 * game_FrameLoop / cdrom_StartRead (u16, the lhu at 80036F9C); expected_pos
 * by cdrom_ReadyCallback / func_80036940 (s32: no access reveals its
 * signedness, and s32 / u32 build byte-identical).
 * Evidence: pre-slim-2026-10-01:memory/grind/func_80036140/evidence.md.
 *
 * The 8-byte `pair` is also one CdFileEntry by the table it is copied from: the
 * source is an element of g_cd_file_table, copied as a whole CdFileEntry
 * aggregate (cdrom_StartRead, cdrom_StartAudio).  The CdPosToInt/CdIntToPos calls on it evidence
 * only `pair.loc`: CdIntToPos (src/system.c) writes just p[0..2]. */
typedef struct {
    s16 unk00; /* 0x80101E60 */
    s16 unk02; /* 0x80101E62 */
    s16 unk04; /* 0x80101E64 */
    s16 unk06; /* 0x80101E66 */
    s16 unk08; /* 0x80101E68 */
    s16 unk0A; /* 0x80101E6A */
    CdFileEntry pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
    s32 unk18; /* 0x80101E78 */
    s32 unk1C; /* 0x80101E7C */
    s32 sectors_remaining; /* 0x80101E80 */
    s32 dest_buffer; /* 0x80101E84 */
    s32 unk28; /* 0x80101E88 */
    s32 unk2C; /* 0x80101E8C */
    u8 unk30; /* 0x80101E90 */
    s32 unk34; /* 0x80101E94 */
    s16 unk38; /* 0x80101E98 */
    s16 unk3A; /* 0x80101E9A */
    s16 unk3C; /* 0x80101E9C */
    u16 unk3E; /* 0x80101E9E */
    s32 expected_pos; /* 0x80101EA0 */
    s32 unk44; /* 0x80101EA4 */
} ReplayCamRec;

/* The CD module's state block, ONE object of 0x50 bytes at 0x80101E58.  Owner
 * ruling Q43 bounds it to the span proven: 0x80101E58..0x80101E99 by the
 * original binary's addressing, through 0x80101EA7 by func_80036140's compiler
 * necessity (ReplayCamRec above; aggregate-merge prong (a)).  The first span
 * is proven by three links:
 *   - 0x80101E58..0x80101E62 is one object: cdrom_StartAudio forms the
 *     CdlSetfilter parameter (file, chan) at 0x80101E58 as &0x80101E62 - 0xA
 *     (800370AC addiu a1,s0,-0xA; 800370B0 sb v0,-0xA(s0)).  Only those two
 *     bytes are accessed; 0x80101E5A..5B is the compiler's alignment padding
 *     before unk04.  cse relates two constant addresses only when
 *     they are offsets of one symbol.
 *   - 0x80101E6C..0x80101E99 is one object: func_80036940 forms &rec.pair
 *     (0x80101E6C) as &0x80101E8C - 0x20 (80036A74) and as &0x80101E98 - 0x2C
 *     (80036B08).
 *   - 0x80101E60..62 and 0x80101E6C are in one object (aggregate-merge prong
 *     (a), (a1)/(a2)): cdrom_StartAudio reloads rec.unk00 (lh at 8003703C)
 *     only after its CdFileEntry copy into rec.pair (sw at 8003702C/80037034).
 *     sched1 orders the two through true_dependence (sched.c:817) ->
 *     memrefs_conflict_p (sched.c:614): SIZE_FOR_MODE(BLKmode) is 0, so the
 *     aggregate store conflicts with the halfword load only when both
 *     addresses share one base symbol_ref (sched.c:777).  As separate objects
 *     there is no dependence and the load is hoisted above the copy.  One
 *     object matches cdrom_StartAudio under both cc1 and the original cc1psx;
 *     a cut at 0x80101E64, 0x80101E68 or 0x80101E6C misses under both.
 *     Evidence: pre-slim-2026-10-01:memory/grind/cdrom_StartAudio/evidence.md.
 */
typedef struct {
    u8 file; /* 0x80101E58 */
    u8 chan; /* 0x80101E59 */
    s32 unk04; /* 0x80101E5C */
    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */
} CdState;

extern CdState D_80101E58;

extern s32 D_80102760;
extern s32 D_80102764;
extern s32 D_80102768;
extern s32 D_80102770;
/* Practice-lesson parameter block 0x80102778..0x80102787 (func_8001C444 sets
 * every byte of it except 0x82/0x83): two u16 values, three per-player byte
 * pairs kept as one array (unk_4[2 * k + player], [0] = P1, [1] = P2), a
 * fourth per-player pair only other functions touch (unk_A, indexed by player
 * in func_80022F34) and four single bytes. One object: func_80034708 reaches
 * unk_4 and unk_E as offsets from the address of unk_C (layout evidence:
 * pre-slim-2026-10-01:memory/grind/func_80034708/evidence.md). */
typedef struct {
    u16 unk_0[2];
    u8 unk_4[6];
    u8 unk_A[2];
    u8 unk_C;
    u8 unk_D;
    u8 unk_E;
    u8 unk_F;
} PracticeParams;
extern PracticeParams D_80102778;
extern PadState g_pad_state;
extern s32 D_801027B0[][5];
extern u8 D_80104E88;
extern s32 MotDataBaseAddress;
/* func_800338CC's list: the set bit numbers of D_80106A50.unk_00 & mask,
 * shuffled, plus up to three appended values (D_800A391F entries; func_80033BC0
 * reads entry D_800A3783, then advances it).  One u8 array, indexed i, i - 1
 * and 10 by func_800338CC (asm/funcs/func_800338CC.s:118-136); 24 bytes up to
 * the next object (_svm_vab_total, 0x801077C8).  Replaces the splat per-byte
 * scalars D_801077AF (entry -1) and D_801077BA (entry 10). */
extern u8 D_801077B0[24];

extern LeafPos D_80107850[6];

/* Functions */
extern void func_8001B748(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void func_8003D52C(u8 *, s32, ...);
extern void func_80021A98(s32, MoveScript *, s32);
extern void func_80022580(s32, s32, s32, s32, s32);
extern s32 func_80036EA8(s32, s32);
extern void func_8003A728(s32);
extern void func_8003AE5C(u8 *);
extern void func_8003DE14(s16 *, s32);
extern void func_8003F1E4(s32);
extern void func_80041688(s32, s32);
extern void func_80048BA4(s32, s32, s32);
extern void func_800493E4(s32);
extern void func_800494D4(s32, s32);
extern void func_80049584(s32);
extern s32 func_8005B8B8(s32);
extern void func_8005C6D0(void);
struct EXEC;
extern void Exec(struct EXEC *, s32, s32 *);
extern s32 format(s32 *);
extern s32 sprintf(char *, char *, ...);
extern void ResetGraph(s32);
extern void SetDispMask(s32);
extern void DrawSync(s32);
extern void CdInit(void);
extern void CdFlush(void);
extern void CdSetDebug(s32);
extern void CdReadyCallback(s32);
extern s32 CdRead(s32, s32, s32);
extern s32 CdReadSync(s32, s32);
extern void SsSetSerialVol(s32, s32, s32);
extern s32 _comb_control(s32, s32, s32);
extern s32 func_80038C70(void);
extern void func_8003E2D8(s32, s32, s32, s32);
extern void func_80036940(void);

#endif /* CODE6CAC_H */
