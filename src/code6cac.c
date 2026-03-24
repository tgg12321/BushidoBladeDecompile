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
extern u32 D_800A37B8;
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

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_ASM("asm/funcs", func_80017FA0);
INCLUDE_ASM("asm/funcs", func_80018094);
INCLUDE_ASM("asm/funcs", func_80018300);
INCLUDE_ASM("asm/funcs", func_800187F4);
INCLUDE_ASM("asm/funcs", func_8001924C);
INCLUDE_ASM("asm/funcs", func_80019310);
void func_8001945C(void) {
    D_80106A70 = 0x11;
    D_80106A71 = 0x44;
    D_80106A72 = 0x88;
}
INCLUDE_ASM("asm/funcs", func_80019488);
void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}
INCLUDE_ASM("asm/funcs", func_800194F4);
void func_80019534(void) {
    func_800194F4();
    D_8010278C = 1;
    D_8010278E = 1;
}
INCLUDE_ASM("asm/funcs", func_80019568);
INCLUDE_ASM("asm/funcs", func_8001979C);
INCLUDE_ASM("asm/funcs", func_800198D0);
INCLUDE_ASM("asm/funcs", func_8001A484);
INCLUDE_ASM("asm/funcs", func_8001A4F0);
INCLUDE_ASM("asm/funcs", func_8001A538);
INCLUDE_ASM("asm/funcs", func_8001A62C);
INCLUDE_ASM("asm/funcs", func_8001A67C);
INCLUDE_ASM("asm/funcs", func_8001A820);
INCLUDE_ASM("asm/funcs", func_8001B138);
INCLUDE_ASM("asm/funcs", func_8001B294);
INCLUDE_ASM("asm/funcs", func_8001B3C0);
INCLUDE_ASM("asm/funcs", func_8001B478);
INCLUDE_ASM("asm/funcs", func_8001B690);
INCLUDE_ASM("asm/funcs", func_8001B6F4);
INCLUDE_ASM("asm/funcs", func_8001B748);
INCLUDE_ASM("asm/funcs", func_8001BAE4);
INCLUDE_ASM("asm/funcs", func_8001BBD8);
INCLUDE_ASM("asm/funcs", func_8001BC70);
INCLUDE_ASM("asm/funcs", func_8001BCF0);
void func_8001BE08(s32 *arg0) {
    arg0[2] = 0;
    arg0[3] = 0;
    arg0[4] = 0;
    arg0[5] = -1;
}
INCLUDE_ASM("asm/funcs", func_8001BE20);
INCLUDE_ASM("asm/funcs", func_8001C444);
INCLUDE_ASM("asm/funcs", func_8001C4C0);
INCLUDE_ASM("asm/funcs", func_8001C51C);
INCLUDE_ASM("asm/funcs", func_8001C624);
INCLUDE_ASM("asm/funcs", func_8001C820);
INCLUDE_ASM("asm/funcs", func_8001C8DC);
INCLUDE_ASM("asm/funcs", func_8001CD68);
INCLUDE_ASM("asm/funcs", func_8001CE60);
INCLUDE_ASM("asm/funcs", func_8001D790);
INCLUDE_ASM("asm/funcs", func_8001D904);
INCLUDE_ASM("asm/funcs", func_8001D998);
INCLUDE_ASM("asm/funcs", func_8001DA2C);
INCLUDE_ASM("asm/funcs", func_8001DA8C);
INCLUDE_ASM("asm/funcs", func_8001DB58);
INCLUDE_ASM("asm/funcs", func_8001DB9C);
INCLUDE_ASM("asm/funcs", func_8001DBE4);
INCLUDE_ASM("asm/funcs", func_8001DCB0);
INCLUDE_ASM("asm/funcs", func_8001E404);
INCLUDE_ASM("asm/funcs", func_8001E6E4);
INCLUDE_ASM("asm/funcs", func_8001E800);
INCLUDE_ASM("asm/funcs", func_8001E878);
INCLUDE_ASM("asm/funcs", func_8001EA04);
INCLUDE_ASM("asm/funcs", func_8001EA84);
INCLUDE_ASM("asm/funcs", func_8001EEB4);
INCLUDE_ASM("asm/funcs", func_8001EFA0);
INCLUDE_ASM("asm/funcs", func_8001F1C4);
INCLUDE_ASM("asm/funcs", func_8001F2E4);
INCLUDE_ASM("asm/funcs", func_8001F860);
INCLUDE_ASM("asm/funcs", func_8001F888);
INCLUDE_ASM("asm/funcs", func_8001F938);
INCLUDE_ASM("asm/funcs", func_8001FAE4);
INCLUDE_ASM("asm/funcs", func_8001FB34);
INCLUDE_ASM("asm/funcs", func_8001FBE8);
INCLUDE_ASM("asm/funcs", func_8002006C);
INCLUDE_ASM("asm/funcs", func_800200DC);
INCLUDE_ASM("asm/funcs", func_800203B4);
INCLUDE_ASM("asm/funcs", func_800204C0);
INCLUDE_ASM("asm/funcs", func_800206B0);
INCLUDE_ASM("asm/funcs", func_800207C8);
INCLUDE_ASM("asm/funcs", func_80020CDC);
void func_80020D38(void) {
    if (D_800A38C6 == 0xFFFF) {
        func_80045188();
    }
    D_800A38C6 = 0;
}
INCLUDE_ASM("asm/funcs", func_80020D70);
INCLUDE_ASM("asm/funcs", func_80020DDC);
INCLUDE_ASM("asm/funcs", func_80020E74);
INCLUDE_ASM("asm/funcs", func_80021210);
INCLUDE_ASM("asm/funcs", func_80021280);
INCLUDE_ASM("asm/funcs", func_800213A0);
INCLUDE_ASM("asm/funcs", func_80021424);
INCLUDE_ASM("asm/funcs", func_800218C8);
INCLUDE_ASM("asm/funcs", func_80021904);
INCLUDE_ASM("asm/funcs", func_80021974);
INCLUDE_ASM("asm/funcs", func_800219E4);
INCLUDE_ASM("asm/funcs", func_80021A3C);
INCLUDE_ASM("asm/funcs", func_80021A98);
INCLUDE_ASM("asm/funcs", func_80021D10);
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
INCLUDE_ASM("asm/funcs", func_80023C30);
INCLUDE_ASM("asm/funcs", func_80023CB4);
void func_80023D08(s32 arg0) {
    func_80023CB4(arg0, 0x200);
}
INCLUDE_ASM("asm/funcs", func_80023D28);
INCLUDE_ASM("asm/funcs", func_80023DB8);
INCLUDE_ASM("asm/funcs", func_80023E40);
INCLUDE_ASM("asm/funcs", func_80023F08);
INCLUDE_ASM("asm/funcs", func_80026DA4);
INCLUDE_ASM("asm/funcs", func_800272FC);
INCLUDE_ASM("asm/funcs", func_80027334);
INCLUDE_ASM("asm/funcs", func_8002738C);
INCLUDE_ASM("asm/funcs", func_80027438);
INCLUDE_ASM("asm/funcs", func_800274BC);
INCLUDE_ASM("asm/funcs", func_80027640);
INCLUDE_ASM("asm/funcs", func_800278C0);
INCLUDE_ASM("asm/funcs", func_8002798C);
INCLUDE_ASM("asm/funcs", func_80027A58);
INCLUDE_ASM("asm/funcs", func_80027AD8);
INCLUDE_ASM("asm/funcs", func_800283D0);
INCLUDE_ASM("asm/funcs", func_8002872C);
INCLUDE_ASM("asm/funcs", func_800288C8);
INCLUDE_ASM("asm/funcs", func_8002906C);
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
INCLUDE_ASM("asm/funcs", func_8002EECC);
INCLUDE_ASM("asm/funcs", func_8002F2D0);
INCLUDE_ASM("asm/funcs", func_8002F770);
INCLUDE_ASM("asm/funcs", func_8002FC80);
INCLUDE_ASM("asm/funcs", func_8002FDB0);
INCLUDE_ASM("asm/funcs", func_8002FF20);
INCLUDE_ASM("asm/funcs", func_800300B4);
INCLUDE_ASM("asm/funcs", func_80030208);
INCLUDE_ASM("asm/funcs", func_8003032C);
INCLUDE_ASM("asm/funcs", func_8003043C);
INCLUDE_ASM("asm/funcs", func_8003047C);
INCLUDE_ASM("asm/funcs", func_80030524);
INCLUDE_ASM("asm/funcs", func_80030580);
INCLUDE_ASM("asm/funcs", func_800307D0);
INCLUDE_ASM("asm/funcs", func_80030900);
INCLUDE_ASM("asm/funcs", func_80030A2C);
INCLUDE_ASM("asm/funcs", func_80030B10);
INCLUDE_ASM("asm/funcs", func_80030BA8);
INCLUDE_ASM("asm/funcs", func_80030D04);
void func_80030D48(void) {
}
INCLUDE_ASM("asm/funcs", func_80030D50);
INCLUDE_ASM("asm/funcs", func_80030D7C);
INCLUDE_ASM("asm/funcs", func_80031890);
INCLUDE_ASM("asm/funcs", func_80031B24);
INCLUDE_ASM("asm/funcs", func_80032040);
INCLUDE_ASM("asm/funcs", func_80032064);
INCLUDE_ASM("asm/funcs", func_800321E8);
INCLUDE_ASM("asm/funcs", func_80032314);
INCLUDE_ASM("asm/funcs", func_800324D0);
INCLUDE_ASM("asm/funcs", func_800325E0);
INCLUDE_ASM("asm/funcs", func_80032854);
INCLUDE_ASM("asm/funcs", func_80032C50);
INCLUDE_ASM("asm/funcs", func_8003339C);
INCLUDE_ASM("asm/funcs", func_80033498);
INCLUDE_ASM("asm/funcs", func_80033510);
INCLUDE_ASM("asm/funcs", func_80033550);
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
INCLUDE_ASM("asm/funcs", func_80033FE4);
INCLUDE_ASM("asm/funcs", func_800340A0);
INCLUDE_ASM("asm/funcs", func_80034200);
INCLUDE_ASM("asm/funcs", func_800342A0);
INCLUDE_ASM("asm/funcs", func_800343F0);
INCLUDE_ASM("asm/funcs", func_800344B4);
INCLUDE_ASM("asm/funcs", func_80034708);
INCLUDE_ASM("asm/funcs", func_80034F88);
INCLUDE_ASM("asm/funcs", func_8003504C);
INCLUDE_ASM("asm/funcs", func_80035280);
void func_80035430(void) {
}
INCLUDE_ASM("asm/funcs", func_80035438);
INCLUDE_ASM("asm/funcs", func_80035480);
INCLUDE_ASM("asm/funcs", func_8003553C);
void func_800355E8(void) {
    func_80035FA8();
    func_80037110(1);
    func_800371E8(1);
}
INCLUDE_ASM("asm/funcs", func_80035618);
INCLUDE_ASM("asm/funcs", func_80035828);
INCLUDE_ASM("asm/funcs", func_80035DC8);
INCLUDE_ASM("asm/funcs", func_80035E38);
INCLUDE_ASM("asm/funcs", func_80035E88);
INCLUDE_ASM("asm/funcs", func_80035EDC);
INCLUDE_ASM("asm/funcs", func_80035F30);
INCLUDE_ASM("asm/funcs", func_80035F78);
void func_80035FA8(void) {
    func_80083B50(0, 0, 1);
    func_80085448(0, 0x7F, 0x7F);
}
INCLUDE_ASM("asm/funcs", func_80035FE0);
void func_80036034(void) {
    func_80080148();
    func_8007FF7C();
    func_800828CC(4);
}
INCLUDE_ASM("asm/funcs", func_80036064);
INCLUDE_ASM("asm/funcs", func_80036140);
INCLUDE_ASM("asm/funcs", func_80036940);
s32 func_80036D88(void) {
    return D_80101E62 == 0;
}
INCLUDE_ASM("asm/funcs", func_80036D98);
INCLUDE_ASM("asm/funcs", func_80036E34);
s32 func_80036EA8(s32 arg0, s32 arg1) {
    return (&D_8008F12C)[arg0] + arg1;
}
INCLUDE_ASM("asm/funcs", func_80036EC0);
u32 func_80036F28(s32 arg0) {
    return (&D_8008EC38)[arg0 * 2];
}
INCLUDE_ASM("asm/funcs", func_80036F40);
INCLUDE_ASM("asm/funcs", func_80036FD4);
INCLUDE_ASM("asm/funcs", func_80037110);
INCLUDE_ASM("asm/funcs", func_800371AC);
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
INCLUDE_ASM("asm/funcs", func_80037260);
void func_800372C0(void) {
    if (D_80101E62 != 0) {
        func_80036EC0();
    }
    func_80036F40();
}
INCLUDE_ASM("asm/funcs", func_800372F4);
INCLUDE_ASM("asm/funcs", func_80037348);
INCLUDE_ASM("asm/funcs", func_80037468);
INCLUDE_ASM("asm/funcs", func_80037540);
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
INCLUDE_ASM("asm/funcs", func_80037F08);
INCLUDE_ASM("asm/funcs", func_80037F40);
INCLUDE_ASM("asm/funcs", func_8003800C);
INCLUDE_ASM("asm/funcs", func_80038148);
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
INCLUDE_ASM("asm/funcs", func_8003A308);
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
INCLUDE_ASM("asm/funcs", func_8003A6FC);
INCLUDE_ASM("asm/funcs", func_8003A728);
INCLUDE_ASM("asm/funcs", func_8003AA48);
INCLUDE_ASM("asm/funcs", func_8003AA78);
INCLUDE_ASM("asm/funcs", func_8003AAB0);
INCLUDE_ASM("asm/funcs", func_8003AB44);
INCLUDE_ASM("asm/funcs", func_8003ACB8);
INCLUDE_ASM("asm/funcs", func_8003AE5C);
INCLUDE_ASM("asm/funcs", func_8003AF40);
INCLUDE_ASM("asm/funcs", func_8003AFFC);
INCLUDE_ASM("asm/funcs", func_8003B10C);
INCLUDE_ASM("asm/funcs", func_8003B20C);
INCLUDE_ASM("asm/funcs", func_8003B2C8);
INCLUDE_ASM("asm/funcs", func_8003B328);
INCLUDE_ASM("asm/funcs", func_8003B3A4);
INCLUDE_ASM("asm/funcs", func_8003B484);
INCLUDE_ASM("asm/funcs", func_8003B4DC);
INCLUDE_ASM("asm/funcs", func_8003B534);
INCLUDE_ASM("asm/funcs", func_8003B56C);
INCLUDE_ASM("asm/funcs", func_8003B5A4);
INCLUDE_ASM("asm/funcs", func_8003B870);
INCLUDE_ASM("asm/funcs", func_8003B8E4);
INCLUDE_ASM("asm/funcs", func_8003B9D0);
INCLUDE_ASM("asm/funcs", func_8003BCB4);
INCLUDE_ASM("asm/funcs", func_8003BE10);
INCLUDE_ASM("asm/funcs", func_8003BEA8);
INCLUDE_ASM("asm/funcs", func_8003BFC4);
INCLUDE_ASM("asm/funcs", func_8003C040);
INCLUDE_ASM("asm/funcs", func_8003C2C0);
INCLUDE_ASM("asm/funcs", func_8003C42C);
INCLUDE_ASM("asm/funcs", func_8003C560);
INCLUDE_ASM("asm/funcs", func_8003C714);
INCLUDE_ASM("asm/funcs", func_8003C8B4);
INCLUDE_ASM("asm/funcs", func_8003C958);
INCLUDE_ASM("asm/funcs", func_8003C9A4);
INCLUDE_ASM("asm/funcs", func_8003CCCC);
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
INCLUDE_ASM("asm/funcs", func_8003D774);
INCLUDE_ASM("asm/funcs", func_8003D7B4);
INCLUDE_ASM("asm/funcs", func_8003D888);
INCLUDE_ASM("asm/funcs", func_8003D91C);
INCLUDE_ASM("asm/funcs", func_8003D9A0);
INCLUDE_ASM("asm/funcs", func_8003DA8C);
INCLUDE_ASM("asm/funcs", func_8003DBE4);
void func_8003DDF8(u32 arg0) {
    u32 *ptr = (u32 *)D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}
INCLUDE_ASM("asm/funcs", func_8003DE14);
INCLUDE_ASM("asm/funcs", func_8003E0E0);
INCLUDE_ASM("asm/funcs", func_8003E120);
INCLUDE_ASM("asm/funcs", func_8003E164);
INCLUDE_ASM("asm/funcs", func_8003E22C);
INCLUDE_ASM("asm/funcs", func_8003E2A0);
INCLUDE_ASM("asm/funcs", func_8003E2AC);
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
