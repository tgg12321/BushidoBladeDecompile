#ifndef CODE6CAC_H
#define CODE6CAC_H

/* Shared declarations for the code6cac module family */

#include "common.h"

/* Named globals */
extern s16 StatusUpBuf;
extern u8 cpu_practice_honmokuroku_data_tbl[][4];
/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF: 0x8008D118..0x8008D517
 * (0x400 bytes; the first 8 are the words after DelDrv in src/main_post.c,
 * the rest asm/data/7D920.data.s dlabel D_8008D120). */
extern u8 g_sqrt_table_u8[0x400];
extern s32 menuDat;
/* 0x1B8-byte per-character records (func_8002A458 / func_800206B0 walk them by byte offset). */
extern u8 D_800F5F68[];
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
 * f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). */
typedef struct {
    u8 pad00[0x14];
    s16 f14;
    u8 pad16[0x4E - 0x16];
    u16 f4E[3];
    u16 f54[3][3];
    u16 f66[11][3];
} Tbl800A3860Entry;

/* s32 x/y/z triple.  PracticeMenuRec's position-like triples are copied as
 * whole 12-byte objects (func_80022580), and func_80021DB0 writes one through
 * its out parameter. */
typedef struct { s32 x, y, z; } Vec3i32;
/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array"). */
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} LeafPos;
/* PsyQ VECTOR / SVECTOR layouts (include/gte.h), spelled with local tags for
 * the same reason as Unk80101DF0Rot below.  func_80022580 copies
 * PracticeMenuRec's +0xB8 and +0x104 as whole 16-byte VECTORs (pad included)
 * and +0x1C8 as a whole 8-byte SVECTOR. */
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
typedef struct { s16 vx, vy, vz, pad; } SVec4i16;

/* Per-character / practice-menu record table (base 0x80101EC8, stride 0x44C,
 * 4 records).  Schema: docs/naming/CHAR_STRUCT_SCHEMA.md; base symbol:
 * named_syms.txt:345 (g_practice_menu_table).  Only the fields reached by C so
 * far are named; the rest is reserved padding.  func_80022580 initializes
 * record [idx] (every field it writes is declared at its offset). */
/* One CPU path waypoint (PracticeMenuRec.unk_364[]): func_800571C0 writes x/z from the
 * opponent's position and kind 2; func_80058580 walks them back to front. */
typedef struct CpuWaypoint {
    s16 x;
    s16 z;
    u8 kind;
    u8 unk5;
} CpuWaypoint;

