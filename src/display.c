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
extern u8 D_8009BE76;
extern void (*D_8009BE70)();
extern u32 D_80015FB0;
extern u32 D_80015F18;
extern u32 D_80015EE8;
extern u32 D_8009BE80;

extern u8 D_8009BE74;
extern s16 D_8009BE78;
extern s16 D_8009BE7A;
extern u8 D_8009BE77;
extern u32 D_80015F04;
extern u32 D_80015F50;

/* --- Functions 0x8007B244 - 0x8007FF7C (text2 segment) --- */

u32 func_8007B244(s32 a0) {
    u32 old;
    if (D_8009BE76 >= 2) {
        D_8009BE70(&D_80015EE8, a0);
    }
    old = D_8009BE80;
    D_8009BE80 = a0;
    return old;
}
void func_8007B2A0(s32 a0) {
    u8 *p = &D_8009BE76;
    if (*p >= 2) {
        D_8009BE70(&D_80015F04, a0);
    }
    if (!a0) {
        func_8007DEE4(p + 0x6A, -1, 0x14);
    }
    {
        u32 cmd = 0x03000001;
        u32 *v0 = D_8009BE6C;
        if (a0) {
            cmd = 0x03000000;
        }
        ((void (*)(u32))v0[4])(cmd);
    }
}
void func_8007B33C(s32 a0) {
    if (D_8009BE76 >= 2) {
        D_8009BE70(&D_80015F18, a0);
    }
    {
        u32 *v0 = D_8009BE6C;
        ((void (*)(s32))v0[15])(a0);
    }
}
INCLUDE_ASM("asm/funcs", func_8007B3A8);
INCLUDE_ASM("asm/funcs", func_8007B4D0);
INCLUDE_ASM("asm/funcs", func_8007B564);
extern u32 D_80015F5C;

void func_8007B600(s32 a0, s32 a1) {
    u32 *v0;
    func_8007B3A8(&D_80015F5C, a0);
    v0 = D_8009BE6C;
    ((void (*)(u32, s32, s32, s32))v0[2])(v0[8], a0, 8, a1);
}
extern u32 D_80015F68;

void func_8007B664(s32 a0, s32 a1) {
    u32 *v0;
    func_8007B3A8(&D_80015F68, a0);
    v0 = D_8009BE6C;
    ((void (*)(u32, s32, s32, s32))v0[2])(v0[7], a0, 8, a1);
}
INCLUDE_ASM("asm/funcs", func_8007B6C8);
extern u32 D_80015F80;
extern u32 D_8009BF30;

u32 *func_8007B78C(u32 *a0, s32 a1) {
    if (D_8009BE76 >= 2) {
        D_8009BE70(&D_80015F80, a0, a1);
    }
    a1--;
    if (a1) {
        u32 mask = 0xFFFFFF;
        u32 himask = 0xFF000000;
        do {
            u32 *next;
            a1--;
            next = a0 + 1;
            ((u8 *)a0)[3] = 0;
            *a0 = (*a0 & himask) | ((u32)next & mask);
            a0 = next;
        } while (a1);
    }
    *a0 = (u32)&D_8009BF30 & 0xFFFFFF;
    return a0;
}
INCLUDE_ASM("asm/funcs", func_8007B844);
void func_8007B8DC(u8 *a0) {
    u32 *dev = D_8009BE6C;
    u32 size = a0[3];
    ((void (*)(s32))dev[15])(0);
    dev = D_8009BE6C;
    ((void (*)(u32 *, u32))dev[5])(a0 + 4, size);
}
void func_8007B93C(s32 a0) {
    if (D_8009BE76 >= 2) {
        D_8009BE70(&D_80015FB0, a0);
    }
    {
        u32 *v0 = D_8009BE6C;
        ((void (*)(u32, s32, s32, s32))v0[2])(v0[6], a0, 0, 0);
    }
}
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
void func_8007C21C(u8 *a0, s32 a1, s32 a2) {
    u32 v0;
    a0[3] = 2;
    v0 = 0xE6000000;
    if (a1) {
        v0 = 0xE6000002;
    }
    if (a2) {
        v0 |= 1;
    }
    *(u32 *)(a0 + 4) = v0;
    *(u32 *)(a0 + 8) = 0;
}
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
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8007DF10\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x49\n"
    ".set reorder\n"
    ".set at\n"
);
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
extern s16 D_8009BF94[];
extern s16 D_8009AF94[];
extern s16 D_8009B794[];
extern s16 D_8009A794[];

