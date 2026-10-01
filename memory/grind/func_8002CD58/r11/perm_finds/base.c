



typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
#pragma _permuter b64literal X19hc21fXygiLmluY2x1ZGUgXCJpbmNsdWRlL2xhYmVscy5pbmNcIlxuIik7





typedef struct {
    u32 addr : 24;
    u32 len : 8;
} OTag;


extern void (*GPU_printf)();
typedef struct GpuDevTable {
               const char *rcsid;
               void (*addque)();
               s32 (*addque2)();
               s32 (*clr)();
               void (*ctl)();
               s32 (*cwb)();
               void (*cwc)();
               s32 (*drs)();
               s32 (*dws)();
               s32 (*exeque)();
               s32 (*getctl)();
               s32 (*otc)();
               s32 (*param)();
               s32 (*reset)();
               u32 (*status)();
               s32 (*sync)();
} GpuDevTable;

extern GpuDevTable *g_gpu_dev_table;


typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    u32 tag;
    u32 code[15];
} DR_ENV;

typedef struct {
    RECT clip;
    s16 ofs[2];
    RECT tw;
    u16 tpage;
    u8 dtd;
    u8 dfe;
    u8 isbg;
    u8 r0, g0, b0;
    DR_ENV dr_env;
} DRAWENV;

typedef struct {
    RECT disp;
    RECT screen;
    u8 isinter;
    u8 isrgb24;
    u8 pad0;
    u8 pad1;
} DISPENV;
typedef struct {
    u8 type;
    u8 queue_mode;
    u8 debug_level;
    u8 reverse;
    s16 width;
    s16 height;
    s32 unk08;
    u32 drawsync_cb;
    DRAWENV draw_env;
    DISPENV disp_env;
} GpuCtx;

extern GpuCtx g_gpu_ctx;


extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void gpu_InitDisplay(void);
typedef struct GpuQueueItem {
               s32 (*func)(s32 *, s32);
               s32 *arg;
               s32 count;
               s32 data[21];
} GpuQueueItem;

extern volatile GpuQueueItem _que[64];
typedef struct GpuDrEnv {
               u32 tag;
               u32 code[15];
} GpuDrEnv;

extern GpuDrEnv D_800F1858;
extern s32 D_800A33B0;
extern s32 D_800A33B4;
extern s16 *D_800A33D0;
struct struct_svm {
    u8 prog_tones; u8 vabId; u8 note; u8 fine; u8 volume; u8 pan;
    u8 prog; u8 field_7_fake_program; u8 field_8_unknown; u8 field_0x9;
    u8 mvol; u8 mpan; u8 tone; u8 tone_vol; u8 tone_pan;
    u8 tone_prior; u8 tone_center; u8 tone_shift; u8 tone_min;
    u8 tone_max; u8 tone_mode; u8 pad; short seq_sep_no; short tone_vag_idx;
    short voice; short voiceOffset; short field_0x1e;
};
extern struct struct_svm _svm_cur;
struct SpuVoice {
    s16 unk0;
    s16 unk2;
    s16 unk04;
    u16 unk6;
    s16 unk8;
    u8 unka;
    u8 unkb;
    s16 unkc;
    s16 note;
    s16 unke;
    s16 unk10;
    s16 prog;
    s16 tone;
    s16 vabId;
    s16 unk18;
    u8 pad4[1];
    u8 unk1b;
    s16 auto_vol;
    s16 unk1e;
    s16 unk20;
    s16 unk22;
    s16 start_vol;
    s16 end_vol;
    s16 auto_pan;
    s16 unk2a;
    s16 unk2c;
    s16 unk2e;
    s16 start_pan;
    s16 end_pan;
};
extern struct SpuVoice _svm_voice[24];



typedef struct {
    u8 tones;
    u8 mvol;
    u8 prior;
    u8 mode;
    u8 mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1;
    u16 reserved2;
    u16 reserved3;
} ProgAtr;


typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u8 vs;
    u8 vspad;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;