typedef struct PracticeMenuRec {
    struct PracticeMenuRec *unk_00; /* the other record: [1] for record 0, else [0] (func_80022580) */
    s16 unk_04;                    /* this record's own index (func_80022580) */
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
    u8  unk_22[0x2C - 0x22];
    s32 unk_2C;
    s32 unk_30;
    u8  unk_34[0x3C - 0x34];
    s32 unk_3C;
    s16 unk_40;
    u8  unk_42[0x50 - 0x42];
    u8 *unk_50;                    /* entry whose byte 8 bounds unk_40 (func_80058580) */
    u8  unk_54[0x58 - 0x54];
    u8 *unk_58;                    /* byte 3 read by func_80056FE8 */
    u8  unk_5C[0x5E - 0x5C];
    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */
    u8  unk_60[0x6A - 0x60];
    u16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */
    u8  unk_6C[0x72 - 0x6C];
    s16 unk_72;
    u8  unk_74[0x7C - 0x74];
    s32 unk_7C;
    u8  unk_80[0x84 - 0x80];
    s16 unk_84;
    s16 unk_86;                    /* row of unk_3A8 / unk_3F8 / unk_3FE / unk_404 */
    s16 unk_88;
    s16 unk_8A;
    s16 unk_8C;                    /* != 0: the second blade (unk_234 / SPAD unk48 points) is live */
    s16 unk_8E;
    s16 unk_90;
    s16 unk_92;
    u8  unk_94[0x96 - 0x94];
    s16 unk_96;
    u8  unk_98[0xA0 - 0x98];
    u8  unk_A0;
    u8  unk_A1[2];
    u8  unk_A3[2];
    u8  unk_A5[0xAD - 0xA5];
    u8  unk_AD;                    /* != 0: func_8002C61C re-runs func_800283D0 for both records */
    u8  unk_AE;
    u8  unk_AF;
    u8  unk_B0;
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3[0xB8 - 0xB3];
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
    u8  unk_154[0x156 - 0x154];
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
    u8  unk_198[0x1C8 - 0x198];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    u8  unk_1DA[0x1DC - 0x1DA];
    s16 unk_1DC;
    u8  unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    u8  unk_1EC[0x1F8 - 0x1EC];
    Vec3i32 unk_1F8;
    u8  unk_204[0x210 - 0x204];
    LeafPos unk_210[3];            /* func_8002C61C: copy of scratchpad points 0x1F800000 + idx * 0x24 */
    LeafPos unk_234[2];            /* func_8002C61C: copy of scratchpad points 0x1F800048 + idx * 0x18 */
    Vec4i32 unk_24C;
    u8  unk_25C[0x268 - 0x25C];
    s32 unk_268;
    s16 unk_26C;
    s16 unk_26E;
    s16 unk_270;
    s16 unk_272;
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
    u8  unk_288[0x28C - 0x288];
    s32 unk_28C;
    u8  unk_290[0x31A - 0x290];
    s16 unk_31A;
    u8  unk_31C[0x330 - 0x31C];
    s16 unk_330;
    s16 unk_332;
    u8  unk_334[0x34A - 0x334];
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E;                   /* written by func_8001BE20 for the OTHER record */
    u8  unk_34F[0x350 - 0x34F];
    s16 unk_350;
    u8  unk_352[0x362 - 0x352];
    u8  unk_362;                   /* number of waypoints in unk_364 */
    u8  unk_363;
    CpuWaypoint unk_364[8];
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
    u8  unk_3D0[0x3D8 - 0x3D0];
    s32 unk_3D8;
    s32 unk_3DC;
    s32 unk_3E0;
    s32 unk_3E4;
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
    u8  unk_414[0x424 - 0x414];
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
    u8  unk_43E[0x440 - 0x43E];
    u8  unk_440;
    u8  unk_441;
    u8  unk_442;
    u8  unk_443;
    u8  unk_444[8];                /* func_80056CB8's per-direction results */
} PracticeMenuRec;                 /* sizeof == 0x44C */

/* Pad input record (0x18 bytes) at 0x80102788.  func_80019568 fills it each
 * frame from the two pads (one u16 half per player in each word):
 * held = current bits, pressed = held & ~previous, released = ~held & previous,
 * unheld = ~held; func_800194F4 resets it (4, 4, 0, 0, 0, -1).  func_8001BE20
 * copies the whole record to its caller's buffer, and func_8001BE08 clears the
 * four bit words of such a buffer. */
typedef struct PadState {
    s16 unk_00[4];                 /* [0..1] and [2..3] filled pairwise by func_80019568 */
    u32 held;                      /* 0x08 */
    u32 pressed;                   /* 0x0C */
    u32 released;                  /* 0x10 */
    u32 unheld;                    /* 0x14 */
} PadState;                        /* sizeof == 0x18 */

extern PracticeMenuRec g_practice_menu_table[];

