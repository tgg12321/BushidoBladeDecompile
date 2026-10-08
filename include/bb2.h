#ifndef BB2_H
#define BB2_H

/* Declarations of the game's objects and functions for the translation units in
 * src/main/ (SLUS_006.63's game code); their types are in game.h. */

#include "game.h"

extern FileRecord D_80106A50;
extern s16 D_800A3710;
extern GpuDb g_gpu_db[2];
/* The current frame's ordering table (OT entry n is g_gpu_ot_ptr + n). */
extern u32 *g_gpu_ot_ptr;
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void func_80016888(void);
extern MoveChannel D_800EF848[];
extern u16 D_80099C34[][7];

/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *D_800A378C;

extern DR_MOVE D_800A3D70[31][2];
extern DR_MOVE D_800A4340[19][2];
extern DR_MOVE D_800A9830[2][10];

/* The VRAM rectangle func_8003D2C4 loads the image at D_80090178 into
 * (x 0x3F0, y 0x1DC, 16 x 36). */
extern RECT D_800A3220;

extern u8 D_800A3768;
extern u8 D_800A36A8;
extern s16 D_800F665C;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 D_80099478;
extern s16 D_8009947A;
extern Unk800F1198Record D_800F1198[];
extern Unk800A9CF8Header D_800A9CF8;
extern Unk800F0EC8Record D_800F0EC8[][10];
extern Unk800F0E38Record D_800F0E38[12];
extern MenuOption D_8009BC0C[8];

/* The three scroll pages' origins: s16 (x, y) pairs for pages 4..6
 * (func_800720FC). */
extern s16 D_8009BCC4[3][2];

/* The current scroll offset, one s16 per axis (func_800720FC). */
extern s16 D_8009BCD0[2];

extern Unk8009BCF8Record D_8009BCF8[2][10];

/* Two 2-byte records, {0x01, 0x02} and {0x03, 0x04}, indexed [row][col] by
 * func_800747D8, func_80074488 and func_800770B8. */
extern u8 D_8009BD20[2][2];

extern u8 *D_800A36A0;
#define SELWORK ((SelWork *)D_800A36A0)

/* One PsyQ MATRIX: func_8004A940 loads it as the GTE rotation, reads t[] and
 * passes it to gte_MulMatrix0ClearTrans; func_80048BA4 writes m[][] and passes
 * t to ApplyMatrix. */
extern MATRIX D_800FF558;

extern Unk8009BC94Record D_8009BC94[][6];
/* Table of 4 sprite-sheet headers, 12 bytes each (func_8005E098,
 * func_8005E54C). */
extern Unk8009B0E0Record D_8009B398[4];
extern Unk8009B400Record D_8009B400[10];
extern Unk8009B400Record D_8009B458[3][2];
extern Unk8009B450Record D_8009B450[2];

/* Two 8-byte sprite cells, the cell table func_8005D554 hands func_80073728
 * for the row's header record 2 (cell 0) and records 3/4 (cell 1). */
extern Unk8009B400Record D_8009B388[2];

/* 2 x 2 table of 8-byte sprite records (func_8005F1C8). */
extern Unk8009B400Record D_8009B5F0[2][2];

/* 2 x 2 table of 8-byte sprite records (func_8005E54C). */
extern Unk8009B400Record D_8009B490[2][2];

extern Unk8009B0E0Record D_8009B0E0[9];
extern Unk8009B0E0Record D_8009B14C;
extern Unk8009B0E0Record D_8009B158;

/* 2 x 2 table of 8-byte sprite records; func_8005C8A8 passes row 1 as the
 * second draw's cell table and stores column 1's x in each row. */
extern Unk8009B400Record D_8009B164[2][2];

/* Two 8-byte sprite records, a cell table func_8005C8A8 passes (the sheet
 * header's count is 2) after storing each record's x. */
extern Unk8009B400Record D_8009B184[2];

/* One 8-byte sprite cell (x, y = 0, u, v = 0, w 0x14, h 0x0E), the cell
 * table func_80060414 hands func_8007352C in its descriptor. */
extern Unk8009B400Record D_800A328C;

extern Unk8009B2BCRecord D_8009B2BC[3];
extern Unk800EFAE8Ctrl D_800EFAE8;
extern Unk800F0C10Record D_800F0C10[4][3];

/* Per-effect-mode state, modes 0..0x11 (func_80065800): the timers here and
 * the position records in D_800F0CA0. The init functions func_80064E90..
 * func_800652AC fill them; the step functions func_800652F4..func_800657B0
 * advance the timers. */
extern s16 D_800F0BA8[18];

