/* 153 game functions: GTE and math helpers (gte_SetRotTransMatrix,
 * math_SquareRoot0, math_LerpMatrix3x3, ...) and the sound-bank loader
 * (snd_Init, snd_Quit, snd_LoadCommonVab, snd_VabOpen, ...). .text 0x8004A348
 * (ROM 0x3AB48). Start boundary: G8 (the Q89 cut). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include "gte.h"

extern s32 func_8005C2A8(Unk8005C2A8Pack *, s16, s32);
extern void func_80052C10(void);

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s32 D_800A33F0;
static Work_80053E9C *D_800A33F4;
static u16 D_800A33F8;
static s32 D_800A33FC; /* not named by any code or data: size from the gap */
static s16 D_800A3400;
static s32 g_vab_sticky_sbaddr;
static s32 D_800A3408;
static s32 D_800A340C;
/* not named by any code or data: size from the gap */
static s32 D_800A3410[2];
static s32 D_800A3418;

INCLUDE_ASM("asm/funcs", math_RotMatrixZYX);
INCLUDE_ASM("asm/funcs", func_8004A4E0);
INCLUDE_ASM("asm/funcs", func_8004A76C);
INCLUDE_ASM("asm/funcs", func_8004A808);

void func_8004A938(void) {}

INCLUDE_ASM("asm/funcs", func_8004A940);
INCLUDE_ASM("asm/funcs", func_8004BB68);
INCLUDE_ASM("asm/funcs", func_8004BCC0);
INCLUDE_ASM("asm/funcs", func_8004C1F4);
INCLUDE_ASM("asm/funcs", math_MidpointS16x3U8x2);
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

void func_8004E564(void) {}

void func_8004E56C(void) {}

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
/* Hand-written asm in the PsyQ source: trapping add/addi/sub ops GCC cannot
 * emit, a dead delay-slot init, three separate jr $ra exits; GTE LZCS/LZCR
 * leading-zero count. */
INCLUDE_ASM("asm/funcs", math_SquareRoot0);
/* GTE SQR of IR1-3, sum MAC1-3, then a frameless tail jump to math_SquareRoot0.
 * Hand-written asm: trapping add ops, mfc2 results in $t0-$t2, hand-scheduled
 * GTE nops. */
INCLUDE_ASM("asm/funcs", math_Length3D);
/* GTE SQR (squared vector length) leaf: mtc2 IR1-3, sqr, sum MAC1-3.
 * Hand-written asm: mfc2 results in $t0-$t2. */
INCLUDE_ASM("asm/funcs", gte_SumSquares3);
INCLUDE_ASM("asm/funcs", math_LerpSVector);
INCLUDE_ASM("asm/funcs", math_LerpMatrix3x3);
/* Loads the rotation matrix at *a0 (five lw, zero translation), reads the s16
 * 3x3 matrix at *a1 (five lw), runs three MVMVA cycles, one per column of *a1,
 * and stores the 3x3 product to *a2 with nine sh. Hand-scheduled asm: each
 * column's packing interleaved with the previous cycle's GTE latency, last
 * store in the jr delay slot. */
INCLUDE_ASM("asm/funcs", gte_MulMatrix0ClearTrans);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIR);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIRVec);
INCLUDE_ASM("asm/funcs", gte_SetRotTransMatrix);
/* Loads a packed 3x3 rotation matrix from *a0 into CR0-CR4 and zeroes the
 * translation CR5-CR7 (last ctc2 in the jr delay slot). Hand-written GTE asm.
 */
INCLUDE_ASM("asm/funcs", gte_SetRotMatrixClearTrans);
INCLUDE_ASM("asm/funcs", func_80052B7C);
/* Reads the far colour (cfc2 RFC/GFC/BFC) >> 4 into a0[0..2]. Hand-written asm:
 * results in $t0-$t2, a nop in the jr delay slot. */
INCLUDE_ASM("asm/funcs", gte_ReadFarColor);
INCLUDE_ASM("asm/funcs", func_80052C10);
INCLUDE_ASM("asm/funcs", func_80052C28);
INCLUDE_ASM("asm/funcs", func_80052C4C);
INCLUDE_ASM("asm/funcs", gte_ReadIR1IR2Sra2);
extern s32 func_80053694(s32 *, s16 *);

/* Walks the 32x32 grid of 2000-unit cells (origin -32000) along the XZ
 * segment from the start point (+0x8/+0x10) to the end point (+0x18/+0x20),
 * DDA style: +0x88/+0x8C are the current and end cells, +0x70/+0x74 the
 * major/minor extents (swapped when Z dominates), +0x7C the 4.12 slope and
 * +0x90 the major-axis steps left. Each cell crossed goes to the per-cell
 * test at +0x5C (func_80053E9C or func_80053754) until one reports a hit;
 * func_80053694 then reads the result back out. */
s32 func_80052D00(s32 *arg0, s16 *arg1) {
    s32 xdir;
    s32 zdir;
    s32 swapped;

    D_800A33F4->unk60 = D_800A33F4->unk8.vx + 32000;
    D_800A33F4->unk64 = D_800A33F4->unk8.vz + 32000;
    D_800A33F4->unk68 = D_800A33F4->unk18.vx + 32000;
    D_800A33F4->unk0 = 0x7FFFFFFF;
    D_800A33F4->unk6C = D_800A33F4->unk18.vz + 32000;
    D_800A33F4->unk70 = D_800A33F4->unk68 - D_800A33F4->unk60;
    D_800A33F4->unk88.x = D_800A33F4->unk60 / 2000;
    D_800A33F4->unk88.z = D_800A33F4->unk64 / 2000;
    D_800A33F4->unk8C.x = D_800A33F4->unk68 / 2000;
    D_800A33F4->unk8C.z = D_800A33F4->unk6C / 2000;
    D_800A33F4->unk74 = D_800A33F4->unk6C - D_800A33F4->unk64;
    if (D_800A33F4->unk88.x == D_800A33F4->unk8C.x &&
        D_800A33F4->unk88.z == D_800A33F4->unk8C.z) {
        if (D_800A33F4->unk8.vx == D_800A33F4->unk18.vx &&
            D_800A33F4->unk8.vy == D_800A33F4->unk18.vy &&
            D_800A33F4->unk8.vz == D_800A33F4->unk18.vz) {
            return 0;
        }
        D_800A33F4->unk5C(D_800A33F4->unk88.x, D_800A33F4->unk88.z);
    } else {
        D_800A33F4->unk80 = D_800A33F4->unk88.x * 2000 + 1000;
        D_800A33F4->unk84 = D_800A33F4->unk88.z * 2000 + 1000;
        D_800A33F4->unk60 -= D_800A33F4->unk80;
        D_800A33F4->unk64 -= D_800A33F4->unk84;
        D_800A33F4->unk68 -= D_800A33F4->unk80;
        D_800A33F4->unk6C -= D_800A33F4->unk84;
        if (D_800A33F4->unk70 < 0) {
            xdir = -1;
            D_800A33F4->unk70 = -D_800A33F4->unk70;
            D_800A33F4->unk60 = -D_800A33F4->unk60;
            D_800A33F4->unk68 = -D_800A33F4->unk68;
        } else {
            xdir = 1;
        }
        if (D_800A33F4->unk74 < 0) {
            zdir = -1;
            D_800A33F4->unk74 = -D_800A33F4->unk74;
            D_800A33F4->unk64 = -D_800A33F4->unk64;
            D_800A33F4->unk6C = -D_800A33F4->unk6C;
        } else {
            zdir = 1;
        }
        if (D_800A33F4->unk74 > D_800A33F4->unk70) {
            swapped = 1;
            D_800A33F4->unk80 = D_800A33F4->unk70;
            D_800A33F4->unk70 = D_800A33F4->unk74;
            D_800A33F4->unk74 = D_800A33F4->unk80;
            D_800A33F4->unk80 = D_800A33F4->unk60;
            D_800A33F4->unk60 = D_800A33F4->unk64;
            D_800A33F4->unk64 = D_800A33F4->unk80;
            D_800A33F4->unk80 = D_800A33F4->unk68;
            D_800A33F4->unk68 = D_800A33F4->unk6C;
            D_800A33F4->unk6C = D_800A33F4->unk80;
        } else {
            swapped = 0;
        }
        D_800A33F4->unk68 += 1000;
        D_800A33F4->unk60 += 1000;
        D_800A33F4->unk64 += 1000;
        D_800A33F4->unk90 =
            D_800A33F4->unk68 / 2000 - D_800A33F4->unk60 / 2000 + 1;
        D_800A33F4->unk7C = (D_800A33F4->unk74 << 12) / D_800A33F4->unk70;
        D_800A33F4->unk64 -= (D_800A33F4->unk60 * D_800A33F4->unk7C) >> 12;
        D_800A33F4->unk78 = (D_800A33F4->unk7C * 2000) >> 12;
        while (--D_800A33F4->unk90 != -1) {
            if ((D_800A33F4->unk80 = D_800A33F4->unk5C(
                     D_800A33F4->unk88.x, D_800A33F4->unk88.z)) != 0) {
                break;
            }
            D_800A33F4->unk64 %= 2000;
            D_800A33F4->unk64 += D_800A33F4->unk78;
            if (D_800A33F4->unk90 == 0) {
                break;
            }
            if (D_800A33F4->unk64 > 2000) {
                if (swapped) {
                    if (xdir < 0) {
                        D_800A33F4->unk88.x--;
                    } else {
                        D_800A33F4->unk88.x++;
                    }
                } else {
                    if (zdir < 0) {
                        D_800A33F4->unk88.z--;
                    } else {
                        D_800A33F4->unk88.z++;
                    }
                }
                if ((D_800A33F4->unk80 = D_800A33F4->unk5C(
                         D_800A33F4->unk88.x, D_800A33F4->unk88.z)) != 0) {
                    break;
                }
            }
            if (swapped) {
                if (zdir < 0) {
                    D_800A33F4->unk88.z--;
                } else {
                    D_800A33F4->unk88.z++;
                }
            } else {
                if (xdir < 0) {
                    D_800A33F4->unk88.x--;
                } else {
                    D_800A33F4->unk88.x++;
                }
            }
        }
        if (D_800A33F4->unk80 == 0 &&
            (D_800A33F4->unk88.x != D_800A33F4->unk8C.x ||
             D_800A33F4->unk88.z != D_800A33F4->unk8C.z)) {
            D_800A33F4->unk5C(D_800A33F4->unk8C.x, D_800A33F4->unk8C.z);
        }
    }
    return func_80053694(arg0, arg1);
}

extern s32 func_80052D00(s32 *, s16 *);
extern s32 func_80053754();
extern s32 func_80053E9C();
extern Work_80053E9C D_800EF9F8;

void func_80053304(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    if (gte_SumSquares3(D_800A33F4->unk18.vx - D_800A33F4->unk8.vx,
                        D_800A33F4->unk18.vy - D_800A33F4->unk8.vy,
                        D_800A33F4->unk18.vz - D_800A33F4->unk8.vz) <= 0x9C3F) {
        hi0 = D_800A33F4->unk18.vx;
        a = D_800A33F4->unk8.vx;
        hi1 = D_800A33F4->unk18.vy;
        b = D_800A33F4->unk8.vy;
        D_800A33F4->unk5C = func_80053754;
        hi2 = D_800A33F4->unk18.vz;
        D_800A33F4->unk8.vx = a - ((hi0 - a) << 1);
        D_800A33F4->unk8.vy = b - ((hi1 - b) << 1);
        c = D_800A33F4->unk8.vz;
        D_800A33F4->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        D_800A33F4->unk5C = func_80053E9C;
    }
    func_80052D00(arg2, arg3);
}

s32 func_8005344C(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3, s32 arg4) {
    s32 a, b, c;
    s32 hi0, hi1, hi2;
    D_800A33F4 = (Work_80053E9C *)arg4;
    D_800A33F4->unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    if (gte_SumSquares3(D_800A33F4->unk18.vx - D_800A33F4->unk8.vx,
                        D_800A33F4->unk18.vy - D_800A33F4->unk8.vy,
                        D_800A33F4->unk18.vz - D_800A33F4->unk8.vz) <= 0x9C3F) {
        hi0 = D_800A33F4->unk18.vx;
        a = D_800A33F4->unk8.vx;
        hi1 = D_800A33F4->unk18.vy;
        b = D_800A33F4->unk8.vy;
        D_800A33F4->unk5C = func_80053754;
        hi2 = D_800A33F4->unk18.vz;
        D_800A33F4->unk8.vx = a - ((hi0 - a) << 1);
        D_800A33F4->unk8.vy = b - ((hi1 - b) << 1);
        c = D_800A33F4->unk8.vz;
        D_800A33F4->unk8.vz = c - ((hi2 - c) << 1);
    } else {
        D_800A33F4->unk5C = func_80053E9C;
    }
    return func_80052D00(arg2, arg3);
}

s32 func_80053584(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3) {
    D_800A33F4 = &D_800EF9F8;
    D_800EF9F8.unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    D_800A33F4->unk5C = func_80053E9C;
    return func_80052D00(arg2, arg3);
}

s32 func_80053614(s32 *arg0, s32 *arg1, s32 *arg2, s16 *arg3, s32 arg4) {
    D_800A33F4 = (Work_80053E9C *)arg4;
    D_800A33F4->unk8 = *(VECTOR *)arg0;
    D_800A33F4->unk18 = *(VECTOR *)arg1;
    D_800A33F4->unk5C = func_80053E9C;
    return func_80052D00(arg2, arg3);
}

s32 func_80053694(s32 *arg0, s16 *arg1) {
    Work_80053E9C *p = D_800A33F4;
    s32 t;
    if (p->unk0 != 0x7FFFFFFF) {
        t = (p->unk48 * 0x7D0) - 0x7D00;
        arg0[0] = p->unk38 + t;
        arg0[1] = p->unk3C;
        t = (p->unk4A * 0x7D0) - 0x7D00;
        arg0[2] = p->unk40 + t;
        arg1[0] = p->unk28 >> 2;
        arg1[1] = p->unk2C >> 2;
        arg1[2] = p->unk30 >> 2;
        D_800A33F8 = p->unk4;
        return 1;
    }
    return 0;
}

extern void func_80052C4C(s32, s32, s32, s32);
extern void gte_ReadIR1IR2Sra2(s32 *, s32 *);

s32 func_80053754(s32 arg0, s32 arg1) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 hdr;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    D_800A33F4->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (D_800A33F4->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + D_800A33F4->unkE0);
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    D_800A33F4->unk4C = D_800A33F4->unk8.vx - x;
    D_800A33F4->unk4E = D_800A33F4->unk8.vy;
    D_800A33F4->unk50 = D_800A33F4->unk8.vz - z;
    D_800A33F4->unk54 = D_800A33F4->unk18.vx - x;
    D_800A33F4->unk56 = D_800A33F4->unk18.vy;
    D_800A33F4->unk58 = D_800A33F4->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        D_800A33F4->unkD0 = *data;
        data++;
        D_800A33F4->unkD4 = *data;
        data++;
        D_800A33F4->unkD8 = *data;
        data++;
        D_800A33F4->unkDC = (u16)*data;
        data++;
        D_800A33F4->unkDC = (*data << 16) | D_800A33F4->unkDC;
        data++;
        D_800A33F4->unkE4 =
            D_800A33F4->unkD0 * D_800A33F4->unk4C +
            D_800A33F4->unkD4 * D_800A33F4->unk4E +
            D_800A33F4->unkD8 * D_800A33F4->unk50 + D_800A33F4->unkDC;
        D_800A33F4->unkE8 =
            D_800A33F4->unkD0 * D_800A33F4->unk54 +
            D_800A33F4->unkD4 * D_800A33F4->unk56 +
            D_800A33F4->unkD8 * D_800A33F4->unk58 + D_800A33F4->unkDC;

        D_800A33F4->unkE4 =
            (D_800A33F4->unkE4 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE4 < 0 ? -D_800A33F4->unkE4 : D_800A33F4->unkE4) >>
             10);
        D_800A33F4->unkE8 =
            (D_800A33F4->unkE8 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE8 < 0 ? -D_800A33F4->unkE8 : D_800A33F4->unkE8) >>
             10);

        if (D_800A33F4->unkE4 >= 0 && D_800A33F4->unkE8 < 0) {
            D_800A33F4->unkE0 = D_800A33F4->unkE4 - D_800A33F4->unkE8;
            D_800A33F4->unkE4 *= 2;
            D_800A33F4->unkA8 = (D_800A33F4->unk54 - D_800A33F4->unk4C) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkAC = (D_800A33F4->unk56 - D_800A33F4->unk4E) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkB0 = (D_800A33F4->unk58 - D_800A33F4->unk50) *
                                D_800A33F4->unkE4 / D_800A33F4->unkE0;
            D_800A33F4->unkA8 =
                (D_800A33F4->unkA8 < 0 ? -1 : 1) *
                ((D_800A33F4->unkA8 >= 0 ? D_800A33F4->unkA8 + 1
                                         : -D_800A33F4->unkA8 + 1) >>
                 1);
            D_800A33F4->unkAC =
                (D_800A33F4->unkAC < 0 ? -1 : 1) *
                ((D_800A33F4->unkAC >= 0 ? D_800A33F4->unkAC + 1
                                         : -D_800A33F4->unkAC + 1) >>
                 1);
            D_800A33F4->unkB0 =
                (D_800A33F4->unkB0 < 0 ? -1 : 1) *
                ((D_800A33F4->unkB0 >= 0 ? D_800A33F4->unkB0 + 1
                                         : -D_800A33F4->unkB0 + 1) >>
                 1);
            D_800A33F4->unkA8 += D_800A33F4->unk4C;
            D_800A33F4->unkAC += D_800A33F4->unk4E;
            D_800A33F4->unkB0 += D_800A33F4->unk50;
            D_800A33F4->unkE0 = *data;
            data++;
            D_800A33F4->unkE4 = *data;
            data++;
            D_800A33F4->unkE8 = *data;
            data++;
            D_800A33F4->unk9C = *data;
            data++;
            D_800A33F4->unkA0 = *data;
            data++;
            D_800A33F4->unkA4 = *data;
            data++;
            D_800A33F4->unkC4 =
                ((D_800A33F4->unkA8 - D_800A33F4->unkE0) * D_800A33F4->unk9C +
                 (D_800A33F4->unkAC - D_800A33F4->unkE4) * D_800A33F4->unkA0 +
                 (D_800A33F4->unkB0 - D_800A33F4->unkE8) * D_800A33F4->unkA4) >>
                14;
            D_800A33F4->unk9C = *data;
            data++;
            D_800A33F4->unkA0 = *data;
            data++;
            D_800A33F4->unkA4 = *data;
            data++;
            D_800A33F4->unkC8 =
                ((D_800A33F4->unkA8 - D_800A33F4->unkE0) * D_800A33F4->unk9C +
                 (D_800A33F4->unkAC - D_800A33F4->unkE4) * D_800A33F4->unkA0 +
                 (D_800A33F4->unkB0 - D_800A33F4->unkE8) * D_800A33F4->unkA4) >>
                14;
            hdr = (u16)*data;
            data++;
            n = hdr;
            D_800A33F4->unkCC = n >> 8;
            n &= 0xFF;
            D_800A33F4->unkB4 = *data;
            data++;
            y = *data;
            data++;
            D_800A33F4->unkE0 = 1;
            D_800A33F4->unkB8 = y;
            while (--n != -1) {
                D_800A33F4->unkBC = *data;
                data++;
                D_800A33F4->unkC0 = *data;
                data++;
                if ((D_800A33F4->unkC4 - D_800A33F4->unkB4) *
                            (D_800A33F4->unkC0 - D_800A33F4->unkB8) -
                        (D_800A33F4->unkC8 - D_800A33F4->unkB8) *
                            (D_800A33F4->unkBC - D_800A33F4->unkB4) >
                    0) {
                    D_800A33F4->unkE0 = 0;
                    break;
                }
                D_800A33F4->unkB4 = D_800A33F4->unkBC;
                D_800A33F4->unkB8 = D_800A33F4->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (D_800A33F4->unkE0 != 0) {
                if ((D_800A33F4->unkE0 = gte_SumSquares3(
                         D_800A33F4->unkA8 - D_800A33F4->unk4C,
                         D_800A33F4->unkAC - D_800A33F4->unk4E,
                         D_800A33F4->unkB0 - D_800A33F4->unk50)) <
                    D_800A33F4->unk0) {
                    D_800A33F4->unk48 = arg0;
                    D_800A33F4->unk4A = arg1;
                    D_800A33F4->unk38 = D_800A33F4->unkA8;
                    D_800A33F4->unk3C = D_800A33F4->unkAC;
                    D_800A33F4->unk40 = D_800A33F4->unkB0;
                    D_800A33F4->unk28 = D_800A33F4->unkD0;
                    D_800A33F4->unk2C = D_800A33F4->unkD4;
                    D_800A33F4->unk30 = D_800A33F4->unkD8;
                    D_800A33F4->unk34 = D_800A33F4->unkDC;
                    D_800A33F4->unk0 = D_800A33F4->unkE0;
                    D_800A33F4->unk4 = D_800A33F4->unkCC;
                }
            }
        } else {
            data += 9;
            n = *data;
            data++;
            n &= 0xFF;
            data += (n + 1) * 2;
        }
    }
    return D_800A33F4->unk0 != 0x7FFFFFFF;
}

s32 func_80053E9C(s32 arg0, s32 arg1) {
    s32 n;
    s16 *data;
    s32 count;
    s32 x;
    s32 z;
    s32 y;
    s16 hdr;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }
    D_800A33F4->unkE0 = ((u16 *)D_800A33F0)[arg1 * 32 + arg0];
    if (D_800A33F4->unkE0 == 0xFFFF) {
        return 0;
    }
    data = (s16 *)(D_800A33F0 + D_800A33F4->unkE0);
    x = arg0 * 2000 - 32000;
    z = arg1 * 2000 - 32000;
    D_800A33F4->unk4C = D_800A33F4->unk8.vx - x;
    D_800A33F4->unk4E = D_800A33F4->unk8.vy;
    D_800A33F4->unk50 = D_800A33F4->unk8.vz - z;
    D_800A33F4->unk54 = D_800A33F4->unk18.vx - x;
    D_800A33F4->unk56 = D_800A33F4->unk18.vy;
    D_800A33F4->unk58 = D_800A33F4->unk18.vz - z;

    count = *data;
    data++;
    while (--count != -1) {
        D_800A33F4->unkD0 = *data;
        data++;
        D_800A33F4->unkD4 = *data;
        data++;
        D_800A33F4->unkD8 = *data;
        data++;
        D_800A33F4->unkDC = (u16)*data;
        data++;
        D_800A33F4->unkDC = (*data << 16) | D_800A33F4->unkDC;
        data++;
        D_800A33F4->unkE4 =
            D_800A33F4->unkD0 * D_800A33F4->unk4C +
            D_800A33F4->unkD4 * D_800A33F4->unk4E +
            D_800A33F4->unkD8 * D_800A33F4->unk50 + D_800A33F4->unkDC;
        D_800A33F4->unkE8 =
            D_800A33F4->unkD0 * D_800A33F4->unk54 +
            D_800A33F4->unkD4 * D_800A33F4->unk56 +
            D_800A33F4->unkD8 * D_800A33F4->unk58 + D_800A33F4->unkDC;

        D_800A33F4->unkE4 =
            (D_800A33F4->unkE4 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE4 < 0 ? -D_800A33F4->unkE4 : D_800A33F4->unkE4) >>
             14);
        D_800A33F4->unkE8 =
            (D_800A33F4->unkE8 < 0 ? -1 : 1) *
            ((D_800A33F4->unkE8 < 0 ? -D_800A33F4->unkE8 : D_800A33F4->unkE8) >>
             14);

        if (D_800A33F4->unkE4 >= 0 && D_800A33F4->unkE8 < 0) {
            D_800A33F4->unkE0 = D_800A33F4->unkE4 - D_800A33F4->unkE8;
            D_800A33F4->unkA8 = (D_800A33F4->unk54 - D_800A33F4->unk4C) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk4C;
            D_800A33F4->unkAC = (D_800A33F4->unk56 - D_800A33F4->unk4E) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk4E;
            D_800A33F4->unkB0 = (D_800A33F4->unk58 - D_800A33F4->unk50) *
                                    D_800A33F4->unkE4 / D_800A33F4->unkE0 +
                                D_800A33F4->unk50;
            func_80052C4C((s32)data, D_800A33F4->unkA8, D_800A33F4->unkAC,
                          D_800A33F4->unkB0);
            data += 9;
            hdr = (u16)*data;
            data++;
            n = hdr;
            D_800A33F4->unkCC = n >> 8;
            n &= 0xFF;
            D_800A33F4->unkB4 = *data;
            data++;
            y = *data;
            data++;
            D_800A33F4->unkE0 = 1;
            D_800A33F4->unkB8 = y;
            gte_ReadIR1IR2Sra2(&D_800A33F4->unkC4, &D_800A33F4->unkC8);
            while (--n != -1) {
                D_800A33F4->unkBC = *data;
                data++;
                D_800A33F4->unkC0 = *data;
                data++;
                if ((D_800A33F4->unkC4 - D_800A33F4->unkB4) *
                            (D_800A33F4->unkC0 - D_800A33F4->unkB8) -
                        (D_800A33F4->unkC8 - D_800A33F4->unkB8) *
                            (D_800A33F4->unkBC - D_800A33F4->unkB4) >
                    0) {
                    D_800A33F4->unkE0 = 0;
                    break;
                }
                D_800A33F4->unkB4 = D_800A33F4->unkBC;
                D_800A33F4->unkB8 = D_800A33F4->unkC0;
            }
            if (n > 0) {
                data += n * 2;
            }
            if (D_800A33F4->unkE0 != 0) {
                if ((D_800A33F4->unkE0 = gte_SumSquares3(
                         D_800A33F4->unkA8 - D_800A33F4->unk4C,
                         D_800A33F4->unkAC - D_800A33F4->unk4E,
                         D_800A33F4->unkB0 - D_800A33F4->unk50)) <
                    D_800A33F4->unk0) {
                    D_800A33F4->unk48 = arg0;
                    D_800A33F4->unk4A = arg1;
                    D_800A33F4->unk38 = D_800A33F4->unkA8;
                    D_800A33F4->unk3C = D_800A33F4->unkAC;
                    D_800A33F4->unk40 = D_800A33F4->unkB0;
                    D_800A33F4->unk28 = D_800A33F4->unkD0;
                    D_800A33F4->unk2C = D_800A33F4->unkD4;
                    D_800A33F4->unk30 = D_800A33F4->unkD8;
                    D_800A33F4->unk34 = D_800A33F4->unkDC;
                    D_800A33F4->unk0 = D_800A33F4->unkE0;
                    D_800A33F4->unk4 = D_800A33F4->unkCC;
                }
            }
        } else {
            data += 9;
            n = *data;
            data++;
            n &= 0xFF;
            data += (n + 1) * 2;
        }
    }
    return D_800A33F4->unk0 != 0x7FFFFFFF;
}

