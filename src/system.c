#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

/* Forward declarations */
extern void func_800817A0(void);
extern void cpu_side_move_dir_4(void);
extern void marionation_Exec(void);
extern s32 func_80081718();
extern s32 func_80081D1C();
extern s32 func_80081E1C(void);
extern void irq_AcknowledgeVblank(s32, s32);
extern s32 saEft01Init(s32);

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
    cpu_side_move_dir_4();
}

void func_80080208(void) {
    marionation_Exec();
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
extern s32 tslTm2LoadImage(s32, void *, void *, s32);

INCLUDE_ASM("asm/funcs", func_80080258);
s32 func_80080390(s32 a0, s32 a1) {
    register s32 result asm("s6");
    s32 count;
    unsigned long long new_var2;
    s32 idx;
    s32 saved;
    int new_var;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = D_800A11B4;
    elem = &D_800A112C[idx];
    new_var = 3;
    result = 0;
    new_var2 = new_var;
    count = new_var2;

loop:
    D_800A11B4 = 0;

    if (idx != 1) {
        if (D_800A11C4 & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, 0, 0) != 0) {
                goto next;
            }
        }
    }
    D_800A11B4 = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, 0, 1) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    D_800A11B4 = saved;
    result = -1;
done:
    return result + 1;
}
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count;
    unsigned long long new_var2;
    s32 idx;
    s32 saved;
    int new_var;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = D_800A11B4;
    elem = &D_800A112C[idx];
    new_var = 3;
    new_var2 = new_var;
    count = new_var2;

loop:
    D_800A11B4 = 0;

    if (idx != 1) {
        if (D_800A11C4 & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    D_800A11B4 = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, a2, 0) == 0) {
        goto done;
    }
next:
    count--;
    if (count != (-1)) {
        goto loop;
    }
    D_800A11B4 = saved;
done:
    if (count == (-1)) {
        return 0;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, a2);
        r ^= 2;
        return (u32)r < 1;
    }
}
/* kengo:MED  |  tsl_pkt/tslPolyF4Init  |  81i */

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
    irq_AcknowledgeVblank(3, a0);
}

void func_80080684(s32 a0) {
    saEft01Init(a0);
}

void func_800806A4(s32 frames, u8 *out) {
    register u8 *v0 asm("v0") = out;
    s32 total;
    register s32 secs asm("a3");
    s32 mins;
    s32 rem75;
    s32 d1;
    s32 m1;
    s32 new_var2;
    s32 d2;
    s32 m2;
    s32 new_var;
    s32 d3;
    s32 m3;
    frames++;
    frames--;
    total = frames + 150;
    secs = total / 75;
    rem75 = total % 75;
    mins = secs / 60;
    secs = secs % 60;
    d1 = rem75 / 10;
    ;
    v0[2] = (u8)((d1 << 4) + (rem75 % 10));
    d2 = secs / 10;
    new_var2 = d2 << 4;
    new_var = new_var2;
    m2 = secs % 10;
    v0[1] = (u8)(new_var + m2);
    d3 = mins / 10;
    new_var = d3 << 4;
    m3 = mins % 10;
    v0[0] = (u8)(new_var + m3);
}

extern s32 D_800A112C[];
extern s32 tslTm2LoadImage(s32, void *, void *, s32);
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
INCLUDE_ASM("asm/funcs", cpu_side_move_dir_4);
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_4  |  160i  |  x4 size collision */
INCLUDE_ASM("asm/funcs", marionation_Exec);
/* kengo:HIGH  |  nm_mario/marionation_Exec  |  180i  |  +1 near-exact */
INCLUDE_ASM("asm/funcs", tslTm2LoadImage);
/* kengo:MED  |  tsl_tm2/tslTm2LoadImage  |  253i  |  -10 x2 size collision */
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
extern void irq_DisableInterrupts(void);
extern void irq_EnableInterrupts(s32, void *);
extern u8 D_80081F1C;
void func_80081974(void) {
    D_800A11B8 = 0;
    D_800A11B4 = 0;
    D_800A11C8 = 0;
    *(s32 *)&D_800A11C4 = 0;
    irq_DisableInterrupts();
    irq_EnableInterrupts(2, &D_80081F1C);
}
extern void D_800162A8;
extern void D_800162B4;
extern void D_800A1498;