extern Unk800F0C10Record D_800F0CA0[18];
extern Unk800948BCEntry D_800948BC[];

/* The per-mode main-loop handlers, indexed by D_800A3834. */
extern void (*D_8008D090[])(void);

extern u8 D_8008E778[][4];

/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF (the first 8 bytes
 * are the last words of .text). */
extern u8 g_sqrt_table_u8[0x400];

extern Unk8008DCCCEntry D_8008DCCC[18];

/* 3 x 3 s16 angle offsets, [row][col] from the pad bits (func_800233AC,
 * func_80023648). */
extern s16 D_8008EB40[3][3];

/* Judge: the sine table, one full turn in 0x1000 steps, 1.0 = 0x1000
 * (cos(a) = Judge[(a + 0x400) & 0xFFF]). */
extern s16 Judge[0x1000];

/* The 16 slots func_800645B0 spawns and func_800646E8 draws (bit i of
 * D_800A3444 live): slot i's position. */
extern Vec3i32 D_800F0D78[16];

extern Unk800EED10Entry D_800EED10[10];

/* The two characters' hit records (0x800F5F68..0x800F62D7): func_800206B0 fills
 * D_800F5F68[ch] from the template D_8008D59C, offsets and limits scaled. */
extern BoneHitRec D_800F5F68[2][22];

extern BoneHitRec D_8008D59C[22];

/* The 6 slots func_8006288C spawns and func_8006295C draws (bit i of D_800A3460
 * live): slot i's position and its RotMatrixZYX angles. */
extern Vec3i32 D_800F0FB8[6];

extern SVec4i16 D_800F10A0[6];
extern s16 D_800F0C04[6];
extern Unk800EFB78Entry D_800EFB78[24];
extern Unk80101EC8Record D_80101EC8[];
extern s32 D_800100A4;
extern const char D_800109C8[];
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
extern u8 D_8008DA08[0x48];
extern s16 D_8008DA50[]; /* [unk_0A] (func_80023F08) */
extern s16 D_8008DA94[]; /* [unk_0A] (func_80023F08) */
extern s16 D_8008DAD8[]; /* [unk_0A] (func_80023F08) */
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_48 model id (func_80020E74) */
extern u16 D_8008DB1C[27][8];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_84 */
extern u8 D_8008DD5C[27][8];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_1C */
extern u16 D_8008DE34[27][6];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_1E / unk_20 */
extern u16 D_8008DF78[27][6];
extern Tbl8008E194 D_8008E194[];

/* 8 byte pairs ({0,0} {1,0} {2,0} {3,0} {4,1} {5,1} {6,1} {7,0});
 * func_80077904 returns [n][0] and caches [n][1] in D_800A35E0,
 * n = D_8009BD24.unk14_0. */
extern u8 D_8009BD58[8][2];

extern Obj80106A78 D_80106A78[12];
/* [unk_0A][i] -> Unk80101EC8Record.unk_332[i] (func_8003047C) */
extern s8 D_8008E338[27][5];
extern u16 D_8008E3C0[28]; /* [unk_0A] -> Unk80101EC8Record.unk_274 */
/* [unk_0A][i] -> Unk80101EC8Record.unk_276[i] */
extern u16 D_8008E3F8[27][4];
/* [unk_0A][i] -> Unk80101EC8Record.unk_27E[i] */
extern u16 D_8008E4D0[27][4];
extern u8 D_8008E5A8[];
extern u8 D_8008E5CC[][8]; /* [unk_0A][unk_0E] of D_80101EC8 */
extern u8 D_8008E6A4[][6]; /* [unk_0A][unk_0E] of D_80101EC8 */
extern u8 D_8008E748;
extern u8 D_8008E75C;
extern Unk8008EA44Entry D_8008EA44[5];

/* per-stage s16 table, 34 entries */
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
/* [unk_0E][flag] -> Unk80101EC8Record.unk_12 */
extern u8 D_8008EB28[8][2];
extern u8 D_8008EB38[8]; /* [unk_0E] -> Unk80101EC8Record.unk_12 */
extern Tbl8008EB54Entry D_8008EB54[6];
extern u8 D_8008EB6C[6];
extern u8 D_8008EB80[];
/* per-limb angle (func_80031890: [j], j < 22) */
extern u16 D_8008EBA0[22];
extern s32 D_8008EBCC[];
extern s32 D_8008EBE0[];
extern u8 D_8008EBF4[6];
extern Unk8008EA44Entry D_8008EBFC[6];
extern u8 D_8008EC30[4];
extern s16 D_8008F12C;