/* Data symbols */
extern s32 D_800100A4;
extern s32 D_800109C8;
extern u8 D_8008D518;
extern u8 D_8008D538[];
extern u8 D_8008D55C;
extern u8 D_8008D578[];
extern u16 D_8008D59E;
extern u8 D_8008D864;
extern s32 D_8008D86C;
extern s32 D_8008D88C;
extern u8 D_8008D9EC[];
extern u8 D_8008DA08[0x48];         /* 0x8008DA08..0x8008DA4F (asm/data/7D920.data.s dlabel D_8008DA08) */
extern s16 D_8008DA50;
extern s16 D_8008DA94;
extern s16 D_8008DAD8;
extern u8 D_8008DB1C;
extern u8 D_8008DD5C[27][8];        /* [unk_0A][unk_0E] -> PracticeMenuRec.unk_84 */
extern u16 D_8008DE34[27][6];       /* [unk_0A][unk_0E] -> PracticeMenuRec.unk_1C */
extern u16 D_8008DF78[27][6];       /* [unk_0A][unk_0E] -> PracticeMenuRec.unk_1E / unk_20 */
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
extern u8 D_8008E338;
extern u16 D_8008E3C0[28];          /* [unk_0A] -> PracticeMenuRec.unk_274 */
extern u16 D_8008E3F8[27][4];       /* [unk_0A][i] -> PracticeMenuRec.unk_276[i] */
extern u16 D_8008E4D0[27][4];       /* [unk_0A][i] -> PracticeMenuRec.unk_27E[i] */
extern u8 D_8008E5A8[];
extern u8 D_8008E5CC[][8];         /* [unk_0A][unk_0E] of g_practice_menu_table */
extern u8 D_8008E6A4[][6];         /* [unk_0A][unk_0E] of g_practice_menu_table */
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
extern u8 D_8008EB1C;
extern u8 D_8008EB28[8][2];         /* [unk_0E][flag] -> PracticeMenuRec.unk_12 */
extern u8 D_8008EB38[8];            /* [unk_0E] -> PracticeMenuRec.unk_12 */
/* 6-row tables func_80026DA4 selects by g_practice_menu_table[0].unk_6A mode (row 0..5): unk0
 * scales the Judge sin/cos offset, unk2 is added to y; D_8008EB6C[row] is
 * passed as func_80032854's arg1. */
typedef struct {
    s16 unk0;
    s16 unk2;
} Tbl8008EB54Entry;
extern Tbl8008EB54Entry D_8008EB54[6];
extern u8 D_8008EB6C[6];
extern u8 D_8008EB80[];
extern u16 D_8008EBA0;
extern s32 D_8008EBCC[];
extern s32 D_8008EBE0[];
extern u8 D_8008EBF4[6];
extern LeafThreshold D_8008EBFC[6];
extern u8 D_8008EC30;
extern u32 g_cd_file_table_plus_0x4;
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
extern s16 D_800906A4;
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
extern u32 D_800A3220;
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
/* libcd CdlATV, the attenuator block CdMix takes (CdMix(CdlATV *)): the CD-audio
 * mix currently applied (g_cd_atv, cdrom_SetMix) and the fade target
 * (D_800A36B8, func_80035F78) that func_80036140 steps toward and finally copies
 * over it with one struct assignment (the unaligned lwl/lwr/swl/swr at 80036310).
 * Both were tentative definitions in the CD module's file: ASPSX 2.34 gives such
 * a COMMON symbol gp only at its base, so byte 0 is gp-relative and bytes 1..3
 * are lui/%lo in all three accessors. Modelled by maspsx for every file from the
 * declarations (owner Q62, 2026-09-30); the tentative definitions are in
 * the CD module's two -G8 units, src/code6cac_b4.c and src/code6cac_b5.c. */
