#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern s16 D_80101E62;
extern s16 D_80101E64;
extern s16 D_80101E6A;
extern u32 D_800905F8;
extern s32 D_800F33D8;
extern s16 D_80101E68;
extern u8 D_80106A70;
extern u8 D_80106A71;
extern u8 D_80106A72;
extern s16 D_8008F12C;
extern u32 D_8008EC38;
extern u32 D_800A378C;
extern u16 D_800A38C6;
extern s16 D_8010278C;
extern s16 D_8010278E;
extern s32 D_800A37B8;
extern s16 D_800A3834;
extern u8 D_800A3912;
extern u8 D_800A3913;
extern u8 D_800A3914;
extern u32 D_800A3220;
extern u32 D_80090178;
extern u32 D_80101E3C;
extern u32 D_80101E44;
extern u8 D_800A3928;

/* Extern function declarations */
extern void func_80023CB4(s32, s32);
extern s32 func_80037110(s32);
extern void func_80036EC0(void);
extern void func_80036F40(void);
extern void gpu_EnableDisplay(void);
extern void func_800194F4(void);
extern void func_80045188(void);
extern s32 func_8008C464(s32, s32, s32);
extern void func_8003A39C(void);
extern void func_80080148(void);
extern void func_8007FF7C(void);
extern void func_800828CC(s32);
extern void spu_SetVolume(s32, s32, s32);
extern void func_80085448(s32, s32, s32);
extern void func_8007B600(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern void func_80079A30(s32 *, s32, s32, s32);
extern void func_80078A28(s32 *);
extern void func_8003A728(s32 *);
extern void gpu_InitDisplay(void);
extern void func_80061178(void);
extern void gpu_DisableDisplay(void);
extern s32 func_800371E8(s16);
extern void func_800450BC(s32, s32);
extern u16 D_800F6656;
extern u8 D_80104E88;
extern s16 D_80101F4C;
extern s16 D_80101F4E;
extern s16 D_80102788;
extern s16 D_8010278A;
extern s32 D_80102790;
extern s32 D_80102794;
extern s32 D_80102798;
extern s32 D_8010279C;
extern s16 D_80106A7A;
extern u8 D_80106A82;
extern u8 D_8008D9EC;
extern u8 D_800A37B0;
extern s32 D_800A3894;
extern s32 D_800A3878;
extern u8 D_800A390C;
extern u8 *D_800A385C;
extern u8 D_800A3817;
extern u8 D_800A3929;
extern s16 D_800A3756;
extern u8 D_800A3918;
extern s32 D_800A3D40;
extern u32 D_800A3D30;
extern u8 D_800A3880;
extern s16 D_800A38C4;
extern u8 D_800A38C1;
extern u8 D_800A38C0;
extern s32 D_800A38A0;
extern u8 D_8010277C;
extern u8 D_800A3836;
extern u8 D_800A376A;
extern u8 D_800A36C8;
extern s16 D_80101F32;
extern s16 D_80101F26;
extern u8 D_80106A80;
extern s16 D_800A38BA;
extern s16 D_800A36F6;
extern s16 D_800A36FC;
extern s16 D_800F5344;
extern s32 D_800A388C;
extern s32 D_800A3888;
extern s32 D_800A3830;
extern s32 D_800A3864;
extern s32 D_80101E5C;
extern void func_80080240(s32);
extern void func_80035F30(s32, s32, s32, s32);
extern void func_80080390(s32, s32);
extern void func_8005B5AC(void);
extern void func_8005BF3C(void);
extern void func_8005B9C4(void);
extern void func_8005B868(void);
extern void func_800826CC(s32);
extern s32 func_800827D0(s32, s32);
extern void func_80041604(s32, s32);
extern void func_80021974(s32);
extern void func_80021A98(s32, s32, s32);
extern void special_camera_Exec(void);
extern s32 D_800100A4;
extern void func_800415C4(s32);
extern void file_ResetDmaFlag(void);
extern void func_8005B72C(void);
extern void func_80077820(s32);
extern void func_80080168(s32);
extern s32 D_80107850;
extern s32 D_80107854;
extern s32 D_80107858;
extern s16 D_80101E60;
extern s32 SpecialCam;
extern s32 D_80101E6C;
extern s32 D_80101E70;
extern s32 D_80101E7C;
extern s16 D_80101E9E;
extern s32 D_80101E78;
extern s32 D_801027B0;
extern s16 D_80101ED2;
extern s16 D_80101EDA;
extern s32 D_800A3778;
extern u8 D_800A38D4;
extern s16 D_8010262E;
extern s16 D_801021E2;
extern u8 D_800A3804;
extern s32 D_80102770;
extern s32 D_801027C0;
extern s32 D_801027D4;
extern u8 D_800A38A4;
extern s32 D_800A3894;
extern u8 D_800A3915;
extern u8 D_800A36F4;
extern s16 D_8010277A;
extern s16 D_80102778;
extern u8 D_80102784;
extern u8 D_80102780;
extern u8 D_80102781;
extern u8 D_80102785;
extern u8 D_80102786;
extern u8 D_80102787;
extern s16 D_800A38DC;
extern u8 D_800A36FA;
extern u8 D_800F6627;
extern u8 D_800F5347;
extern u8 D_800A3712;
extern u8 D_8010277D;
extern u8 D_8010277F;
extern u8 D_80102783;
extern u8 D_8010277E;
extern s16 D_80101F12;
extern s32 D_800A3860;
extern s32 D_80102760;
extern u8 D_800A36F1;
extern s16 D_800A391D;
extern s32 *func_8004153C(s32);
extern void func_800432A0(s32, s32, s32, s32, s32);
extern s32 file_GetFlag2(void);
extern void func_80041688(s32, s32);
extern void func_8003F1E4(s32);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);
extern s32 motion_SetMotion(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);
extern s32 D_800109C8;