s32 func_800819C4(void) {
    u8 v0;
    volatile u8 *p94;

    tslTm2LoadImage_2(&D_800162A8);
    func_80079208(&D_800162B4, &D_800A1498);

    D_800A11D5 = 0;
    D_800A11D4 = 0;
    D_800A11B8 = 0;
    D_800A11B4 = 0;
    D_800A11C8 = 0;
    *(s32 *)&D_800A11C4 = 0;

    irq_DisableInterrupts();
    irq_EnableInterrupts(2, &D_80081F1C);

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

    tslTm2LoadImage(1, 0, 0, 0);

    if (*(s32 *)&D_800A11C4 & 0x10) {
        tslTm2LoadImage(1, 0, 0, 0);
    }

    if (tslTm2LoadImage(0xA, 0, 0, 0) != 0) {
        return -1;
    }
    if (tslTm2LoadImage(0xC, 0, 0, 0) != 0) {
        return -1;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, 0);
        r ^= 2;
        return -((u32)(0 < (u32)r));
    }
}
extern s32 func_800828CC(s32);
extern void tslTm2LoadImage_2(void *);
extern void func_80079208(void *, void *, s32, s32, s32);
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
extern s32 D_800161B8;
extern s32 D_800161C8;
extern void D_800162C0;
INCLUDE_ASM("asm/funcs", saEft01Init);
/* kengo:MED  |  sa_eft/saEft01Init  |  91i */
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
s32 func_80081E1C(s32 a0, s32 a1) {
    volatile u8 *v1;
    u32 v0;
    *D_800A147C = 0;
    *D_800A1488 = 0x80;
    *D_800A14B0 = 0x21020843;
    *D_800A148C = 0x1325;
    *D_800A14B4 = *D_800A14B4 | 0x8000;
    *D_800A14B8 = a0;
    *D_800A14BC = a1 | 0x10000;
    v1 = D_800A147C;
    v0 = *v1 & 0x40;
    if (v0 == 0) {
        do {
            v0 = *v1 & 0x40;
        } while (v0 == 0);
    }
    *D_800A14C0 = 0x11400100;
    {
        volatile s32 tmp;
        tmp = *D_800A14C0;
    }
    return 0;
}

extern s32 D_800A1460;
void func_80081F0C(s32 a0) {
    D_800A1460 = a0;
}

extern volatile u8 D_800A1494;
extern volatile u8 D_800A1495;
extern s32 D_800A11B4;
extern s32 D_800A11B8;
extern void D_800F19A8;
extern void D_800F19A0;
extern s32 func_80080828(void);

__asm__(
    ".set noreorder
"
    ".set noat
"
    "glabel D_80081F1C
"
    ".set reorder
"
    ".set at
"
);

void func_80081F1C(void) {
    volatile u8 *s1 = &D_800A1495;
    volatile u8 *s3 = s1 - 1;
    u8 s2;
    s32 s0;
    s2 = *D_800A147C & 3;
    do {
        s0 = func_80080828();
        if (s0 == 0) break;
        if (s0 & 4) {
            if (D_800A11B8 != 0) {
                ((void (*)(u8, void *))D_800A11B8)(*s1, &D_800F19A8);
            }
        }
        if (!(s0 & 2)) continue;
        if (D_800A11B4 == 0) continue;
        ((void (*)(u8, void *))D_800A11B4)(*s3, &D_800F19A0);
    } while (1);
    *D_800A147C = s2;
}
INCLUDE_ASM("asm/funcs", tslTm2LoadImage_2);
/* kengo:MED  |  tsl_tm2/tslTm2LoadImage_2  |  253i  |  -10 x2 size collision */
INCLUDE_ASM("asm/funcs", saEft00Add);
/* kengo:HIGH  |  sa_eft/saEft00Add  |  169i  |  -3 near-exact */
INCLUDE_ASM("asm/funcs", func_800826CC);
extern s32 D_800A14EC;
extern s32 D_800A14E8;
extern s32 D_800A14E4;
extern s32 D_800A14D0;

s32 func_800827D0(s32 a0, s32 a1) {
    s32 *p = &D_800A14EC;
    s32 result;

    do {
        s32 tick = func_800828CC(-1);
        if (*p + 0x4B0 < tick) {
            result = -1;
        } else {
            if (p[-2] < 0) {
                goto do_seek;
            }
            tick = func_800828CC(-1);
            if (p[-1] + 0x3C < tick) {
do_seek:
                saEft00Add(1);
                result = p[-7];
            } else {
                result = p[-2];
            }
        }
        if (a0 != 0) break;
    } while (result > 0);
    ((void (*)(s32, s32))func_80080208)(1, a1);
    return result;
}
