#include "common.h"
#include "include_asm.h"

/* Forward declarations */
extern s32 func_80046798(void);
extern void func_80017F98(s32, s32, s32);
extern void *func_8004153C(void);
extern void func_800400F8(void *);

/* Externs for globals */
extern u8 D_800A6690;
extern s16 D_800F6656;
extern s16 D_800F6658;
extern u8 D_800A8FB0[];
extern s32 D_800A93B0;
extern s32 D_800A93B4;
extern s32 D_800A93B8;
extern s32 D_800A93BC;
extern s32 D_800A93C0;
extern s32 D_800A93C4;

/* --- Functions 0x8003F168 - 0x8004019C --- */

INCLUDE_ASM("asm/funcs", func_8003F168);
INCLUDE_ASM("asm/funcs", func_8003F1C8);

void *func_8003F1D4(void) {
    return &D_800A6690;
}

void func_8003F1E4(s32 a0) {
    if (a0) {
        D_800F6656 = 3;
        D_800F6658 = 2;
    } else {
        D_800F6656 = 0;
        D_800F6658 = 1;
    }
}

INCLUDE_ASM("asm/funcs", func_8003F218);
INCLUDE_ASM("asm/funcs", func_8003F268);
INCLUDE_ASM("asm/funcs", func_8003F274);
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        D_800A8FB0[y * 32 + x] |= 0x8;
    }
}
INCLUDE_ASM("asm/funcs", func_8003F420);

void func_8003F52C(s32 a0, s32 a1, s32 a2) {
    D_800A8FB0[a1 * 32 + a0] = a2 & 3;
}

u32 func_8003F54C(s32 a0, s32 a1) {
    return D_800A8FB0[a1 * 32 + a0];
}

INCLUDE_ASM("asm/funcs", func_8003F568);

void func_8003F5A8(s32 a0, s32 a1, s32 a2) {
    (&D_800A93B0)[a2] = a0;
    (&D_800A93BC)[a2] = a1;
}

void func_8003F5CC(void) {
    func_80017F98(D_800A93B0, D_800A93BC, 0);
    func_80017F98(D_800A93B4, D_800A93C0, 1);
    func_80017F98(D_800A93B8, D_800A93C4, 2);
}

INCLUDE_ASM("asm/funcs", func_8003F62C);
INCLUDE_ASM("asm/funcs", func_8003F6D8);
INCLUDE_ASM("asm/funcs", func_8003F7F4);
INCLUDE_ASM("asm/funcs", func_8003F824);
INCLUDE_ASM("asm/funcs", func_8003FA24);
INCLUDE_ASM("asm/funcs", func_8003FE40);
INCLUDE_ASM("asm/funcs", func_8003FECC);
s32 func_8003FFA8(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}
void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        v1[3] = 1;
    }
}
void func_8003FFE0(void) {
    s32 *v0 = (s32 *)func_8004153C();
    if (v0) {
        s16 *v1 = (s16 *)v0[9];
        if (v1) {
            v1[1] = 1;
        }
    }
}
void func_8004001C(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 1;
        a0[0xE + i * 0xD0] = 1;
    }
}
void func_80040068(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 0;
        a0[0xE + i * 0xD0] = 0;
    }
}
void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;
        }
    }
}
INCLUDE_ASM("asm/funcs", func_800400F8);

void func_8004016C(void) {
    void *v0 = func_8004153C();
    if (v0) {
        func_800400F8(v0);
    }
}

void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;
        *(s16 *)((s32)v1 + 6) = 1;
    }
}
