# 0 "src/main/psxsdk/libsnd/ssclose.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/mipsel-linux-gnu/include/stdc-predef.h" 1 3
# 0 "<command-line>" 2
# 1 "src/main/psxsdk/libsnd/ssclose.c"

# 1 "include/common.h" 1



# 1 "include/version.h" 1
# 5 "include/common.h" 2
# 23 "include/common.h"
# 1 "include/include_asm.h" 1
# 55 "include/include_asm.h"
__asm__(".include \"macro.inc\"\n");
# 24 "include/common.h" 2
# 1 "include/settings.h" 1
# 25 "include/common.h" 2
# 1 "include/types.h" 1




typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
typedef unsigned int size_t;
# 49 "include/types.h"
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char byte;
typedef unsigned short ushort;
typedef unsigned int uint;


enum { false, true };
typedef signed int bool;
# 73 "include/types.h"
typedef union {
    s32 val;
    struct {
        s16 lo;
        s16 hi;
    } i;
} f32;

typedef union {
    s16 val;
    struct {
        u8 lo;
        u8 hi;
    } i;
} f16;

typedef struct {
              s16 x;
              s16 y;
} Point16;

typedef struct {
              s32 x;
              s32 y;
} Point32;

typedef struct {
    u16 width;
    u16 height;
} Size16;

typedef struct {
    u8 u;
    u8 v;
} uvPair;

typedef struct {
    f32 x;
    f32 y;
} Pos;
# 26 "include/common.h" 2
# 3 "src/main/psxsdk/libsnd/ssclose.c" 2
# 1 "src/main/psxsdk/libsnd/libsnd_i.h" 1
# 9 "src/main/psxsdk/libsnd/libsnd_i.h"
# 1 "include/psxsdk/libspu.h" 1
# 65 "include/psxsdk/libspu.h"
typedef void (*SpuIRQCallbackProc)(void);


typedef struct {
    unsigned short left;
    unsigned short right;
} SpuVolume;

typedef struct {
               unsigned long voice;
               unsigned long mask;
               SpuVolume volume;
               SpuVolume volmode;
               SpuVolume volumex;
               unsigned short pitch;
               unsigned short note;
               unsigned short sample_note;
               short envx;
               unsigned long addr;
               unsigned long loop_addr;
               long a_mode;
               long s_mode;
               long r_mode;
               unsigned short ar;
               unsigned short dr;
               unsigned short sr;
               unsigned short rr;
               unsigned short sl;
               unsigned short adsr1;
               unsigned short adsr2;
} SpuVoiceAttr;

typedef struct {
    unsigned long mask;
    long mode;
    SpuVolume depth;
    long delay;
    long feedback;
} SpuReverbAttr;

typedef struct {
    SpuVolume volume;
    long reverb;
    long mix;
} SpuExtAttr;

typedef struct {
    unsigned long mask;

    SpuVolume mvol;
    SpuVolume mvolmode;
    SpuVolume mvolx;
    SpuExtAttr cd;
    SpuExtAttr ext;
} SpuCommonAttr;

extern long SpuSetTransferMode(long mode);
extern unsigned long SpuWrite(unsigned char* addr, unsigned long size);

extern long SpuSetReverbModeParam(SpuReverbAttr* attr);

extern void SpuSetVoiceAttr(SpuVoiceAttr* arg);
extern void SpuSetKey(long on_off, unsigned long voice_bit);

extern long SpuMallocWithStartAddr(unsigned long addr, long size);

extern SpuIRQCallbackProc SpuSetIRQCallback(SpuIRQCallbackProc);
# 10 "src/main/psxsdk/libsnd/libsnd_i.h" 2


struct Unk {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;
    s16 unkA;
    s16 unkC;
    u16 unkE;
    s16 unk10;
};
void _SsUtResolveADSR(u16 arg0, u16 arg1, struct Unk* arg2);
void _SsSeqPlay(s16, s16);
void _SsClose(s16);

