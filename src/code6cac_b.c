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
extern void func_80016868(void);
extern void func_800194F4(void);
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
extern void func_80021974(s32);
extern void func_80021A98(s32, s32, s32);
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
extern void func_8001B748(s32 *, s32 *, s32 *, s32 *, s32, s32);
extern u8 D_8008F13C;
extern s16 D_80101E74;

extern u8 D_800900EC;
extern u8 D_800A37C6;
extern s32 D_800A37A4;
extern s32 D_800A3844;
extern u8 D_800A376C;
extern u8 D_8008D578;
extern void func_80016C80(void);
extern void func_80022580(s32, s32, s32, s32, s32);
extern void func_80040510(s32, s32, s32);
extern void func_8003AE5C(s32);
extern void func_80046F14(void);
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
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_ASM("asm/funcs", func_80026DA4);
s32 func_800272FC(s32 a0) {
    s32 v1;
    if (a0 < 0) {
        a0 = -a0;
    }
    if (a0 < 0xA01) {
        if (a0 < 0x600) {
            v1 = 1;
        } else {
            v1 = 2;
        }
    } else {
        v1 = 0;
    }
    return v1;
}
void func_80027334(s16 *arg0) {
    arg0[0x1E] = 0x3F5;
    arg0[0x1F] = 0x2B6;
    arg0[0x20] = 0x77A;
    arg0[0x21] = 0xBEE;
    arg0[0x22] = 0x8C;
    arg0[0x23] = 0x227;
    arg0[0x27] = 0x8A0;
    arg0[0x28] = 0xF0E;
    arg0[0x24] = 0;
    arg0[0x25] = 0;
    arg0[0x26] = 0;
    arg0[0x29] = 0xBCD;
}
INCLUDE_ASM("asm/funcs", func_8002738C);
INCLUDE_ASM("asm/funcs", func_80027438);
INCLUDE_ASM("asm/funcs", func_800274BC);
INCLUDE_ASM("asm/funcs", func_80027640);
INCLUDE_ASM("asm/funcs", func_800278C0);
s32 func_8002798C(u8 *a0) {
    s32 ret = 0;
    u16 v1 = *(u16 *)(a0 + 0x6A);
    
    if (v1 == 6 || v1 == 0x25 || v1 == 0x33 || v1 == 4 || v1 == 0x14) {
        goto check_88;
    }
    {
        s16 v40 = *(s16 *)(a0 + 0x40);
        if (v40 < *(u8 *)(a0 + 0xA5)) {
            goto check_88;
        }
        if (!(*(u8 *)(a0 + 0xA6) < v40)) {
            goto final_1;
        }
    }
    
check_88:
    {
        s16 v88 = *(s16 *)(a0 + 0x88);
        s32 a0_6a;
        if (v88 == -1) {
            goto done;
        }
        a0_6a = *(u16 *)(a0 + 0x6A);
        if ((u16)a0_6a == 0xF) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x1C) < 2) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x1E) < 2) {
            goto final_1;
        }
        if ((u32)(a0_6a - 0x20) < 2) {
            goto final_1;
        }
        if ((u16)a0_6a == 0x2B) {
            goto final_1;
        }
        goto done;
    }
    
final_1:
    ret = 1;
done:
    return ret;
}