s32 func_8007DF5C(s32 a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return D_8009BF94[a0];
        }
        return D_8009BF94[0x800 - a0];
    }
    if (a0 < 0xC01) {
        return -D_8009AF94[a0];
    }
    return -D_8009BF94[0x1000 - a0];
}
s32 func_8007DFEC(s32 a0) {
    if (a0 < 0) {
        a0 = -a0;
    }
    a0 = a0 & 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return D_8009BF94[0x400 - a0];
        }
        return -D_8009B794[a0];
    }
    if (a0 < 0xC01) {
        return -D_8009BF94[0xC00 - a0];
    }
    return D_8009A794[a0];
}

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
s32 *func_8007E8AC(s32 *a0, s32 *a1, s32 *a2) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 *v0 asm("v0");
    __asm__ volatile (".word 0x48880000" :: "r"(t0));  /* mtc2 $t0, $0 */
    __asm__ volatile (".word 0x48890800" :: "r"(t1));  /* mtc2 $t1, $1 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");              /* mvmva 1,0,0,3,0 */
    __asm__ volatile (".word 0xE8A90000" :: "r"(a1));  /* swc2 $9, 0($a1) */
    __asm__ volatile (".word 0xE8AA0004" :: "r"(a1));  /* swc2 $10, 4($a1) */
    __asm__ volatile (".word 0xE8AB0008" :: "r"(a1));  /* swc2 $11, 8($a1) */
    __asm__ volatile ("addu %0,%1,$0" : "=r"(v0) : "r"(a2));
    return v0;
}
INCLUDE_ASM("asm/funcs", func_8007E8DC);
PAD_NOPS_3; /* 3 NOPs after func_8007E8DC */
INCLUDE_ASM("asm/funcs", func_8007EA0C);
PAD_NOPS_2; /* 2 NOPs after func_8007EA0C */
INCLUDE_ASM("asm/funcs", func_8007EB4C);
PAD_NOPS_1; /* 1 NOP after func_8007EB4C */
INCLUDE_ASM("asm/funcs", func_8007EC5C);
PAD_NOPS_1; /* 1 NOP after func_8007EC5C */
s32 *func_8007ED6C(s32 *a0, s32 *a1, s32 *a2) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 t2 asm("t2") = a0[2];
    register s32 t3 asm("t3") = a0[3];
    register s32 t4 asm("t4") = a0[4];
    register s32 *v0 asm("v0");
    __asm__ volatile (".word 0x48C80000" :: "r"(t0));  /* ctc2 */
    __asm__ volatile (".word 0x48C90800" :: "r"(t1));  /* ctc2 */
    __asm__ volatile (".word 0x48CA1000" :: "r"(t2));  /* ctc2 */
    __asm__ volatile (".word 0x48CB1800" :: "r"(t3));  /* ctc2 */
    __asm__ volatile (".word 0x48CC2000" :: "r"(t4));  /* ctc2 */
    __asm__ volatile (".word 0xC8A00000" :: "r"(a1));  /* lwc2 */
    __asm__ volatile (".word 0xC8A10004" :: "r"(a1));  /* lwc2 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");              /* mvmva 1,0,0,3,0 */
    __asm__ volatile (".word 0xE8D90000" :: "r"(a2));  /* swc2 */
    __asm__ volatile (".word 0xE8DA0004" :: "r"(a2));  /* swc2 */
    __asm__ volatile (".word 0xE8DB0008" :: "r"(a2));  /* swc2 */
    __asm__ volatile ("addu %0,%1,$0" : "=r"(v0) : "r"(a2));
    return v0;
}
INCLUDE_ASM("asm/funcs", func_8007EDBC);
PAD_NOPS_3; /* 3 NOPs after func_8007EDBC */
void func_8007EEEC(s32 *a0) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 t2 asm("t2") = a0[2];
    register s32 t3 asm("t3") = a0[3];
    register s32 t4 asm("t4") = a0[4];
    __asm__ volatile (".word 0x48C80000" :: "r"(t0));  /* ctc2 $t0, $0 */
    __asm__ volatile (".word 0x48C90800" :: "r"(t1));  /* ctc2 $t1, $1 */
    __asm__ volatile (".word 0x48CA1000" :: "r"(t2));  /* ctc2 $t2, $2 */
    __asm__ volatile (".word 0x48CB1800" :: "r"(t3));  /* ctc2 $t3, $3 */
    __asm__ volatile (".word 0x48CC2000" :: "r"(t4));  /* ctc2 $t4, $4 */
}
void func_8007EF1C(s32 *a0) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 t2 asm("t2") = a0[2];
    register s32 t3 asm("t3") = a0[3];
    register s32 t4 asm("t4") = a0[4];
    __asm__ volatile (".word 0x48C88000" :: "r"(t0));  /* ctc2 $t0, $16 */
    __asm__ volatile (".word 0x48C98800" :: "r"(t1));  /* ctc2 $t1, $17 */
    __asm__ volatile (".word 0x48CA9000" :: "r"(t2));  /* ctc2 $t2, $18 */
    __asm__ volatile (".word 0x48CB9800" :: "r"(t3));  /* ctc2 $t3, $19 */
    __asm__ volatile (".word 0x48CCA000" :: "r"(t4));  /* ctc2 $t4, $20 */
}
void func_8007EF4C(s32 *a0) {
    register s32 t0 asm("t0") = a0[5];
    register s32 t1 asm("t1") = a0[6];
    register s32 t2 asm("t2") = a0[7];
    __asm__ volatile (".word 0x48C82800" :: "r"(t0));  /* ctc2 $t0, $5 */
    __asm__ volatile (".word 0x48C93000" :: "r"(t1));  /* ctc2 $t1, $6 */
    __asm__ volatile (".word 0x48CA3800" :: "r"(t2));  /* ctc2 $t2, $7 */
}
void func_8007EF6C(s32 *a0, s32 *a1, s32 *a2) {
    __asm__ volatile (".word 0xE8910000" :: "r"(a0));  /* swc2 $17, 0($a0) */
    __asm__ volatile (".word 0xE8B20000" :: "r"(a1));  /* swc2 $18, 0($a1) */
    __asm__ volatile (".word 0xE8D30000" :: "r"(a2));  /* swc2 $19, 0($a2) */
}
PAD_NOPS_3; /* 3 NOPs after func_8007EF6C */
s32 func_8007EF8C(void) { s32 ret; __asm__ volatile (".word 0x4842D000" : "=r" (ret)); return ret; }
PAD_NOPS_1; /* 1 NOP after func_8007EF8C */
void func_8007EF9C(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile (".word 0x48C46800" :: "r"(a0));  /* ctc2 $a0, $13 */
    __asm__ volatile (".word 0x48C57000" :: "r"(a1));  /* ctc2 $a1, $14 */
    __asm__ volatile (".word 0x48C67800" :: "r"(a2));  /* ctc2 $a2, $15 */
}
void func_8007EFBC(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile (".word 0x48C4A800" :: "r"(a0));  /* ctc2 $a0, $21 */
    __asm__ volatile (".word 0x48C5B000" :: "r"(a1));  /* ctc2 $a1, $22 */
    __asm__ volatile (".word 0x48C6B800" :: "r"(a2));  /* ctc2 $a2, $23 */
}
void func_8007EFDC(s32 a0, s32 a1) {
    a0 <<= 16;
    a1 <<= 16;
    __asm__ volatile (".word 0x48C4C000" :: "r"(a0));  /* ctc2 $a0, $24 */
    __asm__ volatile (".word 0x48C5C800" :: "r"(a1));  /* ctc2 $a1, $25 */
}
PAD_NOPS_2; /* 2 NOPs after func_8007EFDC */
INCLUDE_ASM("asm/funcs", func_8007EFFC);
INCLUDE_ASM("asm/funcs", func_8007F0BC);
PAD_NOPS_1; /* 1 NOP after func_8007F0BC */
s32 func_8007F21C(s32 *a0, s32 *a1, s32 *a2, s32 *a3) {
    register s32 v1 asm("v1");
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0xC8800000" :: "r"(a0));  /* lwc2 $0, 0($a0) */
    __asm__ volatile (".word 0xC8810004" :: "r"(a0));  /* lwc2 $1, 4($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A180001");              /* rtps */
    __asm__ volatile (".word 0xE8AE0000" :: "r"(a1));  /* swc2 $14, 0($a1) */
    __asm__ volatile (".word 0xE8C80000" :: "r"(a2));  /* swc2 $8, 0($a2) */
    __asm__ volatile (".word 0x4843F800" : "=r"(v1));  /* cfc2 $v1, $31 */
    __asm__ volatile (".word 0x48029800" : "=r"(v0));  /* mfc2 $v0, $19 */
    *a3 = v1;
    return v0 >> 2;
}
PAD_NOPS_1; /* 1 NOP after func_8007F21C */
INCLUDE_ASM("asm/funcs", func_8007F24C);
PAD_NOPS_3; /* 3 NOPs after func_8007F24C */
void func_8007F2AC(s32 *a0, s32 *a1, s32 *a2) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0xC8800000" :: "r"(a0));  /* lwc2 $0, 0($a0) */
    __asm__ volatile (".word 0xC8810004" :: "r"(a0));  /* lwc2 $1, 4($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");              /* mvmva 1,0,0,0,0 */
    __asm__ volatile (".word 0xE8B90000" :: "r"(a1));  /* swc2 $25, 0($a1) */
    __asm__ volatile (".word 0xE8BA0004" :: "r"(a1));  /* swc2 $26, 4($a1) */
    __asm__ volatile (".word 0xE8BB0008" :: "r"(a1));  /* swc2 $27, 8($a1) */
    __asm__ volatile (".word 0x4842F800" : "=r"(v0));  /* cfc2 $v0, $31 */
    *a2 = v0;
}
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
