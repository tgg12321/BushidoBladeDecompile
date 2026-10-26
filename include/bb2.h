#ifndef BB2_H
#define BB2_H

/* Declarations of the game's objects and functions for the translation units in
 * src/main/ (SLUS_006.63's game code); their types are in game.h. */

#include "game.h"

extern FileRecord D_80106A50;
extern s16 g_camera_distance_offset;
extern GpuDb g_gpu_db[2];
/* The current frame's ordering table (OT entry n is g_gpu_ot_ptr + n). */
extern u32 *g_gpu_ot_ptr;
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void gpu_HideResetAndClear(void);
extern Unk800EF848Rec D_800EF848[];
extern u16 D_80099C34[][7];
/* 58 byte pairs (0x80099CC8..0x80099D3B), each byte an index into D_800EF980 or
 * 0xFF for none. */
extern u8 D_80099CC8[58][2];

/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *D_800A378C;

extern DR_MOVE D_800A3D70[31][2];
extern DR_MOVE D_800A4340[19][2];
extern DR_MOVE g_gpu_draw_move_packets[2][10];

/* The VRAM rectangle font_LoadImage loads the image at g_font_image into
 * (x 0x3F0, y 0x1DC, 16 x 36). */
extern RECT g_font_rect;

extern u8 D_800A3768;
extern u8 D_800A36A8;
extern s16 D_800F665C;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_loaded_resource_selector;
extern s16 D_8009947A;
extern Unk800F1198Record g_eff_billboards[];
extern Unk800A9CF8Rec D_800A9CF8;
extern Unk800F0EC8Record D_800F0EC8[][10];
extern Unk800F0E38Record D_800F0E38[12];
extern MenuState D_8009BC04;

/* The three scroll pages' origins: s16 (x, y) pairs for pages 4..6
 * (menu_UpdateChoiceGrid). */
extern s16 D_8009BCC4[3][2];

/* The current scroll offset, one s16 per axis (menu_UpdateChoiceGrid). */
extern s16 D_8009BCD0[2];

/* 20 flag bytes indexed by entry id (bit 0 = selectable; mask 4 << player,
 * i.e. bit 2 + player = taken by that player); d_8C4E4.c, read by 64FD8.c. */
extern u8 D_8009BCE4[20];
extern Unk8009BCF8Record D_8009BCF8[2][10];

/* Two 2-byte records, {0x01, 0x02} and {0x03, 0x04}, indexed [row][col] by
 * menu_HandleSelPhase0CursorInput, func_80074488 and menu_InitSelScreen. */
extern u8 D_8009BD20[2][2];

extern u8 *D_800A36A0;
#define SELWORK ((SelWork *)D_800A36A0)

/* One PsyQ MATRIX: func_8004A940 loads it as the GTE rotation, reads t[] and
 * passes it to gte_MulMatrix0ClearTrans; func_80048BA4 writes m[][] and passes
 * t to ApplyMatrix. */
extern MATRIX D_800FF558;

extern Unk8009BC94Record D_8009BC94[][6];
/* The SEL.BIN menu's tables (d_8C41C.c, read by 5ED34.c): the per-mode
 * update handlers (func_8006EACC), menu_UpdateRowCursors' row values, the
 * entry grid and its per-value flags. */
extern void (*D_8009BC1C[7])(Unk8006EACCRec *);
extern u8 D_8009BC38[];
extern SelectEntryE534 D_8009BC40[][6];
extern u8 D_8009BC7C[];
/* Table of 4 sprite-sheet headers, 12 bytes each (func_8005E098,
 * func_8005E54C). */
extern Unk8009B0E0Record D_8009B398[4];
extern Unk8009B400Record D_8009B400[10];
extern Unk8009B400Record D_8009B458[3][2];
extern Unk8009B450Record D_8009B450[2];

/* Two 8-byte sprite cells, the cell table func_8005D554 hands
 * gpu_AddScaledSpriteQuads for the row's header record 2 (cell 0) and records
 * 3/4 (cell 1). */
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
 * table func_80060414 hands gpu_AddSpriteCells in its descriptor. */
extern Unk8009B400Record D_800A328C;

extern Unk8009B2BCRecord D_8009B2BC[3];
extern Unk800EFAE8Rec D_800EFAE8;
extern Unk800F0C10Record D_800F0C10[4][3];

/* Per-effect-mode state, modes 0..0x11 (eff_DrawAnimatedQuads): the timers here
 * and the position records in D_800F0CA0. The init functions
 * eff_InitCosineScaledQuad.. func_800652AC fill them; the step functions
 * eff_DrawAndAdvanceCosineScaledQuad..func_800657B0 advance the timers. */
extern s16 D_800F0BA8[18];

extern Unk800F0C10Record D_800F0CA0[18];
extern Unk800948BCEntry D_800948BC[];

/* The per-mode main-loop handlers, indexed by D_800A3834. */
extern void (*D_8008D090[])(void);

extern u8 D_8008E778[][4];

/* g_sqrt_table_u8[i] = floor(8 * sqrt(i)), i = 0..0x3FF (d_7D920.c). */
extern u8 g_sqrt_table_u8[0x400];

extern Unk8008DCCCEntry D_8008DCCC[18];

/* 3 x 3 s16 angle offsets, [row][col] from the pad bits
 * (fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow,
 * fighter_UpdateInputSteeredForwardMotion). */
extern s16 D_8008EB40[3][3];

/* Judge: the sine table, one full turn in 0x1000 steps, 1.0 = 0x1000
 * (cos(a) = Judge[(a + 0x400) & 0xFFF]). */
extern s16 Judge[0x1000];

/* The 16 slots eff_SpawnJitteredBillboards spawns and func_800646E8 draws (bit
 * i of D_800A3444 live): slot i's position. */
extern Vec3i32 D_800F0D78[16];

extern Unk800EED10Entry g_arena_blocks[10];

/* The two characters' hit records (0x800F5F68..0x800F62D7):
 * fighter_ScaleBoneHitRecords fills D_800F5F68[ch] from the template
 * D_8008D59C, offsets and limits scaled. */