extern VabHdr *_svm_vh;
struct SeqStruct {
    u8 *read_pos;
    u8 *next_sep_pos;
    u8 *loop_pos;
    u8 *unk0C;
    u8 *unk10;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 channel;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24[3];
    u8 panpot[16];
    u8 programs[16];
    s16 unk48;
    s16 unk4A;
    s16 unk4C;
    s16 unk4E;
    s16 unk50;
    s16 unk52;
    s16 unk54;
    s16 unk56;
    u16 unk58;
    u16 unk5A;
    s16 unk5C;
    s16 unk5E;
    s16 vol[16];
    s32 unk80;
    s32 unk84;
    s32 unk88;
    s32 unk8C;
    s32 delta_value;
    u32 unk94;
    s32 unk98;
    s32 unk9C;
    s32 unkA0;
    s32 unkA4;
    s32 unkA8;
    u32 unkAC;
};
extern struct SeqStruct *_ss_score[32];
typedef struct {
 void (*noteon) ();
 void (*programchange) ();
 void (*pitchbend) ();
 void (*metaevent) ();
 void (*control[13]) ();
 void (*ccentry[20]) ();
} _SsFCALL;
extern _SsFCALL D_800F3340;


extern void SsSetSerialAttr(s32, s32, s32);
extern u8 g_cd_file_table;
extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_stage_id;
extern s16 g_stage_variant;
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F1198Record;

extern Unk800F1198Record D_800F1198[];
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
} Unk800A9CF8Header;

extern Unk800A9CF8Header D_800A9CF8;
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0EC8Record;

extern Unk800F0EC8Record D_800F0EC8[][10];
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0E38Record;

extern Unk800F0E38Record D_800F0E38[12];
extern s16 D_8009BCC4[3][2];




extern s16 D_8009BCD0[2];
typedef struct {
    u8 unk0;
    u8 unk1;
} Unk8009BCF8Record;

extern Unk8009BCF8Record D_8009BCF8[20];
extern u8 D_8009BD20[2][2];
typedef struct {
    void *f00;
    s32 *f04;
    s16 f08[2];
    s16 f0C[2];
    union {
        s16 half[2];
        s32 word;
    } f10;
    union {
        s16 half[2];
        s32 word;
    } f14;
    s16 f18[2];
    s16 f1C[2];
    s16 f20[2];
    void *f24;
    u8 pad28[4];
    s32 *f2C;
    s32 f30;
    u16 f34;
    u16 f36;
    s16 f38[2];
    s16 f3C[2];
    s16 f40[2][2];
    s16 f48[2][5];
    s16 f5C[2];
    s16 f60[2];
    u8 f64;
    u8 f65;
    u8 f66;
    u8 f67;
    u8 f68[2];
    s16 f6A[2][5];
    s16 f7E[2][5];
} SelWork;

extern u8 *D_800A36A0;
extern struct MATRIX D_800FF558;
typedef struct {
    s16 x;
    s16 y;
} Unk8009BC94Record;

extern Unk8009BC94Record D_8009BC94[][6];
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk8009B398Record;

extern Unk8009B398Record D_8009B398[4];
typedef struct {
    s16 unk0;
    s16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} Unk8009B400Record;

extern Unk8009B400Record D_8009B400[10];
extern Unk8009B400Record D_8009B458[3][2];
typedef struct {
    s16 x;
    s16 y;
} Unk8009B450Record;

extern Unk8009B450Record D_8009B450[2];
extern Unk8009B400Record D_8009B5F0[2][2];
extern Unk8009B400Record D_8009B490[2][2];
typedef struct {
               s16 unk0;
               s16 unk2;
               s32 unk4;
               s16 unk8;
               s16 unkA;
               s32 unkC;
               s32 unk10;
               s32 unk14;
               s32 unk18;
               s16 unk1C;
               s16 unk1E;
               s16 unk20;
               s16 unk22;
               s16 unk24[4];
               s32 unk2C;
               s32 unk30;
               s32 unk34[2];
               s32 unk3C[2];
               s16 unk44[2];
               s16 unk48[2];
} Unk800EFAE8Ctrl;

extern Unk800EFAE8Ctrl D_800EFAE8;
extern s16 D_800A34F0[2];
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
} Unk800A3560Record;

typedef union {
    Unk800A3560Record rec[2];
    s32 word;
} Unk800A3560Slots;

extern Unk800A3560Slots D_800A3560;
typedef struct {
              s32 unk0;
              s32 unk4;
              s32 unk8;
} Unk800F0C10Record;

