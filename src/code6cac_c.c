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
extern u8 D_800F33D8;
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
extern void func_80080148(void);
extern void func_8007FF7C(void);
extern void func_800828CC(s32);
extern void spu_SetVolume(s32, s32, s32);
extern void func_80085448(s32, s32, s32);
extern void replay_camera_get_attack_number(s32, s32, s32, s32);
extern void func_8007B600(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern void func_80079A30(s32 *, s32, s32, s32);
extern s32 D_800109BC;
extern void func_80078A28(s32 *);
extern void func_8003DE14(s32 *, s32);
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
extern u8 *D_800A3894;
extern s32 D_800A3878;
extern u8 D_800A390C;
extern u8 *D_800A385C;
extern u8 D_800A3817;
extern u8 D_800A3929;
extern s16 D_800A3756;
extern u8 D_800A3918;
extern s32 D_800A3D40;
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
extern void DispSleepMenuTex(s32, s32, s32, s32);
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
extern s32 D_8008EC34;
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
extern s32 file_GetFlag2(void);
extern void func_80041688(s32, s32);
extern void func_8003F1E4(s32);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);
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
extern s32 func_80036D88(void);
extern u8 D_800A3906;
extern void gnd_disp_loop_ctrl(void);
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
extern void stage_GetDataPtr(void);
extern s16 D_800A36A4;

extern u8 D_800A3768;
extern u8 D_800A38F8;
extern void func_8005B50C(void);
extern void irq_DisableInterrupts(void);
extern void func_80037348(s32 *);
extern void func_8007AE7C(s32);
extern void func_80078D68(void);
extern void irq_Reset(void);
extern s32 D_800A3210;
extern void func_8008BE04(void);
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
extern u8 D_8008EC34;
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
extern s32 D_8008DCCC;
extern s16 D_8008E194;
extern s16 D_8008E19E;
extern u8 D_8008E1A1;
extern u8 D_8008E338;
extern u16 D_8008E3C0;
extern u8 D_8008E5A8;
extern u8 D_8008E748;
extern u8 D_8008E75C;
extern u8 D_8008E778;
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
extern s16 D_8008EB56;
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
extern s32 D_80090604;
extern s16 D_80090608;
extern s16 D_800906A4;
extern s16 D_800906A6;
extern s32 D_800948C0;
extern u16 D_80094C68;
extern s16 D_800973FC;
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
extern s32 D_800A36C0;
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
extern s16 D_800A3904;
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
extern u8 D_80106A54;
extern u8 D_801077AF;
extern u8 D_801077B0;
extern u8 D_801077BA;

/* GP-relative extern data (for decompiled functions) */
extern s32 D_800A3730;
extern s16 D_800A379E;
extern s16 D_800A3814;
extern s16 D_800A37C8;
extern s32 D_800A31F4;
extern u8 D_800A38CC;
extern u8 D_800A382C;
extern s32 D_800A3734;
extern s32 D_800A373C;
extern s32 D_800A3810;
extern s32 D_800A3738;
extern u8 D_800A320C;
extern s32 D_800A37DC;
extern s32 D_800A37F0;
extern s32 D_800A37FC;
extern s32 D_800A3800;
extern s32 D_800A3838;
extern s32 D_800A383C;
extern s32 D_800A3848;
extern s32 D_800A3850;
extern s32 D_800A31F8;
extern u8 D_800A36F8;
extern s32 D_800A36EC;
extern u8 D_800A3782;
extern u8 D_800A3203;
extern u8 D_800A31FC;
extern s32 D_800A38C8;
extern u8 D_80102810;
extern s32 D_800A3870;
extern s32 D_800A3688;
extern u8 D_800A37D0;
extern s32 D_800A3915_ext;
extern s32 D_800A36F4_ext;