extern BoneHitRec D_800F5F68[2][22];

extern BoneHitRec D_8008D59C[22];

/* The 6 slots func_8006288C spawns and func_8006295C draws (bit i of D_800A3460
 * live): slot i's position and its RotMatrixZYX angles. */
extern Vec3i32 D_800F0FB8[6];

extern SVec4i16 D_800F10A0[6];
extern s16 D_800F0C04[6];
extern Unk800EFB78Entry g_snd_queue[24];
extern Unk80101EC8Record D_80101EC8[];
extern const char D_800109C8[];
extern u8 D_8008D518;
extern u8 D_8008D538[];
extern u8 D_8008D55C;
extern u8 D_8008D578[];

/* Attachment point sets fighter_UpdateBoneWorldData rotates by a character's
 * bone matrices: D_8008D86C[unk_0E] (D_8008D864[unk_0E] points; D_8008D774 when
 * unk_12 == 50) and D_8008D88C[unk_14] (two points, when unk_8C != 0).
 * D_800A3138 is the unit y vector (0, 0x1000, 0). */
extern u8 D_8008D864[8];

extern SVec4i16 *D_8008D86C[8];
extern SVec4i16 *D_8008D88C[32];
extern SVec4i16 D_8008D774[2];
extern SVec4i16 D_800A3138;
extern u8 D_8008D9EC[];
extern u8 D_8008DA08[0x48];
extern s16 D_8008DA50[]; /* [unk_0A] (fighter_Update) */
extern s16 D_8008DA94[]; /* [unk_0A] (fighter_Update) */
extern s16 D_8008DAD8[]; /* [unk_0A] (fighter_Update) */
/* [unk_0A][unk_0E] -> Unk80101EC8Record.motion_data_id
 * (motion_LoadFighterData) */
extern u16 D_8008DB1C[27][8];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_84 */
extern u8 D_8008DD5C[27][8];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.base_move_frame_step */
extern u16 D_8008DE34[27][6];
/* [unk_0A][unk_0E] -> Unk80101EC8Record.unk_1E / unk_20 */
extern u16 D_8008DF78[27][6];
extern Tbl8008E194 D_8008E194[];

/* 8 byte pairs ({0,0} {1,0} {2,0} {3,0} {4,1} {5,1} {6,1} {7,0});
 * ResetFlowAndFetchRowTablePair returns [n][0] and caches [n][1] in D_800A35E0,
 * n = D_8009BD24.unk14_0. */
extern u8 D_8009BD58[8][2];

extern Obj80106A78 g_object_pool[12];
/* [unk_0A][i] -> Unk80101EC8Record.carried_object_kinds[i]
 * (object_InitKindQueue) */
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
/* per-limb angle (object_DeflectFromFighterHit: [j], j < 22) */
extern u16 D_8008EBA0[22];
extern s32 D_8008EBCC[];
extern s32 D_8008EBE0[];
extern u8 D_8008EBF4[6];
extern Unk8008EA44Entry D_8008EBFC[6];
extern u8 D_8008EC30[4];
extern s16 D_8008F12C[8];

/* Twelve rows cdrom_StartAudioEntry reads by index: unk_0, the group-5 file
 * cdrom_GetFileIndex resolves; unk_1, cdrom_StartAudio's second argument;
 * unk_4, an offset added to the file's start sector (-1: none). */
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
extern u8 D_8009016C[];
extern u32 g_font_image[];
extern u32 g_gpu_last_fog_ot_index;
extern s32 D_80090600;
extern s32 D_80090604;
extern s16 D_80090608;

/* Two per-stage tables, indexed by the stage id
 * gpu_RunGridDrawPassAndQueueFogMoves passes to
 * gpu_LatchFogDepthOnTransitionAndQueueMoves: 38 s32, then s16 pairs ([0]
 * tested, [1] passed to gpu_QueueDepthSpacedMovesAndLinkTerminalPacket; the
 * last pair is zero). */
extern s32 D_8009060C[38];

extern s16 D_800906A4[39][2];
extern u16 D_80094C68[];
extern Unk80099D88Rec D_80099D88[];
extern Unk8009A8C8Entry D_8009A8C8[][8];
extern u8 D_8009A9B4[][2]; /* byte pairs (fighter_InitCpuControl) */
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
extern u8 D_800A3690;
/* FAKE: word 1 of the 8-byte sio send buffer as its own symbol; declaring
 * g_comb_send_buf s32[2] makes cc1 keep its address live through
 * comb_ExchangePad (35 words differ) */
extern s32 g_comb_send_buf_plus_0x4;
extern s16 D_800A36A4;
extern s32 D_800A36AC;
/* the Rec44 view record camera_ApplyFightView / camera_ApplyReplayView last
 * placed (read by snd_QueuePositionalSound as the listener) */
extern Rec44 *g_listener_cam;
extern CdlATV g_cd_atv_target;
/* FAKE: the high halfword (+2) of the s32 link pad word D_800A36C0 as its own
 * symbol. comb_ExchangePad reads it lui / %lo (symbol + 2) while the word
 * and its low half go gp-relative. One-object forms score: Q33 union per
 * pad word 4 (member reads force the address into $s0), (u16) word low
 * reads 4, u16 h[2] 7, s16 h[2] with casts 4, aligned halves 4. Owner
 * ruling Q125: comb_ExchangePad only. */
extern s16 D_800A36C2;
extern s32 D_800A36C4;
extern s16 D_800A36C6;
extern u8 D_800A36C8;
extern u16 D_800A36CA;
extern u8 D_800A36CC;
/* FAKE: the high halfword (+2) of the s32 link pad word D_800A36D0 as its own
 * symbol. comb_ExchangePad reads it lui / %lo (symbol + 2) while the word
 * and its low half go gp-relative. One-object forms score: Q33 union per
 * pad word 4 (member reads force the address into $s0), (u16) word low
 * reads 4, u16 h[2] 7, s16 h[2] with casts 4, aligned halves 4. Owner
 * ruling Q125: comb_ExchangePad only. */
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern MoveScript *D_800A36D8;
extern u8 D_800A36E8;
extern u8 D_800A36F0;
extern u8 D_800A36F1;
/* per-player byte, [unk_04] (object_InitKindQueue) */
extern u8 D_800A36F2[2];
extern u8 D_800A36F4;
extern s16 D_800A36F6;
extern u8 D_800A36F9;
extern u8 g_camera_fight_view_mode;
extern s16 D_800A36FC;
extern u8 D_800A3712;
extern u8 D_800A3713;
extern CdlATV g_cd_atv;
extern s32 D_800A371C;
extern u8 D_800A3728;
extern s8 D_800A3748;

