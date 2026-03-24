#include "common.h"
#include "include_asm.h"

/* Padding NOP macro - emits NOP instructions between functions to match original layout */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")


/* Forward declarations */
extern void func_8007D3F8(s32, s32, s32, s32);
extern s32 func_800828CC(s32);
extern s32 func_80079120(s32, void *, s32);
extern u32 func_8007C97C(s32);
extern u32 func_8007C938(s32, s32);
extern u32 func_8007C7A0(s32, s32);
extern u32 func_8007C86C(s32, s32);

/* Externs for globals */
extern u32 *D_8009BF48;
extern u32 *D_8009BF44;
extern u32 *D_8009BF4C;
extern u32 *D_8009BF50;
extern u32 *D_8009BF54;
extern u8 D_800F189C[];
extern u32 *D_8009BE6C;
extern u8 D_8009BE84;
extern u8 D_8009BEE0;
extern s32 D_8009BF8C;
extern s32 D_8009BF90;

/* --- Functions 0x8007B244 - 0x8007FF7C (text2 segment) --- */

INCLUDE_ASM("asm/funcs", func_8007B244);
INCLUDE_ASM("asm/funcs", func_8007B2A0);
INCLUDE_ASM("asm/funcs", func_8007B33C);
INCLUDE_ASM("asm/funcs", func_8007B3A8);
INCLUDE_ASM("asm/funcs", func_8007B4D0);
INCLUDE_ASM("asm/funcs", func_8007B564);
INCLUDE_ASM("asm/funcs", func_8007B600);
INCLUDE_ASM("asm/funcs", func_8007B664);
INCLUDE_ASM("asm/funcs", func_8007B6C8);
INCLUDE_ASM("asm/funcs", func_8007B78C);
INCLUDE_ASM("asm/funcs", func_8007B844);
INCLUDE_ASM("asm/funcs", func_8007B8DC);
INCLUDE_ASM("asm/funcs", func_8007B93C);
INCLUDE_ASM("asm/funcs", func_8007B9B0);
INCLUDE_ASM("asm/funcs", func_8007BAB4);
s32 func_8007BBD0(s32 a0) {
    func_80079120(a0, &D_8009BE84, 0x5C);
    return a0;
}
INCLUDE_ASM("asm/funcs", func_8007BC08);
s32 func_8007C0B0(s32 a0) {
    func_80079120(a0, &D_8009BEE0, 0x14);
    return a0;
}
u32 func_8007C0E8(void) {
    s32 (*func)(void) = ((s32 (**)(void))D_8009BE6C)[0xE];
    return (u32)func() >> 31;
}
void func_8007C118(u8 *a0, s32 a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C97C(a1);
    *(u32 *)(a0 + 8) = 0;
}
void func_8007C154(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C7A0(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = func_8007C86C((s32)(s16)((u16)a1[0] + (u16)a1[2] - 1), (s32)(s16)((u16)a1[1] + (u16)a1[3] - 1));
}
void func_8007C1D8(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C938(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = 0;
}
INCLUDE_ASM("asm/funcs", func_8007C21C);
extern u32 func_8007C748(s32, s32, u16);
void func_8007C248(u8 *a0, s32 a1, s32 a2, u16 a3, s32 a4) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C748(a1, a2, a3);
    *(u32 *)(a0 + 8) = func_8007C97C(a4);
}
INCLUDE_ASM("asm/funcs", func_8007C2A0);
INCLUDE_ASM("asm/funcs", func_8007C4B8);
INCLUDE_ASM("asm/funcs", func_8007C748);
INCLUDE_ASM("asm/funcs", func_8007C7A0);
INCLUDE_ASM("asm/funcs", func_8007C86C);
INCLUDE_ASM("asm/funcs", func_8007C938);
INCLUDE_ASM("asm/funcs", func_8007C97C);
INCLUDE_ASM("asm/funcs", func_8007CA00);
u32 func_8007CAB0(void) {
    return *D_8009BF48;
}
INCLUDE_ASM("asm/funcs", func_8007CAC8);
INCLUDE_ASM("asm/funcs", func_8007CBB0);
INCLUDE_ASM("asm/funcs", func_8007CE0C);
INCLUDE_ASM("asm/funcs", func_8007D048);
void func_8007D2CC(u32 a0) {
    *D_8009BF48 = a0;
    D_800F189C[a0 >> 24] = a0;
}
u32 func_8007D2F4(s32 a0) {
    return D_800F189C[a0];
}
s32 func_8007D308(u32 *a0, s32 a1) {
    s32 i;
    *(volatile u32 *)D_8009BF48 = 0x04000000;
    for (i = a1 - 1; i != -1; i--) {
        *(volatile u32 *)D_8009BF44 = *a0++;
    }
    return 0;
}
void func_8007D358(u32 a0) {
    *(volatile u32 *)D_8009BF48 = 0x04000002;
    *(volatile u32 *)D_8009BF4C = a0;
    *(volatile u32 *)D_8009BF50 = 0;
    *(volatile u32 *)D_8009BF54 = 0x01000401;
}
u32 func_8007D3A4(u32 a0) {
    *D_8009BF48 = a0 | 0x10000000;
    return *D_8009BF44 & 0xFFFFFF;
}
void func_8007D3D4(s32 a0, s32 a1, s32 a2) {
    func_8007D3F8(a0, a1, 0, a2);
}
INCLUDE_ASM("asm/funcs", func_8007D3F8);
INCLUDE_ASM("asm/funcs", func_8007D6D8);
INCLUDE_ASM("asm/funcs", func_8007D9C4);
INCLUDE_ASM("asm/funcs", func_8007DB20);
void func_8007DC68(void) {
    D_8009BF8C = func_800828CC(-1) + 0xF0;
    D_8009BF90 = 0;
}
INCLUDE_ASM("asm/funcs", func_8007DC9C);
INCLUDE_ASM("asm/funcs", func_8007DE08);
void func_8007DEE4(u8 *a0, u8 a1, s32 a2) {
    s32 i;
    for (i = a2 - 1; i != -1; i--) {
        *a0++ = a1;
    }
}
INCLUDE_ASM("asm/funcs", func_8007DF10);
PAD_NOPS_1; /* 1 NOP after func_8007DF10 */
extern s32 func_8007DF5C(s32);
s32 func_8007DF20(s32 a0) {
    s32 v;
    if (a0 < 0) {
        v = func_8007DF5C((-a0) & 0xFFF);
        return -v;
    }
    return func_8007DF5C(a0 & 0xFFF);
}
INCLUDE_ASM("asm/funcs", func_8007DF5C);
INCLUDE_ASM("asm/funcs", func_8007DFEC);

/* Data blob D_8007E08C between func_8007DFEC and func_8007E094 */
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .include \"asm/funcs/D_8007E08C.s\"\n"
    "    .set reorder\n"
    "    .set at\n"
);