typedef struct {
    u8 val0;
    u8 val1;
    u8 val2;
    u8 val3;
} CdlATV;
extern CdlATV D_800A36B8;
extern s16 D_800A36C2;
extern s32 D_800A36C4;
extern s16 D_800A36C6;
extern u8 D_800A36C8;
extern u16 D_800A36CA;
extern u8 D_800A36CC;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern s32 D_800A36D8;
extern u8 D_800A36E8;
extern u8 D_800A36F0;
extern u8 D_800A36F1;
extern u8 D_800A36F2;
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
extern s32 D_800A3864;
extern u8 D_800A3874;
extern s16 D_800A3876;
extern s32 D_800A3878;
extern s32 D_800A387C;
extern u8 D_800A3880;
extern s32 D_800A3888;
extern s32 D_800A388C;
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
extern u8 D_800A38C0;
extern u8 D_800A38C1;
extern u16 D_800A38C6;
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
extern u8 D_800F65F9;
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
 * base+0x1C (xf.mat.t). Rot / Mat are the PsyQ SVECTOR / MATRIX layouts
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
    u8 unk2[6];            /* +0x02 */
    s16 unk8;              /* +0x08 g_anim_func_table index */
    s16 unkA;              /* +0x0A */
    s32 unkC;              /* +0x0C */
    Unk80101DF0Xform xf;   /* +0x10 */
    Unk80101DF0Mat work;   /* +0x38 */
} Unk80101DF0Record;      /* 0x58 */
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
typedef struct {
    s32 a;
    s32 b;
} CamPair;

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
 * Scratch whole-file scores: separate variables 18, block-local pointers 18,
 * function-scope pointers 43, the object ending at 0x80101E9F or 0x80101EA3
 * (0x80101EA4 separate) 8, a separate 12-byte record at 0x80101E9C 8 (and
 * cdrom_ReadyCallback 12), this object 2 (the jump-table operand only, which
 * the link settles); linked, this object scores 0.  unk3E and expected_pos
 * lie inside that span but func_80036140 never touches them: they are typed
 * by their other users' original accesses (owner rulings 2026-09-26
 * Q13/Q14): unk3E by game_FrameLoop / cdrom_StartRead (u16, the lhu at
 * 80036F9C); expected_pos by cdrom_ReadyCallback / func_80036940 (s32: no
 * access reveals its signedness, and s32 / u32 build byte-identical).
 * Measurements and dumps: pre-slim-2026-10-01:memory/grind/func_80036140/evidence.md; scratch
 * rows pre-slim-2026-10-01:memory/grind/func_80036140/q62/runs.txt, linked
 * pre-slim-2026-10-01:memory/grind/func_80036140/q62/landed_sandbox.txt.
 *
 * The 8-byte `pair` is also one CamPair by the table it is copied from: the
 * source is indexed `&g_cd_file_table + i*8` and copied as a whole CamPair
 * aggregate (cdrom_StartRead, cdrom_StartAudio), so the table is an array of
 * this same 8-byte record.  The CdPosToInt/CdIntToPos calls on it evidence
 * only `pair.a`: CdIntToPos (src/system.c) writes just p[0..2]. */
typedef struct {
    s16 unk00; /* 0x80101E60 */
    s16 unk02; /* 0x80101E62 */
    s16 unk04; /* 0x80101E64 */
    s16 unk06; /* 0x80101E66 */
    s16 unk08; /* 0x80101E68 */
    s16 unk0A; /* 0x80101E6A */
    CamPair pair; /* 0x80101E6C .. 0x80101E73 */
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
 * ruling Q43 (2026-09-30, docs/grind/owner-rulings-2026-09-26.md) bounds it to
 * the span proven: 0x80101E58..0x80101E99 by the original binary's addressing,
 * through 0x80101EA7 by func_80036140's compiler necessity (ReplayCamRec
 * above).  Q43 dropped 0x80101E9C..0x80101EA7 because that part "lost its
 * proof when func_80036140 went back to assembly"; with func_80036140 in C
 * the proof is re-made on its own body (aggregate-merge prong (a)), the case
 * Q43 anticipated, not a reversal of it.  The first span is proven by three
 * links:
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
 *     only after its CamPair copy into rec.pair (sw at 8003702C/80037034).
 *     sched1 orders the two through true_dependence (sched.c:817) ->
 *     memrefs_conflict_p (sched.c:614): SIZE_FOR_MODE(BLKmode) is 0, so the
 *     aggregate store conflicts with the halfword load only when both
 *     addresses share one base symbol_ref (sched.c:777).  As separate objects
 *     there is no dependence and the load is hoisted above the copy.  One
 *     object: cdrom_StartAudio 0 under both cc1 and the original cc1psx; cut
 *     at 0x80101E64, 0x80101E68 or 0x80101E6C: 8 (cc1) / 12 (cc1psx).
 *     Dumps and runs: pre-slim-2026-10-01:memory/grind/cdrom_StartAudio/evidence.md.
 */