extern Unk800F0C10Record D_800F0C10[4][3];
extern s16 D_800F0BA8[18];
extern Unk800F0C10Record D_800F0CA0[18];




typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} StageFuncEntry;
extern StageFuncEntry g_stage_init_tbl[];
typedef struct {
    u8 unk_0;
    u8 unk_1;
    s32 unk_4;
} FileTimeRec;
typedef struct {
    s32 unk_00;
    u8 unk_04;
    u8 unk_05[3];
    FileTimeRec times[3];
    u8 color[3];
    u8 flags;
} FileRecord;
extern FileRecord D_80106A50;
extern s16 D_800A3710;





typedef struct {
    s32 time;
    s32 count;
    char *name;
} Alarm_t;
extern Alarm_t Alarm;


extern void ResetCallback(void);
typedef struct VECTOR { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;
extern s16 StatusUpBuf;
extern u8 cpu_practice_honmokuroku_data_tbl[][4];



extern u8 g_sqrt_table_u8[0x400];
extern s32 menuDat;

extern u8 D_800F5F68[];




extern s16 D_8008EB40[3][3];




extern s16 Judge[0x1000];







typedef struct {
    u8 pad00[0x14];
    s16 f14;
    u8 pad16[0x4E - 0x16];
    u16 f4E[3];
    u16 f54[3][3];
    u16 f66[11][3];
} Tbl800A3860Entry;




typedef struct { s32 x, y, z; } Vec3i32;




typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
typedef struct { s16 vx, vy, vz, pad; } SVec4i16;






typedef struct PracticeMenuRec {
    struct PracticeMenuRec *unk_00;
    s16 unk_04;
    s16 unk_06;
    s16 unk_08;
    s16 unk_0A;
    s16 unk_0C;
    s16 unk_0E;
    u8 unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14;
    u8 unk_16[0x1A - 0x16];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8 unk_22[0x2C - 0x22];
    s32 unk_2C;
    s32 unk_30;
    u8 unk_34[0x3C - 0x34];
    s32 unk_3C;
    s16 unk_40;
    u8 unk_42[0x5E - 0x42];
    s16 unk_5E;
    u8 unk_60[0x6A - 0x60];
    s16 unk_6A;
    u8 unk_6C[0x72 - 0x6C];
    s16 unk_72;
    u8 unk_74[0x7C - 0x74];
    s32 unk_7C;
    u8 unk_80[0x84 - 0x80];
    s16 unk_84;
    u8 unk_86[0x88 - 0x86];
    s16 unk_88;
    s16 unk_8A;
    u8 unk_8C[0x8E - 0x8C];
    s16 unk_8E;
    s16 unk_90;
    u8 unk_92[0x96 - 0x92];
    s16 unk_96;
    u8 unk_98[0xA0 - 0x98];
    u8 unk_A0;
    u8 unk_A1[0xB1 - 0xA1];
    u8 unk_B1;
    u8 unk_B2;
    u8 unk_B3[0xB8 - 0xB3];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8 unk_E4[0xE8 - 0xE4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8 unk_100[0x104 - 0x100];
    Vec4i32 unk_104;
    Vec4i32 unk_114;
    Vec4i32 unk_124;
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    u8 unk_154[0x156 - 0x154];
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8 unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8 unk_164[0x168 - 0x164];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    u8 unk_198[0x1C8 - 0x198];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    u8 unk_1DA[0x1DC - 0x1DA];
    s16 unk_1DC;
    u8 unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    u8 unk_1EC[0x1F8 - 0x1EC];
    Vec3i32 unk_1F8;
    u8 unk_204[0x24C - 0x204];
    Vec4i32 unk_24C;
    u8 unk_25C[0x268 - 0x25C];
    s32 unk_268;
    u8 unk_26C[0x274 - 0x26C];
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
    u8 unk_288[0x28C - 0x288];
    s32 unk_28C;
    u8 unk_290[0x31A - 0x290];
    s16 unk_31A;
    u8 unk_31C[0x330 - 0x31C];
    s16 unk_330;
    s16 unk_332;
    u8 unk_334[0x34A - 0x334];
    u8 unk_34A;
    u8 unk_34B;
    u8 unk_34C;
    u8 unk_34D;
    u8 unk_34E;
    u8 unk_34F[0x350 - 0x34F];
    s16 unk_350;
    u8 unk_352[0x44C - 0x352];
} PracticeMenuRec;







typedef struct PadState {
    s16 unk_00[4];
    u32 held;
    u32 pressed;
    u32 released;
    u32 unheld;
} PadState;

extern PracticeMenuRec g_practice_menu_table[];


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
extern u8 D_8008DA08[0x48];
extern s16 D_8008DA50;
extern s16 D_8008DA94;
extern s16 D_8008DAD8;
extern u8 D_8008DB1C;
extern u8 D_8008DD5C[27][8];
extern u16 D_8008DE34[27][6];
extern u16 D_8008DF78[27][6];



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
extern u16 D_8008E3C0[28];
extern u16 D_8008E3F8[27][4];
extern u16 D_8008E4D0[27][4];
extern u8 D_8008E5A8[];
extern u8 D_8008E5CC[][8];
extern u8 D_8008E6A4[][6];
extern u8 D_8008E748;
extern u8 D_8008E75C;



typedef struct {
    u8 a;
    u8 b;
} LeafThreshold;

extern LeafThreshold D_8008EA44[5];

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
extern u8 D_8008EB28[8][2];
extern u8 D_8008EB38[8];



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






typedef struct StatusFlagRec {
    u16 flags;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 pick_weight[7];
    u8 script_weight[8];
    u8 unk17;
} StatusFlagRec;
extern StatusFlagRec D_80099D88[];




extern u8 D_8009A8C4[][8][4];
extern u8 D_8009A9B4[][2];
extern u8 D_800A3100[][4];
extern s16 D_800A310C[4];
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



extern s16 D_800A3750[4];
extern u8 D_800A3758;







extern u8 g_cd_result[8];
extern u8 D_800A3769;
extern u8 D_800A376A;
extern u8 D_800A376B;
extern u8 D_800A376C;
extern s16 D_800A376E;
extern s32 D_800A3778;
extern u8 D_800A377B;
extern u8 D_800A377C[];
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



extern u8 D_800A3918[6];
extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern u8 D_800A3928;
extern u8 D_800A3929;
extern s32 D_800A3D40;






typedef struct Rec44 {
    s32 w0; s32 w4; s32 w8; s32 wC;
    s16 h10; s16 h12; s16 h14; s16 h16;
    s32 w18;
    s16 h1C; u8 b1E; u8 b1F;
    s32 w20; s32 w24; s32 w28; s32 w2C;
    s16 h30[2][4];
    u8 b40; u8 b41; u8 b42; u8 b43;
} Rec44;



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
typedef struct { s16 vx, vy, vz, pad; } Unk80101DF0Rot;
typedef struct { s16 m[3][3]; u16 pad; s32 t[3]; } Unk80101DF0Mat;
typedef struct {
    Unk80101DF0Rot rot;
    Unk80101DF0Mat mat;
} Unk80101DF0Xform;
typedef struct {
    u8 unk0;
    s8 unk1;
    u8 unk2[6];
    s16 unk8;
    s16 unkA;
    s32 unkC;
    Unk80101DF0Xform xf;
    Unk80101DF0Mat work;
} Unk80101DF0Record;
extern Unk80101DF0Record D_80101DF0;
extern Unk80101DF0Record D_800FF638;
typedef struct {
    s32 a;
    s32 b;
} CamPair;
typedef struct {
    s16 unk00;
    s16 unk02;
    s16 unk04;
    s16 unk06;
    s16 unk08;
    s16 unk0A;
    CamPair pair;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 sectors_remaining;
    s32 dest_buffer;
    s32 unk28;
    s32 unk2C;
    u8 unk30;
    s32 unk34;
    s16 unk38;
    s16 unk3A;
    s16 unk3C;
    u16 unk3E;
    s32 expected_pos;
    s32 unk44;
} ReplayCamRec;
typedef struct {
    u8 file;
    u8 chan;
    s32 unk04;
    ReplayCamRec rec;
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
extern u8 D_80101F75;
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
extern s32 D_801020D8;
extern s32 D_801020DC;
extern s32 D_801020E0;
extern s32 D_801020E4;
extern s32 D_801020E8;
extern s32 D_801020EC;
extern s32 D_801020FC;
extern s32 D_80102100;
extern s32 D_80102104;
extern s32 D_80102108;
extern s32 D_8010210C;
extern s32 D_80102110;

extern s16 D_801021E2;
extern s16 D_8010231A;
extern s16 D_80102334;
extern s32 D_80102350;
extern s16 D_8010235C;

extern s16 D_8010238E;
extern s16 D_801023AA;
extern u8 D_801023C1;
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


typedef struct {
    s32 x;
    s32 y;
    s32 z;
} LeafPos;

extern LeafPos D_80107850[6];


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
extern s32 ratan2(s32, s32);
extern void RotMatrixX(s32, s32 *);
extern void RotMatrixY(s32, s32 *);
extern void RotMatrixZ(s32, s32 *);
s32 func_8002CD58(u8 *obj) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 sp_tmp3;
    s32 len_sq;
    s32 xz_sq;
    s32 nxz_sq;
    s32 yaw;
    s32 pitch;
    s32 nyaw;
    s32 npitch;
    s32 nxz_dist;

    *(s32 *)(obj + 0xA8) = (*(s32 **)(obj + 0x64))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xAC) = (*(s32 **)(obj + 0x64))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xB0) = (*(s32 **)(obj + 0x64))[2] - (*(s32 **)(obj + 0x60))[2];
    *(s32 *)(obj + 0xB8) = (*(s32 **)(obj + 0x68))[0] - (*(s32 **)(obj + 0x60))[0];
    *(s32 *)(obj + 0xBC) = (*(s32 **)(obj + 0x68))[1] - (*(s32 **)(obj + 0x60))[1];
    *(s32 *)(obj + 0xC0) = (*(s32 **)(obj + 0x68))[2] - (*(s32 **)(obj + 0x60))[2];



#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgImx3ICAgICAkMTMsIDAoJDEyKVxuIgogICAgICAgICJsdyAgICAgJDE0LCA0KCQxMilcbiIKICAgICAgICAiY3RjMiAgICQxMywgJDBcbiIKICAgICAgICAibHcgICAgICQxNSwgOCgkMTIpXG4iCiAgICAgICAgImN0YzIgICAkMTQsICQyXG4iCiAgICAgICAgImN0YzIgICAkMTUsICQ0XG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QTgpIDogIiQxMiIsICIkMTMiLCAiJDE0IiwgIiQxNSIpOw==


#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgImx3YzIgICAkMTEsIDgoJDEyKVxuIgogICAgICAgICJsd2MyICAgJDksIDAoJDEyKVxuIgogICAgICAgICJsd2MyICAgJDEwLCA0KCQxMilcbiIKICAgICAgICAibm9wXG4iCiAgICAgICAgIm5vcFxuIgogICAgICAgIDo6ICJyIihvYmogKyAweEI4KSA6ICIkMTIiKTs=

#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoIi53b3JkIDB4NEI3MDAwMEMiKTs=

#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICJzd2MyICAgJDI2LCA0KCQxMilcbiIKICAgICAgICAic3djMiAgICQyNywgOCgkMTIpXG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QzgpIDogIiQxMiIsICJtZW1vcnkiKTs=

    if ((u32)(*(s32 *)(obj + 0xC8) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xCC) + 0x3FFF) < 0x7FFF
        && (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {
        s32 len;
        s32 dist;

#pragma _permuter b64literal ICAgICAgICBfX2FzbV9fIHZvbGF0aWxlKAogICAgICAgICAgICAibm9wXG4iCiAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgIi53b3JkIDB4NEFBMDA0MjhcbiIpOw==

#pragma _permuter b64literal ICAgICAgICBfX2FzbV9fIHZvbGF0aWxlKAogICAgICAgICAgICAibW92ZSAgICQxMiwgJTBcbiIKICAgICAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICAgICAic3djMiAgICQyNiwgNCgkMTIpXG4iCiAgICAgICAgICAgICJzd2MyICAgJDI3LCA4KCQxMilcbiIKICAgICAgICAgICAgOjogInIiKG9iaiArIDB4MTAwKSA6ICIkMTIiLCAibWVtb3J5Iik7
        len_sq = *(s32 *)(obj + 0x100) + *(s32 *)(obj + 0x104) + *(s32 *)(obj + 0x108);
        if ((u32)len_sq < 0x400) {
            len = (u32)g_sqrt_table_u8[len_sq] >> 3;
        } else {
            s32 lzcr = 0;
            if (len_sq >= 0) {



#pragma _permuter b64literal ICAgICAgICAgICAgICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgICAgICAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgICAgICAgICAgICAgIm10YzIgICAkMTIsICQzMFxuIgogICAgICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgICAgICAibm9wXG4iCiAgICAgICAgICAgICAgICAgICAgOjogInIiKGxlbl9zcSkgOiAiJDEyIik7
#pragma _permuter b64literal ICAgICAgICAgICAgICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgICAgICAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgICAgICAgICAgICAgInN3YzIgICAkMzEsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgICAgIDo6ICJyIigmc3BfdG1wKSA6ICIkMTIiLCAibWVtb3J5Iik7
                lzcr = sp_tmp;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = g_sqrt_table_u8[(u32)len_sq >> shift];
                len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        if ((u32)len < 0x4000) {
            yaw = ratan2(*(s32 *)(obj + 0xA8), *(s32 *)(obj + 0xB0));
            xz_sq = *(s32 *)(obj + 0xA8) * *(s32 *)(obj + 0xA8)
                  + *(s32 *)(obj + 0xB0) * *(s32 *)(obj + 0xB0);
            *(s16 *)(obj + 0xFA) = 0x800 - yaw;
            if ((u32)xz_sq < 0x400) {
                dist = (u32)g_sqrt_table_u8[xz_sq] >> 3;
            } else {
                s32 lzcr = 0;
                if (xz_sq >= 0) {


#pragma _permuter b64literal ICAgICAgICAgICAgICAgICAgICBfX2FzbV9fIHZvbGF0aWxlKAogICAgICAgICAgICAgICAgICAgICAgICAibW92ZSAgICQxMiwgJTBcbiIKICAgICAgICAgICAgICAgICAgICAgICAgIm10YzIgICAkMTIsICQzMFxuIgogICAgICAgICAgICAgICAgICAgICAgICAibm9wXG4iCiAgICAgICAgICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgICAgICAgICAgOjogInIiKHh6X3NxKSA6ICIkMTIiKTs=
#pragma _permuter b64literal ICAgICAgICAgICAgICAgICAgICBfX2FzbV9fIHZvbGF0aWxlKAogICAgICAgICAgICAgICAgICAgICAgICAibW92ZSAgICQxMiwgJTBcbiIKICAgICAgICAgICAgICAgICAgICAgICAgInN3YzIgICAkMzEsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgICAgICAgICA6OiAiciIoJnNwX3RtcDIpIDogIiQxMiIsICJtZW1vcnkiKTs=
                    lzcr = sp_tmp2;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = g_sqrt_table_u8[(u32)xz_sq >> shift];
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            pitch = ratan2(*(s32 *)(obj + 0xAC), dist);
            *(s16 *)(obj + 0xF8) = 0x800 - pitch;
            *(s16 *)(obj + 0xD8) = 0x1000;
            *(s16 *)(obj + 0xDA) = 0;
            *(s16 *)(obj + 0xDC) = 0;
            *(s16 *)(obj + 0xDE) = 0;
            *(s16 *)(obj + 0xE0) = 0x1000;
            *(s16 *)(obj + 0xE2) = 0;
            *(s16 *)(obj + 0xE4) = 0;
            *(s16 *)(obj + 0xE6) = 0;
            *(s16 *)(obj + 0xE8) = 0x1000;
            RotMatrixY(*(s16 *)(obj + 0xFA), (s32 *)(obj + 0xD8));
            RotMatrixX(*(s16 *)(obj + 0xF8), (s32 *)(obj + 0xD8));


#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgImx3ICAgICAkMTMsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgImx3ICAgICAkMTQsIDQoJDEyKVxuIgogICAgICAgICAgICAgICAgImN0YzIgICAkMTMsICQwXG4iCiAgICAgICAgICAgICAgICAiY3RjMiAgICQxNCwgJDFcbiIKICAgICAgICAgICAgICAgICJsdyAgICAgJDEzLCA4KCQxMilcbiIKICAgICAgICAgICAgICAgICJsdyAgICAgJDE0LCAxMigkMTIpXG4iCiAgICAgICAgICAgICAgICAibHcgICAgICQxNSwgMTYoJDEyKVxuIgogICAgICAgICAgICAgICAgImN0YzIgICAkMTMsICQyXG4iCiAgICAgICAgICAgICAgICAiY3RjMiAgICQxNCwgJDNcbiIKICAgICAgICAgICAgICAgICJjdGMyICAgJDE1LCAkNFxuIgogICAgICAgICAgICAgICAgOjogInIiKG9iaiArIDB4RDgpIDogIiQxMiIsICIkMTMiLCAiJDE0IiwgIiQxNSIpOw==


#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgImxodSAgICAkMTQsIDQoJDEyKVxuIgogICAgICAgICAgICAgICAgImxodSAgICAkMTMsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgInNsbCAgICAkMTQsICQxNCwgMTZcbiIKICAgICAgICAgICAgICAgICJvciAgICAgJDEzLCAkMTMsICQxNFxuIgogICAgICAgICAgICAgICAgIm10YzIgICAkMTMsICQwXG4iCiAgICAgICAgICAgICAgICAibHdjMiAgICQxLCA4KCQxMilcbiIKICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgIDo6ICJyIihvYmogKyAweEE4KSA6ICIkMTIiLCAiJDEzIiwgIiQxNCIpOw==


#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgiLndvcmQgMHg0QTQ4NjAxMiIpOw==

#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjYsIDQoJDEyKVxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjcsIDgoJDEyKVxuIgogICAgICAgICAgICAgICAgOjogInIiKG9iaiArIDB4QTgpIDogIiQxMiIsICJtZW1vcnkiKTs=


#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgImxodSAgICAkMTQsIDQoJDEyKVxuIgogICAgICAgICAgICAgICAgImxodSAgICAkMTMsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgInNsbCAgICAkMTQsICQxNCwgMTZcbiIKICAgICAgICAgICAgICAgICJvciAgICAgJDEzLCAkMTMsICQxNFxuIgogICAgICAgICAgICAgICAgIm10YzIgICAkMTMsICQwXG4iCiAgICAgICAgICAgICAgICAibHdjMiAgICQxLCA4KCQxMilcbiIKICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgICJub3BcbiIKICAgICAgICAgICAgICAgIDo6ICJyIihvYmogKyAweEI4KSA6ICIkMTIiLCAiJDEzIiwgIiQxNCIpOw==
#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgiLndvcmQgMHg0QTQ4NjAxMiIpOw==
#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjYsIDQoJDEyKVxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMjcsIDgoJDEyKVxuIgogICAgICAgICAgICAgICAgOjogInIiKG9iaiArIDB4QjgpIDogIiQxMiIsICJtZW1vcnkiKTs=
            return 0;
        }
    }

    nyaw = ratan2(*(s32 *)(obj + 0xC8), *(s32 *)(obj + 0xD0));
    *(s32 *)(obj + 0xC8) >>= 6;
    *(s32 *)(obj + 0xCC) >>= 6;
    *(s32 *)(obj + 0xD0) >>= 6;
    nxz_sq = *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8)
           + *(s32 *)(obj + 0xD0) * *(s32 *)(obj + 0xD0);
    *(s16 *)(obj + 0xFA) = 0x800 - nyaw;
    if ((u32)nxz_sq < 0x400) {
        nxz_dist = (u32)g_sqrt_table_u8[nxz_sq] >> 3;
    } else {
        s32 lzcr = 0;
        if (nxz_sq >= 0) {


#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgIm10YzIgICAkMTIsICQzMFxuIgogICAgICAgICAgICAgICAgIm5vcFxuIgogICAgICAgICAgICAgICAgIm5vcFxuIgogICAgICAgICAgICAgICAgOjogInIiKG54el9zcSkgOiAiJDEyIik7
#pragma _permuter b64literal ICAgICAgICAgICAgX19hc21fXyB2b2xhdGlsZSgKICAgICAgICAgICAgICAgICJtb3ZlICAgJDEyLCAlMFxuIgogICAgICAgICAgICAgICAgInN3YzIgICAkMzEsIDAoJDEyKVxuIgogICAgICAgICAgICAgICAgOjogInIiKCZzcF90bXAzKSA6ICIkMTIiLCAibWVtb3J5Iik7
            lzcr = sp_tmp3;
        }
        {
            s32 shift = 0x16 - (lzcr & ~1);
            s32 tbl = g_sqrt_table_u8[(u32)nxz_sq >> shift];
            nxz_dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    npitch = ratan2(*(s32 *)(obj + 0xCC), nxz_dist);
    *(s16 *)(obj + 0xF8) = 0x800 - npitch;
    *(s16 *)(obj + 0xD8) = 0x1000;
    *(s16 *)(obj + 0xDA) = 0;
    *(s16 *)(obj + 0xDC) = 0;
    *(s16 *)(obj + 0xDE) = 0;
    *(s16 *)(obj + 0xE0) = 0x1000;
    *(s16 *)(obj + 0xE2) = 0;
    *(s16 *)(obj + 0xE4) = 0;
    *(s16 *)(obj + 0xE6) = 0;
    *(s16 *)(obj + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)(obj + 0xFA), (s32 *)(obj + 0xD8));
    RotMatrixX(*(s16 *)(obj + 0xF8), (s32 *)(obj + 0xD8));



#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgImx3ICAgICAkMTMsIDAoJDEyKVxuIgogICAgICAgICJsdyAgICAgJDE0LCA0KCQxMilcbiIKICAgICAgICAiY3RjMiAgICQxMywgJDBcbiIKICAgICAgICAiY3RjMiAgICQxNCwgJDFcbiIKICAgICAgICAibHcgICAgICQxMywgOCgkMTIpXG4iCiAgICAgICAgImx3ICAgICAkMTQsIDEyKCQxMilcbiIKICAgICAgICAibHcgICAgICQxNSwgMTYoJDEyKVxuIgogICAgICAgICJjdGMyICAgJDEzLCAkMlxuIgogICAgICAgICJjdGMyICAgJDE0LCAkM1xuIgogICAgICAgICJjdGMyICAgJDE1LCAkNFxuIgogICAgICAgIDo6ICJyIihvYmogKyAweEQ4KSA6ICIkMTIiLCAiJDEzIiwgIiQxNCIsICIkMTUiKTs=
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgImxodSAgICAkMTQsIDQoJDEyKVxuIgogICAgICAgICJsaHUgICAgJDEzLCAwKCQxMilcbiIKICAgICAgICAic2xsICAgICQxNCwgJDE0LCAxNlxuIgogICAgICAgICJvciAgICAgJDEzLCAkMTMsICQxNFxuIgogICAgICAgICJtdGMyICAgJDEzLCAkMFxuIgogICAgICAgICJsd2MyICAgJDEsIDgoJDEyKVxuIgogICAgICAgICJub3BcbiIKICAgICAgICAibm9wXG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QTgpIDogIiQxMiIsICIkMTMiLCAiJDE0Iik7
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoIi53b3JkIDB4NEE0ODYwMTIiKTs=
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICJzd2MyICAgJDI2LCA0KCQxMilcbiIKICAgICAgICAic3djMiAgICQyNywgOCgkMTIpXG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QTgpIDogIiQxMiIsICJtZW1vcnkiKTs=
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgImxodSAgICAkMTQsIDQoJDEyKVxuIgogICAgICAgICJsaHUgICAgJDEzLCAwKCQxMilcbiIKICAgICAgICAic2xsICAgICQxNCwgJDE0LCAxNlxuIgogICAgICAgICJvciAgICAgJDEzLCAkMTMsICQxNFxuIgogICAgICAgICJtdGMyICAgJDEzLCAkMFxuIgogICAgICAgICJsd2MyICAgJDEsIDgoJDEyKVxuIgogICAgICAgICJub3BcbiIKICAgICAgICAibm9wXG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QjgpIDogIiQxMiIsICIkMTMiLCAiJDE0Iik7
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoIi53b3JkIDB4NEE0ODYwMTIiKTs=
#pragma _permuter b64literal ICAgIF9fYXNtX18gdm9sYXRpbGUoCiAgICAgICAgIm1vdmUgICAkMTIsICUwXG4iCiAgICAgICAgInN3YzIgICAkMjUsIDAoJDEyKVxuIgogICAgICAgICJzd2MyICAgJDI2LCA0KCQxMilcbiIKICAgICAgICAic3djMiAgICQyNywgOCgkMTIpXG4iCiAgICAgICAgOjogInIiKG9iaiArIDB4QjgpIDogIiQxMiIsICJtZW1vcnkiKTs=
    return 1;
}