/* Twelve rows func_80037110 reads by index: unk_0, the group-5 file
 * func_80036EA8 resolves; unk_1, cdrom_StartAudio's second argument; unk_4, an
 * offset added to the file's start sector (-1: none). */
typedef struct {
    u8 unk_0;
    u8 unk_1;
    u16 unk_2;
    s32 unk_4;
} Unk8008F13CRow;

extern Unk8008F13CRow D_8008F13C[12];
extern u8 D_8008F19C[];
extern u8 D_8008F1A8[];
extern u8 D_8008F204[];
extern u8 *D_800900EC[];
extern u8 D_8009016C;
extern u32 D_80090178;
extern u32 D_800905F8;
extern s32 D_80090600;
extern s32 D_80090604;
extern s16 D_80090608;

/* Two per-stage tables, indexed by the stage id func_80046EA0 passes to
 * func_8003DA8C: 38 s32, then s16 pairs ([0] tested, [1] passed to
 * func_8003DBE4; the last pair is zero). */
extern s32 D_8009060C[38];

extern s16 D_800906A4[39][2];
extern u16 D_80094C68[];
extern Unk80099D88Rec D_80099D88[];
extern Unk8009A8C8Entry D_8009A8C8[][8];
extern u8 D_8009A9B4[][2]; /* byte pairs (func_80055138) */
/* [D_8008D9EC flag] -> 3 bytes (func_80041BF4 args), stride 4 */
extern u8 D_800A3100[][4];
extern s16 D_800A310C[4];
extern s32 D_800A3134;
extern s32 D_800A3140;
extern u8 D_800A31DA;
extern u8 D_800A3670;
extern u8 D_800A3671;
/* Three angles: func_80044504 passes them to math_RotMatrixZXY; func_8003EDC0
 * zeroes them. */
extern u16 D_800A3678[3];
extern u8 D_800A3680;
extern s32 g_comb_recv_buf_plus_0x4;
extern u8 D_800A3690;
extern s32 g_comb_send_buf_plus_0x4;
extern s16 D_800A36A4;
extern s32 D_800A36AC;
/* the Rec44 view record func_8001E404 / func_8001E6E4 last placed (read by
 * func_800325E0 as the listener) */
extern Rec44 *g_listener_cam;
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
/* per-player byte, [unk_04] (func_8003047C) */
extern u8 D_800A36F2[2];
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

/* Four s16 counters / latches, one per D_8008E914 row func_800335D8 walks
   (type 1 latches 1; types 2..6 count up until an rng_Next() draw clears
   them); func_80033510 clears it from the last element down. */
extern s16 D_800A3750[4];

extern u8 D_800A3758;

/* The 8-byte CD status/result buffer CdSync / CdReady fill: result[0] is the
 * status byte; func_80036140 hands &result[3] / &result[5] (the reported
 * position) to CdPosToInt. Tentative definition in src/main/26940.c (Q62
 * global COMMON model). */
extern u8 g_cd_result[8];

extern u8 D_800A3769;
extern u8 D_800A376A;
extern u8 D_800A376B;
extern u8 D_800A376C;
extern s16 D_800A376E;
extern s32 D_800A3778;
extern u8 D_800A377B;
/* round-result table: 0/1/2 per round, indexed by round */
extern u8 D_800A377C[];

/* Two buffer addresses selected by frame parity: func_80016D78 sets 0x801D8800
 * / 0x801EBC00, func_80016E60 and main read [D_800A36AC & 1]. */
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
/* the draw queue cursor (into g_draw_queue) */
extern void **g_draw_queue_cursor;
extern s16 D_800A3824;
extern u8 D_800A382D;
extern s16 D_800A382E;
extern s32 D_800A3830;
extern s16 D_800A3834;
extern u8 D_800A3836;
extern u8 *D_800A3844;
extern s32 D_800A3858;
extern u8 *D_800A385C;
extern Tbl800A3860Entry *D_800A3860[];
extern u8 D_800A3874;
extern s16 D_800A3876;
extern u8 *D_800A3878;
extern s32 D_800A387C;
extern u8 D_800A3880;
/* per player: preset motion frames (func_80020D70, func_80023F08) */
extern MotionFrame *D_800A3888[2];
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
/* per player: character of the D_800A3888 motion set (0xFF = none) */
extern u8 D_800A38C0[2];
/* per slot: model id in the D_800A3860[i] buffer (func_80020E74); [1] = 0xFFFF:
 * func_8001DB9C passed 0x80190800 (D_800A3860[1]'s buffer) to func_800450BC;
 * func_80020D38 calls func_80045188 for it */
extern u16 D_800A38C4[2];
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