/* Extern function declarations for decompiled functions */
extern s32 func_80078998(s32);
extern void func_80078988(s32);
extern void func_800789A8(s32);
extern void func_800789B8(void);
extern void func_800789C8(void);
extern void func_800789F8(s32, s32 *, s32);
extern void func_80078A18(s32);
extern s32 func_80078A38(s32 *, s32 *);
extern s32 func_80078A48(s32 *);
extern void func_80078BA8(s32);
extern s32 func_80078B04(s32);
extern void func_8007A400(void);
extern void func_8008BE4C(void);
extern s32 func_8006BEC4(s32, s32);
extern void func_8003E22C(void);
extern void func_8003F218(s32);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8001B6F4(void);
extern void func_80022568(u8 *);
extern s32 D_800109B0;
extern s32 D_80102810;
extern s32 D_800F34D8;
extern u8 D_80101EC8;

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_800375EC(void) {
    func_8007A370(1);
    func_8007A3C8();
    func_80078958();
    func_80078A58(0);
    func_800789B8();
    D_800A37DC = func_80078978(0xF4000001, 4, 0x2000, 0);
    D_800A37F0 = func_80078978(0xF4000001, 0x8000, 0x2000, 0);
    D_800A37FC = func_80078978(0xF4000001, 0x100, 0x2000, 0);
    D_800A3800 = func_80078978(0xF4000001, 0x2000, 0x2000, 0);
    D_800A3838 = func_80078978(0xF0000011, 4, 0x2000, 0);
    D_800A383C = func_80078978(0xF0000011, 0x8000, 0x2000, 0);
    D_800A3848 = func_80078978(0xF0000011, 0x100, 0x2000, 0);
    D_800A3850 = func_80078978(0xF0000011, 0x2000, 0x2000, 0);
    func_800789C8();
    func_800789A8(D_800A37DC);
    func_800789A8(D_800A37F0);
    func_800789A8(D_800A37FC);
    func_800789A8(D_800A3800);
    func_800789A8(D_800A3838);
    func_800789A8(D_800A383C);
    func_800789A8(D_800A3848);
    func_800789A8(D_800A3850);
}
void func_80037774(void) {
    func_800789B8();
    func_80078988(D_800A37DC);
    func_80078988(D_800A37F0);
    func_80078988(D_800A37FC);
    func_80078988(D_800A3800);
    func_80078988(D_800A3838);
    func_80078988(D_800A383C);
    func_80078988(D_800A3848);
    func_80078988(D_800A3850);
    func_800789C8();
    func_8007A400();
}
s32 func_80037804(void) {
    extern s32 D_800A3924;
    s32 result;
    s32 one;
    s32 temp;
    result = (func_80078998(D_800A37DC) == 1);
    one = 1;
    if (func_80078998(D_800A37F0) == one) {
        result = 2;
    }
    if (func_80078998(D_800A37FC) == one) {
        result = 3;
    }
    if (func_80078998(D_800A3800) == one) {
        result = 4;
    }
    temp = D_800A3924;
    D_800A3924 = temp + 1;
    if (temp >= 0x78) {
        result = 2;
    }
    return result;
}
s32 func_800378A8(void) {
    if (func_80078998(D_800A37DC) == 1) {
        return 1;
    }
    if (func_80078998(D_800A37F0) == 1) {
        return 2;
    }
    if (func_80078998(D_800A37FC) == 1) {
        return 3;
    }
    return (func_80078998(D_800A3800) == 1) * 4;
}
void func_8003791C(void) {
    func_80078998(D_800A37DC);
    func_80078998(D_800A37F0);
    func_80078998(D_800A37FC);
    func_80078998(D_800A3800);
}
s32 func_80037964(void) {
    s32 one = 1;
loop:
    if (func_80078998(D_800A3838) == one) { return 1; }
    if (func_80078998(D_800A383C) == one) { return 2; }
    if (func_80078998(D_800A3848) == one) { return 3; }
    if (func_80078998(D_800A3850) != one) { goto loop; }
    return 4;
}
void func_800379D8(void) {
    func_80078998(D_800A3838);
    func_80078998(D_800A383C);
    func_80078998(D_800A3848);
    func_80078998(D_800A3850);
}
INCLUDE_ASM("asm/funcs", func_80037A20);
s32 func_80037AA4(void) {
    s32 var_a1;
    register s32 var_a2 asm("a2");
    register s32 var_a0 asm("a0");
    s8 *var_v1;
    register s32 var_v0 asm("v0");
    s32 sp_dummy[2];

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a2 > 0) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
s32 func_80037B00(u8 *arg0) {
    s32 sp_dummy[2];
    register s32 var_t1 asm("t1");
    register s32 var_t3 asm("t3");
    register s32 var_t2 asm("t2");
    register s8 *var_a3 asm("a3");
    register s8 *var_a1 asm("a1");
    register s8 *var_a2 asm("a2");
    register s8 *var_t0 asm("t0");
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");

    var_t1 = 0;
    var_v0 = D_800A38C8;
    if (var_v0 <= 0) {
        goto block_end;
    }
    var_t3 = var_v0;
    var_a3 = (s8 *)&D_80102810;
loop_outer:
    var_t2 = 0;
    var_a1 = var_a3;
    var_a2 = (s8 *)arg0;
    var_t0 = var_a3 + 0x15;
loop_inner:
    var_v1 = (u8)*var_a2;
    if (var_v1 == 0) {
        goto block_5c;
    }
    var_v0 = (u8)*var_a1;
    if (var_v1 != var_v0) {
        goto block_6c;
    }
    var_a1 += 1;
    var_a2 += 1;
    if ((s32)var_a1 < (s32)var_t0) {
        goto loop_inner;
    }
block_5c:
    var_t1 += 1;
    if (var_t2 != 0) {
        goto block_74;
    }
    return 1;
block_6c:
    var_t2 = 1;
    goto block_5c;
block_74:
    if (var_t1 < var_t3) {
        var_a3 += 0x28;
        goto loop_outer;
    }
block_end:
    return 0;
}
extern s32 func_800789E8(s32 *, s32);
typedef void (*Func79A30_5)(s32 *, s32 *, s32, s32, s32);
s32 func_80037B90(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 sp18[8];
    register s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    temp_v0 = func_800789E8(sp18, 0x8001);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_800789F8(temp_v0, arg3, arg4);
    return -(func_80037964() != 1);
}
extern void func_80078A18(s32);
extern void func_80078A08(s32, s32, s32);
s32 func_80037C34(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s32 sp18[8];
    s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    if (arg6 != 0) {
        temp_v0 = func_800789E8(sp18, (arg4 << 16) | 0x200);
        if (temp_v0 == -1) {
            return -1;
        }
        func_80078A18(temp_v0);
    }
    temp_v0 = func_800789E8(sp18, 0x8002);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_80078A08(temp_v0, arg3, arg5);
    return -(func_80037964() != 1);
}
INCLUDE_ASM("asm/funcs", func_80037D14);
extern void func_80078A28(s32 *);
extern s32 D_800109C8;
void func_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    func_80079A30(buf, &D_800109C8, a0, a1);
    func_80078A28(buf);
}
INCLUDE_ASM("asm/funcs", func_80037F40);
INCLUDE_ASM("asm/funcs", damage_DebugDisp);
/* kengo:HIGH  |  is_damage_calc/damage_DebugDisp  |  79i */