/* Four s16 counters / latches, one per D_8008E914 row snd_UpdateStageSounds
   walks (type 1 latches 1; types 2..6 count up until an rng_Next() draw clears
   them); snd_ResetStageSoundState clears it from the last element down. */
extern s16 g_snd_stage_sound_state[4];

extern u8 D_800A3758;

/* The 8-byte CD status/result buffer CdSync / CdReady fill: result[0] is the
 * status byte; cdrom_UpdateAudio hands &result[3] / &result[5] (the reported
 * position) to CdPosToInt. Tentative definition in src/main/26940.c (Q62
 * global COMMON model). */
extern u8 g_cd_result[8];

extern u8 D_800A3769;
extern u8 D_800A376A;
extern u8 D_800A376B;
extern u8 D_800A376C;
extern s16 D_800A376E;
extern MATRIX *D_800A3778;
/* match-result table: 0/1/2 per match, indexed by g_fight_match_cursor */
extern u8 g_fight_match_results[];

/* Two buffer addresses selected by frame parity: func_80016D78 sets 0x801D8800
 * / 0x801EBC00, menu_RunPauseMenu and main read [D_800A36AC & 1]. */
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
extern u8 g_fight_win_target;
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
extern s32 g_motion_common_buffer;
extern s16 D_800A3834;
extern u8 D_800A3836;
extern u8 *D_800A3844;
extern s32 D_800A3858;
extern u8 *D_800A385C;
extern Tbl800A3860Entry *g_motion_fighter_buffers[];
extern u8 g_fight_match_cursor;
extern s16 D_800A3876;
extern u8 *D_800A3878;
extern s32 D_800A387C;
extern u8 g_motion_common_loaded;
/* per player: preset motion frames (motion_InitBuffers, fighter_Update) */
extern MotionFrame *g_motion_frame_buffers[2];
extern u8 D_800A389A;
extern u8 g_fight_match_count;
extern u16 D_800A389C;
extern s32 D_800A38A0;
extern u8 D_800A38A4;
extern s16 D_800A38A8;
extern s16 D_800A38AE;
extern u8 D_800A38B0;
extern u8 D_800A38B8;
extern s16 D_800A38BA;
/* per player: id of the 7-sector frame set loaded into
 * g_motion_frame_buffers[i] (0xFF = none) */
extern u8 g_motion_frame_set_ids[2];
/* per slot: model id in the g_motion_fighter_buffers[i] buffer
 * (motion_LoadFighterData); [1] = 0xFFFF: func_8001DB9C passed 0x80190800
 * (g_motion_fighter_buffers[1]'s buffer) to ndata_LoadPack; func_80020D38 calls
 * ndata_InvalidatePack for it */
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

/* Six per-slot counters, 0 = free: snd_QueuePositionSequence claims a slot with
   1, snd_UpdateStageSounds advances and frees them, snd_ResetStageSoundState
   zeroes all six. D_800A391E is not an element: it is its own countdown byte,
   and snd_UpdateStageSounds uses only its address, as the loop bound. */
extern u8 g_snd_sequence_ticks[6];

extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern u8 g_comb_abort_request;
extern u8 D_800A3929;
extern Unk800A3D40Rec g_bitstream_delta_accumulators[];

/* Three word pairs: the uncalled func_8003F5A8 stores pair i,
 * ClearSavedWordPairsAndTwoWriteOnlyWords zeroes all six, func_8003F5CC passes
 * pair i to func_80017F98(a, b, i), an empty function; nothing else reads them.
 */
extern s32 D_800A93B0[3];

extern s32 D_800A93BC[3];
extern Rec44 D_800F5328;
extern Rec44 D_800F6608;

/* Per-match snapshot: fight_RecordMatchResult stores both players' counters
 * (D_800A3898 / D_800A3899) for match g_fight_match_cursor;
 * fight_PackMatchScores packs the pairs into D_800A3784,
 * fight_TallyMatchResultsAndInitPresentation sums them. */
extern u8 g_fight_match_scores[8][2];

extern Unk800F68E0Rec g_replay_model_draws[0xB4];
/* The two InitPAD receive buffers, 0x24 bytes each (sys_Init); pad_Update
 * copies the first two words of each. */
extern s32 g_pad_buf[2][9];
extern Unk800FF5C8Rec g_camera_view_offset;
extern Unk80101BF0Rec g_replay_events[0x20];
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
extern Unk80101DF0Record D_800EEDF0;
extern Unk80101DF0Record D_800EF070;
extern Unk800F62E0Rec D_800F62E0[8];
extern RotMatrixFunc D_800F66A0[6];

/* The CD file table at 0x8008EC34: one 8-byte record per disc file, indexed by
 * the file numbers cdrom_GetFileIndex forms. `loc` is sought (cdrom_StartRead
 * copies the record into D_80101E58.rec.pair, whose position is sought;
 * cdrom_LoadExec seeks to it directly); `size` is cdrom_StartRead's sector
 * count, cdrom_GetFileSize's result and cdrom_StartAudio's end position. */
extern CdFileEntry g_cd_file_table[159]; /* 0x8008EC34..0x8008F12B */