INCLUDE_ASM("asm/funcs", func_8007E094);
PAD_NOPS_2; /* 2 NOPs after func_8007E094 */
INCLUDE_ASM("asm/funcs", func_8007E11C);
PAD_NOPS_3; /* 3 NOPs after func_8007E11C */
INCLUDE_ASM("asm/funcs", func_8007E1AC);
PAD_NOPS_1; /* 1 NOP after func_8007E1AC */
INCLUDE_ASM("asm/funcs", func_8007E43C);
PAD_NOPS_3; /* 3 NOPs after func_8007E43C */
INCLUDE_ASM("asm/funcs", func_8007E4DC);
PAD_NOPS_1; /* 1 NOP after func_8007E4DC */
INCLUDE_ASM("asm/funcs", func_8007E5EC);
INCLUDE_ASM("asm/funcs", func_8007E74C);
INCLUDE_ASM("asm/funcs", func_8007E8AC);
INCLUDE_ASM("asm/funcs", func_8007E8DC);
PAD_NOPS_3; /* 3 NOPs after func_8007E8DC */
INCLUDE_ASM("asm/funcs", func_8007EA0C);
PAD_NOPS_2; /* 2 NOPs after func_8007EA0C */
INCLUDE_ASM("asm/funcs", func_8007EB4C);
PAD_NOPS_1; /* 1 NOP after func_8007EB4C */
INCLUDE_ASM("asm/funcs", func_8007EC5C);
PAD_NOPS_1; /* 1 NOP after func_8007EC5C */
INCLUDE_ASM("asm/funcs", func_8007ED6C);
INCLUDE_ASM("asm/funcs", func_8007EDBC);
PAD_NOPS_3; /* 3 NOPs after func_8007EDBC */
INCLUDE_ASM("asm/funcs", func_8007EEEC);
INCLUDE_ASM("asm/funcs", func_8007EF1C);
INCLUDE_ASM("asm/funcs", func_8007EF4C);
INCLUDE_ASM("asm/funcs", func_8007EF6C);
PAD_NOPS_3; /* 3 NOPs after func_8007EF6C */
INCLUDE_ASM("asm/funcs", func_8007EF8C);
PAD_NOPS_1; /* 1 NOP after func_8007EF8C */
INCLUDE_ASM("asm/funcs", func_8007EF9C);
INCLUDE_ASM("asm/funcs", func_8007EFBC);
INCLUDE_ASM("asm/funcs", func_8007EFDC);
PAD_NOPS_2; /* 2 NOPs after func_8007EFDC */
INCLUDE_ASM("asm/funcs", func_8007EFFC);
INCLUDE_ASM("asm/funcs", func_8007F0BC);
PAD_NOPS_1; /* 1 NOP after func_8007F0BC */
INCLUDE_ASM("asm/funcs", func_8007F21C);
PAD_NOPS_1; /* 1 NOP after func_8007F21C */
INCLUDE_ASM("asm/funcs", func_8007F24C);
PAD_NOPS_3; /* 3 NOPs after func_8007F24C */
INCLUDE_ASM("asm/funcs", func_8007F2AC);
PAD_NOPS_2; /* 2 NOPs after func_8007F2AC */
INCLUDE_ASM("asm/funcs", func_8007F2DC);
PAD_NOPS_2; /* 2 NOPs after func_8007F2DC */
INCLUDE_ASM("asm/funcs", func_8007F35C);
PAD_NOPS_1; /* 1 NOP after func_8007F35C */
INCLUDE_ASM("asm/funcs", func_8007F5EC);
PAD_NOPS_1; /* 1 NOP after func_8007F5EC */
INCLUDE_ASM("asm/funcs", func_8007F87C);
PAD_NOPS_2; /* 2 NOPs after func_8007F87C */
INCLUDE_ASM("asm/funcs", func_8007FA1C);
PAD_NOPS_2; /* 2 NOPs after func_8007FA1C */
INCLUDE_ASM("asm/funcs", func_8007FBBC);
PAD_NOPS_2; /* 2 NOPs after func_8007FBBC */
INCLUDE_ASM("asm/funcs", func_8007FD5C);
INCLUDE_ASM("asm/funcs", func_8007FEDC);
PAD_NOPS_1; /* 1 NOP after func_8007FEDC */
INCLUDE_ASM("asm/funcs", func_8007FF7C);