extern s32 func_80046DEC(s32);
void func_80027A58(s32 *a0) {
    s16 v1 = *(s16 *)((u8 *)a0 + 0x86);
    if (v1 == *(s16 *)((u8 *)a0 + 0x88)) {
        if (*(s16 *)((u8 *)a0 + 0x8A)) {
            if (func_8002798C()) {
                s32 v0 = func_80046DEC(*(s16 *)((u8 *)a0 + 4));
                func_80030900(a0, *(s32 *)(v0 + 0x4C) + 0x14);
                *(s16 *)((u8 *)a0 + 0x8A) = 0;
                *(s16 *)((u8 *)a0 + 0x86) = *(u16 *)((u8 *)a0 + 0x84);
            }
        }
    }
}
INCLUDE_ASM("asm/funcs", func_80027AD8);
INCLUDE_ASM("asm/funcs", func_800283D0);
INCLUDE_ASM("asm/funcs", func_8002872C);
INCLUDE_ASM("asm/funcs", func_800288C8);
void func_8002906C(void) {
    s16 *ptr = func_8004678C();
    while (*(s16 *)ptr != 0) {
        *(s16 *)((u8 *)ptr + 2) = 0;
        ptr = (s16 *)((u8 *)ptr + 0x10);
    }
}
INCLUDE_ASM("asm/funcs", func_800290B8);
INCLUDE_ASM("asm/funcs", func_80029454);
INCLUDE_ASM("asm/funcs", func_8002A458);
INCLUDE_ASM("asm/funcs", func_8002AB08);
INCLUDE_ASM("asm/funcs", func_8002BC68);
INCLUDE_ASM("asm/funcs", func_8002BEA0);
INCLUDE_ASM("asm/funcs", func_8002C0DC);
INCLUDE_ASM("asm/funcs", func_8002C22C);
INCLUDE_ASM("asm/funcs", func_8002C61C);
INCLUDE_ASM("asm/funcs", func_8002CA8C);
INCLUDE_ASM("asm/funcs", func_8002CD58);
INCLUDE_ASM("asm/funcs", func_8002D320);
INCLUDE_ASM("asm/funcs", func_8002D518);
INCLUDE_ASM("asm/funcs", func_8002D780);
INCLUDE_ASM("asm/funcs", func_8002DAD0);
INCLUDE_ASM("asm/funcs", func_8002DE20);
INCLUDE_ASM("asm/funcs", func_8002E6B0);
INCLUDE_ASM("asm/funcs", func_8002E838);
INCLUDE_ASM("asm/funcs", func_8002EA24);
INCLUDE_ASM("asm/funcs", func_8002EBDC);
void func_8002EECC(void *arg0, void *arg1) {
    s16 temp_a3;
    s16 temp_t0;
    s16 temp_t1;
    s16 temp_t2;
    s16 temp_v0;
    s16 temp_v1;
    s32 temp_a2;
    s32 temp_v1_2;

    temp_t2 = *(s16 *)((u8 *)arg0 + 0xA);
    temp_t1 = *(s16 *)((u8 *)arg0 + 0xE);
    temp_t0 = *(s16 *)((u8 *)arg0 + 8);
    temp_a3 = *(s16 *)((u8 *)arg0 + 0x10);
    temp_a2 = (temp_t2 * temp_t1) - (temp_t0 * temp_a3);
    temp_v1 = *(s16 *)((u8 *)arg0 + 2);
    temp_v0 = *(s16 *)((u8 *)arg0 + 4);
    temp_v1_2 = (s32) ((*(s16 *)((u8 *)arg0 + 0) * (temp_a2 >> 0xC)) + (*(s16 *)((u8 *)arg0 + 6) * ((s32) ((temp_v1 * temp_a3) - (temp_v0 * temp_t1)) >> 0xC)) + (*(s16 *)((u8 *)arg0 + 0xC) * ((s32) ((temp_v0 * temp_t0) - (temp_v1 * temp_t2)) >> 0xC))) >> 0xC;
    *(s16 *)((u8 *)arg1 + 0) = (s16) (temp_a2 / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 2) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0x10)) - (*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 0xE))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 4) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 8)) - (*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0xA))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 6) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 6) * *(s16 *)((u8 *)arg0 + 0x10)) - (*(s16 *)((u8 *)arg0 + 0xA) * *(s16 *)((u8 *)arg0 + 0xC))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 8) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 0xC)) - (*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0x10))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xA) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0xA)) - (*(s16 *)((u8 *)arg0 + 4) * *(s16 *)((u8 *)arg0 + 6))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xC) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 8) * *(s16 *)((u8 *)arg0 + 0xC)) - (*(s16 *)((u8 *)arg0 + 6) * *(s16 *)((u8 *)arg0 + 0xE))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0xE) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 0xE)) - (*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 0xC))) / temp_v1_2);
    *(s16 *)((u8 *)arg1 + 0x10) = (s16) ((s32) ((*(s16 *)((u8 *)arg0 + 2) * *(s16 *)((u8 *)arg0 + 6)) - (*(s16 *)((u8 *)arg0 + 0) * *(s16 *)((u8 *)arg0 + 8))) / temp_v1_2);
}
INCLUDE_ASM("asm/funcs", func_8002F2D0);
INCLUDE_ASM("asm/funcs", func_8002F770);
INCLUDE_ASM("asm/funcs", func_8002FC80);
INCLUDE_ASM("asm/funcs", func_8002FDB0);
INCLUDE_ASM("asm/funcs", func_8002FF20);
INCLUDE_ASM("asm/funcs", func_800300B4);
INCLUDE_ASM("asm/funcs", func_80030208);
INCLUDE_ASM("asm/funcs", func_8003032C);
void func_8003043C(void) {
    s32 i = 0;
    s16 neg = -1;
    u8 val = 0xFF;
    s32 off = 0;
    do {
        *(s16 *)((u8 *)&D_80106A7A + off) = neg;
        *(u8 *)((u8 *)&D_80106A82 + off) = val;
        off += 0x64;
    } while (++i < 12);
}
extern u8 D_8008E338;
extern u8 D_800A36F2;
extern u8 D_8008E338;
extern u8 D_800A36F2;
extern u8 D_8008E338;
extern u8 D_800A36F2;

