#ifndef BB2_H
#define BB2_H

/* Declarations of the game's objects and functions for the translation units in src/main/
 * (SLUS_006.63's game code); their types are in game.h. */

#include "game.h"

extern FileRecord D_80106A50;
extern s16 D_800A3710;
extern GpuDb g_gpu_db[2];
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void gpu_InitDisplay(void);
extern MoveChannel D_800EF848[];
extern u16 D_80099C34[][7];

/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *D_800A378C;

extern DR_MOVE light_effect_col[31][2];
extern DR_MOVE D_800A4340[19][2];
extern DR_MOVE D_800A9830[2][10];

/* 0x800A3220: the VRAM rectangle func_8003D2C4 passes to LoadImage with the
 * image at D_80090178 (x 0x3F0, y 0x1DC, 16 x 36); defined in main/2B344.c. */
extern RECT D_800A3220;

extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_stage_id;
extern s16 g_stage_variant;
extern Unk800F1198Record D_800F1198[];
extern Unk800A9CF8Header D_800A9CF8;
extern Unk800F0EC8Record D_800F0EC8[][10];
extern Unk800F0E38Record D_800F0E38[12];
extern MenuOption D_8009BC0C[8];

/* 0x8009BCC4: the three scroll pages' origins, s16 (x, y) pairs for pages 4..6
 * (0x8009BCC4..0x8009BCCF). Object model evidence from the original binary:
 * func_800720FC reads (x, y) of entry `mode` through ONE index register
 * (`sll $a2,mode,2`, then `lhu %lo(D_8009BCC4)($at)` and `lhu %lo(D_8009BCC6)($at)`
 * with the same $a2), and its scroll loop reads page p's pair at
 * 0x8009BCB4 + p * 4 for p = 4..6 (asm lines 220-233), i.e. entries 0..2 of
 * this table. Replaces the splat
 * per-word symbols D_8009BCC4 / D_8009BCC6 (per-word splat symbol -> aggregate
 * merge family, owner ruling). */
extern s16 D_8009BCC4[3][2];

/* 0x8009BCD0: the current scroll offset, one s16 per axis. func_800720FC walks
 * it with a 2-byte step bounded by &D_8009BCD0 + 4 (asm lines 220-288). Replaces
 * D_8009BCD0 / D_8009BCD2 (same merge family). */
extern s16 D_8009BCD0[2];

extern Unk8009BCF8Record D_8009BCF8[2][10];

/* 0x8009BD20: two 2-byte records, {0x01, 0x02} and {0x03, 0x04}
 * (asm/data/7D920.data.s:23805-23817, splat's D_8009BD20 / D_8009BD21 dlabels).
 * Object model evidence from the original binary: func_800747D8 takes the table
 * base into a register (`lui $a2,%hi(D_8009BD20); addiu $a2,$a2,%lo(D_8009BD20)`
 * at 0x800749B4) and adds the column; func_80074488 (0x800746A8-0x800746D4) and
 * func_800770B8 (0x80077338-0x80077344) index it with the row shifted left 1 (a
 * 2-byte stride); func_80074488 reads both columns (column 0 at %lo(D_8009BD20),
 * column 1 at %lo(D_8009BD21)), func_800770B8 only column 1 at
 * %lo(D_8009BD21). */
extern u8 D_8009BD20[2][2];

extern u8 *D_800A36A0;
#define SELWORK ((SelWork *)D_800A36A0)

/* 0x800FF558: one PsyQ MATRIX (m[3][3], pad, t[3]; 0x20 bytes). Object model
 * evidence from the original binary: func_8004A940 forms the single base
 * %hi/%lo(0x800FF558) and reads t[0..2] at +0x14/+0x18/+0x1C, loads words
 * +0x0..+0x10 into GTE control registers 0..4 (the SetRotMatrix sequence) and
 * passes the base as the MATRIX * of gte_MulMatrix0ClearTrans; func_80048BA4
 * passes +0x14 as ApplyMatrix's VECTOR * and writes the nine s16 at +0..+0x10.
 * Replaces the twelve per-word splat scalars 0x800FF558..0x800FF574 (per-word
 * splat symbol -> aggregate merge family). */
