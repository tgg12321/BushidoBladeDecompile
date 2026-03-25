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
extern s32 func_80037110(s32);
extern void func_80036EC0(void);
extern void func_80036F40(void);
extern void func_80016868(void);
extern void func_80045188(void);
extern s32 func_8008C464(s32, s32, s32);
extern void func_8003A39C(void);
extern void func_80080148(void);
extern void func_8007FF7C(void);
extern void func_800828CC(s32);
extern void func_80083B50(s32, s32, s32);
extern void func_80085448(s32, s32, s32);
extern void func_8003E2D8(s32, s32, s32, s32);
extern void func_8007B600(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern void func_80079A30(s32 *, s32, s32, s32);
extern void func_80078A28(s32 *);
extern void func_8003A728(s32 *);
extern void func_8003DE14(s32 *, s32);
extern void func_80016888(void);
extern void func_80061178(void);
extern void func_800168D0(void);
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
extern void func_80036940(void);
extern void func_8003D52C(s32, s32, s32, s32);
extern s32 D_800100A4;
extern void func_800415C4(s32);
extern void func_80016C74(void);
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
extern s32 func_800167D4(void);
extern void func_80041688(s32, s32);
extern void func_8003F1E4(s32);
extern s16 *func_8004678C(void);
extern void func_8003553C(void);
extern s32 func_80038C70(void);
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
extern void func_80016C3C(void);
extern s32 func_8005B9FC(s32);
extern s32 D_800A37C0;
extern s32 D_800A38B4;
extern u8 D_800A390D;
extern s32 func_80079120(s32 *, s32, s32);
extern void func_8005BA6C(s32);
extern s32 D_80104F38;
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);
extern void func_80048BA4(s32, s32, s32);
extern u8 D_80101EC8;

extern s32 func_8005B8B8(s32);
extern void func_8005B98C(s32);
extern void func_8005C6D0(void);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern u8 D_800A3906;
extern void func_800174F4(void);
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
extern u8 D_8008F13C;
extern s16 D_80101E74;

extern u8 D_800900EC;
extern u8 D_800A37C6;
extern s32 D_800A37A4;
extern s32 D_800A3844;
extern u8 D_800A376C;
extern u8 D_8008D578;
extern void func_80016C80(void);
extern void func_80040510(s32, s32, s32);
extern void func_8003AE5C(s32);
extern s32 func_80046F14(void);
extern s16 D_800A36A4;

