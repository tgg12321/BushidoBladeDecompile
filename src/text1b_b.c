#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"


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
#define NULL ((void *)0)

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;

extern s32 func_800484A0(s32, s32, s32);
extern void func_800485EC(s32, s32, s32, s32, s32, s32);
extern s16 g_color_mode;
extern s32 snd_LoadBgm(u8);
extern s32 snd_PlayBgm(s32);
extern u8 D_80099BCC;
extern s32 D_800A33E0;
extern s32 D_800A33E4;
extern s32 func_8004153C(s32);
extern s32 g_snd_play_count;
extern u8 g_snd_ch_data[];
extern u16 g_snd_se_bank[];
extern void InitFadePanel(void);
extern s32 D_800A36AC;
extern s32 D_800A378C;
extern s32 D_800EF848;
extern s32 initLoadImage(void *, s16 *, s32, s32);
extern s16 D_800EF9F2;
extern s16 D_800EF9F4;
extern s16 D_800A33EA;
extern s16 D_800A33E8;
extern s32 D_800A33EC;
extern s32 InitFadePanel();
extern u8 D_80099CC8[];
extern u8 D_80099CC9[];
extern s16 D_800EF980[];
extern s16 D_80099C50[];
extern s32 D_800A324C;
extern s32 func_8004954C(s32, s32, s32);
extern s32 saTanMainDispGnd_80046020();
extern void efc_rob_set_type_particle(s32, s32, s16 *, s32);
extern s32 func_8003E120();
extern u8 *D_800A3820;
extern u8 *D_800A38B4;
extern s32 (*g_anim_func_table)(s16 *, s16 *);
extern u8 *func_8004153C(s32);
extern void func_8007E4DC(s16 *, s16 *, s16 *);
extern void func_8007ED6C(s32, s16 *, s32 *);
extern s16 D_80099D3C[];
extern void func_800417D0(s32 *);
extern s16 D_80099CC2;
extern s32 func_800418D0();
extern s32 func_8004A1FC();
extern void *D_800A3708;
extern void *D_800A370C;
extern u8 D_800FF638;
extern s8 D_800FF639;
extern s16 D_800FF640;
extern s32 D_800FF644;
extern s16 D_800FF648;
extern s16 D_800FF64A;
extern s16 D_800FF64C;
extern s32 D_800FF684;
extern s32 D_800FF688;
extern s32 D_800FF68C;
extern u8 D_80101DF0;
extern s8 D_80101DF1;
extern s16 D_80101DF8;
extern s32 D_80101DFC;
extern s16 D_80101E00;
extern s16 D_80101E02;
extern s16 D_80101E04;
extern s32 D_80101E3C;
extern s32 D_80101E40;
extern s32 D_80101E44;
extern u8 D_800153F0;
extern u8 D_800F62E0;
extern s32 D_800F6318;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern void func_8004A09C(s32, u16 *);
extern void gte_SetColorMatrix(s32 *);
extern void gte_SetBackColor(s32, s32, s32);
extern s32 math_Cos();
extern s32 math_Sin();
extern s32 func_80052754(s32, s32, s32);
extern s32 func_80052D00();
extern void func_80053754();
extern void func_80053E9C();
extern u8 D_800EFA00;
extern u8 D_800EF9F8;
extern s32 D_800A33F4;
extern s32 func_80052754(s32, s32, s32, s32);
extern s32 func_80052D00(s32, s32);
extern u8 *D_800A33F4;
extern u16 D_800A33F8;
extern s32 D_800A33F0;
extern s16 D_800A33F8;
extern s16 InfoPosYTbl1[];
extern void func_80054604(s32, s32, s32, s32, s32, s32, s32);
extern u32 D_80102C00;
extern u16 D_800A38D6;
extern s32 D_800A3808;
extern s32 func_8005490C(void);
extern void leaf_muki_awase_800444E0(void);
extern s32 D_800EFB14;
extern s32 D_800EFB18;
extern s32 D_800EFB1C;
extern s32 D_800EFB20;
extern s32 D_800EFB24;
extern s32 D_800EFB28;
extern s32 D_800EFB0C;
extern s32 D_80101E1C;
extern u16 D_80099D88;
extern u8 D_8009A830;
extern s8 D_8009A838;
extern u8 D_8009A840;
extern s32 single_game_getEnemyCharId(s32, s32);
extern s32 func_800233AC(void *, s32 *);
extern s32 D_8009AA50[];
extern s16 Judge;
extern s16 D_800A3400;
extern s32 D_800A3408;
extern s32 D_800EFB38;
extern s32 D_800EFB78;
extern s32 D_800EFB7C;
extern s32 D_800EFC38;
extern void DispStuff(void);
extern s32 SetBloodSpot(s32);
extern s32 memcard_SetSlot(s32);
extern s32 sys_Shutdown(void);
extern s32 title_mv_exec2(s32);
extern s32 func_80085E4C(s32, s32);
extern s32 func_80085EE4(s32);
extern s32 func_80085F98(void);
extern s32 D_800EFB38[];
extern s32 D_800EFC38[];
extern void func_800858D0(s32);
extern void func_80086130(s32, s32, s32);
extern u32 D_800EFB78[];
extern u8 D_800EFB7C[];
extern s32 D_80015470;
extern u8 D_80099CC8;
extern u8 D_80099CC9;
extern u8 D_80099D8B;
extern u8 D_80099D8C;
extern u8 D_80099D8D;
extern u8 D_80099D8E;
extern u8 D_80099D8F;
extern u8 D_80099D94;
extern u8 D_80099D9C;
extern u8 D_80099D9D;
extern u8 D_8009A088;
extern u8 D_8009A820;
extern u8 D_8009A821;
extern u8 D_8009A850;
extern u8 D_8009A851;
extern u8 D_8009A852;
extern u8 D_8009A853;
extern u16 D_8009A8CA;
extern u8 D_8009AD18;
extern u8 D_8009B14E;
extern s16 D_8009B16C;
extern s16 D_8009B17C;
extern s16 D_8009B18C;
extern s16 D_8009B2BC;
extern s16 D_8009B2BE;
extern s16 D_8009B2C4;
extern u16 D_8009B450;
extern u16 D_8009B452;
extern u8 D_8009B48E;
extern u8 D_8009B58C;
extern u8 D_8009BA60;
extern s32 chractar_use_pset_combo_id_table;
extern s32 D_8009BC04;
extern s32 D_8009BC08;
extern u8 D_8009BC0C;
extern u8 D_8009BC0D;
extern s32 D_8009BC1C;
extern u8 D_8009BC38;
extern u8 D_8009BC40;
extern u8 D_8009BC41;
extern u8 D_8009BC44;
extern u8 D_8009BC72;
extern u8 D_8009BC76;
extern u8 D_8009BC7C;
extern s16 D_8009BC94;
extern s16 D_8009BC96;
extern u16 D_8009BCC4;
extern u16 D_8009BCC6;
extern s16 D_8009BCD0;
extern s16 D_8009BCD2;
extern u8 D_8009BCE4;
extern u8 D_8009BCF8;
extern u8 D_8009BCF9;
extern u8 D_8009BD20;
extern u8 D_8009BD21;
extern s32 D_8009BD38;
extern s32 D_8009BD44;
extern u8 D_8009BD58;
extern u8 D_8009BD59;
extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;
extern s32 D_8009BD84;
extern s32 D_8009BD88;
extern u8 D_800A3270;
extern s32 D_800A32C8;
extern s16 D_800A3438;
extern s32 D_800A344C;
extern s16 D_800A34E8;
extern s16 D_800A3530;
extern s16 D_800A3534;
extern s16 D_800A3540;
extern s16 D_800A3544;
extern u8 D_800A3560;
extern u8 D_800A3561;
extern u8 D_800A3562;
extern s16 D_800A3588;
extern s16 D_800A358C;
extern s16 D_800A3590;
extern s16 D_800A3594;
extern s32 D_800A3618;
extern s32 D_800A3628;
extern s32 D_800A362C;
extern s32 D_800A3638;
extern s32 D_800A3708;
extern s32 D_800A370C;
extern s32 D_800A3828;
extern s32 D_800A38B4;
extern s32 D_800A38D8;
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern u16 D_800EFC8A;
extern s16 D_800F0B78;
extern s16 D_800F0B7C;
extern s16 D_800F0B98;
extern s16 D_800F0BA4;
extern s16 D_800F0BB2;
extern s16 D_800F0BCC;
extern s16 D_800F0BEC;
extern s32 D_800F0C10;
extern s32 D_800F0C14;
extern s32 D_800F0C18;
extern s32 D_800F0CA0;
extern s32 D_800F0CA4;
extern s32 D_800F0CA8;
extern s32 D_800F0CAC;
extern s32 D_800F0CB0;
extern s32 D_800F0CB4;
extern s32 D_800F0CB8;
extern s32 D_800F0CBC;
extern s32 D_800F0CC0;
extern s32 D_800F0CC4;
extern s32 D_800F0CC8;
extern s32 D_800F0CCC;
extern s32 D_800F0CD0;
extern s32 D_800F0CD4;
extern s32 D_800F0CD8;
extern s32 D_800F0CDC;
extern s32 D_800F0CE0;
extern s32 D_800F0CE4;
extern s32 D_800F0CE8;
extern s32 D_800F0CEC;
extern s32 D_800F0CF0;
extern s32 D_800F0CF4;
extern s32 D_800F0CF8;
extern s32 D_800F0CFC;
extern s32 D_800F0D18;
extern s32 D_800F0D1C;
extern s32 D_800F0D20;
extern s32 D_800F0D24;
extern s32 D_800F0D28;
extern s32 D_800F0D2C;
extern s32 D_800F0D30;
extern s32 D_800F0D34;
extern s32 D_800F0D38;
extern s32 D_800F0D3C;
extern s32 D_800F0D40;
extern s32 D_800F0D44;
extern s32 D_800F0D48;
extern s32 D_800F0D4C;
extern s32 D_800F0D50;
extern s32 D_800F0D54;
extern s32 D_800F0D58;
extern s32 D_800F0D5C;
extern s32 D_800F0D60;
extern s32 D_800F0D64;
extern s32 D_800F0D68;
extern s32 D_800F0D6C;
extern s32 D_800F0D70;
extern s32 D_800F0D74;
extern s32 D_800F0D78;
extern s32 D_800F0D7C;
extern s32 videoDec;
extern s32 D_800F0E38;
extern s32 D_800F0E3C;
extern s32 D_800F0E40;
extern s32 D_800F0EC8;
extern s32 D_800F0ECC;
extern s32 D_800F0ED0;
extern s32 D_800F0FB8;
extern s32 D_800F0FBC;
extern s32 D_800F0FC0;
extern s16 D_800F1000;
extern s16 D_800F1002;
extern s16 D_800F1004;
extern s16 D_800F10A0;
extern s16 D_800F10A2;
extern s16 D_800F10A4;
extern s32 D_800F10D0;
extern s32 D_800F10D4;
extern s32 D_800F10E0;
extern s32 D_800F10E4;
extern s32 D_800F10E8;
extern s32 D_800F10EC;
extern s32 D_800F10F0;
extern s32 D_800F10F4;
extern s32 D_800F10F8;
extern s32 D_800F10FC;
extern s32 D_800F1100;
extern s32 D_800F1104;
extern s32 D_800F1108;
extern s32 D_800F110C;
extern s32 D_800F1110;
extern s32 D_800F1114;
extern s32 D_800F1118;
extern s32 D_800F111C;
extern s32 D_800F1138;
extern s32 D_800F1140;
extern s32 D_800F1144;
extern s32 D_800F1148;
extern u8 D_800F1150;
extern s32 D_800F1178;
extern s32 D_800F117C;
extern s32 D_800F1180;
extern s32 D_800F1198;
extern s32 D_800F119C;
extern s32 D_800F11A0;
extern s32 D_800F1850;
extern s32 g_anim_func_table;
extern u8 D_800F74A4;
extern u8 D_800F74A5;
extern u8 D_800FB534;
extern u8 D_800FB535;
extern s16 D_800FF558;
extern s16 D_800FF55A;
extern s16 D_800FF55C;
extern s16 D_800FF55E;
extern s16 D_800FF560;
extern s16 D_800FF562;
extern s16 D_800FF564;
extern s16 D_800FF566;
extern s16 D_800FF568;
extern s32 D_800FF570;
extern s32 D_800FF574;
extern u8 D_800FF639;
extern u8 D_80101DF1;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s32 D_80103624;
extern s32 D_800EFC40;
extern s32 D_800EFB40;
extern s32 D_800EFC4C;
extern s32 D_800EFB4C;
extern s32 D_800EFC3C;
extern s32 D_800EFB3C;
extern s32 D_800EFB3C[];
extern s32 D_800EFC3C[];
extern s32 func_80036EA8();
extern s32 func_80036F28();
extern s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);
extern s32 D_800158B4;
extern s32 D_800A3404;
extern s32 D_800A340C;
extern s32 D_800EFC58;
extern s32 D_800EFB58;
extern s32 D_800EFC48;
extern s32 D_800EFB48;
extern s32 func_80036EA8(s32, s32);
extern s32 func_80036F28(s32);
extern void obj_InitPair(void);
extern void replay_camera_Init(s32, s32);
extern s32 D_800EFC5C;
extern s32 D_800EFB5C;
extern void title_mv_exec2(s32);
extern void saFidLoad(s32, s32);
extern u32 D_800EFC38[];
extern u32 D_800EFB38[];
extern u8 D_8009AD18[];
extern void func_80087F64(s32);
extern s32 D_8009AD1C;
extern s32 func_80085F98();
extern s32 func_80085EE4(s16);
extern s32 func_80085E4C(s16, s16);
extern s32 func_80085FB8();
extern s32 md_game_check_change_main_mode_katinuki(s16);
extern s32 func_80087F64();
extern s32 tslCDFileRead();
extern s32 coli_CheckBukiPreHit_800880B8();
extern s32 func_8008AD64();
extern s32 func_8008ADC4();
extern s32 func_8008AE24();
extern s32 func_8008AEB0();
extern s32 *D_800EFC38[];
extern s32 func_80087F64(s32);
extern s16 coli_CheckBukiPreHit_800880B8(s32, s32, s32);
extern s16 tslCDFileRead(s16);
extern s32 D_8009AA70;
extern u8 D_800EFB7C;
extern u8 D_800EFB7D;
extern s32 func_80073728();
extern s32 D_8009B2C8;
extern s32 D_8009B340;
extern s32 D_8009B358;
    extern s32 func_80079154(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
extern s16 D_8009B488;
extern s8 D_8009B48E;
extern s32 D_8009B610;
extern s32 D_8009B634;
extern s32 D_8009B63C;
extern s32 D_8009B660;
extern s32 D_8009B670;
extern s32 D_8009B678;
extern u8 D_800A327C[8];
extern u8 D_800A3284[8];
extern s32 D_800A3278;
extern s32 D_8009B698;
extern s32 D_8009B6B0;
extern s32 D_800F7438;
extern s32 initDrawArea(s32, u16 *);
extern s32 initPolyG4(s32);
extern s32 gpu_SetSemiTransp(s32, s32);
extern s32 func_8007352C(s32);
extern s32 D_8009B6F0;
extern s32 D_8009B6FC;
extern s32 D_8009B708;
extern s32 D_8009B758;
extern u8 D_800A3294[8];
extern u8 D_800A329C[8];
extern u8 D_800A32A4[8];
extern u8 D_800A32AC[8];
extern s32 saMotionSet();
extern s32 func_8007352C();
extern s32 D_8009B7AC;
extern s32 D_8009B7B8;
extern s32 D_8009B7C4;
extern u16 D_8009B850;
extern u8 D_8009BD24;
extern s32 D_800A328C;
extern s32 D_8009B3B0;
extern s32 D_8009B770;
extern s32 D_8009B7A0;
extern s32 D_8009B7D0;
extern s32 D_8009B7D8;
extern s32 D_8009B800;
extern s32 D_8009B820;
extern s32 D_8009B840;
extern u16 D_800A32B6;
extern u16 D_800A32B4;
extern s32 D_8009B0C0;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
extern s32 gpu_SetSemiTransp(void *, s32);
extern s32 initTexPage(void *, s32, s32, s32, s32);
extern s32 initTile(void *);
extern s32 ot_Link(s32, void *);
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    extern s32 D_800A346C;
    extern s32 D_800A3470;
    extern s32 D_800A3474;
    extern void func_80061FAC(s32, s32, s32);
extern s32 D_800F10D0[];
extern u8 D_800F1150[];
extern s16 D_800A345E;
extern s16 D_800A345C;
extern s32 D_800A3458;
extern s32 D_800A3454;
extern s32 D_800A3450;
extern s32 D_800A3460;
extern s32 D_800A3444;
extern s32 D_800A3448;
extern s32 D_800A3420;
extern s32 D_800A3424;
extern volatile s32 D_800A37D4;
extern s32 D_800A3720;
extern s32 D_800A3468;
extern s32 D_800A346C;
extern s32 D_800A3470;
extern s32 D_800A3474;
extern s32 D_800A3480;
extern s32 D_800A3484;
extern s32 D_800A3488;
extern s32 D_800A348C;
extern s32 D_800A3490;
extern s32 D_800A3494;
extern s32 D_800A3498;
extern s32 D_800A349C;
extern s32 D_800A34A0;
extern s32 D_800A34A4;
extern s32 D_800A34A8;
extern s32 D_800A34AC;
extern s32 D_800A34B0;
extern s32 D_800A34B4;
extern s32 D_800A34B8;
extern s32 D_800A34BC;
extern s32 D_800A34C0;
extern s32 D_800A34C4;
extern s32 D_800A34C8;
extern s32 D_800A34CC;
extern s32 D_800A34D0;
extern s32 D_800A34D4;
extern s32 D_800A34D8;
extern s32 D_800A34DC;
extern s32 D_800A34E0;
extern s32 D_800A34E4;
extern s32 D_800A34E8;
extern s32 D_800A34EC;
extern s32 func_80041E10();
extern s32 leaf_muki_awase_800421A4();
extern s32 func_80060B70();
extern s32 func_80060E38();
extern s32 debug_printf(s32 *, s32);
extern s32 D_800158E0;
extern s32 D_800A32BC;
extern s32 D_800A3464;
extern s32 *D_800A3468;
extern s32 D_800A37D4;
extern u8 D_800F116A;
extern s32 D_800F116C;
extern volatile u8 D_800F1159;
extern u8 D_800F1154;
extern u8 D_800F115B;
extern u8 D_800F115C;
extern u8 D_800F1160[];
extern u8 D_800F1152[];
extern s32 D_800F1158;
extern u8 D_800F1154[];
extern u8 D_800F1151;
extern u8 D_800F1164[];
extern s16 D_800A34F0;
extern s16 D_800A34F2;
extern u8 D_800F1168[];
extern u8 D_8009BB74[];
extern s32 D_800A32B8;
    extern s32 D_800A3460;
    extern volatile s32 D_800A347C;
    extern volatile s32 D_800A3478;
    extern s16 D_800F0C04;
    extern s32 D_800F0FB8;
    extern s32 D_800F0FBC;
    extern s32 D_800F0FC0;
    extern s32 D_800F10A0;
    extern s32 D_800F10A2;
    extern s32 D_800F10A4;
    extern s32 D_800F1138;
extern volatile s32 D_800A347C;
extern s32 func_80079154(void);
extern void *D_800A347C;
extern volatile s32 D_800F0CA0;
extern volatile s32 D_800F0CA4;
extern volatile s32 D_800F0CA8;
extern volatile s32 D_800F10E0;
extern volatile s16 D_800F0BA8;
extern volatile s32 D_800F0CAC;
extern volatile s32 D_800F0CB0;
extern volatile s32 D_800F0CB4;
extern volatile s32 D_800F10E4;
extern volatile s16 D_800F0BAA;
extern volatile s32 D_800F0CB8;
extern volatile s32 D_800F0CBC;
extern volatile s32 D_800F0CC0;
extern volatile s32 D_800F10E8;
extern volatile s16 D_800F0BAC;
extern s16 D_800F0BAE;
extern s16 D_800F0BB0;
extern volatile s32 D_800A3468;
extern volatile s32 D_800F0CDC;
extern volatile s32 D_800F0CE0;
extern volatile s32 D_800F0CE4;
extern volatile s32 D_800F10FC;
extern volatile s16 D_800F0BB2;
extern volatile s16 D_800A3440;
extern volatile s32 D_800F0CE8;
extern volatile s32 D_800F0CEC;
extern volatile s32 D_800F0CF0;
extern volatile s32 D_800F1100;
extern volatile s16 D_800F0BB4;
extern volatile s32 D_800F0CF4;
extern volatile s32 D_800F0CF8;
extern volatile s32 D_800F0CFC;
extern volatile s32 D_800F1104;
extern volatile s16 D_800F0BB6;
extern volatile s32 D_800F0D18;
extern volatile s32 D_800F0D1C;
extern volatile s32 D_800F0D20;
extern volatile s32 D_800F1108;
extern volatile s16 D_800F0BBC;
extern volatile s32 D_800F0D24;
extern volatile s32 D_800F0D28;
extern volatile s32 D_800F0D2C;
extern volatile s32 D_800F110C;
extern volatile s16 D_800F0BBE;
extern u16 D_800F0BC0;
extern u16 D_800F0BC4;
extern u16 D_800F0BC2;
extern u16 D_800F0BC6;
extern volatile s32 D_800F0D60;
extern volatile s32 D_800F0D64;
extern volatile s32 D_800F0D68;
extern volatile s32 D_800F1118;
extern volatile s16 D_800F0BC8;
extern volatile s32 D_800F0D6C;
extern volatile s32 D_800F0D70;
extern volatile s32 D_800F0D74;
extern volatile s32 D_800F111C;
extern volatile s16 D_800F0BCA;
extern s16 D_800F0BA8;
extern s16 D_800F0BAA;
extern s16 D_800F0BAC;
extern s32 *D_800A3484;
extern s16 D_800F0BB4;
extern s16 D_800F0BB6;
extern s16 D_800F0BBC;
extern s16 D_800F0BBE;
extern s32 motion_SetExMotion(s32);
extern s16 D_800F0BC4;
extern s16 D_800F0BC8;
extern s16 D_800F0BCA;
extern s32 D_800F10D8;
extern s32 D_800F10DC;
extern s32 D_800F1120;
extern s32 D_800F1124;
extern s32 D_800F1128;
extern s32 D_800F112C;
extern s32 D_800F1130;
extern s32 D_800F1134;
    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3724;
    extern s32 D_800A34E4;
    extern s32 D_800A34E8;
    extern s32 D_800A374C;
extern s32 D_800A3500;
extern s32 D_800A351C;
extern s32 D_800A3524;
extern s32 D_800A34FC;
extern s32 D_800A372C;
extern s32 D_800A3518;
extern u32 D_800A34F8;
extern s16 D_800A3528;
extern s16 D_800A3512;
extern s16 D_800A3510;
extern s16 D_800A350E;
extern s16 D_800A350C;
extern u8 D_800A32C0[8];
extern s32 snd_StopAll(void);
extern s32 func_8006E950(s32, s32);
extern s32 efc_rob_Close(s32);
extern s32 *func_8006E49C(s32, s32);
extern s32 gpu_DrawSync(s32);
extern s32 func_8007B6C8(u8 *, s32, s32);
extern s32 func_8005C650();
extern void func_80069E18(s32, s32);
extern s32 func_8006E390();
extern s32 D_800A3514;
extern u32 D_800A32D0;
extern void initTile(u8 *p);
extern void gpu_SetSemiTransp(u8 *p, s32 semi);
extern void ot_Link(u32 *ot, u32 *prim);
extern void initPolyF4(u8 *p);
extern void func_80069A8C(u8 *p);
extern s32 func_80073728(s32, s32);
extern u8 g_disp_fb_base;
extern s32 D_800A350C;
extern s32 D_800A36E0;
extern s32 D_800A36E4;
extern void initTexPage();
extern void ot_Link();
extern void func_8006BB68(s32);
extern s32 func_8006B92C();
extern void saTan4GaugeMain(s32);
extern s32 func_8006CFBC(s32);
extern void func_8006CCC8(s32, s32, s32);
extern void *D_800A3524;
extern u16 D_800A3528;
extern s32 func_8006E390(s32, s32);
extern s32 func_80069AE4(s32, s32, s32);
extern void func_8006D3DC(s32);
extern s32 func_8006D5D4(s32, u32);
extern s32 func_8005C6D0(void);
extern s32 D_800A352C;
extern s32 func_800692C0();
extern void func_8006DD94(s32);
extern s32 func_8006DF68();
extern u8 D_800A32D8[8];
extern void func_80036F28(s32);
extern void game_FrameLoop(void);
extern void gpu_InitDrawEnv(s32, s32, s32, s32, s32);
extern void gpu_InitDispEnv(s32, s32, s32, s32, s32);
extern void gpu_LoadImage(u8 *, s32);
extern void func_8007B4D0(s32, s32, s32, s32);
extern void func_8007B9B0(s32);
extern void func_8007BC08(s32);
extern void gpu_SetDispMask(s32);
extern void gpu_DrawSync(s32);
extern u8 D_800A32E0[8];
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern s32 D_800A3520;
extern s32 D_800A35AC;
extern s32 D_800A3548;
extern s32 D_800A354C;
extern s16 D_800A3580;
extern s32 D_800A35A0;
extern void *D_800A35A4;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern s32 D_800A35C0;
extern void *D_800A35C4;
extern s16 D_800A3570;
extern u16 D_800A3578;
extern s16 D_800A3584;
extern volatile u16 D_800A3550;
extern s32 initTile(s32);
extern s32 D_800A3558;
extern s32 D_800A35B0;
extern s32 func_8007352C(s32 *prim);
extern s32 func_80069898(s32 a0, s32 *p, s32 mode);
extern void motion_ShiftControl(s32 a0, s32 *prim);
extern void func_8006ECF4(s32);
extern void func_80072E10(s32);
extern void saTan3GaugeMain_80073200(s32);
extern u8 D_8009BC7C[];
extern s32 gpu_SetSemiTransp(GameObj *, s32);
extern s32 initPolyG4(GameObj *);
extern s32 ot_Link(s32, GameObj *);
extern s32 func_80073060(s32);
extern s32 func_80072CD4(s32, GameObj *);
extern s32 initSprt(s32, s16);
extern s32 gpu_SetRawTexture(s32, s32);
extern s32 DispSleepMenuTex(s32 *, s32);
extern s32 gpu_CalcClut(u16, u16);
extern s32 D_800159A0;
extern void initPolyF4(s32);
extern s32 func_80069A8C(s32);
extern s32 initTile(GameObj *);
extern u8 *D_800A36A0;
extern s16 D_800A35D0;
extern s32 func_8005C650(s32, s32, s32);
extern s32 D_800A35D8;
extern s8 D_800A35DC;
extern s32 func_8007B844(s32, s32);
extern s32 func_8006E950(s32, s32 *);
extern s32 func_80076FF8(s32 *);
extern u8 *func_8006E49C(s32, s32);
extern s32 *func_80077098(s32);
extern void func_80077374(s32, s32 *);
extern s32 *D_800A36A0;
extern s32 D_800A35E0;
extern s32 D_800A35E8;
extern u8 D_8009BD24[];

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

extern s32 D_800A35E4;
extern u8 D_8009BD3B;
extern u8 D_8009BD3C;
extern u8 D_8009BD3D;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern u8 D_8009BD43;
extern s32 motion_CalcMotion_80077B30(s32, s32);
extern s32 saTan2InfoInit_8006C1FC_local(s32, s32);
extern void cpu_reset_dir(void);
extern s32 motion_shift_check_80077B30(s32, s32);
s32 func_80077B30(s32 arg0, s32 arg1) {
    extern s32 func_8006B898(s32, s32);
    extern s32 saTan2InfoInit_8006C1FC(s32, s32);
    extern s32 func_8006D338(s32, s32);
    s32 s2;
    s32 result;

    if ((u32)D_800A35E4 >= 6) goto end;
    switch (D_800A35E4) {
    case 0:
        s2 = 0;
        result = func_8006B898(arg0, arg1);
        switch (result) {
        case 1: s2 = -1; goto end;
        case 2: D_800A35E4 = 1; goto end;
        case 3:
            D_8009BD3B = D_8009BD41;
            D_8009BD3C = D_8009BD42;
            D_8009BD3D = D_8009BD43;
            D_800A35E4 = 4;
            goto end;
        }
        goto end;
    case 1:
        s2 = 0;
        result = saTan2InfoInit_8006C1FC(arg0, arg1);
        if (result == 1) { D_800A35E4 = 0; goto end; }
        if (result == 2) { D_800A35E4 = result; goto end; }
        if (result == 3) { D_800A35E4 = result; }
        goto end;
    case 2:
        s2 = 2;
        goto end;
    case 3:
        s2 = 3;
        goto end;
    case 4:
        cpu_reset_dir();
        D_800A35E4 = D_800A35E4 + 1;
        /* fall through */
    case 5:
        s2 = 0;
        result = func_8006D338(arg0, arg1);
        if (result == 1) {
            D_800A35E4 = 0;
            D_8009BD41 = D_8009BD3B;
            D_8009BD42 = D_8009BD3C;
            D_8009BD43 = D_8009BD3D;
            goto end;
        }
        if (result == -1) { D_800A35E4 = 0; }
        goto end;
    }
end:
    return s2;
}
extern s32 D_8009BD24;
s32* saTan2GaugeInit_80077D00(void) {
    return &D_8009BD24;
}
void func_8006920C(s32*, s32);
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
extern s32 D_800A35F4;
extern s32 D_800A35F0;
extern s32 D_800A35F8;
extern s32 D_800A35FC;
extern s32 D_800A3600;
s32 func_80077D74(s32 a0) {
    return D_800A35F4 + a0 * 44;
}
void func_80077D94(s32 *arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80077D94.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
s32 camera_get_rot_normal_rad(s32 arg0) {
    s32 s0;
    s32 r;

    func_8007B844(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A35F4 = arg0;
    D_800A35F8 = s0;
    func_8006E950(0x32, s0);
    r = func_80077D10(s0);
    func_8006E49C(r, D_800A35F4);
    D_800A35FC = 0;
    D_800A35F0 = 0;
    D_800A3600 = 0;
    return 1;
}
extern s32 *func_80077D74(s32);
extern void func_80077D94(s32 *);
extern s32 D_800A35F0;
extern s32 *D_800A35F8;
extern s32 D_800A35FC;
typedef struct {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
} S855C;
s32 func_8007855C(s32 arg0) {
    S855C s;
    s32 *p;
    s32 c;
    c = D_800A35FC + 1;
    D_800A35FC = c;
    p = func_80077D74(c & 1);
    s.sp14 = p[0];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    if (D_800A35F0 > 0 && (arg0 & 0x40)) {
        return 1;
    }
    func_80077D94(&s.sp10);
    {
        s32 *p_struct = *(s32 **)((s32)D_800A35F8 + 0x34);
        s32 new_val = D_800A35F0 + 1;
        int cond = new_val < *(s16 *)((s32)p_struct + 0xA);
        D_800A35F0 = new_val;
        return cond ? 0 : 1;
    }
}
s32 func_80078628(s32 *a0) {
    return a0[1];
}
extern s32 D_800A360C;
s32 func_80078634(s32 a0) {
    return D_800A360C + a0 * 44;
}
extern s32 saMotionSet(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 D_800A374C;

typedef struct {
    s32 a;       /* sp18 - 0x00 */
    s32 b;       /* sp1C - 0x04 */
    s32 c;       /* sp20 - 0x08 */
    s32 d;       /* sp24 - 0x0C - unused */
    s32 e;       /* sp28 - 0x10 */
    s32 f;       /* sp2C - 0x14 */
    s32 g;       /* sp30 - 0x18 */
    s32 h;       /* sp34 - 0x1C */
    s32 i;       /* sp38 - 0x20 - unused */
    s32 j;       /* sp3C - 0x24 - unused */
    u8 cd_flag;  /* sp40 - 0x28 */
    u8 r;        /* sp41 - 0x29 */
    u8 g_;       /* sp42 - 0x2A */
    u8 b_;       /* sp43 - 0x2B */
} S78654;

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 v;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s32 v_copy;
            s.cd_flag = 1;
            v = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v));
            if ((s32)(v << 16) < 0) {
                v_copy = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) v_copy));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        initTexPage(arg0[5], 1, 0, saMotionSet(s.a, zero), 0);
        ot_Link(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    initTexPage(arg0[5], 1, 0, saMotionSet(s.a, zero), 0);
    ot_Link(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
extern s32 D_800A3610;
extern s32 D_800A3614;
extern s32 D_800A3304;
extern s32 D_800A3608;
extern s32 func_80078628(s32);
s32 func_80078824(s32 arg0) {
    s32 s0;
    s32 r;

    func_8007B844(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A360C = arg0;
    D_800A3610 = s0;
    func_8006E950(0x5F, s0);
    r = func_80078628(s0);
    func_8006E49C(r, D_800A360C);
    D_800A3304 = 0;
    D_800A3608 = 0;
    D_800A3614 = 0;
    disp_SetFramebufferMode(1, 0, 0, 0);
    return 1;
}
extern s32 D_800A3304;
extern s32 D_800A3608;
s32 *func_80078634(s32);
void func_80078654(s32 *);
s32 func_800788B0(void) {
    s32 buf[8];
    s32 *v0;
    D_800A3304++;
    v0 = func_80078634(D_800A3304 & 1);
    buf[0] = v0[0];
    buf[2] = v0[2];
    buf[3] = v0[4];
    buf[4] = v0[3];
    buf[5] = v0[5];
    buf[6] = v0[6];
    buf[7] = v0[7];
    func_80078654(buf);
    D_800A3608++;
    return D_800A3608 >= 0xB40;
}

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_Exec\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel bios_Exec\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_Exec */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios__bu_init_A0\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x70\n"
    "endlabel bios__bu_init_A0\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios__bu_init_A0 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_SetMem\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9F\n"
    "endlabel bios_SetMem\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SetMem */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_OpenEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x8\n"
    "endlabel bios_OpenEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_OpenEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_CloseEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9\n"
    "endlabel bios_CloseEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_CloseEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_TestEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xB\n"
    "endlabel bios_TestEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_TestEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_EnableEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xC\n"
    "endlabel bios_EnableEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_EnableEvent */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel EnterCriticalSection\n"
    "    addiu  $a0,$zero,1\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel ExitCriticalSection\n"
    "    addiu  $a0,$zero,2\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789D8\n"
    "    addu  $v0,$sp,$zero\n"
    "    jr  $ra\n"
    "    addu  $sp,$a0,$zero\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789D8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_FileOpen_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x32\n"
    "endlabel bios_FileOpen_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileOpen_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_FileRead_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x34\n"
    "endlabel bios_FileRead_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileRead_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_FileWrite_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x35\n"
    "endlabel bios_FileWrite_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileWrite_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_FileClose_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x36\n"
    "endlabel bios_FileClose_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileClose_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_FormatDevice_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x41\n"
    "endlabel bios_FormatDevice_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FormatDevice_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_firstfile_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x42\n"
    "endlabel bios_firstfile_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_firstfile_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_nextfile_B\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel bios_nextfile_B\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_nextfile_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_ChangeClearPad\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x5B\n"
    "endlabel bios_ChangeClearPad\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_ChangeClearPad */
s32 func_80078A68(s32 arg0, s32 arg1, s32 arg2) {
    s32 a3;
    s32 t0;
    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
    a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    base = (t0 * 0x10) + D_8009BD6C;
    *((s16 *) (base + 4)) = 0;
    *((s16 *) (base + 8)) = (s16) arg1;
    if (((u32) t0) < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        v0 = arg2 & 0x1000;
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else {
        v0 = arg2 & 0x1000;
        if (t0 == 2) {
            ;
            if (!(arg2 & 1)) {
                a3 = 0x248;
            }
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    *((s16 *) (((t0 * 0x10) + D_8009BD6C) + 4)) = a3;
    return 1;
}
s32 func_80078B04(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    return *(volatile u16 *)(D_8009BD6C + v * 0x10);
}
s32 func_80078B3C(s32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] | (&D_8009BD70)[v];
    return v < 3;
}
s32 func_80078B70(s32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] & ~(&D_8009BD70)[v];
    return 1;
}
s32 func_80078BA8(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    *(volatile u16 *)(D_8009BD6C + v * 0x10) = 0;
    return 1;
}
extern s32 D_8009BD80;
void func_80078BE0(s32 a0) {
    D_8009BD80 = a0;
}
extern s32 D_8009BD80;
s32 func_80078BF0(void) {
    return D_8009BD80;
}
void func_800790C0(void);
void EnterCriticalSection(void);
void func_80078F88(void);
void ExitCriticalSection(void);
void bios_ChangeClearPad(s32);
s32 func_80078DA0(void);
void bios_OutdatedPadInitAndStart(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C00(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    EnterCriticalSection();
    func_80078F88();
    ExitCriticalSection();
    bios_ChangeClearPad(0);
    func_80078DA0();
    bios_OutdatedPadInitAndStart(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_800790C0(void);
void EnterCriticalSection(void);
void func_80078F88(void);
void ExitCriticalSection(void);
void bios_ChangeClearPad(s32);
s32 func_80078DA0(void);
void bios_InitPad(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C9C(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    EnterCriticalSection();
    func_80078F88();
    ExitCriticalSection();
    bios_ChangeClearPad(0);
    func_80078DA0();
    bios_InitPad(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void bios_StartPad(void);
void bios_ChangeClearPad(s32);
void func_80078F60(void);
void func_80078D38(void) {
    bios_StartPad();
    bios_ChangeClearPad(0);
    func_80078F60();
}
extern s32 D_8009BD80;
void func_80078F74(void);
void bios_StopPad(void);
s32 func_80078E20(void);
void pad_Init(void) {
    func_80078F74();
    bios_StopPad();
    func_80078E20();
    D_8009BD80 = 0;
}
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void bios_SysDeqIntRP(s32, u32 *);
extern void bios_SysEnqIntRP(s32, u32 *);
extern void func_80078E58(void);
extern s32 func_80078EC0(void);
extern u32 D_800F183C;
extern u32 D_800F1840;
extern u32 D_800F1838;
extern u32 D_800F1844;
s32 func_80078DA0(void) {
    u32 *v1 = &D_800F183C;
    u32 *s0 = v1 - 1;
    EnterCriticalSection();
    *v1 = (u32)func_80078E58;
    D_800F1840 = (u32)func_80078EC0;
    D_800F1838 = 0;
    D_800F1844 = 0;
    bios_SysDeqIntRP(1, s0);
    bios_SysEnqIntRP(1, s0);
    ExitCriticalSection();
    return 1;
}
void EnterCriticalSection(void);
void bios_SysDeqIntRP(s32, s16*);
void ExitCriticalSection(void);
extern s16 D_800F1838;
s32 func_80078E20(void) {
    EnterCriticalSection();
    bios_SysDeqIntRP(1, &D_800F1838);
    ExitCriticalSection();
    return 1;
}
s32 func_80078E58(void) {
    volatile s32 i;
    s32 pad[2];
    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;
    i = 10;
    i = i - 1;
    if (i != -1) {
        do {
            i = i - 1;
        } while (i != -1);
    }
    return 0;
}
s32 func_80078EC0(void) {
    register s32 *p asm("$3") = (s32 *)D_8009BD88;
    register volatile s32 ret asm("$2") = 0;
    if ((p[1] & 1) == 0) goto end;
    if ((p[0] & 1) != 0) {
        ret = 1;
        goto end;
    }
    ret = 1;
    ret = 0;
end:
    return ret;
}
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_InitPad\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x12\n"
    "endlabel bios_InitPad\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_InitPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_StartPad\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x13\n"
    "endlabel bios_StartPad\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_StartPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_StopPad\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x14\n"
    "endlabel bios_StopPad\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_StopPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_OutdatedPadInitAndStart\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x15\n"
    "endlabel bios_OutdatedPadInitAndStart\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_OutdatedPadInitAndStart */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_SysEnqIntRP\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x2\n"
    "endlabel bios_SysEnqIntRP\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SysEnqIntRP */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel bios_SysDeqIntRP\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x3\n"
    "endlabel bios_SysDeqIntRP\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SysDeqIntRP */
extern void (*jtbl_800A3620)(void);
extern void (*jtbl_800A3624)(void);
/* func_80078F60 / func_80078F74: 5-insn bare tail-jump trampolines
   (lui/lw/nop/jr/nop) through the jtbl_800A3620 / jtbl_800A3624 function
   pointers that the Pad-init wrapper func_80078F88 installs at runtime. GCC
   2.7.2 has no MIPS sibling-call optimization, so no pure-C `(*fp)()` form
   emits a frameless `jr $t1` (it always builds a stack frame + jalr + jr $ra).
   Hand-coded canonical asm; user-authorized 2026-06-12. */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F60\n"
    "    lui  $t1,%hi(jtbl_800A3620)\n"
    "    lw  $t1,%lo(jtbl_800A3620)($t1)\n"
    "    nop\n"
    "    jr  $t1\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F74\n"
    "    lui  $t1,%hi(jtbl_800A3624)\n"
    "    lw  $t1,%lo(jtbl_800A3624)($t1)\n"
    "    nop\n"
    "    jr  $t1\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F88\n"
    "    lui  $at,%hi(D_800A3618)\n"
    "    sw  $ra,%lo(D_800A3618)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $at,%hi(jtbl_800A3620)\n"
    "    addi  $v1,$v0,2180\n"
    "    sw  $v1,%lo(jtbl_800A3620)($at)\n"
    "    lui  $at,%hi(jtbl_800A3624)\n"
    "    addi  $v1,$v0,2196\n"
    "    addiu  $t1,$zero,11\n"
    "    sw  $v1,%lo(jtbl_800A3624)($at)\n"
    ".L80078FC4:\n"
    "    addiu  $t1,$t1,-1\n"
    "    sw  $zero,1428($v0)\n"
    "    bnez  $t1,.L80078FC4\n"
    "    addiu  $v0,$v0,4\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3618)\n"
    "    lw  $ra,%lo(D_800A3618)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078FF0\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x44\n"
    "    nop\n"
    "    lui   $t1, %hi(D_800A362C)\n"
    "    lw    $t1, %lo(D_800A362C)($t1)\n"
    "    addiu $sp, $sp, -24\n"
    "    sw    $ra, 20($sp)\n"
    "    jalr  $t1\n"
    "    nop\n"
    "    lw    $ra, 20($sp)\n"
    "    addiu $sp, $sp, 24\n"
    "    jr    $ra\n"
    "    nop\n"
    "endlabel func_80078FF0\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80079028\n"
    "    lui  $at,%hi(D_800A3628)\n"
    "    sw  $ra,%lo(D_800A3628)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $t2,%hi(func_800790A4)\n"
    "    lui  $t1,%hi(D_800790B4)\n"
    "    lui  $at,%hi(D_800A362C)\n"
    "    addi  $v1,$v0,1952\n"
    "    addiu  $t2,$t2,%lo(func_800790A4)\n"
    "    addiu  $t1,$t1,%lo(D_800790B4)\n"
    "    sw  $v1,%lo(D_800A362C)($at)\n"
    ".L80079064:\n"
    "    lw  $v1,0($t2)\n"
    "    addiu  $t2,$t2,4\n"
    "    sw  $v1,984($v0)\n"
    "    addiu  $v0,$v0,4\n"
    "    bne  $t2,$t1,.L80079064\n"
    "    sw  $v1,1244($v0)\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3628)\n"
    "    lw  $ra,%lo(D_800A3628)($ra)\n"
    "    lui  $v0,%hi(D_800A362C)\n"
    "    lw  $v0,%lo(D_800A362C)($v0)\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800790A4\n"
    "    and  $v0,$v0,$s5\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800790C0\n"
    "    lui  $at,%hi(D_800A3638)\n"
    "    sw  $ra,%lo(D_800A3638)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    addiu  $t2,$zero,9\n"
    "    addi  $v1,$v0,1580\n"
    ".L800790E8:\n"
    "    addiu  $t2,$t2,-1\n"
    "    sw  $zero,0($v1)\n"
    "    bnez  $t2,.L800790E8\n"
    "    addiu  $v1,$v1,4\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3638)\n"
    "    lw  $ra,%lo(D_800A3638)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800790C0 */
u8* bb2_memcpy(u8 *dst, u8 *src, s32 len) {
    u8 *ret;
    if (!dst) {
        return 0;
    }
    ret = dst;
    while (len > 0) {
        *dst = *src;
        src++;
        len--;
        dst++;
    }
    return ret;
}
extern u32 D_800F1848;
s32 func_80079154(void) {
    D_800F1848 = D_800F1848 * 0x41C64E6D + 0x3039;
    return (D_800F1848 >> 16) & 0x7FFF;
}
extern s32 D_800F1848;
void func_80079184(s32 a0) {
    D_800F1848 = a0;
}
u8 *func_80079194(u8 *a0, u8 *a1) {
    u8 *v1;
    if (!a0) {
        return 0;
    }
    if (!a1) {
        return 0;
    }
    v1 = a0;
    while ((*a0++ = *a1++) != 0) {
    }
    return v1;
}
s32 func_800791D8(u8 *a0) {
    s32 v1 = 0;
    if (!a0) {
        return 0;
    }
    while (*a0++ != 0) {
        v1++;
    }
    return v1;
}
void debug_printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;
    ap[1] = a;
    ap[2] = b;
    ap[3] = c;
    func_80079244(1, fmt, ap + 1);
}
void func_80079244(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80079244.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 D_8009BD8D;
u8 func_800798CC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}
extern u8 D_8009BD8D;
u8 func_800798FC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
u8 *func_8007992C(u8 *buf, s32 ch, s32 len) {
    if (buf == 0) return 0;
    if (len <= 0) return 0;
    len--;
    goto check;
found:
    return buf - 1;
check:
    if (len < 0) return 0;
    ch &= 0xFF;
loop:
    if (*buf++ == ch) goto found;
    --len;
    if (len >= 0) goto loop;
    return 0;
}
void bios_FileWrite_B(s32, u8 *, s32);
void func_8007997C(s8 arg0) {
    u8 sp10;
    s32 temp_a0;

    sp10 = arg0;
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 == 9) goto loop;
    if (temp_a0 == 0xA) {
        func_8007997C(0xD);
        D_800F1850 = 0;
        goto tail;
    }
    goto def;
loop:
    func_8007997C(0x20);
    if ((D_800F1850 & 7) == 0) return;
    goto loop;
def:
    if ((&D_8009BD8D)[temp_a0] & 0x97) {
        D_800F1850 += 1;
    }
tail:
    bios_FileWrite_B(1, &sp10, 1);
}
void func_80079A30(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80079A30.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