extern CdState D_80101E58;
/* the common file's section pointers */
extern Unk801027B0Rec g_motion_common_sections;
extern Unk80102778Rec D_80102778;
extern PadState g_pad_state;
extern Unk801027B0Rec g_motion_fighter_sections[]; /* per load slot */
/* [i] points to block i's slots, the words after its header word
 * (reloc_RelocateBlock records it and D_80103658[i] holds the slot count). Each
 * slot is a word holding a block-relative offset that reloc_RelocateBlock turns
 * into an address by adding the block's base, unless header bit 15 marks the
 * block relocated; reloc_UnrelocateBlock subtracts the base again (and clears
 * bit 15), and reloc_RebaseBlock moves [i] by a1 / 4 words and adds a1 to each
 * slot.  gpu_OffsetBlockTexPrimStreams / gpu_OffsetBlockTexPrimStream store a
 * slot to scratchpad word 0, the u16 * cursor gpu_OffsetTexPrimStream reads,
 * and softbody_CreateForNode reads u16 data through one.
 */
extern s32 *D_80103608[];
extern Unk80104E88Rec D_80104E88[];
extern s32 D_80104F38;

/* fight_BuildOpponentOrder's list: the set bit numbers of D_80106A50.unk_00 &
 * mask, shuffled, plus up to three appended values (D_800A391F entries;
 * fight_AdvanceOpponentOrder reads entry D_800A3783, then advances it). */
extern u8 D_801077B0[24];

extern Unk80107850Rec g_snd_sequence_positions[6];
extern void camera_UpdateReplayView(Rec44 *, Rec1C *, Rec1C *, s32, s32, s32);
extern void font_Printf(u8 *, s32, ...);
extern void fighter_StartMoveScript(s32, MoveScript *, s32);
extern void fighter_Init(s32, s32, s32, s32, s32);
extern s32 cdrom_GetFileIndex(s32, s32);
extern void comb_ExchangePad(PadState *);
extern void func_8003AE5C(u8 *);
extern void gpu_BuildFogImageLevels(RECT *, s32);
extern void func_8003F1E4(s32);
extern void func_80041688(s32, s32);
extern void func_80048BA4(s32, s32, s32);
extern void func_800493E4(s32);
extern void func_800494D4(s32, s32);
extern void func_80049584(s32);
extern s32 func_8005B8B8(s32);
extern void snd_FlushSoundQueue(void);
extern s32 func_80038C70(void);
extern void gpu_SetSegmentCellsGateBit(s32, s32, s32, s32);
extern void cdrom_Update(void);

/* Declared by more than one translation unit: data, then functions, by name. */
extern u8 D_8008E908[][5];
extern u8 D_8008E914[][8];
extern s32 D_8008EA50[2][4];
extern u8 D_8008EC24[][5];
/* 51268's sprite texture records (TexRec runs 0x8009B890..0x8009B9FF; sizes
 * from the data labels) */
extern TexRec D_8009B890[4];
extern TexRec D_8009B8B0[3];
extern TexRec D_8009B8C8[1];
extern TexRec D_8009B8D0[1];
extern TexRec D_8009B8D8[1];
extern TexRec D_8009B8E0[1];
extern TexRec D_8009B8E8[7];
extern TexRec D_8009B920[4];
extern TexRec D_8009B940[1];
extern TexRec D_8009B948[1];
extern TexRec D_8009B950[1];
extern TexRec D_8009B958[1];
extern TexRec D_8009B960[1];
extern TexRec D_8009B968[1];
extern TexRec D_8009B970[1];
extern TexRec D_8009B978[1];
extern TexRec D_8009B980[1];
extern TexRec D_8009B988[1];
extern TexRec D_8009B990[1];
extern TexRec D_8009B998[4];
extern TexRec D_8009B9B8[4];
extern TexRec D_8009B9D8[1];
extern TexRec D_8009B9E0[1];
extern TexRec D_8009B9E8[1];
extern TexRec D_8009B9F0[2];
/* one table of 12 records: [0..5] / [6..9] the two sprite sets, [10] / [11]
 * their D_8009BD24.unk20_0 CLUT substitutes (eff_DrawBillboardList) */
extern TexRec D_8009BA00[12];
extern u8 D_8009BA60[];
extern Unk8009BD24Block D_8009BD24;
extern s32 g_ndata_pack_loaded;
extern s32 D_800A326C;
extern s32 D_800A32BC;
extern u32 D_800A32C8[2];
extern Unk80101DF0Record *D_800A370C;
extern Unk80101DF0Record *D_800A3708;
extern s32 g_memcard_poll_count;
extern void mem_MoveBlockRoundedUpToWords(s32, s32, s32);
extern u8 D_800A37A8[];
extern u16 D_800A37C4;
extern POLY_FT4 *g_eff_quad_cursor;
extern u32 *D_800A3808;
extern s16 g_cd_fade_step;
extern u8 D_800A384C;
extern s16 g_cd_fade_len;
extern u8 *D_800A3894;
extern u32 *g_prim_buf_cursor; /* the draw-chunk builders' word cursor */
extern s32 D_800A38D0;
extern u16 D_800A38D6;
extern s32 D_800A38FC;
extern s32 D_800A3908;
extern u8 D_800A3916;
extern u8 g_arena;
extern s16 D_800F0BCC[];
extern s16 D_800F0BEC[];
extern s32 g_eff_command_phase[];
/* A 16-byte vector: thirteen functions store x / y / z; func_80061064 passes
 * it to light_SetPointLight, which copies all 16 bytes to D_800A9B28. */
extern VECTOR D_800F1140;
extern Unk800F1B18Rec g_motion_decoders[];
extern u8 D_800F33D8[];
extern s16 D_800F6650;
extern s16 D_800F6656;
extern u8 D_801027A0[5][2];
extern u8 D_801027D8[5][2];
/* the draw queue: record pointers (0x80102C00..0x801035FF) */
extern void *g_draw_queue[640];
extern s32 D_8009BA7C[];
extern Func80017A44Output g_softbody_slots[8];
extern s32 D_80094B88[];
extern Unk80045878Obj *D_800A9A10[];