void func_80054410(s32 a0) { D_800A33F0 = a0; }

void func_8005441C(s32 a0) { D_800A33F0 += a0; }

s16 func_80054434(void) { return D_800A33F8; }

INCLUDE_ASM("asm/funcs", func_80054440);
INCLUDE_ASM("asm/funcs", func_800545F4);
extern const char D_80015840[];

s32 func_80054604(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6) {
    /* FAKE: second handle to the ctrl block (pointer alias): keeps &D_800EFAE8
     * in one base register ($s1) for the whole body; direct field access
     * rematerializes it: score 63 */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    /* FAKE: id (a0 + 0x131) computed at entry; at its uses the addiu moves
     * below the prologue stores (score 6). */
    s32 id = a0 + 0x131;
    s32 ret;
    s16 *t;
    Unk80045878Obj *v;
    s32 n;

    if (a6 != 0) {
        ret = func_80044FA0(id, a6);
        D_800EFAE8.unk2C = a6;
    } else {
        if (func_80045080(id) < 0) {
            func_80046914();
            printf(D_80015840);
        }
        D_800EFAE8.unk2C = (s32)func_800469C4(id);
        ret = 0;
    }
    s->unk4 = *(s32 *)(((Unk800469C4Hdr *)s->unk2C)->unk4 + s->unk2C);
    s->unk2 = *(u16 *)(((Unk800469C4Hdr *)s->unk2C)->unk8 + s->unk2C);
    s->unk0 = 0;
    t = func_80046F14();
    t += stage_GetId() * 24 + a1 * 6;
    s->unkC = *t++;
    s->unk10 = *t++;
    s->unk14 = *t++;
    s->unk1C = 0;
    s->unk20 = 0;
    s->unk44[0] = a2;
    s->unk44[1] = a3;
    s->unk48[0] = a4;
    s->unk48[1] = a5;
    s->unk1E = (((s->unk4 >> 8) & 0x7F) << 14) / 360;
    if (s->unk4 >= 0) {
        s->unk44[0] = -1;
    }
    if (!(s->unk4 & 0x40000000)) {
        s->unk44[1] = -1;
    }
    v = func_8004153C(0);
    if (v != 0) {
        func_8003FFC4(v);
    }
    v = func_8004153C(1);
    if (v != 0) {
        func_8003FFC4(v);
    }
    s->unk8 = a1;
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    if (s->unk4 & 0x3F) {
        /* FAKE: n computed ahead of the a6 test; computed in each call the
         * subtraction moves into the call setup (score 5). */
        n = (s->unk4 & 0x3F) - 1;
        if (a6 != 0) {
            a6 += ret;
            game_StageCleanup(n, a6);
        } else {
            gpu_ResetGraphMode1();
            game_StageCleanup(n, (s32)D_800A3770);
        }
    }
    if (s->unk4 & 0x8000) {
        func_8004659C(-1);
    }
    return ret;
}

extern s16 InfoPosYTbl1[];