extern s32 D_800A387C;
extern s32 D_800A38F0;
extern s16 D_800A38A8;
extern s32 D_80102764;
extern s32 D_80102768;
extern s32 D_800F6608;
extern s16 D_800F6618;
extern s16 D_800F661A;
extern s16 D_800F661C;
extern s32 D_800F6620;
extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 func_8005B9FC(s32);
extern s32 D_800A37C0;
extern s32 D_800A38B4;
extern u8 D_800A390D;
extern s32 func_80079120(s32 *, s32, s32);
extern void func_8005BA6C(s32);
extern s32 D_80104F38;
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);
extern void func_80048BA4(s32, s32, s32);
extern u8 D_80101EC8;

extern s32 func_8005B8B8(s32);
extern void func_8005B98C(s32);
extern void func_8005C6D0(void);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern u8 D_800A3906;
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern u16 D_80101E9E;
extern u8 D_8008D538;
extern u8 D_800A37A0;
extern u8 D_800A3680;
extern u8 D_800A3671;
extern u8 D_800A37B4;
extern u8 D_800A37B5;
extern u8 D_800A37B6;
extern s32 D_800A384C;
extern u8 D_8008EB1C;
extern u8 D_8008DB1C;
extern s32 D_800F5328;
extern s32 func_8007FD5C(s32, s32);
extern void DispPracticeMenuTex_A(s32 *, s32 *, s32 *, s32 *, s32, s32);
extern u8 D_8008F13C;
extern s16 D_80101E74;

extern u8 D_800900EC;
extern u8 D_800A37C6;
extern s32 D_800A37A4;
extern s32 D_800A3844;
extern u8 D_800A376C;
extern u8 D_8008D578;
extern void file_LoadOverlay(void);
extern void func_80022580(s32, s32, s32, s32, s32);
extern void func_80040510(s32, s32, s32);
extern void func_8003AE5C(s32);
extern void stage_GetDataPtr(void);
extern s16 D_800A36A4;