void func_8003047C(u8 *a0) {
    s32 i;
    u8 *table;
    s32 neg1;
    u8 *a2;
    s16 val;
    s16 idx;

    *(u16 *)(a0 + 0x330) = 0;
    i = 0;
    table = &D_8008E338;
    neg1 = -1;
    a2 = a0;
    goto loop_start;
loop_start:
    idx = *(s16 *)(a0 + 0xA);
    val = (s8)*(table + idx * 5 + i);
    *(s16 *)(a2 + 0x332) = val;
    if (val != neg1) {
        u16 cnt = *(u16 *)(a0 + 0x330);
        *(u16 *)(a0 + 0x330) = cnt + 1;
    }
    i++;
    if (i < 5) {
        a2 += 2;
        goto loop_start;
    }

    idx = *(s16 *)(a0 + 0xA);
    {
        s16 idx2 = *(s16 *)(a0 + 0x4);
        *(&D_800A36F2 + idx2) = *(&D_8008E338 + idx * 5);
    }
}






void func_80030524(void) {
    s32 i = 0;
    s32 neg = -1;
    s16 *p = &D_80106A7A;
    s32 off = 0;
    do {
        if (*p != neg) {
            if (*(u8 *)((u8 *)&D_80106A80 + off) != 0) {
                *p = neg;
            }
        }
        p = (s16 *)((u8 *)p + 0x64);
        off += 0x64;
    } while (++i < 12);
}
INCLUDE_ASM("asm/funcs", func_80030580);
INCLUDE_ASM("asm/funcs", func_800307D0);
INCLUDE_ASM("asm/funcs", func_80030900);
extern s32 func_800171B8(void);
extern s32 *func_80030580(s32 *a0, s16 a1);
extern s32 func_800171B8(void);
extern s32 *func_80030580(s32 *a0, s32 a1);
extern s32 func_800171B8(void);
extern s32 *func_80030580(s32 *a0, s32 a1);
extern s32 func_800171B8(void);
extern s32 *func_80030580(s32 *a0, s32 a1);
extern s32 func_800171B8(void);
extern s32 *func_80030580(s32 *a0, s32 a1);
INCLUDE_ASM("asm/funcs", func_80030A2C);





INCLUDE_ASM("asm/funcs", func_80030B10);