void func_80054884(
    s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    func_80054604(InfoPosYTbl1[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}

void func_800548DC(void) {
    DrawSync(0);
    func_8004659C(-1);
    func_80046A60();
}

extern s32 D_800A3250[2];

/* Per-frame stage handler on the ctrl block D_800EFAE8. On the first frame
 * (unk0 == 0) it resolves the loaded data's offset table (unk2C) into the
 * camera stream (unk30), the per-player motion streams (unk34[], dropped when
 * they start with the "NULL" tag D_800A3250) and the per-player nibble tables
 * (unk3C[]). Every frame it places the camera (rotated about y by unk1E, plus
 * the stage offset unkC..unk14), then decodes and places each player's motion
 * frame. The block holds those addresses as integers: typed as pointers, the
 * relocation sums in func_80054FDC and func_80054604 swap their addu operands.
 */
s32 func_8005490C(void) {
    /* FAKE: second handle to the ctrl block (pointer alias), as in
     * func_80054604: keeps &D_800EFAE8 in $s3 for the whole body; direct field
     * access: score 157 */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    VECTOR vec;
    /* The 0x84-byte motion frame func_800198D0 decodes, as s16 channels: the
     * root offset, heading and distance load signed (lh) and the frame goes to
     * func_80040D48's s16 * parameter. */
    s16 frame[0x42];
    s16 *v;
    s32 i;
    /* FAKE: one variable for the three player objects (func_8004153C(0) and (1)
     * on the first frame, func_8004153C(i) in the loop): shared, it takes $s0
     * for all three; one local per value: score 9 */
    Unk80045878Obj *player;
    /* FAKE: one variable for the camera-rotated z of the camera and of player
     * i's root offset: read in two blocks it is not tied to the subtraction and
     * takes $t0; one local per block: score 41 */
    s32 rot_z;

    if (s->unk0 < 0) {
        return 0;
    }
    if (s->unk0 == 0) {
        s32 j;

        s->unk30 = ((Unk800469C4Hdr *)s->unk2C)->unkC + s->unk2C;
        s->unk34[0] = ((Unk800469C4Hdr *)s->unk2C)->unk10[0] + s->unk2C;
        s->unk34[1] = ((Unk800469C4Hdr *)s->unk2C)->unk10[1] + s->unk2C;
        func_8003D774(s->unk30, 0);
        for (j = 0; j < 2; j++) {
            if (*(s32 *)s->unk34[j] == D_800A3250[0]) {
                s->unk34[j] = 0;
            }
            if (s->unk34[j] != 0) {
                func_8001979C(j, (u32 *)s->unk34[j]);
            }
        }
        s->unk3C[1] = 0;
        s->unk3C[0] = 0;
        if (s->unk4 & 0x80) {
            s->unk3C[0] = ((Unk800469C4Hdr *)s->unk2C)->unk18[0] + s->unk2C;
        }
        if (s->unk4 & 0x40) {
            s->unk3C[1] = ((Unk800469C4Hdr *)s->unk2C)->unk18[1] + s->unk2C;
        }
        player = func_8004153C(0);
        if (player != 0) {
            func_8003FFC4(player);
        }
        player = func_8004153C(1);
        if (player != 0) {
            func_8003FFC4(player);
        }
    }
    v = func_8003D7B4(0);
    vec.vx = -v[0];
    vec.vy = v[1];
    vec.vz = -v[2];
    {
        s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
        s32 sn = Judge[s->unk1E & 0xFFF];
        rot_z = (vec.vz * c - vec.vx * sn) >> 12;
        vec.vx = (vec.vz * sn + vec.vx * c) >> 12;
        vec.vz = rot_z;
    }
    D_80101DF0.work.t[0] = vec.vx + s->unkC;
    D_80101DF0.work.t[1] = vec.vy + s->unk10;
    D_80101DF0.work.t[2] = vec.vz + s->unk14;
    D_80101DF0.xf.rot.vx = v[3];
    D_80101DF0.xf.rot.vy = v[4] + 0x800;
    D_80101DF0.xf.rot.vz = v[5];
    if (s->unk1E != 0) {
        MATRIX m;
        SVECTOR rot;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = s->unk1E;
        g_anim_func_table[0](&rot, &m);
        g_anim_func_table[0](&D_80101DF0.xf.rot, &D_80101DF0.work);
        MulMatrix2(&m, &D_80101DF0.work);
        math_MatrixToAnglesYXZ(&D_80101DF0.work, &D_80101DF0.xf.rot);
        math_TransposeMatrixInPlace(&D_80101DF0.work);
        D_80101DF0.xf.mat = D_80101DF0.work;
    } else {
        func_800418D0(&D_80101DF0);
    }
    s->unk24[0] = -D_80101DF0.xf.rot.vx;
    s->unk24[1] = -D_80101DF0.xf.rot.vy;
    s->unk24[2] = -D_80101DF0.xf.rot.vz;
    func_800420D0();
    func_8004211C();
    func_80047210();
    func_8003F274();
    func_8004A1FC(&D_800F62E0[0]);
    func_8004A1FC(&D_800F62E0[1]);
    func_8004A1FC(&D_800F62E0[4]);
    for (i = 0; i < 2; i++) {
        if (s->unk34[i] != 0) {
            s32 ang;
            player = func_8004153C(i);
            func_800198D0(i, s->unk0, (MotionFrame *)frame, (u16 *)0x1F800000);
            /* FAKE: vec.vy set twice; one expression drops the target's spill
             * of frame[0] (sw v1,28(sp)) (score 2) */
            vec.vy = frame[0];
            vec.vy = (vec.vy * player->unk_12) >> 12;
            ang = frame[1];
            vec.vx = (Judge[ang & 0xFFF] * frame[2]) >> 12;
            vec.vz = (Judge[(ang + 0x400) & 0xFFF] * frame[2]) >> 12;
            vec.vy = -vec.vy;
            vec.vz = -vec.vz;
            {
                s32 c = Judge[(s->unk1E + 0x400) & 0xFFF];
                s32 sn = Judge[s->unk1E & 0xFFF];
                rot_z = (vec.vz * c - vec.vx * sn) >> 12;
                vec.vx = (vec.vz * sn + vec.vx * c) >> 12;
                vec.vz = rot_z;
            }
            vec.vy += s->unk10;
            vec.vx += s->unkC;
            vec.vz += s->unk14;
            func_80040D48(i, 0, &vec.vx, &s->unk1C, frame, s->unk10);
            if (s->unk44[i] >= 0) {
                func_80049718(s->unk44[i], (i * 2) | 0x8000, 0, 0);
                func_80049A2C(s->unk44[i], i * 2, 0);
            }
            if (s->unk48[i] >= 0) {
                func_80049A2C(s->unk48[i], (i * 2) | 1, 1);
            }
            if (s->unk3C[i] != 0) {
                func_80040304(i, (((u32 *)s->unk3C[i])[s->unk0 / 8] >>
                                  ((s->unk0 % 8) * 4)) &
                                     0xF);
            }
        }
    }
    func_80046EA0(10000);
    s->unk0++;
    if (s->unk0 >= s->unk2) {
        s->unk0 = -1;
    }
    return 1;
}

extern s32 func_8005490C(void);

s32 func_80054F68(void) {
    s32 s0;
    g_draw_queue_cursor = g_draw_queue;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = g_gpu_ot_ptr;
    D_800A378C = g_gpu_ot_ptr + 4;
    s0 = func_8005490C();
    func_800444E0();
    return s0;
}

void func_80054FDC(s32 a0) {
    s32 *p = &D_800EFAE8.unk2C;
    *p = a0 + *p;
    D_800EFAE8.unk30 = a0 + D_800EFAE8.unk30;
    if (D_800EFAE8.unk34[0]) {
        D_800EFAE8.unk34[0] = a0 + D_800EFAE8.unk34[0];
    }
    if (D_800EFAE8.unk34[1]) {
        D_800EFAE8.unk34[1] = a0 + D_800EFAE8.unk34[1];
    }
    if (D_800EFAE8.unk3C[0]) {
        D_800EFAE8.unk3C[0] = a0 + D_800EFAE8.unk3C[0];
    }
    if (D_800EFAE8.unk3C[1]) {
        D_800EFAE8.unk3C[1] = a0 + D_800EFAE8.unk3C[1];
    }
}

s16 *func_8005507C(void) { return D_800EFAE8.unk24; }

s32 *func_8005508C(void) { return D_80101DF0.xf.mat.t; }

void func_8005509C(s32 arg0) {
    s32 i;
    Unk80101EC8Record *p = &D_80101EC8[arg0];
    i = 0;
    do {
        p->unk_414[i][1] = 0;
        p->unk_414[i][0] = 0;
    } while ((++i) < 8);
}

void func_800550E8(s32 arg0) {
    s32 i;
    Unk80101EC8Record *p = &D_80101EC8[arg0];
    i = 0;
    do {
        p->unk_414[i][1] = p->unk_414[i][1] >> 1;
    } while (++i < 8);
}

void func_80055138(s32 arg0, u16 *arg1, u16 *arg2) {
    Unk80101EC8Record *p = &D_80101EC8[arg0];
    CpuLevelEntry *src;
    u8 *pair;
    u8 base;
    /* idx counts two loops: the eight bytes cleared at 0x444, then the two
     * players (0 = this record, 1 = the opponent's) (Ruling 11,
     * ordinary-c-judge-decidable). */
    s32 idx;
    s32 sec;
    Unk80101EC8Record *rec;
    u16 *cursor;
    u16 *list;
    s32 chr;
    s32 bit;
    s32 lo, hi1, hi2;
    /* temp holds six values in turn, each read before the next write: case 2's
     * level D_800A37D2 / 5; case 2's practice level D_800A37D2 / 3 (0 once it
     * reaches 3); case 3's row in D_8009A9B4; a move entry's byte-assembled
     * character mask; the entry's stat bytes e[1] and e[2] (Ruling 11,
     * ordinary-c-judge-decidable). */
    s32 temp;
    u32 cat;
    s32 lo_val, hi1_val, hi2_val;
    Unk80101EC8Record *other;

    p->unk_443 = p->unk_0A;
    p->unk_438 = p->unk_08;
    switch (D_800A38DC) {
    case 1:
        p->unk_438 = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (p->unk_438 > 0xD00) {
            p->unk_438 = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            func_8005509C(p->index);
        }
        if (D_80099D88[p->unk_443].flags & 0x300) {
            src = &D_8009A8C8[p->unk_86][D_800A37A0 - 1];
            p->unk_424 = src->unk0;
            p->unk_3F6 = src->unk1;
        }
        break;
    case 2:
        if (D_800A389A) {
            temp = D_800A37D2 / 5;
            p->unk_438 = temp * 0x180 + 0x280;
            if (p->unk_438 > 0x1000) {
                p->unk_438 = 0x1000;
            }
            if (D_800A37D2 % 5 == 0) {
                func_8005509C(p->index);
            }
        } else {
            temp = D_800A37D2 / 3;
            if (temp >= 3) {
                D_800A37D2 = 0;
                temp = 0;
            }
            p->unk_443 = 0x19;
            p->unk_1C = (temp + 2) << 10;
            p->unk_438 = 0;
            p->unk_424 = 0;
            p->unk_3F6 = 0x3C - temp * 15;
        }
        break;
    case 3:
        p->unk_443 =
            cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1][0] + 0x1B;
        base = D_800A38E2 / 10;
        p->unk_438 = base * 16 + 0x80;
        if (D_80099D88[p->unk_443].flags & 0x3000) {
            p->unk_438 = base * 16 + 0x180;
        }
        if (D_80099D88[p->unk_443].flags & 0x4000) {
            p->unk_438 += 0x200;
        }
        if ((D_800A38E2 - 1) % 10 == 0) {
            func_8005509C(p->index);
        }
        temp = D_800A38E2 / 10 * 2;
        if (D_800A38E2 % 10 == 0) {
            temp--;
        }
        pair = D_8009A9B4[temp];
        p->unk_424 = pair[0];
        p->unk_3F6 = pair[1];
        break;
    }
    if (func_800167BC() && D_800A38DC != 3) {
        p->unk_438 = p->unk_438 * 11 >> 4;
    }
    if (!(D_80099D88[p->unk_443].flags & 0xFF00)) {
        p->unk_424 = 0x11 - (p->unk_438 >> 8);
    }
    p->unk_39A = 0x8000 / p->unk_1C;
    p->unk_3BD = 0x10 - (p->unk_438 >> 8);
    if (D_80099D88[p->unk_443].flags & 0x100) {
        D_80099D88[p->unk_443].unk3 = (rand() & 3) + 1;
    }
    for (idx = 0; idx < sizeof(p->unk_444); idx++) {
        p->unk_444[idx] = 0;
    }
    p->unk_3A4 = arg1;
    for (idx = 0; idx < 2; idx++) {
        if (idx) {
            rec = p->other;
            cursor = arg2;
            list = arg2;
            chr = rec->unk_0A;
        } else {
            rec = p;
            cursor = arg1;
            list = arg1;
            chr = p->unk_443;
        }
        rec->unk_40A = (rec->unk_1A - 0x1000) * 225 >> 11;
        for (sec = 0; sec < 3; sec++) {
            bit = 1 << chr;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (idx == 0) {
                p->unk_3A8[sec] = cursor;
            }
            while (*cursor != 0) {
                u8 *e = (u8 *)list + *cursor;
                if (e[4] == 0x40) {
                    temp = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(temp & bit)) {
                        goto next;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    temp = e[1];
                    if (temp < lo) {
                        lo = temp;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    temp = e[2];
                    if (hi1 < temp) {
                        hi1 = temp;
                    }
                    cat = e[0] & 7;
                    if (hi2 < temp && (e[3] & 0xF) * 4 < 0x10 &&
                        (cat < 2 || cat == 7)) {
                        hi2 = temp;
                    }
                }
            next:
                cursor++;
            }
            if (rec->unk_0E >= 6) {
                lo_val = 0;
                hi2_val = 0x7530;
                hi1_val = 0x7530;
            } else {
                /* FAKE: the shared base (rec's 0x40A halfword + 100) is staged
                 * through hi2_val, completed below; a separate base local is
                 * tied to the lh result (target: lh v0; addiu v1,v0,100)
                 * (staged-value-reused-variable); a separate base local: score
                 * 6 */
                hi2_val = rec->unk_40A + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
            }
            cursor++;
            rec->unk_3F8[sec] = lo_val;
            rec->unk_3FE[sec] = hi1_val;
            rec->unk_404[sec] = hi2_val;
        }
    }
    other = p->other;
    p->unk_40D = -1;
    p->unk_40C = -1;
    p->unk_428 = -1;
    p->unk_425 = 0;
    p->unk_426 = 0;
    p->unk_3B4 = 0;
    p->unk_3D0.released = 0;
    p->unk_3D0.pressed = 0;
    p->unk_3D0.held = 0;
    other->unk_440 = 0;
    p->unk_440 = 0;
    other->unk_441 = 0;
    p->unk_441 = 0;
    other->unk_43C = 0;
    other->unk_43A = 0;
    p->unk_43C = 0;
    p->unk_43A = 0;
    p->cpu_route.count = 0;
    p->unk_39D = 0;
    p->unk_3F5 = 0;
    p->unk_3F4 = 0;
    p->unk_3F3 = 0;
    p->unk_3F2 = 0;
    p->unk_3EE = 0;
    p->unk_3F0 = 0;
    p->unk_3E8 = 0;
    p->unk_39C = 0;
    p->unk_394 = 0;
    p->unk_398 = 0;
    p->unk_3C4 = 0;
    p->unk_3C2 = 0;
    p->unk_3C1 = 0;
    p->unk_3C0 = 0;
    p->unk_440 = 0;
    p->unk_430 = 0;
    p->unk_3D0.unheld = -1;
}

s32 func_80055948(Unk80101EC8Record *arg0) {
    u8 *p;
    u8 ctr;
    s32 t;
    s32 mask;
    s32 dx, dy, dz;

    ctr = arg0->unk_3B8;
    p = arg0->unk_3B4;
    if (ctr != 0) {
        if (arg0->unk_46 == 0) {
            arg0->unk_3B8 = ctr - 1;
        }
        goto ret_3c8;
    }
    if (arg0->unk_3BC != 0) {
        goto check_loop;
    }
    {
        u8 idx = arg0->unk_443;
        if (idx == 22)
            goto check_loop;
        if ((D_80099D88[idx].flags & 0xBF00) != 0)
            goto check_loop;
        {
            s32 limit;
            if (arg0->unk_430 & 0x200) {
                limit = (arg0->unk_43C < 0x801);
            } else {
                limit = (arg0->unk_43C < 0x401);
            }
            if (limit == 0)
                goto reset_ret_neg1;
        }
        if (arg0->unk_430 & 0x800)
            goto reset_ret_neg1;
        if ((u32)(arg0->unk_425 - 1) < 2U)
            goto reset_ret_neg1;
        if (arg0->unk_442 != 0)
            goto reset_ret_neg1;
        {
            Unk80101EC8Record *other = arg0->other;
            if (other->unk_6A == 0x2D)
                goto reset_ret_neg1;
            dx = other->unk_F4.x - arg0->unk_40E;
            dy = other->unk_F4.z - arg0->unk_410;
            dz = arg0->unk_412;
            if ((dz * dz) >= ((dx * dx) + (dy * dy)))
                goto loop;
        }
    }
    goto reset_ret_neg1;
check_loop:
    if (arg0->unk_3BC != 1)
        goto loop;
    if (arg0->unk_430 & 0x800) {
        goto loop;
    }
reset_ret_neg1:
    arg0->unk_3B4 = 0;
    return -1;
sentinel_reset:
    arg0->unk_3B4 = 0;
    goto ret_3c8;
loop:
    while (1) {
        t = *p;
        p += 1;
        if (t & 0x80) {
            if (t == 0x80)
                goto sentinel_reset;
            arg0->unk_3B4 = p;
            arg0->unk_3B8 = (t & 0x7F) - 1;
            goto ret_3c8;
        }
        mask = 1 << (t & 0xF);
        if (t & 0x10) {
            arg0->unk_3C8 &= ~mask;
        } else {
            arg0->unk_3C8 |= mask;
        }
    }
ret_3c8:
    return arg0->unk_3C8;
}

void func_80055B44(Unk80101EC8Record *a0, u8 *a1, s32 a2, s32 a3) {
    a0->unk_3B4 = a1;
    a0->unk_3BC = a2;
    a0->unk_3B8 = a3;
    a0->unk_3C8 = 0;
    a0->unk_3CC = -1;
}

/* Four pad-bit numbers; func_80055B60 copies the table whole (align 1: lwl/lwr)
   and indexes the copy by Unk80101EC8Record.unk_441. */
typedef struct {
    u8 bit[4];
} PadBitTable;

extern PadBitTable D_800A3258;
extern u8 D_8009A088[];
extern s32 func_80058580(Unk80101EC8Record *);
extern void func_80056CB8(Unk80101EC8Record *);
extern s32 func_80056FE8(Unk80101EC8Record *);

void func_80055B60(s32 arg0, PadState *arg1) {
    Unk80101EC8Record *rec;
    Unk80101EC8Record *me;
    Unk80101EC8Record *opp;
    PadState pad;
    PadBitTable bits;
    s32 lo, hi;
    /* work holds two values: D_800A387C - unk_43E, then its sign (work >> 31)
     * (Ruling 11, ordinary-c-judge-decidable). */
    s32 work;
    /* temp holds six values: the unk_148 distance, a slot count minus one
     * (clamped at 0), the slot increment (4 or 8, then plus the old count,
     * clamped at 0xFF), the 0x20/0x60 flag, the wrapped ratan2() angle
     * difference and the poll result of func_80055948/func_80058580 (Ruling 11;
     * per-branch constants: Q20). */
    s32 temp;
    /* temp2 holds three values: the unk7 * 25 >> 3 limit, the func_80056FE8()
     * result and the SquareRoot0() distance (Ruling 11). */
    s32 temp2;
    /* temp3 holds two values: the least slot count seen (starting at 0x100) and
     * the func_80056FE8() result plus 800 (Ruling 11). */
    s32 temp3;
    /* the counter of each of the four loops, reused loop to loop the way
       SOTN's AddToInventory reuses i for its two loops (Q51) */
    s32 i; /* SOTN: src/dra/5D5BC.c:173 @aa53500 */

    rec = &D_80101EC8[arg0];
    rec->unk_3CC = 0;
    if (rec->unk_3E8 & 1) {
        opp = rec->other;
        me = rec;
    } else {
        me = rec->other;
        opp = rec;
    }
    me->unk_441 = me->unk_58[2] & 0xF;
    me->unk_43A =
        (ratan2(opp->unk_F4.x - me->unk_F4.x, opp->unk_F4.z - me->unk_F4.z) -
         me->unk_1C8.vy) &
        0xFFF;
    if (me->unk_43A > 0x800) {
        me->unk_43A -= 0x1000;
    }
    me->unk_43C = me->unk_43A < 0 ? -me->unk_43A : me->unk_43A;
    if (me->unk_6A == 0x15) {
        me->unk_440 = me->unk_441;
    }

    rec->unk_430 =
        (rec->unk_430 & 0x40060) |
        ((rand() & 0xFFF) < (rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D) |
        ((D_80099D88[rec->unk_443].flags & 0xFF00)
             ? (rec->unk_3F6 < rec->unk_3F5) << 2
             : ((rand() & 0xFFF) < (rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D)
                   << 2) |
        (((rand() & 0xFFF) < rec->unk_438 || rec->unk_3E8 < (rec->unk_438 >> 3))
         << 3) |
        (((rand() & 0xFFF) < (rec->unk_438 >> 1) ||
          rec->unk_3E8 < (rec->unk_438 >> 4))
         << 4) |
        ((rec->other->unk_6A == 2 || rec->other->unk_6A == 0x1B ||
          rec->other->unk_6A == 0x28 || rec->other->unk_6A == 0x26 ||
          (rec->unk_6A == 0x11 && rec->unk_50->unk_08 != rec->unk_58[1] - 1))
         << 7) |
        ((rec->unk_6A == 0x13 || rec->unk_6A == 0x1B || rec->unk_6A == 0x30)
         << 8) |
        ((rec->other->unk_6A == 0x13 || rec->other->unk_6A == 0x1B ||
          rec->other->unk_6A == 0x30)
         << 9) |
        ((rec->unk_6A == 6 || rec->unk_6A == 4 || rec->unk_6A == 0x14) << 10) |
        ((rec->other->unk_6A == 6 || rec->other->unk_6A == 4 ||
          rec->other->unk_6A == 0x14)
         << 11) |
        (rec->unk_6A == 0x15 ? 0x1000 : 0) |
        (rec->other->unk_6A == 0x15 ? 0x2000 : 0) |
        (rec->unk_6A == 0x19 ? 0x4000 : 0) |
        (rec->other->unk_6A == 0x19 ? 0x8000 : 0) |
        (rec->unk_6A == 0x1A ? 0x10000 : 0);
    rec->unk_3E8++;

    if (rec->unk_6A != 2 && rec->unk_6A != 0x1B && rec->unk_6A != 0x28 &&
        rec->unk_6A != 0x26 && rec->unk_6A != 0x2C && rec->unk_6A != 3 &&
        rec->unk_6A != 7) {
        if (rec->unk_3F5 != 0xFF) {
            rec->unk_3F5++;
        }
    } else {
        rec->unk_3F5 = 0;
    }

    if (((D_8009A088[rec->other->unk_0E] >> rec->other->unk_440) & 1) &&
        rec->other->unk_43C < (rec->unk_438 >> 4) && rec->unk_0E < 6 &&
        (rec->unk_430 & 0x2000) && !(D_80099D88[rec->unk_443].flags & 0xBF00)) {
        rec->unk_430 |= 0x20000;
    } else {
        rec->unk_430 &= ~0x20000;
    }

    rec->unk_43E = rec->unk_3F8[rec->unk_86] +
                   (((rec->unk_404[rec->unk_86] - rec->unk_3F8[rec->unk_86]) *
                     D_80099D88[rec->unk_443].unk5) >>
                    8);
    temp2 = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;
    work = D_800A387C - rec->unk_43E;
    if (temp2 < (work >= 0 ? work : -work)) {
        work >>= 31;
        if (work != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work ? -1 : 1;
    } else {
        rec->unk_3F0 = 0;
    }

    temp = rec->other->unk_148 - rec->unk_148;
    if (temp < -1000) {
        rec->unk_442 = 1;
    } else if (temp > 1000) {
        rec->unk_442 = 2;
    } else {
        rec->unk_442 = 0;
    }
    if (rec->unk_430 & 0x15500) {
        func_80056CB8(rec);
    }
    if ((rec->unk_40 == 0 && (rec->unk_6A == 3 || rec->unk_6A == 0x2C)) ||
        (rec->other->unk_40 == 0 &&
         (rec->other->unk_6A == 0xD || rec->other->unk_6A == 0x2C))) {
        if (rec->unk_3F4 != 0xFF) {
            rec->unk_3F4++;
        }
    }

    if (rec->unk_430 & 0x80) {
        lo = rec->other->unk_A1[0] != 0xFF ? rec->other->unk_A1[0]
                                           : rec->other->unk_A1[1];
        hi = rec->other->unk_A3[0] != 0xFF ? rec->other->unk_A3[0]
                                           : rec->other->unk_A3[1];
    }
    if (!(rec->unk_430 & 0x80) ||
        (rec->unk_6A != 0x11
             ? (hi < rec->other->unk_40 || lo - rec->other->unk_40 >= 9)
             : rec->unk_50->unk_08 < rec->unk_40)) {
        if (rec->unk_428 != -1) {
            if (rec->unk_424 != 0 &&
                (func_800167BC() == 0 || D_800A38DC == 3)) {
                s32 slot;
                s32 found;

                temp3 = 0x100;
                i = 0;
                slot = -1;
                found = -1;

                for (; i < 8; i++) {
                    if (rec->unk_414[i][0] == rec->unk_428) {
                        found = i;
                    } else {
                        temp = rec->unk_414[i][1] - 1;
                        if (rec->unk_414[i][1] < temp3) {
                            slot = i;
                            temp3 = rec->unk_414[i][1];
                        }
                        if (temp < 0) {
                            temp = 0;
                        }
                        rec->unk_414[i][1] = temp;
                    }
                }
                temp = rec->unk_6A == 0x11 ? 8 : 4;
                if (found == -1) {
                    rec->unk_414[slot][0] = rec->unk_428;
                    rec->unk_414[slot][1] = temp;
                } else {
                    temp += rec->unk_414[found][1];
                    if (temp > 0xFF) {
                        temp = 0xFF;
                    }
                    rec->unk_414[found][1] = temp;
                }
            }
            rec->unk_428 = -1;
            rec->unk_426 = 0;
            rec->unk_430 &= ~0x60;
        }
    } else {
        if (rec->unk_428 != rec->other->unk_5C) {
            if (rec->unk_6A == 0x11) {
                rec->unk_428 = 0xFF;
            } else {
                rec->unk_428 =
                    (rec->other->unk_6C != 0xE && rec->other->unk_6C != 0x2C)
                        ? rec->other->unk_5C
                        : 0xFE;
                rec->unk_427 = lo;
                temp2 = func_80056FE8(rec);
                temp3 = temp2 + 800;
                if (rec->unk_442 != 0 ||
                    ((rec->unk_430 & 0x100) && rec->unk_43C > 0x400)) {
                    if (temp2 / 2 >= D_800A387C) {
                        rec->unk_426 = 1;
                    } else if (temp2 >= D_800A387C) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                } else {
                    if (temp2 >= D_800A387C) {
                        rec->unk_426 = 1;
                    } else if (temp3 >= D_800A387C) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                }
                rec->unk_42A = rec->other->unk_F4.x;
                rec->unk_42C = rec->other->unk_F4.z;
                rec->unk_42E = temp3;
            }
            for (i = 0; i < 8; i++) {
                if (rec->unk_414[i][0] == rec->unk_428 &&
                    rec->unk_414[i][1] != 0 &&
                    rec->unk_414[i][1] >= rec->unk_424) {
                    temp = 0x20;
                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {
                        temp = 0x60;
                    }
                    rec->unk_430 |= temp;
                    break;
                }
            }
        }
        if (rec->unk_430 & 0x20) {
            rec->unk_430 |= 0x18;
        }
    }

    if (rec->other->unk_6A == 0x12 && rec->other->unk_43C < 0x80 &&
        D_800A387C < 0x1194) {
        rec->unk_425 = 1;
    } else {
        rec->unk_425 = 0;
        for (i = 0; i < 12; i++) {
            Obj80106A78 *obj = &D_80106A78[i];

            temp2 = SquareRoot0(
                (rec->unk_F4.x - obj->pos.x) * (rec->unk_F4.x - obj->pos.x) +
                (rec->unk_F4.z - obj->pos.z) * (rec->unk_F4.z - obj->pos.z));
            if (obj->kind != -1 && obj->unk_04 != 0 &&
                obj->owner != rec->index) {
                temp = (ratan2(rec->unk_F4.x - obj->pos.x,
                               rec->unk_F4.z - obj->pos.z) -
                        ratan2(obj->pos.x - obj->prev_pos.x,
                               obj->pos.z - obj->prev_pos.z)) &
                       0xFFF;
                if (temp > 0x800) {
                    temp -= 0x1000;
                }
                if ((temp < 0 ? -temp : temp) < 0x80) {
                    if ((rec->unk_B8.vy - obj->pos.y >= 0)
                            ? (rec->unk_B8.vy - obj->pos.y < 2000)
                            : (obj->pos.y - rec->unk_B8.vy < 2000)) {
                        rec->unk_425 = temp2 < 3000 ? 2 : 1;
                    }
                }
            }
        }
    }

    if ((rec->unk_430 & 1) && !(D_80099D88[rec->unk_443].flags & 0xFF00) &&
        rec->unk_426 != 1 && rec->unk_426 != 2 && rec->unk_425 != 1 &&
        rec->unk_425 != 2 &&
        (((rec->unk_430 & 0x80) &&
          ((rec->other->unk_7C == 0 && hi < rec->other->unk_40) ||
           lo - rec->other->unk_40 >= 9)) ||
         rec->other->unk_6A == 0x10 || rec->other->unk_6A == 3 ||
         rec->other->unk_6A == 7 || rec->other->unk_6A == 0x2C ||
         rec->other->unk_6A == 0x24 ||
         (!(D_80099D88[rec->unk_443].flags & 0x20) &&
          (((rec->unk_430 & 0x80) && rec->unk_426 == 3) ||
           (rec->other->unk_6A == 0x2A && rec->other->unk_26C == 0) ||
           (rec->other->unk_6A == 0x12 &&
            (!((0x78 >> rec->other->unk_B1) & 1) ||
             rec->other->unk_26C == 0)) ||
           (rec->other->unk_6A == 0xB &&
            ((rec->other->unk_330 == 0 && rec->unk_425 != 1 &&
              rec->unk_425 != 2) ||
             rec->other->unk_26C == 0)))) ||
         ((D_80099D88[rec->unk_443].flags & 0x20) &&
          (rec->other->unk_6A == 0x25 || rec->other->unk_6A == 9 ||
           rec->other->unk_6A == 0x16 || rec->other->unk_6A == 0x17 ||
           rec->other->unk_6A == 0xA ||
           (rec->other->unk_6A == 0x22 && rec->unk_442 == 0) ||
           rec->other->unk_43C > 0x400)))) {
        rec->unk_430 |= 2;
    } else {
        rec->unk_430 &= ~2;
    }

    if (D_80099D88[rec->unk_443].flags & 0x8000) {
        rec->unk_430 &= ~0x78;
        if (rec->unk_6A != 0x25 && (g_pad_state.held & 0x100)) {
            rec->unk_430 |= 0x40000;
            rec->unk_3F2 = 0;
        }
    }
    if (rec->unk_0E >= 6) {
        rec->unk_430 |= 0x78;
    }

    i = 0;
    do {
        if (rec->unk_3B4 != 0) {
            temp = func_80055948(rec);
        } else {
            temp = func_80058580(rec);
        }
        i++;
    } while (temp == -1 && i < 4);
    if (temp != -1) {
        pad.held = temp;
    } else {
        pad.held = 0;
    }
    if (pad.held & 0x660) {
        bits = D_800A3258;
        if (rec->unk_6A == 0x19) {
            pad.held |= 1 << bits.bit[rec->unk_441];
        } else if (rec->unk_6A == 0x13) {
            pad.held |= 4;
        }
    }
    pad.held &= 0xFFFF;
    pad.pressed = pad.held & ~rec->unk_3D0.held;
    pad.unheld = ~pad.held & 0xFFFF;
    pad.released = ~pad.held & rec->unk_3D0.held;
    pad.type[arg0] = 4;
    rec->unk_3D0 = pad;
    *arg1 = rec->unk_3D0;
}

extern u8 D_8009A820[];
extern u8 D_8009A821[];

void func_80056CB8(Unk80101EC8Record *arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s16 work[4];
    s32 start;
    s32 i;

    start = (arg0->unk_3E8 & 3) * 2;
    for (i = start; i < start + 2; i++) {
        Unk80101EC8Record *obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 idx;

        /* FAKE: idx names the byte-table index for the first lookup only, which
         * steers loop strength reduction's giv choice; `[i * 2]`: score 53 */
        idx = i * 2;
        obj = arg0;
        flags = D_8009A821[idx] << 8;
        if ((flags & 0x1000) != 0) {
            obj = arg0->other;
        }

        if (arg0->unk_6A == 0x13 || arg0->unk_6A == 6) {
            flags += obj->unk_1C8.vy;
        } else {
            flags += ratan2(D_800F6608.unk_00.x - obj->unk_F4.x,
                            D_800F6608.unk_00.z - obj->unk_F4.z);
        }

        sin_p = &Judge[flags & 0xFFF];
        scale = D_8009A820[i * 2] << 8;
        x = obj->unk_B8.vx + ((scale * *sin_p) >> 12);
        cos_p = &Judge[(flags + 0x400) & 0xFFF];
        z = obj->unk_B8.vz + ((scale * *cos_p) >> 12);
        pt0[0] = obj->unk_B8.vx;
        pt0[1] = obj->unk_B8.vy - 0x320;
        pt0[2] = obj->unk_B8.vz;
        pt1[0] = x;
        pt1[1] = obj->unk_B8.vy - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, hit0, work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = obj->unk_B8.vy - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = obj->unk_B8.vy + 0x1004;
        pt1[2] = z;

        flags |= func_80053614(pt0, pt1, hit1, work, 0x1F8002B8) << 1;
        flags += 1;
        if (flags == 3 && hit1[1] - obj->unk_B8.vy < 5) {
            flags = 0;
        } else if (flags == 4) {
            s32 dx = hit0[0] - obj->unk_B8.vx;
            s32 dz = hit0[2] - obj->unk_B8.vz;
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = obj->unk_B8.vy;
                if ((y - hit1[1] >= 0 ? y - hit1[1] : hit1[1] - y) >= 0x3E9) {
                    flags = 5;
                }
            }
        }
        arg0->unk_444[i] = flags;
    }
}

#undef sp18
#undef sp1C
#undef sp20
#undef sp28
#undef sp2C
#undef sp30
#undef sp38
#undef sp3C
#undef sp40
#undef sp48
#undef sp4C
#undef sp50
#undef sp58
#undef sp5C
#undef sp60
#undef sp68
#undef sp70
#undef sp78
/* Three byte tables (8, 8 and 16 bytes). D_8009A838 is read signed (lb) here
 * and in func_80058580; the other two unsigned (lbu). */
extern u8 D_8009A830[];
extern s8 D_8009A838[];
extern u8 D_8009A840[];

/* func_80056FE8: per-record table lookup. FAKE family:
 * duplicated-statement-into-arms
 * (.claude/rules/duplicated-statement-into-arms.md, a SOTN-accepted shape): one
 * real statement, the arm's table term (D_8009A830 / D_8009A838 / D_8009A840)
 * added into `base`, is written in
 * each dispatch arm. */
s32 func_80056FE8(Unk80101EC8Record *arg0) {
    Unk80101EC8Record *a2 = arg0->other;
    s32 a3 = a2->unk_58[3];
    s32 base = a3 * 40;
    /* FAKE: `base += <arm value>` duplicated into all three dispatch arms
     * (cross-jumping re-merges them byte-neutrally); the extra references put
     * base ahead of the struct pointer in register allocation; one post-join
     * add: score 13 (duplicated-statement) */
    if (a2->unk_A3[0] != 0xFF) {
        if (arg0->unk_5E == 0) {
            /* FAKE: duplicated copy (see above; one post-join add scores 13) */
            base += D_8009A830[a2->unk_0E] * 2;
        } else {
            /* FAKE: duplicated copy (see above; one post-join add scores 13) */
            base += D_8009A838[a2->unk_0E] * 8;
        }
    } else {
        /* FAKE: duplicated copy (see above; one post-join add scores 13) */
        base += D_8009A840[a2->unk_14] * 2;
    }
    return base + arg0->other->unk_40A + 0x12C;
}

extern s32 D_8009AA50[];

s32 func_80057094(Unk80101EC8Record *arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 sp10;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_v0;

    temp_s0 = ratan2(D_800F6608.unk_00.x - arg0->unk_F4.x,
                     D_800F6608.unk_00.z - arg0->unk_F4.z);
    var_v0 = temp_s0 - ratan2(arg1 - arg0->unk_F4.x, arg2 - arg0->unk_F4.z);
    var_v0 -= 0x100;
    temp_v0 = (s32)var_v0 >> 9;
    temp_v1 = temp_v0 & 7;
    sp10 = temp_v1;
    if ((arg3 == 0) && !(temp_v0 & 1)) {
        if (arg0->unk_3E8 & 0x10) {
            sp10 = temp_v1 + 1;
            if (sp10 >= 8) {
                sp10 = 0;
            }
        } else {
            sp10 = temp_v1 - 1;
            if (sp10 < 0) {
                sp10 = 7;
            }
        }
    }
    var_v0 = D_8009AA50[sp10 & 7];
    if (arg3 == 1) {
        var_v0 |= 4;
    }
    if (func_800233AC(arg0, &sp10) != 0) {
        var_v0 |= 8;
    }
    return var_v0;
}

typedef struct {
    s32 x, y, z, w;
} Vec4_571C0;

s32 func_800571C0(Unk80101EC8Record *obj) {
    Vec4_571C0 probe;
    Vec4_571C0 top;
    Vec4_571C0 left;
    Vec4_571C0 right;
    s32 hit[4];
    s16 work[4];
    s32 ret;
    s8 nl;
    /* temp holds two values: the count of clear probe steps on the right-hand
     * side, then which side was chosen (0 right, 1 left; per-branch constants,
     * Q20) (Ruling 11, ordinary-c-judge-decidable). */
    s8 temp;
    u8 goL;
    u8 goR;
    s32 ang;
    s32 rad;
    s32 a;
    Unk80101EC8Record *p;
    s32 dx;
    s32 dz;
    s32 x;
    s32 z;

    temp = 0;
    nl = 0;
    goR = 1;
    goL = 1;
    ret = 0;
    left.x = obj->unk_B8.vx;
    left.y = obj->unk_B8.vy - 5;
    rad = D_800A387C + 800;
    left.z = obj->unk_B8.vz;
    right = left;
    for (ang = 0x200; ang <= 0x800; ang += 0x200) {
        if (goL) {
            p = obj->other;
            a = p->unk_1D8 + ang;
            goL = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = p->unk_B8.vx + (dx >> 12);
            z = p->unk_B8.vz + (dz >> 12);
            probe.x = x;
            probe.y = obj->unk_B8.vy - 5;
            probe.z = z;
            top.x = x;
            top.y = obj->unk_B8.vy + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, hit, work, 0x1F8002B8) != 0) {
                goL = func_80053614(&left.x, &probe.x, hit, work, 0x1F8002B8) ==
                      0;
            }
            if (goL) {
                left = probe;
                nl++;
            }
        }
        if (goR) {
            p = obj->other;
            a = p->unk_1D8 - ang;
            goR = 0;
            dx = rad * Judge[a & 0xFFF];
            dz = rad * Judge[(a + 0x400) & 0xFFF];
            x = p->unk_B8.vx + (dx >> 12);
            z = p->unk_B8.vz + (dz >> 12);
            probe.x = x;
            probe.y = obj->unk_B8.vy - 5;
            probe.z = z;
            top.x = x;
            top.y = obj->unk_B8.vy + 5;
            top.z = z;
            if (func_80053614(&probe.x, &top.x, hit, work, 0x1F8002B8) != 0) {
                goR = func_80053614(
                          &right.x, &probe.x, hit, work, 0x1F8002B8) == 0;
            }
            if (goR) {
                right = probe;
                temp++;
            }
        }
    }
    if (nl != 0 || temp != 0) {
        if (nl == temp) {
            if (rand() & 1) {
                nl = 0;
            } else {
                temp = 0;
            }
        }
        if (nl < temp) {
            nl = temp;
            temp = 0;
        } else {
            temp = 1;
        }
        ret = nl--;
        for (ang = 0x200; nl >= 0; nl--, ang += 0x200) {
            s32 base = obj->other->unk_1D8;
            if (temp != 0) {
                a = base + ang;
            } else {
                a = base - ang;
            }
            obj->cpu_route.node[nl].x =
                obj->other->unk_B8.vx + ((D_800A387C * Judge[a & 0xFFF]) >> 12);
            obj->cpu_route.node[nl].z =
                obj->other->unk_B8.vz +
                ((D_800A387C * Judge[(a + 0x400) & 0xFFF]) >> 12);
            obj->cpu_route.node[nl].kind = 2;
        }
        obj->unk_398 = 0;
        obj->unk_3A0 = obj->cpu_route.node[0].x;
        obj->unk_3A2 = obj->cpu_route.node[0].z;
        obj->unk_39E = obj->cpu_route.node[0].kind;
    }
    return ret;
}

s32 func_8005763C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                  s32 arg6, s32 arg7, s32 *arg8, s32 *arg9) {
    s32 slope1;
    s32 x1;
    s32 dx1;
    s32 dy1;
    s32 dx2;
    s32 dy2;
    s32 slope2;
    s32 intercept1;
    s32 intersection_x;
    s32 y;

    x1 = arg0;
    x1 >>= 3;
    arg2 >>= 3;
    arg1 >>= 3;
    arg3 >>= 3;
    dx1 = arg2 - x1;
    dy1 = arg3 - arg1;
    arg4 >>= 3;
    arg6 >>= 3;
    arg5 >>= 3;
    arg7 >>= 3;
    dx2 = arg6 - arg4;
    dy2 = arg7 - arg5;
    if ((arg4 == arg6) && (arg1 == arg3)) {
        *arg8 = arg6;
        *arg9 = arg3;
    } else if ((x1 == arg2) && (arg5 == arg7)) {
        *arg8 = arg2;
        *arg9 = arg7;
    } else if ((arg4 == arg6) && (x1 != arg2)) {
        *arg8 = arg6;
        *arg9 = arg1 + (dy1 * (arg6 - x1)) / dx1;
    } else if ((x1 == arg2) && (arg4 != arg6)) {
        *arg8 = x1;
        *arg9 = arg5 + (dy2 * (x1 - arg4)) / dx2;
    } else if ((arg1 == arg3) && (arg5 != arg7)) {
        *arg9 = arg3;
        *arg8 = arg4 + (dx2 * (arg3 - arg5)) / dy2;
    } else if ((arg5 == arg7) && (arg1 != arg3)) {
        *arg9 = arg5;
        *arg8 = x1 + (dx1 * (arg5 - arg1)) / dy1;
    } else {
        if (arg2 == x1) {
            return 0;
        }
        if (arg6 == arg4) {
            return 0;
        }
        slope1 = (dy1 << 7) / dx1;
        slope2 = (dy2 << 7) / dx2;
        if (slope1 == slope2) {
            return 0;
        }
        intercept1 = (((arg1 * arg2) - (arg3 * x1)) << 7) / dx1;
        intersection_x =
            (((((arg5 * arg6) - (arg7 * arg4)) << 7) / dx2) - intercept1) /
            (slope1 - slope2);
        *arg8 = intersection_x;
        *arg9 = ((intersection_x * slope1) + intercept1) >> 7;
    }
    if (!((((*arg8 - x1) >= -50) || ((*arg8 - arg2) >= -50)) &&
          (((x1 - *arg8) >= -50) || ((arg2 - *arg8) >= -50)) &&
          ((y = *arg9, ((arg1 - y) >= -50)) || ((arg3 - y) >= -50)) &&
          (((y - arg1) >= -50) || ((y - arg3) >= -50)) &&
          (((*arg8 - arg4) >= -50) || ((*arg8 - arg6) >= -50)) &&
          (((arg4 - *arg8) >= -50) || ((arg6 - *arg8) >= -50)) &&
          (((arg5 - y) >= -50) || ((arg7 - y) >= -50)) &&
          (((y - arg5) >= -50) || ((y - arg7) >= -50)))) {
        return 0;
    }
    *arg8 = *arg8 << 3;
    *arg9 = *arg9 << 3;
    return 1;
}

extern s32 func_8005763C(s32, s32, s32, s32, s32, s32, s32, s32, s32 *, s32 *);

s32 func_80057ACC(
    Unk80101EC8Record *arg0, NavPolySet *arg1, s32 arg2, s32 arg3) {
    s32 sp28;
    s32 sp2C;
    s32 best;
    s16 i;
    s16 j;
    s16 k;
    s16 n;
    NavPoly *poly;
    s32 dx;
    s32 dy;
    s32 d;

    best = 100000;
    for (i = 0; i < arg1->npolys; i++) {
        poly = &arg1->polys[i];
        n = poly->nvtx;
        if (poly->flags & 0x80) {
            n = poly->nvtx - 1;
        }
        for (j = 0; j < n; j++) {
            k = j + 1;
            if (!(k < poly->nvtx)) {
                k = 0;
            }
            if (func_8005763C(arg0->unk_F4.x, arg0->unk_F4.z, arg2, arg3,
                              poly->vtx[j][0], poly->vtx[j][1], poly->vtx[k][0],
                              poly->vtx[k][1], &sp28, &sp2C) != 0) {
                dx = sp28 - arg0->unk_F4.x;
                dy = sp2C - arg0->unk_F4.z;
                d = SquareRoot0(dx * dx + dy * dy);
                if (d < best) {
                    best = d;
                    arg0->cpu_route.poly = i;
                    arg0->cpu_route.vtx = j;
                }
            }
        }
    }
    return best;
}

/* Per-vertex neighbour-angle midpoint: computes the outward bisector direction
 * at vertex arg1 of polygon arg0 and writes the offset point into *arg2 /
 * *arg3. FAKE: arg0->vtx is spelled at each of its five uses rather than cached
 * in a local (F3, no-new-park-categories entry 15; ruling 6b): cse folds them
 * to the target's two loads (the ratan2 call stops the fold); one local gives
 * one load: score 30. */
void func_80057CC8(NavPoly *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 *p;
    s32 pi;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = arg0->vtx[arg1][0];
    cy = arg0->vtx[arg1][1];

    if ((s16)prev_idx < 0) {
        prev_idx = arg0->nvtx - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16)tmp >= (s32)arg0->nvtx) {
            next_idx = 0;
        }
    }

    pi = (s16)prev_idx;
    ang_prev =
        ratan2(arg0->vtx[pi][0] - (s16)cx, arg0->vtx[pi][1] - (s16)cy) & 0xFFF;
    /* FAKE: the next vertex's address spelled as the integer sum, index first;
     * every pointer spelling expands base first and local-alloc ties the sum to
     * the table load (arg0->vtx[(s16)next_idx]: score 4) */
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)arg0->vtx);
    ang_next = ratan2(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;

    if (ang_next < ang_prev) {
        /* FAKE: `base` and `half` name the antipode of ang_prev and half the
         * angular gap (named intermediates, no-new-park-categories entry 6):
         * block-local, they are allocated
         * before global RA; as one expression the scratch is allocated
         * globally: score 6 */
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0->margin * 40;
    *arg2 = cx + ((scale * (s32)Judge[ang_mid & 0xFFF]) >> 12);
    *arg3 = cy + ((scale * (s32)Judge[((s16)ang_mid + 0x400) & 0xFFF]) >> 12);
}

/* Route arg0 around the edges of its current polygon toward (goal_x, goal_z):
 * walk the vertex ring both ways from the vertex arg0 stands at, stepping to
 * the next corner while the segment to the goal is blocked by an edge, and
 * append the cheaper of the two corner chains (up to 8 corners) to arg0's
 * route. */
void func_80057E84(
    Unk80101EC8Record *arg0, NavPolySet *arg1, s32 goal_x, s32 goal_z) {
    CpuRoute path[2];
    s16 ofs0_x;
    s16 ofs0_z;
    s16 ofs1_x;
    s16 ofs1_z;
    s32 hit_x;
    s32 hit_z;
    s16 dn_x;
    s16 dn_z;
    s16 up_x;
    s16 up_z;
    u8 go_dn;
    s8 idx_dn;
    s8 idx_up;
    s16 iter;
    s16 nedges;
    s32 dist_dn;
    s32 dist_up;
    u8 go_up;
    u8 hit_dn;
    u8 hit_up;
    /* FAKE: one counter for the edge scan and the route copy, reused the way
     * SOTN's DebugCaptureScreen reuses its i for its file-search loop and its
     * row countdown (Q51, Q53); a separate copy counter: score 79. */
    s16 i; /* SOTN: src/dra/42398.c:75 @aa53500 */
    s16 next;
    s16 ax;
    s16 az;
    s16 bx;
    s16 bz;
    NavPoly *poly;
    /* route holds three values: the down route (&path[0]) in the down corner
     * block, the up route (&path[1]) in the up corner block, and the cheaper of
     * the two for the copy loop (Ruling 11, reused-local-necessity). */
    CpuRoute *route;

    go_dn = 1;
    go_up = 1;
    poly = &arg1->polys[arg0->cpu_route.poly];
    dn_x = up_x = arg0->unk_F4.x;
    dn_z = up_z = arg0->unk_F4.z;
    path[1].count = 0;
    path[0].count = 0;
    dist_up = 0;
    dist_dn = 0;
    idx_dn = arg0->cpu_route.vtx;
    idx_up = idx_dn + 1;
    if (!(idx_up < poly->nvtx)) {
        if (poly->flags & 0x80) {
            go_up = 0;
        } else {
            idx_up = 0;
        }
    }
    nedges = poly->nvtx;
    if (poly->flags & 0x80) {
        nedges--;
    }
    for (iter = 0; iter < nedges; iter++) {
        /* vtx holds four values, each the address of one vertex's x/z pair: the
         * edge's start (i) and end (next) in the edge scan, then the down
         * corner (idx_dn) and the up corner (idx_up) (Ruling 11,
         * reused-local-necessity). */
        s16 *vtx;
        /* node holds two values: the waypoint appended to the down route, then
         * the one appended to the up route (Ruling 11, reused-local-necessity).
         */
        CpuWaypoint *node;

        hit_up = 0;
        hit_dn = 0;
        for (i = 0; i < nedges; i++) {
            next = i + 1;
            if (!(next < poly->nvtx)) {
                next = 0;
            }
            vtx = poly->vtx[i];
            ax = vtx[0];
            az = vtx[1];
            func_80057CC8(poly, i, &ofs0_x, &ofs0_z);
            vtx = poly->vtx[next];
            bx = vtx[0];
            bz = vtx[1];
            func_80057CC8(poly, next, &ofs1_x, &ofs1_z);
            if (go_dn && !hit_dn) {
                if ((dn_x != ofs0_x || dn_z != ofs0_z) &&
                    (dn_x != ofs1_x || dn_z != ofs1_z)) {
                    if (func_8005763C(dn_x, dn_z, goal_x, goal_z, ax, az, bx,
                                      bz, &hit_x, &hit_z)) {
                        hit_dn = 1;
                    }
                }
            }
            if (go_up && !hit_up) {
                if ((up_x != ofs0_x || up_z != ofs0_z) &&
                    (up_x != ofs1_x || up_z != ofs1_z)) {
                    if (func_8005763C(up_x, up_z, goal_x, goal_z, ax, az, bx,
                                      bz, &hit_x, &hit_z)) {
                        hit_up = 1;
                    }
                }
            }
            if (hit_dn && hit_up) {
                break;
            }
        }
        if (go_dn) {
            if (hit_dn) {
                s32 c;

                vtx = poly->vtx[idx_dn];
                dist_dn += SquareRoot0((vtx[0] - dn_x) * (vtx[0] - dn_x) +
                                       (vtx[1] - dn_z) * (vtx[1] - dn_z));
                func_80057CC8(poly, idx_dn, &dn_x, &dn_z);
                route = &path[0];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    go_dn = 0;
                }
                node = &route->node[c];
                node->x = dn_x;
                node->z = dn_z;
                node->kind = poly->kind;
                if (--idx_dn < 0) {
                    if (poly->flags & 0x80) {
                        go_dn = 0;
                        dist_dn = 100000;
                    } else {
                        idx_dn = poly->nvtx - 1;
                    }
                }
            } else {
                dist_dn += SquareRoot0((goal_x - dn_x) * (goal_x - dn_x) +
                                       (goal_z - dn_z) * (goal_z - dn_z));
                go_dn = 0;
            }
        }
        if (go_up) {
            if (hit_up) {
                s32 c;

                vtx = poly->vtx[idx_up];
                dist_up += SquareRoot0((vtx[0] - up_x) * (vtx[0] - up_x) +
                                       (vtx[1] - up_z) * (vtx[1] - up_z));
                func_80057CC8(poly, idx_up, &up_x, &up_z);
                route = &path[1];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    go_up = 0;
                }
                node = &route->node[c];
                node->x = up_x;
                node->z = up_z;
                node->kind = poly->kind;
                if (!(++idx_up < poly->nvtx)) {
                    if (poly->flags & 0x80) {
                        go_up = 0;
                        dist_up = 100000;
                    } else {
                        idx_up = 0;
                    }
                }
            } else {
                dist_up += SquareRoot0((goal_x - up_x) * (goal_x - up_x) +
                                       (goal_z - up_z) * (goal_z - up_z));
                go_up = 0;
            }
        }
        if (!go_dn && !go_up) {
            break;
        }
    }
    if (dist_dn < dist_up) {
        route = &path[0];
    } else {
        route = &path[1];
    }
    for (i = route->count - 1; i >= 0; i--) {
        arg0->cpu_route.node[arg0->cpu_route.count] = route->node[i];
        arg0->cpu_route.count++;
    }
}

