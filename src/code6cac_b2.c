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
extern void replay_camera_get_attack_number(s32, s32, s32, s32);
extern void func_8007B600(s32, s32);
extern void func_80079A30(s32 *, s32, s32, s32);
extern void func_80078A28(s32 *);
extern void func_8003A728(s32 *);
extern void func_8003DE14(s32 *, s32);
extern void gpu_InitDisplay(void);
extern void func_80061178(void);
extern void gpu_DisableDisplay(void);
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
extern s32 file_GetFlag2(void);
extern void func_80041688(s32, s32);
extern void func_8003F1E4(s32);
extern s16 *snd_GetSeId(void);
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
extern s32 D_80101E74;

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
extern void func_80037774(void);
extern void irq_DisableInterrupts(void);
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
extern u8 D_80106A54;
extern u8 D_801077AF;
extern u8 D_801077B0;
extern u8 D_801077BA;
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

extern void func_80035280(void);
extern void func_80068ECC(s32);
extern u8 D_800A3740;
void func_80035438(void) {
    s32 a0;
    D_800A3740 = 1;
    func_80035280();
    if (D_80106A54 == 0x3F) {
        a0 = 0xFF;
    } else {
        a0 = 0xF7;
    }
    func_80068ECC(a0);
}
extern u8 D_800A31D8;
extern void func_8003A41C(void);
extern void func_80020CDC(void);
void func_80035480(void) {
    s32 v0;
    gpu_EnableDisplay();
    gpu_InitDisplay();
    if (D_800A31DA == 0) {
        func_8003A41C();
    }
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    file_ResetDmaFlag();
    if (D_800A31D8 != 0) {
        func_8005B72C();
        D_800A390E = -1;
        v0 = 1;
    } else {
        v0 = 1;
    }
    D_800A31D8 = v0;
    func_80035438();
    func_80077820((s32)0x80118800);
    D_800A37B8 = 0;
    D_800A3834 = 9;
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", func_8003553C);
void func_800355E8(void) {
    func_80035FA8();
    func_80037110(1);
    func_800371E8(1);
}
INCLUDE_ASM("asm/funcs", func_80035618);
INCLUDE_ASM("asm/funcs", func_80035828);
void func_80035DC8(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    file_ResetDmaFlag();
    func_8005B72C();
    func_80077820((s32)0x80118800);
    D_800A3834 = 0x1B;
    gpu_DisableDisplay();
}
void func_80035E38(void) {
    D_800A36F1 = 1;
    func_8003553C();
    if (motion_SetMotion() != 0) {
        D_800A3834 = 8;
        D_800A36F1 = 2;
    }
}
s32 func_80035E88(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << i)) {
                result |= (one << j);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
s32 func_80035EDC(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << j)) {
                result |= (one << i);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
extern void func_80080600(u8 *);
extern u8 D_800A3718;
extern u8 D_800A3719;
extern u8 D_800A371A;
extern u8 D_800A371B;
extern s16 D_800A3854;
void func_80035F30(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    D_800A3718 = (u8)arg0;
    D_800A3719 = (u8)arg1;
    D_800A371A = (u8)arg2;
    D_800A371B = (u8)arg3;
    func_80080600(&D_800A3718);
    D_800A3854 = 0;
}
extern u8 D_800A36B8;
extern u8 D_800A36B9;
extern u8 D_800A36BA;
extern u8 D_800A36BB;
extern s16 D_800A3840;
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8 = (u8)arg1;
    D_800A36B9 = (u8)arg2;
    D_800A36BA = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36BB = (u8)arg4;
}
void func_80035FA8(void) {
    spu_SetVolume(0, 0, 1);
    func_80085448(0, 0x7F, 0x7F);
}
extern u8 D_800A31E4;
void func_80035FE0(void) {
    func_8007FF7C();
    func_80080168(0);
    func_80035F30(0, 0, 0, 0);
    D_80101E62 = 0;
    if (D_800A31E4 == 0) {
        D_800A31E4 = 1;
    }
}
void func_80036034(void) {
    func_80080148();
    func_8007FF7C();
    func_800828CC(4);
}
extern void func_80080620(s32, s32);
extern s32 func_800807A8(s32);
void func_80036064(u8 arg0) {
    s32 sp[4];
    if (arg0 == 1) {
        D_80101E98 = 0;
        if (D_80101E80 <= 0) {
            return;
        }
        func_80080620((s32)sp, 3);
        {
            s32 v0 = func_800807A8((s32)sp);
            if (v0 != D_80101EA0) {
                D_80101E80 = -2;
                goto do_stop;
            }
        }
        func_80080620(D_80101E84, 0x200);
        D_80101E84 = D_80101E84 + 0x800;
        D_80101E80 = D_80101E80 - 1;
        D_80101EA0 = D_80101EA0 + 1;
        if (D_80101E80 == 0) {
            goto do_stop;
        }
        return;
    } else {
        D_80101E80 = -1;
    }
do_stop:
    func_80080240(0);
    func_80080390(9, 0);
}
INCLUDE_ASM("asm/funcs", special_camera_set_win_cam);
/* kengo:MED  |  nm_special_cam/special_camera_set_win_cam  |  502i  |  -10 */
INCLUDE_ASM("asm/funcs", special_camera_Exec);
/* kengo:HIGH  |  nm_special_cam/special_camera_Exec  |  274i */
s32 func_80036D88(void) {
    return D_80101E62 == 0;
}
INCLUDE_ASM("asm/funcs", replay_camera_Init);
/* kengo:HIGH  |  nm_replay_cam/replay_camera_Init  |  39i */
s32 func_80036E34(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    if (replay_camera_Init(arg0, arg1) == 0) {
        return 0;
    }
    func_800806A4(func_800807A8((s32)&D_80101E6C) + arg2, (s32)&D_80101E6C);
    D_80101E78 = arg3;
    return 1;
}
s32 func_80036EA8(s32 arg0, s32 arg1) {
    return (&D_8008F12C)[arg0] + arg1;
}
void func_80036EC0(void) {
    func_80080240(0);
    func_80035F30(0, 0, 0, 0);
    func_80080148();
    func_80080390(9, 0);
    D_80101E68 = 1;
    D_80101E62 = 0xB;
    D_80101E5C = 0;
}
u32 func_80036F28(s32 arg0) {
    return (&D_8008EC38)[arg0 * 2];
}
void func_80036F40(void) {
    u16 *s0;
    func_8003AA78();
    s0 = (u16 *)&D_80101E9E;
    while (1) {
        if (func_80036D88() != 0) {
            break;
        }
        func_8003AA48();
        special_camera_Exec();
        if (D_800A3906 != 0) {
            func_8005C6D0();
        }
        gnd_disp_loop_ctrl();
        *s0 = *s0 + 2;
        func_800828CC(2);
    }
    func_8003AAB0();
}
INCLUDE_ASM("asm/funcs", func_80036FD4);
s32 func_80037110(s32 arg0) {
    u8 *s0 = (u8 *)&D_8008F13C + (arg0 << 3);
    s32 v0;
    v0 = func_80036EA8(5, s0[0]);
    v0 = func_80036FD4(v0, s0[1]);
    if (v0 != 0) {
        if (*(s32 *)(s0 + 4) != -1) {
            v0 = func_800807A8((s32)&D_8008EC34 + (s32)D_80101E60 * 8);
            D_80101E74 = v0 + *(s32 *)(s0 + 4);
        }
        return 1;
    }
    return 0;
}
extern s16 D_80101E64;

