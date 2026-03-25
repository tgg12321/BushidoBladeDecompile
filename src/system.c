#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

/* Forward declarations */
extern void func_800817A0(void);
extern void func_80080DB0(void);
extern void func_80081030(void);
extern s32 func_80081718();
extern s32 func_80081D1C();
extern s32 func_80081E1C(void);
extern void func_80082B20(s32, s32);
extern s32 func_80081BB0(s32);

/* Externs for globals */
extern u8 D_800A11C4;
extern u8 D_800A11D0;
extern u8 D_800A11D4;
extern u8 D_800A11D5;
extern s32 D_800A11B4;
extern s32 D_800A11B8;

/* --- Functions 0x8008008C - 0x800807A8 --- */

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008008C\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x7\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);

u32 func_8008009C(void) {
    return D_800A11C4;
}

u32 func_800800AC(void) {
    return D_800A11D4;
}

u32 func_800800BC(void) {
    return D_800A11D5;
}

void *func_800800CC(void) {
    return &D_800A11D0;
}

extern void func_80081974(void);
extern s32 func_800819C4(void);
extern s32 func_80081880(void);
s32 func_800800DC(s32 a0) {
    if (a0 == 2) {
        func_80081974();
        return 1;
    }
    if (func_800819C4() != 0) {
        return 0;
    }
    if (a0 == 1) {
        if (func_80081880() != 0) {
            return 0;
        }
    }
    return 1;
}

void func_80080148(void) {
    func_800817A0();
}

extern s32 D_800A11C0;
extern s32 D_800A11DC[];
extern s32 D_800A125C[];
extern char D_80016074;

s32 func_80080168(s32 a0) {
    s32 old = D_800A11C0;
    D_800A11C0 = a0;
    return old;
}

void *func_80080180(u32 a0) {
    u32 idx = a0 & 0xFF;
    void *ret;
    if (idx < 0x1C) {
        ret = (void *)D_800A11DC[idx];
        goto done;
    }
    ret = &D_80016074;
done:
    return ret;
}

void *func_800801B4(u32 a0) {
    u32 idx = a0 & 0xFF;
    void *ret;
    if (idx < 0x7) {
        ret = (void *)D_800A125C[idx];
        goto done;
    }
    ret = &D_80016074;
done:
    return ret;
}

void func_800801E8(void) {
    func_80080DB0();
}

void func_80080208(void) {
    func_80081030();
}

s32 func_80080228(s32 a0) {
    s32 old = D_800A11B4;
    D_800A11B4 = a0;
    return old;
}

s32 func_80080240(s32 a0) {
    s32 old = D_800A11B8;
    D_800A11B8 = a0;
    return old;
}

extern s32 D_800A112C[];
extern s32 func_800812FC(s32, void *, void *, s32);

INCLUDE_ASM("asm/funcs", func_80080258);
INCLUDE_ASM("asm/funcs", func_80080390);
INCLUDE_ASM("asm/funcs", func_800804BC);

s32 func_80080600(void) {
    func_80081718();
    return 1;
}

s32 func_80080620(void) {
    return func_80081D1C() == 0;
}

s32 func_80080640(void) {
    return func_80081E1C() == 0;
}

void func_80080660(s32 a0) {
    func_80082B20(3, a0);
}

void func_80080684(s32 a0) {
    func_80081BB0(a0);
}

INCLUDE_ASM("asm/funcs", func_800806A4);

extern s32 D_800A112C[];
extern s32 func_800812FC(s32, void *, void *, s32);
extern s32 func_80080DB0_ret(s32, void *);
/* --- text3 segment functions (0x800807A8-0x800827D0, 17 funcs) --- */

s32 func_800807A8(u8 *a0) {
    u8 b0 = a0[0];
    u8 b1 = a0[1];
    s32 min, sec, frm;
    min = (b0 >> 4) * 10 + (b0 & 0xF);
    sec = min * 60;
    sec += (b1 >> 4) * 10 + (b1 & 0xF);
    {
        s32 total = sec * 75;
        u8 b2 = a0[2];
        frm = (b2 >> 4) * 10 + (b2 & 0xF);
        total += frm;
        return total - 150;
    }
}
INCLUDE_ASM("asm/funcs", func_80080828);
INCLUDE_ASM("asm/funcs", func_80080DB0);
INCLUDE_ASM("asm/funcs", func_80081030);
INCLUDE_ASM("asm/funcs", func_800812FC);
extern volatile u8 *D_800A147C;
extern volatile u8 *D_800A1484;
extern volatile u8 *D_800A1488;
extern volatile u8 *D_800A1480;