void func_80038148(void) {
    u8 *p = &D_800F33D8;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}
INCLUDE_ASM("asm/funcs", func_80038170);
INCLUDE_ASM("asm/funcs", pad_FuncAnalog);
/* kengo:HIGH  |  is_pad/pad_FuncAnalog  |  173i */
extern s32 damage_DebugDisp(s32 *);
void func_80038658(void) {
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");
    s32 var_s0;

    var_v1 = D_800A31F4;
    if (var_v1 == 4) {
        goto block_4;
    }
    if (var_v1 == 6) {
        goto block_6;
    }
    return;
block_4:
    var_s0 = func_800378A8();
    var_v0 = 1;
    if (var_s0 == 0) {
        goto block_store;
    }
    func_80078A18(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 3;
        goto block_store_clear;
    }
    var_v0 = 2;
    goto block_store_clear;
block_6:
    var_s0 = func_800378A8();
    var_v0 = 4;
    if (var_s0 == 0) {
        goto block_store;
    }
    func_80078A18(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 6;
        goto block_store_clear;
    }
    var_v0 = 5;
    D_800A379E = (s16)var_v0;
    if (damage_DebugDisp(&D_800F34D8) != 0) {
        goto block_clear;
    }
    var_v0 = 0xF;
block_store_clear:
    D_800A379E = (s16)var_v0;
block_clear:
    D_800A31F4 = 0;
    return;
block_store:
    D_800A379E = (s16)var_v0;
}
s32 func_80038734(void) {
    if ((u32)D_800A31F4 < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    pad_FuncAnalog();
    func_80038658();
    return D_800A379E;
}
void func_8003877C(void) {
    D_800A379E = 4;
    D_800A3814 = 0;
    D_800A37C8 = 0;
    D_800A31F4 = 1;
}
void func_8003879C(void) {
    D_800A379E = 1;
    D_800A37C8 = 1;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387C0(void) {
    D_800A379E = 1;
    D_800A37C8 = 2;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387E8(void) {
    D_800A379E = 9;
    D_800A37C8 = 3;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
INCLUDE_ASM("asm/funcs", func_8003880C);
s32 func_800388A8(void) {
    extern u8 D_800A3204;
    extern u8 D_800A3318;
    s32 result = 0;
    u32 buttons;
    if (D_800A3204 != 0) {
        D_800A3204 = 0;
        D_800A3318 = 0;
    }
    buttons = D_80102794;
    if (buttons & 0x400040) {
        result = -1;
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A3318 != 0) {
        } else {
            result = 1;
        }
    } else if (buttons & (u32)0x80008000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3318 = 0;
    } else if (buttons & 0x20002000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3318 = 1;
    }
    func_8006BEC4(0x13, D_800A3318);
    if (result != 0) {
        D_800A3204 = 1;
    }
    return result;
}
INCLUDE_ASM("asm/funcs", func_80038988);
INCLUDE_ASM("asm/funcs", func_80038C70);
s32 *func_800392B8(void) {
    return &D_800F33D8;
}
void func_800392C8(void) {
    u8 val;
    int new_var;
    s32 i;
    s32 j;

    val = 0xFF;
    i = 0x1F0;
    D_800A36EC = (u8 *)&D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
loop1:
    *(&D_80101BF0 + i) = val;
    i -= 0x10;
    if (i >= 0) goto loop1;

    new_var = -1;
    j = 0xB30;
loop2:
    *((s16 *)((u8 *)&D_800F68E0 + j)) = new_var;
    j -= (0, 0x10);
    if (j >= 0) goto loop2;
}
void func_80039320(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    u8 *p;
    s16 *q;
    s16 val;
    s16 newval;

    i = 0;
    newval = 0xFF;
    p = &D_80101BF0;

    do {
        if (*p == D_800A36F8) {
            *p = newval;
        }
        i++;
        p += 0x10;
    } while (i < 0x20);

    q = (s16 *)&D_800F68E0;
    i = 0;
    do {
        val = *q;
        if (val != -1) {
            newval = val + 1;
            *q = newval;
            if ((s16)newval - *(u8 *)((u8 *)q + 2) >= 0x101) {
                *q = -1;
            }
        }
        i++;
        q = (s16 *)((u8 *)q + 0x10);
    } while (i < 0xB4);

    D_800A379C = 0;
    D_800A3714 = 0;
}
INCLUDE_ASM("asm/funcs", saSeInit_2);
/* kengo:MED  |  sa_se/saSeInit_2  |  123i  |  x2 size collision */
void func_800395B4(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    u8 *slot;
    u8 idx;
    u8 sentinel;

    if (D_800A3208 == 0) {
        idx = D_800A379C;
        slot = &D_80101BF0 + (u32)(idx & 0xFF) * 0x10;
        if ((u32)(idx & 0xFF) < 0x20U) {
            sentinel = 0xFF;
loop:
            if (*slot != sentinel) {
                D_800A379C = idx + 1;
                idx = idx + 1;
                slot += 0x10;
                if ((u32)(idx & 0xFF) < 0x20U) {
                    goto loop;
                }
            }
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            slot[1] = arg0;
            slot[2] = arg1;
            slot[0] = tmp;
            *(s16 *)&slot[4] = (s16)arg2[0];
            *(s16 *)&slot[6] = (s16)arg2[1];
            *(s16 *)&slot[8] = (s16)arg2[2];
            if (arg3 != NULL) {
                *(u16 *)&slot[0xA] = arg3[0];
                *(u16 *)&slot[0xC] = arg3[1];
                *(u16 *)&slot[0xE] = arg3[2];
            }
        }
    }
}
void func_80039680(u8 *a0) {
    s16 idx;
    u8 *base;
    u8 *dest;

    idx = *(s16 *)(a0 + 4);
    base = (u8 *)(D_800A36EC + D_800A36F8 * 56);
    dest = base + idx * 28;

    *(s16 *)(dest + 4) = *(s32 *)(a0 + 0xF4);
    *(s16 *)(dest + 8) = *(s32 *)(a0 + 0xFC);
    *(s16 *)(dest + 6) = *(s32 *)(a0 + 0xF8);

    {
        u16 v = *(u16 *)(a0 + 0x1CA);
        u8 b = *(u8 *)(a0 + 0xB3);
        *(s16 *)(dest + 0xA) = (v & 0xFFF) | (b << 12);
    }

    *(s16 *)(dest + 0xC) = *(s32 *)(a0 + 0x148);
    *(u8 *)(dest + 0x14) = *(u16 *)(a0 + 0x1E6) >> 2;
    *(u8 *)(dest + 0x15) = *(u16 *)(a0 + 0x1E8) >> 2;
    *(u8 *)(dest + 0x16) = *(u16 *)(a0 + 0x1EA) >> 2;
    *(s32 *)(dest + 0) = *(s32 *)(a0 + 0x50);
    *(u8 *)(dest + 0x17) = 0;

    if (*(u8 *)(a0 + 0x60) != 0) {
        *(u8 *)(dest + 0x17) = 1;
    }
    if (*(u8 *)(a0 + 0x61) != 0) {
        *(u8 *)(dest + 0x17) |= 2;
    }

    *(s16 *)(dest + 0xE) = *(u16 *)(a0 + 0x64);
    *(s16 *)(dest + 0x10) = *(u16 *)(a0 + 0x66);
    *(s16 *)(dest + 0x12) = *(u16 *)(a0 + 0x68);
    *(u8 *)(dest + 0x18) = *(u8 *)(a0 + 0x62);
    *(u8 *)(dest + 0x19) = *(u16 *)(a0 + 0x40);
}
void func_800397A0(void) {
    u8 val = D_800A36F8;
    if ((u8)val == 0x77) {
        D_800A36F8 = 0;
        D_800A3782 = 1;
    } else {
        D_800A36F8 = val + 1;
    }
}
void func_800397D4(void) {
    gpu_EnableDisplay();
    func_8003E22C();
    func_8003F218(0);
    func_8007EFFC(disp_CalcFov(0x2D));
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_8001B6F4();
    func_80061178();
    D_800A37D0 = 0;
    D_800A3834 = 5;
}
extern s32 func_80053584(s32 *, s32 *, s32 *, s32 *);
extern s32 func_80054434(void);
void func_8003984C(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 sp10[3];
    s32 sp20[3];
    s32 sp30[4];
    s32 sp40[2];
    s32 mid_x, mid_y, mid_z;
    s32 result;

    mid_x = (s32)(arg0[0x198 / 4] + arg0[0x1A4 / 4]) / 2;
    sp10[0] = mid_x;
    mid_y = (s32)(arg0[0x19C / 4] + arg0[0x1A8 / 4]) / 2;
    sp10[1] = mid_y - 0x190;
    mid_z = (s32)(arg0[0x1A0 / 4] + arg0[0x1AC / 4]) / 2;
    sp20[0] = mid_x;
    sp20[1] = mid_y + 0x190;
    sp10[2] = mid_z;
    sp20[2] = mid_z;
    if (func_80053584(sp10, sp20, sp30, sp40) != 0) {
        result = func_80054434();
        *arg1 = result;
        if (result == 7) {
            goto neg;
        }
        if (result == 0) {
            goto neg;
        }
        *arg2 = ((0x2A >> result) ^ 1) & 1;
    } else {
        *arg1 = -1;
neg:
        *arg2 = -1;
    }
}
INCLUDE_ASM("asm/funcs", func_8003993C);
void func_8003A174(void) {
    s32 neg1;
    func_800789B8();
    neg1 = -1;
    do {
        D_800A3738 = func_80078978(0xF000000B, 0x400, 0x2000, 0);
    } while (D_800A3738 == neg1);
    neg1 = -1;
    do {
        D_800A3810 = func_80078978(0xF000000B, 0x8000, 0x2000, 0);
    } while (D_800A3810 == neg1);
    func_800789C8();
    neg1 = -1;
    func_800828CC(2);
    func_8008BE04();
    do {
        D_800A373C = func_800789E8(&D_800A3210, 2);
    } while (D_800A373C == neg1);
    neg1 = -1;
    do {
        D_800A3734 = func_800789E8(&D_800A3210, 0x8001);
    } while (D_800A3734 == neg1);
    func_8008C464(2, 0, 0);
    func_8008C464(1, 3, 0xE100);
    func_8008C464(1, 4, 1);
}
void func_8003A264(void) {
    func_80078A18(D_800A3734);
    func_80078A18(D_800A373C);
    func_800789B8();
    func_80078988(D_800A3738);
    func_80078988(D_800A3810);
    func_800789C8();
    func_800828CC(2);
    func_8008BE4C();
    func_8008C464(1, 1, 0);
}
s32 func_8003A2DC(void) {
    return (func_8008C464(0, 0, 0) & 0x180) == 0;
}
void func_8003A308(void) {
    if (func_8008C464(3, 1, 0) != 0) {
        D_800A38A0 = 1;
    } else {
        D_800A38A0 = 0;
    }
    func_8008C464(3, 0, 1);
}
void func_8003A360(void) {
    func_800789A8(D_800A3810);
    func_800789A8(D_800A3738);
    D_800A320C = 1;
    D_800A3730 = 0;
}
void func_8003A39C(void) {
    D_800A320C = 0;
    D_800A3730 = 0;
    func_8008C464(2, 0, 0);
    func_8008C464(1, 1, 0);
    func_8003A264();
    D_800A3834 = 8;
}
void func_8003A3F0(void) {
    func_8003A39C();
    D_800A3928 = 1;
}
void func_8003A41C(void) {
    D_800A3730 = 1;
}
s32 func_8003A42C(s32 a0, u32 a1) {
    if (a1 > 0x10000) {
        D_800A382C = 0;
        return 0;
    }
    return 1;
}
INCLUDE_ASM("asm/funcs", func_8003A450);
void func_8003A574(void) {
    func_800789F8(D_800A3734, &D_800A3688, 8);
}
INCLUDE_ASM("asm/funcs", pad_ClearAppliBuffer);
/* kengo:HIGH  |  is_pad/pad_ClearAppliBuffer  |  87i */
s32 func_8003A6FC(u32 arg0) {
    s32 count = 0;
    s32 i;
    for (i = 0; i < 32; i++) {
        count += (arg0 >> i) & 1;
    }
    return count;
}
INCLUDE_ASM("asm/funcs", func_8003A728);

void func_8003AA48(void) {
    s16 buf[12];
    *(s32 *)&buf[4] = 0;
    buf[1] = 4;
    buf[0] = 4;
    func_8003A728((s32)buf);
}
void func_8003AA78(void) {
    D_800A3870 = 1;
    func_800828CC(2);
    func_8003AA48();
    func_800828CC(2);
}
void func_8003AAB0(void) {
    s32 val;
    D_800A3870 = 2;
    func_800828CC(2);
    val = 2;
    do {
        func_8003AA48();
        if (D_800A320C == 0) {
            goto end;
        }
        func_80078BA8(0xF2000001);
        do {
        } while (func_80078B04(0xF2000001) < 0x100);
    } while (D_800A3870 == val);
end:
    func_800828CC(2);
    func_8003AA48();
    func_800828CC(2);
}
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern u16 func_80035E88(s32);
extern s32 func_80035EDC(s16);
extern u16 func_80019488(void);
extern void func_800194C0(s16);
extern u16 func_80079154(void);
extern void func_80019568(s32);
extern u8 D_800A38AC;
extern s32 D_800A38D0;
extern s32 D_800A3908;
extern s32 D_800A38FC;
extern u16 D_800A37C4;

INCLUDE_ASM("asm/funcs", func_8003AB44);
s32 func_8003ACB8(void) {
    s32 temp_s0;

    func_80077AE0();
    func_8007B2A0(0);
    D_800A37B8 = 0;
    D_800A38AC = 0;
    D_800A38D0 = 0;
    D_800A3908 = 0;
    D_800A38FC = 0;
    func_8003A174();
    do {
        func_80019568(1);
        func_8005C6D0();
        temp_s0 = func_8003AB44();
        func_800828CC(2);
    } while (temp_s0 == 0);
    func_80077B00();
    func_800194F4();
    D_800A37C4 = func_80035E88(D_80106A50);
    func_8003AA48();
    func_800828CC(1);
    func_8003AA48();
    D_800A38E4 = func_80035EDC(D_800A36C6);
    D_800A37C4 = func_80019488();
    func_800828CC(1);
    func_8003AA48();
    func_800828CC(1);
    func_8003AA48();
    func_800828CC(1);
    func_8003AA48();
    func_800194C0(D_800A36C6);
    D_800A37C4 = func_80079154();
    func_800828CC(1);
    func_8003AA48();
    func_800828CC(1);
    func_8003AA48();
    func_800828CC(1);
    func_8003AA48();
    {
        s32 var_v0;
        if (D_800A38A0 == 0) {
            var_v0 = (u16)D_800A37C4;
        } else {
            var_v0 = D_800A36C6;
        }
        D_800A3904 = var_v0;
    }
    gpu_InitDisplay();
    gpu_DisableDisplay();
    func_80078BA8(0xF2000001);
    return temp_s0;
}
INCLUDE_ASM("asm/funcs", func_8003AE5C);
void func_8003AF40(s32 arg0) {
    if ((&D_8010277E)[arg0] == 0xFF) {
        (&D_8010277E)[arg0] = (&D_8010277E)[(u32)arg0 < 1u];
    }
    func_80022580(arg0, ((s8 *)&D_80102780)[arg0], ((s8 *)&D_8010277C)[arg0], ((s8 *)&D_8010277E)[arg0], 0);
    gpu_EnableDisplay();
    func_80020D38();
    func_80040510(arg0, (&D_8008D578)[(s8)(&D_8010277C)[arg0]], (s32)0x80190800);
}
INCLUDE_ASM("asm/funcs", func_8003AFFC);
INCLUDE_ASM("asm/funcs", func_8003B10C);
void func_8003B20C(s32 arg0) {
    u8 *new_var;
    register s32 one asm("v1") = 1;
    D_80102780 = 0;
    D_80102781 = one;
    arg0 = ((s32 *)(new_var = &D_800900EC))[arg0];
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = one;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = arg0;
    file_LoadOverlay();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = (&D_8008D538)[(s8)D_8010277C];
}
extern u8 D_8010277C;
extern u8 D_800A3836;
extern u8 *D_800A3894;
extern u8 D_800A376A;
extern u8 D_800A36C8;
extern void func_80041604(s32, s32);
void func_8003B2C8(void) {
    u8 *p = &D_8010277C;
    D_800A3836 = *p;
    {
        u8 *base = D_800A3894;
        u8 v1 = D_800A376A;
        u8 v0 = *base;
        D_800A36C8 = v1;
        D_800A376A = 0;
        *p = v0;
    }
    func_80041604(0, 0);
}
void func_8003B328(void) {
    u8 *p = &D_8010277C;
    u8 v_277C = *p;
    u8 v_376A = D_800A376A;
    u8 v_3836 = D_800A3836;
    u8 v_36C8 = D_800A36C8;
    D_800A3836 = 0xFF;
    D_800A3915 = v_277C;
    D_800A36F4 = v_376A;
    *p = v_3836;
    D_800A376A = v_36C8;
    func_80041604(0, v_36C8);
    func_80022568(&D_80101EC8);
}
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
s32 func_8003B484(u8 *arg0) {
    D_800A3712 = 1;
    D_8010277D = arg0[0];
    D_8010277F = arg0[1];
    D_80102783 = arg0[2];
    func_8003AF40(1);
    func_8003AFFC();
    return 3;
}
void func_8003B4DC(void) {
    D_800A3712 = 1;
    D_8010277E = 0;
    D_8010277D = 0x1F;
    D_8010277F = 0;
    func_8003AF40(0);
    func_8003AF40(1);
    func_8003AFFC();
}
void func_8003B534(s32 a0) {
    D_800A37B0 = a0;
    D_800A3834 = 6;
    D_800A3878 = D_800A3894 + (a0 * 4 + 1);
}
void func_8003B56C(s32 arg0) {
    D_800A390C = arg0;
    D_800A3834 = 6;
    D_800A3878 = D_800A385C + (arg0 * 4 - 4);
}
INCLUDE_ASM("asm/funcs", func_8003B5A4);
void func_8003B870(void) {
    func_80041604(0, D_800A376A);
    func_80041604(1, 0);
    func_8005B5AC();
    gpu_InitDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    gpu_DisableDisplay();
}
void func_8003B8E4(void) {
    s32 tmp;
    s32 ret;

    tmp = D_800A37B8 + 1;
    D_800A37B8 = tmp;
    if (tmp < 3) {
        ret = func_80060544(D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    }
    if (D_800A37B8 == 3) {
        func_8007B33C(0);
        func_8003AE5C(D_800A3844);
        D_800A37C0 = 500;
        D_800A38F8 = 0;
        D_800A3768 = 0x14;
        se_data_set();
        func_8003B5A4();
        disp_SetFramebufferMode(1, 0, 0, 0);
        D_800A390D = 1;
    }
}
