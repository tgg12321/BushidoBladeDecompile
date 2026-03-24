#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */

INCLUDE_ASM("asm/funcs", func_800401CC);
INCLUDE_ASM("asm/funcs", func_80040304);
INCLUDE_ASM("asm/funcs", func_80040400);
s32 func_8004046C(s32 a0, s32 a1) {
    s32 *base = (s32 *)func_8004153C(a0);
    return *(s32 *)((u8 *)base + a1 * 4 + 0x1A34);
}
INCLUDE_ASM("asm/funcs", func_800404A0);
INCLUDE_ASM("asm/funcs", func_800404D8);
INCLUDE_ASM("asm/funcs", func_80040510);
INCLUDE_ASM("asm/funcs", func_80040594);
INCLUDE_ASM("asm/funcs", func_800408F8);
INCLUDE_ASM("asm/funcs", func_80040A78);
INCLUDE_ASM("asm/funcs", func_80040B44);
INCLUDE_ASM("asm/funcs", func_80040CB8);
INCLUDE_ASM("asm/funcs", func_80040D48);
INCLUDE_ASM("asm/funcs", func_80041188);
INCLUDE_ASM("asm/funcs", func_80041398);
INCLUDE_ASM("asm/funcs", func_80041430);
INCLUDE_ASM("asm/funcs", func_800414FC);
extern s32 D_800A9A10[];
s32 func_8004153C(s32 a0) {
    return D_800A9A10[a0];
}
s32 func_80041554(s32 a0) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        return ptr[4];
    }
    return -1;
}
INCLUDE_ASM("asm/funcs", func_80041584);
extern void func_8004016C(s32);
extern void func_80045A50(s32);
void func_800415C4(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    D_800A9A10[a0] = 0;
}
extern s32 D_80094B88[];
void func_80041604(s32 a0, s32 a1) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }
    }
    D_80094B88[a0] = a1;
}
s32 func_80041650(s32 a0) {
    s16 *ptr = (s16 *)D_800A9A10[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;
    }
    return -1;
}
INCLUDE_ASM("asm/funcs", func_80041688);
INCLUDE_ASM("asm/funcs", func_800417D0);
INCLUDE_ASM("asm/funcs", func_800418D0);
INCLUDE_ASM("asm/funcs", func_80041988);
INCLUDE_ASM("asm/funcs", func_80041AC8);
INCLUDE_ASM("asm/funcs", func_80041BF4);
INCLUDE_ASM("asm/funcs", func_80041E10);
INCLUDE_ASM("asm/funcs", func_80041EB0);
INCLUDE_ASM("asm/funcs", func_800420D0);
extern s16 D_800A3380[];
extern s32 D_800A3384[];
void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        D_800A3380[a0] = 1;
        D_800A3384[a0] = a1;
    }
}
INCLUDE_ASM("asm/funcs", func_8004211C);
extern void func_80041EB0(s32, s32);
void func_800421A4(void) {
    func_80041EB0(0, 0);
}
INCLUDE_ASM("asm/funcs", func_800421C8);
INCLUDE_ASM("asm/funcs", func_800422BC);
INCLUDE_ASM("asm/funcs", func_80042478);
INCLUDE_ASM("asm/funcs", func_80042504);
INCLUDE_ASM("asm/funcs", func_80042684);
extern s16 D_800F6650;
void func_8004283C(s32 a0) {
    if (a0) {
        D_800F6650 = 1;
    } else {
        D_800F6650 = 0;
    }
}
extern s16 D_800F6650;
s32 func_80042864(void) {
    return D_800F6650;
}
INCLUDE_ASM("asm/funcs", func_80042874);
INCLUDE_ASM("asm/funcs", func_80042A88);
INCLUDE_ASM("asm/funcs", func_80042C80);
extern void func_8004A348(void);
extern void func_80042874(void);
extern void func_80042A88(void);
extern void func_80042C80(void);
extern u32 D_800F66A0;
extern u32 D_800F66A8;
extern u32 D_800F66B0;
extern u32 D_800F66B4;
void func_80042E90(void) {
    D_800F66A0 = (u32)func_8004A348;
    D_800F66A8 = (u32)func_80042874;
    D_800F66B0 = (u32)func_80042A88;
    D_800F66B4 = (u32)func_80042C80;
}
void func_80042ED8(u16 *a0) {
    u16 t0, t1, t2, t3;
    t0 = a0[6];
    t1 = a0[2];
    t2 = a0[1];
    t3 = a0[5];
    a0[2] = t0;
    t0 = a0[3];
    a0[1] = t0;
    t0 = a0[7];
    a0[6] = t1;
    a0[3] = t2;
    a0[7] = t3;
    a0[5] = t0;
}
INCLUDE_ASM("asm/funcs", func_80042F10);
INCLUDE_ASM("asm/funcs", func_80042FA0);
INCLUDE_ASM("asm/funcs", func_800430E4);
INCLUDE_ASM("asm/funcs", func_80043244);
INCLUDE_ASM("asm/funcs", func_80043278);
INCLUDE_ASM("asm/funcs", func_800432A0);
extern void func_800432A0(s16, s16, s16, s16, s16);
void func_80043398(s16 a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    func_800432A0(a0, (s16)(a1 << 6), (s16)(a2 << 8), (s16)(a3 << 6), (s16)(a4 << 8));
}
INCLUDE_ASM("asm/funcs", func_800433E4);
INCLUDE_ASM("asm/funcs", func_80043454);
INCLUDE_ASM("asm/funcs", func_80043BD0);
INCLUDE_ASM("asm/funcs", func_80043C7C);
INCLUDE_ASM("asm/funcs", func_80043D34);
INCLUDE_ASM("asm/funcs", func_80043DE0);
INCLUDE_ASM("asm/funcs", func_80043E98);
INCLUDE_ASM("asm/funcs", func_80043F0C);
INCLUDE_ASM("asm/funcs", func_80043F80);
INCLUDE_ASM("asm/funcs", func_80043FCC);
INCLUDE_ASM("asm/funcs", func_80044010);
INCLUDE_ASM("asm/funcs", func_80044098);
INCLUDE_ASM("asm/funcs", func_80044100);
INCLUDE_ASM("asm/funcs", func_80044170);
INCLUDE_ASM("asm/funcs", func_8004428C);
INCLUDE_ASM("asm/funcs", func_80044378);
extern s16 D_8010367E;
void func_80044498(void) {
    s32 i = 0x13;
    s16 *p = &D_8010367E;
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}
INCLUDE_ASM("asm/funcs", func_800444BC);
INCLUDE_ASM("asm/funcs", func_800444E0);
INCLUDE_ASM("asm/funcs", func_80044504);
extern void func_80052C10(void);
void func_80044650(void) {
    func_80052C10();
}
INCLUDE_ASM("asm/funcs", func_80044670);
INCLUDE_ASM("asm/funcs", func_8004473C);
INCLUDE_ASM("asm/funcs", func_80044800);
INCLUDE_ASM("asm/funcs", func_80044B30);
extern s16 D_800A9CF8;
extern s32 D_800A9D04;
extern s32 D_800A9D00;
extern void func_80044100(s32, s32);
void func_80044C70(s32 a0) {
    func_80044100((s32)D_800A9CF8, a0);
    D_800A9D04 += a0;
    D_800A9D00 += a0;
}
INCLUDE_ASM("asm/funcs", func_80044CCC);
INCLUDE_ASM("asm/funcs", func_80044DE4);
s32 func_80044E64(void) {
    return 0x25;
}
s32 func_80044E6C(void) {
    return 0x26;
}
extern void func_80036F40(void);
INCLUDE_ASM("asm/funcs", func_80044E74);
extern s32 func_800450F4(s32, s32);
void func_80044ED8(s32 a0, s32 a1) {
    if (a0 >= 0x1F) {
        a0 -= 0x1B;
    }
    if (!func_800450F4(a0, a1)) {
        func_80044E74(a0, a1);
    }
}
extern void func_80044E74(s32, s32);
void func_80044F30(s32 a0) {
    func_80044E74(a0 + 0x27);
}
void func_80044F50(s32 a0, s32 a1, s32 a2) {
    if (!a0) {
        func_80044E74(a1 + 0x83, a2);
    } else {
        func_80044E74(a1 + 0x10C, a2);
    }
}
void func_80044F80(s32 a0, s32 a1) {
    func_80044E74(a0 + 0x4D, a1);
}
INCLUDE_ASM("asm/funcs", func_80044FA0);
extern s16 D_800963EE;
extern s32 func_800457DC(void);
s32 func_80045080(s32 a0) {
    s32 val = (s32)*(s16 *)((u8 *)&D_800963EE + a0 * 4) << 11;
    return func_800457DC() - val;
}
INCLUDE_ASM("asm/funcs", func_800450BC);
INCLUDE_ASM("asm/funcs", func_800450F4);
INCLUDE_ASM("asm/funcs", func_80045188);
INCLUDE_ASM("asm/funcs", func_80045194);
extern u8 D_800963EC;
extern void func_80036E34(s32, void *, s32, s32);
void func_800451A0(void) {
    func_80036E34(1, &D_800963EC, 0, 2);
}
INCLUDE_ASM("asm/funcs", func_800451D0);
INCLUDE_ASM("asm/funcs", func_80045230);
INCLUDE_ASM("asm/funcs", func_80045294);
INCLUDE_ASM("asm/funcs", func_800453E0);
INCLUDE_ASM("asm/funcs", func_80045510);
INCLUDE_ASM("asm/funcs", func_800455AC);
INCLUDE_ASM("asm/funcs", func_80045600);
INCLUDE_ASM("asm/funcs", func_80045694);
INCLUDE_ASM("asm/funcs", func_800456F0);
INCLUDE_ASM("asm/funcs", func_8004574C);
extern s32 *func_8004574C(void);
s32 func_800457A0(void) {
    s32 *v0 = func_8004574C();
    if (v0) {
        return v0[1];
    }
    return 0;
}
void func_800457D4(void) {
}
INCLUDE_ASM("asm/funcs", func_800457DC);
INCLUDE_ASM("asm/funcs", func_800457E8);
INCLUDE_ASM("asm/funcs", func_800457FC);
s32 func_80045808(void) {
    return 0x45000;
}
extern u8 D_800A9D10;
void *func_80045814(void) {
    return &D_800A9D10;
}
extern void func_800520B8(s32, s32, s32);
void func_80045824(s32 a0, s32 a1, s32 a2) {
    func_80045230(a1 + a2);
    func_800520B8(a0, a1, a2);
}
INCLUDE_ASM("asm/funcs", func_80045878);
void func_80045A28(s32 a0, s32 a1) {
    func_80045510(a0 + 3, a1);
    func_80045230(0);
}
extern void func_8005B644(void);
extern void func_800456F0(s32);
void func_80045A50(s32 a0) {
    s32 a0p3 = a0 + 3;
    func_8005B644();
    func_800456F0(a0p3);
    func_800456F0(a0);
    func_800453E0(a0p3);
    func_800453E0(a0);
}
INCLUDE_ASM("asm/funcs", func_80045AA4);
INCLUDE_ASM("asm/funcs", func_80045B68);
extern void func_8005B6AC(void);
void func_80046020(void) {
    func_800453E0(6);
    func_8005B6AC();
}
INCLUDE_ASM("asm/funcs", func_80046048);
INCLUDE_ASM("asm/funcs", func_800460E4);
INCLUDE_ASM("asm/funcs", func_800464C4);
INCLUDE_ASM("asm/funcs", func_8004659C);
INCLUDE_ASM("asm/funcs", func_8004668C);
INCLUDE_ASM("asm/funcs", func_800466C0);