INCLUDE_ASM("asm/funcs", func_80030BA8);
void func_80030D04(void) {
    s32 i = 0;
    s32 neg = -1;
    u16 *p = (u16 *)&D_80106A7A;
    do {
        if ((u32)(*p - 0x12) < 12u) {
            *p = neg;
        }
        p = (u16 *)((u8 *)p + 0x64);
    } while (++i < 12);
}
void func_80030D48(void) {
}
s32 func_80030D50(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = (arg0 - arg1) & 0xFFF;
    if (arg0 >= 0x800) {
        arg0 -= 0x1000;
    }
    return arg1 + ((arg0 * arg2) >> 12);
}
INCLUDE_ASM("asm/funcs", func_80030D7C);
INCLUDE_ASM("asm/funcs", func_80031890);
INCLUDE_ASM("asm/funcs", func_80031B24);
void func_80032040(void) {
    s32 i;
    for (i = 0x84; i >= 0; i -= 0x2C) {
        (&D_80104E88)[i] = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_80032064);
INCLUDE_ASM("asm/funcs", func_800321E8);
INCLUDE_ASM("asm/funcs", func_80032314);
INCLUDE_ASM("asm/funcs", func_800324D0);
INCLUDE_ASM("asm/funcs", func_800325E0);
INCLUDE_ASM("asm/funcs", func_80032854);
INCLUDE_ASM("asm/funcs", func_80032C50);
INCLUDE_ASM("asm/funcs", func_8003339C);
INCLUDE_ASM("asm/funcs", func_80033498);
extern s16 D_800A3756;
extern u8 D_800A391D;
void func_80033510(void) {
    s32 i = 3;
    s16 *p1 = &D_800A3756;
    do {
        *p1 = 0;
        i--;
        p1--;
    } while (i >= 0);
    i = 5;
    {
        u8 *p2 = &D_800A391D;
        do {
            *p2 = 0;
            i--;
            p2--;
        } while (i >= 0);
    }
}
void func_80033550(s32 *arg0)
{
  int new_var;
  register s32 *a3_arg asm("a3") = arg0;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  do { idx = new_var; w0 = a3_arg[0]; } while (0);
  w1 = a3_arg[1];
  w2 = a3_arg[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}

INCLUDE_ASM("asm/funcs", func_800335D8);
void func_80033898(void) {
    func_80016868();
    D_800A37B8 = 0;
    D_800A3834 = 3;
}
INCLUDE_ASM("asm/funcs", func_800338CC);
INCLUDE_ASM("asm/funcs", func_80033BC0);
INCLUDE_ASM("asm/funcs", func_80033D38);
INCLUDE_ASM("asm/funcs", func_80033DF4);
void func_80033FE4(void) {
    s32 v1;
    if (D_800A36F0 != 0) {
        v1 = 6;
        if (*((u8 *)&D_8008D9EC + D_80101ED2) != 0) {
            v1 = 7;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A3781 != 0) {
        v1 = 8;
        if (*((u8 *)&D_8008D9EC + D_80101ED2) != 0) {
            v1 = 9;
        }
        D_800A38A4 = v1;
        D_800A3834 = 0x12;
        return;
    }
    if (D_800A38E9 < 3) {
        D_800A3834 = 0x1A;
    } else {
        D_800A3834 = 8;
    }
}


INCLUDE_ASM("asm/funcs", func_800340A0);
INCLUDE_ASM("asm/funcs", func_80034200);
extern void func_80034200(void);
extern void func_80016888(void);
extern void func_800372C0(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
extern u8 D_8010277C;
extern u8 D_8010277E;
extern void func_80034200(void);
extern void func_80016888(void);
extern void func_800372C0(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
extern u8 D_8010277C;
extern u8 D_8010277E;
void func_800342A0(void) {
    func_80034200();
    if (D_800A3874 == D_800A389B) {
        D_800A3768 = 0xFF;
        D_800A36A8 = 0;
        D_800A3834 = 0x14;
    } else {
        s32 v1;
        u8 *a1 = &D_801027A0;
        u8 *a0 = &D_801027D8;
        D_800A38F4 = 0;
        D_800A3899 = 0;
        D_800A3898 = 0;
        D_800A38AB = 0;
        D_800A38AA = 0;
        D_800A3816 = 0;
        D_800A391E = 0x50;
        D_800A381E = 0;
        D_800A37E1 = 0;
        D_800A36E8 = 0;
        D_800A38B8 = 0;
        D_800A3920 = 0;
        v1 = 0;
loop:
        {
            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a1;
            v0 = *((u8 *)v0 + v1);
            *((u8 *)&D_8010277C + v1) = v0;
        }
        {
            s32 v0 = D_800A3874;
            v0 = v0 << 1;
            v0 = v0 + (s32)a0;
            v0 = *((u8 *)v0 + v1);
            *((u8 *)&D_8010277E + v1) = v0;
        }
        v1++;
        if (v1 < 2) goto loop;
        func_80016888();
        func_800372C0();
        D_800A3834 = 0;
    }
}


INCLUDE_ASM("asm/funcs", func_800343F0);



INCLUDE_ASM("asm/funcs", func_800344B4);
INCLUDE_ASM("asm/funcs", func_80034708);
INCLUDE_ASM("asm/funcs", func_80034F88);
INCLUDE_ASM("asm/funcs", func_8003504C);
INCLUDE_ASM("asm/funcs", func_80035280);
void func_80035430(void) {
}
