#include "common.h"
#include "include_asm.h"

/* Forward declarations for called functions */
extern void func_8001945C(void);
extern void func_8007AE7C(s32);
extern void func_8007B2A0(s32);
extern void func_8007B4D0(void *, s32, s32, s32);
extern void func_8007B33C(s32);
extern void func_80046AE8(void);

/* Externs for globals */
extern u8 D_80106A73;
extern u8 D_80106A54;
extern u16 D_800A3710;
extern u32 D_80106A50;
extern u32 D_80106A5C;
extern u8 D_800A3716;
extern s32 D_800A38BC;
extern u32 D_800A30CC;
extern u8 D_800F6740[];
extern u8 D_800F7438;
extern u8 D_800F7450;
extern u32 D_800F5370;
extern u8 D_80010000;
extern u8 D_8001000C;

extern void func_80079208();
extern void func_800164F8(void);
extern s16 D_800973FC[];
extern s32 func_80083698(s32, s32, s32);
extern s32 func_800836C8(s32, s32, s32);
extern s32 func_8008387C(s32, u8 *, s32);
extern void func_80078A18(s32);
extern void func_800836B8(s32);


/* --- Non-decompiled functions (INCLUDE_ASM) --- */
INCLUDE_ASM("asm/funcs", func_800164AC);
INCLUDE_ASM("asm/funcs", func_800164F8);
s32 func_80016514(s32 a0, u8 *dest) {
    s32 fd;
    s32 total;
    s32 remaining;
    s32 chunk;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    total = func_800836C8(fd, 0, 2);
    remaining = total;
    func_800836C8(fd, 0, 0);
    if (total > 0) {
        do {
            chunk = 0x4000;
            if (remaining < 0x4001) {
                chunk = remaining;
            }
            if (func_8008387C(fd, dest, chunk) != chunk) {
                func_80078A18(fd);
                return -1;
            }
            remaining -= chunk;
            dest += chunk;
        } while (remaining > 0);
    }
    func_800836B8(fd);
    return total;
}
s32 func_800165F8(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 _pad[2];
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    func_800836C8(fd, sector << 11, 0);
    i = 0;
    if (count > 0) {
        do {
            if (func_8008387C(fd, dest, 0x800) != 0x800) {
                func_80078A18(fd);
                return -1;
            }
            i += 1;
            dest += 0x800;
        } while (i < count);
    }
    func_800836B8(fd);
    return count << 11;
}
s32 func_800166C4(s32 a0) {
    s32 tmp = (a0 << 12) / 360;
    s32 v1 = tmp / 2;
    s16 cos_val = D_800973FC[(v1 + 0x400) & 0xFFF];
    s16 sin_val = D_800973FC[v1 & 0xFFF];
    return (cos_val * 320) / sin_val;
}
void func_80016768(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 i;
    u8 *ptr;
    s32 offset;

    i = 0;
    ptr = (u8 *)&D_800F7438;
    offset = 0;

    for (; i < 2; i++) {
        *(vu8 *)((u8 *)&D_800F7450 + offset) = a0;
        *(vu8 *)(ptr + 0x19) = a1;
        *(vu8 *)(ptr + 0x1A) = a2;
        *(vu8 *)(ptr + 0x1B) = a3;
        ptr += 0x4090;
        offset += 0x4090;
    }
}

/* --- Decompiled functions --- */

u32 func_800167AC(void) {
    return D_80106A73 & 1;
}

u32 func_800167BC(void) {
    return (D_80106A73 >> 1) & 1;
}

u32 func_800167D4(void) {
    return (D_80106A73 >> 2) & 1;
}

INCLUDE_ASM("asm/funcs", func_800167EC);

void func_80016868(void) {
    func_8007AE7C(1);
}

void func_80016888(void) {
    func_8007B2A0(0);
    func_8007AE7C(1);
    func_8007B4D0(&D_800A30CC, 0, 0, 0);
    func_8007B33C(0);
}

void func_800168D0(void) {
    func_8007B2A0(1);
}

void func_800168F0(void) {
}

void func_800168F8(void) {
    func_80046AE8();
}