/* Six per-slot counters, 0 = free: func_80033550 claims a slot with 1,
   func_800335D8 advances and frees them, func_80033510 zeroes all six.
   D_800A391E is not an element: it is its own countdown byte, and
   func_800335D8 uses only its address, as the loop bound. */
extern u8 D_800A3918[6];

extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern u8 D_800A3928;
extern u8 D_800A3929;
extern Unk800A3D40Rec D_800A3D40[];

/* Three word pairs: the uncalled func_8003F5A8 stores pair i,
 * func_8003F568 zeroes all six, func_8003F5CC passes pair i to
 * func_80017F98(a, b, i), an empty function; nothing else reads them. */
extern s32 D_800A93B0[3];

extern s32 D_800A93BC[3];
extern Rec44 D_800F5328;
extern Rec44 D_800F6608;

/* Per-round snapshot: func_800340A0 stores both players' counters
 * (D_800A3898 / D_800A3899) for round D_800A3874; func_80034200 packs the
 * pairs into D_800A3784, func_8003C42C sums them. */
extern u8 D_800F65F8[8][2];

extern Unk800F68E0Rec D_800F68E0[0xB4];
/* The two InitPAD receive buffers, 0x24 bytes each (sys_Init); func_80019568
 * copies the first two words of each. */
extern s32 g_pad_buf[2][9];
extern s32 D_800FF5C8;
extern s32 D_800FF5CC;
extern s32 D_800FF5D0;
extern s16 D_800FF5D8;
extern s16 D_800FF5DA;
extern s16 D_800FF5DC;
extern s32 D_800FF5E0;
extern Unk80101BF0Rec D_80101BF0[0x20];
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
extern Unk80101DF0Record D_800EEDF0;
extern Unk80101DF0Record D_800EF070;
extern Unk800F62E0Rec D_800F62E0[8];
extern RotMatrixFunc D_800F66A0[6];

/* The CD file table at 0x8008EC34: one 8-byte record per disc file, indexed by
 * the file numbers func_80036EA8 forms. `loc` is sought (cdrom_StartRead copies
 * the record into D_80101E58.rec.pair, whose position is sought; cdrom_LoadExec
 * seeks to it directly); `size` is cdrom_StartRead's sector count,
 * cdrom_GetFileSize's result and cdrom_StartAudio's end position. */
extern CdFileEntry g_cd_file_table[159]; /* 0x8008EC34..0x8008F12B */

extern CdState D_80101E58;
extern Unk801027B0Pack D_80102760; /* the common motion pack */
extern Unk80102778Rec D_80102778;
extern PadState g_pad_state;
extern Unk801027B0Pack D_801027B0[]; /* per load slot */
/* [i] points to block i's slots, the words after its header word (func_80044010
 * records it and D_80103658[i] holds the slot count).  Each slot is a
 * word holding a block-relative offset that func_80044010 turns into an address
 * by adding the block's base, unless header bit 15 marks the block relocated;
 * func_80044098 subtracts the base again (and clears bit 15), and func_80044100
 * moves [i] by a1 / 4 words and adds a1 to each slot.  func_800432A0 /
 * func_800433E4 store a slot to scratchpad word 0, the u16 * cursor
 * func_80043454 reads, and func_8003FA24 reads u16 data through one. */
extern s32 *D_80103608[];
extern Unk80104E88Rec D_80104E88[];
extern s32 MotDataBaseAddress;

/* func_800338CC's list: the set bit numbers of D_80106A50.unk_00 & mask,
 * shuffled, plus up to three appended values (D_800A391F entries; func_80033BC0
 * reads entry D_800A3783, then advances it). */
extern u8 D_801077B0[24];