typedef struct {
    u8 file; /* 0x80101E58 */
    u8 chan; /* 0x80101E59 */
    s32 unk04; /* 0x80101E5C */
    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */
} CdState;

extern CdState D_80101E58;

extern s16 D_80101EE8;
extern s32 D_80101F04;

extern s16 D_80101F10;
extern s16 D_80101F12;
extern s16 D_80101F14;
extern s16 D_80101F42;
extern s16 D_80101F4C;
extern s16 D_80101F4E;
extern s16 D_80101F5E;
extern u8 D_80101F79;
extern u8 D_80101F7A;
extern u8 D_80101F7B;
extern s32 D_80101F80;
extern s32 D_80101F84;
extern s32 D_80101F88;
extern s32 D_80101FA4;
extern s32 D_80101FB0;
extern s32 D_80101FB4;
extern s32 D_80101FB8;
extern s32 D_80101FBC;
extern s32 D_80101FC0;
extern s32 D_80101FC4;
extern s32 D_80101FCC;
extern s32 D_80101FD0;
extern s32 D_80101FD4;
extern s32 D_80101FDC;
extern s32 D_80101FE0;
extern s32 D_80101FE4;
extern s32 D_80101FEC;
extern s32 D_80101FF0;
extern s32 D_80101FF4;
extern s32 D_80101FFC;
extern s32 D_80102000;
extern s32 D_80102004;
extern s32 D_8010200C;
extern s32 D_80102010;
extern s16 D_80102014;
extern s16 D_80102016;
extern s16 D_80102018;
extern s16 D_8010201A;
extern s32 D_8010203C;
extern s32 D_80102040;
extern s32 D_80102044;
extern s32 D_80102054;
extern s32 D_80102058;
extern s32 D_8010205C;

extern s16 D_801021E2;
extern s16 D_8010231A;
extern s16 D_80102334;
extern s32 D_80102350;
extern s16 D_8010235C;

extern s16 D_8010238E;
extern s16 D_801023AA;
extern u8 D_801023C5;
extern s32 D_80102408;
extern s32 D_80102410;
extern s32 D_80102448;
extern s32 D_80102450;
extern s16 D_80102462;
extern s16 D_801024DE;

extern s16 D_8010262E;
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
 * pre-slim-2026-10-01:memory/grind/func_80034708/evidence.md [s4]-[s5]). */
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
extern PadState D_80102788;
extern s32 D_801027B0[][5];
extern s32 D_801027B4;
extern s32 D_801027B8;
extern s32 D_801027BC[][5];
extern s32 D_801027C0;
extern s32 D_801027D4;
extern u8 D_80104E88;
extern s32 MotDataBaseAddress;
extern s16 D_80106A7A;
extern u8 D_80106A80;
extern u8 D_80106A82;
extern u8 D_801077AF;
extern u8 D_801077B0;
extern u8 D_801077BA;

extern LeafPos D_80107850[6];

/* Functions */
extern void func_8001B748(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void func_8003D52C(u8 *, s32, ...);
extern void func_80021A98(s32, u8 *, s32);
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
extern void Exec(s32 *, s32, s32 *);
extern s32 format(s32 *);
extern s32 sprintf(char *, char *, ...);
extern void ResetGraph(s32);
extern void SetDispMask(s32);
extern void DrawSync(s32);
extern void CdInit(void);
extern void CdFlush(void);
extern void CdSetDebug(s32);
extern void CdReadyCallback(s32);
extern void CdControlF(s32, s32);
extern s32 CdRead(s32, s32, s32);
extern s32 CdReadSync(s32, s32);
extern void SsSetSerialVol(s32, s32, s32);
extern s32 _comb_control(s32, s32, s32);
extern s32 func_80038C70(void);
extern void func_8003E2D8(s32, s32, s32, s32);
extern void func_80036940(void);

#endif /* CODE6CAC_H */