extern void func_8007B114(s32);
extern void func_8007E094(void);
extern void func_8007EFDC(s32, s32);
extern void func_8007EFFC(s32);
extern void func_8007A694(u8 *, s32, s32, s32, s32);
extern void func_8007A74C(u8 *, s32, s32, s32, s32);
void func_80016918(void) {
    u8 *base;

    func_8007AE7C(0);
    func_8007B114(0);
    func_8007B2A0(0);
    func_8007E094();
    func_8007EFDC(0x140, 0x78);
    func_8007EFFC(func_800166C4(0x2D));
    base = &D_800F7438;
    func_8007A694(base, 0, 0, 0x280, 0xF0);
    func_8007A694(base + 0x4090, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x5C, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x40EC, 0, 0, 0x280, 0xF0);
    func_80016768(1, 0, 0, 0);
    func_8007B4D0(&D_800A30CC, 0, 0, 0);
    func_8007B33C(0);
}
extern void func_80082AC0(void);
extern void func_80078C9C(u8 *, s32, u8 *, s32);
extern void func_80078D38(void);
extern void func_80078A58(s32);
extern void func_80035FE0(void);
extern void func_800375EC(void);
extern u8 D_800FF580;
extern u8 D_800A3768;
extern u8 D_800A36A8;
void func_80016A18(void) {
    u8 *base = &D_800FF580;
    func_80082AC0();
    func_80078C9C(base, 8, base + 0x24, 8);
    func_80078D38();
    func_80078A58(0);
    func_80016918();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FE0();
    func_800375EC();
    func_800168F8();
}
INCLUDE_ASM("asm/funcs", func_80016A8C);
void func_80016C3C(void) {
    func_80079208((s32)&D_80010000);
    while (1) {
        func_800164F8();
    }
}
void func_80016C74(void) {
    D_800A3716 = 0;
}
extern s32 func_80060CB8(u32, u32);
void func_80016C80(void) {
    s32 size;

    if (D_800A3716 != 0) {
        return;
    }
    size = func_80060CB8(0x801D8800, 0x8010E800);
    func_80079208((s32)&D_8001000C, 0x8010E800, size);
    if (0xA000 < size) {
        func_80016C3C();
    }
    D_800A3716 = 1;
}
extern void func_8005B43C(void);
extern s32 func_8005B7C4(u32);
extern void func_80079120(u32, u32, s32);
extern void func_8005C4C0(u32, s32);
extern void func_8005C614(void);
extern u8 D_800A3906;
void func_80016CF8(void) {
    s32 size;

    func_8005B43C();
    size = func_8005B7C4(0x801D8800);
    if (size >= 0xD01) {
        func_80016C3C();
    }
    func_80079120(0x8010DB00, 0x801D8800, size);
    func_8005C4C0(0xFFF35300, 0);
    func_8005C614();
    D_800A3906 = 1;
}
extern u8 D_80010028;
extern u32 D_800A3770;
extern u32 D_800A3774;
extern u32 D_800A3798;
extern u8 D_800A36F9;
extern u8 D_800A3690;
extern u8 D_800A3744;
extern u8 D_800A3745;
extern u8 D_800A3746;
extern void func_80020D70(void);
extern void func_80046B44(void);
extern u8 D_800A36F1;
extern u16 D_800A38C6;
extern u8 D_800A36B0;
extern u8 D_800A3928;
extern void func_80019534(void);
extern void func_8003D2C4(void);
extern void func_8001C444(void);
void func_80016D78(void) {
    func_80079208((s32)&D_80010028, 0x8010DB00);
    func_800167EC();
    func_80020D70();
    D_800A3770 = 0x801D8800;
    D_800A3774 = 0x801EBC00;
    D_800A3798 = 0x13400;
    D_800A3716 = 0;
    D_800A3906 = 0;
    func_80016CF8();
    func_80019534();
    func_8003D2C4();
    func_8001C444();
    D_800A36F9 = 0;
    D_800A3690 = 0;
    D_800A3744 = 0;
    D_800A3745 = 0;
    D_800A3746 = 0;
    func_80046B44();
    D_800A36F1 = 2;
    D_800A38C6 = 0;
    D_800A36B0 = 0;
    D_800A3928 = 0;
}

void func_80016E40(void) {
    func_8007B33C(0);
}

INCLUDE_ASM("asm/funcs", func_80016E60);
void func_800171AC(s32 a0) {
    D_800A38BC = a0;
}
s32 func_800171B8(void) {
    s32 seed = D_800A38BC;
    s32 result = seed * 5497 + 0x7FA9;
    seed = (seed >> 16) ^ result;
    D_800A38BC = seed;
    return seed & 0x7FFF;
}
INCLUDE_ASM("asm/funcs", func_80017200);
INCLUDE_ASM("asm/funcs", func_800174F4);
void func_80017714(void) {
    s32 i;
    for (i = 0x16C; i >= 0; i -= 0x34) {
        *(s32 *)(D_800F6740 + i) = 0;
    }
}

s32 func_80017738(s32 a0, s32 a1) {
    return (a0 << 6) + (a1 << 4);
}

extern s32 func_8007F0BC(s32 *, s32 *);
extern s32 func_8007E43C(s32);
s32 func_80017748(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 2;
    in[1] = (a0[1] - a1[1]) >> 2;
    in[2] = (a0[2] - a1[2]) >> 2;
    func_8007F0BC(in, out);
    return func_8007E43C(out[0] + out[1] + out[2]) << 2;
}
s32 func_800177C8(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 4;
    in[1] = (a0[1] - a1[1]) >> 4;
    in[2] = (a0[2] - a1[2]) >> 4;
    func_8007F0BC(in, out);
    return func_8007E43C(out[0] + out[1] + out[2]) << 4;
}
INCLUDE_ASM("asm/funcs", func_80017848);
INCLUDE_ASM("asm/funcs", func_80017A44);
INCLUDE_ASM("asm/funcs", func_80017D84);
void func_80017E8C(s32 a0) {
    *(s32 *)(D_800F6740 + a0 * 52) = 0;
}
void func_80017EB4(s32 a0, s32 a1) {
    u8 *ptr = D_800F6740 + a0 * 52;
    s32 c = *(s32 *)(ptr + 0xC) + a1;
    *(s32 *)(ptr + 0xC) = c;
    *(s32 *)(ptr + 0x10) = c + (*(s16 *)(ptr + 4) << 6);
}
void func_80017EF4(s32 a0, s32 a1) {
    s32 *ptr = (s32 *)(D_800F6740 + a0 * 52);
    *ptr = *ptr + a1;
}
void func_80017F28(void) {
    vu32 *src = (vu32 *)0x1F800000;
    u32 *dst = (u32 *)&D_800F5370;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}
void func_80017F5C(void) {
    u32 *src = (u32 *)&D_800F5370;
    vu32 *dst = (vu32 *)0x1F800000;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void func_80017F90(void) {
}

void func_80017F98(void) {
}
