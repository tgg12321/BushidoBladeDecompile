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
extern s16 D_800A37B0;
extern s32 D_800A3894;
extern s32 D_800A3878;
extern s16 D_800A390C;
extern s32 D_800A385C;
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
extern void func_800167D4(void);
extern void func_80041688(s32, s32);
extern void func_8003F1E4(s32);
extern s16 *func_8004678C(void);
extern void func_8003553C(void);
extern s32 func_80038C70(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);
extern s32 D_800109C8;



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
INCLUDE_ASM("asm/funcs", func_8001B294);
INCLUDE_ASM("asm/funcs", func_8001B3C0);
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
extern s32 func_80021974(s32);
extern void func_80021A98(s32, s32, s32);
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
INCLUDE_ASM("asm/funcs", func_8001D904);
INCLUDE_ASM("asm/funcs", func_8001D998);
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
INCLUDE_ASM("asm/funcs", func_8001DB58);
void func_8001DB9C(void) {
    func_800450BC((&D_8008D9EC)[D_80101ED2] < 1, (s32)0x80190800);
    D_800A38C6 = (u16)0xFFFF;
}
INCLUDE_ASM("asm/funcs", func_8001DBE4);
INCLUDE_ASM("asm/funcs", func_8001DCB0);
INCLUDE_ASM("asm/funcs", func_8001E404);
INCLUDE_ASM("asm/funcs", func_8001E6E4);
INCLUDE_ASM("asm/funcs", func_8001E800);
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
INCLUDE_ASM("asm/funcs", func_8001F1C4);
INCLUDE_ASM("asm/funcs", func_8001F2E4);
void func_8001F860(s16 *arg0, s32 arg1) {
    arg1 = (arg1 - *(s16 *)((u8 *)arg0 + 0x1CA)) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    *(s16 *)((u8 *)arg0 + 0x14C) = arg1;
}
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
extern void func_80020CDC(void);

INCLUDE_ASM("asm/funcs", func_80020D70);
INCLUDE_ASM("asm/funcs", func_80020DDC);
INCLUDE_ASM("asm/funcs", func_80020E74);
extern s32 D_80102770;
extern u16 D_800A38C4;
extern s32 D_801027C0;
extern u16 D_800A38C6;
extern s32 D_801027D4;
extern void func_8001979C(s32, s32);
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
INCLUDE_ASM("asm/funcs", func_800213A0);
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
INCLUDE_ASM("asm/funcs", func_80023D28);
INCLUDE_ASM("asm/funcs", func_80023DB8);
INCLUDE_ASM("asm/funcs", func_80023E40);
INCLUDE_ASM("asm/funcs", func_80023F08);
INCLUDE_ASM("asm/funcs", func_80026DA4);
INCLUDE_ASM("asm/funcs", func_800272FC);
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
INCLUDE_ASM("asm/funcs", func_8002798C);
extern s32 func_8002798C(void);
extern s32 func_80046DEC(s32);
extern void func_80030900(s32 *, s32);
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
void func_80035DC8(void) {
    func_80016868();
    func_80016888();
    func_80020CDC();
    func_800415C4(0);
    func_800415C4(1);
    func_80016C74();
    func_8005B72C();
    func_80077820((s32)0x80118800);
    D_800A3834 = 0x1B;
    func_800168D0();
}
void func_80035E38(void) {
    D_800A36F1 = 1;
    func_8003553C();
    if (func_80038C70() != 0) {
        D_800A3834 = 8;
        D_800A36F1 = 2;
    }
}
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
s32 func_80036E34(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    if (func_80036D98(arg0, arg1) == 0) {
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
INCLUDE_ASM("asm/funcs", func_80036F40);
INCLUDE_ASM("asm/funcs", func_80036FD4);
INCLUDE_ASM("asm/funcs", func_80037110);
extern s16 D_80101E64;
extern s32 func_80037110(void);

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
        func_80036940();
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
INCLUDE_ASM("asm/funcs", func_8003AF40);
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
INCLUDE_ASM("asm/funcs", func_8003B534);
INCLUDE_ASM("asm/funcs", func_8003B56C);
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
INCLUDE_ASM("asm/funcs", func_8003B8E4);
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
INCLUDE_ASM("asm/funcs", func_8003C8B4);
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