extern s32 bits_DepositMask3F83F8(s32);
extern s32 bits_ExtractMask3F83F8(s32);
extern s16 *camera_CalcAngles(void);
extern void camera_CopyViewMatrixWithHalvedMiddleRow(void);
extern u32 cdrom_GetFileSize(s32);
extern void cdrom_Init(void);
extern void cdrom_ReadyCallback(u8, u8 *);
extern void cdrom_SetMix(s32, s32, s32, s32);
extern s32 cdrom_StartAudio(s32, s32);
extern s32 cdrom_StartRead(s32, s32);
extern s32 cdrom_StartReadAt(s32, s32, s32, s32);
extern MATRIX *camera_GetViewMatrixWithHalvedMiddleRow(void);
extern void eff_Init(void);
extern void func_800164F8(void);
extern u32 func_800167AC(void);
extern u32 func_800167BC(void);
extern u32 func_800167D4(void);
extern void snd_InitAndLoadCommonVab(void);
extern void eff_ClearInitFlag(void);
extern void func_800174F4(void);
extern s32 softbody_Create(Func80017A44Input *);
extern void softbody_UpdateList(Unk80017FA0Rec *, s32);
extern void func_8001945C(void);
extern s32 func_80019488(void);
extern void func_800194C0(s32);
/* No prototype: defined void (s32), but menu_RunPauseMenu calls it with no
 * argument. */
extern void pad_Update();
extern void motion_InitStream(s32, u32 *);
extern void motion_DecodeFrame(s32, s32, MotionFrame *, u16 *);
extern void camera_UpdatePadOffset(s32 *);
extern void func_8001B6F4(void);
extern void camera_UpdateReplayViewWithSineYawOffset(Rec1C *, Rec1C *, s32);
extern void camera_UpdateReplayViewWithConstantYawOffset(Rec1C *, Rec1C *, s32);
extern void fight_InitSettings(void);
extern void func_8001CD68(Unk8001CD68Rec *);
extern void func_8001D790(void);
extern void func_8001DA2C(void);
extern s32 snd_ShouldUpdateStageSoundSlots(void);
extern void fight_WaitForCounterThenClearAndEnableDisplay(void);
extern void camera_ApplyReplayView(s32);
extern void motion_ApplyRecordedPoseAdjustments(
    Unk80101EC8Record *, Rec1C *, MotionFrame *, MotionFrame *);
extern void fighter_SetTurnDelta(Unk80101EC8Record *, s32);
extern void func_800203B4(Unk80101EC8Record *, s32, s16 *);
extern s32 fight_UpdateSeparationAndPairedMoves(void);
extern void fighter_UpdateBoneWorldData(
    Unk80101EC8Record *, Unk80107850Rec *, Unk80107850Rec *, Unk80107850Rec *);
extern void func_80020CDC(void);
extern void func_80020D38(void);
extern void motion_InitBuffers(void);
extern void fighter_AssignMotionSlotAndScanDataIdNibbles(s32);
extern void *motion_ResolveMoveData(Unk80101EC8Record *, s32, s16 *);
extern void motion_ResetVariantIndex(s32);
extern MoveScript *motion_GetCurrentVariantScript(s32);
extern MoveScript *motion_GetDefaultVariantScript(s32);
extern MoveScript *func_800219E4(s32);
extern MoveScript *func_80021A3C(s32, s32);
extern void fight_GetStartPosition(s32, s32 *, s32);
extern s32 fight_FindNearestStartPair(Vec3i32 *);
extern void func_80022568(Unk80101EC8Record *);
extern void fighter_ConfigureCpuControl(void);
extern s32 fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow(
    Unk80101EC8Record *, s32 *);
extern void fighter_Update(s32, PadState *);
extern void func_80026DA4(void);
extern void motion_SetFixedChannelBlock(MotionFrame *);
extern s32 func_8002798C(Unk80101EC8Record *);
/* fighter_TryDropQueuedHeadAtBone (17AFC.c, void (Unk80101EC8Record *)) has no
 * declaration: 175A4.c calls it implicitly, and any truthful declaration
 * changes func_80026DA4's register-save order. */
extern void fighter_ResolveBodyOverlap(void);
extern void func_8002906C(void);
extern s32 fighter_ProcessWeaponContacts(void);
extern void fighter_ProcessWeaponBodyHits(s32);
extern s32 fighter_CalcSeparationErrorAndOpposedOffsets(void);
extern void fight_UpdateFighterInteractionsAndPointCaches(void);
extern void math_ScaleVectorInDirectionFrame(s32 *, s16 *, s32 *, s32, s32);
extern void math_InvertMatrix3x3(void *, void *);
extern void math_ComposeAxisRotationsAndRewriteAngles(s16 *, s32, s32, s32);
extern s32 fighter_IsBoneTriangleCrossYPositive(Unk80101EC8Record *);
extern void object_ResetPool(void);
extern void object_InitKindQueue(Unk80101EC8Record *);
extern void object_FreeAttachedObjects(void);
extern s32 object_ThrowQueuedKind(Unk80101EC8Record *);
extern void object_DropQueuedHeadAt(Unk80101EC8Record *, Vec3i32 *);
extern void object_SpawnScatteredAt(Unk80101EC8Record *, s32, Vec3i32 *);
extern s32 object_PickUpNearby(Unk80101EC8Record *);
extern void func_80030D04(void);
extern void object_UpdatePool(void);
extern void object_ResolveFighterHits(void);
extern void fight_ClearBallisticRecords(void);
extern Unk80104E88Rec *func_80032064(Unk80101EC8Record *, s32);
extern void fight_UpdateBallisticRecords(void);
extern void fight_TriggerOpposingFighterEventFromNearBallisticRecords(void);
extern void fighter_ReadFrameMetadata(Unk80101EC8Record *);
extern void snd_QueuePositionalSound(s32, s32 *);
extern void func_80032854(s32, s32, s32 *, s16 *);
extern void fighter_ProcessFrameEvents(Unk80101EC8Record *);
extern void snd_ResetStageSoundState(void);
extern void snd_UpdateStageSounds(void);
extern void fight_BuildOpponentOrder(void);
extern void fight_AdvanceOpponentOrder(void);
extern s32 func_80033DF4(void);
extern void func_80033FE4(void);
extern void fight_RecordMatchResult(void);
extern void fight_AdvanceMatch(void);
extern void func_800344B4(void);
extern void func_80034F88(void);
extern void func_8003504C(void);
extern void func_80035280(void);
extern void cdrom_StartMixFade(s16, s32, s32, s32, s32);
extern s32 cdrom_StartAudioEntry(s32);
extern void cdrom_SetAudioRepeat(s16);
extern void cdrom_WaitAudioStarted(void);
extern void cdrom_PauseAndWait(void);
extern void sys_ExecMovieOverlay(s32, s32, s32, s32, s32);
extern s32 memcard_PollSwEvents(void);
extern s32 memcard_CountFreeBlocks(void);
extern s32 memcard_HasFile(u8 *);
extern s32 func_80037D14(s32, s32);
extern s32 func_8003880C(void);
extern s32 func_800388A8(void);
extern s32 func_80038988(void);
extern s32 *func_800392B8(void);
extern void replay_Reset(void);
extern void replay_BeginFrame(void);
extern void replay_RecordModelDraw(s32, s32, s32 *, s16 *);
/* replay_RecordEvent (28708.c, u8 / u8 / s32 * / u16 *) has no prototype here:
 * 17AFC.c declares it unprototyped, as its prototype changes 17AFC.c's bytes.
 */
