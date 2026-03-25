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

INCLUDE_ASM("asm/funcs", func_800375EC);
INCLUDE_ASM("asm/funcs", func_80037774);
INCLUDE_ASM("asm/funcs", func_80037804);
INCLUDE_ASM("asm/funcs", func_800378A8);
INCLUDE_ASM("asm/funcs", func_8003791C);
INCLUDE_ASM("asm/funcs", func_80037964);
INCLUDE_ASM("asm/funcs", func_800379D8);
INCLUDE_ASM("asm/funcs", func_80037A20);
INCLUDE_ASM("asm/funcs", func_80037AA4);
INCLUDE_ASM("asm/funcs", func_80037B00);
INCLUDE_ASM("asm/funcs", func_80037B90);
INCLUDE_ASM("asm/funcs", func_80037C34);
INCLUDE_ASM("asm/funcs", func_80037D14);
extern void func_80079A30(s32 *, s32 *, s32, s32);
extern void func_80078A28(s32 *);
extern s32 D_800109C8;
void func_80037F08(s32 a0, s32 a1) {
    s32 buf[2];
    func_80079A30(buf, &D_800109C8, a0, a1);
    func_80078A28(buf);
}
INCLUDE_ASM("asm/funcs", func_80037F40);
INCLUDE_ASM("asm/funcs", func_8003800C);
extern u8 D_800F33D8;
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
INCLUDE_ASM("asm/funcs", func_800383A4);
INCLUDE_ASM("asm/funcs", func_80038658);
INCLUDE_ASM("asm/funcs", func_80038734);
INCLUDE_ASM("asm/funcs", func_8003877C);
INCLUDE_ASM("asm/funcs", func_8003879C);
INCLUDE_ASM("asm/funcs", func_800387C0);
INCLUDE_ASM("asm/funcs", func_800387E8);
INCLUDE_ASM("asm/funcs", func_8003880C);
INCLUDE_ASM("asm/funcs", func_800388A8);
INCLUDE_ASM("asm/funcs", func_80038988);
INCLUDE_ASM("asm/funcs", func_80038C70);
s32 *func_800392B8(void) {
    return &D_800F33D8;
}
INCLUDE_ASM("asm/funcs", func_800392C8);
INCLUDE_ASM("asm/funcs", func_80039320);
INCLUDE_ASM("asm/funcs", func_800393C8);
INCLUDE_ASM("asm/funcs", func_800395B4);
INCLUDE_ASM("asm/funcs", func_80039680);
INCLUDE_ASM("asm/funcs", func_800397A0);
INCLUDE_ASM("asm/funcs", func_800397D4);
INCLUDE_ASM("asm/funcs", func_8003984C);
INCLUDE_ASM("asm/funcs", func_8003993C);
INCLUDE_ASM("asm/funcs", func_8003A174);
INCLUDE_ASM("asm/funcs", func_8003A264);
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
INCLUDE_ASM("asm/funcs", func_8003A360);
INCLUDE_ASM("asm/funcs", func_8003A39C);
void func_8003A3F0(void) {
    func_8003A39C();
    D_800A3928 = 1;
}
INCLUDE_ASM("asm/funcs", func_8003A41C);
INCLUDE_ASM("asm/funcs", func_8003A42C);
INCLUDE_ASM("asm/funcs", func_8003A450);
INCLUDE_ASM("asm/funcs", func_8003A574);
INCLUDE_ASM("asm/funcs", func_8003A5A0);
s32 func_8003A6FC(u32 arg0) {
    s32 count = 0;
    s32 i;
    for (i = 0; i < 32; i++) {
        count += (arg0 >> i) & 1;
    }
    return count;
}
INCLUDE_ASM("asm/funcs", func_8003A728);
extern void func_8003A728(s32);