s32 func_80081718(u8 *a0) {
    *D_800A147C = 2;
    *D_800A1484 = a0[0];
    *D_800A1488 = a0[1];
    *D_800A147C = 3;
    *D_800A1480 = a0[2];
    *D_800A1484 = a0[3];
    *D_800A1488 = 0x20;
    return 0;
}
extern volatile u32 *D_800A148C;
extern volatile u32 *D_800A14C0;
extern volatile u8 D_800A1494;
extern volatile u8 D_800A1495;
extern volatile u8 D_800A1496;
void func_800817A0(void) {
    u8 v0;
    volatile u8 *p94;
    *D_800A147C = 1;
    v0 = *D_800A1488 & 7;
    if (v0 != 0) {
        do {
            *D_800A147C = 1;
            *D_800A1488 = 7;
            *D_800A1484 = 7;
            v0 = *D_800A1488 & 7;
        } while (v0 != 0);
    }
    D_800A1496 = 0;
    v0 = D_800A1496;
    p94 = &D_800A1494;
    D_800A1495 = v0;
    *p94 = 2;
    *D_800A147C = 0;
    *D_800A1488 = 0;
    *D_800A148C = 0x1325;
}
extern volatile u16 * volatile D_800A1490;
s32 func_80081880(void) {
    u8 buf[4];
    volatile u16 *v1;
    v1 = D_800A1490;
    if (v1[0xDC] == 0) {
        if (v1[0xDD] == 0) {
            v1[0xC0] = 0x3FFF;
            v1[0xC1] = 0x3FFF;
            v1 = D_800A1490;
        }
    }
    v1[0xD8] = 0x3FFF;
    v1[0xD9] = 0x3FFF;
    v1[0xD5] = 0xC001;
    buf[2] = 0x80;
    buf[0] = 0x80;
    buf[3] = 0;
    buf[1] = 0;
    *D_800A147C = 2;
    *D_800A1484 = buf[0];
    *D_800A1488 = buf[1];
    *D_800A147C = 3;
    *D_800A1480 = buf[2];
    *D_800A1484 = buf[3];
    *D_800A1488 = 0x20;
    return 0;
}
extern s32 D_800A11C8;
extern void func_80082AC0(void);
extern void func_80082AF0(s32, void *);
extern u8 D_80081F1C;
void func_80081974(void) {
    D_800A11B8 = 0;
    D_800A11B4 = 0;
    D_800A11C8 = 0;
    *(s32 *)&D_800A11C4 = 0;
    func_80082AC0();
    func_80082AF0(2, &D_80081F1C);
}
INCLUDE_ASM("asm/funcs", func_800819C4);
extern s32 func_800828CC(s32);
extern void func_80082000(void *);
extern void func_80079208(void *, void *, s32, s32, s32);
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
extern s32 D_800161B8;
extern s32 D_800161C8;
extern void D_800162C0;
INCLUDE_ASM("asm/funcs", func_80081BB0);
extern volatile u32 *D_800A148C;
extern volatile u32 *D_800A14B0;
extern volatile u32 *D_800A14B4;
extern volatile u32 *D_800A14B8;
extern volatile u32 *D_800A14BC;
extern volatile u32 *D_800A14C0;

s32 func_80081D1C(s32 a0, s32 a1) {
    volatile u8 *v1;
    u32 v0;
    *D_800A147C = 0;
    *D_800A1488 = 0x80;
    *D_800A14B0 = 0x20943;
    *D_800A148C = 0x1323;
    *D_800A14B4 = *D_800A14B4 | 0x8000;
    *D_800A14B8 = a0;
    *D_800A14BC = a1 | 0x10000;
    v1 = D_800A147C;
    do {
        __asm__ volatile("nop");
        v0 = *v1 & 0x40;
    } while (v0 == 0);
    *D_800A14C0 = 0x11000000;
    if ((*D_800A14C0 & 0x1000000) != 0) {
        do {
            v0 = *D_800A14C0 & 0x1000000;
        } while (v0 != 0);
    }
    *D_800A148C = 0x1325;
    return 0;
}
INCLUDE_ASM("asm/funcs", func_80081E1C);
INCLUDE_ASM("asm/funcs", func_80082000);
INCLUDE_ASM("asm/funcs", func_8008241C);
INCLUDE_ASM("asm/funcs", func_800826CC);
INCLUDE_ASM("asm/funcs", func_800827D0);
