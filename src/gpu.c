#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "psx.h"

/* Forward declarations */
extern void bios_StopCard(void);
extern void func_8007A458(void);

/* Externs for globals */
extern s32 D_80015D58;
extern s32 D_80015D70;
extern s32 D_80015EA8;
extern u8 g_gpu_interlace;
extern s32 D_80015ED4;
extern s32 D_80015E90;
extern s32 D_80015DD8;
extern s32 D_80015DF4;
extern s32 D_80015E10;
extern s32 D_80015E1C;
extern s32 D_80015D80;
extern s32 D_80015D98;
extern s32 D_80015DA8;
extern s32 D_80015DC0;
extern s32 D_80015DCC;
extern s32 D_80015E5C;
extern s32 D_80015E7C;
extern s32 D_8009BE2C;
extern s32 D_8009BEF4[];
extern s32 D_8009BF08[];

/* --- Functions 0x8007A28C - 0x8007B234 --- */

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
#define NULL ((void *)0)

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;
u8 *func_8007A28C(u8 *dst, u8 *src, s32 n) {
    s32 cnt;
    u8 *de;
    u8 *se;
    u8 b;

    if ((u32)dst >= (u32)src) {
        if (n > 0) {
            n -= 1;
            de = dst + n;
            se = src + n;
            do {
                b = *se;
                se -= 1;
                cnt = n;
                n -= 1;
                *de = b;
                de -= 1;
            } while (cnt > 0);
        }
    } else {
        if (n > 0) {
            n -= 1;
            do {
                b = *src;
                src += 1;
                cnt = n;
                n -= 1;
                *dst = b;
                dst += 1;
            } while (cnt > 0);
        }
    }
    return dst;
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_A0_0xAB_wrapper\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xAB\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_A0_0xAC_wrapper\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xAC\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
void func_8007A318(s32 a0) {
    bios__new_card(a0);
    bios__card_write(a0, 0x3F, 0);
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios__card_write\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x4E\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios__new_card\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x50\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
void func_8007A370(s32 a0) {
    bios_ChangeClearPad(0);
    EnterCriticalSection();
    if (func_80078BF0() == 0) {
        a0 = 0;
    }
    bios_InitCard(a0);
    ExitCriticalSection();
}
void func_8007A3C8(void) {
    EnterCriticalSection();
    bios_StartCard();
    bios_ChangeClearPad(0);
    ExitCriticalSection();
}

void func_8007A400(void) {
    bios_StopCard();
    func_8007A458();
}

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_InitCard\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x4A\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_StartCard\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x4B\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_StopCard\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x4C\n"
    "    nop\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoreorder\n"
    ".set noreorder\n"
    ".set\tnoat\n"
    ".set noat\n"
    "glabel func_8007A458\n"
    "\tlui\t$at,%hi(D_800A3648)\n"
    "\tsw\t$ra,%lo(D_800A3648)($at)\n"
    "\tjal\tEnterCriticalSection\n"
    "\tnop\n"
    "\taddiu\t$t2,$zero,176\n"
    "\tjalr\t$t2\n"
    "\taddiu\t$t1,$zero,86\n"
    "\tlui\t$t2,%hi(D_8007A4C0)\n"
    "\tlui\t$t1,%hi(D_8007A4CC)\n"
    "\tlw\t$v0,24($v0)\n"
    "\taddiu\t$t2,$t2,%lo(D_8007A4C0)\n"
    "\taddiu\t$t1,$t1,%lo(D_8007A4CC)\n"
    ".L8007A488:\n"
    "\tlw\t$v1,0($t2)\n"
    "\taddiu\t$t2,$t2,4\n"
    "\taddiu\t$v0,$v0,4\n"
    "\tbne\t$t2,$t1,.L8007A488\n"
    "\tsw\t$v1,108($v0)\n"
    "\tjal\tfunc_80078FF0\n"
    "\tnop\n"
    "\tjal\tExitCriticalSection\n"
    "\tnop\n"
    "\tlui\t$ra,%hi(D_800A3648)\n"
    "\tlw\t$ra,%lo(D_800A3648)($ra)\n"
    "\tnop\n"
    "\tjr\t$ra\n"
    "\tnop\n"
    ".word 0\n"
    ".word 0\n"
    ".word 0\n"
    ".word 0\n"
    ".word 0\n"
    ".word 0\n"
    ".set\treorder\n"
    ".set reorder\n"
    ".set\tat\n"
    ".set at\n"
);
u16 gpu_LoadTexture(s32 a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {
    s16 buf[4];
    buf[0] = a3;
    buf[3] = clut;
    buf[1] = texpage;
    switch (mode) {
    case 0:
        buf[2] = width / 4;
        break;
    case 1:
        buf[2] = width / 2;
        break;
    case 2:
        buf[2] = width;
        break;
    }
    gpu_LoadImage((s32)buf, a0);
    return gpu_CalcTPage(mode, a2, a3, texpage) & 0xFFFF;
}

u16 gpu_LoadClut256(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    buf[0] = a1;
    buf[1] = a2;
    buf[2] = 0x100;
    buf[3] = 1;
    gpu_LoadImage((s32)buf, a0);
    return gpu_CalcClut(a1, a2);
}
u16 gpu_LoadClut16(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    buf[0] = a1;
    buf[1] = a2;
    buf[2] = 0x10;
    buf[3] = 1;
    gpu_LoadImage((s32)buf, a0);
    return gpu_CalcClut(a1, a2);
}
s16 *gpu_InitDrawEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    s32 ret;
    ret = sys_GetVideoMode();
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[6] = 0;
    a0[7] = 0;
    a0[8] = 0;
    a0[9] = 0;
    ((s8 *)a0)[0x19] = 0;
    ((s8 *)a0)[0x1A] = 0;
    ((s8 *)a0)[0x1B] = 0;
    ((s8 *)a0)[0x16] = 1;
    a0[3] = a4;
    if (ret) {
        ((s8 *)a0)[0x17] = (a4 < 0x121);
    } else {
        ((s8 *)a0)[0x17] = (a4 < 0x101);
    }
    a0[4] = a1;
    a0[5] = a2;
    a0[0xA] = 10;
    ((s8 *)a0)[0x18] = 0;
    return a0;
}

s16 *gpu_InitDispEnv(s16 *a0, s16 a1, s16 a2, s16 a3, s32 a4) {
    a0[0] = a1;
    a0[1] = a2;
    a0[2] = a3;
    a0[4] = 0;
    a0[5] = 0;
    a0[6] = 0;
    a0[7] = 0;
    ((s8 *)a0)[0x11] = 0;
    ((s8 *)a0)[0x10] = 0;
    ((s8 *)a0)[0x13] = 0;
    ((s8 *)a0)[0x12] = 0;
    a0[3] = a4;
    return a0;
}
u32 gpu_CalcTPage(s32 a0, s32 a1, s32 a2, s32 a3) {
    return ((a0 & 3) << 7) | ((a1 & 3) << 5) | ((a3 & 0x100) >> 4) | ((a2 & 0x3FF) >> 6) | ((a3 & 0x200) << 2);
}
u32 gpu_CalcClut(s32 a0, s32 a1) {
    return ((a1 << 6) | ((a0 >> 4) & 0x3F)) & 0xFFFF;
}
void gpu_DebugTPage(s32 a0) {
    u32 val = a0 & 0xFFFF;
    g_gpu_debug_func(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
}
void gpu_DebugClut(s32 a0) {
    g_gpu_debug_func(&D_80015D70, (a0 & 0x3F) << 4, (a0 & 0xFFFF) >> 6);
}
u32 ot_GetTag(u32 *a0) {
    return (*a0 & OT_ADDR_MASK) | OT_TAG_BASE;
}

u32 ot_IsEnd(u32 *a0) {
    return (*a0 & OT_ADDR_MASK) == OT_ADDR_MASK;
}
void ot_Link(u32 *a0, u32 *a1) {
    register u32 mask asm("a2") = OT_ADDR_MASK;
    register u32 tag_mask asm("a3") = OT_TAG_MASK;
    *a1 = (*a1 & tag_mask) | (*a0 & mask);
    *a0 = (*a0 & tag_mask) | ((u32)a1 & mask);
}
void ot_Insert(u32 *a0, u32 a1, u32 *a2) {
    register u32 mask asm("a3") = OT_ADDR_MASK;
    register u32 tag_mask asm("t0") = OT_TAG_MASK;
    *a2 = (*a2 & tag_mask) | (*a0 & mask);
    *a0 = (*a0 & tag_mask) | (a1 & mask);
}
void ot_SetAddr(u32 *a0, u32 a1) {
    *a0 = (*a0 & OT_TAG_MASK) | (a1 & OT_ADDR_MASK);
}
void ot_SetEnd(u32 *a0) {
    *a0 |= OT_ADDR_MASK;
}
void gpu_SetSemiTransp(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 2;
    } else {
        a0[7] &= ~2;
    }
}

void gpu_SetRawTexture(u8 *a0, s32 a1) {
    if (a1) {
        a0[7] |= 1;
    } else {
        a0[7] &= ~1;
    }
}

void initPolyF3(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x20;
}

void initPolyFT3(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x24;
}

void initPolyG3(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x30;
}

void initPolyGT3(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x34;
}

void initPolyF4(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x28;
}

void initPolyFT4(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x2C;
}

void initPolyG4(u8 *p) {
    p[3] = 0x8;
    p[7] = 0x38;
}

void initPolyGT4(u8 *p) {
    p[3] = 0xC;
    p[7] = 0x3C;
}

void initSprt8(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x74;
}

void initSprt16(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x7C;
}

void initSprt(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x64;
}

void initTile1(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x68;
}

void initTile8(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x70;
}

void initTile16(u8 *p) {
    p[3] = 0x2;
    p[7] = 0x78;
}

void initTile(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x60;
}

void initLineF2(u8 *p) {
    p[3] = 0x3;
    p[7] = 0x40;
}

void initLineG2(u8 *p) {
    p[3] = 0x4;
    p[7] = 0x50;
}

void initPolyF3_dither(u8 *p) {
    p[3] = 0x5;
    p[7] = 0x48;
    *(u32 *)(p + 0x14) = GPU_DITHER_PATTERN;
}

void initPolyG3_dither(u8 *p) {
    p[3] = 0x7;
    p[7] = 0x58;
    *(u32 *)(p + 0x1C) = GPU_DITHER_PATTERN;
}

void initPolyF4_dither(u8 *p) {
    p[3] = 0x6;
    p[7] = 0x4C;
    *(u32 *)(p + 0x18) = GPU_DITHER_PATTERN;
}

void initPolyG4_dither(u8 *p) {
    p[3] = 0x9;
    p[7] = 0x5C;
    *(u32 *)(p + 0x24) = GPU_DITHER_PATTERN;
}

void initDrawMode(u8 *a0, s32 a1, s32 a2, u32 a3) {
    register u32 cmd asm("v1");
    register u32 val asm("v0");
    a0[3] = 1;
    cmd = GP0_DRAW_MODE;
    if (a2) {
        cmd = (GP0_DRAW_MODE | GPU_DRAW_MODE_DITHER);
    }
    val = a3 & GPU_DRAW_MODE_MASK;
    if (a1) {
        val |= GPU_DRAW_MODE_TEXOFF;
    }
    *(u32 *)(a0 + 4) = cmd | val;
}
void initLoadImage(u32 *a0, s16 *a1, u32 a2, u32 a3) {
    s32 size = 5;
    if (a1[2] == 0) {
        size = 0;
    } else if (a1[3] == 0) {
        size = 0;
    }
    a0[1] = OT_TERMINATOR;
    a0[2] = OT_TAG_BASE;
    ((u8 *)a0)[3] = size;
    a0[3] = *(u32 *)a1;
    a0[4] = (a3 << 16) | (a2 & 0xFFFF);
    a0[5] = *(u32 *)&a1[2];
}
void initStoreImage(u32 *a0, s16 *a1) {
    u32 nwords;
    s32 size;
    u32 *end;
    nwords = (a1[2] * a1[3] + 1) / 2;
    size = nwords + 4;
    if (nwords >= 13) {
        size = 0;
    }
    ((u8 *)a0)[3] = size;
    a0[1] = GP0_COPY_RECT_C2V;
    a0[2] = *(u32 *)&a1[0];
    a0[3] = *(u32 *)&a1[2];
    end = a0 + size;
    *end = OT_TERMINATOR;
}
s32 gpu_CatPacket(u8 *a0, u32 *a1) {
    s32 size;
    size = a0[3] + ((u8 *)a1)[3] + 1;
    if (size >= 17) {
        return -1;
    }
    a0[3] = size;
    *a1 = 0;
    return 0;
}
void gpu_DebugDispEnv(s16 *a0) {
    u32 val;
    g_gpu_debug_func(&D_80015D80, a0[0], a0[1], a0[2], a0[3]);
    g_gpu_debug_func(&D_80015D98, a0[4], a0[5]);
    g_gpu_debug_func(&D_80015DA8, a0[6], a0[7], a0[8], a0[9]);
    g_gpu_debug_func(&D_80015DC0, ((u8 *)a0)[0x16]);
    g_gpu_debug_func(&D_80015DCC, ((u8 *)a0)[0x17]);
    val = ((u16 *)a0)[0xA];
    g_gpu_debug_func(&D_80015D58, (val >> 7) & 3, (val >> 5) & 3, (val << 6) & 0x7C0,
               ((val << 4) & 0x100) + ((val >> 2) & 0x200));
}
void gpu_DebugDrawEnv(s16 *a0) {
    g_gpu_debug_func(&D_80015DD8, a0[0], a0[1], a0[2], a0[3]);
    g_gpu_debug_func(&D_80015DF4, a0[4], a0[5], a0[6], a0[7]);
    g_gpu_debug_func(&D_80015E10, ((u8 *)a0)[0x10]);
    g_gpu_debug_func(&D_80015E1C, ((u8 *)a0)[0x11]);
}
typedef struct {
    u8 mode;
    u8 active;
    u8 level;
    u8 flag;
    s16 width;
    s16 height;
} GpuConfig;
u32 gpu_SetMode(s32 a0) {
    GpuConfig *s0;
    int new_var;
    u32 idx;
    switch (a0 & 7) {
    case 0:
    case 3:
        debug_printf(&D_80015E5C, &D_8009BE2C, &g_gpu_type);
        /* fallthrough */
    case 5:
        s0 = (GpuConfig *)&g_gpu_type;
        bb2_memset(s0, 0, 0x80);
        irq_DisableInterrupts();
        bios_GPU_cw((u32)g_gpu_dev_table & 0xFFFFFF);
        s0->mode = (idx = func_8007D9C4(a0));
        idx = (u8)idx;
        s0->active = 1;
        s0->width = D_8009BEF4[idx];
        new_var = -1;
        s0->height = D_8009BF08[idx];
        bb2_memset((u8 *)s0 + 0x10, new_var, 0x5C);
        bb2_memset((u8 *)s0 + 0x6C, -1, 0x14);
        return s0->mode;
    default:
        if (g_gpu_debug_level >= 2) {
            g_gpu_debug_func(&D_80015E7C, a0);
        }
        ((void (*)(s32))g_gpu_dev_table[0x34 / 4])(1);
        break;
    }
}
u32 gpu_SetDither(s32 a0) {
    u8 *p = &g_gpu_dither;
    u32 old = *p;
    u32 val;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015E90, a0);
    }
    *p = a0;
    val = ((u32 (*)(s32))g_gpu_dev_table[0x28 / 4])(8);
    if (*p) {
        val |= 0x8000080;
    } else {
        val |= 0x8000000;
    }
    ((void (*)(u32))g_gpu_dev_table[0x10 / 4])(val);
    if (g_gpu_type == 2) {
        u32 *tbl = g_gpu_dev_table;
        val = 0x20000504;
        if (g_gpu_dither) {
            val = 0x20000501;
        }
        ((void (*)(u32))tbl[0x10 / 4])(val);
    }
    return old;
}

u32 gpu_SetDebugLevel(s32 a0) {
    u8 *p = &g_gpu_debug_level;
    u32 old = *p;
    u32 val = a0 & 0xFF;
    *p = a0;
    if (val) {
        g_gpu_debug_func(&D_80015EA8, val, g_gpu_type, g_gpu_dither);
    }
    return old;
}
u32 gpu_SetInterlace(s32 a0) {
    u8 *p = &g_gpu_interlace;
    u32 old = *p;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015ED4, a0);
    }
    if (a0 != *p) {
        ((void (*)(s32))g_gpu_dev_table[0x34 / 4])(1);
        *p = a0;
        irq_AcknowledgeVblank(2, 0);
    }
    return old;
}

u32 gpu_GetType(void) {
    return g_gpu_type;
}

u32 gpu_GetDebugLevel(void) {
    return g_gpu_debug_level;
}