extern u8 D_8009A870[];
extern u8 D_8009A874[];
extern u8 D_8009A878[];
extern u8 D_8009A880[];
extern u8 D_8009A888[];
extern u8 D_8009A890[];
extern u8 D_8009A898[];
extern u8 D_8009A89C[];
extern u8 D_8009A8A4[];
extern u8 D_8009A8AC[];
extern u8 D_8009A8B4[];
extern u8 D_8009A8C0[];
extern u8 D_8009A850[8][4];
extern NavPolySet D_8009A658[];
extern u16 D_8009A928[][23];
extern u8 D_8009A9DC[][3];
extern s32 D_8009A9F0[][8];
extern u8 D_800A325C[4];
extern u8 D_800A3260[4];
extern void func_80057E84(Unk80101EC8Record *, NavPolySet *, s32, s32);

#define CPU_SQ(x) ((x) * (x))

s32 func_80058580(Unk80101EC8Record *p) {
    s32 wx;
    u8 *pscript;
    s32 wz;
    s8 bestflip;
    s16 pbesti;
    s32 phi;
    s32 lv;
    u8 *script1;
    u8 *script2;
    u8 *script3;
    u8 *script4;
    u8 mode;
    /* work1 holds nine values in turn, each read before the next write: the
     * unk_444[5] == 0 flag of the state-0x15 script choice; the stage distance
     * base (100000, or D_8009A838[stage] * 8) of the D_8009A850 scan;
     * unk_444[6] for the lim chain; unk_444[6] again for the waypoint script;
     * the x of waypoint 1; the unk_444[1] == 0 flag of the 0x394 action pick;
     * case 2's D_8009A9F0 pattern word, shifted in place; a script entry's low
     * distance bound (e[1] * 40, then adjusted); the state-0x15 script's near
     * bound (the opponent's unk_3F8 entry, or its unk_404 entry + 300)
     * (Ruling 11, reused-local-necessity). */
    s32 work1;
    /* work2 holds seven values in turn, each read before the next write: the
     * unk_444[1] == 0 flag of the state-0x15 script choice; a D_8009A850
     * entry's distance; the pace byte unk_444[0]; the z of waypoint 1; the
     * unk_444[5] == 0 flag of the 0x394 action pick; the best random pick score
     * so far (Q75 constant start + copy; compared as an s16, `sll; sra 16`,
     * Q82); case 2's nibble count, counted down (Ruling 11,
     * reused-local-necessity). */
    s32 work2;
    /* work3 holds thirteen values in turn, each read before the next write: the
     * script side bit (opponent unk_AF & 1, possibly inverted); unk_444[3] ==
     * 0; the unk_43A angle, wrapped to +-0x800; the state-0x11 threshold
     * (0x1000 - (stance sum << 8)), scaled by unk_438 >> 12; the forced-scan
     * flag of the D_8009A850 scan; the "longer than lim" flag; the bearing to
     * the next waypoint, wrapped; the 0x394 action pick's coin bit, stepped per
     * try; the 0x394 slot (unk_394, or a D_800A325C / D_800A3260 entry); case
     * 3's column in D_8009A928; the entry-type mask; a script entry's character
     * mask; the entry's accept flag. Read-before-write kept from the original
     * (Q74): the 0x394 slot switch below reads work3 with no write on the path
     * unk_39C == 1, opponent state neither 0x19 nor 0x1A (target 0x80059D18 ->
     * 0x80059D6C -> 0x80059DB0; $s3 read by the sltiu at 0x80059D70 and the sll
     * at 0x80059DB4), switching on whatever earlier work left there (that read
     * is excluded from the value grouping above) (Ruling 11,
     * reused-local-necessity). */
    s32 work3;
    /* work4 holds three values in turn, each read before the next write: the
     * D_8009A850 scan index; the waypoint walk index (a copy of wi taken in the
     * walk branch, Q34, counted down); the script-list entry index (Ruling 11,
     * reused-local-necessity). */
    s32 work4;
    s32 pick;
    s32 hi;
    s16 et;
    s32 va;
    s32 vd;
    s32 vb;
    s32 vn;
    s32 vc;
    s16 sc;
    NavPolySet *pois;
    s32 tx, tz;
    s32 r;
    s32 sel;
    u16 *list;
    u8 *e;
    u8 *ep;
    /* FAKE: second handle to the script start, see `q = ep;` (read through ep:
     * score 58) */
    u8 *q;
    u16 off;
    s16 pbest;
    u8 st2;
    s8 flip;
    s32 wtype;
    s32 cnt;
    s32 ok4;
    s32 far;
    s32 lim;
    s8 besti;
    s32 score;

    if (p->other->unk_6A == 4 || p->other->unk_6A == 0x14) {
        return 0;
    }
    if (p->unk_443 != 0x16 &&
        ((p->unk_430 & 0x15100) ||
         (p->unk_6A == 0xD && (p->unk_426 == 4 || p->unk_425 == 4)))) {
        script1 = 0;
        if ((p->unk_426 == 1 && p->other->unk_40 + 1 >= p->unk_427) ||
            p->unk_425 == 2) {
            if (p->unk_430 & 8) {
                u8 *tbl[2];
                s32 f;
                tbl[0] = D_8009A874;
                tbl[1] = D_8009A870;
                work3 = p->other->unk_AF & 1;
                f = p->unk_430;
                if (!(((f & 0x20) ||
                       ((f & 0x10) && p->unk_3F3 % ((p->unk_438 >> 8) + 2) !=
                                          (p->unk_438 >> 8) + 1)) &&
                      (!(D_80099D88[p->unk_443].flags & 0xFF00) ||
                       (f & 0x40))) ||
                    (func_800167BC() && D_800A38DC != 3)) {
                    work3 = !work3;
                }
                script1 = tbl[work3];
                mode = 3;
            }
            if (p->unk_6A == 0xD) {
                script1 = 0;
                p->unk_3CC = 0;
                p->unk_426 = 4;
                p->unk_425 = 4;
                p->unk_3F3++;
            }
        } else if (p->unk_6A == 0x15) {
            mode = 4;
            work3 = p->unk_444[3] == 0;
            work1 = p->unk_444[5] == 0;
            work2 = p->unk_444[1] == 0;
            if (p->unk_426 == 2) {
                if (p->unk_42E * p->unk_42E <
                    CPU_SQ(p->unk_42A - p->unk_F4.x) +
                        CPU_SQ(p->unk_42C - p->unk_F4.z)) {
                    p->unk_3CC = 0;
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                } else if (work3 && (p->unk_430 & 8) &&
                           (!(D_80099D88[p->unk_443].flags & 0x8F00) ||
                            ((D_80099D88[p->unk_443].flags & 0x300) &&
                             D_800A37A0 >= 6))) {
                    script1 = D_8009A890;
                }
            }
            if (p->unk_425 == 1 ||
                (p->unk_426 == 1 && p->other->unk_441 == 2 &&
                 p->unk_427 - p->other->unk_40 >= 6) ||
                (p->unk_426 == 2 && script1 == 0)) {
                if ((p->unk_430 & 8) &&
                    (!(D_80099D88[p->unk_443].flags & 0xBF00) ||
                     ((D_80099D88[p->unk_443].flags & 0x300) &&
                      D_800A37A0 >= 7))) {
                    if (p->other->unk_43A > 0) {
                        if (work1) {
                            script1 = D_8009A880;
                        } else if (work2) {
                            script1 = D_8009A878;
                        }
                    } else {
                        if (work2) {
                            script1 = D_8009A878;
                        } else if (work1) {
                            script1 = D_8009A880;
                        }
                    }
                    if (script1 == 0) {
                        p->unk_426 = 1;
                        p->unk_425 = 2;
                        return -1;
                    }
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                }
            }
        }
        if (script1 != 0) {
            func_80055B44(p, script1, mode, 0);
            return p->unk_3CC;
        }
    }

    if ((p->unk_430 & 0x400) && p->unk_441 != 2) {
        if (D_80099D88[p->unk_443].flags & 0x8000) {
            p->unk_3CC = 0x2000;
        } else if (p->unk_430 & 8) {
            work3 = p->unk_43A;
            if (p->unk_441 == 0) {
                work3 = (work3 + 0x800) & 0xFFF;
                if (work3 > 0x800) {
                    work3 -= 0x1000;
                }
            }
            if (!(D_80099D88[p->unk_443].flags & 0xF00)) {
                if (work3 < 0) {
                    if (p->unk_444[1] == 0 || p->unk_444[1] == 3) {
                        p->unk_3CC = 0x4000;
                    }
                } else {
                    if (p->unk_444[5] == 0 || p->unk_444[5] == 3) {
                        p->unk_3CC = 0x1000;
                    }
                }
            }
            if (!(p->unk_425 == 1 || p->unk_425 == 2) && p->unk_426 != 2 &&
                (p->unk_3CC == 0 || p->unk_442 != 0 || p->other->unk_6A == 2 ||
                 p->other->unk_6A == 0x29 || p->other->unk_6A == 0x13 ||
                 p->other->unk_6A == 6 ||
                 p->other->unk_404[p->other->unk_86] < D_800A387C)) {
                if (p->unk_43C < 0x400 && p->unk_444[3] == 0) {
                    p->unk_3CC = 0x8000;
                } else {
                    p->unk_3CC = 0x2000;
                }
            }
        }
    } else {
        u16 state;

        state = p->unk_6A;
        if (state == 0xF || state == 0x1C || state == 0x1D || state == 0x1E ||
            state == 0x1F || state == 0x20 || state == 0x21) {
            if (p->unk_6A == 0x1D &&
                (rand() & 0xFF) < D_80099D88[p->unk_443].unk4 &&
                p->unk_444[3] == 0) {
                p->unk_3CC = 0x8000;
            } else {
                vd = 0x80;
                if (p->unk_3E8 % (0x12 - (p->unk_438 >> 8)) == 0) {
                    vd = 0x20;
                }
                p->unk_3CC = vd;
                if (p->unk_444[3] != 0) {
                    if (p->unk_444[2] != 0) {
                        p->unk_3CC = vd | 0x1000;
                    } else if (p->unk_444[4] != 0) {
                        p->unk_3CC = vd | 0x4000;
                    }
                } else if (p->unk_444[7] == 0) {
                    p->unk_3CC = (p->unk_443 & 1) ? vd | 0x1000 : vd | 0x4000;
                }
            }
        } else if (state == 0x11 && p->index != D_800A38AE &&
                   p->unk_40 == p->unk_50->unk_08 - 1) {
            {
                s32 a, b, c;
                a = p->unk_26E;
                b = p->unk_270;
                c = p->unk_272;
                work3 =
                    0x1000 - (((p->unk_26C == 0 ? a + 4 + b : a + b) + c) << 8);
                work3 = (p->unk_438 * work3) >> 12;
                if ((rand() & 0xFFF) < work3) {
                    p->unk_3CC = 0x20;
                } else if (p->unk_430 & 0x20) {
                    p->unk_3CC = 0x20;
                }
            }
        } else {
            if ((p->unk_148 - p->unk_B8.vy >= 0
                     ? p->unk_148 - p->unk_B8.vy
                     : p->unk_B8.vy - p->unk_148) < 200 &&
                !(D_80099D88[p->unk_443].flags & 0x8C00) &&
                ((!func_800167BC() && D_800A38DC != 3) || D_800A38DC == 3)) {
                work3 = 0;
                if ((p->unk_426 == 1 || p->unk_425 == 2) && (p->unk_430 & 8)) {
                    work3 = 1;
                }
                far = 0;
                if ((rand() & 0xFFF) < (p->unk_438 >> 3)) {
                    far = p->unk_3E8 > 0x3C;
                }
                if (p->unk_0E >= 6) {
                    work1 = 100000;
                } else {
                    work1 = D_8009A838[p->unk_0E] * 8;
                }
                for (work4 = 0;
                     work4 < sizeof(D_8009A850) / sizeof(D_8009A850[0]);
                     work4++) {
                    if (!(D_8009A850[work4][3] & 1) ||
                        p->unk_40 >= p->unk_50->unk_08 - 2 || work3) {
                        work2 = D_8009A850[work4][2] * 16 + work1 + p->unk_40A;
                        if ((D_800A387C < work2 &&
                             (!(D_8009A850[work4][3] & 8) ||
                              p->unk_43C < 0x100) &&
                             (far || (D_8009A850[work4][3] & 4))) ||
                            work3) {
                            if (D_8009A850[work4][0] == p->unk_6A &&
                                ((st2 = D_8009A850[work4][1]) == 0xFF ||
                                 st2 == p->other->unk_6A)) {
                                if (D_8009A850[work4][3] & 2) {
                                    vb = 0x40;
                                    if (p->unk_3E8 & 1) {
                                        vb = 0x20;
                                    }
                                    p->unk_3CC = vb;
                                } else if (
                                    (D_80099D88[p->unk_443].flags & 0x20) &&
                                    !(p->unk_440 == 3 || p->unk_440 == 4)) {
                                    p->unk_3CC = (p->unk_3E8 & 1) * 8;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 0x4108) == 8) {
        u16 state;

        state = p->unk_6A;
        if (state == 3 || state == 0x2C || state == 7 ||
            (p->unk_426 == 1 || p->unk_426 == 2) ||
            (p->unk_425 == 1 || p->unk_425 == 2)) {
            p->unk_39D = 0;
            p->cpu_route.count = 0;
            p->unk_398 = 0;
            return 0;
        }
    }

    if (p->unk_430 & 0x5100) {
        pois = &D_8009A658[D_800A36A4];
        if (p->unk_39D == 0) {
            tx = p->other->unk_F4.x;
            tz = p->other->unk_F4.z;
            wtype = 1;
        } else {
            tx = p->unk_3A0;
            tz = p->unk_3A2;
            wtype = p->unk_39E;
        }
        /* !FAKE: the trailing `&& wtype == 1` repeats the first test (redundant
         * condition, no-new-park-categories entry 16): the target re-tests $s2
         * after the || chain; without it the chain is two instructions shorter
         */
        if (!(wtype == 1 &&
              (p->other->unk_6A == 0xA || p->unk_443 == 0xA ||
               (p->unk_0E >= 6 && p->unk_34A == 0)) &&
              wtype == 1)) {
            if (!(p->unk_3E8 & 7)) {
                p->unk_434 = func_80057ACC(p, pois, tx, tz);
            }
            if (wtype == 1) {
                work1 = p->unk_444[6];
                if (!(p->unk_430 & 0x800) || D_800A387C < 4000) {
                    if (p->unk_442 == 2 && !(work1 == 1 || work1 == 2)) {
                        lim = p->other->unk_3F8[p->other->unk_86];
                    } else if ((p->unk_442 == 1 || p->unk_442 == 2) ||
                               p->unk_442 == 3) {
                        if (!(D_80099D88[p->unk_443].flags & 0x4000)) {
                            lim = p->other->unk_3FE[p->other->unk_86];
                        } else {
                            lim = p->other->unk_404[p->other->unk_86];
                        }
                    } else if (p->unk_434 != 100000) {
                        if (p->unk_430 & 0x200) {
                            lim = p->other->unk_404[p->other->unk_86];
                        } else {
                            lim = p->unk_404[p->unk_86] +
                                  p->other->unk_404[p->other->unk_86];
                        }
                    } else {
                        lim = p->unk_3FE[p->unk_86] +
                              p->other->unk_3FE[p->other->unk_86];
                        if (p->unk_430 & 0x200) {
                            if (p->other->unk_43C > 0x600) {
                                lim = p->other->unk_3F8[p->other->unk_86];
                            } else if (p->other->unk_43C > 0x300) {
                                lim = p->other->unk_404[p->other->unk_86];
                            } else if (p->other->unk_43C > 0x100) {
                                lim = p->other->unk_3FE[p->other->unk_86];
                            }
                        }
                    }
                } else {
                    lim = 2000;
                }
            } else {
                lim = 2000;
            }
            if (wtype == 1) {
                if (p->cpu_route.count < 2 &&
                    (p->unk_434 != 100000 || D_800A387C >= lim)) {
                    goto record;
                }
            } else if (p->cpu_route.count == 0) {
                if (p->unk_434 != 100000 ||
                    CPU_SQ(p->unk_F4.x - tx) + CPU_SQ(p->unk_F4.z - tz) >
                        0x15F8F) {
                record:
                    p->cpu_route.node[0].x = tx;
                    p->cpu_route.node[0].z = tz;
                    p->cpu_route.node[0].kind = wtype;
                    p->cpu_route.count = 1;
                    if (p->unk_434 != 100000 && !(p->unk_3E8 & 7)) {
                        func_80057E84(p, pois, tx, tz);
                    }
                }
            }
            if (p->cpu_route.count != 0) {
                if (p->cpu_route.node[0].kind == 1
                        ? (p->unk_434 == 100000 && D_800A387C < lim)
                        : SquareRoot0(CPU_SQ(p->unk_F4.x - tx) +
                                      CPU_SQ(p->unk_F4.z - tz)) < 2000) {
                    p->cpu_route.count = 0;
                    p->unk_39D = 0;
                    goto after_nav;
                }
                {
                    s32 dist;
                    s32 wi;
                    work1 = p->unk_444[6];
                    wi = p->cpu_route.count - 1;
                    work2 = p->unk_444[0];
                    wx = p->cpu_route.node[wi].x;
                    wz = p->cpu_route.node[wi].z;
                    if (!(work1 == 1 || work1 == 2)) {
                        script2 = 0;
                        if (p->cpu_route.node[wi].kind == 1) {
                            if (work2 == 3 &&
                                D_800A387C <
                                    p->other->unk_404[p->other->unk_86]) {
                                goto pick2;
                            }
                            /* !FAKE: the inner test contradicts the outer one,
                             * so pick2's body runs only by the goto above; the
                             * target compares twice, and failing either compare
                             * still reaches the script2 call test (redundant
                             * condition, no-new-park-categories entry 16);
                             * without the work2 == 5 arm: score 20 */
                            if (work2 == 5 &&
                                p->other->unk_404[p->other->unk_86] <
                                    D_800A387C) {
                                if (D_800A387C <
                                    p->other->unk_404[p->other->unk_86]) {
                                pick2:
                                    if (p->unk_6A == 0x13) {
                                        script2 = D_8009A8A4;
                                    } else if (p->unk_440 != 4) {
                                        script2 = D_8009A89C;
                                    }
                                }
                                if (script2 != 0) {
                                    func_80055B44(p, script2, 4, 0);
                                }
                            }
                        }
                    }
                    if (p->unk_3CC != 0) {
                        return p->unk_3CC;
                    }
                    if (wi == 0) {
                        if (p->cpu_route.node[0].kind == 1) {
                            dist = SquareRoot0(CPU_SQ(p->unk_F4.x - tx) +
                                               CPU_SQ(p->unk_F4.z - tz));
                        } else {
                            dist = SquareRoot0(CPU_SQ(p->unk_F4.x - wx) +
                                               CPU_SQ(p->unk_F4.z - wz));
                        }
                    } else {
                        work4 = wi;
                        dist = SquareRoot0(CPU_SQ(wx - p->unk_F4.x) +
                                           CPU_SQ(wz - p->unk_F4.z));
                        while (work4 >= 2) {
                            dist += SquareRoot0(
                                CPU_SQ(p->cpu_route.node[work4].x -
                                       p->cpu_route.node[work4 - 1].x) +
                                CPU_SQ(p->cpu_route.node[work4].z -
                                       p->cpu_route.node[work4 - 1].z));
                            work4--;
                        }
                        {
                            work1 = p->cpu_route.node[1].x;
                            work2 = p->cpu_route.node[1].z;
                            if (p->cpu_route.node[0].kind == 1) {
                                dist += SquareRoot0(
                                    CPU_SQ(work1 - tx) + CPU_SQ(work2 - tz));
                            } else {
                                dist += SquareRoot0(
                                    CPU_SQ(work1 - p->cpu_route.node[0].x) +
                                    CPU_SQ(work2 - p->cpu_route.node[0].z));
                            }
                        }
                    }
                    work3 = lim < dist;
                    p->unk_3CC = func_80057094(p, wx, wz, work3);
                    va = 300;
                    vn = p->unk_3CC & 4;
                    if (vn) {
                        va = 1000;
                    }
                    if (CPU_SQ(p->cpu_route.node[p->cpu_route.count - 1].x -
                               p->unk_F4.x) +
                            CPU_SQ(p->cpu_route.node[p->cpu_route.count - 1].z -
                                   p->unk_F4.z) <
                        va * (vn ? 1000 : 300)) {
                        if (--p->cpu_route.count != 0) {
                            work3 =
                                (ratan2(
                                     p->cpu_route.node[p->cpu_route.count - 1]
                                             .x -
                                         p->unk_F4.x,
                                     p->cpu_route.node[p->cpu_route.count - 1]
                                             .z -
                                         p->unk_F4.z) -
                                 p->unk_1C8.vy) &
                                0xFFF;
                            if (work3 > 0x800) {
                                work3 -= 0x1000;
                            }
                            if ((work3 < 0 ? -work3 : work3) > 0x300) {
                                return 0;
                            }
                        }
                    }
                }
            }
        after_nav:
            if (p->unk_3CC != 0) {
                return p->unk_3CC;
            }
        }

        if ((!(p->unk_430 & 0x80) &&
             D_800A387C < p->other->unk_404[p->other->unk_86] &&
             ((p->unk_444[3] >= 2 &&
               (p->unk_444[5] >= 2 || p->unk_444[1] >= 2)) ||
              p->unk_444[3] == 1 || p->unk_444[4] == 1 ||
              p->unk_444[2] == 1)) ||
            ((D_80099D88[p->unk_443].flags & 0x4000 || p->unk_443 == 0x18 ||
              p->unk_443 == 0x1A) &&
             D_800A387C < p->other->unk_404[p->other->unk_86] &&
             p->other->unk_43C < 0x10) ||
            (p->unk_0E >= 7 &&
             D_800A387C < p->other->unk_404[p->other->unk_86] &&
             p->other->unk_43C < 0x10 && p->unk_34A != 0)) {
            if (!(D_80099D88[p->unk_443].flags & 0x8F00)) {
                if ((p->cpu_route.count = func_800571C0(p)) != 0) {
                    p->unk_39D = 2;
                    return -1;
                }
            }
            if (!(D_80099D88[p->unk_443].flags & 0x8C00)) {
                cnt = 0;
                work3 = D_80099D88[p->unk_443].pick_weight[4] < (rand() & 0xFF);
                sel = -1;
                work1 = p->unk_444[1] == 0;
                work2 = p->unk_444[5] == 0;
                for (; cnt < 2; cnt++, work3++) {
                    if (work3 & 1) {
                        if (p->other->unk_43A < 0) {
                            if (work1) {
                                sel = 6;
                            } else if (work2) {
                                sel = 7;
                            }
                        } else {
                            if (work2) {
                                sel = 7;
                            } else if (work1) {
                                sel = 6;
                            }
                        }
                        if (sel != -1) {
                            break;
                        }
                    } else if (p->unk_440 != 4 && D_800A387C > 3000 &&
                               D_800A387C < 5000) {
                        sel = 8;
                        break;
                    }
                }
                if (sel != -1) {
                    p->unk_394 = sel;
                    p->unk_39C = 0;
                    p->unk_398 = p->unk_39A;
                }
            }
        }

        if (p->unk_398 >= p->unk_39A) {
            script3 = 0;
            if (p->unk_39C != 1) {
                work3 = p->unk_394;
            } else if (p->other->unk_6A == 0x19) {
                u8 slots0[4];
                __builtin_memcpy(slots0, D_800A325C, 4);
                work3 = slots0[p->other->unk_441];
            } else if (p->other->unk_6A == 0x1A) {
                u8 slots1[4];
                __builtin_memcpy(slots1, D_800A3260, 4);
                work3 = slots1[p->other->unk_441];
            }
            switch (work3) {
            case 0:
                if (p->unk_444[3] == 0) {
                    p->unk_3CC = 0x8000;
                }
                break;
            case 1:
                if (p->unk_444[0] == 0) {
                    p->unk_3CC = 0x2000;
                }
                break;
            case 2:
                if (p->unk_444[1] == 0) {
                    p->unk_3CC = 0x4000;
                }
                break;
            case 3:
                if (p->unk_444[5] == 0) {
                    p->unk_3CC = 0x1000;
                }
                break;
            case 4:
                if (p->unk_444[3] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A890;
                }
                break;
            case 5:
                if (p->unk_444[0] == 0 && p->unk_3F8[p->unk_86] < D_800A387C) {
                    script3 = D_8009A888;
                }
                break;
            case 6:
                if (p->unk_444[1] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A878;
                }
                break;
            case 7:
                if (p->unk_444[5] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A880;
                }
                break;
            case 8:
                if (!(p->unk_444[6] == 1 || p->unk_444[6] == 2) &&
                    D_800A387C > 3000 && D_800A387C < 5000 &&
                    p->unk_43C < 0x200) {
                    script3 = D_8009A89C;
                }
                break;
            }
            if ((p->unk_426 == 1 || p->unk_426 == 2) ||
                (p->unk_425 == 1 || p->unk_425 == 2)) {
                if (p->unk_3CC != 0) {
                    p->unk_398 = p->unk_39A + 1;
                } else {
                    p->unk_398 = 0;
                    script3 = 0;
                }
            } else if ((p->unk_430 & 0x800) || p->unk_442 != 0 ||
                       (p->unk_3CC == 0x2000 &&
                        D_800A387C < p->other->unk_3F8[p->other->unk_86])) {
                script3 = 0;
                p->unk_398 = 0;
                p->unk_3CC = 0;
            }
            if (script3 != 0) {
                p->unk_398 = 0;
                if (p->unk_430 & 0xA000) {
                    func_80055B44(p, script3, 4, 0);
                }
            }
        } else if (p->unk_398 == 0) {
            s32 tired = D_80099D88[p->unk_443].unk6;
            r = rand() & 0xFF;
            if (p->unk_440 == 4 ? r < (tired >> 2) : r < tired) {
                work2 = -1;
                besti = -1;
                pick = 0;
            pick_loop: {
                score = ((rand() & 0xFFF) *
                         D_80099D88[p->unk_443].pick_weight[pick]) >>
                        12;
                if (score != 0) {
                    flip = 0;
                    switch (pick) {
                    case 0:
                    case 2:
                        if (rand() & 1) {
                            flip = 1;
                        } else if (p->unk_444[3] != 0) {
                            goto pick_next;
                        }
                        vc = p->unk_3F0;
                        if (D_80099D88[p->unk_443].unk7 <
                            (vc >= 0 ? vc : -vc)) {
                            score += 0x80;
                            flip = vc > 0;
                        }
                        if (p->unk_0E >= 6 && p->unk_34A == 0 && pick == 2) {
                            score += 0x100;
                            flip = 0;
                        }
                        break;
                    case 1:
                    case 3:
                        if (rand() & 1) {
                            if (p->unk_444[5] != 0) {
                                goto pick_next;
                            }
                            flip = 1;
                        } else if (p->unk_444[1] != 0) {
                            goto pick_next;
                        }
                        if (p->unk_430 & 0x20000) {
                            score += 0x80;
                            flip = p->unk_43A > 0;
                            if (*(flip ? &p->unk_444[5] : &p->unk_444[1]) !=
                                0) {
                                flip ^= 1;
                            }
                        }
                        break;
                    case 4:
                        if ((p->unk_444[6] == 1 || p->unk_444[6] == 2) ||
                            !(D_800A387C >= 3000 && D_800A387C <= 5000) ||
                            p->unk_43C >= 0x201 || p->unk_440 == 4 ||
                            p->other->unk_6A == 0x18 ||
                            p->other->unk_6A == 0x2A) {
                            goto pick_next;
                        }
                        break;
                    }
                    /* Q82: work2's best score (the Q75 value) is compared as an
                     * s16, as the target does (sll 16; sra 16; slt). No cast,
                     * an s16 best or score local, or work2 as s16 all differ.
                     */
                    if ((s16)work2 < score) {
                        besti = pick;
                        work2 = score;
                        bestflip = flip;
                    }
                }
            }
            pick_next:
                if (++pick < 7) {
                    goto pick_loop;
                }
                if (besti != -1) {
                    p->unk_394 = 0;
                    p->unk_39C = 0;
                    p->unk_398 =
                        (((((rand() & 0xFFF) * D_80099D88[p->unk_443].unk6) >>
                           12) +
                          0x17)
                         << 12) /
                        p->unk_1C;
                    switch (besti) {
                    case 0:
                        p->unk_394 = bestflip != 0;
                        break;
                    case 1:
                        if (bestflip) {
                            p->unk_394 = 3;
                        } else {
                            p->unk_394 = 2;
                        }
                        break;
                    case 2:
                        if (bestflip) {
                            p->unk_394 = 5;
                        } else {
                            p->unk_394 = 4;
                        }
                        break;
                    case 3:
                        if (bestflip) {
                            p->unk_394 = 7;
                        } else {
                            p->unk_394 = 6;
                        }
                        break;
                    case 4:
                        p->unk_394 = 8;
                        break;
                    case 5:
                        p->unk_39C = 1;
                        break;
                    }
                }
            }
        }
        if (p->unk_398 != 0) {
            p->unk_398--;
        }
    }

    if (p->unk_3CC != 0 || p->unk_398 != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 6) && !(p->unk_430 & 0x800)) {
        u16 state;

        state = p->unk_6A;
        if (state == 0x15 || (state == 0x19 && p->unk_441 >= 2)) {
            if ((p->unk_3E8 & 1) || p->unk_0E >= 6) {
                if (p->unk_442 == 0 && p->unk_444[3] != 1 &&
                    (p->unk_0E < 6 || p->unk_34A != 0)) {
                    pbest = -1;
                    if (p->unk_3F2 % ((p->unk_438 >> 8) + 2) ==
                        (p->unk_438 >> 8) + 1) {
                        lv = p->unk_438 >> 1;
                    } else {
                        lv = p->unk_438;
                    }
                    list = p->unk_3A8[p->unk_86];
                    off = *list;
                    work4 = 0;
                    while (off != 0) {
                        /* work5 holds three values in turn, each read before
                         * the next write: case 2's pattern-word top bits
                         * (work1 >> 27); the skill offset
                         * ((0x1000 - lv) * 625 >> 10) - 400; a copy of the
                         * entry type et for the et < 5 and et == 5 / 6 tests
                         * (Q34) (Ruling 11, reused-local-necessity). */
                        s32 work5;
                        /* FAKE: opaque 1 (no-new-park-categories entry 2,
                         * loop-rotation-two-shift): with a
                         * literal the mask tests `(x & (1 << n)) == 0` fold to
                         * srav; andi, while the target shifts the 1 held in
                         * $fp, set once before the loop and shared with case
                         * 2's mask shifts. `1U << n`, a u32 mask local and a
                         * function-scope `one` all differ */
                        s32 one = 1;
                        ep = off + (u8 *)p->unk_3A4;
                        e = ep;
                        ep += 4;
                        /* FAKE: pass-through pointer alias
                         * (pointer-alias-fake-exception; SOTN precedent
                         * below): the target copies the script start into its
                         * own register and reads the 0x40 character-mask header
                         * through it while ep stays in $s6; read through ep the
                         * allocation order shifts: score 58 */
                        /* SOTN: src/st/no0/e_stone_rose.c:611 @aa53500 */
                        q = ep;
                        if (D_80099D88[p->unk_443].flags & 0xFF00) {
                            switch (D_800A38DC) {
                            case 3:
                                if (D_800A38E2 < 0x5B) {
                                    work3 = D_800A38E2 / 10 * 2;
                                    if (D_800A38E2 % 10 == 0) {
                                        work3--;
                                    }
                                } else if (D_800A38E2 < 0x5E) {
                                    work3 = 0x12;
                                } else if (D_800A38E2 < 0x60) {
                                    work3 = 0x13;
                                } else if (D_800A38E2 < 0x62) {
                                    work3 = 0x14;
                                } else if (D_800A38E2 < 0x64) {
                                    work3 = 0x15;
                                } else {
                                    work3 = 0x16;
                                }
                                work3 = D_8009A928[p->unk_440][work3];
                                break;
                            case 2:
                                work1 = D_8009A9F0[D_8009A9DC[p->unk_0E]
                                                             [p->unk_440]]
                                                  [D_800A3788];
                                work3 = 0;
                                work5 = work1 >> 27;
                                work2 = work1 & 0xF;
                                if (work2 != 0) {
                                    if (work5) {
                                        while (work2 > 0) {
                                            work1 >>= 4;
                                            work3 |= one << ((work1 & 0xF) - 1);
                                            work2--;
                                        }
                                    } else {
                                        work1 >>=
                                            (p->unk_3F2 / 3 % work2) * 4 + 4;
                                        work3 = one << ((work1 & 0xF) - 1);
                                    }
                                }
                                break;
                            default:
                                work3 =
                                    D_8009A8C8[p->unk_440][D_800A37A0 - 1].mask;
                                break;
                            }
                            if ((e[3] >> 4) == 0 ||
                                !(work3 & (one << ((e[3] >> 4) - 1)))) {
                                goto next;
                            }
                        }
                        if (!(D_80099D88[p->unk_443].flags & 0x80) &&
                            p->unk_40D == p->unk_86 && p->unk_40C == work4) {
                            goto next;
                        }
                        if (q[0] == 0x40) {
                            work3 = q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1];
                            if (!(work3 & (one << p->unk_443))) {
                                goto next;
                            }
                            ep += 5;
                        }
                        if ((e[0] & 0x80) && p->unk_26C == 0) {
                            goto next;
                        }
                        work1 = e[1] * 40;
                        hi = e[2] * 40;
                        /* FAKE: do-while(0): its loop depth weights et's
                         * references so et is allocated ahead of ep (et $s5, ep
                         * $s6, as in the target); unwrapped: score 10
                         * (do-while-zero-exception) */
                        do {
                            et = e[0] & 7;
                        } while (0);
                        work3 = 0;
                        if (et == 0) {
                            if (work1 < D_800A387C && D_800A387C < hi) {
                                if (p->other->unk_6A == 0x15 ||
                                    p->other->unk_6A == 0x2C ||
                                    p->other->unk_6A == 0xE ||
                                    p->other->unk_6A == 0x19) {
                                    work3 = 1;
                                }
                            }
                        } else {
                            if ((D_80099D88[p->unk_443].flags & 0xFC00) ||
                                p->unk_0E >= 6) {
                                work1 = et < 5 ? 100000 : 0;
                                hi = 100000;
                            } else {
                                work5 = (((0x1000 - lv) * 625) >> 10) - 400;
                                work1 += work5 + p->unk_40A;
                                hi += work5 + p->unk_40A;
                            }
                            work5 = et;
                            if (work5 < 5) {
                                if (D_800A387C < work1 &&
                                    !(p->unk_430 & 0x20000)) {
                                    if ((p->unk_430 & 0x200)
                                            ? p->unk_43C < 0x800
                                            : p->unk_43C < 0x400) {
                                        work3 = 1;
                                    } else if (p->unk_0E >= 6) {
                                        work3 = 1;
                                    }
                                }
                            } else if (
                                work1 < D_800A387C && D_800A387C < hi &&
                                p->unk_43C <
                                    0x200 - ((p->unk_438 * 0x100) >> 12) &&
                                /* Q76: unk_438 / 16 as the 4.12 multiply by
                                 * 0x100 (1/16); `>> 4` merges the shift into
                                 * the sign extension (lhu; sll 16; sra 20), the
                                 * target has lh; sra 4. */
                                (p->unk_430 & 0x280) != 0x280) {
                                switch (work5) {
                                case 5:
                                    if ((0x78 >> p->unk_B1) & 1) {
                                        work3 = 1;
                                    }
                                    break;
                                case 6:
                                    if (p->unk_443 == 0x15 || p->unk_330 != 0) {
                                        work3 = 1;
                                    }
                                    break;
                                default:
                                    work3 = 1;
                                    break;
                                }
                            }
                        }
                        if (work3) {
                            sc = ((rand() & 0xFFF) *
                                  D_80099D88[p->unk_443].script_weight[et]) >>
                                 12;
                            if (sc != 0 && pbest < sc) {
                                pbest = sc;
                                pbesti = work4;
                                pscript = ep;
                                phi = hi;
                            }
                        }
                    next:
                        list++;
                        off = *list;
                        work4++;
                    }
                    if (pbest != -1) {
                        func_80055B44(p, pscript, 0, 0);
                        p->unk_40C = pbesti;
                        p->unk_40D = p->unk_86;
                        p->unk_40E = p->unk_F4.x;
                        p->unk_410 = p->unk_F4.z;
                        p->unk_3F2++;
                        p->unk_412 = phi;
                    }
                }
            } else if (state == 0x15 && p->unk_26C != 0 && p->unk_440 != 4 &&
                       /* Q76: as above, 4.12 factor 0x100; target lh; sra 4 at
                          0x8005ACCC. */
                       p->unk_43C < 0x200 - ((p->unk_438 * 0x100) >> 12) &&
                       !(D_800A38DC == 2 || D_800A38DC == 3)) {
                script4 = 0;
                if ((rand() & 0xFF) <
                        (D_80099D88[p->unk_443].script_weight[5] >> 2) &&
                    ((0x78 >> p->unk_B1) & 1) && p->unk_442 == 0 &&
                    p->other->unk_404[p->other->unk_86] < D_800A387C &&
                    D_800A387C < 4500) {
                    script4 = D_8009A8C0;
                } else {
                    s32 level;

                    if (p->unk_443 == 0x15) {
                        level = p->unk_34D;
                        work1 = p->other->unk_3F8[p->other->unk_86];
                    } else {
                        work1 = p->other->unk_404[p->other->unk_86] + 300;
                        if (D_80099D88[p->unk_443].flags & 0x300) {
                            level = 0;
                            if (D_800A37A0 >= 6) {
                                level = p->unk_34A;
                            }
                        } else {
                            level = p->unk_330;
                        }
                    }
                    ok4 = 0;
                    if ((rand() & 0xFF) <
                            (D_80099D88[p->unk_443].script_weight[6] >> 2) &&
                        level != 0 && work1 < D_800A387C &&
                        p->unk_434 == 100000 && p->unk_442 == 0 &&
                        (p->unk_430 & 0xA002) &&
                        (p->unk_443 != 0x15 || D_800A387C < 3000) &&
                        (p->unk_8A == 0 || level >= 2)) {
                        ok4 = 1;
                    }
                    if (ok4) {
                        if ((D_80099D88[p->unk_443].flags & 0x10) &&
                            level >= 2 && (rand() & 1)) {
                            script4 = D_8009A8B4;
                        } else {
                            script4 = D_8009A8AC;
                        }
                    }
                }
                if (script4 != 0) {
                    func_80055B44(p, script4, 2, 0);
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if (p->unk_6A == 0x15) {
        if (p->unk_43C > 0x100 && !(p->unk_6C == 0x19 || p->unk_6C == 0x1A) &&
            p->unk_0E < 7 && p->unk_443 != 0x16) {
            p->unk_39C = 0;
            p->unk_398 = 0x17000 / p->unk_1C;
            if (p->unk_444[5] == 0) {
                p->unk_394 = 3;
            } else if (p->unk_444[1] == 0) {
                p->unk_394 = 2;
            } else if (p->unk_444[0] == 0) {
                p->unk_394 = 1;
            } else {
                p->unk_394 = 0;
            }
        } else if (p->unk_0E >= 6) {
            if (p->unk_34A == 0 && p->unk_34B != 0 && p->unk_26C != 0 &&
                p->other->unk_404[p->other->unk_86] < D_800A387C) {
                p->unk_3CC = 0x80;
            }
        } else if ((p->unk_430 & 0x800) && (D_80099D88[p->unk_443].flags & 1) &&
                   D_800A387C < 4000 && p->unk_440 != 4 &&
                   (p->unk_442 == 0 || p->unk_442 == 2)) {
            func_80055B44(p, D_8009A898, 1, p->unk_3BD);
            p->unk_3F2++;
        } else if ((p->unk_430 & 0xA801) || p->other->unk_6A == 0x18 ||
                   p->other->unk_6A == 0x25 || p->other->unk_6A == 8 ||
                   p->other->unk_6A == 0xA ||
                   (p->other->unk_6A == 0x1A && p->unk_441 == 1)) {
            if (D_80099D88[p->unk_443].unk3 != 0 &&
                ((!(D_80099D88[p->unk_443].flags & 0xFF00) &&
                  p->other->unk_404[p->other->unk_86] < D_800A387C &&
                  p->unk_3F4 >= D_80099D88[p->unk_443].unk3) ||
                 ((D_80099D88[p->unk_443].flags & 0x100) &&
                  p->other->unk_3F8[p->other->unk_86] < D_800A387C &&
                  p->unk_3F4 >= D_80099D88[p->unk_443].unk3 &&
                  p->unk_440 != 2) ||
                 ((D_80099D88[p->unk_443].flags & 0x7C00) &&
                  p->other->unk_3F8[p->other->unk_86] < D_800A387C &&
                  p->unk_3F4 >= D_80099D88[p->unk_443].unk3 &&
                  D_800A38E2 >= 0x5B) ||
                 (p->unk_430 & 0x40000))) {
                p->unk_3F4 = 0;
                p->unk_3CC = 0x80;
                p->unk_430 &= ~0x40000;
            }
        }
    }
    return p->unk_3CC;
}

#undef CPU_SQ
extern s32 g_vab_vb_sbaddr[];
extern Unk8005C2A8Pack *g_vab_rec_ptr[];
extern void func_800858D0(s32);
extern s32 SsUtSetReverbType(s32);

void snd_Init(void) {
    s32 *p1;
    Unk8005C2A8Pack **p2;
    s32 i;
    s32 j;

    i = 0;
    p1 = g_vab_vb_sbaddr;
    p2 = g_vab_rec_ptr;
    do {
        *p2 = 0;
        *p1 = 0;
        p1 += 1;
        i += 1;
        p2 += 1;
    } while (i < 0x10);
    SsInit();
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    SsSetReservedVoice(0);
    SsSetTickMode(1);
    for (j = 0; j < 24; j++) {
        D_800EFB78[j].req = 0;
        D_800EFB78[j].volr = D_800EFB78[j].voll = 0x7F;
    }
    SsStart();
    D_800A3408 = 0;
    D_800A3400 = 0;
}

void func_800858D0(s32);

void snd_Quit(void) {
    s32 i;
    s32 *a0;
    Unk8005C2A8Pack **v1;
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    SsEnd();
    SsQuit();
    i = 0;
    a0 = g_vab_vb_sbaddr;
    v1 = g_vab_rec_ptr;
    do {
        *v1 = 0;
        *a0 = 0;
        a0++;
        i++;
        v1++;
    } while (i < 0x10);
    D_800A3408 = 0;
}

void func_8005B58C(void) { func_800858D0(0); }

void func_8005B5AC(void) {
    s32 i;
    func_800858D0(0);
    for (i = 0; i < 24; i++) {
        D_800EFB78[i].req = 0;
        D_800EFB78[i].volr = D_800EFB78[i].voll = 0x7F;
        func_80086130((s16)i, 0, 0);
    }
}

extern s32 D_800F1180;

void func_8005B644(s32 a0) {
    s32 v;
    func_800858D0(0);
    v = a0 * 2 + a0 + 1;
    SsVabClose(v);
    g_vab_rec_ptr[v] = 0;
    g_vab_vb_sbaddr[v] = 0;
}

void func_8005B6AC(void) {
    func_800858D0(0);
    SsVabClose(2);
    g_vab_rec_ptr[2] = 0;
    g_vab_vb_sbaddr[2] = 0;
    SsVabClose(5);
    g_vab_rec_ptr[5] = 0;
    g_vab_vb_sbaddr[5] = 0;
}

void snd_CloseVab1(void) {
    SsVabClose(1);
    g_vab_rec_ptr[1] = 0;
    g_vab_vb_sbaddr[1] = 0;
}

s32 SsUtSetReverbType(s32);

void func_8005B72C(void) {
    s32 s0;
    s32 *s2;
    Unk8005C2A8Pack **s1;
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    s2 = &g_vab_vb_sbaddr[1];
    s1 = &g_vab_rec_ptr[1];
    for (s0 = 1; s0 < 0x10; s0++) {
        SsVabClose((s16)s0);
        *s1 = 0;
        *s2 = 0;
        s2++;
        s1++;
    }
    D_800A3408 = 0;
    func_8005B5AC();
}

#define NULL ((void *)0)

typedef struct Vec3s16 {
    s16 x;
    s16 y;
    s16 z;
} Vec3s16;

typedef struct Vec3s32 {
    s32 x;
    s32 y;
    s32 z;
} Vec3s32;

typedef struct Vec3 {
    s32 vx, vy, vz, pad;
} Vec3;

s32 printf(s32 *, s32); /* extern */

const char D_800158B4[24] = "common_vab start:%08x\n";

s32 snd_LoadCommonVab(s32 arg0) {
    s32 temp_v0;
    u32 temp_s0;
    s32 ret;

    func_800858D0(0);
    printf(&D_800158B4, arg0);
    game_FrameLoop();
    temp_v0 = func_80036EA8(2, 1);
    cdrom_StartRead(temp_v0, arg0);
    temp_s0 = cdrom_GetFileSize(temp_v0);
    game_FrameLoop();
    D_800A3408 = 0;
    D_800A340C = 0x1010;
    g_vab_sticky_sbaddr = 0x1010;
    ret = func_8005C2A8((Unk8005C2A8Pack *)arg0, 0, arg0 + temp_s0);
    D_800A340C = g_vab_sticky_sbaddr;
    return ret;
}

void func_8005B868(void) {
    func_800858D0(0);
    SsVabClose(8);
    g_vab_rec_ptr[8] = 0;
    g_vab_vb_sbaddr[8] = 0;
    SsVabClose(4);
    g_vab_rec_ptr[4] = 0;
    g_vab_vb_sbaddr[4] = 0;
}

s32 func_8005B8B8(s32 arg0) {
    s32 t0;
    s32 size;
    s32 ret;
    s32 t0_2;

    func_8005B868();
    func_800858D0(0);
    t0 = func_80036EA8(2, 0x5D);
    game_FrameLoop();
    cdrom_StartRead(t0, arg0);
    size = cdrom_GetFileSize(t0);
    game_FrameLoop();
    ret = func_8005C2A8((Unk8005C2A8Pack *)arg0, 8, arg0 + size);
    t0_2 = func_80036EA8(2, 0x5E);
    game_FrameLoop();
    cdrom_StartRead(t0_2, arg0 + ret);
    size = cdrom_GetFileSize(t0_2) + ret;
    game_FrameLoop();
    return func_8005C2A8((Unk8005C2A8Pack *)(arg0 + ret), 4, arg0 + size) + ret;
}

s32 snd_VabFakeOpen(s32, s16);

void snd_VabFakeOpen8And4(s32 a0) {
    snd_VabFakeOpen(a0, 8);
    snd_VabFakeOpen(a0, 4);
}

void func_8005B9C4(void) {
    func_800858D0(0);
    SsVabClose(9);
    g_vab_rec_ptr[9] = 0;
    g_vab_vb_sbaddr[9] = 0;
}

s32 func_8005B9FC(s32 a0) {
    s32 task;
    s32 size;
    func_8005B9C4();
    task = func_80036EA8(2, 8);
    game_FrameLoop();
    cdrom_StartRead(task, a0);
    size = cdrom_GetFileSize(task);
    game_FrameLoop();
    return func_8005C2A8((Unk8005C2A8Pack *)a0, 9, a0 + size);
}

void snd_VabFakeOpen9(s32 a0) { snd_VabFakeOpen(a0, 9); }

typedef struct {
    s32 off;
    s32 size;
} VabEnt;

typedef struct {
    VabEnt ent[3];
    s32 len[3];
} VabLoad;

extern u8 g_vab_id_list[];

s32 func_8005BA8C(s32 hdr, s32 arg1, s32 arg2, s32 arg3) {
    VabLoad loc;
    u8 *p;
    s32 base;
    s32 task;
    s32 size;
    u8 count;
    s32 i;
    u32 j;

    p = (u8 *)hdr;
    func_800858D0(0);
    for (i = 0; i < 3; i++) {
        SsVabClose(g_vab_id_list[i]);
        g_vab_rec_ptr[g_vab_id_list[i]] = 0;
        g_vab_vb_sbaddr[g_vab_id_list[i]] = 0;
    }
    task = func_80036EA8(2, arg1 + 9);
    game_FrameLoop();
    cdrom_StartRead(task, (s32)p);
    size = cdrom_GetFileSize(task);
    game_FrameLoop();
    ((s32 *)p)[12] += (s32)p;
    func_80062020(((s32 *)p)[12]);
    count = 3;
    base = (s32)p;
    if (arg2 == arg3) {
        count = 2;
    }
    loc.ent[0].off = ((VabEnt *)p)[0].off;
    loc.ent[0].size = ((VabEnt *)p)[0].size;
    loc.ent[1].off = ((VabEnt *)p)[arg2 + 1].off;
    loc.ent[1].size = ((VabEnt *)p)[arg2 + 1].size;
    if (count == 3) {
        loc.ent[2].off = ((VabEnt *)p)[arg3 + 1].off;
        loc.ent[2].size = ((VabEnt *)p)[arg3 + 1].size;
    }
    for (i = 0; i < count; i++) {
        loc.ent[i].off += (s32)p;
        loc.len[i] = func_8005C2A8(
            (Unk8005C2A8Pack *)loc.ent[i].off, g_vab_id_list[i], (s32)p + size);
    }
    for (i = 0; i < count; i++) {
        for (j = 0; j < (u32)loc.len[i]; j++) {
            p[j] = ((u8 *)loc.ent[i].off)[j];
        }
        snd_VabFakeOpen((s32)p - loc.ent[i].off, g_vab_id_list[i]);
        loc.ent[i].off = (s32)p;
        p += loc.len[i];
    }
    if (count == 2) {
        g_vab_rec_ptr[6] = g_vab_rec_ptr[3];
    }
    return (s32)p - base;
}

void func_8005BD30(s32 arg0) {
    u8 count;
    s32 i;
    func_800858D0(0);
    count = (g_vab_rec_ptr[6] == g_vab_rec_ptr[3]) ? 2 : 3;
    i = 0;
    if (count != 0) {
        do {
            snd_VabFakeOpen(arg0, g_vab_id_list[i & 0xFF]);
            i += 1;
        } while ((u32)(i & 0xFF) < (u32)count);
    }
    if (count == 2) {
        g_vab_rec_ptr[6] = g_vab_rec_ptr[3];
    }
}

void snd_CloseListedVabs(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        SsVabClose(g_vab_id_list[i]);
        g_vab_rec_ptr[g_vab_id_list[i]] = 0;
        g_vab_vb_sbaddr[g_vab_id_list[i]] = 0;
    }
}

extern s16 D_8009AD1C[][2];

s32 func_8005BE84(s32 arg0) {
    s32 result;
    s16 *p;
    s16 temp_a0;
    s16 *base;
    s32 doubled;
    func_800858D0(0);
    base = &D_8009AD1C[0][0];
    p = base + arg0 * 2;
    doubled = arg0 << 1;
    if (*p >= 0) {
        SsUtReverbOff();
        SsUtSetReverbType(0);
        SsUtSetReverbDepth(0, 0);
        result = SsUtSetReverbType(*p);
        SpuClearReverbWorkArea(*p);
        temp_a0 = doubled + 1;
        SsUtSetReverbDepth(temp_a0, temp_a0);
        SsUtReverbOn();
    } else {
        result = -1;
    }
    return (s16)result;
}

void func_8005BF3C(void) {
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
}

extern s32 SsVabFakeHead();

s32 snd_MoveVabBody(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    func_800858D0(0);
    SsVabClose((s16)arg1);
    SpuSetTransferStartAddr(arg3);
    SpuRead(arg0, g_vab_rec_ptr[arg1]->unk_0C);
    SpuIsTransferCompleted(1);
    SpuSetTransferStartAddr(arg2);
    SpuWrite(arg0, g_vab_rec_ptr[arg1]->unk_0C);
    SpuIsTransferCompleted(1);
    SsVabFakeHead(g_vab_rec_ptr[arg1]->unk_04, (s16)arg1, arg2);
    SsVabFakeBody((s16)arg1);
    g_vab_vb_sbaddr[arg1] = arg2;
    return arg2 + g_vab_rec_ptr[arg1]->unk_0C;
}

/* SPU VAB compaction: sorts the resident VAB slots 1..15 by SPU address, then
 * walks them from the end of slot 0, moving down (snd_MoveVabBody) the first
 * slot that is not already contiguous and every slot after it. `vabid` is never
 * read. `addr` is assigned inside the first (otherwise empty) loop: the target
 * computes it after the loop's `count > 0` guard. */
s32 func_8005C074(s16 vabid, s32 base) {
    s16 order[16];
    s16 count;
    u16 mask;
    u32 min;
    s16 minidx;
    s16 i;
    s16 j;
    s16 k;
    s32 addr;

    count = 0;
    mask = 0;
    for (;;) {
        min = 0x7FFFF;
        minidx = -1;
        for (i = 1; i < 16; i++) {
            if (!((mask >> i) & 1) && g_vab_vb_sbaddr[i] != 0 &&
                g_vab_vb_sbaddr[i] < min) {
                min = g_vab_vb_sbaddr[i];
                minidx = i;
            }
        }
        if (minidx == -1) {
            break;
        }
        order[count++] = minidx;
        mask += 1 << minidx;
    }
    for (j = 0; j < count; j++) {
        addr = g_vab_vb_sbaddr[0] + g_vab_rec_ptr[0]->unk_0C;
    }
    for (j = 0; j < count; j++) {
        if (g_vab_vb_sbaddr[order[j]] == addr) {
            addr += g_vab_rec_ptr[order[j]]->unk_0C;
        } else {
            for (k = j; k < count; k++) {
                addr = snd_MoveVabBody(
                    base, order[k], addr, g_vab_vb_sbaddr[order[k]]);
            }
            return 0;
        }
    }
    return 0;
}

/* func_8005C2A8 - load a VAB: close whatever occupies slot `vabid`, recompute
 * the sticky SPU address from the resident banks (compacting them with
 * func_8005C074 for vabid != 0), relocate the header's three offsets, open it
 * with snd_VabOpen and record its slot and SPU address. Returns the third
 * header offset (unrelocated), or 0 on failure. snd_VabOpen returns s32, as
 * this call site's bytes say (the sll/sra in its body comes from an explicit
 * cast). */

extern s32 snd_VabOpen(Unk8005C2A8Pack *, s16);

const char D_800158CC[20] = "vab id:%d mistake\n";

s32 func_8005C2A8(Unk8005C2A8Pack *hdr, s16 vabid, s32 arg2) {
    s16 i;
    s16 id;

    if (func_80077D00()->unk14_0 == 3 && vabid == 5) {
        return 0;
    }
    func_800858D0(0);
    if (g_vab_rec_ptr[vabid] != 0) {
        SsVabClose(vabid);
        g_vab_rec_ptr[vabid] = 0;
        g_vab_vb_sbaddr[vabid] = 0;
    }
    if (vabid != 0) {
        g_vab_sticky_sbaddr = g_vab_vb_sbaddr[0];
        for (i = 0; i < 16; i++) {
            if (g_vab_rec_ptr[i] != 0) {
                g_vab_sticky_sbaddr += g_vab_rec_ptr[i]->unk_0C;
            }
        }
    }
    D_800A3408 = g_vab_sticky_sbaddr - D_800A340C;
    if (vabid != 0) {
        func_8005C074(vabid, arg2);
    }
    hdr->unk_00 += (s32)hdr;
    hdr->unk_04 += (s32)hdr;
    hdr->unk_08 += (s32)hdr;
    id = snd_VabOpen(hdr, vabid);
    SsVabTransCompleted(1);
    if (id != -1) {
        g_vab_rec_ptr[id] = hdr;
        D_800A3408 += hdr->unk_0C;
        g_vab_sticky_sbaddr = D_800A340C + D_800A3408;
        g_vab_vb_sbaddr[vabid] = SsUtGetVBaddrInSB(vabid);
        return hdr->unk_08 - (s32)hdr;
    }
    printf(D_800158CC, vabid);
    return 0;
}

/* snd_VabFakeOpen tail: the s16 result carrier and single trailing return give
 * the target's CFG (li -1 in its own block, shared sign-extend join); a direct
 * return or an s32 carrier does not match
 * (proven-spelling-class-reconstruction). */
s32 snd_VabFakeOpen(s32 arg0, s16 arg1) {
    s32 idx;
    Unk8005C2A8Pack **base;
    Unk8005C2A8Pack **p;
    Unk8005C2A8Pack *v;
    s16 ret;
    func_800858D0(0);
    idx = arg1;
    /* FAKE: the slot array's address is staged in base ahead of the slot index:
       written `&g_vab_rec_ptr[idx]`, the index's `sll s3,s2,2` rises above the
       array's lui / addiu (score 2). */
    base = g_vab_rec_ptr;
    p = base + idx;
    v = *p;
    if (v != 0) {
        v = (Unk8005C2A8Pack *)((u8 *)v + arg0);
        *p = v;
        v->unk_00 = v->unk_00 + arg0;
        (*p)->unk_04 = (*p)->unk_04 + arg0;
        SsVabClose(idx);
        ret = SsVabFakeHead((*p)->unk_04, idx, g_vab_vb_sbaddr[idx]);
        if (ret != idx) {
            return ret;
        }
        ret = SsVabFakeBody(ret);
    } else {
        ret = -1;
    }
    return ret;
}

void SsVabOpenHeadSticky(s32, s16, s32);
s32 SsVabTransBody(s32, s16);

s32 snd_VabOpen(Unk8005C2A8Pack *a0, s16 a1) {
    SsVabClose(a1);
    SsVabOpenHeadSticky(a0->unk_04, a1, g_vab_sticky_sbaddr);
    ((VabHdr *)a0->unk_04)->id = a1;
    return (s16)SsVabTransBody(a0->unk_08, a1);
}

void func_8005C614(void) {
    SsSetMVol(0x7F, 0x7F);
    func_800858D0(0);
    SsSetStereo();
    SsSetAutoKeyOffMode(0);
}

extern u16 D_8009AA70[][2];

void func_8005C650(s32 a0, s32 a1, s32 a2) {
    s16 i = 0;
    u16 *req = D_8009AA70[a0];
    do {
        if (D_800EFB78[i].req == 0) {
            D_800EFB78[i].req = req;
            D_800EFB78[i].volr = a1;
            D_800EFB78[i].voll = a2;
            return;
        }
        i++;
    } while (i < 0x18);
}

/* Per-frame sound-request flush: walk the 24-entry pending-sound pool, and for
 * every entry whose VAB is loaded, find the first free SPU voice at or after
 * the running `next` cursor and key the note on with the entry's stored
 * volumes. Each pool slot is cleared as it is visited.
 */
void func_8005C6D0(void) {

    u8 keys[24];
    s16 i;
    s16 voice;
    s16 next;
    u16 vab;
    u16 *p;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; i < 0x18; i++) {
        p = D_800EFB78[i].req;
        if (p != 0 && (s32)g_vab_rec_ptr[*p] < 0) {
            voice = next;
            for (; voice < 0x18; voice++) {
                if (SpuGetKeyStatus(1 << voice) != 1) {
                    vab = *p;
                    if (vab == 6 && g_vab_rec_ptr[6] == g_vab_rec_ptr[3]) {
                        vab = 3;
                    }
                    ev = &((u32 *)g_vab_rec_ptr[vab]->unk_00)[p[1]];
                    SsUtKeyOnV(
                        (s16)voice, (s16)vab, (s16)(*ev & 0x7F),
                        (s16)((*ev >> 7) & 0xF), (s16)((*ev >> 11) & 0x7F),
                        (s16)((*ev >> 18) & 0x7F), D_800EFB78[i].voll,
                        D_800EFB78[i].volr);
                    next = voice + 1;
                    break;
                }
            }
        }
        D_800EFB78[i].req = 0;
    }
}

extern Unk8009B400Record D_8009B194[3];
extern Unk8009B400Record D_8009B1AC[2];
extern Unk8009B400Record D_8009B1BC[2];
extern Unk8009B400Record D_8009B20C[6];
extern Unk8009B400Record D_8009B23C[12];
extern Unk8009B400Record D_8009B29C[2];
extern Unk8009B400Record *D_8009B2AC[4];
/* AddPrim has no prototype here: this TU called it as an implicit-int function.
 * With PsyQ's void AddPrim(void *, void *), func_80060768's return value is
 * computed after its last AddPrim's dead sp1C bump and a load-delay nop is
 * added (192 -> 193 insns). */
extern int AddPrim();

s32 func_8005C8A8(s32 mode, s32 arg1, s32 arg2, s32 ot) {
    Unk8005C8A8Rec *chunk = (Unk8005C8A8Rec *)arg2;
    Unk8007352CEnv s;
    TILE *tile;
    s32 cur;
    s16 sel;
    u16 y_base;
    DR_MODE *mode_off;
    s32 size;
    s16 top;
    s16 i;
    s16 j;
    s16 x;
    s16 y;

    tile = chunk->unk_00;
    cur = arg2 + sizeof(chunk->unk_00);
    y_base = 0;
    /* FAKE: the low half read from arg1's stack home, which keeps arg1 in
       memory for the in-loop `lw 0xBC($sp)` too; (s16)arg1 does not. `sel =
       arg1`: score 58. */
    sel = *(s16 *)&arg1;
    mode_off = &chunk->unk_4D8;
    /* FAKE: the chunk's 0x4F0 bytes (sizeof(Unk8005C8A8Rec)) spelled from
     * mode_off as an int: combine folds it with no REG_EQUAL note, so size
     * keeps the target's sp+0x70 slot to the return and the deleted temp's
     * stale count takes the untouched sp+0x78; the literal or sizeof is
     * rematerialised at the return (frame 0x10 short). sizeof: score 33 */
    size = (s32)mode_off + 0x18 - arg2;
    top = (0xF0 - D_8009B2BC[mode].h) / 2;
    s.col_b = 0x40;
    s.col_g = 0x40;
    s.col_r = 0x40;
    s.has_color = 0;

    switch (mode) {
    case 2:
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        y_base = 0x33;
        s.y = top + 0x73;
        s.x = 0;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = ot;
        D_8009B184[0].x = (0x280 - D_8009B2BC[2].w) / 2;
        D_8009B184[1].x = (D_8009B2BC[2].w + 0x280) / 2 - 0xC;
        cur = func_8007352C(&s);
        s.header = &D_8009B0E0[8];
        s.table = D_8009B20C;
        s.x = 0;
        s.y = 0;
        s.sprt_out = cur;
        s.ot_idx = ot;
        cur = func_8007352C(&s);
        for (j = 0; j < 3; j++) {
            if (sel == j + 3) {
                s.semi = 0;
            } else {
                s.semi = 1;
            }
            s.header = &D_8009B14C;
            s.table = &D_8009B23C[j * s.header->count];
            s.x = 0;
            s.y = 0;
            s.sprt_out = cur;
            s.ot_idx = ot;
            cur = func_8007352C(&s);
            for (i = 0; i < 2; i++) {
                if (((arg1 >> (j + 16)) & 1) == i) {
                    s.semi = 0;
                } else {
                    s.semi = 1;
                }
                s.header = &D_8009B158;
                s.table = &D_8009B29C[i];
                s.x = 0;
                s.y = j * 15;
                s.sprt_out = cur;
                s.ot_idx = ot;
                cur = func_8007352C(&s);
            }
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
            tile->y0 = top + 0x73 + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot, tile);
            tile++;
        }
        /* fallthrough */
    case 0:
        if (sel == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.table = D_8009B1AC;
        s.header = &D_8009B0E0[4];
        s.x = 0;
        s.sprt_out = cur;
        s.y = y_base + top;
        s.ot_idx = ot;
        cur = func_8007352C(&s);
        s.header = &D_8009B0E0[3];
        s.has_color = 0;
        s.table = D_8009B194;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = ot;
        cur = func_8007352C(&s);
        top += 0xE;
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = ot;
        D_8009B184[0].x = (0x280 - D_8009B2BC[mode].w) / 2;
        D_8009B184[1].x = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
        cur = func_8007352C(&s);
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
            tile->y0 = top + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot, tile);
            tile++;
        }
        break;
    case 1:
        if (sel == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[5];
        s.table = D_8009B1BC;
        s.x = 0;
        s.y = top;
        s.sprt_out = cur;
        s.ot_idx = ot;
        cur = func_8007352C(&s);
        break;
    }

    s.has_color = 0;
    for (j = 2; j < 4; j++) {
        if (sel == j - 1) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[4 + j];
        s.table = D_8009B2AC[j];
        s.x = 0;
        s.y = y_base + top;
        s.sprt_out = cur;
        s.ot_idx = ot;
        cur = func_8007352C(&s);
    }

    s.has_color = 0;
    s.header = &D_8009B0E0[0];
    s.table = D_8009B164[0];
    s.x = 0;
    s.y = (0xF0 - D_8009B2BC[mode].h) / 2;
    s.semi = 0;
    s.sprt_out = cur;
    s.ot_idx = ot;
    D_8009B164[0][0].x = (0x280 - D_8009B2BC[mode].w) / 2;
    D_8009B164[0][1].x = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
    cur = func_8007352C(&s);
    s.header = &D_8009B0E0[1];
    s.table = D_8009B164[1];
    s.y = (D_8009B2BC[mode].h + 0xF0) / 2;
    s.sprt_out = cur;
    D_8009B164[1][0].x = (0x280 - D_8009B2BC[mode].w) / 2;
    D_8009B164[1][1].x = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
    func_8007352C(&s);

    for (j = 0; j < 2; j++) {
        x = (0x280 - D_8009B2BC[mode].w) / 2;
        if (j != 0) {
            y = (0xF0 - D_8009B2BC[mode].h) / 2;
        } else {
            y = (D_8009B2BC[mode].h + 0xF0) / 2;
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = x;
            tile->y0 = y + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot, tile);
            tile++;
        }
        if (j != 0) {
            x = (0x280 - D_8009B2BC[mode].w) / 2;
        } else {
            x = (D_8009B2BC[mode].w + 0x280) / 2;
        }
        y = (0xF0 - D_8009B2BC[mode].h) / 2;
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (j != 0) {
                if (i != 0) {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                } else {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                }
            } else {
                if (i != 0) {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                } else {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                }
            }
            tile->x0 = x + i * 2;
            tile->y0 = y;
            tile->w = 2;
            tile->h = D_8009B2BC[mode].h + 2;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot, tile);
            tile++;
        }
    }

    SetTile(tile);
    tile->r0 = 0;
    tile->g0 = 0;
    tile->b0 = 0;
    tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
    tile->y0 = (0xF0 - D_8009B2BC[mode].h) / 2;
    tile->w = D_8009B2BC[mode].w;
    tile->h = D_8009B2BC[mode].h;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + ot, tile);
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B0E0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + ot, mode_off);
    return size;
}