void EnterCriticalSection(void);
void VSyncCallback(void (*func)());
void ExitCriticalSection(void);
void* InterruptCallback(u8, void (*)());
void ResetCallback(void);
void SpuInit(void);
void _SsInit(void);
void SsSeqCalledTbyT(void);
void Snd_SetPlayMode(s16, s16, u8, s16);
void SpuQuit(void);

extern s32 D_8003C74C;
extern SpuReverbAttr _svm_rattr;

void SpuVmSeKeyOn(s16 arg0, s16 arg1, u16 arg2, s32 arg3, u16 arg4, u16 arg5);
s32 SpuVmSetSeqVol(s16 seq_sep_no, u16 voll, u16 volr, s16 arg3);
s32 SpuVmGetSeqVol(s16, s16*, s16*);
s16 SpuIsTransferCompleted(s16);

void _spu_setInTransfer(s32);
u32 SpuSetTransferStartAddr(u32);
extern s32 _svm_vab_total[];
extern s32 _svm_vab_start[];
extern u8 _svm_vab_used[];

void SpuFree(s32);
extern u16 _svm_vab_count;

typedef struct VabHdr {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u16 vs;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;

s16 SsVabOpenHead(u8*, s16);
s16 SsVabTransBody(u8*, s16);
extern s32 _svm_brr_start_addr[];

extern u8 spuVmMaxVoice;

extern s16 _svm_stereo_mono;

void vmNoiseOn2(u8 arg0, u16 arg1, u16 arg2, u16 arg3, u16 arg4);

struct struct_svm {
    char field_0_sep_sep_no_tonecount;
    char field_1_vabId;
    char field_2_note;
    char field_0x3;
    char field_4_voll;
    char field_0x5;
    char field_6_program;
    char field_7_fake_program;
    char field_8_unknown;
    char field_0x9;
    char field_A_mvol;
    char field_B_mpan;
    char field_C_vag_idx;
    char field_D_vol;
    char field_E_pan;
    char field_F_prior;
    char field_10_centre;
    unsigned char field_11_shift;
    char field_12_mode;
    char field_0x13;
    u8 field_14_seq_sep_no;
    u8 pad;
    short field_16_vag_idx;
    short field_18_voice_idx;
    short field_0x1a;
    short field_0x1c;
    short field_0x1e;
};

extern struct struct_svm _svm_cur;

extern u8 spuVmMaxVoice;
void SeAutoVol(s16, s16, s16, s16);
void SeAutoPan(s16, s16, s16, s16);




struct SeqStruct {
    u8 unk0;
    u8 pad1[3];
    u8* read_pos;
    u8* next_sep_pos;
    u8* loop_pos;
    u8 unk10;
    u8 unk11;
    u8 channel;
    u8 unk13;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 panpot[16];
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2a;
    u8 unk2b;
    u8 programs[16];
    u8 unk3C;
    u8 pad3D;
    s16 unk3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    s16 unk46;
    s16 unk48;
    s16 unk4a;
    s16 unk4c;
    s16 vol[16];
    s16 unk6E;
    s16 unk70;
    s16 unk72;
    u16 unk74;
    u16 unk76;
    s16 unk78;
    s16 unk7A;
    s32 unk7c;
    u32 unk80;
    s32 unk84;
    s32 delta_value;
    s32 unk8c;
    s32 unk90;
    u32 unk94;
    u32 unk98;
    s32 unk9C;
    u32 unkA0;
    u32 unkA4;
    s16 padA6;
    s16 padaa;
};




extern struct SeqStruct* _ss_score[32];


extern void SpuSetCommonAttr(SpuCommonAttr* attr);

extern s16 _snd_seq_s_max;
extern s16 _snd_seq_t_max;

typedef struct ProgAtr {

    unsigned char tones;
    unsigned char mvol;
    unsigned char prior;
    unsigned char mode;
    unsigned char mpan;
    char reserved0;
    short attr;
    u32 reserved1;
    unsigned short reserved2;
    unsigned short reserved3;
} ProgAtr;



extern u8 spuVmMaxVoice;

struct SpuVoice {
    s16 unk0;
    s16 unk2;
    s16 unk04;
    u16 unk6;
    s16 unk8;
    char unka;
    char unkb;
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

u32 SpuVmVSetUp(s16, s16);

typedef struct VagAtr {

    unsigned char prior;
    unsigned char mode;
    unsigned char vol;
    unsigned char pan;
    unsigned char center;
    unsigned char shift;
    unsigned char min;
    unsigned char max;
    unsigned char vibW;
    unsigned char vibT;
    unsigned char porW;
    unsigned char porT;
    unsigned char pbmin;
    unsigned char pbmax;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned short adsr1;
    unsigned short adsr2;
    short prog;
    short vag;
    short reserved[4];

} VagAtr;

extern VagAtr* _svm_tn;

void SpuVmFlush();
void _SsSndCrescendo(s16, s16);
void _SsSndDecrescendo(s16, s16);
void _SsSndPause(s16, s16);
void _SsSndPlay(s16, s16);
void _SsSndReplay(s16, s16);
void _SsSndTempo(s16, s16);
extern s32 _snd_ev_flag;
extern s32 _snd_openflag;

short SsUtGetProgAtr(short vabId, short progNum, ProgAtr* progatrptr);
short SsUtGetVagAtr(
    short vabId, short progNum, short toneNum, VagAtr* vagatrptr);
short SsUtSetVagAtr(
    short vabId, short progNum, short toneNum, VagAtr* vagatrptr);

short SsVabTransBodyPartly(
    unsigned char* addr, unsigned long bufsize, short vabid);

u32 SpuWritePartly(u8*, u32);

struct SndSeqTickEnv {
    s32 unk0;
    s32 unk4;
    void (*unk8)();
    void (*unk12)();
    u8 unk16;
    u8 unk17;
    u8 unk18;
    u8 unk19;
    u32 unk20;
};

extern struct SndSeqTickEnv _snd_seq_tick_env;

extern u32 VBLANK_MINUS;

extern s16 _svm_damper;

extern VagAtr* _svm_vab_tn[16];
extern ProgAtr* _svm_vab_pg[16];
extern VabHdr* _svm_vab_vh[16];
extern ProgAtr* _svm_pg;
extern VabHdr* _svm_vh;
extern s16 kMaxPrograms;

extern unsigned short _svm_okon1;
extern unsigned short _svm_okon2;
extern unsigned short _svm_okof1;
extern unsigned short _svm_okof2;

void SsUtSetReverbDepth(short, short);
# 4 "src/main/psxsdk/libsnd/ssclose.c" 2

void _SsClose(s16 seq_sep_num) {
    s32 seq_num;
    SpuVmSetSeqVol(seq_sep_num, 0, 0, 1);
    SpuVmSeqKeyOff(seq_sep_num);
    _snd_openflag &= ~(1 << seq_sep_num);
    for (seq_num = 0; seq_num < _snd_seq_t_max; seq_num++) {
        _ss_score[seq_sep_num][seq_num].unk90 = 0;
        _ss_score[seq_sep_num][seq_num].unk3C = 0xFF;
        _ss_score[seq_sep_num][seq_num].unk0 = 0;
        _ss_score[seq_sep_num][seq_num].unk3E = 0;
        _ss_score[seq_sep_num][seq_num].unk40 = 0;
        _ss_score[seq_sep_num][seq_num].unk94 = 0;
        _ss_score[seq_sep_num][seq_num].unk98 = 0;
        _ss_score[seq_sep_num][seq_num].unk42 = 0;
        _ss_score[seq_sep_num][seq_num].unkA4 = 0;
        _ss_score[seq_sep_num][seq_num].unkA0 = 0;
        _ss_score[seq_sep_num][seq_num].unk9C = 0;
        _ss_score[seq_sep_num][seq_num].unk44 = 0;
        _ss_score[seq_sep_num][seq_num].unk74 = 0x7f;
        _ss_score[seq_sep_num][seq_num].unk76 = 0x7f;
    }
}

void SsSeqClose(short seq_access_num) { _SsClose(seq_access_num); }

void SsSepClose(short sep_access_num) { _SsClose(sep_access_num); }