extern void replay_RecordFighter(Unk80101EC8Record *);
extern void replay_AdvanceFrame(void);
extern void comb_Init(void);
extern void comb_ReadCtsSetRts(void);
extern void comb_EnableEvents(void);
extern void comb_ResetClose(void);
extern void comb_RequestClose(void);
extern void comb_ExchangeEmptyPad(void);
extern void comb_BeginWait(void);
extern void comb_EndWait(void);
extern s32 comb_Connect(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);
extern void fight_InitCommandStream(s32);
extern void func_8003B328(void);
extern s32 func_8003B484(u8 *);
extern void func_8003B534(s32);
extern void func_8003B56C(s32);
extern void fight_RunCommandStream(void);
extern void font_LoadImage(void);
extern void font_BeginFrame(void);
extern void font_AddTpage(void);
extern void bitstream_InitDeltaAccumulator(s32, s32);
extern s16 *bitstream_AccumulateDeltas(s32);
extern void gpu_InitFogDrawMoves(void);
extern void gpu_LatchFogDepthOnTransitionAndQueueMoves(s32, s32);
extern void func_8003DDF8(u32);
extern void func_8003E0E0(void);
extern void func_8003E120(void);
extern void gpu_MoveObjectClutsToFogBlock(s32);
extern void gpu_RestoreObjectClutOffsets(void);
extern s32 gpu_GetFogClutObject(void);
extern u32 gpu_GetLastFogOtIndex(void);
extern s32 gpu_DepthToOtIndex(s32, s32);
extern void gpu_SetViewOriginSegmentCellsGateBit(s32, s32);
extern void
    gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards(s32);
extern void func_8003EDC0(u16 *, s32);
extern void func_8003F218(s32);
extern s32 func_8003F268(void);
extern void gpu_SetCellForceBit(s16 *);
extern void func_8003F52C(s32, s32, s32);
extern void softbody_UpdateObject(Unk80045878Obj *);
extern void func_8003F7F4(void);
extern void softbody_SetupObject(Unk80045878Obj *, s32);
extern void softbody_RequestRebuild(Unk80045878Obj *);
extern void softbody_RequestSnap(s32);
extern void softbody_SetObjectGround(Unk80045878Obj *, s32);
extern void softbody_FreeObject(Unk80045878Obj *);
extern void softbody_FreeObjectById(s32);
extern void softbody_RebaseObject(Unk80045878Obj *, s32);
extern void func_80040304(s32, s32);
extern void func_800404A0(Unk80045878Node *, s32);
extern void func_800404D8(void);
extern Unk80045878Obj *func_80040510(s32, s32, s32);
extern void func_80040A78(Unk80045878Obj *);
extern void func_80040CB8(Unk80045878Obj *);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80041188(s32, u8 *, u8 *, s32, MATRIX *);
extern void func_80041398(s32);
extern void func_80041430(s32, s32);
extern void node_RebaseParents(s32, Unk80045878Node *, s32);
extern Unk80045878Obj *func_8004153C(s32);
extern void node_UpdateWorldMatrix(Unk80101DF0Record *);
extern void node_SetMatrixFromNegatedRot(Unk80101DF0Record *);
extern void func_80041988(s32, s32, s32, s32);
extern void func_80041AC8(Unk80045878Obj *);
extern void func_80041BF4(s32, s32, s32);
extern void light_SetPointLight(VECTOR *, s32);
extern void light_ResetPointLight(void);
extern void light_SetPointLightTarget(s32, s32);
extern void light_ApplyPointLightTargets(void);
extern void light_DisablePointLight(void);
extern void func_800421C8(s32);
extern void math_InitRotMatrixFuncs(void);
extern void gpu_OffsetBlockTexPrimStreams(s16, s16, s16, s16, s32);
extern void gpu_OffsetBlockTexPrimStreamsByPages(s16, s16, s16, s16, s16);
extern s16 gpu_OffsetTPage(s16, s16, s16);
extern s16 gpu_OffsetClut(s16, s16, s32);
extern void reloc_RelocateBlock(s32 *, s16);
extern void reloc_UnrelocateBlock(s16);
/* No prototype: defined void (s32, s32), but func_800466C0 calls it as
 * reloc_RebaseBlock(8). */