/* 0x8009B2C8: two rows of five sprite-sheet headers (func_8005D46C draws
 * headers 0 / 1 of row idx with the cells at D_8009B340 / D_8009B358;
 * func_8005D554 headers 2 and 3 / 4 with D_8009B388's), then the cells: three
 * under header 0 (count 3), six under header 1 (count 6). */
extern Unk8009B0E0Record D_8009B2C8[2][5];
extern Unk8009B400Record D_8009B340[3];
extern Unk8009B400Record D_8009B358[6];

s32 func_8005D46C(s32 arg0, s32 arg1) {
    Unk8007352CEnv s;
    /* FAKE: holder of the first walk's cursor until the second descriptor is
       set; stored directly: score 6. */
    s32 ret;
    s32 idx;
    idx = arg1;
    if (arg1 > 0) {
        idx = arg1 - 1;
    }
    s.has_color = 0;
    s.header = &D_8009B2C8[idx][0];
    s.table = D_8009B340;
    s.scale_y = 0x100;
    s.scale_x = 0x100;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = 1;
    s.ft4_out = arg0;
    ret = func_80073728(&s, 0);
    s.has_color = 0;
    s.header = &D_8009B2C8[idx][1];
    s.table = D_8009B358;
    s.scale_y = 0x100;
    s.scale_x = 0x100;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = 1;
    s.ft4_out = ret;
    return func_80073728(&s, 0);
}