extern Unk80107850Rec D_80107850[6];
extern void func_8001B748(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void func_8003D52C(u8 *, s32, ...);
extern void func_80021A98(s32, MoveScript *, s32);
extern void func_80022580(s32, s32, s32, s32, s32);
extern s32 func_80036EA8(s32, s32);
extern void func_8003A728(PadState *);
extern void func_8003AE5C(u8 *);
extern void func_8003DE14(RECT *, s32);
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
extern u8 D_8009BA60[];
extern s32 D_8009BC04;
extern Unk8009BD24Block D_8009BD24;
extern s32 D_800A3244;
extern s32 D_800A326C;
extern s32 D_800A32BC;
extern u32 D_800A32C8[2];
extern Unk80101DF0Record *D_800A370C;
extern Unk80101DF0Record *D_800A3708;
extern s32 g_memcard_poll_count;
extern void func_800520B8(s32, s32, s32);
extern u8 D_800A37A8[];
extern u16 D_800A37C4;
extern POLY_FT4 *D_800A37D4;
extern u32 *D_800A3808;
extern s16 D_800A3840;
extern u8 D_800A384C;
extern s16 D_800A3854;
extern u8 *D_800A3894;
extern u32 *g_prim_buf_cursor; /* the draw-chunk builders' word cursor */
extern s32 D_800A38D0;
extern u16 D_800A38D6;
extern s32 D_800A38FC;
extern s32 D_800A3908;
extern u8 D_800A3916;
extern u8 D_800A9D10;
extern s16 D_800F0BCC[];
extern s16 D_800F0BEC[];
extern s32 D_800F10D0[];
extern s32 D_800F10EC;
extern s32 D_800F10F0;
extern s32 D_800F1138;
/* A 16-byte vector: thirteen functions store x / y / z; func_80061064 passes
 * it to func_80041E10, which copies all 16 bytes to D_800A9B28. */
extern VECTOR D_800F1140;
extern s32 D_800F1178;
extern s32 D_800F117C;
extern Unk800F1B18Rec D_800F1B18[];
extern u8 D_800F33D8[];
extern s16 D_800F6650;
extern s16 D_800F6656;
extern u8 D_801027A0;
extern u8 D_801027D8;
/* the draw queue: record pointers (0x80102C00..0x801035FF) */
extern void *g_draw_queue[640];
extern s32 D_8009BA7C[];
extern Func80017A44Output g_file_data_buf[8];
extern s32 D_80094B88[];
extern Unk80045878Obj *D_800A9A10[];

extern s32 bits_DepositMask3F83F8(s32);
extern s32 bits_ExtractMask3F83F8(s32);
extern s16 *camera_CalcAngles(void);
extern void func_80047210(void);
extern u32 cdrom_GetFileSize(s32);
extern void cdrom_Init(void);
extern void cdrom_ReadyCallback(u8, u8 *);
extern void cdrom_SetMix(s32, s32, s32, s32);
extern s32 cdrom_StartAudio(s32, s32);
extern s32 cdrom_StartRead(s32, s32);
extern s32 cdrom_StartReadAt(s32, s32, s32, s32);
extern void *func_800472B0(void);
extern void eff_Init(void);
extern u32 func_800167AC(void);
extern u32 func_800167BC(void);
extern u32 func_800167D4(void);
extern void snd_InitAndLoadCommonVab(void);
extern void eff_ClearInitFlag(void);
extern void func_800174F4(void);
extern s32 func_80017D84(Func80017A44Input *);
extern void func_8001924C(Unk80017FA0Rec *, s32);
extern void func_8001945C(void);
extern s32 func_80019488(void);
extern void func_800194C0(s32);
/* No prototype: defined void (s32), but func_80016E60 calls it with no
 * argument. */
extern void func_80019568();
extern void func_8001979C(s32, u32 *);
extern void func_800198D0(s32, s32, MotionFrame *, u16 *);
extern void func_8001B6F4(void);
extern void func_8001C444(void);
extern void func_8001CD68(Unk8001CD68Rec *);
extern void func_8001DA2C(void);
extern s32 func_8001DB58(void);
extern void func_8001DBE4(void);
extern void func_8001F1C4(
    Unk80101EC8Record *, Rec1C *, MotionFrame *, MotionFrame *);
extern void func_8001F860(Unk80101EC8Record *, s32);
extern void func_800203B4(Unk80101EC8Record *, s32, s16 *);
extern void func_800207C8(
    Unk80101EC8Record *, Unk80107850Rec *, Unk80107850Rec *, Unk80107850Rec *);
extern void func_80020CDC(void);
extern void func_80020D38(void);
extern void func_80020D70(void);
extern void *func_80021424(Unk80101EC8Record *, s32, s16 *);
extern void func_80021D10(s32, s32 *, s32);
extern s32 func_80022408(Vec3i32 *);
extern void func_80022568(Unk80101EC8Record *);
extern s32 func_800233AC(Unk80101EC8Record *, s32 *);
extern void func_80023F08(s32, PadState *);
extern void func_80026DA4(void);
extern void func_80027334(MotionFrame *);
extern s32 func_8002798C(Unk80101EC8Record *);
extern void func_800288C8(void);
extern s32 func_80029454(void);
extern void func_8002AB08(s32);
extern void func_8002C61C(void);
extern void func_8002EBDC(s16 *, s16 *, s32 *, s32, s32);
extern void func_8002EECC(void *, void *);
extern void func_8002F770(s16 *, s32, s32, s32);
extern s32 func_8002FDB0(Unk80101EC8Record *);
extern s32 func_800307D0(Unk80101EC8Record *);
extern void func_80030A2C(Unk80101EC8Record *, s32, Vec3i32 *);
extern s32 func_80030BA8(Unk80101EC8Record *);
extern void func_80030D7C(void);
extern void func_80031B24(void);
extern Unk80104E88Rec *func_80032064(Unk80101EC8Record *, s32);
extern void func_800321E8(void);
extern void func_800324D0(Unk80101EC8Record *);
extern void func_800325E0(s32, s32 *);
extern void func_80032854(s32, s32, s32 *, s16 *);
extern void func_8003339C(Unk80101EC8Record *);
extern void func_80033510(void);
extern void func_800335D8(void);
extern void func_800338CC(void);
extern void func_80033BC0(void);
extern s32 func_80033DF4(void);
extern void func_800340A0(void);
extern void func_800342A0(void);
extern void func_800344B4(void);
extern void func_80034F88(void);
extern void func_8003504C(void);
extern void func_80035280(void);
extern void func_80035F78(s16, s32, s32, s32, s32);
extern s32 func_80037110(s32);
extern void func_800371E8(s16);
extern void func_80037260(void);
extern void func_800372C0(void);
extern s32 func_80037AA4(void);
extern s32 func_80037B00(u8 *);
extern s32 func_8003880C(void);
extern s32 func_800388A8(void);
extern s32 func_80038988(void);
extern s32 *func_800392B8(void);
extern void func_80039320(void);
extern void func_80039680(Unk80101EC8Record *);
extern void func_800397A0(void);
extern void func_8003A41C(void);
extern void func_8003AA48(void);
extern void func_8003AA78(void);
extern void func_8003AAB0(void);
extern void func_8003B20C(s32);
extern void func_8003B5A4(void);
extern void func_8003D2C4(void);
extern void func_8003D2F4(void);
extern void func_8003D330(void);
extern void func_8003D774(s32, s32);
extern s16 *func_8003D7B4(s32);
extern void func_8003D91C(void);
extern void func_8003DA8C(s32, s32);
extern void func_8003E0E0(void);
extern void func_8003E120(void);
extern void func_8003E164(s32);
extern void func_8003E22C(void);
extern s32 func_8003E2A0(void);
extern u32 func_8003E2C8(void);
extern void func_8003E6A0(s32, s32);
extern void func_8003E6D8(s32);
extern void func_8003EDC0(u16 *, s32);
extern void func_8003F218(s32);
extern s32 func_8003F268(void);
extern void func_8003F3D4(s16 *);
extern void func_8003F62C(Unk80045878Obj *);
extern void func_8003F7F4(void);
extern void func_8003F824(Unk80045878Obj *, s32);
extern void func_8003FFC4(Unk80045878Obj *);
extern void func_8003FFE0(s32);
extern void func_800400B0(Unk80045878Obj *, s32);
extern void func_800400F8(Unk80045878Obj *);
extern void func_8004016C(s32);
extern void func_8004019C(Unk80045878Obj *, s32);
extern void func_80040304(s32, s32);
extern void func_800404A0(Unk80045878Node *, s32);
extern void func_800404D8(void);
extern Unk80045878Obj *func_80040510(s32, s32, s32);
extern void func_80040A78(Unk80045878Obj *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80041188(s32, u8 *, u8 *, s32, MATRIX *);
extern void func_80041398(s32);
extern void func_80041430(s32, s32);
extern void func_800414FC(s32, Unk80045878Node *, s32);
extern Unk80045878Obj *func_8004153C(s32);
extern void func_800417D0(Unk80101DF0Record *);
extern void func_800418D0(Unk80101DF0Record *);
extern void func_80041BF4(s32, s32, s32);
extern void func_80041E10(VECTOR *, s32);
extern void func_800420D0(void);
extern void func_800420E8(s32, s32);
extern void func_8004211C(void);
extern void func_800421A4(void);
extern void func_800421C8(s32);
extern void func_80042E90(void);
extern void func_800432A0(s16, s16, s16, s16, s32);
extern void func_80044010(s32 *, s16);
extern void func_80044098(s16);
/* No prototype: defined void (s32, s32), but func_800466C0 calls it as
 * func_80044100(8). */
extern void func_80044100();
extern s32 func_8004428C(s32 *, s16 *);
extern s32 func_80044378(s32, s32 *, s16 *);
extern void func_80044498(void);
extern void func_800444BC(void);
extern void func_800444E0(void);
extern s32 func_80044670(s16 *, s16, s32);
extern void func_8004473C(void);
extern void func_80044800(void);
extern void func_80044B30(s32, s32);
extern void func_80044C70(s32);
extern void func_80044DE4(s16 *, s16 *, s32, s32);
extern void func_80044F30(s32, s32);
extern void func_80044F80(s32, s32);
extern s32 func_80044FA0(s32, s32);
extern s32 func_80045080(s32);
extern void func_800451A0(void);
extern void func_800451D0(void);
extern void func_80045230(s32);
extern void func_800453E0(s32);
extern void func_80045510(s32, s32);
extern s32 *func_800455AC(s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, s32);
extern s32 *func_8004574C(s32);
extern s32 func_800457A0(s32);
extern void func_80045824(s32, s32, s32);
extern Unk80045878Obj *func_80045878(s32, s32, s32);
extern void func_80045A28(s32, s32);
extern void func_80045A50(s32);
extern void func_80045B68(s32, s32, s16 *, s32);
extern void func_80046020(void);
extern void func_8004659C(s32);
extern s32 func_8004678C(void);
extern void func_80046914(void);
extern s32 *func_800469C4(s32);
extern void func_80046A60(void);
extern void func_80046BF4(Vec3i32 *, SVECTOR *, s32);
extern void func_80046DA8(s32);
extern void func_80046E54(s32);
extern s32 func_80046E7C(void);
extern void func_80046EA0(s32);
extern void func_80046F24(void);
extern void func_800480C0(s32, s32, s16, s16, s16, s16);
extern s32 func_800486FC(void);
extern void func_80048A7C(s32, s32, s32, s32, s32, s32);
extern s32 func_80048AD0(s32);
extern void func_8004939C(void);
extern void func_80049718(s32, s32, s32 *, s16 *);
extern void func_80049A2C(s32, s32, s32);
extern s32 func_80049C24(s32, s32);
extern void func_80049E1C(void);
extern void calc_LightMatrix(Unk800F62E0Rec *);
extern s16 *func_8004BCC0(s32, s16 *, s16 *, s32);
extern void func_800523E0(MATRIX *, MATRIX *, s32, MATRIX *);
/* No prototype: defined with no arguments, but func_8003EDC0 / func_8003FA24
 * pass it one. */
extern void func_80052C10();
extern s32 func_8005344C(s32 *, s32 *, s32 *, s16 *, s32);
extern s32 func_80053584(s32 *, s32 *, s32 *, s16 *);
extern s32 func_80053614(s32 *, s32 *, s32 *, s16 *, s32);
extern void func_80054410(s32);
extern void func_8005441C(s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800548DC(void);
extern s32 func_80054F68(void);
extern void func_80054FDC(s32);
extern s16 *func_8005507C(void);
extern s32 *func_8005508C(void);
extern void func_8005509C(s32);
extern void func_800550E8(s32);
extern void func_80055138(s32, u16 *, u16 *);
extern void func_8005B5AC(void);
extern void func_8005B644(s32);
extern void func_8005B6AC(void);
extern void func_8005B72C(void);
extern void func_8005B868(void);
extern void func_8005B9C4(void);
extern s32 func_8005B9FC(s32);
extern s32 func_8005BE84(s32);
extern void func_8005BF3C(void);
extern void func_8005C614(void);
extern void func_8005C650(s32, s32, s32);
extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005D46C(s32, s32);
extern s32 func_8005D554(s32, s32);
extern s32 func_8005D814(Unk8001CD68Rec *, s32, s32, s32);
extern s32 func_8005E098(s32, s32, s32, s32);
extern s32 func_8005E51C(s32, s32, s32);
extern s32 func_8005E54C(u32, s32, s32);
extern s32 func_8005F1C8(Unk8001CD68Rec *, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern void func_8005FBC8(s32, u8 *);
extern s32 func_8005FC9C(s32, s32);
extern s32 func_800600C8(s32, s32, s32);
extern void func_800602AC(s32, s32 *);
extern void func_80060758(void);
extern s32 func_80060CB8(s32, s32);
extern void func_80060E04(s32);
extern void func_80061064(s16 *, s32 *);
extern void func_800618B4(s32 *, s16 *);
extern void func_80061A3C(s32 *, s16, s32, s32);
extern void func_80061FAC(s16 *, s32 *, MATRIX *);
extern void func_800620B8(s16 *, s32 *);
extern void func_80068ECC(s32);
extern s32 func_80068F70(s32, Unk8009BD24Block *);
extern void func_8006920C(s32 *, s32);
extern s32 func_80069250(s32, s32);
extern s32 func_800692C0(u32 *, s32, s16 *, s16 *);
extern s32 func_800693CC(s32, s32);
extern void func_80069898(s32 *, u16 *, s32);
extern void func_80069A30(void *);
extern void func_80069A8C(void *);
extern s32 func_8006B898(s32, u32);
extern void func_8006BEC4(s32, s32);
extern s32 func_8006C1FC(s32, s32);
extern void func_8006D324(void);
extern s32 func_8006D338(s32, s32);
extern s32 func_8006D74C(s32, s32);
extern s32 func_8006D7FC(void);
extern void func_8006D808(s32 *, s32 *, Unk8006D808Set *, s32, s32);
extern s32 func_8006E068(s32, s32);
extern s32 func_8006E10C(void);
extern s32 func_8006E2A8(void);
extern void func_8006E440(s32 *);
extern s32 func_8006E480(Unk8009B0E0Record *, s32);
extern s32 func_8006E49C(s32, Unk8006E49CRec *);
extern s32 func_8006E534(s32, s32, Unk8009BD24Block *, u32);
extern void func_8006E950(s32, Unk8006E950Head *);
extern s32 func_8006EACC(s32, s32);
extern s32 func_8007352C(Unk8007352CEnv *);
extern s32 func_80073728(Unk8007352CEnv *, s32);
extern void func_80074220(Unk8006EACCRec *, s32);
extern void func_80074488(Unk8006EACCRec *);
extern s32 func_80077820(s32);
extern s32 func_80077894(s32, s32);
extern s32 func_80077904(void);
extern void func_80077940(s32);
extern s32 func_80077984(s32);
extern s32 func_800779C8(s32, s32);
extern s32 func_80077A04(s32, s32);
extern s32 func_80077A60(s32, s32);
extern s32 func_80077AC0(s32, s32);
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern void func_80077B20(void);
extern s32 func_80077B30(s32, s32);
extern Unk8009BD24Block *func_80077D00(void);
extern s32 func_8007855C(s32);
extern s32 func_80078824(s32);
extern s32 func_800788B0(void);
extern void func_80061178(void);
extern void func_80036F40(void);
extern void *func_8003F1D4(void);
extern void *func_80046DEC(s32);
extern void func_80046B44(void);
extern void func_80046EDC(s32, s32);
extern void gpu_SetDrawEnvBg(s32, s32, s32, s32);
extern void gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *);
extern s32 gte_SumSquares3(s32, s32, s32);
extern s32 math_FovToScreenDist(s32);
extern s32 math_Grayscale3(s32, s32, s32);
extern void math_MatrixToAnglesYXZ(MATRIX *, SVECTOR *);
extern void math_RotMatrixYXZ(u16 *, MATRIX *);
extern void math_RotMatrixZYX(SVECTOR *, MATRIX *);
extern void math_TransposeMatrixInPlace(MATRIX *);
extern void memcard_AckHwEvents(void);
extern void memcard_AckSwEvents(void);
extern s32 memcard_CountFiles(s32, s32);
extern void memcard_Init(void);
extern s32 memcard_PollSwEventsTimeout(void);
extern void memcard_Quit(void);
extern s32 memcard_ReadFile(s32, s32, s32, s32, s32);
extern s32 memcard_WaitHwEvent(void);
extern s32 memcard_WriteFile(s32, s32, s32, s32, s32, s32, s32);
extern s32 func_80017738(s32, s32);
extern void func_80017E8C(s32);
extern void func_80017714(void);
extern void pad_ResetState(void);
extern void pad_ResetStateMarkValid(void);
extern void func_800415C4(s32);
extern void func_80041604(s32, s32);
extern void rcnt_StartCnt1(void);
extern s32 rng_Next(void);
extern void rng_SetSeed(s32);
extern void scratchpad_Restore(void);
extern void scratchpad_Save(void);
extern void func_80045188(void);
extern void func_800450BC(s32, s32);
extern void snd_CloseVab1(void);
extern void snd_Init(void);
extern s32 snd_LoadCommonVab(s32);
extern void snd_Quit(void);
extern void snd_SerialMixOn(void);
extern void func_80046AA0(void);
extern void snd_VabFakeOpen8And4(s32);
extern void snd_VabFakeOpen9(s32);
extern void func_8003F5CC(void);
extern void func_8003F568(void);
extern void func_8003F168(void);
extern s32 func_80046798(void);
extern void *func_80046F14(void);
extern void func_8003F274(void);
extern void sys_Init(void);
extern void sys_Panic(void);
extern void func_80017F90(void);
extern void func_80017F98(s32, s32, s32);

/* Sony library functions the game declares differently from the library. */
extern void ResetGraph(s32);

#endif /* BB2_H */