extern void reloc_RebaseBlock();
extern s32 reloc_KeepSelectedEntries(s32 *, s16 *);
extern s32 reloc_CopySelectedEntries(s32, s32 *, s16 *);
extern void reloc_ClearCounts(void);
extern void func_800444BC(void);
extern void func_800444E0(void);
extern void func_80044504(u32 *);
extern s32 func_80044670(s16 *, s16, s32);
extern void func_8004473C(void);
extern void func_80044800(void);
extern void func_80044B30(s32, s32);
extern void func_80044C70(s32);
extern void math_LerpShortVectorsNegateYZ(s16 *, s16 *, s32, s32);
extern void func_80044F30(s32, s32);
extern void func_80044F80(s32, s32);
extern s32 ndata_ReadChecked(s32, s32);
extern s32 ndata_GetArenaRoomAfter(s32);
extern s32 ndata_ReadFromPack(s32, s32);
extern s32 ndata_IsPackLoaded(void);
extern void ndata_StartReadInf(void);
extern void arena_Reset(void);
extern void arena_UpdatePeak(s32);
extern void arena_Free(s32);
extern void arena_Resize(s32, s32);
extern s32 *arena_OpenBlock(s32);
extern void arena_SetBlockEnd(s32, s32);
extern void arena_SetMoveCallback(s32, s32);
extern s32 *arena_FindBlock(s32);
extern s32 arena_GetBlockAddr(s32);
extern s32 arena_GetFreeSize(void);
extern s32 arena_GetSize(void);
extern void *arena_GetBase(void);
extern void arena_Copy(s32, s32, s32);
extern Unk80045878Obj *func_80045878(s32, s32, s32);
extern void func_80045A28(s32, s32);
extern void func_80045A50(s32);
extern void func_80045B68(s32, s32, s16 *, s32);
extern void func_80046020(void);
extern void func_8004659C(s32);
extern s32 fight_GetPositionRecordList(void);
extern void func_80046914(void);
extern s32 *func_800469C4(s32);
extern void motion_FreePlaybackBlock(void);
extern void gpu_ResetDrawQueueAndPrepareViewAndLighting(
    Vec3i32 *, SVECTOR *, s32);
extern void
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(
        s32);
extern void gpu_SetRaisedOtCutoffRequested(s32);
extern s32 gpu_GetRaisedOtCutoffRequested(void);
extern void gpu_RunGridDrawPassAndQueueFogMoves(s32);
extern void math_SetLightPlaneProjection(void);
extern void gpu_LoadTimGroup(u32 *, s32);
extern void gpu_LoadTimGroupOffset(u32 *, s32, s16, s16);
extern void gpu_LoadTimGroupSplitOffset(u32 *, s32, s16, s16, s16, s16);
/* gpu_LoadTim and gpu_LoadTimClut have no prototype: 368E4.c calls them
 * above their old-style (K&R) definitions with int arguments, and a prototype's
 * s16 / u16 parameters change its bytes. */
extern s32 gpu_RefreshGrayscaleMode(void);
extern void math_GrayscaleRgb555(u16 *, s32, u16 *);
extern void gpu_TintVramRowInPlace(s32, s32, s32, s32, s32, s32);
extern s32 func_80048AD0(s32);
extern void ResetEntrySelectionState(void);
extern void node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
    s32, s32, s32 *, s16 *);
extern void node_QueueTableEntryNodePairUnderSlotNode(s32, s32, s32);
extern s32 func_80049C24(s32, s32);
extern void func_80049E1C(void);
extern void calc_LightMatrix(Unk800F62E0Rec *);
extern s16 *gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision(
    s32, s16 *, s16 *, s32);
extern void gte_LerpMatrixRowsAndRebuildOrthonormalBasis(
    MATRIX *, MATRIX *, s32, MATRIX *);
/* No prototype: defined with no arguments, but func_8003EDC0 /
 * softbody_CreateForNode pass it one. */
