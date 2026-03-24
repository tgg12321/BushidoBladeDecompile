#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

INCLUDE_ASM("asm/funcs", func_80047ED0);
INCLUDE_ASM("asm/funcs", func_80047EE8);
INCLUDE_ASM("asm/funcs", func_80047FBC);
INCLUDE_ASM("asm/funcs", func_800480C0);
INCLUDE_ASM("asm/funcs", func_800481E8);
INCLUDE_ASM("asm/funcs", func_800482C8);
INCLUDE_ASM("asm/funcs", func_800483DC);
INCLUDE_ASM("asm/funcs", func_800484A0);
INCLUDE_ASM("asm/funcs", func_80048530);
INCLUDE_ASM("asm/funcs", func_800485EC);
extern s16 D_800F6652;
s32 func_800167AC(void);
s16 func_800486FC(void) {
    if (func_800167AC()) {
        D_800F6652 = 1;
    } else {
        D_800F6652 = 0;
    }
    return D_800F6652;
}
extern s16 D_800F6652;
void func_80048744(s32 a0) {
    if (a0) {
        D_800F6652 = 1;
    } else {
        D_800F6652 = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_8004876C);
INCLUDE_ASM("asm/funcs", func_8004881C);
INCLUDE_ASM("asm/funcs", func_80048864);
INCLUDE_ASM("asm/funcs", func_80048A7C);
INCLUDE_ASM("asm/funcs", func_80048AD0);
INCLUDE_ASM("asm/funcs", func_80048B8C);
INCLUDE_ASM("asm/funcs", func_80048BA4);
extern u8 D_800EF848[];
extern u16 D_80099C34[];
extern void func_80052C10(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 s1 = a0;
    s32 s0 = a1;
    s32 i;
    u16 *src;
    u16 *dst;
    u8 *base;
    if (s0 > 0) {
        func_80052C10();
    }
    base = D_800EF848 + s0 * 308;
    *(u32 *)base = 0;
    src = (u16 *)(D_80099C34 + s1 * 7);
    dst = (u16 *)(base + 0x124);
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}
INCLUDE_ASM("asm/funcs", func_80048FFC);
INCLUDE_ASM("asm/funcs", func_8004939C);
INCLUDE_ASM("asm/funcs", func_800493E4);
INCLUDE_ASM("asm/funcs", func_800494D4);
INCLUDE_ASM("asm/funcs", func_8004954C);
INCLUDE_ASM("asm/funcs", func_80049584);
void func_80049710(void) {
}
INCLUDE_ASM("asm/funcs", func_80049718);
INCLUDE_ASM("asm/funcs", func_80049A2C);
INCLUDE_ASM("asm/funcs", func_80049C24);
INCLUDE_ASM("asm/funcs", func_80049E1C);
INCLUDE_ASM("asm/funcs", func_80049E4C);
INCLUDE_ASM("asm/funcs", func_80049F4C);
INCLUDE_ASM("asm/funcs", func_8004A09C);
INCLUDE_ASM("asm/funcs", func_8004A1FC);
INCLUDE_ASM("asm/funcs", func_8004A348);
INCLUDE_ASM("asm/funcs", func_8004A4E0);
INCLUDE_ASM("asm/funcs", func_8004A76C);
INCLUDE_ASM("asm/funcs", func_8004A808);
void func_8004A938(void) {
}
INCLUDE_ASM("asm/funcs", func_8004A940);
INCLUDE_ASM("asm/funcs", func_8004BB68);
INCLUDE_ASM("asm/funcs", func_8004BCC0);
INCLUDE_ASM("asm/funcs", func_8004C1F4);
INCLUDE_ASM("asm/funcs", func_8004C388);
PAD_NOPS_1; /* padding after func_8004C388 */
INCLUDE_ASM("asm/funcs", func_8004C404);
INCLUDE_ASM("asm/funcs", func_8004C994);
INCLUDE_ASM("asm/funcs", func_8004CB8C);
INCLUDE_ASM("asm/funcs", func_8004CDB0);
INCLUDE_ASM("asm/funcs", func_8004CFE0);
INCLUDE_ASM("asm/funcs", func_8004D244);
INCLUDE_ASM("asm/funcs", func_8004D424);
INCLUDE_ASM("asm/funcs", func_8004D634);
INCLUDE_ASM("asm/funcs", func_8004D838);
INCLUDE_ASM("asm/funcs", func_8004DA74);
INCLUDE_ASM("asm/funcs", func_8004DDB4);
PAD_NOPS_1; /* padding after func_8004DDB4 */
void func_8004E564(void) {
}
void func_8004E56C(void) {
}
INCLUDE_ASM("asm/funcs", func_8004E574);
INCLUDE_ASM("asm/funcs", func_8004E7E4);
INCLUDE_ASM("asm/funcs", func_8004EAC8);
INCLUDE_ASM("asm/funcs", func_8004ECC8);
INCLUDE_ASM("asm/funcs", func_8004EF10);
INCLUDE_ASM("asm/funcs", func_8004F0FC);
INCLUDE_ASM("asm/funcs", func_8004F314);
INCLUDE_ASM("asm/funcs", func_8004F53C);
INCLUDE_ASM("asm/funcs", func_8004F798);
INCLUDE_ASM("asm/funcs", func_8004F970);
INCLUDE_ASM("asm/funcs", func_8004FB74);
INCLUDE_ASM("asm/funcs", func_8004FD40);
INCLUDE_ASM("asm/funcs", func_8004FF40);
INCLUDE_ASM("asm/funcs", func_80050120);
INCLUDE_ASM("asm/funcs", func_80050334);
INCLUDE_ASM("asm/funcs", func_80050538);
INCLUDE_ASM("asm/funcs", func_80050774);
INCLUDE_ASM("asm/funcs", func_80050908);
INCLUDE_ASM("asm/funcs", func_80050AB8);
INCLUDE_ASM("asm/funcs", func_80050C68);
INCLUDE_ASM("asm/funcs", func_80050E60);
INCLUDE_ASM("asm/funcs", func_80051010);
INCLUDE_ASM("asm/funcs", func_80051208);
INCLUDE_ASM("asm/funcs", func_800513B0);
INCLUDE_ASM("asm/funcs", func_800515AC);
INCLUDE_ASM("asm/funcs", func_80051754);
INCLUDE_ASM("asm/funcs", func_80051944);
INCLUDE_ASM("asm/funcs", func_80051B04);
INCLUDE_ASM("asm/funcs", func_80051D08);
INCLUDE_ASM("asm/funcs", func_80051ED4);
INCLUDE_ASM("asm/funcs", func_800520B8);
INCLUDE_ASM("asm/funcs", func_800523E0);
INCLUDE_ASM("asm/funcs", func_800525D8);
INCLUDE_ASM("asm/funcs", func_800526A0);
PAD_NOPS_2; /* padding after func_800526A0 */
INCLUDE_ASM("asm/funcs", func_80052720);
INCLUDE_ASM("asm/funcs", func_80052754);
INCLUDE_ASM("asm/funcs", func_80052788);
INCLUDE_ASM("asm/funcs", func_800527FC);
INCLUDE_ASM("asm/funcs", func_80052930);
INCLUDE_ASM("asm/funcs", func_80052A20);
INCLUDE_ASM("asm/funcs", func_80052A88);
INCLUDE_ASM("asm/funcs", func_80052B00);
INCLUDE_ASM("asm/funcs", func_80052B44);
INCLUDE_ASM("asm/funcs", func_80052B7C);
INCLUDE_ASM("asm/funcs", func_80052BE4);
INCLUDE_ASM("asm/funcs", func_80052C10);
PAD_NOPS_1; /* padding after func_80052C10 */
INCLUDE_ASM("asm/funcs", func_80052C28);
INCLUDE_ASM("asm/funcs", func_80052C4C);
INCLUDE_ASM("asm/funcs", func_80052CD4);
PAD_NOPS_3; /* padding after func_80052CD4 */
INCLUDE_ASM("asm/funcs", func_80052D00);
INCLUDE_ASM("asm/funcs", func_80053304);
INCLUDE_ASM("asm/funcs", func_8005344C);
INCLUDE_ASM("asm/funcs", func_80053584);
INCLUDE_ASM("asm/funcs", func_80053614);
INCLUDE_ASM("asm/funcs", func_80053694);
INCLUDE_ASM("asm/funcs", func_80053754);
INCLUDE_ASM("asm/funcs", func_80053E9C);
INCLUDE_ASM("asm/funcs", func_80054410);
INCLUDE_ASM("asm/funcs", func_8005441C);
INCLUDE_ASM("asm/funcs", func_80054434);
INCLUDE_ASM("asm/funcs", func_80054440);
INCLUDE_ASM("asm/funcs", func_800545F4);
INCLUDE_ASM("asm/funcs", func_80054604);
extern s16 D_80099D54[];
extern void func_80054604(s32, s32, s32, s32, s32, s32, s32);
void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    func_80054604(D_80099D54[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}
void func_8007B33C(s32);
void func_8004659C(s32);
void func_80046A60(void);
void func_800548DC(void) {
    func_8007B33C(0);
    func_8004659C(-1);
    func_80046A60();
}
INCLUDE_ASM("asm/funcs", func_8005490C);
extern u32 D_80102C00;
extern u32 *D_800A3820;
extern u16 D_800A38D6;
extern s32 D_800A374C;
extern s32 D_800A3808;
extern s32 D_800A378C;
extern s32 func_8005490C(void);
extern void func_800444E0(void);
s32 func_80054F68(void) {
    s32 v3;
    s32 s0;
    D_800A3820 = &D_80102C00;
    v3 = D_800A374C;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = v3;
    D_800A378C = v3 + 0x10;
    s0 = func_8005490C();
    func_800444E0();
    return s0;
}
extern s32 D_800EFB14;
extern s32 D_800EFB18;
extern s32 D_800EFB1C;
extern s32 D_800EFB20;
extern s32 D_800EFB24;
extern s32 D_800EFB28;
void func_80054FDC(s32 a0) {
    s32 *p = &D_800EFB14;
    *p = a0 + *p;
    D_800EFB18 = a0 + D_800EFB18;
    if (D_800EFB1C) {
        D_800EFB1C = a0 + D_800EFB1C;
    }
    if (D_800EFB20) {
        D_800EFB20 = a0 + D_800EFB20;
    }
    if (D_800EFB24) {
        D_800EFB24 = a0 + D_800EFB24;
    }
    if (D_800EFB28) {
        D_800EFB28 = a0 + D_800EFB28;
    }
}
extern s32 D_800EFB0C;
s32* func_8005507C(void) {
    return &D_800EFB0C;
}
extern s32 D_80101E1C;
s32* func_8005508C(void) {
    return &D_80101E1C;
}
INCLUDE_ASM("asm/funcs", func_8005509C);
INCLUDE_ASM("asm/funcs", func_800550E8);
INCLUDE_ASM("asm/funcs", func_80055138);
INCLUDE_ASM("asm/funcs", func_80055948);
void func_80055B44(u8 *a0, s32 a1, s32 a2, s32 a3) {
    *(s32 *)(a0 + 0x3B4) = a1;
    a0[0x3BC] = (u8)a2;
    a0[0x3B8] = (u8)a3;
    *(s32 *)(a0 + 0x3C8) = 0;
    *(s32 *)(a0 + 0x3CC) = -1;
}
INCLUDE_ASM("asm/funcs", func_80055B60);
INCLUDE_ASM("asm/funcs", func_80056CB8);
INCLUDE_ASM("asm/funcs", func_80056FE8);
INCLUDE_ASM("asm/funcs", func_80057094);
INCLUDE_ASM("asm/funcs", func_800571C0);
INCLUDE_ASM("asm/funcs", func_8005763C);
INCLUDE_ASM("asm/funcs", func_80057ACC);
INCLUDE_ASM("asm/funcs", func_80057CC8);
INCLUDE_ASM("asm/funcs", func_80057E84);
INCLUDE_ASM("asm/funcs", func_80058580);
INCLUDE_ASM("asm/funcs", func_8005B43C);
INCLUDE_ASM("asm/funcs", func_8005B50C);
void func_800858D0(s32);
void func_8005B58C(void) {
    func_800858D0(0);
}
extern void func_800858D0(s32);
extern void func_80086130(s32, s32, s32);
extern u32 D_800EFB78[];
extern u8 D_800EFB7C[];
void func_8005B5AC(void) {
    s32 s1;
    s32 s3;
    u8 *s2;
    s32 s0;
    func_800858D0(0);
    s1 = 0;
    s3 = 0x7F;
    s2 = (u8 *)D_800EFB78;
    s0 = 0;
    do {
        *(u32 *)((u8 *)D_800EFB78 + s0) = 0;
        s2[5] = s3;
        *((u8 *)D_800EFB7C + s0) = s3;
        func_80086130((s16)s1, 0, 0);
        s2 += 8;
        s1++;
        s0 += 8;
    } while (s1 < 24);
}
void func_800858D0(s32);
void func_80087F64(s32);
extern s32 D_800EFC38;
extern s32 D_800EFB38;
void func_8005B644(s32 a0) {
    s32 v;
    func_800858D0(0);
    v = a0 * 2 + a0 + 1;
    func_80087F64((s16)v);
    *(s32*)((u8*)&D_800EFC38 + (v * 4)) = 0;
    *(s32*)((u8*)&D_800EFB38 + (v * 4)) = 0;
}
extern s32 D_800EFC40;
extern s32 D_800EFB40;
extern s32 D_800EFC4C;
extern s32 D_800EFB4C;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B6AC(void) {
    func_800858D0(0);
    func_80087F64(2);
    D_800EFC40 = 0;
    D_800EFB40 = 0;
    func_80087F64(5);
    D_800EFC4C = 0;
    D_800EFB4C = 0;
}
extern s32 D_800EFC3C;
extern s32 D_800EFB3C;
void func_80087F64(s32);
void func_8005B6FC(void) {
    func_80087F64(1);
    D_800EFC3C = 0;
    D_800EFB3C = 0;
}
INCLUDE_ASM("asm/funcs", func_8005B72C);
INCLUDE_ASM("asm/funcs", func_8005B7C4);
extern s32 D_800EFC58;
extern s32 D_800EFB58;
extern s32 D_800EFC48;
extern s32 D_800EFB48;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B868(void) {
    func_800858D0(0);
    func_80087F64(8);
    D_800EFC58 = 0;
    D_800EFB58 = 0;
    func_80087F64(4);
    D_800EFC48 = 0;
    D_800EFB48 = 0;
}
INCLUDE_ASM("asm/funcs", func_8005B8B8);
void func_8005C4C0(s32, s32);
void func_8005B98C(s32 a0) {
    func_8005C4C0(a0, 8);
    func_8005C4C0(a0, 4);
}
extern s32 D_800EFC5C;
extern s32 D_800EFB5C;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B9C4(void) {
    func_800858D0(0);
    func_80087F64(9);
    D_800EFC5C = 0;
    D_800EFB5C = 0;
}
void func_8005B9C4(void);
s32 func_80036EA8(s32, s32);
s32 func_80036F40(void);
void func_80036D98(s32, s32);
s32 func_80036F28(s32);
void func_8005C2A8(s32, s32, s32);
void func_8005B9FC(s32 a0) {
    s32 s1;
    func_8005B9C4();
    s1 = func_80036EA8(2, 8);
    func_80036F40();
    func_80036D98(s1, a0);
    s1 = func_80036F28(s1);
    func_80036F40();
    func_8005C2A8(a0, 9, a0 + s1);
}
void func_8005C4C0(s32, s32);
void func_8005BA6C(s32 a0) {
    func_8005C4C0(a0, 9);
}
INCLUDE_ASM("asm/funcs", func_8005BA8C);
INCLUDE_ASM("asm/funcs", func_8005BD30);
}
extern u32 D_800EFC38[];
extern u32 D_800EFB38[];
extern u8 D_8009AD18[];
extern void func_80087F64(s32);
void func_8005BDF0(void) {
    u32 *s3 = D_800EFC38;
    u32 *s2 = D_800EFB38;
    u8 *s0 = D_8009AD18;
    u8 *s1 = (u8 *)((s32)s0 + 3);
    do {
        func_80087F64(*s0);
        s3[*s0] = 0;
        s2[*s0] = 0;
        s0++;
    } while ((s32)s0 < (s32)s1);
}
INCLUDE_ASM("asm/funcs", func_8005BE84);
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void func_8005BF3C(void) {
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
}
INCLUDE_ASM("asm/funcs", func_8005BF78);
INCLUDE_ASM("asm/funcs", func_8005C074);
INCLUDE_ASM("asm/funcs", func_8005C2A8);
INCLUDE_ASM("asm/funcs", func_8005C4C0);
INCLUDE_ASM("asm/funcs", func_8005C5A8);
void func_80083BE4(s32, s32);
void func_800858D0(s32);
void func_80087F24(void);
void func_80087F00(s32);
void func_8005C614(void) {
    func_80083BE4(0x7F, 0x7F);
    func_800858D0(0);
    func_80087F24();
    func_80087F00(0);
}
extern s32 D_8009AA70;
extern s32 D_800EFB78;
extern u8 D_800EFB7C;
extern u8 D_800EFB7D;
void func_8005C650(s32 a0, s32 a1, s32 a2) {
    s16 a3 = 0;
    s32 *base = (s32 *)((u8 *)&D_8009AA70 + a0 * 4);
    do {
        s32 off = a3 * 8;
        if (!*(s32 *)((u8 *)&D_800EFB78 + off)) {
            *(s32 *)((u8 *)&D_800EFB78 + off) = (s32)base;
            *((u8 *)&D_800EFB7C + off) = (u8)a1;
            *((u8 *)&D_800EFB7D + off) = (u8)a2;
            return;
        }
        a3 = (s16)(a3 + 1);
    } while ((s16)a3 < 0x18);
}
INCLUDE_ASM("asm/funcs", func_8005C6D0);
INCLUDE_ASM("asm/funcs", func_8005C8A8);
INCLUDE_ASM("asm/funcs", func_8005D46C);
INCLUDE_ASM("asm/funcs", func_8005D554);
INCLUDE_ASM("asm/funcs", func_8005D814);
INCLUDE_ASM("asm/funcs", func_8005E098);
s32 func_8005E098(s32, s32, s32, s32);
s32 func_8005E51C(s32 a0, s32 a1, s32 a2) {
    return func_8005E098(-1, a0 - 1, a1, a2);
}
INCLUDE_ASM("asm/funcs", func_8005E54C);
INCLUDE_ASM("asm/funcs", func_8005F1C8);
INCLUDE_ASM("asm/funcs", func_8005FA98);
INCLUDE_ASM("asm/funcs", func_8005FBC8);
INCLUDE_ASM("asm/funcs", func_8005FC9C);
INCLUDE_ASM("asm/funcs", func_800600C8);
INCLUDE_ASM("asm/funcs", func_800602AC);
INCLUDE_ASM("asm/funcs", func_80060414);
INCLUDE_ASM("asm/funcs", func_80060544);
INCLUDE_ASM("asm/funcs", func_80060758);
INCLUDE_ASM("asm/funcs", func_80060768);
INCLUDE_ASM("asm/funcs", func_80060A68);
INCLUDE_ASM("asm/funcs", func_80060B70);
INCLUDE_ASM("asm/funcs", func_80060C60);
INCLUDE_ASM("asm/funcs", func_80060CB8);
INCLUDE_ASM("asm/funcs", func_80060E04);
INCLUDE_ASM("asm/funcs", func_80060E38);
INCLUDE_ASM("asm/funcs", func_80061064);
INCLUDE_ASM("asm/funcs", func_80061178);
INCLUDE_ASM("asm/funcs", func_800611A4);
INCLUDE_ASM("asm/funcs", func_80061250);
INCLUDE_ASM("asm/funcs", func_8006133C);
INCLUDE_ASM("asm/funcs", func_800613C8);
INCLUDE_ASM("asm/funcs", func_80061454);
INCLUDE_ASM("asm/funcs", func_800614E0);
INCLUDE_ASM("asm/funcs", func_8006156C);
INCLUDE_ASM("asm/funcs", func_80061658);
INCLUDE_ASM("asm/funcs", func_80061710);
INCLUDE_ASM("asm/funcs", func_800617C8);
INCLUDE_ASM("asm/funcs", func_800618B4);
INCLUDE_ASM("asm/funcs", func_800619A4);
INCLUDE_ASM("asm/funcs", func_800619F0);
INCLUDE_ASM("asm/funcs", func_80061A3C);
INCLUDE_ASM("asm/funcs", func_80061ACC);
INCLUDE_ASM("asm/funcs", func_80061C00);
INCLUDE_ASM("asm/funcs", func_80061D74);
INCLUDE_ASM("asm/funcs", func_80061EC0);
INCLUDE_ASM("asm/funcs", func_80061FAC);
INCLUDE_ASM("asm/funcs", func_80062020);
INCLUDE_ASM("asm/funcs", func_800620B8);
INCLUDE_ASM("asm/funcs", func_8006288C);
INCLUDE_ASM("asm/funcs", func_8006295C);
INCLUDE_ASM("asm/funcs", func_80062FEC);
INCLUDE_ASM("asm/funcs", func_80063084);
INCLUDE_ASM("asm/funcs", func_80063AF0);
INCLUDE_ASM("asm/funcs", func_80063B34);
INCLUDE_ASM("asm/funcs", func_80063B78);
INCLUDE_ASM("asm/funcs", func_80063BA4);
INCLUDE_ASM("asm/funcs", func_80063BD0);
INCLUDE_ASM("asm/funcs", func_80063E10);
INCLUDE_ASM("asm/funcs", func_800644FC);
INCLUDE_ASM("asm/funcs", func_800645B0);
INCLUDE_ASM("asm/funcs", func_800646E8);
INCLUDE_ASM("asm/funcs", func_80064E90);
INCLUDE_ASM("asm/funcs", func_80064ED8);
INCLUDE_ASM("asm/funcs", func_80064F20);
INCLUDE_ASM("asm/funcs", func_80064F68);
INCLUDE_ASM("asm/funcs", func_80064FB4);
INCLUDE_ASM("asm/funcs", func_80065000);
INCLUDE_ASM("asm/funcs", func_8006505C);
INCLUDE_ASM("asm/funcs", func_800650A4);
INCLUDE_ASM("asm/funcs", func_800650EC);
INCLUDE_ASM("asm/funcs", func_80065134);
INCLUDE_ASM("asm/funcs", func_8006517C);
INCLUDE_ASM("asm/funcs", func_800651F0);
INCLUDE_ASM("asm/funcs", func_80065264);
INCLUDE_ASM("asm/funcs", func_800652AC);
extern s16 D_800F0BA8;
u8 func_80065800(s32);
u8 func_800652F4(void) {
    u8 v0 = func_80065800(0);
    s16 *p = &D_800F0BA8;
    s16 v1 = *p;
    v1 += 0x1FF;
    *p = v1;
    if ((s16)v1 < 0x1001) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAA;
u8 func_80065344(void) {
    u8 v0 = func_80065800(1);
    s16 *p = &D_800F0BAA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAC;
u8 func_80065394(void) {
    u8 v0 = func_80065800(2);
    s16 *p = &D_800F0BAC;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAE;
u8 func_800653E4(void) {
    u8 v0 = func_80065800(3);
    s16 *p = &D_800F0BAE;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB0;
u8 func_80065434(void) {
    u8 v0 = func_80065800(4);
    s16 *p = &D_800F0BB0;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80065484);
extern s16 D_800F0BB4;
u8 func_80065540(void) {
    u8 v0 = func_80065800(6);
    s16 *p = &D_800F0BB4;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB6;
u8 func_80065590(void) {
    u8 v0 = func_80065800(7);
    s16 *p = &D_800F0BB6;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBC;
u8 func_800655E0(void) {
    u8 v0 = func_80065800(0xA);
    s16 *p = &D_800F0BBC;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBE;
u8 func_80065630(void) {
    u8 v0 = func_80065800(0xB);
    s16 *p = &D_800F0BBE;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s32 func_80065800(s32);
extern u16 D_800F0BC0;
extern s16 D_800F0BC4;
s32 func_80065680(void) {
    u16 *v1;
    s32 v0;
    func_80065800(0xC);
    v1 = &D_800F0BC0;
    v0 = *v1 + 1;
    *v1 = v0;
    v0 = func_80065800(0xE);
    D_800F0BC4 = D_800F0BC4 + 1;
    if ((s16)D_800F0BC4 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern u16 D_800F0BC2;
extern u16 D_800F0BC6;
s32 func_800656EC(void) {
    u16 *s0 = &D_800F0BC2;
    s32 v0;
    func_80065800(0xD);
    *s0 = *s0 + 1;
    v0 = func_80065800(0xF);
    D_800F0BC6 = D_800F0BC6 + 1;
    if ((s16)*s0 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern s16 D_800F0BC8;
u8 func_80065760(void) {
    u8 v0 = func_80065800(0x10);
    s16 *p = &D_800F0BC8;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BCA;
u8 func_800657B0(void) {
    u8 v0 = func_80065800(0x11);
    s16 *p = &D_800F0BCA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80065800);
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EC0(void) {
    u8 ret = func_80067200(0, 0, 0);
    D_800F10D8 = 1;
    return ret;
}
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EF4(void) {
    u8 ret = func_80067200(0, 0, 1);
    D_800F10D8 = 2;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F28(void) {
    u8 ret = func_80067200(1, 1, 0);
    D_800F10DC = 1;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F5C(void) {
    u8 ret = func_80067200(1, 1, 1);
    D_800F10DC = 2;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066F90(void) {
    u8 ret = func_80067200(2, 1, 0);
    D_800F1120 = 1;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066FC4(void) {
    u8 ret = func_80067200(2, 1, 1);
    D_800F1120 = 2;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_80066FF8(void) {
    u8 ret = func_80067200(3, 0, 0);
    D_800F1124 = 1;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_8006702C(void) {
    u8 ret = func_80067200(3, 0, 1);
    D_800F1124 = 2;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067060(void) {
    u8 ret = func_80067200(4, 2, 0);
    D_800F1128 = 1;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067094(void) {
    u8 ret = func_80067200(4, 2, 1);
    D_800F1128 = 2;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670C8(void) {
    u8 ret = func_80067200(5, 3, 0);
    D_800F112C = 1;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670FC(void) {
    u8 ret = func_80067200(5, 3, 1);
    D_800F112C = 2;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067130(void) {
    u8 ret = func_80067200(6, 3, 0);
    D_800F1130 = 1;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067164(void) {
    u8 ret = func_80067200(6, 3, 1);
    D_800F1130 = 2;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_80067198(void) {
    u8 ret = func_80067200(7, 2, 0);
    D_800F1134 = 1;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_800671CC(void) {
    u8 ret = func_80067200(7, 2, 1);
    D_800F1134 = 2;
    return ret;
}
INCLUDE_ASM("asm/funcs", func_80067200);
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800676C8(void) {
    func_800678A8(0, 0);
    func_80067D14(0, 0);
    return func_80068D88(0, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067704(void) {
    func_800678A8(1, 1);
    func_80067D14(1, 1);
    return func_80068D88(1, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067740(void) {
    func_800678A8(2, 1);
    func_80067D14(2, 1);
    return func_80068D88(2, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006777C(void) {
    func_800678A8(3, 0);
    func_80067D14(3, 0);
    return func_80068D88(3, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677B8(void) {
    func_800678A8(4, 2);
    func_80067D14(4, 2);
    return func_80068D88(4, 2);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677F4(void) {
    func_800678A8(5, 3);
    func_80067D14(5, 3);
    return func_80068D88(5, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067830(void) {
    func_800678A8(6, 3);
    func_80067D14(6, 3);
    return func_80068D88(6, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006786C(void) {
    func_800678A8(7, 2);
    func_80067D14(7, 2);
    return func_80068D88(7, 2);
}
INCLUDE_ASM("asm/funcs", func_800678A8);
INCLUDE_ASM("asm/funcs", func_80067D14);
INCLUDE_ASM("asm/funcs", func_80068D88);
INCLUDE_ASM("asm/funcs", func_80068ECC);
INCLUDE_ASM("asm/funcs", func_80068F70);
INCLUDE_ASM("asm/funcs", func_80069120);
void func_8006920C(s32 *, s32);
void func_8005C2A8(s32, s32, s32);
s32 func_8006919C(s32 *a0) {
    s32 i = 0;
    s32 *p = &a0[5];
    do {
        func_8006920C(a0, *p);
        p++;
        i++;
    } while (i < 12);
    func_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
void func_8006920C(s32 *a0, s32 a1) {
    s32 *p = (s32 *)a1;
    if (!*p) return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)a0;
        }
        p++;
    }
}
INCLUDE_ASM("asm/funcs", func_80069250);
INCLUDE_ASM("asm/funcs", func_800692C0);
INCLUDE_ASM("asm/funcs", func_800693CC);
INCLUDE_ASM("asm/funcs", func_80069898);
s32 *func_80077D00(void);
void func_80069A30(u8 *a0) {
    s32 *p = func_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 0x22;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x4C;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
        v0 = 0x6C;
    }
    a0[6] = (u8)v0;
}
s32 *func_80077D00(void);
void func_80069A8C(u8 *a0) {
    s32 *p = func_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 8;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x31;
        a0[4] = 0;
        a0[5] = 0;
    }
    a0[6] = (u8)v0;
}
INCLUDE_ASM("asm/funcs", func_80069AE4);
INCLUDE_ASM("asm/funcs", func_80069E18);
INCLUDE_ASM("asm/funcs", func_80069F80);
INCLUDE_ASM("asm/funcs", func_8006A1A0);
INCLUDE_ASM("asm/funcs", func_8006A3CC);
INCLUDE_ASM("asm/funcs", func_8006A494);
INCLUDE_ASM("asm/funcs", func_8006A564);
INCLUDE_ASM("asm/funcs", func_8006A880);
INCLUDE_ASM("asm/funcs", func_8006B120);
INCLUDE_ASM("asm/funcs", func_8006B578);
INCLUDE_ASM("asm/funcs", func_8006B898);
INCLUDE_ASM("asm/funcs", func_8006B92C);
INCLUDE_ASM("asm/funcs", func_8006BB68);
INCLUDE_ASM("asm/funcs", func_8006BD28);
INCLUDE_ASM("asm/funcs", func_8006BEC4);
INCLUDE_ASM("asm/funcs", func_8006C168);
s32 func_8006C168(s32, s32);
s32 func_8006C1FC(s32 a0, s32 a1) {
    return func_8006C168(a0, a1);
}
INCLUDE_ASM("asm/funcs", func_8006C21C);
INCLUDE_ASM("asm/funcs", func_8006CBD4);
INCLUDE_ASM("asm/funcs", func_8006CCC8);
INCLUDE_ASM("asm/funcs", func_8006CFBC);
INCLUDE_ASM("asm/funcs", func_8006D324);
INCLUDE_ASM("asm/funcs", func_8006D338);
INCLUDE_ASM("asm/funcs", func_8006D3DC);
INCLUDE_ASM("asm/funcs", func_8006D5D4);
INCLUDE_ASM("asm/funcs", func_8006D74C);
INCLUDE_ASM("asm/funcs", func_8006D7FC);
INCLUDE_ASM("asm/funcs", func_8006D808);
INCLUDE_ASM("asm/funcs", func_8006DD94);
INCLUDE_ASM("asm/funcs", func_8006DF68);
INCLUDE_ASM("asm/funcs", func_8006E068);
INCLUDE_ASM("asm/funcs", func_8006E10C);
INCLUDE_ASM("asm/funcs", func_8006E2A8);
INCLUDE_ASM("asm/funcs", func_8006E390);
void func_8006E440(s32 *a0) {
    s32 *p = a0;
    if (*p == -1) return;
    while (*p != -1) {
        *p = *p + (s32)a0;
        p++;
    }
}
INCLUDE_ASM("asm/funcs", func_8006E480);
INCLUDE_ASM("asm/funcs", func_8006E49C);
INCLUDE_ASM("asm/funcs", func_8006E534);
INCLUDE_ASM("asm/funcs", func_8006E8AC);
s32* func_80077D00(void);
void func_8007B33C(s32);
void func_8007B600(s16*, s32);
void func_8006E8CC(s32 *a0) {
    s32 *p;
    s32 data;
    s16 rect[4];
    p = func_80077D00();
    if (p[8] & 1) {
        data = a0[4];
    } else {
        data = a0[3];
    }
    rect[0] = 0;
    rect[1] = 0x1E0;
    rect[2] = 0x280;
    rect[3] = 0x20;
    func_8007B33C(0);
    func_8007B600(rect, data);
    func_8007B33C(0);
}
INCLUDE_ASM("asm/funcs", func_8006E950);
void func_8006920C(s32 *, s32);
void func_8005C2A8(s32, s32, s32);
s32 func_8006EA28(s32 *a0) {
    func_8006920C(a0, a0[21]);
    func_8006920C(a0, a0[22]);
    func_8006920C(a0, a0[23]);
    func_8006920C(a0, a0[24]);
    func_8006920C(a0, a0[25]);
    func_8006920C(a0, a0[26]);
    func_8006920C(a0, a0[27]);
    func_8006920C(a0, a0[28]);
    func_8006920C(a0, a0[29]);
    func_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
INCLUDE_ASM("asm/funcs", func_8006EACC);
INCLUDE_ASM("asm/funcs", func_8006EC0C);
INCLUDE_ASM("asm/funcs", func_8006ECF4);
INCLUDE_ASM("asm/funcs", func_8006F038);
INCLUDE_ASM("asm/funcs", func_8006F100);
INCLUDE_ASM("asm/funcs", func_8006F528);
INCLUDE_ASM("asm/funcs", func_8006F97C);
INCLUDE_ASM("asm/funcs", func_80070188);
INCLUDE_ASM("asm/funcs", func_80070C70);
INCLUDE_ASM("asm/funcs", func_80070F78);
INCLUDE_ASM("asm/funcs", func_80071C20);
INCLUDE_ASM("asm/funcs", func_80071C4C);
INCLUDE_ASM("asm/funcs", func_80072084);
INCLUDE_ASM("asm/funcs", func_800720AC);
INCLUDE_ASM("asm/funcs", func_800720D4);
INCLUDE_ASM("asm/funcs", func_800720FC);
INCLUDE_ASM("asm/funcs", func_80072BC4);
INCLUDE_ASM("asm/funcs", func_80072CD4);
INCLUDE_ASM("asm/funcs", func_80072E10);
void func_8007AAD0(s32);
void func_8007A968(s32, s32);
extern s32 D_800A374C;
void func_8007A8B4(s32, s32);
s32 *func_80072F30(s32 a0, u8 *a1) {
    func_8007AAD0((s32)a1);
    if (a0 < 4) {
        a1[4] = 0x9E;
        a1[5] = 0x64;
        a1[6] = 0;
        func_8007A968((s32)a1, 1);
    } else {
        a1[4] = 0x28;
        a1[5] = 0x28;
        a1[6] = 0x18;
        func_8007A968((s32)a1, 0);
    }
    func_8007A8B4(D_800A374C + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
INCLUDE_ASM("asm/funcs", func_80072FCC);
INCLUDE_ASM("asm/funcs", func_80073060);
INCLUDE_ASM("asm/funcs", func_80073200);
INCLUDE_ASM("asm/funcs", func_8007352C);
INCLUDE_ASM("asm/funcs", func_80073728);
INCLUDE_ASM("asm/funcs", func_80073C78);
INCLUDE_ASM("asm/funcs", func_80074220);
INCLUDE_ASM("asm/funcs", func_80074488);
INCLUDE_ASM("asm/funcs", func_800747D8);
INCLUDE_ASM("asm/funcs", func_80074B18);
INCLUDE_ASM("asm/funcs", func_80074D2C);
INCLUDE_ASM("asm/funcs", func_80074E08);
INCLUDE_ASM("asm/funcs", func_8007526C);
INCLUDE_ASM("asm/funcs", func_800753D8);
INCLUDE_ASM("asm/funcs", func_80075670);
INCLUDE_ASM("asm/funcs", func_80075830);
INCLUDE_ASM("asm/funcs", func_800759D0);
INCLUDE_ASM("asm/funcs", func_80075F80);
INCLUDE_ASM("asm/funcs", func_8007636C);
INCLUDE_ASM("asm/funcs", func_800768DC);
INCLUDE_ASM("asm/funcs", func_80076D74);
void func_8006920C(s32 *, s32);
s32 func_80076FF8(s32 *a0) {
    func_8006920C(a0, a0[5]);
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    func_8006920C(a0, a0[11]);
    func_8006920C(a0, a0[12]);
    func_8006920C(a0, a0[13]);
    func_8006920C(a0, a0[14]);
    return a0[1];
}
INCLUDE_ASM("asm/funcs", func_80077098);
INCLUDE_ASM("asm/funcs", func_800770B8);
INCLUDE_ASM("asm/funcs", func_80077374);
INCLUDE_ASM("asm/funcs", func_80077724);
INCLUDE_ASM("asm/funcs", func_80077820);
INCLUDE_ASM("asm/funcs", func_80077860);
INCLUDE_ASM("asm/funcs", func_80077894);
INCLUDE_ASM("asm/funcs", func_80077904);
INCLUDE_ASM("asm/funcs", func_80077940);
INCLUDE_ASM("asm/funcs", func_80077984);
s32 func_8006EACC(void);
void func_8005B6FC(void);
s32 func_800779C8(void) {
    s32 ret = func_8006EACC();
    if (ret) {
        func_8005B6FC();
    }
    return ret;
}
INCLUDE_ASM("asm/funcs", func_80077A04);
INCLUDE_ASM("asm/funcs", func_80077A28);
void func_8006E068(void);
void func_80077A60(void) {
    func_8006E068();
}
INCLUDE_ASM("asm/funcs", func_80077A80);
void func_80077724(void);
void func_80077AC0(void) {
    func_80077724();
}
void func_8006E10C(void);
void func_80077AE0(void) {
    func_8006E10C();
}
void func_8006E2A8(void);
void func_80077B00(void) {
    func_8006E2A8();
}
INCLUDE_ASM("asm/funcs", func_80077B20);
INCLUDE_ASM("asm/funcs", func_80077B30);
extern s32 D_8009BD24;
s32* func_80077D00(void) {
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
INCLUDE_ASM("asm/funcs", func_80077D74);
INCLUDE_ASM("asm/funcs", func_80077D94);
INCLUDE_ASM("asm/funcs", func_800784E4);
INCLUDE_ASM("asm/funcs", func_8007855C);
s32 func_80078628(s32 *a0) {
    return a0[1];
}
INCLUDE_ASM("asm/funcs", func_80078634);
INCLUDE_ASM("asm/funcs", func_80078654);
INCLUDE_ASM("asm/funcs", func_80078824);
INCLUDE_ASM("asm/funcs", func_800788B0);
INCLUDE_ASM("asm/funcs", func_80078948);
PAD_NOPS_1; /* padding after func_80078948 */
INCLUDE_ASM("asm/funcs", func_80078958);
PAD_NOPS_1; /* padding after func_80078958 */
INCLUDE_ASM("asm/funcs", func_80078968);
PAD_NOPS_1; /* padding after func_80078968 */
INCLUDE_ASM("asm/funcs", func_80078978);
PAD_NOPS_1; /* padding after func_80078978 */
INCLUDE_ASM("asm/funcs", func_80078988);
PAD_NOPS_1; /* padding after func_80078988 */
INCLUDE_ASM("asm/funcs", func_80078998);
PAD_NOPS_1; /* padding after func_80078998 */
INCLUDE_ASM("asm/funcs", func_800789A8);
PAD_NOPS_1; /* padding after func_800789A8 */
INCLUDE_ASM("asm/funcs", func_800789B8);
INCLUDE_ASM("asm/funcs", func_800789C8);
INCLUDE_ASM("asm/funcs", func_800789D8);
PAD_NOPS_1; /* padding after func_800789D8 */
INCLUDE_ASM("asm/funcs", func_800789E8);
PAD_NOPS_1; /* padding after func_800789E8 */
INCLUDE_ASM("asm/funcs", func_800789F8);
PAD_NOPS_1; /* padding after func_800789F8 */
INCLUDE_ASM("asm/funcs", func_80078A08);
PAD_NOPS_1; /* padding after func_80078A08 */
INCLUDE_ASM("asm/funcs", func_80078A18);
PAD_NOPS_1; /* padding after func_80078A18 */
INCLUDE_ASM("asm/funcs", func_80078A28);
PAD_NOPS_1; /* padding after func_80078A28 */
INCLUDE_ASM("asm/funcs", func_80078A38);
PAD_NOPS_1; /* padding after func_80078A38 */
INCLUDE_ASM("asm/funcs", func_80078A48);
PAD_NOPS_1; /* padding after func_80078A48 */
INCLUDE_ASM("asm/funcs", func_80078A58);
PAD_NOPS_1; /* padding after func_80078A58 */
INCLUDE_ASM("asm/funcs", func_80078A68);
INCLUDE_ASM("asm/funcs", func_80078B04);
INCLUDE_ASM("asm/funcs", func_80078B3C);
INCLUDE_ASM("asm/funcs", func_80078B70);
INCLUDE_ASM("asm/funcs", func_80078BA8);
extern s32 D_8009BD80;
void func_80078BE0(s32 a0) {
    D_8009BD80 = a0;
}
extern s32 D_8009BD80;
s32 func_80078BF0(void) {
    return D_8009BD80;
}
void func_800790C0(void);
void func_800789B8(void);
void func_80078F88(void);
void func_800789C8(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F30(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C00(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    func_800789B8();
    func_80078F88();
    func_800789C8();
    func_80078A58(0);
    func_80078DA0();
    func_80078F30(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_800790C0(void);
void func_800789B8(void);
void func_80078F88(void);
void func_800789C8(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F00(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C9C(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    func_800789B8();
    func_80078F88();
    func_800789C8();
    func_80078A58(0);
    func_80078DA0();
    func_80078F00(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_80078F10(void);
void func_80078A58(s32);
void func_80078F60(void);
void func_80078D38(void) {
    func_80078F10();
    func_80078A58(0);
    func_80078F60();
}
extern s32 D_8009BD80;
void func_80078F74(void);
void func_80078F20(void);
s32 func_80078E20(void);
void func_80078D68(void) {
    func_80078F74();
    func_80078F20();
    func_80078E20();
    D_8009BD80 = 0;
}
extern void func_800789B8(void);
extern void func_800789C8(void);
extern void func_80078F50(s32, u32 *);
extern void func_80078F40(s32, u32 *);
extern void func_80078E58(void);
extern s32 func_80078EC0(void);
extern u32 D_800F183C;
extern u32 D_800F1840;
extern u32 D_800F1838;
extern u32 D_800F1844;
s32 func_80078DA0(void) {
    u32 *v1 = &D_800F183C;
    u32 *s0 = v1 - 1;
    func_800789B8();
    *v1 = (u32)func_80078E58;
    D_800F1840 = (u32)func_80078EC0;
    D_800F1838 = 0;
    D_800F1844 = 0;
    func_80078F50(1, s0);
    func_80078F40(1, s0);
    func_800789C8();
    return 1;
}
void func_800789B8(void);
void func_80078F50(s32, s16*);
void func_800789C8(void);
extern s16 D_800F1838;
s32 func_80078E20(void) {
    func_800789B8();
    func_80078F50(1, &D_800F1838);
    func_800789C8();
    return 1;
}
INCLUDE_ASM("asm/funcs", func_80078E58);
INCLUDE_ASM("asm/funcs", func_80078EC0);
INCLUDE_ASM("asm/funcs", func_80078F00);
PAD_NOPS_1; /* padding after func_80078F00 */
INCLUDE_ASM("asm/funcs", func_80078F10);
PAD_NOPS_1; /* padding after func_80078F10 */
INCLUDE_ASM("asm/funcs", func_80078F20);
PAD_NOPS_1; /* padding after func_80078F20 */
INCLUDE_ASM("asm/funcs", func_80078F30);
PAD_NOPS_1; /* padding after func_80078F30 */
INCLUDE_ASM("asm/funcs", func_80078F40);
PAD_NOPS_1; /* padding after func_80078F40 */
INCLUDE_ASM("asm/funcs", func_80078F50);
PAD_NOPS_1; /* padding after func_80078F50 */
INCLUDE_ASM("asm/funcs", func_80078F60);
INCLUDE_ASM("asm/funcs", func_80078F74);
INCLUDE_ASM("asm/funcs", func_80078F88);
INCLUDE_ASM("asm/funcs", func_80078FF0);
INCLUDE_ASM("asm/funcs", func_80079028);
INCLUDE_ASM("asm/funcs", func_800790A4);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
INCLUDE_ASM("asm/funcs", func_800790C0);
PAD_NOPS_1; /* padding after func_800790C0 */
u8* func_80079120(u8 *dst, u8 *src, s32 len) {
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
INCLUDE_ASM("asm/funcs", func_80079208);
INCLUDE_ASM("asm/funcs", func_80079244);
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
INCLUDE_ASM("asm/funcs", func_8007992C);
INCLUDE_ASM("asm/funcs", func_8007997C);
INCLUDE_ASM("asm/funcs", func_80079A30);