extern u8 D_800A3768;
extern u8 D_800A38F8;
extern void func_8005B50C(void);
extern void func_8007ABB8(s32 *, s32 *, s32, s32);
extern s32 func_8003F268(void);
extern s32 func_80052C28(s32, s32);
extern s32 func_800788B0(void);
extern void func_800372C0(void);
extern void func_800548DC(void);
extern s32 func_8005FC9C(s32, s32);
extern s32 func_80054F68(void);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void func_8003B5A4(void);
extern s32 func_8005E54C(s32, s32, s32);
extern void func_8005C650(s32, s32, s32);
extern s32 *func_80077D00(void);
extern void func_80060758(void);
extern void func_8001CD68(u8 *);
extern void func_80046BF4(s16 *, s32, s32);
extern void game_StageInit(s32);
extern s32 func_800600C8(s32, s32, s32);
extern void func_8001DA2C(void);
extern void func_8003B10C(s32);
extern void func_8005FBC8(s32, s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_8001DBE4(void);
extern void func_80041BF4(s32, s32, s32);
extern void func_8004659C(s32);
extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern void func_800342A0(void);
extern s32 func_80022408(s32 *);
extern void func_8007B664(s32 *, u16 *);
extern void func_80052BE4(u8 *);
extern void func_8003F388(s16 *);
extern void func_80037774(void);
extern void irq_DisableInterrupts(void);
extern void special_camera_get_rot_dir(s32 *);
extern void func_8007AE7C(s32);
extern void func_80078D68(void);
extern void irq_Reset(void);
extern s32 func_800789B8(void);
extern void func_80078948(s32 *, s32, s32 *);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern void func_8007B2A0(s32);
extern void func_8007B33C(s32);
extern void func_800493E4(s32);
extern s32 func_8004939C(void);
extern void func_800494D4(s32, s32);
extern u8 D_8008EB80;
extern u8 D_8008EB8E;
extern s16 D_80101EDC;
extern u8 D_8008E5CC;
extern void func_80049584(s32);
extern s32 func_800392B8(void);
extern u8 SpecialCam;
extern u8 D_8008D118;
extern u8 D_8008D518;
extern u8 D_8008D55C;
extern u16 D_8008D59E;
extern u8 D_8008D864;
extern s32 D_8008D86C;
extern s32 D_8008D88C;
extern u8 D_8008DA08;
extern s16 D_8008DA50;
extern s16 D_8008DA94;
extern s16 D_8008DAD8;
extern s32 menuDat;
extern s16 D_8008E194;
extern s16 D_8008E19E;
extern u8 D_8008E1A1;
extern u8 D_8008E338;
extern u16 D_8008E3C0;
extern u8 D_8008E5A8;
extern u8 D_8008E748;
extern u8 D_8008E75C;
extern u8 cpu_practice_honmokuroku_data_tbl;
extern u8 D_8008EA44;
extern u8 D_8008EA45;
extern s16 D_8008EAC0;
extern s16 D_8008EB04;
extern s16 D_8008EB06;
extern s16 D_8008EB08;
extern s16 D_8008EB0A;
extern s16 D_8008EB0C;
extern s32 D_8008EB10;
extern s32 D_8008EB14;
extern s32 D_8008EB18;
extern u8 D_8008EB38;
extern s16 D_8008EB54;
extern s16 single_dojo_yaburi_char_id_tbl;
extern u8 D_8008EB6C;
extern u16 D_8008EBA0;
extern s32 D_8008EBCC;
extern s32 D_8008EBE0;
extern u8 D_8008EBF4;
extern u8 D_8008EBFC;
extern u8 D_8008EBFD;
extern u8 D_8008EC30;
extern u8 D_8008F19C;
extern u8 D_8008F19D;
extern u8 D_8008F1A8;
extern u8 D_8008F204;
extern u8 D_8009016C;
extern s32 D_80090600;
extern s8 D_8008EA70;
extern s32 D_8009060C;
extern s32 D_80090604;
extern s16 D_80090608;
extern s16 D_800906A4;
extern s16 StatusUpBuf;
extern s32 D_800948C0;
extern u16 D_80094C68;
extern s16 Judge;
extern s16 D_800A310C;
extern s32 D_800A3134;
extern s32 D_800A3140;
extern u8 D_800A31DA;
extern u8 D_800A3670;
extern s16 D_800A3678;
extern s16 D_800A367A;
extern s16 D_800A367C;
extern s32 D_800A368C;
extern u8 D_800A3690;
extern s32 D_800A369C;
extern u8 D_800A36A8;
extern s32 D_800A36AC;
extern s32 D_800A36B4;
extern u8 D_800A36B9;
extern u8 D_800A36BA;
extern u8 D_800A36BB;
extern s16 D_800A36C2;
extern s32 D_800A36C4;
extern s16 D_800A36C6;
extern u16 D_800A36CA;
extern u8 D_800A36CC;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern s32 D_800A36D8;
extern u8 D_800A36E8;
extern u8 D_800A36F0;
extern u8 D_800A36F2;
extern u8 D_800A36F9;
extern s32 D_800A3708;
extern u16 D_800A3710;
extern u8 D_800A3713;
extern u8 D_800A3719;
extern u8 D_800A371A;
extern u8 D_800A371B;
extern s32 D_800A371C;
extern u8 D_800A3728;
extern s8 D_800A3748;
extern s32 D_800A374C;
extern u8 D_800A3758;
extern u8 D_800A3764;
extern u8 D_800A3769;
extern u8 D_800A376B;
extern s16 D_800A376E;
extern u8 D_800A377B;
extern u8 D_800A377C;
extern u8 D_800A3781;
extern u8 D_800A3783;
extern s32 D_800A3784;
extern u8 D_800A3788;
extern s32 D_800A3794;
extern u8 D_800A37A8;
extern u8 D_800A37BC;
extern u8 D_800A37D2;
extern u8 D_800A37D3;
extern u8 D_800A37E0;
extern u8 D_800A37E1;
extern s16 D_800A37E8;
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern u8 D_800A37F8;
extern s32 D_800A380C;
extern u8 D_800A3816;
extern s16 D_800A381C;
extern u8 D_800A381E;
extern s32 D_800A3820;
extern s16 D_800A3824;
extern u8 D_800A382D;
extern s16 D_800A382E;
extern s32 D_800A3858;
extern u8 D_800A3874;
extern s16 D_800A3876;
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A389A;
extern u8 D_800A389B;
extern u16 D_800A389C;
extern u8 D_800A38AA;
extern u8 D_800A38AB;
extern s16 D_800A38AE;
extern u8 D_800A38B0;
extern u8 D_800A38B8;
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
extern s32 D_800A38F4;
extern u16 D_800A3904;
extern u8 D_800A3907;
extern s8 D_800A390E;
extern u8 D_800A390F;
extern s16 D_800A3910;
extern u8 D_800A391E;
extern u8 D_800A391F;
extern u8 D_800A3920;
extern s32 D_800F532C;
extern s32 D_800F5330;
extern s16 D_800F5358;
extern s16 D_800F535A;
extern s16 D_800F535C;
extern s16 D_800F5360;
extern s16 D_800F5362;
extern s16 D_800F5364;
extern u8 D_800F5368;
extern u8 D_800F65F8;
extern u8 D_800F65F9;
extern s32 D_800F660C;
extern s32 D_800F6610;
extern u8 D_800F6626;
extern s16 D_800F6638;
extern s16 D_800F663A;
extern s16 D_800F663C;
extern s16 D_800F6640;
extern s16 D_800F6642;
extern s16 D_800F6644;
extern s32 D_800F66A0;
extern s16 D_800F68E0;
extern s32 D_800FF580;
extern s32 D_800FF584;
extern s32 D_800FF5A4;
extern s32 D_800FF5A8;
extern s32 D_800FF5C8;
extern s32 D_800FF5CC;
extern s32 D_800FF5D0;
extern s16 D_800FF5D8;
extern s16 D_800FF5DA;
extern s16 D_800FF5DC;
extern s32 D_800FF5E0;
extern u8 D_80101BF0;
extern s16 D_80101E00;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern u8 D_80101E59;
extern s16 D_80101E66;
extern s32 D_80101E80;
extern s32 D_80101E84;
extern s32 D_80101E88;
extern s32 D_80101E8C;
extern u8 D_80101E90;
extern s32 D_80101E94;
extern s16 D_80101E98;
extern s16 D_80101E9A;
extern s16 D_80101E9C;
extern s32 D_80101EA0;
extern s32 D_80101EA4;
extern s16 D_80101ECE;
extern s16 D_80101ED6;
extern s16 D_80101EE8;
extern s32 D_80101F04;
extern s16 D_80101F08;
extern s16 D_80101F10;
extern s16 D_80101F14;
extern s16 D_80101F42;
extern s16 D_80101F5E;
extern u8 D_80101F75;
extern u8 D_80101F79;
extern u8 D_80101F7A;
extern u8 D_80101F7B;
extern s32 D_80101F80;
extern s32 D_80101F84;
extern s32 D_80101F88;
extern s32 D_80101FA0;
extern s32 D_80101FA4;
extern s32 D_80101FA8;
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
extern s16 D_8010214E;
extern s32 D_80102154;
extern u8 D_80102216;
extern s16 D_8010231A;
extern u16 D_80102322;
extern s16 D_80102326;
extern s16 D_80102334;
extern s32 D_80102350;
extern s16 D_8010235C;
extern s16 D_80102372;
extern u16 D_8010237E;
extern s16 D_8010238E;
extern s16 D_801023AA;
extern u8 D_801023C1;
extern u8 D_801023C5;
extern s32 D_801023EC;
extern s32 D_801023F4;
extern s32 D_80102408;
extern s32 D_80102410;
extern s32 D_80102448;
extern s32 D_80102450;
extern s16 D_80102462;
extern s16 D_801024DE;
extern s16 D_8010259A;
extern s32 D_801025A0;
extern u8 D_80102782;
extern u8 D_80102795;
extern s32 D_801027B4;
extern s32 D_801027B8;
extern s32 D_801027BC;
extern s32 D_80106A50;
extern s32 D_80106A58;
extern u8 D_80106A54;
extern u8 D_801077AF;
extern u8 D_801077B0;
extern u8 D_801077BA;
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_ASM("asm/funcs", func_8003B9D0);
INCLUDE_ASM("asm/funcs", md_game_check_change_sub_mode);
/* kengo:HIGH  |  md_game/md_game_check_change_sub_mode  |  87i */

extern void gpu_EnableDisplay(void);
extern void gpu_InitDisplay(void);
extern void func_800415C4(s32);
extern void file_ResetDmaFlag(void);
extern void func_8005B72C(void);
extern void func_80078824(s32);
extern void func_80035FA8(void);
extern s32 func_80036EA8(s32, s32);
extern void func_80036FD4(s32, s32);
extern void func_80037260(void);
extern s16 D_800A3834;
extern void gpu_DisableDisplay(void);
void func_8003BE10(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    file_ResetDmaFlag();
    func_8005B72C();
    func_80078824((s32)0x80118800);
    func_80035FA8();
    {
        s32 v0 = func_80036EA8(5, 0x20);
        func_80036FD4(v0, 4);
    }
    func_80037260();
    D_800A3834 = 0xB;
    gpu_DisableDisplay();
}
void func_8003BEA8(void) {
    s32 s0 = 0;
    s32 v0;
    s32 v1;

    v0 = func_800788B0();
    v1 = D_80102794 & 0x40;
    v0 = (u32)v0 > 0u;
    if (v1 != 0) {
        v0 = 1;
    }
    if (v0 != 0) {
        func_800372C0();
        {
            s32 *ptr = &D_80106A50;
            s32 old_val = *ptr;
            s32 new_val = old_val | D_800A37A4;
            if (new_val != old_val) {
                *ptr = new_val;
                s0 = 1;
            }
        }
        {
            s32 stage_u = (s32)(u16)D_80101ED2;
            s16 stage;
            if ((u32)stage_u < 2u) {
                goto check_bit;
            }
            stage = (s16)stage_u;
            if (stage == 2) {
                goto check_bit;
            }
            if ((u32)(stage_u - 0xC) < 2u) {
                goto check_bit;
            }
            if (stage == 0xE) {
                goto check_bit;
            }
            goto skip_bit;
        }
    check_bit:
        {
            s32 bit = (s16)D_80101ED2;
            u8 val;
            if (bit >= 0xC) {
                bit = bit - 9;
            }
            {
                u8 *vptr = &D_80106A54;
                val = *vptr;
                if (!((val >> bit) & 1)) {
                    *vptr = val | (1 << bit);
                    s0 = 1;
                }
            }
        }
    skip_bit:
        if (s0 != 0) {
            D_800A3834 = 0x1A;
        } else {
            D_800A3834 = 8;
        }
    }
}
void func_8003BFC4(void) {
    s32 v;
    gpu_EnableDisplay();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    file_ResetDmaFlag();
    v = func_80045814();
    func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    game_Init();
    D_800A3834 = 8;
}
INCLUDE_ASM("asm/funcs", cpu_side_move_dir_2);
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_2  |  160i  |  x4 size collision */
void func_8003C2C0(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_8005FC9C(D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    if (func_80054F68() == 0 || (D_80102794 & 0x400040) != 0) {
        if ((u32)(D_800A38A4 - 6) < 2u && D_800A3781 != 0) {
            s32 stage = (s16)D_80101ED2;
            s32 newval = 8;
            if (*((u8 *)&D_8008D9EC + stage) != 0) {
                newval = 9;
            }
            D_800A38A4 = newval;
            D_800A3834 = 0x12;
        } else {
            u8 a4val = D_800A38A4;
            if ((u32)(a4val - 4) < 2u) {
                D_800A3834 = 0x18;
            } else if ((u32)(a4val - 6) < 2u) {
                D_800A3834 = 0x1A;
            } else if ((u32)(a4val - 8) < 2u) {
                D_800A3834 = 0x1A;
            } else if (a4val < 2u) {
                D_800A3834 = 0xA;
            } else {
                D_800A3834 = 8;
            }
        }
    }
    {
        s16 state = D_800A3834;
        if (state != 0x13) {
            func_800372C0();
            state = D_800A3834;
            if (state != 0x12) {
                func_800548DC();
            }
        }
    }
}
extern u8 D_800A389B;
extern u8 D_800A377C[];
extern u8 D_800F65F8[];
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
void func_8003C42C(void) {
    s32 counts[8];
    s32 i;
    s32 v0;
    v0 = D_800A389B;
    i = 0;
    counts[2] = 0;
    counts[1] = 0;
    counts[0] = 0;
    if (v0 > 0) {
        s32 *base = counts;
        s32 n = v0;
        do {
            s32 idx = D_800A377C[i];
            base[idx]++;
            i++;
        } while (i < n);
    }
    v0 = counts[0];
    if (v0 != counts[1]) {
        v0 = counts[0] < counts[1];
    } else {
        v0 = D_800A389B;
        i = 0;
        counts[1] = 0;
        counts[0] = 0;
        if (v0 > 0) {
            s32 *base2 = counts;
            s32 n2 = v0;
            u8 *tbl = D_800F65F8;
            do {
                s32 j = 0;
                s32 *p = base2;
                u8 *q = tbl;
                do {
                    *p += *q++;
                    j++;
                    p++;
                } while (j < 2);
                i++;
                tbl += 2;
            } while (i < n2);
        }
        v0 = counts[0];
        if (v0 != counts[1]) {
            v0 = v0 < counts[1];
        } else {
            v0 = 2;
        }
    }
    D_800A382D = v0;
    do { } while (0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x15;
}
INCLUDE_ASM("asm/funcs", func_8003C560);
INCLUDE_ASM("asm/funcs", SetCurrentCursor);
/* kengo:LOW  |  su_menu_edit/_SetCurrentCursor  |  104i  |  PS2 UI — reverted */
void func_8003C8B4(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_80060768(D_800A38B4, 1, D_800A38E9);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((D_80102794 & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        func_80033FE4();
    }
}
void func_8003C958(void) {
    gpu_InitDisplay();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", func_8003C9A4);
void func_8003CCCC(void) {
    gpu_InitDisplay();
    func_80061178();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    gpu_DisableDisplay();
}
void func_8003CD10(void) {
    s32 *a0 = (s32 *)&D_800F6608;
    s16 *a1 = (s16 *)((u8 *)a0 + 0x10);
    s32 ret;

    func_8003F1E4(0);
    a0[0] = 0;
    D_800F660C = -0xBB8;
    D_800F6610 = 0;
    *a1 = 0x20;
    D_800F661C = 0;
    D_800F6620 = 0x2710;
    D_800F661A = (s16)(D_800A36AC << 2);
    func_80046BF4((s16 *)a0, a1, 0x2710);
    game_StageInit(1);

    ret = func_800600C8(D_800A391F, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 >= 0x97 || (D_80102794 & 0x400040) != 0) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
extern s8 D_800A3748;
extern u16 D_80101ED6;
extern s32 D_80101FBC;
extern s16 D_80101EDA;
extern s16 D_80102326;
extern s32 D_800A3818;
extern void func_8001DA2C(void);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8003E22C(void);
extern void func_8003F218(s32);
extern s32 func_80022408(s32 *);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_80041688(s32, s32);
void func_8003CE18(void) {
    s32 s0;
    s32 v0;
    s8 player;

    func_8001DA2C();
    func_800372C0();
    gpu_InitDisplay();
    gpu_EnableDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8003E22C();
    func_8003F218(0);
    v0 = disp_CalcFov(0x2D);
    func_8007EFFC(v0);
    player = D_800A3748;
    {
        u16 val = *((u16 *)((u8 *)&D_80101ED6 + player * 1100));
        if ((u16)(val - 6) < 2) {
            s0 = 8;
            if (player != 0) {
                s0 = 9;
            }
        } else {
            s0 = 6;
            if (player != 0) {
                s0 = 7;
            }
        }
    }
    {
        s32 *addr = (s32 *)&D_80101FBC;
        s32 result;
        if (D_800A3748 == 0) {
            addr = (s32 *)((u8 *)addr + 0x44C);
        }
        result = func_80022408(addr);
        D_800A3818 = result;
        func_80054884(0x16, s0, result, (s32)D_80101EDA, (s32)D_80102326, -1, -1, 0);
    }
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_80061178();
    D_800A37B8 = 0;
    D_800A3834 = 0x1D;
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", func_8003CF84);
void func_8003D2C4(void) {
    func_8007B600((s32)&D_800A3220, (s32)&D_80090178);
}
extern s32 D_800A3364;
extern s32 D_800A3218;
extern s32 D_800A321C;
extern s32 D_800A3358;
extern s32 D_800A335C;
extern s32 D_800A3360;
void func_8003D2F4(void) {
    s32 v0;
    s32 v1;
    D_800A3364 = 0xF0F0F0;
    v0 = D_800A3218;
    v1 = D_800A321C;
    D_800A3358 = 0;
    D_800A3360 = 0;
    D_800A335C = 0;
    D_800A3218 = (u32)v0 < 1u;
    if (v1 == 0) {
        D_800A3358 = 0x20;
    }
}
INCLUDE_ASM("asm/funcs", func_8003D330);
INCLUDE_ASM("asm/funcs", func_8003D39C);
void func_8003D478(s32 x, s32 y, u8 *str, s32 color) {
    s32 ch;
    s32 start_x = x;

    ch = *str++;
    if (ch == 0) return;

    do {
        if (ch == 0x20) {
            /* space - advance */
        } else if (ch == 0x0A) {
            x = start_x;
            y += 8;
            goto next_char;
        } else {
            func_8003D39C(x, y, ch, color);
        }
        x += 8;
    next_char:
        ch = *str++;
    } while (ch != 0);
}
INCLUDE_ASM("asm/funcs", DispSleepMenuTex);
/* kengo:LOW  |  su_menu_home/_DispSleepMenuTex  |  146i  |  PS2 UI — reverted */
void func_8003D774(s32 arg0, s32 arg1) {
    s32 *ptr = (s32 *)((u8 *)&D_800A3D40 + arg1 * 24);
    ptr[0] = arg0;
    ptr[1] = 0;
    ptr[2] = 0;
    *(s16 *)((u8 *)ptr + 0x16) = 0;
    *(s16 *)((u8 *)ptr + 0x14) = 0;
    *(s16 *)((u8 *)ptr + 0x12) = 0;
    *(s16 *)((u8 *)ptr + 0x10) = 0;
    *(s16 *)((u8 *)ptr + 0xE) = 0;
    *(s16 *)((u8 *)ptr + 0xC) = 0;
}
extern s32 func_8003D888(s32 *, s32);
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    s32 new_var2;
    u8 *new_var;
    u8 *p = base;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        do {
            nbits = func_8003D888((s32 *)base, 4);
            if (nbits == 0) {
                nbits = 16;
            }
            new_var = p;
            val = (s16)func_8003D888((s32 *)base, nbits);
        } while (0);
        if (!val) {
        }
        sign_bit = nbits - 1;
        new_var2 = 1;
        if ((val >> sign_bit) & new_var2) {
            val = val | (0xFFFF << sign_bit);
        }
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(new_var + 0xC) + val);
        i++;
        p += 2;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}

INCLUDE_ASM("asm/funcs", func_8003D888);
extern s32 light_effect_col;
extern s32 D_800A4340;
void func_8003D91C(void) {
    s16 buf[4];
    s16 s1 = 0x1E0;
    s16 s0 = 0x140;
    buf[0] = 0;
    buf[1] = s1;
    buf[2] = s0;
    buf[3] = 1;
    func_8003D9A0(buf, 0x1F, &light_effect_col);
    buf[2] = 0x40;
    buf[0] = s0;
    buf[1] = s1;
    buf[3] = 8;
    func_8003D9A0(buf, 0x13, &D_800A4340);
}
INCLUDE_ASM("asm/funcs", func_8003D9A0);
INCLUDE_ASM("asm/funcs", func_8003DA8C);
INCLUDE_ASM("asm/funcs", func_8003DBE4);
void func_8003DDF8(u32 arg0) {
    u32 *ptr = (u32 *)D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}
INCLUDE_ASM("asm/funcs", func_8003DE14);
void func_8003E0E0(void) {
    s16 buf[4];
    buf[1] = 0x1E1;
    buf[2] = 0x140;
    buf[0] = 0;
    buf[3] = 1;
    func_8003DE14(buf, 0x1F);
}
void func_8003E120(void) {
    s16 buf[4];
    buf[0] = 0x140;
    buf[1] = 0x1E8;
    buf[2] = 0x40;
    buf[3] = 8;
    func_8003DE14(buf, 0x13);
}
extern s32 D_800A3228;
extern void func_8007B6C8(s16 *, s32, s32);
void func_8003E164(s32 arg0) {
    s16 buf[4];
    s32 *s0;

    if (D_800A3228 == arg0) {
        goto end;
    }
    func_8003E22C();
    s0 = func_8004153C(arg0);
    if (s0 == 0) {
        goto end;
    }
    if (arg0 != 0) {
        buf[0] = 0x300;
    } else {
        buf[0] = 0x280;
    }
    buf[1] = 0xF8;
    buf[2] = 0x40;
    buf[3] = 6;
    func_8007B6C8(buf, 0x140, 0x1E8);
    if (arg0 == 0) {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x140, 0xE8);
    } else {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x1C0, 0xE8);
    }
    func_8007B33C(0);
    func_8003E120();
end:
    D_800A3228 = arg0;
}
extern s32 D_800A3228;
void func_8003E22C(void) {
    s32 *v1;

    if (D_800A3228 != -1) {
        v1 = func_8004153C(D_800A3228);
        if (v1 != 0) {
            if (D_800A3228 == 0) {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x140, -0xE8);
            } else {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x1C0, -0xE8);
            }
        }
        D_800A3228 = -1;
    }
}
extern s32 D_800A3228;
s32 func_8003E2A0(void) {
    return D_800A3228;
}
extern u16 D_800F6656;
void func_8003E2AC(void) {
    u16 *p = &D_800F6656;
    *p = *p & 0xFFFD;
}
u32 func_8003E2C8(void) {
    return D_800905F8;
}
INCLUDE_ASM("asm/funcs", replay_camera_get_attack_number);
/* kengo:HIGH  |  nm_replay_cam/replay_camera_get_attack_number  |  242i */
void func_8003E6A0(s32 arg0, s32 arg1) {
    replay_camera_get_attack_number(D_80101E3C, D_80101E44, arg0, arg1);
}
INCLUDE_ASM("asm/funcs", DispHira);
/* kengo:MED  |  am_rmd/DispHira  |  299i */
INCLUDE_ASM("asm/funcs", func_8003EB84);
INCLUDE_ASM("asm/funcs", md_game_check_mode);
/* kengo:HIGH  |  md_game/md_game_check_mode  |  234i */