extern MATRIX D_800FF558;

extern Unk8009BC94Record D_8009BC94[][6];
extern Unk8009B398Record D_8009B398[4];
extern Unk8009B400Record D_8009B400[10];
extern Unk8009B400Record D_8009B458[3][2];
extern Unk8009B450Record D_8009B450[2];

/* 0x8009B388: two adjacent 8-byte sprite cells (Unk8009B400Record),
 * 0x8009B388..0x8009B397, the cell table func_8005D554 hands func_80073728 for
 * the row's header record 2 (cell 0) and records 3/4 (cell 1); each of those
 * D_8009B2E0 header records has cell count 1. No other function or asm file
 * references either label. One object: spelled &D_8009B388[0] / [1],
 * func_8005D554 keeps the base in $s7 as the original does
 * (asm/funcs/func_8005D554.s); two separate symbols do not.
 * Replaces the splat per-cell scalars D_8009B388 / D_8009B390 in C. */
extern Unk8009B400Record D_8009B388[2];

/* 0x8009B5F0: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B5F0..0x8009B60F. Object model evidence from the original binary:
 * asm/funcs/func_8005F1C8.s forms ONE
 * stride `sll $s0,$s0,4` (row counter * 16) and adds it to both
 * %lo(D_8009B5F0) (column 0: 0x8009B5F0 / 0x8009B600) and %lo(D_8009B5F8)
 * (column 1: 0x8009B5F8 / 0x8009B608) -- rows of two 8-byte records. Data:
 * all four records share the {s16, s16, u8 x4} shape (u8 [3] = 0x01 in each);
 * D_8009B610 (a 12-byte sheet header) follows. Replaces the splat per-word
 * scalars D_8009B5F0 / D_8009B5F8 in C (per-word splat symbol -> aggregate
 * merge family, owner ruling). */
extern Unk8009B400Record D_8009B5F0[2][2];

/* 0x8009B490: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B490..0x8009B4AF. Object model evidence from the original binary:
 * asm/funcs/func_8005E54C.s forms ONE stride `sll $s0,$s0,4` (row counter * 16)
 * and adds it to both %lo(D_8009B490) (column 0) and %lo(D_8009B498) (column 1),
 * the same shape as D_8009B5F0 above. Data: all four records share the
 * {s16, s16, u8 x4} shape; D_8009B4B0 (a 12-byte sheet header) follows. Replaces
 * the splat per-word labels D_8009B490 / D_8009B498 in C (per-word splat symbol
 * -> aggregate merge family, owner ruling). */
extern Unk8009B400Record D_8009B490[2][2];

extern Unk8009B0E0Record D_8009B0E0[9];
extern Unk8009B0E0Record D_8009B14C;
extern Unk8009B0E0Record D_8009B158;

/* 0x8009B164: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B164..0x8009B183. Object model evidence from the original binary:
 * asm/funcs/func_8005C8A8.s forms ONE base %hi/%lo(D_8009B164) in $s1 and passes
 * `addiu $v1,$s1,0x10` (row 1) as the second draw's cell table, and stores the
 * x of column 1 of each row through the splat labels D_8009B16C (row 0) and
 * D_8009B17C (row 1). Replaces the splat per-word labels D_8009B164 /
 * D_8009B16C / D_8009B17C in C (per-word splat symbol -> aggregate merge
 * family). */
extern Unk8009B400Record D_8009B164[2][2];

/* 0x8009B184: 2 x 8-byte sprite records (Unk8009B400Record), 0x8009B184..
 * 0x8009B193. Object model evidence from the original binary:
 * asm/funcs/func_8005C8A8.s forms ONE base %hi/%lo(D_8009B184) in $a2, passes it
 * as a cell table (the sheet header's count is 2) and stores record 0's x
 * through it (`sh $v0,0x0($a2)`); record 1's x is the splat label D_8009B18C.
 * Replaces the splat per-word labels D_8009B184 / D_8009B18C in C (per-word
 * splat symbol -> aggregate merge family). */
extern Unk8009B400Record D_8009B184[2];