s32 func_8005D554(s32 arg0, s32 arg1) {
    Unk8007352CEnv s;
    s32 i;
    u32 base_x;
    u32 base_y;
    s32 ft4;
    s32 row_off;
    s32 x;
    s32 y;
    s32 tmp1; /* FAKE: carrier, see the y sites (without tmp1: score 4) */
    /* FAKE: carrier, see the y sites (without tmp2: score 4; without both: 15)
     */
    s32 tmp2;
    s32 scale; /* FAKE: constant 0x100 in a local; the literal: score 15 */
    s32 ot;    /* FAKE: constant 1 in a local; the literal: score 2 */
    /* FAKE: pointer alias of D_8009B2C8[0][2]; direct use: score 41 */
    Unk8009B0E0Record *hdr0;
    /* FAKE: the row's header 3 in a local; at the use: score 36 */
    Unk8009B0E0Record *hdr1_row;

    D_800A326C %= 4;

    ft4 = arg0;
    if (arg1 > 0)
        arg1 -= 1;

    D_800A3418 ^= rand();
    base_x = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    base_y = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    i = 0;
    if (i < ((D_800A326C + 1) * 2)) {
        scale = 0x100;
        ot = 1;
        row_off = arg1 * 0x3C;
        hdr0 = &D_8009B2C8[0][2];
        hdr1_row = &D_8009B2C8[arg1][3];
        do {
            s.has_color = 0;
            /* FAKE: integer sum keeps `addu v0,s0,fp`; the byte-pointer sum
             * swaps it: score 1 */
            s.header = (Unk8009B0E0Record *)(row_off + (s32)hdr0);
            s.table = &D_8009B388[0];
            D_800A3418 ^= rand();
            s.scale_y = scale;
            s.scale_x = scale;
            D_800A3418 ^= rand();
            /* FAKE: split init; one expression does not match (score 3) */
            x = (s32)base_x - 0x19;
            x += ((u32)(D_800A3418 * 0x32) >> 0xF);
            /* FAKE: tmp1 is written twice (y base, then a copy of ft4) so
             * sched1 places the y base after the argument setup; dropping
             * either write or one carrier for both halves does not match (no
             * y-base write: score 4; no ft4 copy: 35; one carrier: 30) */
            tmp1 = (s32)base_y - 0xC;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp1 + ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.semi = 0;
            s.ot_idx = ot;
            tmp1 = ft4;
            s.ft4_out = tmp1;
            s.y = y;
            ft4 = func_80073728(&s, 0);

            s.has_color = 0;
            s.scale_y = scale;
            s.scale_x = scale;
            s.table = &D_8009B388[1];
            s.header = hdr1_row + (D_800A3418 & 1);
            D_800A3418 ^= rand();
            /* FAKE: split init, as in the first half (one expression: score 3)
             */
            x = (s32)base_x - 0x32;
            x += ((u32)(D_800A3418 * 0x64) >> 0xF);
            /* FAKE: tmp2 written twice, as tmp1 above (no y-base write: score
             * 4; no ft4 copy: 35) */
            tmp2 = (s32)base_y - 0x19;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp2 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.semi = 0;
            s.ot_idx = ot;
            tmp2 = ft4;
            s.ft4_out = tmp2;
            s.y = y;
            ft4 = func_80073728(&s, 0);
            i += 1;
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ft4;
}

extern Unk8009B400Record D_8009B3C8[3];
extern Unk8009B400Record D_8009B3E0[2];
extern Unk8009B400Record D_8009B3F0;
extern Unk8009B400Record D_8009B3F8;

s32 func_8005D814(Unk8001CD68Rec *arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *chunk = (Unk8005D814Rec *)arg2;
    Unk8007352CEnv s;
    s16 digit[3];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk8005D814Rec): score 76. */
    s32 end_off;
    s16 i;
    s16 j;
    s16 shown;
    Unk8009B0E0Record *hdr2;  /* FAKE: D_8009B398[2] alias; direct: score 16 */
    Unk8009B0E0Record *hdr3;  /* FAKE: D_8009B398[3] alias; direct: score 12 */
    Unk8009B400Record *cell2; /* FAKE: D_8009B3F0 alias; direct: score 18 */
    Unk8009B400Record *cell3; /* FAKE: D_8009B3F8 alias; direct: score 6 */

    arg1--;
    tile = chunk->unk_00;
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.ot_idx = arg3;
    s.semi = 0;
    s.header = &D_8009B398[0];
    cur = arg2 + sizeof(chunk->unk_00);
    mode_off = &chunk->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    for (i = 0; i < 3; i++) {
        s.table = &D_8009B3C8[i];
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }

    s.header = &D_8009B398[1];
    for (i = 0; i < 2; i++) {
        s.table = &D_8009B3E0[i];
        if (i != 0) {
            if (arg1 == 1) {
                s.table->w = 0x2D;
            } else {
                s.table->w = 0x3C;
            }
        }
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.y = 0x16;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 2; i++) {
            switch (j) {
            case 0:
                digit[i] = arg0->unk_0;
                if (i != 0) {
                    digit[i] = digit[i] % 10;
                } else {
                    digit[0] = digit[0] / 10 % 10;
                }
                s.table = &D_8009B400[digit[i]];
                if (digit[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x1A2;
                break;
            case 1:
                digit[i] = arg0->unk_2;
                if (i != 0) {
                    digit[i] = digit[i] % 10;
                } else {
                    digit[0] = digit[0] / 10 % 10;
                }
                s.table = &D_8009B400[digit[i]];
                if (digit[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x1D3;
                break;
            case 2:
                digit[i] = arg0->unk_3;
                if (i != 0) {
                    digit[i] = digit[i] % 10;
                } else {
                    digit[0] = digit[0] / 10 % 10;
                }
                s.table = &D_8009B400[digit[i]];
                if (digit[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x209;
                break;
            }
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
    }

    s.header = &D_8009B398[0];
    digit[0] = digit[1] = digit[2] = arg1;
    digit[2] = digit[2] % 10;
    digit[1] = digit[1] / 10 % 10;
    digit[0] = digit[0] / 100;
    digit[1] = digit[1] % 100;
    s.y = 0x29;
    /* FAKE: the tile loop's second sheet and cell, set ahead of the digit loop
     * (pointer-alias-fake-exception): their live range spans both loops, so
     * they are spilled and rematerialized inside the tile loop, and header[3]
     * is not cse-related to header[2] (which would reverse the $s6/$s7 order);
     * both direct: score 18 */
    /* FAKE: pointer alias; direct use scores 12 (both direct: 18) */
    hdr3 = &D_8009B398[3];
    cell3 = &D_8009B3F8; /* FAKE: pointer alias; direct use scores 6 */
    shown = 0;
    for (j = 0; j < 3; j++) {
        if (shown || digit[j] != 0 || j == 2) {
            s.table = &D_8009B400[digit[j]];
            s.table->x = 0x1F3;
            shown = 1;
            if (digit[j] == 1) {
                s.x = j * 21 + 3;
            } else {
                s.x = j * 21;
            }
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        SetTile(tile);
        tile->r0 = 0xFF;
        tile->g0 = 0x10;
        tile->b0 = 0x10;
        tile->x0 = D_8009B450[j].x;
        tile->y0 = D_8009B450[j].y;
        tile->w = 0x238 - D_8009B450[j].x;
        tile->h = 1;
        SetSemiTrans(tile, 0);
        AddPrim(g_gpu_ot_ptr + arg3, tile);
        tile++;
        /* FAKE: the first sheet and cell (pointer-alias-fake-exception), named
         * a few insns before their stores so loop.c hoists them, header then
         * cell; the cell's shorter live range takes $s6, the header $s7; both
         * direct: score 24 (all four direct: 17) */
        /* FAKE: pointer alias; direct use scores 16 (both direct: 24) */
        hdr2 = &D_8009B398[2];
        cell2 = &D_8009B3F0; /* FAKE: pointer alias; direct use scores 18 */
        s.y = D_8009B450[j].y;
        s.header = hdr2;
        s.table = cell2;
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        s.header = hdr3;
        s.table = cell3;
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B398[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

extern Unk8009B400Record D_8009B488;

s32 func_8005E098(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *chunk = (Unk8005D814Rec *)arg2;
    Unk8007352CEnv s;
    s16 d[2];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk8005D814Rec): score 53. */
    s32 end_off;
    s16 i;
    s16 j;

    tile = chunk->unk_00;
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.header = &D_8009B398[1];
    cur = arg2 + sizeof(chunk->unk_00);
    mode_off = &chunk->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    s.ot_idx = arg3;
    for (i = 0; i < 2; i++) {
        if (arg0 < 0) {
            s.table = &D_8009B488;
            if (arg1 == 1) {
                s.table->w = 0x2D;
            } else {
                s.table->w = 0x3C;
            }
        } else {
            s.table = &D_8009B458[0][i];
        }
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        if (arg0 < 0) {
            break;
        }
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.semi = 0;
    s.y = 0x16;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        if (j) {
            d[0] = d[1] = arg0;
        } else {
            d[0] = d[1] = arg1;
        }
        d[1] = d[1] % 10;
        d[0] = d[0] / 10 % 10;
        for (i = 0; i < 2; i++) {
            if (d[i] == 0 && i == 0 && arg0 < 0) {
                i++;
            }
            s.table = &D_8009B400[d[i]];
            if (j != 0) {
                s.table->x = 0x50;
            } else {
                s.table->x = 0x209;
            }
            if (d[i] == 1) {
                s.x = i * 20 + 3;
            } else {
                s.x = i * 20;
            }
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
        if (arg0 < 0) {
            break;
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        SetTile(tile);
        tile->r0 = 0xFF;
        tile->g0 = 0x10;
        tile->b0 = 0x10;
        tile->x0 = 0x209 - j * 0x1C1;
        tile->y0 = 0x24;
        tile->w = 0x30;
        tile->h = 1;
        SetSemiTrans(tile, 0);
        AddPrim(g_gpu_ot_ptr + arg3, tile);
        tile++;
        s.y = 0x24;
        s.header = &D_8009B398[2];
        s.table = &D_8009B458[j + 1][0];
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        s.header = &D_8009B398[3];
        s.table = &D_8009B458[j + 1][1];
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        if (arg0 < 0) {
            break;
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B398[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

s32 func_8005E51C(s32 a0, s32 a1, s32 a2) {
    return func_8005E098(-1, a0 - 1, a1, a2);
}

extern Unk8009B0E0Record D_8009ADB4;
extern Unk8009B400Record D_8009ADC0[3];
extern Unk8009B400Record UesrWorkDef[][3];
extern Unk8009B0E0Record D_8009B4B0;
extern Unk8009B400Record D_8009B4BC[5];
extern Unk8009B0E0Record D_8009B4E4;
extern Unk8009B0E0Record D_8009B4F0;
extern Unk8009B400Record D_8009B4FC;
extern Unk8009B400Record D_8009B504;
extern Unk8009B400Record D_8009B50C;
extern Unk8009B400Record D_8009B514;
extern Unk8009B400Record D_8009B51C;
extern Unk8009B0E0Record D_8009B524;
extern Unk8009B0E0Record D_8009B530;
extern Unk8009B0E0Record D_8009B53C;
extern Unk8009B0E0Record D_8009B548;
extern Unk8009B400Record D_8009B554[3];
extern Unk8009B400Record D_8009B56C[2];
extern Unk8009B400Record D_8009B57C[2];
extern u8 D_8009B58C[];
extern u8 D_800A3270[];

s32 func_8005E54C(u32 arg0, s32 arg1, s32 arg2) {
    Unk8005E54CRec *chunk = (Unk8005E54CRec *)arg1;
    /* The per-player points pair: each round's points in the round rows,
       then the per-player totals under them. The target addresses both
       through the one frame slot sp+0x18 (a separate totals array does not
       match). */
    s16 points[2];
    s16 wins[2];
    Unk8007352CEnv s;
    /* FAKE: frame layout: unused; keeps the 8 untouched bytes at sp+0x58 where
     * the siblings func_8005D814 and func_8005F1C8 keep a real s16[3] digit
     * array (Q35, no-new-park-categories phantom-frame-slot pad family).
     * Removed: score 47 */
    volatile s16 digit[3];
    TILE *tile;
    s32 cur;
    s32 ft4;
    DR_MODE *mode_off;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk8005E54CRec): score 75. */
    s32 end_off;
    /* FAKE: i counts the players (first loop) and then the rounds; j is the
       player and k the mark; each phase restarts them as plain loop indices,
       the counter reuse of func_8005E098 / func_8005F1C8. Separate counters
       per phase (a round counter of its own): score 8. */
    s16 i;
    s16 j;
    s16 k;
    s16 c;
    s16 y;

    tile = chunk->unk_00;
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    cur = arg1 + sizeof(chunk->unk_00);
    ft4 = (s32)chunk->unk_898;
    mode_off = &chunk->unk_BB8;
    end_off = arg1 + sizeof(Unk8005E54CRec);
    s.ot_idx = arg2;
    for (i = 0; i < 2; i++) {
        if (!(D_8009BD24.unk14_15 >> i & 1)) {
            s.header = &D_8009B524;
        } else {
            s.header = &D_8009B53C;
        }
        s.x = i * 320;
        s.table = D_8009B554;
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        if (!(D_8009BD24.unk14_15 >> i & 1)) {
            s.header = &D_8009B530;
            s.table = D_8009B56C;
        } else {
            s.header = &D_8009B548;
            s.table = D_8009B57C;
        }
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }

    s.semi = 0;
    s.has_color = 0;
    for (i = 0; i < D_8009BD24.unk14_10 + 3; i++) {
        points[0] = (arg0 >> (i * 4)) & 3;
        points[1] = (arg0 >> (i * 4 + 2)) & 3;
        if (D_8009BD24.unk14_10 == 2) {
            s.y = i * 24 + 0x44;
        } else if (D_8009BD24.unk14_10 == 1) {
            s.y = i * 24 + 0x4F;
        } else {
            s.y = i * 34 + 0x4F;
        }
        if (points[0] == 3 || points[1] == 3) {
            s.header = &D_8009B4B0;
            s.x = 0;
            s.y += 2;
            s.table = &D_8009B4BC[D_800A3270[i]];
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        } else {
            s.header = &D_8009B4E4;
            s.x = 0;
            s.table = &D_8009B514;
            s.sprt_out = cur;
            cur = func_8007352C(&s);
            for (j = 0; j < 2; j++) {
                s.x = j * 70;
                /* FAKE: the other player's points read through a selected
                   address; points[j ^ 1] (both sites): score 110. */
                if (points[j] > *(j ? &points[0] : &points[1])) {
                    s.table = &D_8009B4FC;
                } else if (points[j] < *(j ? &points[0] : &points[1])) {
                    s.table = &D_8009B504;
                } else {
                    s.table = &D_8009B50C;
                }
                s.sprt_out = cur;
                cur = func_8007352C(&s);
            }
            s.y += 5;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B4F0;
                s.table = &D_8009B51C;
                for (k = 0; k < points[j]; k++) {
                    if (j) {
                        s.x = k * 16 + 0x179;
                    } else {
                        s.x = (1 - k) * 16 + 0xE2;
                    }
                    s.sprt_out = cur;
                    cur = func_8007352C(&s);
                }
            }
        }
    }

    s.header = &D_8009B4E4;
    s.x = 0;
    if (D_8009BD24.unk14_10 == 2) {
        y = 0xC6;
    } else if (D_8009BD24.unk14_10 == 1) {
        y = 0xC2;
    } else {
        y = 0xBE;
    }
    s.y = y + 3;
    s.table = &D_8009B514;
    s.sprt_out = cur;
    cur = func_8007352C(&s);
    /* FAKE: one 32-bit store clears the whole pair (target `sw
     * $zero,0x18($sp)`; Q36, no-new-park-categories); `points[0] = points[1] =
     * 0`: score 2 */
    *(s32 *)points = 0;
    for (i = 0; i < D_8009BD24.unk14_10 + 3; i++) {
        if (((arg0 >> (i * 4)) & 3) != 3) {
            points[0] += (arg0 >> (i * 4)) & 3;
        }
        if (((arg0 >> (i * 4 + 2)) & 3) != 3) {
            points[1] += (arg0 >> (i * 4 + 2)) & 3;
        }
    }
    for (j = 0; j < 2; j++) {
        s.header = &D_8009B4F0;
        s.table = &D_8009B51C;
        for (k = 0; k < points[j]; k++) {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;
            } else {
                s.x = 0xF2 - (k >> 1) * 20;
            }
            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B524, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    /* The second DR_MODE goes at the chunk's end, past the size this function
     * returns (2B344 func_8003C560 advances its cursor g_prim_buf_cursor by
     * that size; 6CF8 func_800174F4 ignores it). */
    mode_off++;

    s.ot_idx = arg2;
    wins[0] = wins[1] = 0;
    for (i = 0; i < D_8009BD24.unk14_10 + 3; i++) {
        s.header = &D_8009ADB4;
        s.semi = 0;
        points[0] = (arg0 >> (i * 4)) & 3;
        points[1] = (arg0 >> (i * 4 + 2)) & 3;
        s.col_r = s.col_g = s.col_b = 0x40;
        for (j = 0; j < 2; j++) {
            /* FAKE: the other player's points, as above; points[j ^ 1] here:
             * score 59. */
            if (points[j] <= *(j ? &points[0] : &points[1])) {
                if (points[j] != 3) {
                    s.has_color = 1;
                } else {
                    s.has_color = 0;
                }
            } else {
                if (points[j] != 3) {
                    wins[j]++;
                }
                s.has_color = 0;
            }
            c = D_8009BD24.unk00[j][i].chr;
            if (c >= 12) {
                c -= 2;
            }
            s.table = UesrWorkDef[c];
            s.x = j * 320 + D_8009B58C[c];
            if (D_8009BD24.unk14_10 == 2) {
                s.y = i * 24 - 8;
            } else if (D_8009BD24.unk14_10 == 1) {
                s.y = i * 24 + 3;
            } else {
                s.y = i * 34 + 3;
            }
            s.sprt_out = cur;
            cur = func_8007352C(&s);
            if (D_8009BD24.unk00[j][i].chr == 8) {
                s.table = D_8009ADC0;
                s.sprt_out = cur;
                cur = func_8007352C(&s);
            }
        }
        s.has_color = 0;
        if (points[0] == 3) {
            s.y += 0x4C;
            s.scale_x = 0x100;
            s.x = 0;
            s.semi = 0;
            s.scale_y = 0x400;
            s.y += 6;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B398[j + 2];
                s.table = D_8009B490[j];
                s.ft4_out = ft4;
                ft4 = func_80073728(&s, 0);
                s.table = &D_8009B490[j][1];
                s.ft4_out = ft4;
                ft4 = func_80073728(&s, 0);
            }
        }
    }

    s.header = &D_8009B398[0];
    s.semi = 0;
    if (D_8009BD24.unk14_10 == 2) {
        s.y = 0xC9;
    } else if (D_8009BD24.unk14_10 == 1) {
        s.y = 0xC5;
    } else {
        s.y = 0xC1;
    }
    for (j = 0; j < 2; j++) {
        s.x = j * 70 + 0x113;
        if (wins[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[wins[j]];
        s.table->x = s.table->y = 0;
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }

    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 8;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 0x19D;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    /* FAKE: each arm sets the whole (x0, y0) position: the target stores x0
       once per arm (0x8005F0E0, 0x8005F0F8, 0x8005F104); one x0 store above the
       if/else: score 10. */
    if (D_8009BD24.unk14_10 == 2) {
        tile->x0 = 0x5E;
        tile->y0 = 0xC1;
    } else if (D_8009BD24.unk14_10 == 1) {
        tile->x0 = 0x5E;
        tile->y0 = 0xBD;
    } else {
        tile->x0 = 0x5E;
        tile->y0 = 0xB9;
    }
    tile->w = 0x1C5;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009ADB4, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    return end_off - arg1;
}

extern Unk8009B0E0Record D_8009B5A0[2];
extern Unk8009B400Record D_8009B5B8[2][2];
extern Unk8009B400Record D_8009B5D8[2];
extern Unk8009B400Record D_8009B5E8;

s32 func_8005F1C8(Unk8001CD68Rec *arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *chunk = (Unk8005D814Rec *)arg2;
    Unk8007352CEnv s;
    s16 d[3];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk8005D814Rec): score 97. */
    s32 end_off;
    /* FAKE: i/row count the win-mark pips' players and rows; k and j are reused
     * as plain loop indices by the later phases (tile strip k/j, timer j/k),
     * the same counter reuse as func_8005E098 and the func_8003800C
     * single-counter shape. Separate counters for the later phases: score 11.
     */
    s16 i;
    s16 j;
    s16 k;
    s16 row;
    s16 wins;
    s16 count;
    s32 x;

    s.has_color = 0;
    s.y = 0;
    s.semi = 0;
    tile = chunk->unk_00;
    cur = arg2 + sizeof(chunk->unk_00);
    mode_off = &chunk->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    s.ot_idx = arg3;
    for (i = 0; i < 2; i++) {
        s.header = &D_8009B5A0[i];
        wins = (arg1 >> (i * 8)) & 0xFF;
        if (i != 0) {
            count = 2;
        } else {
            count = D_8009BD24.unk14_14 + 1;
        }
        for (row = 0; row < 2; row++) {
            for (k = 0; k < count; k++) {
                if (row != 0) {
                    s.x = i * 8 + 0x1C2 - (0x1C - i * 8) * k;
                } else {
                    s.x = (0x1C - i * 8) * k;
                }
                s.table = &D_8009B5B8[i][0];
                if (k >= ((wins >> (row * 4)) & 0xF)) {
                    s.table++;
                }
                s.sprt_out = cur;
                cur = func_8007352C(&s);
            }
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B5A0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    /* The second DR_MODE goes at the chunk's end, past the size this function
     * returns (its caller, 9F9C func_8001CE60, advances its cursor
     * g_prim_buf_cursor by that size). */
    mode_off++;

    s.has_color = 0;
    s.y = 0;
    s.semi = 0;
    s.header = &D_8009B398[1];
    s.ot_idx = arg3;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            s.x = j * 550;
            s.table = &D_8009B5D8[k];
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = 0xFF;
            tile->g0 = 0x10;
            tile->b0 = 0x10;
            tile->x0 = 0x48 + j * 431 + j * (k << 4);
            tile->y0 = k * 20 + 0x24;
            tile->w = 0x42 - k * 16;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
            s.y = k * 20 + 0x24;
            s.header = &D_8009B398[2];
            s.table = &D_8009B5F0[j][0];
            s.sprt_out = cur;
            cur = func_8007352C(&s);
            s.header = &D_8009B398[3];
            s.table = &D_8009B5F0[j][1];
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.y = 0x16;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        for (k = 0; k < 3; k++) {
            switch (j) {
            case 0:
                if (k < 2 || D_8009BD24.unk14_12 == 2) {
                    d[k] = arg0->unk_2;
                    if (k == 0 && D_8009BD24.unk14_12 == 2) {
                        d[k] = d[k] / 100;
                    } else if (k == 0 || (k == 1 && D_8009BD24.unk14_12 == 2)) {
                        d[k] = d[k] / 10;
                    }
                    d[k] = d[k] % 10;
                    s.table = &D_8009B400[d[k]];
                    if (d[k] == 1) {
                        s.x = k * 20 + 3;
                    } else {
                        s.x = k * 20;
                    }
                }
                break;
            case 1:
                if (k < 2) {
                    d[k] = arg0->unk_3;
                    if (k != 0) {
                        d[k] = d[k] % 10;
                    } else {
                        d[k] = d[k] / 10 % 10;
                    }
                    x = (D_8009BD24.unk14_12 == 2) ? k * 20 + 0x48
                                                   : k * 20 + 0x34;
                    s.table = &D_8009B400[d[k]];
                    if (d[k] == 1) {
                        s.x = x + 3;
                    } else {
                        s.x = x;
                    }
                }
                break;
            }
            if (D_8009BD24.unk14_12 == 2) {
                s.table->x = 0x109;
            } else {
                s.table->x = 0x113;
            }
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
        s.table = &D_8009B5E8;
        if (D_8009BD24.unk14_12 == 2) {
            s.x = j * 6 + 0x145;
        } else {
            s.x = j * 6 + 0x13B;
        }
        s.sprt_out = cur;
        cur = func_8007352C(&s);
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(&D_8009B398[1], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

/* func_8005FA98's sheets: three headers at 0x8009B610 (the second draw, header
 * arg0, its one cell at 0x8009B634) and three at 0x8009B63C (counts 2 / 1 / 4)
 * whose cells follow at 0x8009B660 (0-1, 2, 3-6). */
extern Unk8009B0E0Record D_8009B610[3];
extern Unk8009B400Record D_8009B634;
extern Unk8009B0E0Record D_8009B63C[3];
extern Unk8009B400Record D_8009B660[7];

s32 func_8005FA98(s32 arg0, s32 arg1, s32 arg2) {
    Unk8007352CEnv s;
    /* FAKE: holder of the first walk's cursor until the second descriptor is
       set; stored directly: score 11. */
    s32 ret;
    /* FAKE: copy of arg1 for the first cursor; arg1 directly: score 2. */
    s32 start = arg1;
    /* FAKE: the returned size as end minus start; 0x190: score 12. */
    s32 end = arg1 + 0x190;

    s.scale_x = 0x200;
    s.scale_y = 0x100;
    s.header = &D_8009B63C[arg0];
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg2;
    switch (arg0) {
    case 0:
        s.table = &D_8009B660[0];
        break;
    case 1:
        s.table = &D_8009B660[2];
        break;
    case 2:
        s.table = &D_8009B660[3];
        break;
    }
    s.ft4_out = start;
    ret = func_80073728(&s, 0);
    s.header = &D_8009B610[arg0];
    s.table = &D_8009B634;
    s.ft4_out = ret;
    func_80073728(&s, 0);
    return end - arg1;
}

extern RECT D_800A327C;
extern RECT D_800A3284;
extern s32 D_800A3278;

void func_8005FBC8(s32 arg0, u8 *arg1) {
    RECT r1, r2;
    s32 s0;
    s0 = func_80036EA8(2, arg0 + 0x33);
    cdrom_StartRead(s0, (s32)arg1);
    game_FrameLoop();
    cdrom_GetFileSize(s0);
    r1 = D_800A327C;
    r2 = D_800A3284;
    LoadImage(&r1, (u32 *)(arg1 + 0x40));
    DrawSync(0);
    LoadImage(&r2, (u32 *)(arg1 + 0x14));
    DrawSync(0);
    D_800A3278 = 0;
}

extern Unk8009B0E0Record D_8009B698[2];
extern Unk8009B400Record D_8009B6B0[8];

s32 func_8005FC9C(s32 arg0, s32 arg1) {
    Unk8005FC9CRec *chunk = (Unk8005FC9CRec *)arg0;
    Unk8007352CEnv s;
    RECT r;
    RECT *clip;
    GpuDb *env;
    s32 cur;
    DR_MODE *mode_off;
    POLY_G4 *poly;
    DR_AREA *area;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk8005FC9CRec): score 55. */
    s32 end_off;
    s16 j;
    s16 i;
    s16 off;
    s16 x;
    u8 c;

    cur = arg0;
    mode_off = &chunk->unk_280;
    poly = chunk->unk_28C;
    area = chunk->unk_2D4;
    end_off = arg0 + sizeof(Unk8005FC9CRec);
    j = 0;
    env = &g_gpu_db[D_800A36AC & 1];
    r.x = env->draw.clip.x;
    r.y = env->draw.clip.y;
    r.w = env->draw.clip.w;
    r.h = env->draw.clip.h;
    clip = &env->draw.clip;
    SetDrawArea(area, &r);
    AddPrim(g_gpu_ot_ptr + arg1, area);
    area++;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.ot_idx = arg1;
    s.table = D_8009B6B0;
    off = (D_800A3278 - 0xB4) * 24;
    do {
        if (D_800A3278 >= 0xB5) {
            SetPolyG4(poly);
            SetSemiTrans(poly, 1);
            if (j != 0) {
                x = off + 0x140;
                r.x = clip->x + x;
                r.y = clip->y;
                r.w = clip->w / 2 - off;
                r.h = clip->h;
                poly->x0 = x;
                poly->y0 = 0;
                poly->x1 = x;
                poly->y1 = 0xF0;
                poly->x2 = off + 0x154;
                poly->y2 = 0;
                poly->x3 = off + 0x154;
                poly->y3 = 0xF0;
            } else {
                r.x = clip->x;
                r.y = clip->y;
                r.w = clip->w / 2 - off;
                r.h = clip->h;
                poly->x0 = 0x140 - off;
                poly->y0 = 0;
                poly->x1 = 0x140 - off;
                poly->y1 = 0xF0;
                poly->x2 = 0x12C - off;
                poly->y2 = 0;
                poly->x3 = 0x12C - off;
                poly->y3 = 0xF0;
            }
            c = ~((off * 255) / 320);
            poly->r0 = c;
            poly->g0 = 0;
            poly->b0 = 0;
            poly->r1 = c;
            poly->g1 = 0;
            poly->b1 = 0;
            poly->r2 = 0;
            poly->g2 = 0;
            poly->b2 = 0;
            poly->r3 = 0;
            poly->g3 = 0;
            poly->b3 = 0;
            AddPrim(g_gpu_ot_ptr + arg1, poly);
            poly++;
        }
        for (i = 0; i < 2; i++) {
            s.header = &D_8009B698[i];
            s.y = i << 6;
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
        if (D_800A3278 >= 0xB5) {
            SetDrawArea(area, &r);
            AddPrim(g_gpu_ot_ptr + arg1, area);
            area++;
        }
        j++;
    } while (j < 2);
    SetDrawMode(mode_off, 1, 0, func_8006E480(D_8009B698, 0x20), 0);
    AddPrim(g_gpu_ot_ptr + arg1, mode_off);
    if (off <= 0x140) {
        D_800A3278++;
    }
    return end_off - arg0;
}

/* func_800600C8's sheets: the frame header (3 cells, D_8009B758) and the digit
 * header (1 cell; one of the ten digit cells D_8009B708). */
extern Unk8009B0E0Record D_8009B6F0[2];
extern Unk8009B400Record D_8009B708[10];
extern Unk8009B400Record D_8009B758[3];

s32 func_800600C8(s32 arg0, s32 arg1, s32 arg2) {
    Unk8007352CEnv s;
    s16 d[2];
    Unk800600C8Rec *chunk = (Unk800600C8Rec *)arg1;
    DR_MODE *mode_off = &chunk->unk_B4;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk800600C8Rec): score 20. */
    s32 end_off = arg1 + sizeof(Unk800600C8Rec);
    s32 cur = arg1;
    s32 i;

    s.header = &D_8009B6F0[0];
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    s.ot_idx = arg2;
    if (arg0 < 0xA) {
        s.x = 0x93;
    } else {
        s.x = 0xA3;
    }
    s.table = D_8009B758;
    s.sprt_out = cur;
    cur = func_8007352C(&s);
    s.header = &D_8009B6F0[1];
    d[0] = d[1] = arg0;
    d[1] = d[1] / 10 % 10;
    d[0] = d[0] % 10;
    for (i = 0; i < 2; i++) {
        s.table = &D_8009B708[d[i]];
        if (arg0 < 0xA) {
            s.x = 0x64;
        } else {
            s.x = (((1 - i) << 2) << 3) + 0x54;
        }
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        if (d[1] == 0) {
            break;
        }
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480(D_8009B6F0, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    return end_off - arg1;
}

extern RECT D_800A3294;
extern RECT D_800A329C;
extern RECT D_800A32A4;
extern RECT D_800A32AC;

void func_800602AC(s32 arg0, s32 *arg1) {
    RECT r1, r2, r3, r4;
    s32 s1;
    u8 *p;
    u8 *q;
    s1 = func_80036EA8(2, arg0 + 0x3D);
    cdrom_StartRead(s1, (s32)arg1);
    game_FrameLoop();
    cdrom_GetFileSize(s1);
    arg1[0] = arg1[0] + (s32)arg1;
    arg1[1] = arg1[1] + (s32)arg1;
    r1 = D_800A3294;
    r2 = D_800A329C;
    p = (u8 *)arg1[0];
    LoadImage(&r1, (u32 *)(p + 0x40));
    DrawSync(0);
    LoadImage(&r2, (u32 *)(p + 0x14));
    DrawSync(0);
    r3 = D_800A32A4;
    r4 = D_800A32AC;
    q = (u8 *)arg1[1];
    LoadImage(&r3, (u32 *)(q + 0x60));
    DrawSync(0);
    LoadImage(&r4, (u32 *)(q + 0x14));
    DrawSync(0);
}

extern Unk8009B0E0Record D_8009B7AC[3];
/* packed screen positions: x = (v >> 7) + 0x37, y = (v & 0x7F) + 0x2A */
extern u16 D_8009B850[];

s32 func_80060414(s16 arg0, s32 arg1, s32 arg2) {
    Unk8007352CEnv s;
    Unk80060414Rec *chunk = (Unk80060414Rec *)arg1;
    DR_MODE *mode_off;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk80060414Rec): score 22. */
    s32 end_off;
    mode_off = &chunk->unk_14;
    end_off = arg1 + sizeof(Unk80060414Rec);
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = arg2;
    s.x = (D_8009B850[arg0 & 0x7FFF] >> 7) + 0x37;
    s.y = (D_8009B850[arg0 & 0x7FFF] & 0x7F) + 0x2A;
    if (arg0 & 0x8000) {
        s.header = &D_8009B7AC[0];
    } else if (D_8009BD24.unk00[0][0].chr < 0xC) {
        s.header = &D_8009B7AC[1];
    } else {
        s.header = &D_8009B7AC[2];
    }
    s.table = &D_800A328C;
    s.sprt_out = (s32)&chunk->unk_00;
    func_8007352C(&s);
    SetDrawMode(mode_off, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    return end_off - arg1;
}

extern Unk8009B0E0Record D_8009B770[4];
extern Unk8009B0E0Record D_8009B7A0;
extern Unk8009B400Record D_8009B7D0;
extern Unk8009B400Record D_8009B7D8[5];
extern Unk8009B400Record D_8009B800[4];
extern Unk8009B400Record D_8009B820[4];
extern Unk8009B400Record D_8009B840[2];

s32 func_80060544(s32 arg0, s32 arg1) {
    Unk80060544Rec *chunk = (Unk80060544Rec *)arg0;
    Unk8007352CEnv s;
    /* FAKE: the returned size is the chunk's end minus its start;
     * sizeof(Unk80060544Rec): score 25. */
    s32 end_off;
    s32 ft4;
    s32 i;
    s32 j;
    s32 cur;
    Unk8009B0E0Record *p0;
    Unk8009B400Record *p1;
    DR_MODE *mode_off;
    cur = arg0;
    ft4 = (s32)chunk->unk_4EC;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = arg1;
    mode_off = &chunk->unk_5DC;
    end_off = arg0 + sizeof(Unk80060544Rec);
    s.scale_x = 0x200;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    i = 0;
    do {
        s.header = &D_8009B770[i];
        switch (i) {
        case 0:
            s.table = D_8009B7D8;
            break;
        case 1:
        case 2:
            s.table = D_8009B800;
            break;
        case 3:
            s.table = &D_8009B7D0;
            s.ft4_out = ft4;
            ft4 = func_80073728(&s, 0);
            break;
        }
        if (i != 3) {
            s.sprt_out = cur;
            cur = func_8007352C(&s);
        }
        i += 1;
    } while (i < 4);
    s.header = &D_8009B7A0;
    s.table = D_8009B820;
    s.sprt_out = cur;
    cur = func_8007352C(&s);
    j = 0;
    p1 = D_8009B840;
    p0 = &D_8009B398[2];
    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    do {
        s.header = p0;
        s.table = p1;
        s.sprt_out = cur;
        cur = func_8007352C(&s);
        p1++;
        j += 1;
        p0++;
    } while (j < 2);
    SetDrawMode(mode_off, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg1, mode_off);
    return end_off - arg0;
}

extern u16 D_800A32B6;
extern u16 D_800A32B4;

void func_80060758(void) {
    D_800A32B6 = 0;
    D_800A32B4 = 0;
}

extern Unk8006D808Set D_8009B0C0;

s32 func_80060768(s32 arg0, s32 arg1, s32 arg2) {
    s32 sp18;
    s32 sp1C;
    s32 t1;
    s32 t2;
    u16 cur1;
    u16 cur2;
    /* FAKE: the returned size is the chunk's end minus its start; 0xAC8:
     * score 26. */
    s32 end_off;
    s32 tile_off;

    tile_off = arg0 + 0x7D0;
    end_off = arg0 + 0xAC8;
    sp18 = arg0;
    sp1C = arg0 + 0x870;
    func_8006D808(&sp18, &sp1C, &D_8009B0C0, arg1, arg2);
    if ((u32)arg2 < 3U) {
        SetTile((TILE *)tile_off);
        *(u8 *)(arg0 + 0x7D4) = 0xFF;
        *(s16 *)(arg0 + 0x7D8) = 0x6A;
        *(u8 *)(arg0 + 0x7D5) = 0;
        *(u8 *)(arg0 + 0x7D6) = 0;
        *(s16 *)(arg0 + 0x7DA) = (s16)(arg2 * 0x1A + 0x5B);
        cur1 = D_800A32B4;
        /* FAKE: increment staged through t1 (stored next line, then reused for
         * the product): the reassignment breaks cse's mem==reg equivalence, so
         * the clamp re-read emits lh (staged-value-reused-variable); direct
         * increment: score 8 (with t2's: 16) */
        t1 = cur1 + 1;
        D_800A32B4 = t1;
        t1 = (s32)((s16)cur1) * 0x1AA;
        *(s16 *)(arg0 + 0x7DE) = 2;
        *(s16 *)(arg0 + 0x7DC) = (s16)(t1 / 0x1E);
        if ((s16)D_800A32B4 >= 0x1F) {
            D_800A32B4 = 0x1E;
        }
        SetSemiTrans((TILE *)tile_off, 0);
        AddPrim(g_gpu_ot_ptr + arg1, (void *)tile_off);
        tile_off = arg0 + 0x7E0;
    }
    SetTile((TILE *)tile_off);
    *(u8 *)(tile_off + 4) = 0xFF;
    *(s16 *)(tile_off + 8) = 0x9E;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xA) = 0xBD;
    cur2 = D_800A32B6;
    /* FAKE: increment staged through t2, as t1 above: the clamp re-read emits
     * lh (staged-value-reused-variable); direct increment: score 8 */
    t2 = cur2 + 1;
    D_800A32B6 = t2;
    t2 = (s32)((s16)cur2) * 0x144;
    *(s16 *)(tile_off + 0xE) = 2;
    *(s16 *)(tile_off + 0xC) = (s16)(t2 / 0x1E);
    if ((s16)D_800A32B6 >= 0x1F) {
        D_800A32B6 = 0x1E;
    }
    SetSemiTrans((TILE *)tile_off, 0);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)tile_off);
    tile_off += 0x10;

    SetTile((TILE *)tile_off);
    *(s16 *)(tile_off + 8) = 0x3F;
    *(s16 *)(tile_off + 0xA) = 0x2D;
    *(s16 *)(tile_off + 0xC) = 0x202;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x6C;
    SetSemiTrans((TILE *)tile_off, 1);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)tile_off);
    tile_off += 0x10;

    SetTile((TILE *)tile_off);
    *(s16 *)(tile_off + 8) = 0x92;
    *(s16 *)(tile_off + 0xA) = 0xAA;
    *(s16 *)(tile_off + 0xC) = 0x15C;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x1A;
    SetSemiTrans((TILE *)tile_off, 1);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)tile_off);

    SetDrawMode((void *)sp1C, 1, 0, 0, 0);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)sp1C);
    sp1C += 0xC;
    return end_off - arg0;
}

/* This file's initialized small data (.sdata), in address order (Q65). */
/* "NULL" tag func_8005490C compares the motion streams against (gp-relative):
 * size from the blob label */
s32 D_800A3250[2] = {0x4c4c554e, 0};
PadBitTable D_800A3258 = {{0xd, 0xf, 0xc, 0xe}};
u8 D_800A325C[4] = {1, 0, 2, 3};
u8 D_800A3260[4] = {5, 4, 6, 7};
/* named by the pointer word at 0x8009B0D8; this file's by layout, between its
 * gp-reached objects (Q80): size from the blob label */
s32 D_800A3264[2] = {0x4e00b3, 0xc180000};
s32 D_800A326C = 0;
u8 D_800A3270[8] = {0, 1, 2, 3, 4, 0, 0, 0};
s32 D_800A3278 = 0;
RECT D_800A327C = {0x380, 0, 0x40, 0x100};
RECT D_800A3284 = {0x3E0, 0x1FF, 0x10, 1};
Unk8009B400Record D_800A328C = {0, 0, 0, 0, 0x14, 0xE};
RECT D_800A3294 = {0x3C0, 0x180, 0x40, 0x16};
RECT D_800A329C = {0x3C0, 0x1FF, 0x10, 1};
RECT D_800A32A4 = {0x380, 0x17F, 0x40, 0x7F};
RECT D_800A32AC = {0x380, 0x1FF, 0x20, 1};
u16 D_800A32B4 = 0;
u16 D_800A32B6 = 0;