extern u8 D_800A3768;
extern u8 D_800A38F8;
extern void func_8005B50C(void);
extern void func_80037774(void);
extern void func_80082AC0(void);
extern void func_80037348(s32 *);
extern void func_8007AE7C(s32);
extern void func_80078D68(void);
extern void func_80082BB4(void);
extern s32 func_800789B8(void);
extern void func_80078948(s32 *, s32, s32 *);
extern void func_80016A18(void);
extern void func_80016CF8(void);
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
extern s32 func_80036EA8(s32, s32);
extern s32 func_80036D98(s32, s32);
extern void func_80079154(s32);
extern void func_800325E0(s32, s32);
extern void func_80046BF4(s32 *, s32 *, s32);
extern s32 func_80046DEC(s32);
extern void func_8002EECC(s32, s32 *);
extern void func_80061064(s32 *, s32 *);
extern s32 func_8007E11C(s32);
extern void func_8007F87C(s32, s32);
extern void func_8007FA1C(s32, s32);
extern void func_8007FBBC(s32, s32);
extern s32 func_80053614(s32 *, s32 *, s32 *, s32 *);
extern u16 D_8008D59C;
extern u8 D_800F5F68;
extern s16 D_800973FC;
extern s32 D_80101FBC;
extern s32 D_80101FC4;
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_ASM("asm/funcs", func_80017FA0);
INCLUDE_ASM("asm/funcs", func_80018094);
INCLUDE_ASM("asm/funcs", func_80018300);
INCLUDE_ASM("asm/funcs", func_800187F4);
extern s32 D_800F6740;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0;
    s32 pad[2];
    s32 new_var;

    i = 0;
    if (arg1 <= 0) return;

    new_var = (s32)&D_800F6740;
    s0 = arg0;
    do {
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            func_800187F4(s0, (s32 *)(val * 52 + new_var));
        }
        i++;
        s0 = (s16 *)((u8 *)s0 + 16);
    } while (i < arg1);
}
INCLUDE_ASM("asm/funcs", func_80019310);
void func_8001945C(void) {
    D_80106A70 = 0x11;
    D_80106A71 = 0x44;
    D_80106A72 = 0x88;
}
s32 func_80019488(void) {
    return (D_80106A70 & 0xF) | ((D_80106A71 & 0xF) << 4) | ((D_80106A72 & 0xF) << 8);
}
void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}
void func_800194F4(void) {
    D_80102788 = 4;
    D_8010278A = 4;
    D_80102790 = 0;
    D_80102794 = 0;
    D_80102798 = 0;
    D_8010279C = -1;
}
void func_80019534(void) {
    func_800194F4();
    D_8010278C = 1;
    D_8010278E = 1;
}
INCLUDE_ASM("asm/funcs", func_80019568);
INCLUDE_ASM("asm/funcs", func_8001979C);
INCLUDE_ASM("asm/funcs", func_800198D0);
void func_8001A484(u16 *arg0) {
    s32 i;
    u16 *p;
    i = 0;
    p = arg0 + 2;
    do {
        i++;
        func_8003D52C((s32)&D_800100A4, arg0[0], p[-1], p[0]);
        p += 3;
        arg0 += 3;
    } while (i < 0x16);
}
s32 func_8001A4F0(s32 arg0, s32 arg1) {
    s32 v = arg0 & 0xFFF;
    if (v >= 0x800) {
        v -= 0x1000;
    }
    return v / arg1;
}
INCLUDE_ASM("asm/funcs", func_8001A538);
INCLUDE_ASM("asm/funcs", func_8001A62C);
INCLUDE_ASM("asm/funcs", func_8001A67C);
INCLUDE_ASM("asm/funcs", func_8001A820);
INCLUDE_ASM("asm/funcs", func_8001B138);
void func_8001B294(s32 *a0, s32 *a1) {    s32 v0;    D_800A36FA = 0;    D_800F6638 = 0x64;    D_800F663A = 0;    D_800F663C = 0x64;    D_800F6640 = 0x64;    D_800F6642 = 0;    D_800F6644 = 0x64;    func_8003F1E4(0);    D_800F6608 = (*(s32 *)((u8 *)a0 + 0xF4) + *(s32 *)((u8 *)a1 + 0xF4)) / 2;    D_800F660C = (*(s32 *)((u8 *)a0 + 0xF8) + *(s32 *)((u8 *)a1 + 0xF8)) / 2;    {        s32 t1 = *(s32 *)((u8 *)a0 + 0xFC);        s32 t2 = *(s32 *)((u8 *)a1 + 0xFC);        D_800F6618 = 0;        D_800F6610 = (t1 + t2) / 2;    }    {        s32 dx = *(s32 *)((u8 *)a1 + 0xF4) - *(s32 *)((u8 *)a0 + 0xF4);        s32 dy = *(s32 *)((u8 *)a1 + 0xFC) - *(s32 *)((u8 *)a0 + 0xFC);        v0 = func_8007FD5C(dx, dy);    }    D_800F661A = 0x400 - v0;    D_800F661C = 0;    D_800F6620 = 0x1388;    D_800F6626 = 0;}
void func_8001B3C0(s32 *a0, s32 *a1) {    D_800A36FA = 0;    D_800F5358 = 0x64;    D_800F535A = 0;    D_800F535C = 0x64;    D_800F5360 = 0x64;    D_800F5362 = 0;    D_800F5364 = 0x64;    func_8003F1E4(0);    if (D_800A36F6 != 0) {        a0 = a1;    }    D_800F5328 = *(s32 *)((u8 *)a0 + 0x180);    D_800F5330 = *(s32 *)((u8 *)a0 + 0x188);    {        s32 v = *(s32 *)((u8 *)a0 + 0x184);        D_800F5368 = 0;        D_800F532C = v;    }}
INCLUDE_ASM("asm/funcs", func_8001B478);
void func_8001B690(s32 arg0, s32 arg1) {
    if (D_800A38BA == 0) {
        return;
    }
    if (D_800A36F6 != arg0) {
        return;
    }
    arg1 &= 0xFFF;
    if (arg1 >= 0x800) {
        arg1 = 0x1000 - arg1;
    }
    if (arg1 >= 0x401) {
        D_800A36FC = 0x19;
        D_800F5344 = 0x800;
    }
}
void func_8001B6F4(void) {
    func_80041688(0, 0);
    func_80041688(1, 0);
    D_800A36FA = 1;
    D_800F6627 = 0;
    D_800F5347 = 0;
    func_8003F1E4(0);
}
INCLUDE_ASM("asm/funcs", func_8001B748);
INCLUDE_ASM("asm/funcs", func_8001BAE4);
void func_8001BBD8(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 temp_s0;
    temp_s0 = (D_800A387C < 0x2711) << 0xB;
    func_8001B748((s32 *)&D_800F5328, arg0, arg1, arg2, temp_s0, -0x200 - func_8007FD5C(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4), *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8)));
}
INCLUDE_ASM("asm/funcs", func_8001BC70);
INCLUDE_ASM("asm/funcs", func_8001BCF0);
void func_8001BE08(s32 *arg0) {
    arg0[2] = 0;
    arg0[3] = 0;
    arg0[4] = 0;
    arg0[5] = -1;
}
INCLUDE_ASM("asm/funcs", func_8001BE20);
void func_8001C444(void) {
    D_8010277A = 0x800;
    D_80102778 = 0x800;
    D_8010277C = 1;
    D_8010277D = 0x10;
    D_80102784 = 0xC;
    D_8010277E = 0;
    D_8010277F = 0;
    D_80102781 = 0;
    D_80102780 = 0;
    D_80102785 = 4;
    D_80102786 = 0;
    D_80102787 = 0;
}
extern u16 D_80101F32;
extern s16 D_80101F26;
void func_8001C4C0(void) {
    u16 v = D_80101F32;
    if (v == 0x32 || v == 0x11) {
        func_800218C8(0);
        {
            s32 v0 = func_80021974(0);
            D_80101F26 = 0;
            func_80021A98(0, v0, 0);
        }
    }
}
INCLUDE_ASM("asm/funcs", func_8001C51C);
INCLUDE_ASM("asm/funcs", func_8001C624);
INCLUDE_ASM("asm/funcs", func_8001C820);
INCLUDE_ASM("asm/funcs", func_8001C8DC);
INCLUDE_ASM("asm/funcs", func_8001CD68);
INCLUDE_ASM("asm/funcs", func_8001CE60);
INCLUDE_ASM("asm/funcs", func_8001D790);
void func_8001D904(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    func_80016868();
    func_80020D38();
    func_8005B9C4();
    s1 = func_8005B9FC((s32)0x80190800);
    if (s1 >= 0xE81) {
        func_80016C3C();
    }
    s0 = &D_80104F38;
    func_80079120(s0, (s32)0x80190800, s1);
    func_8005BA6C((s32)s0 - s2);
}
void func_8001D998(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    func_80016868();
    func_80020D38();
    func_8005B868();
    s1 = func_8005B8B8((s32)0x80190800);
    if (s1 >= 0x1B19) {
        func_80016C3C();
    }
    s0 = &D_80104F38;
    func_80079120(s0, (s32)0x80190800, s1);
    func_8005B98C((s32)s0 - s2);
}
void func_8001DA2C(void) {
    func_8005B5AC();
    func_8005BF3C();
    if (D_800A38DC == 5) {
        func_8005B9C4();
    }
    if (D_800A38DC == 3) {
        func_8005B868();
    }
}
INCLUDE_ASM("asm/funcs", func_8001DA8C);
s32 func_8001DB58(void) {
    s32 v = D_800A38DC;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return func_800167D4();
    }
    return 1;
}
void func_8001DB9C(void) {
    func_800450BC((&D_8008D9EC)[D_80101ED2] < 1, (s32)0x80190800);
    D_800A38C6 = (u16)0xFFFF;
}
void func_8001DBE4(void) {
    s32 i;

    if (D_800A3768 != 0x14) {
        return;
    }
    func_8003AA78();
    if (D_800A38F8 > D_800A37A0) {
    } else {
        do {
            func_8003AA48();
            func_800174F4();
            func_800828CC(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            func_8003AA48();
            i += 1;
            func_800174F4();
            func_800828CC(2);
        } while (i < 15);
    }
    func_8003AAB0();
    func_80016888();
    func_800168D0();
}
INCLUDE_ASM("asm/funcs", func_8001DCB0);
INCLUDE_ASM("asm/funcs", func_8001E404);
INCLUDE_ASM("asm/funcs", func_8001E6E4);
void func_8001E800(void) {
    s32 v = D_800A36F6;
    volatile u8 *ptr = (volatile u8 *)(&D_80101EC8 + v * 1100);
    s32 a1 = -1;
    if (ptr[0x62] & 1) {
        a1 = *(s16 *)(ptr + 0xE);
    }
    {
        u32 flags = ptr[0x62] & 4;
        func_80048BA4(D_800F5344, a1, flags > 0);
    }
}
INCLUDE_ASM("asm/funcs", func_8001E878);
void func_8001EA04(void) {
    u8 v;
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_80061178();
    v = D_800A38D4;
    D_8010262E = 0;
    D_801021E2 = 0;
    D_800A37B8 = 0;
    D_800A3929 = 0;
    D_800A3834 = 0xD;
    D_800A3804 = v < 1;
    D_800A3817 = v < 1;
}
INCLUDE_ASM("asm/funcs", func_8001EA84);
INCLUDE_ASM("asm/funcs", func_8001EEB4);
INCLUDE_ASM("asm/funcs", func_8001EFA0);
void func_8001F1C4(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3) {
    s16 temp_v1;
    if (!(*(u8 *)(arg1 + 0x18) & 0x80)) {
        func_80027334((s32 *)arg2);
        func_80027334((s32 *)arg3);
    }
    func_8002F770((s32 *)(arg2 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    func_8002F770((s32 *)(arg3 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    temp_v1 = *(s16 *)(arg0 + 0xC);
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        *(u16 *)(arg2 + 0x7E) = (u16)(*(u16 *)(arg2 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x7E) = (u16)(*(u16 *)(arg3 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
    }
    if ((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) {
        *(u16 *)(arg2 + 0x72) = (u16)(*(u16 *)(arg2 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x72) = (u16)(*(u16 *)(arg3 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
    }
}
INCLUDE_ASM("asm/funcs", func_8001F2E4);
void func_8001F860(s16 *arg0, s32 arg1) {
    arg1 = (arg1 - *(s16 *)((u8 *)arg0 + 0x1CA)) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    *(s16 *)((u8 *)arg0 + 0x14C) = arg1;
}
s32 func_8001F888(void) {
    s32 dx = D_80102408 - D_80101FBC;
    s32 dy = D_80102410 - D_80101FC4;
    s32 s0 = 0;
    while ((u32)(dx + 0x4000) > 0x8000 || (u32)(dy + 0x4000) > 0x8000) {
        s32 t;
        t = dx + ((u32)dx >> 31);
        dx = (s32)t >> 1;
        t = dy + ((u32)dy >> 31);
        dy = (s32)t >> 1;
        s0 += 1;
    }
    {
        s32 v0 = dx * dx;
        s32 v1 = dy * dy;
        s32 r = func_8007E11C(v0 + v1);
        return r << s0;
    }
}
INCLUDE_ASM("asm/funcs", func_8001F938);
INCLUDE_ASM("asm/funcs", func_8001FAE4);
INCLUDE_ASM("asm/funcs", func_8001FB34);
INCLUDE_ASM("asm/funcs", func_8001FBE8);
s32 func_8002006C(void) {
    s32 s0 = D_800A387C;
    s32 v0 = func_8001F888();
    s32 v = D_800A38DC;
    D_800A387C = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        func_8001FBE8();
    }
    D_800A38A8 = 0;
}
INCLUDE_ASM("asm/funcs", func_800200DC);
INCLUDE_ASM("asm/funcs", func_800203B4);
INCLUDE_ASM("asm/funcs", func_800204C0);
INCLUDE_ASM("asm/funcs", func_800206B0);
INCLUDE_ASM("asm/funcs", func_800207C8);
void func_80020CDC(void) {
    if (D_800A38C6 == 0xFFFF) {
        func_80045188();
    }
    D_800A3880 = 0;
    D_800A38C6 = 0;
    D_800A38C4 = 0;
    D_800A38C1 = 0xFF;
    D_800A38C0 = 0xFF;
}
void func_80020D38(void) {
    if (D_800A38C6 == 0xFFFF) {
        func_80045188();
    }
    D_800A38C6 = 0;
}
extern s32 D_800A388C;
extern s32 D_800A3888;
extern s32 D_800A3830;
extern s32 D_800A3860;
extern s32 D_800A3864;

void func_80020D70(void) {
    D_800A3888 = (s32)0x80118800;
    D_800A388C = (s32)0x8011C400;
    D_800A3830 = (s32)0x80120000;
    D_800A3860 = (s32)0x80148800;
    D_800A3864 = (s32)0x80190800;
    func_80020CDC();
}
void func_80020DDC(void) {    s32 v0;    s32 v1;    s32 v2;    v0 = func_80036EA8(1, 1);    func_80036D98(v0, D_800A3830);    func_80036F40();    v1 = D_800A3830;    D_80102760 = v1 + 0x14;    D_80102764 = v1 + *(s32 *)(v1 + 4);    D_80102768 = v1 + *(s32 *)(v1 + 8);    v2 = *(s32 *)(v1 + 0x10);    D_800A3880 = 1;    D_80102770 = v1 + v2;}
INCLUDE_ASM("asm/funcs", func_80020E74);
extern s32 D_80102770;
extern u16 D_800A38C4;
extern s32 D_801027C0;
extern u16 D_800A38C6;
extern s32 D_801027D4;
void func_80021210(void) {
    func_8001979C(0, D_80102770);
    if (D_800A38C4) {
        func_8001979C(1, D_801027C0);
    }
    if (D_800A38C6) {
        func_8001979C(2, D_801027D4);
    }
}
INCLUDE_ASM("asm/funcs", func_80021280);
void func_800213A0(s16 *arg0) {
    s16 a1 = arg0[0x86 / 2];
    if (a1 != arg0[0x88 / 2]) {
        if (a1 != arg0[0x8E / 2]) {
            return;
        }
    }
    {
        s16 *v = (s16 *)(&D_800A3860)[arg0[0x4A / 2]];
        arg0[0x86 / 2] = (s16)((a1 + 1) % v[0x14 / 2]);
    }
}
INCLUDE_ASM("asm/funcs", func_80021424);
extern u16 D_80101F4C;
extern u16 D_80101F4E;
void func_800218C8(s32 a0) {
    s32 offset = a0 * 1100;
    *(u16 *)((u8 *)&D_80101F4E + offset) = *(u16 *)((u8 *)&D_80101F4C + offset);
}
extern s16 D_80101F4E;
s32 func_80021904(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4E + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
extern s16 D_80101F12;
extern s16 D_80101F4C;
extern s32 D_800A3860;
extern s32 D_801027B0;
s32 func_80021974(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4C + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
extern s32 D_80102760;
s32 func_800219E4(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + 0x16);
    return D_80102760 + idx * 2;
}
s32 func_80021A3C(s32 a0, s32 a1) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + a1 * 2 + 0x18);
    return D_80102760 + idx * 2;
}
INCLUDE_ASM("asm/funcs", func_80021A98);
void func_80021D10(s32 arg0, s32 *arg1, s32 arg2) {
    s16 *temp_v0;
    temp_v0 = (s16 *)(func_80046F14() + (((D_800A36A4 * 0x18) + (arg2 * 6) + (arg0 * 3)) * 2));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}
INCLUDE_ASM("asm/funcs", func_80021DB0);
INCLUDE_ASM("asm/funcs", func_80022224);
INCLUDE_ASM("asm/funcs", func_80022408);
INCLUDE_ASM("asm/funcs", func_800224E0);
void func_80022568(s16 *arg0) {
    arg0[0x136] = 1;
    arg0[0x137] = 0;
    arg0[0x138] = 0;
    arg0[0x139] = 0;
}
INCLUDE_ASM("asm/funcs", func_80022580);
INCLUDE_ASM("asm/funcs", func_80022F34);
INCLUDE_ASM("asm/funcs", func_8002304C);
INCLUDE_ASM("asm/funcs", func_800233AC);
INCLUDE_ASM("asm/funcs", func_80023648);
INCLUDE_ASM("asm/funcs", func_800238C4);
void func_80023C30(s32 arg0, s32 arg1, s32 arg2, s16 *arg3) {
    arg3[0] = 0x1000;
    arg3[1] = 0;
    arg3[2] = 0;
    arg3[3] = 0;
    arg3[4] = 0x1000;
    arg3[5] = 0;
    arg3[6] = 0;
    arg3[7] = 0;
    arg3[8] = 0x1000;
    func_8007F87C(arg0, (s32)arg3);
    func_8007FA1C(arg1, (s32)arg3);
    func_8007FBBC(arg2, (s32)arg3);
}
void func_80023CB4(s16 *arg0, s16 arg1) {
    s16 v;
    *(u16 *)((u8 *)arg0 + 0x31A) += 1;
    v = *(s16 *)((u8 *)arg0 + 0x31A);
    if (v == 1) {
        *(s16 *)((u8 *)arg0 + 0x318) = arg1;
        *(s32 *)((u8 *)arg0 + 0x320) = 0;
        *(s32 *)((u8 *)arg0 + 0x328) = 0;
    }
    *(s16 *)((u8 *)arg0 + 0x31C) = arg1;
    if (*(s16 *)((u8 *)arg0 + 0x152) == 0) {
        *(s16 *)((u8 *)arg0 + 0x152) = 1;
        *(s16 *)((u8 *)arg0 + 0x154) = *(u16 *)((u8 *)arg0 + 0x1D8);
    }
}
void func_80023D08(s32 arg0) {
    func_80023CB4(arg0, 0x200);
}
void func_80023D28(u8 *arg0) {
    if (*(s32 *)(arg0 + 0x108) < 0) {
        *(s16 *)(arg0 + 0x1DC) = 0;
        return;
    }
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 0x1F4;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    *(s16 *)(arg0 + 0x1DC) = func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8);
}
s32 func_80023DB8(u8 *arg0) {
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 5;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    if (func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8) != 0) {
        if ((arg0 && arg0) && arg0) {
        }
        return *(s16 *)0x1F8001C2 < -0x800;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80023E40);
INCLUDE_ASM("asm/funcs", func_80023F08);