/* 0x800A328C: one 8-byte sprite cell (Unk8009B400Record: x, y = 0, u, v = 0,
 * w 0x14, h 0x0E), the cell table func_80060414 hands func_8007352C in its
 * descriptor (func_8007352C reads it as SprtEntA {s16 x, y; u8 u, v, w, h}).
 * Defined in src/main/3AB48.c. */
extern Unk8009B400Record D_800A328C;

extern Unk8009B2BCRecord D_8009B2BC[3];
extern Unk800EFAE8Ctrl D_800EFAE8;
extern Unk800F0C10Record D_800F0C10[4][3];

/* Per-effect-mode state, modes 0..0x11 (func_80065800 and its per-mode init and step
 * functions). Object model evidence (the original binary): asm/funcs/func_80065800.s
 * :39-45 addresses the position records as base + mode*12 (sll/addu/sll, then
 * %lo(D_800F0CA0)($at)) and :147-152 the timers as base + mode*2; modes 8/9 read
 * element mode-2, base - 4 + mode*2 (:351-356). The init functions
 * func_80064E90..func_800652AC fill the records and initialise the timers; the step
 * functions func_800652F4..func_800657B0 advance the timers. Replaces the splat
 * per-word scalars D_800F0BA8..D_800F0BCA and D_800F0CA0..D_800F0D74. */
extern s16 D_800F0BA8[18];

extern Unk800F0C10Record D_800F0CA0[18];
extern StageFuncEntry g_stage_init_tbl[];

/* 0x8008D090: the per-mode main-loop handlers, indexed by D_800A3834 (defined in src/main/d_7D870.c). */
extern void (*g_module_func_tbl[])(void);

extern u8 cpu_practice_honmokuroku_data_tbl[][4];

/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF: 0x8008D118..0x8008D517
 * (0x400 bytes; the first 8 are the last words of .text, src/main/d_7D870.c,
 * the rest asm/data/7D920.data.s dlabel D_8008D120). */
extern u8 g_sqrt_table_u8[0x400];

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

/* The 16 slots func_800645B0 spawns and func_800646E8 draws (bit i of D_800A3444
 * live): slot i's position.  Both functions address it as base + i*12
 * (asm/funcs/func_800645B0.s:31-55, func_800646E8.s:93-109).  Replaces the splat
 * per-word scalars D_800F0D78 / D_800F0D7C / D_800F0D80 (the last was misnamed
 * "videoDec": it is slot 0's z). */
extern Vec3i32 D_800F0D78[16];

extern Unk800EED10Entry D_800EED10[10];

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
extern Unk800EFB78Entry D_800EFB78[24];
extern Unk80101EC8Record D_80101EC8[];
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
extern Tbl8008E194 D_8008E194[];

/* 8 initialized byte pairs (asm/data/7D920.data.s, 0x8009BD58: {0,0} {1,0}
 * {2,0} {3,0} {4,1} {5,1} {6,1} {7,0}); func_80077904 returns [n][0] and
 * caches [n][1] in D_800A35E0, n = D_8009BD38.unk0 (base + n*2,
 * asm/funcs/func_80077904.s).  Replaces the splat per-byte scalar D_8009BD59. */
extern u8 D_8009BD58[8][2];

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
extern StatusFlagRec D_80099D88[];
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
 * src/main/26940.c). */
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
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
extern Unk80101DF0Record g_cam_bone_data2;
extern Unk80101DF0Record D_800EF070;
extern Unk800F62E0Rec D_800F62E0[8];
extern AnimRotFunc g_anim_func_table[6];

/* The CD file table at 0x8008EC34: one 8-byte record per disc file, indexed by the
 * file numbers func_80036EA8 forms. `loc` is sought (cdrom_StartRead copies the record
 * into D_80101E58.rec.pair, whose position is sought; cdrom_LoadExec seeks to it
 * directly); `size` is cdrom_StartRead's sector count, cdrom_GetFileSize's result and
 * cdrom_StartAudio's end position. */
extern CdFileEntry g_cd_file_table[159]; /* 0x8008EC34..0x8008F12B */