extern void func_80052C10();
extern s32 func_8005344C(s32 *, s32 *, s32 *, s16 *, s32);
extern s32 collision_TestSegment(s32 *, s32 *, s32 *, s16 *);
extern s32 collision_TestSegmentWithWork(s32 *, s32 *, s32 *, s16 *, s32);
extern void collision_SetGrid(s32);
extern void collision_RebaseGrid(s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_800548DC(void);
extern s32 func_80054F68(void);
extern void motion_RebasePlayback(s32);
extern s16 *motion_GetPlaybackViewAngles(void);
extern s32 *func_8005508C(void);
extern void fighter_ClearCpuTally(s32);
extern void fighter_HalveCpuTally(s32);
extern void fighter_InitCpuControl(s32, u16 *, u16 *);
extern void fighter_BuildCpuPad(s32, PadState *);
extern void snd_StopAllVoices(void);
extern void snd_ResetVoicesAndQueue(void);
extern void func_8005B644(s32);
extern void func_8005B6AC(void);
extern void func_8005B72C(void);
extern void func_8005B868(void);
extern void func_8005B9C4(void);
extern s32 func_8005B9FC(s32);
extern s32 snd_SetReverb(s32);
extern s32 func_8005BA8C(s32, s32, s32, s32);
extern void snd_RebaseListedVabs(s32);
extern void snd_CloseListedVabs(void);
extern void snd_StopVoicesAndReverb(void);
extern void func_8005C614(void);
extern s32 snd_QueueSound(s32, s32, s32); /* FAKE: int, see definition */
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
extern s32 gpu_DrawImageAndAdvanceSplitWipe(s32, s32);
extern s32 func_800600C8(s32, s32, s32);
extern void func_800602AC(s32, s32 *);
extern s32 func_80060544(s32, s32);
extern void func_80060758(void);
extern s32 func_80060768(s32, s32, s32);
extern s32 eff_LoadImagesAndSetup(s32, s32);
extern void eff_SelectQuadBuffer(s32);
extern void func_80061064(s16 *, s32 *);
extern void func_800611A4(s32 *, s16 *);
extern void eff_StartRecycledCommandWithRoseLight(s32 *);
extern s32 eff_StartCosineScaledQuadWithPaleBlueLight(s32 *);
extern s32 func_800613C8(s32 *);
extern s32 func_80061454(s32 *);
extern s32 func_800614E0(s32 *);
extern void eff_StartRecycledCommandWithPaleRedLight(s32 *);
extern void eff_StartArgSelectedCommandWithCyanLight(s32 *, s32);
extern void func_80061710(s32 *, s32);
extern void eff_StartRecycledCommandWithOrangeLight(s32 *);
extern void eff_StartRecycledVectorCommandWithRedLight(s32 *, s16 *);
extern void func_800619A4(s32 *);
extern void func_800619F0(s32 *);
extern void eff_StartYawedQuadSlotCommand(s32 *, s16, s32, s32);
extern void func_80061ACC(s32 *, s16 *);
extern void func_80061D74(s32 *, s16);
extern void func_80061EC0(s32 *);
extern void eff_SetRotMatrix(s16 *, s32 *, MATRIX *);
extern void eff_DrawBillboardList(s16 *, s32 *);
extern void menu_SetEnabledRows(s32);
extern s32 menu_LoadResourcesInitStateSelectFirstEnabledRow(
    s32, Unk8009BD24Block *);
extern void reloc_RelocateOffsetList(s32 *, s32);
extern s32 gpu_DrawWhiteTileAndSheetsTestPadMask(s32, s32);
extern s32 pad_StepOffsetsGetDirectionEvents(u32 *, s32, s16 *, s16 *);
extern s32 menu_UpdateRowMenu(s32, s32);
extern void gpu_AddThreeGreyTilesStackedUp(s32 *, u16 *, s32);
extern void gpu_SetTileRgbFromBlockFlag(void *);
extern void gpu_SetPolyF4RgbFromBlockFlag(void *);
extern s32 menu_UpdateOptionMenuChoices(s32, u32);
extern void gpu_DrawSheetPairCenteredTileWithOptionalSelection(s32, s32);
extern s32 func_8006C1FC(s32, s32);
extern void menu_ResetRowBitCursors(void);
extern s32 menu_UpdateRowBitMenu(s32, s32);
extern s32 menu_UpdateStoredChoice(s32, s32);
extern s32 menu_ResetConfirmCursor(void);
extern void gpu_DrawIndexedCellsAndDecimalDigitPairs(
    s32 *, s32 *, Unk8006D808Rec *, s32, s32);
extern s32 menu_UpdateConfirmMenu(s32, s32);
extern s32 gpu_BeginFullscreenImageDisplay(void);
extern s32 gpu_EndFullscreenImageDisplay(void);
extern void reloc_RelocateSelfOffsets(s32 *);
extern s32 gpu_GetSheetTPage(Unk8009B0E0Record *, s32);
extern s32 gpu_InitPrimBufferPair(s32, Unk8006E49CRec *);
extern s32 func_8006E534(s32, s32, Unk8009BD24Block *, u32);
extern void func_8006E950(s32, Unk8006E950Head *);
extern s32 func_8006EA28(Unk8006EA28Rec *);
extern s32 func_8006EACC(s32, s32);
extern void gpu_AddPanelGrid(Unk8006EACCRec *);
extern s32 gpu_AddSpriteCells(Unk8007352CRec *);
extern s32 gpu_AddScaledSpriteQuads(Unk8007352CRec *, s32);
/* No prototype: defined old-style (63D2C.c) with an s16 parameter, which a
 * prototype cannot match. */
extern s32 gpu_AddRotatedSpriteQuads();
extern void func_80074220(Unk8006EACCRec *, s32);
extern void func_80074488(Unk8006EACCRec *);
extern s32 InitRowMenuBlackBgResetFlow(s32);
extern s32 menu_UpdateRowMenuStoreResult(s32, s32);
extern s32 ResetFlowAndFetchRowTablePair(void);
extern void StoreCompactedBits(s32);
extern s32 InitWithCachedValueAndStoredBitsBlackBg(s32);
extern s32 StepThenCloseVab1OnNonzero(s32, s32);
extern s32 menu_ResetFlowThenRunUpdate(s32, s32);
extern s32 func_80077A60(s32, s32);
extern s32 func_80077AC0(s32, s32);
extern void menu_ResetFlowBackgroundAndConfirmCursor(void);
extern s32 menu_InitSelWithStoredBits(s32);
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern void SetFlowStateOne(void);
extern s32 menu_RunOptionModeFlowStateWithRowBitEdit(s32, s32);
extern Unk8009BD24Block *func_80077D00(void);
extern s32 InitTimedSpriteSequence(s32);
extern s32 StepTimedSpriteSequenceWithSkip(s32);
extern s32 InitScrollingSpriteDisplay(s32);
extern s32 StepScrollingSpriteDisplay(void);
extern void eff_StopCommands(void);
extern void cdrom_WaitIdle(void);
extern void *func_8003F1D4(void);
extern void *node_GetSlotMatrixPointerArray(s32);
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
extern s32 memcard_ReadFile(s32, s32, s32, void *, s32);
extern s32 memcard_WaitHwEvent(void);
extern s32 memcard_WriteFile(s32, s32, s32, void *, s32, s32, s32);
extern s32 softbody_GetBufferSize(s32, s32);
extern void softbody_Free(s32);
extern void softbody_FreeAll(void);
extern void pad_ResetState(void);
extern void pad_ResetStateMarkValid(void);
extern void func_800415C4(s32);
extern void func_80041604(s32, s32);
extern void rcnt_StartCnt1(void);
extern s32 rng_Next(void);
extern void rng_SetSeed(s32);
extern void scratchpad_Restore(void);
extern void scratchpad_Save(void);
extern void ndata_InvalidatePack(void);
extern void ndata_LoadPack(s32, s32);
extern void snd_CloseVab1(void);
extern void snd_Init(void);
extern s32 snd_LoadCommonVab(s32);
extern void snd_Quit(void);
extern void snd_SerialMixOn(void);
extern void func_80046AA0(void);
extern void snd_VabFakeOpen8And4(s32);
extern void snd_VabFakeOpen9(s32);
extern void func_8003F5CC(void);
extern void ClearSavedWordPairsAndTwoWriteOnlyWords(void);
extern void func_8003F168(void);
extern s32 resource_GetLoadedSelector(void);
extern void *fight_GetStartPositionPairTable(void);
extern void gpu_ResetAndStampViewCenteredCellAttributes(void);
extern void sys_Init(void);
extern void sys_Panic(void);
extern void func_80017F90(void);
extern void func_80017F98(s32, s32, s32);

/* Sony library functions the game declares differently from the library. */
extern void ResetGraph(s32);

#endif /* BB2_H */