s32 func_800371AC(void) {
    s32 ret = func_80037110();
    if (ret) {
        D_80101E64 = 1;
        return 1;
    }
    return 0;
}
void func_800371E8(s16 arg0) {
    D_80101E6A = arg0;
}
s32 func_800371F8(void) {
    if (func_80036FD4() != 0) {
        D_80101E64 = 1;
        return 1;
    }
    return 0;
}
void func_80037234(void) {
    D_80101E64 = 0;
    D_80101E68 = 1;
}
void func_80037250(void) {
    D_80101E64 = 0;
}
void func_80037260(void) {
    while (D_80101E62 != 0x16) {
        func_8003AA48();
        special_camera_Exec();
        func_800828CC(2);
    }
}
void func_800372C0(void) {
    if (D_80101E62 != 0) {
        func_80036EC0();
    }
    func_80036F40();
}
void func_800372F4(s32 arg0) {
    s32 v = arg0 + 0x7FF;
    if (v < 0) {
        v = arg0 + 0xFFE;
    }
    func_800826CC(v >> 11);
    do {
        v = func_800827D0(1, 0);
        if (v > 0) {
            func_800828CC(0);
        }
    } while (v > 0);
}
INCLUDE_ASM("asm/funcs", special_camera_get_rot_dir);
/* kengo:MED  |  nm_special_cam/special_camera_get_rot_dir  |  66i  |  +6 9.1% */
void func_80037468(s32 a0, s32 *a1, s32 a2) {
    s32 sp[16];
    func_800828CC(0);
    func_8007B2A0(0);
    gpu_EnableDisplay();
    func_8005B50C();
    func_80037774();
    irq_DisableInterrupts();
    func_8007FF7C();
    special_camera_get_rot_dir(sp);
    func_8007B33C(0);
    func_8007AE7C(0);
    func_80078D68();
    irq_Reset();
    sp[8] = a2;
    sp[9] = 0;
    func_800789B8();
    func_80078948(sp, a0, a1);
    sys_Init();
    file_LoadSoundData();
    func_800828CC(0);
    func_8007B2A0(1);
}
extern s32 func_80036EA8(s32, s32);
extern s32 func_800392B8(void);
extern void func_80037468(s32, s32 *, s32);
extern u8 D_8008EC34;
void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&D_8008EC34 + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&D_8008EC34 + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    func_80037468(6, sp, v0 + 0x7FC);
}