void func_8003AA48(void) {
    s16 buf[12];
    *(s32 *)&buf[4] = 0;
    buf[1] = 4;
    buf[0] = 4;
    func_8003A728((s32)buf);
}
INCLUDE_ASM("asm/funcs", func_8003AA78);
INCLUDE_ASM("asm/funcs", func_8003AAB0);
INCLUDE_ASM("asm/funcs", func_8003AB44);
INCLUDE_ASM("asm/funcs", func_8003ACB8);
INCLUDE_ASM("asm/funcs", func_8003AE5C);
void func_8003AF40(s32 arg0) {
    if ((&D_8010277E)[arg0] == 0xFF) {
        (&D_8010277E)[arg0] = (&D_8010277E)[(u32)arg0 < 1u];
    }
    func_80022580(arg0, ((s8 *)&D_80102780)[arg0], ((s8 *)&D_8010277C)[arg0], ((s8 *)&D_8010277E)[arg0], 0);
    func_80016868();
    func_80020D38();
    func_80040510(arg0, (&D_8008D578)[(s8)(&D_8010277C)[arg0]], (s32)0x80190800);
}
INCLUDE_ASM("asm/funcs", func_8003AFFC);
INCLUDE_ASM("asm/funcs", func_8003B10C);
INCLUDE_ASM("asm/funcs", func_8003B20C);
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
INCLUDE_ASM("asm/funcs", func_8003B328);
INCLUDE_ASM("asm/funcs", func_8003B3A4);
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
    func_80016888();
    func_80016768(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    func_800168D0();
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
        func_8001D790();
        func_8003B5A4();
        func_80016768(1, 0, 0, 0);
        D_800A390D = 1;
    }
}
INCLUDE_ASM("asm/funcs", func_8003B9D0);
INCLUDE_ASM("asm/funcs", func_8003BCB4);
extern void func_80016868(void);
extern void func_80016888(void);
extern void func_800415C4(s32);
extern void func_80016C74(void);
extern void func_8005B72C(void);
extern void func_80078824(s32);
extern void func_80035FA8(void);
extern s32 func_80036EA8(s32, s32);
extern void func_80036FD4(s32, s32);
extern void func_80037260(void);
extern s16 D_800A3834;
extern void func_800168D0(void);
void func_8003BE10(void) {
    func_80016868();
    func_80016888();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    func_80016C74();
    func_8005B72C();
    func_80078824((s32)0x80118800);
    func_80035FA8();
    {
        s32 v0 = func_80036EA8(5, 0x20);
        func_80036FD4(v0, 4);
    }
    func_80037260();
    D_800A3834 = 0xB;
    func_800168D0();
}
INCLUDE_ASM("asm/funcs", func_8003BEA8);
void func_8003BFC4(void) {
    s32 v;
    func_80016868();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    func_80016C74();
    v = func_80045814();
    func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    func_80046B44();
    D_800A3834 = 8;
}
INCLUDE_ASM("asm/funcs", func_8003C040);
INCLUDE_ASM("asm/funcs", func_8003C2C0);
INCLUDE_ASM("asm/funcs", func_8003C42C);
INCLUDE_ASM("asm/funcs", func_8003C560);
INCLUDE_ASM("asm/funcs", func_8003C714);
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
    func_80016888();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    func_800168D0();
}
INCLUDE_ASM("asm/funcs", func_8003C9A4);
void func_8003CCCC(void) {
    func_80016888();
    func_80061178();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    func_800168D0();
}
INCLUDE_ASM("asm/funcs", func_8003CD10);
INCLUDE_ASM("asm/funcs", func_8003CE18);
INCLUDE_ASM("asm/funcs", func_8003CF84);
void func_8003D2C4(void) {
    func_8007B600((s32)&D_800A3220, (s32)&D_80090178);
}
INCLUDE_ASM("asm/funcs", func_8003D2F4);
INCLUDE_ASM("asm/funcs", func_8003D330);
INCLUDE_ASM("asm/funcs", func_8003D39C);
INCLUDE_ASM("asm/funcs", func_8003D478);
INCLUDE_ASM("asm/funcs", func_8003D52C);
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
INCLUDE_ASM("asm/funcs", func_8003D7B4);
INCLUDE_ASM("asm/funcs", func_8003D888);
extern s32 D_800A3D70;
extern s32 D_800A4340;
extern void func_8003D9A0(s16 *, s32, s32 *);
void func_8003D91C(void) {
    s16 buf[4];
    s16 s1 = 0x1E0;
    s16 s0 = 0x140;
    buf[0] = 0;
    buf[1] = s1;
    buf[2] = s0;
    buf[3] = 1;
    func_8003D9A0(buf, 0x1F, &D_800A3D70);
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
extern void func_8003DE14(s16 *, s32);
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
INCLUDE_ASM("asm/funcs", func_8003E164);
INCLUDE_ASM("asm/funcs", func_8003E22C);
INCLUDE_ASM("asm/funcs", func_8003E2A0);
extern u16 D_800F6656;
void func_8003E2AC(void) {
    u16 *p = &D_800F6656;
    *p = *p & 0xFFFD;
}
u32 func_8003E2C8(void) {
    return D_800905F8;
}
INCLUDE_ASM("asm/funcs", func_8003E2D8);
void func_8003E6A0(s32 arg0, s32 arg1) {
    func_8003E2D8(D_80101E3C, D_80101E44, arg0, arg1);
}
INCLUDE_ASM("asm/funcs", func_8003E6D8);
INCLUDE_ASM("asm/funcs", func_8003EB84);
INCLUDE_ASM("asm/funcs", func_8003EDC0);