extern CdState D_80101E58;
extern s32 D_80102760;
extern s32 D_80102764;
extern s32 D_80102768;
extern s32 D_80102770;
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
extern s32 func_80038C70(void);
extern void func_8003E2D8(s32, s32, s32, s32);
extern void func_80036940(void);

/* Declared by more than one translation unit: data, then functions, by name. */
extern u8 D_8008E908[][5];
extern u8 D_8008E914[][8];
extern s32 D_8008EA00[][4];
extern u8 D_8008EC24[][5];
extern s32 D_8009BC04;
extern Unk8009BD24Record D_8009BD24[2][5];
extern s32 D_800A3244;
extern s32 D_800A326C;
extern s32 D_800A32BC;
extern s32 D_800A32C8;
extern u16 D_800A37C4;
extern s32 D_800A37D4;
extern s32 D_800A3808;
extern s16 D_800A3840;
extern u8 D_800A384C;
extern s16 D_800A3854;
extern u8 *D_800A3894;
extern s32 D_800A38D0;
extern u16 D_800A38D6;
extern s32 D_800A38FC;
extern s32 D_800A3908;
extern u8 D_800A3916;
extern u8 D_800A9D10;
extern s16 D_800F0BCC[];
extern s16 D_800F0BEC[];
extern s32 D_800F10EC;
extern s32 D_800F10F0;
extern s32 D_800F1138;
extern s32 D_800F1144;
extern s32 D_800F1148;
extern s32 D_800F1178;
extern s32 D_800F117C;
extern u8 D_800F1B18[];
extern s16 D_800F6650;
extern s16 D_800F6656;
extern u8 D_801027A0;
extern u8 D_801027D8;
extern s32 D_80102C00;
extern u8 g_file_data_buf[];
extern s32 g_player_char_ids[];
extern s32 g_player_ptrs[];

extern void eff_Init(void);
extern void file_LoadSoundData(void);
extern void file_ResetDmaFlag(void);
extern void func_800174F4(void);
extern void func_8001979C(s32, u32 *);
extern s32 func_8001DB58(void);
extern void func_8001F860(s16 *, s32);
extern void func_80020CDC(void);
extern void func_80020D38(void);
extern void func_800288C8(void);
extern void func_8002AB08(s32);
extern void func_8002EBDC(s16 *, s16 *, s32 *, s32, s32);
extern void func_800321E8(void);
extern void func_800324D0(Unk80101EC8Record *);
extern void func_800335D8(void);
extern void func_80033BC0(void);
extern void func_800344B4(void);
extern void func_8003B5A4(void);
extern void func_8003E22C(void);
extern void func_8003F218(s32);
extern s32 func_8003F268(void);
extern void func_8003FFC4(s32 *);
extern void func_80040304(s32, s32);
extern void func_80041BF4(s32, s32, s32);
extern void func_80045230(s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, s32);
extern s32 *func_8004574C(s32);
extern void func_80046DA8(s32);
extern s16 *func_8004BCC0(s32, s16 *, s16 *, s32);
extern void func_8005B5AC(void);
extern void func_8005B72C(void);
extern void func_8005B868(void);
extern void func_8005B9C4(void);
extern void func_8005BF3C(void);
extern s32 *func_80077D00(void);
extern void game_Cleanup(void);
extern void gpu_SetDrawEnvBg(s32, s32, s32, s32);
extern s32 gte_SumSquares3(s32, s32, s32);
extern s32 math_FovToScreenDist(s32);
extern void memcard_Quit(void);
extern void player_Destroy(s32);
extern void player_SetCharId(s32, s32);
extern s32 rng_Next(void);
extern void seq_Reset(void);
extern void seq_Start(s32, s32);
extern void snd_Quit(void);
extern void snd_VabFakeOpen8And4(s32);
extern void snd_VabFakeOpen9(s32);
extern s32 stage_GetId(void);
extern void stage_InitCollision(void);
extern void sys_Init(void);
extern void sys_Panic(void);

/* Game-side declarations of Sony library functions whose spelling differs from the library's
 * definition (src/main/psxsdk/). Reconciling them is Phase 2 work; until then they stay here,
 * where the game code has always seen them. */
extern void ResetGraph(s32);
extern void CdReadyCallback(s32);

#endif /* BB2_H */
