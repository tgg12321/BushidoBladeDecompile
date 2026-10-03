/* Rotation-matrix and colour math (math_RotMatrixZXY / YXZ / XYZ, math_Rotate2D, math_RgbToHsv,
 * ...) and the primitive texture-offset helpers (gpu_OffsetTexPolyFT3..gpu_OffsetClut). .text
 * 0x80042504 (ROM 0x32D04). Start boundary: G8 (the -G8 run ends). */
#include "common.h"
#include "gpu.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"
#include "gte.h"



extern u8 D_800A9D10;
extern void func_80049E1C(void);
extern void func_80052C10(void);



extern s32 D_800A3244;
extern s16 D_800963EE;



extern void func_80041430(s32, s32);
extern s32 func_8004019C(s32 *, s32);
/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */












extern s32 func_800486FC(s32 *);

void func_80042504(s32 *hsv, s32 *rgb) {
    s32 h = hsv[0];
    s32 s = hsv[1];
    s32 v = hsv[2];
    s32 i, f, p, q, t;
    s32 r, g, b;

    if (func_800486FC(hsv)) {
        s = 0;
    }
    if (s == 0) {
        r = v;
        g = v;
        b = v;
        goto out;
    }
    if (h == 0x1000) {
        h = 0;
    }
    p = v * (0x1000 - s);
    p = p >> 12;
    h = h * 6;
    f = h & 0xFFF;
    i = h >> 12;
    q = v * (0x1000 - ((s * f) >> 12));
    q = q >> 12;
    t = v * (0x1000 - ((s * (0x1000 - f)) >> 12));
    t = t >> 12;

    if ((u32)i >= 6U) goto out;
    switch (i) {
    case 0: r = v; g = t; b = p; goto out;
    case 1: r = q; g = v; b = p; goto out;
    case 2: r = p; g = v; b = t; goto out;
    case 3: r = p; g = q; b = v; goto out;
    case 4: r = t; g = p; b = v; goto out;
    case 5: r = v; g = p; b = q; goto out;
    }
out:
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}
/* RGB -> HSV (4.12 fixed point). Inverse of rob_life_ctrl_2 above.
 * Outputs a1[] = { hue, sat, val }; val (V) = max(r,g,b) is the third
 * output channel, held separately from the max used for the chroma
 * deltas / max-channel compares. */
void math_RgbToHsv(s32 *a0, s32 *a1) {
    s32 r, g, b;
    s32 max_val, min_val;
    s32 chroma;
    s32 sat;
    s32 hue;
    s32 dR, dG, dB;
    s32 val;

    r = a0[0];
    g = a0[1];
    b = a0[2];

    /* find max */
    if (r >= g) {
        if (r >= b) {
            max_val = r;
        } else {
            max_val = b;
        }
    } else {
        if (g >= b) {
            max_val = g;
        } else {
            max_val = b;
        }
    }

    /* find min */
    if (g >= r) {
        if (b >= r) {
            min_val = r;
        } else {
            min_val = b;
        }
    } else {
        if (b >= g) {
            min_val = g;
        } else {
            min_val = b;
        }
    }

    chroma = max_val - min_val;
    /* V = max(r,g,b): the value channel, third HSV output */
    val = max_val;

    if (val != 0) {
        sat = (chroma << 12) / val;
    } else {
        sat = 0;
    }

    if (sat != 0) {
        dR = ((max_val - r) << 12) / chroma;
        dG = ((max_val - g) << 12) / chroma;
        dB = ((max_val - b) << 12) / chroma;

        if (r == max_val) {
            hue = dB - dG;
        } else if (g == max_val) {
            s32 tmp = dB - 0x2000;
            hue = dR - tmp;
        } else if (b == max_val) {
            s32 tmp = dR - 0x4000;
            hue = dG - tmp;
        }

        hue /= 6;

        if (hue < 0) {
            hue += 0x1000;
        }
    } else {
        hue = 0;
    }

    a1[0] = hue;
    a1[1] = sat;
    a1[2] = val;
}
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
extern s16 Judge[];
void math_RotMatrixZXY(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 negSinAxsinB_12;
    s32 prod_sinC, cosB_cosC;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 cosA_negSinC;
    s32 prod2_sinC, sinB_cosC;
    s32 sinAxsinB;
    s32 sinAxsinB_12;
    s32 sinAxsinB_12_cosC;
    s32 cosB_sinC;
    s32 negSinA_cosB;
    s32 cosA_cosC;
    s32 negSinAxcosB_12;
    s32 negSinAxcosB_12_cosC;
    s32 cosA_negSinB;
    s32 cosA_cosB;
    s32 sinB_sinC;
    s32 cosA;
    s32 angC;
    s32 idxB;
    u16 rawA;

    angA = a0[0];
    angB = a0[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];

    idxB = (s16)angB + 0x400;

    negSinAxsinB_12 = (sinA * -sinB) >> 12;

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = negSinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;

    sinAxcosB = sinA * cosB;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[7] = sinA;
    cosA = (s16)rawA;

    cosA_negSinC = cosA * -sinC;

    sinAxcosB_12 = sinAxcosB >> 12;
    prod2_sinC = sinAxcosB_12 * sinC;

    sinAxsinB = sinA * sinB;
    sinB_cosC = sinB * cosC;

    sinAxsinB_12 = sinAxsinB >> 12;
    sinAxsinB_12_cosC = sinAxsinB_12 * cosC;

    cosB_sinC = cosB * sinC;

    negSinA_cosB = -sinA * cosB;

    cosA_cosC = cosA * cosC;

    negSinAxcosB_12 = negSinA_cosB >> 12;
    negSinAxcosB_12_cosC = negSinAxcosB_12 * cosC;

    cosA_negSinB = cosA * -sinB;

    cosA_cosB = cosA * cosB;

    a1[1] = cosA_negSinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[6] = cosA_negSinB >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[2] = (prod2_sinC + sinB_cosC) >> 12;
    a1[3] = (sinAxsinB_12_cosC + cosB_sinC) >> 12;

    sinB_sinC = sinB * sinC;
    a1[5] = (negSinAxcosB_12_cosC + sinB_sinC) >> 12;
}
/* Euler angles (a0[0..2], 12-bit) -> 3x3 rotation matrix a1[9], Y-X-Z order.
 *
 * cosA is read through a `u16 rawA` staging local and sign-extended with an
 * explicit (s16) cast, with the `a1[5] = -sinA;` store placed between the load
 * and the cast: combine will not merge a MEM load into a later user across a
 * store, so the target's lhu + sll 16 + sra 16 shape survives (sched1 then
 * hoists the store back out at no instruction cost).  Both halves matter: the
 * store after the cast, or the same interleave without the u16 local, is inert.
 *
 * `angC = a0[2]` is read after the sinA*sinB statement: it is the last use of
 * `a0`, so its position decides where $a0 dies; reading it here gives angC
 * $v1 and its cos-index temp $v0, as in the target.
 */
extern s16 Judge[];
void math_RotMatrixYXZ(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 idxB;
    s32 sinAxsinB_12;
    s32 prod_sinC, prod_cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 cosA_sinB, cosA_sinC, cosA_cosC, cosA_cosB;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 scb_sinC, neg_sinB_cosC;
    s32 scb_cosC;
    s32 sinB_sinC;
    s32 cosA;
    u16 rawA;
    s32 angC;

    angA = a0[0];
    angB = a0[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    idxB = (s16)angB + 0x400;

    sinAxsinB_12 = (sinA * sinB) >> 12;

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = sinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;

    cosB_negsinC = cosB * -sinC;

    cosA_sinB = cosA * sinB;
    cosA_sinC = cosA * sinC;

    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;

    sinAxcosB_12 = sinAxcosB >> 12;

    scb_sinC = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;

    scb_cosC = sinAxcosB_12 * cosC;

    cosA_cosB = cosA * cosB;

    a1[2] = cosA_sinB >> 12;
    a1[3] = cosA_sinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[1] = (prod_cosC + cosB_negsinC) >> 12;
    a1[6] = (scb_sinC + neg_sinB_cosC) >> 12;

    sinB_sinC = sinB * sinC;
    a1[7] = (scb_cosC + sinB_sinC) >> 12;
}
extern s16 Judge[];
void math_RotMatrixXYZ(u16 *a0, s16 *a1) {
    s32 angA, angB, angC;
    s16 sinA, sinB, sinC, cosA;
    s32 cosB, cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 sab, sab12;
    s32 sab12_cosC, sab12_negsinC;
    s32 cosA_sinC, cosA_cosC;
    s32 cosA_negsinB;
    s32 negsinA_cosB;
    s32 cab12, cab12_cosC;
    s32 cosA_sinB;
    s32 sinA_sinC;
    s32 csb12_sinC;
    s32 cosA_cosB;
    s32 sinA_cosC;

    angB = a0[1];
    angC = a0[2];
    sinB = Judge[angB & 0xFFF];
    angA = a0[0];
    sinA = Judge[angA & 0xFFF];
    sinC = Judge[angC & 0xFFF];
    cosA = Judge[((s16)angA + 0x400) & 0xFFF];

    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    a1[2] = sinB;
    sab = sinA * sinB;
    cosB_negsinC = cosB * -sinC;
    sab12 = sab >> 12;
    sab12_cosC = sab12 * cosC;
    cosA_sinC = cosA * sinC;
    sab12_negsinC = sab12 * -sinC;
    cosA_cosC = cosA * cosC;
    cosA_negsinB = cosA * -sinB;
    negsinA_cosB = -sinA * cosB;
    cab12 = cosA_negsinB >> 12;
    cab12_cosC = cab12 * cosC;
    cosA_sinB = cosA * sinB;
    sinA_sinC = sinA * sinC;
    csb12_sinC = (cosA_sinB >> 12) * sinC;
    cosA_cosB = cosA * cosB;

    a1[0] = cosB_cosC >> 12;
    a1[1] = cosB_negsinC >> 12;
    a1[5] = negsinA_cosB >> 12;
    sinA_cosC = sinA * cosC;
    a1[8] = cosA_cosB >> 12;
    a1[3] = (sab12_cosC + cosA_sinC) >> 12;
    a1[4] = (sab12_negsinC + cosA_cosC) >> 12;
    a1[6] = (cab12_cosC + sinA_sinC) >> 12;
    a1[7] = (csb12_sinC + sinA_cosC) >> 12;
}
extern void math_RotMatrixZYX(Unk80101DF0Rot *, Unk80101DF0Mat *);


extern void math_RotMatrixXYZ();
void func_80042E90(void) {
    g_anim_func_table[0] = math_RotMatrixZYX;
    /* the three below are defined above on the angle / matrix element arrays */
    g_anim_func_table[2] = (AnimRotFunc)math_RotMatrixZXY;
    g_anim_func_table[4] = (AnimRotFunc)math_RotMatrixYXZ;
    g_anim_func_table[5] = (AnimRotFunc)math_RotMatrixXYZ;
}
void math_TransposeMatrixInPlace(u16 *a0) {
    /* FAKE: statement staging — saving one
       side of all three pairs up front seats x/y/z in $a1/$v1/$v0 for
       the whole body with the scratch reloads sharing $a2, and keeps
       the load-delay nop at +0x18 unfilled, as in the target. */
    u16 t, x, y, z;
    y = a0[1];
    x = a0[2];
    z = a0[5];
    t = a0[6];
    a0[2] = t;
    a0[6] = x;
    t = a0[3];
    a0[1] = t;
    a0[3] = y;
    t = a0[7];
    a0[5] = t;
    a0[7] = z;
}
extern s16 Judge[];
void math_Rotate2D(s32 *a0, s32 *a1, s32 a2) {
    s16 sin_val, cos_val;
    s32 x, y;
    s32 sin_x, cos_x, sin_y, cos_y;
    sin_val = Judge[(a2 + 0x400) & 0xFFF];
    x = *a0;
    cos_val = Judge[a2 & 0xFFF];
    y = *a1;
    sin_x = sin_val * x;
    cos_x = cos_val * x;
    sin_y = sin_val * y;
    cos_y = cos_val * y;
    *a1 = (cos_x + sin_y) >> 12;
    *a0 = (sin_x - cos_y) >> 12;
}
extern s32 *ApplyMatrix(s32 *, s16 *, s32 *);
extern s16 ratan2(s32, s32);
extern s32 rcos(s32);
extern s32 rsin(s32);
extern void MulMatrix(s32 *, s32 *);
void math_MatrixToAnglesYXZ(s32 *a0, s16 *a1) {
    s16 rot[3];
    s32 result[4];
    s32 sp28[8];
    s16 angle1;
    s32 cos_val, sin_val;
    s16 neg_angle2;
    s32 combined;

    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x1000;
    ApplyMatrix(a0, rot, result);

    angle1 = ratan2(result[0], result[2]);

    cos_val = rcos((s16)angle1);
    sin_val = rsin((s16)angle1);

    combined = (cos_val * result[2] + sin_val * result[0]) >> 12;
    neg_angle2 = -ratan2(result[1], combined);

    rot[0] = -neg_angle2;
    rot[1] = -angle1;
    rot[2] = 0;
    math_RotMatrixXYZ(rot, sp28);

    MulMatrix(sp28, a0);

    rot[0] = 0;
    rot[1] = 0x1000;
    rot[2] = 0;
    ApplyMatrix(sp28, rot, result);

    {
        s16 angle3;
        angle3 = ratan2(result[0], result[1]);
        a1[0] = neg_angle2;
        a1[1] = angle1;
        a1[2] = -angle3;
    }
}

extern void func_8004DDB4(s32, s32, s32, s32);
extern MATRIX D_800FF610;
extern s32 D_800951D8;
extern s32 D_80095280;
extern s32 D_80095328;
extern s32 D_800A3828;
void func_800430E4(s32 arg0, s32 arg1, s16 arg2, u8 *arg3) {
    MATRIX *dst = (MATRIX *)0x1F8003A0;
    s32 t0;

    t0 = *(s32 *)0x1F800008;
    *(s32 *)0x1F800008 = 3 - t0;

    *dst = D_800FF610;

    D_800A3828 = (s32)&D_800F62E0[arg2];

    dst->m[1][0] >>= 1;
    dst->m[1][1] >>= 1;
    dst->m[1][2] >>= 1;
    dst->t[1] >>= 1;

    if (arg3[1] & 1) {
        *(s32 *)0x1F80001C = (s32)&D_80095280;
    } else {
        *(s32 *)0x1F80001C = (s32)&D_800951D8;
    }

    func_8004DDB4(arg0, arg1, (s32)dst, t0);

    *(s32 *)0x1F80001C = (s32)&D_80095328;
}

s32 func_80043244(s32 a0) {
    s32 ret;
    if (a0 > 0x16A09) {
        ret = 3;
    } else if (a0 > (s32)0xB500) {
        ret = 2;
    } else {
        ret = a0 >= 0x5A01;
    }
    return ret;
}
s32 func_80043278(s32 a0) {
    /* FAKE: constant holder for the 0xFFFFF000 mask; with the literal the function is one insn longer (score 4) */
    s32 mask = (s32)0xFFFFF000;
    s32 v0 = a0 >> *(s32 *)0x1F800008;
    s32 a0_new = v0 >> 11;
    v0 = (v0 & 0x7FF) | mask;
    v0 = v0 >> a0_new;
    return v0 & 0xFFF;
}
extern s32 *D_80103608[];
extern u16 D_80103658[];
extern void func_80043454(s16, s16, s16, s16);
void func_800432A0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    u16 arg4_lo = *(u16 *)&arg4;
    s32 idx = arg0;
    u16 *cnt_base = D_80103658;
    u16 *countPtr;
    s16 i;

    countPtr = &cnt_base[idx];
    i = 0;
    if (*countPtr == 0) goto done;
    {
        s32 **base_addr = D_80103608;
        s32 **basePtr = &base_addr[idx];
        u16 *cntPtr = countPtr; /* load-bearing named intermediate: countPtr's
           caller-save home ($a1) must die before the loop's jal; this rebind
           gives the loop reads their own callee-save home ($s1), reproducing
           the target's addu $s1,$a1,$zero copy */
    loop:
        *(s32 *)0x1F800000 = (*basePtr)[(s16)i];
        func_80043454((s16)arg1, (s16)arg2, (s16)arg3, (s16)arg4_lo);
        i++;
        if ((s16)i < *cntPtr) goto loop;
    }
    done:
    ;
}
void func_80043398(s16 a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    func_800432A0(a0, (s16)(a1 << 6), (s16)(a2 << 8), (s16)(a3 << 6), (s16)(a4 << 8));
}
/* Old-style definition: the only caller, func_80045B68, passes every
 * argument as a plain int (no sign-extension at the call site), so no
 * prototype was in scope there; the body narrows each argument itself. */
void func_800433E4(arg0, arg1, arg2, arg3, arg4, arg5)
    s16 arg0; s16 arg1; s16 arg2; s16 arg3; s16 arg4; s16 arg5;
{
    *(s32 *)0x1F800000 = D_80103608[arg0][arg1];
    func_80043454(arg2, arg3, arg4, arg5);
}
extern s16 D_80095588[];
extern void gpu_OffsetTexPolyFT3();
extern void gpu_OffsetTexPolyFT4();
extern void gpu_OffsetTexPolyGT3();
extern void gpu_OffsetTexPolyGT4();
void gpu_OffsetTPageClutAt0And4(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
void gpu_OffsetTPageClutAt6And2(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4);
/* Read cursor into the primitive packet stream, kept in scratchpad word 0. */
#define SCRATCH_PTR (*(u16 **)0x1F800000)
/* Walk a packet stream of primitive groups (the cursor starts at the
 * scratchpad word) and shift every textured primitive's texture source by
 * (arg0, arg1) and its CLUT by (arg2, arg3): via gpu_OffsetTexPoly* for
 * mode-0 groups, via func_80043E98 / func_80043F0C plus a per-vertex v shift
 * for mode-1 / mode-2 groups.  Untextured groups are skipped using the
 * per-type halfword sizes in D_80095588. */
void func_80043454(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    s32 count;
    s32 mode;
    s32 type;
    s32 kind;
    s32 packed;
    s32 blocks;
    u16 *p;
    u8 *b;

    blocks = 0;
    do {
        p = SCRATCH_PTR;
        SCRATCH_PTR = p + 1;
        count = *p;
        if (count & 0x8000) {
            packed = 1;
            mode = 2;
            SCRATCH_PTR = SCRATCH_PTR + (count & 0x7FFF) + 1;
            if ((u32)SCRATCH_PTR & 3) {
                SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + (4 - ((u32)SCRATCH_PTR & 3)));
            }
        } else {
            SCRATCH_PTR = p + 2;
            mode = p[1];
            packed = 0;
            if (mode == 0) {
                SCRATCH_PTR += count * 12;
            } else {
                SCRATCH_PTR += count * 3;
            }
        }
        if (mode == 1) {
            if ((u32)SCRATCH_PTR & 3) {
                SCRATCH_PTR++;
            }
        }
        while ((count = *SCRATCH_PTR++) != 0) {
            type = *SCRATCH_PTR++;
            if ((type & 4) || (u32)(type - 0x26) < 4) {
                if ((u32)(type - 0x26) < 4) {
                    kind = type - 0x26;
                } else {
                    kind = (type >> 3) & 3;
                }
                switch (mode) {
                case 0:
                    while (--count != -1) {
                        switch (kind) {
                        case 0:
                            gpu_OffsetTexPolyFT3(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT3((u8 *)SCRATCH_PTR + 0x20, arg0, arg1, arg2, arg3);
                            break;
                        case 1:
                            gpu_OffsetTexPolyFT4(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT4((u8 *)SCRATCH_PTR + 0x28, arg0, arg1, arg2, arg3);
                            break;
                        case 2:
                            gpu_OffsetTexPolyGT3(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT3((u8 *)SCRATCH_PTR + 0x28, arg0, arg1, arg2, arg3);
                            break;
                        case 3:
                            gpu_OffsetTexPolyGT4(SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT4((u8 *)SCRATCH_PTR + 0x34, arg0, arg1, arg2, arg3);
                            break;
                        }
                        SCRATCH_PTR += D_80095588[type];
                    }
                    break;
                case 1:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt0And4((s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 1:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 2:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x24);
                            break;
                        case 3:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x2C);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                case 2:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt6And2((s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x14);
                            break;
                        case 1:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        case 2:
                            /* FAKE: cases 2/3 repeat cases 0/1 (one body per kind, as in
                               mode 1) instead of sharing their labels. jump2 cross-jump
                               re-merges the copies (bytes identical to `case 0: case 2:`),
                               but flow.c counts them before global RA: the extra refs and
                               live length seat count/base/kind in s3/s4/s5 as the target
                               does. */
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x14);
                            break;
                        case 3:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR = (u16 *)((u8 *)SCRATCH_PTR + 0x18);
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                }
            } else if (!packed) {
                SCRATCH_PTR += (s16)(count * D_80095588[type]);
            } else {
                switch (type) {
                case 0:
                    SCRATCH_PTR += count * 26;
                    break;
                case 8:
                    SCRATCH_PTR += count * 32;
                    break;
                default:
                    SCRATCH_PTR += (s16)(count * D_80095588[type]);
                    break;
                }
            }
        }
        if (packed) {
            if (blocks == 12) {
                return;
            }
            blocks++;
            while (*SCRATCH_PTR != 0xFFFF) {
                SCRATCH_PTR += 3;
            }
            SCRATCH_PTR++;
        }
    } while (*SCRATCH_PTR++ != 0);
}
/* PsyQ LIBGPU.H POLY_FT3 (0x20 bytes) -- the only libgpu primitive with a
 * u16 clut at +0xE, u16 tpage at +0x16 and v0/v1/v2 at +0xD/+0x15/+0x1D;
 * the caller (func_80043454) walks a 0x20-stride primitive array. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} POLY_FT3;

/* Shift a textured triangle's texture source: the tpage x field (64-px
 * pages) by du, the tpage y field (256-px pages) and every vertex v by dv,
 * and the clut x (16-px units) / y fields by dcx / dcy.  Each packed u16
 * field is updated in place inside its bit range. */
void gpu_OffsetTexPolyFT3(POLY_FT3 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes) -- u16 clut at +0xE, u16 tpage at
 * +0x16, v0/v1/v2/v3 at +0xD/+0x15/+0x1D/+0x25; the 0x28-stride quad
 * sibling of func_80043BD0 (POLY_FT3). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

/* Quad counterpart of func_80043BD0: shift a textured quad's texture source
 * (tpage x/y by du/dv, all four vertex v by dv, clut x/y by dcx/dcy), each
 * packed u16 field updated in place inside its bit range. */
void gpu_OffsetTexPolyFT4(POLY_FT4 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    p->v3 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
/* PsyQ LIBGPU.H POLY_GT3 (0x28 bytes) -- u16 clut at +0xE, u16 tpage at
 * +0x1A, v0/v1/v2 at +0xD/+0x19/+0x25; the caller (func_80043454) walks a
 * 0x28-stride primitive array (asm/funcs/func_80043454.s:182,198). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;

/* Shift a gouraud-textured triangle's texture source: the tpage x field
 * (64-px pages) by du, the tpage y field (256-px pages) and every vertex v
 * by dv, and the clut x (16-px units) / y fields by dcx / dcy.  Same update
 * as func_80043BD0 on a POLY_GT3. */
void gpu_OffsetTexPolyGT3(POLY_GT3 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
/* PsyQ LIBGPU.H POLY_GT4 (0x34 bytes) -- u16 clut at +0xE, u16 tpage at
 * +0x1A, v0/v1/v2/v3 at +0xD/+0x19/+0x25/+0x31; the 0x34-stride quad
 * sibling of func_80043D34 (POLY_GT3). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} POLY_GT4;

/* Quad counterpart of func_80043D34: shift a gouraud-textured quad's texture
 * source (tpage x/y by du/dv, all four vertex v by dv, clut x/y by dcx/dcy),
 * each packed u16 field updated in place inside its bit range. */
void gpu_OffsetTexPolyGT4(POLY_GT4 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    p->v3 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
void gpu_OffsetTPageClutAt0And4(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    s16 r1;
    r1 = gpu_OffsetTPage(a0[0], a1, a2);
    a0[0] = r1;
    a0[2] = gpu_OffsetClut(a0[2], a3, a4);
}
extern s16 gpu_OffsetTPage(s16, s16, s16);
extern s16 gpu_OffsetClut(s16, s16, s32);
void gpu_OffsetTPageClutAt6And2(s16 *a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    s16 r1;
    r1 = gpu_OffsetTPage(a0[3], a1, a2);
    a0[3] = r1;
    a0[1] = gpu_OffsetClut(a0[1], a3, a4);
}
s16 gpu_OffsetTPage(s16 a0, s16 a1, s16 a2) {
    s32 low = (a0 & 0xF) + (a1 >> 6);
    s32 mid;
    low &= 0xF;
    mid = ((a0 >> 4) & 1) + (a2 >> 8);
    mid &= 1;
    return (s16)(low | ((a0 & ~0x1F) | (mid << 4)));
}
s16 gpu_OffsetClut(s16 a0, s16 a1, s32 a2) {
    s32 low = (a1 >> 4) + (a0 & 0x3F);
    s32 mid;
    low &= 0x3F;
    mid = (a2 + (((u32)(a0 << 17)) >> 23)) & 0x1FF;
    return (s16)(low | ((a0 & (s16)0x8000) | (mid << 6)));
}
/* Relocates a block of pointer slots in place. p[0] is the header word: the slot count in its low
 * 15 bits, bit 15 set once the block is relocated. Marks the block relocated (keeping the low
 * halfword), records its first slot and count in D_80103608 / D_80103658 [slot], and, if it was not
 * relocated yet, turns each slot's block-relative offset into an address by adding the block's base.
 * func_80044098 is the inverse. */
void func_80044010(s32 *p, s16 slot) {
    s32 *base = p;
    s32 hdr;
    s32 i;
    u16 n;

    hdr = *p;
    *p = (hdr | 0x8000) & 0xFFFF;
    p++;
    D_80103608[slot] = p;
    D_80103658[slot] = hdr & 0x7FFF;
    if (!(hdr & 0x8000)) {
        n = hdr;
        for (i = 0; i < n; i++) {
            *p++ += (s32)base;
        }
    }
}
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                /* FAKE: semantically-null cancellation pair `v1++; v1--;`
                   adjacent to the real `v1++` (F6 cancellation pair,
                   .claude/rules/no-new-park-categories.md) -- the pair nets
                   zero and emits no bytes, but flow.c reg_n_refs counts the
                   extra loop-weighted pointer refs before combine.c re-merges
                   the chain into the single target addiu (combine.c:52-57 -
                   reg_n_refs is never adjusted afterwards), so global.c's
                   allocno priority for the pointer overtakes the counter's
                   and the pointer lands $v1 / the counter $a0 as target has
                   them. */
                v1++;
                v1--;
                v1++;
                a4--;
            } while (a4 != -1);
        }
    }
}
void func_80044100(s32 a0, s32 a1) {
    s32 *ptr = D_80103608[a0];
    s32 count = D_80103658[a0];
    D_80103608[a0] = (ptr = ptr + (a1 / 4));
    count--;
    if (count != -1) {
        do {
            *ptr += a1;
            ptr++;
            count--;
        } while (count != -1);
    }
}
extern void func_800520B8(s32, s32, s32);
s32 func_80044170(s32 *a0, ...) {
    s32 *base;
    s32 old_first;
    s32 count;
    s32 *slots;
    s32 *varptr;
    s32 dest;
    s32 entry;
    s32 size;

    base = a0;
    old_first = *base;
    count = *(s32 *)((s32)&a0 + 4);
    a0 = base + 1;
    *base = count;
    slots = a0;
    dest = (s32)base + *slots;
    count--;
    varptr = (s32 *)((s32)&a0 + 8);
    if (count != -1) {
        do {
            s32 *tbl;
            s32 cur_off;

            varptr++;
            entry = *(varptr - 1);
            if (entry >= old_first) {
                func_80052C10();
            }
            count--;
            tbl = (s32 *)((entry * 4) + (s32)a0);
            cur_off = *tbl;
            size = *(tbl + 1);
            *slots = dest - (s32)base;
            slots++;
            size = size - cur_off;
            func_800520B8((s32)base + cur_off, dest, size);
            size = (s32)(((u32)size >> 2) * 4);
            dest += size;
        } while (count != -1);
    }
    *slots = dest - (s32)base;
    return dest;
}
extern void func_800520B8(s32, s32, s32);
s32 func_8004428C(s32 *base, s16 *offsets) {
    s32 *b = base; /* FAKE: prologue pair order -- single forward-order
                      param alias (pointer-alias-fake-exception);
                      combine merges the
                      single-use param's entry copy into this init at the
                      later insn position, yielding target's s3-pair-first
                      prologue */
    s32 *slots = b + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)b + b[1]);
    s32 v1;
    s32 *walker;
    s32 size;
    s32 cur_off;
    s32 ret;

    v1 = *offsets;
    offsets++;
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }

    {
    s32 stop = -2; /* FAKE: constant-holder, named-local-fake-exception; the literal scores 2 */
    walker = slots;
    do {
        if (v1 >= 0) {
            size = walker[1];
            cur_off = walker[0];
            *slots = (s32)dest - (s32)b;
            slots++;
            count++;
            size = size - cur_off;
            func_800520B8((s32)b + cur_off, (s32)dest, size);
            size = (u32)size >> 2;
            size = size << 2;
            dest = (s32 *)((s32)dest + size);
        }
        walker++;
        v1 = *offsets;
        offsets++;
    } while (v1 != stop);
    }
    ret = (s32)dest;

done:
    v1 = ret - (s32)b;
    *b = count;
    *slots = v1;
    return ret;
}
extern void func_800520B8(s32, s32, s32);
s32 func_80044378(s32 src_base, s32 *dest_arr, s16 *frame_offsets) {
    s16 *fp;
    s16 *scan;
    s32 *orig_dest;
    s32 val;
    s32 data_ptr;
    s32 src_orig;
    s32 sentinel;
    s32 count;
    s32 *src_ptr;
    s32 start;
    s32 size;

    fp = frame_offsets;
    count = 0;
    scan = fp + 1;
    src_orig = src_base;
    orig_dest = dest_arr;

    val = *fp;
    if (val != -2) {
        do {
            if (val >= 0) {
                count++;
            }
            val = *scan;
            scan++;
        } while (val != -2);
    }

    *dest_arr = count;
    dest_arr++;
    src_base += 4;
    data_ptr = (s32)orig_dest + (count + 2) * 4;

    val = *fp;
    fp++;
    if (val != -2) {
        sentinel = -2;
        src_ptr = (s32 *)src_base;
        do {
            if (val >= 0) {
                size = src_ptr[1];
                start = *src_ptr;
                *dest_arr = data_ptr - (s32)orig_dest;
                dest_arr++;
                size = size - start;
                func_800520B8(src_orig + start, data_ptr, size);
                size = (u32)size >> 2;
                size = size << 2;
                data_ptr += size;
            }
            src_ptr++;
            val = *fp;
            fp++;
        } while (val != sentinel);
    }

    *dest_arr = data_ptr - (s32)orig_dest;
    return data_ptr;
}
extern s16 D_8010367E;
void func_80044498(void) {
    s32 i = 0x13;
    s16 *p = &D_8010367E;
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}
void func_800444BC(void) {
    func_80044504(D_800A378C);
}
void func_800444E0(void) {
    func_80044504(D_800A378C);
}
extern s32 D_800A3678;
extern s32 D_80101BD0;
extern s32 D_800A3708;
extern s32 D_800A370C;

extern s32 D_80102C00;

extern void MulMatrix(s32 *, s32 *);
extern void MulMatrix2(s32 *, s32 *);
extern void MulMatrix0(MATRIX *, MATRIX *, MATRIX *);
extern void func_80046F24(void);
extern s32 func_8003E2C8(void);
extern s32 func_8003F268(void);
extern s32 func_80046E7C(void);
extern void func_8004A4E0(void);
extern void func_80046E54(s32);
void func_80044504(u32 *a0) {
    s32 *s0 = &D_80101BD0;
    math_RotMatrixZXY(&D_800A3678, s0);
    MulMatrix(s0, (s32 *)(D_800A3708 + 0x18));
    MulMatrix2((s32 *)(D_800A370C + 0x18), s0);
    MulMatrix0((MATRIX *)(D_800A370C + 0x18), (MATRIX *)(D_800A3708 + 0x18), &D_800FF610);
    if (D_800A36AC & 1) {
        *(s32 *)0x1F800014 = -1;
    } else {
        *(s32 *)0x1F800014 = 0;
    }
    func_80046F24();
    *(s32 *)0x1F80001C = (s32)&D_80095328;
    *(s32 *)0x1F80000C = (s32)a0;
    {
        s32 v1;
        if (D_800A3790 & 8) {
            v1 = func_8003E2C8();
        } else {
            v1 = 0x7FFFFFFF;
        }
        *(s32 *)0x1F800010 = v1;
    }
    {
        s32 v0 = func_8003F268();
        if (v0 != 0) {
            v0 = 0xBE;
        } else {
            v0 = func_80046E7C();
            if (v0 != 0) {
                v0 = 0x182;
            } else {
                v0 = 0xBE;
            }
        }
        *(s32 *)0x1F800018 = v0;
    }
    func_8004A4E0();
    func_80046E54(1);
    D_800A3820 = (s32)&D_80102C00;
}
extern void func_80052C10(void);
void func_80044650(void) {
    func_80052C10();
}
extern s32 stage_GetId(void);
s32 func_80044670(s16 *a0, s16 a1, s32 a2) {
    s32 v0;
    /* FAKE: keeps reorg.c relax_delay_slots from inverting the two default-path
       j/nop pairs in the stage-id switch (NOTE_INSN_LOOP_BEG sets
       LABEL_OUTSIDE_LOOP_P, suppressing the invert-jump peephole) */
    do { } while (0);
    D_800A9CF8.unk0 = a1;
    v0 = *(u16 *)a0;
    a0++;
    D_800A9CF8.unk8 = (s32)a0;
    D_800A9CF8.unkC = a2;
    D_800A9CF8.unk2 = v0;
    v0 = stage_GetId();
    D_800A9CF8.unk4 = v0;
    switch ((s16)v0) {
    case 7:
        D_800A9CF8.unk6 = 0x11;
        break;
    case 4:
        D_800A9CF8.unk6 = 2;
        break;
    case 0x12:
        D_800A9CF8.unk6 = 0xA;
        break;
    }
    {
        s32 val = D_800A9CF8.unk6;
        return a2 + val * 104;
    }
}
extern void *game_GetCharData(void);
void func_8004473C(void)
{
    Unk800A6690Rec *src;
    Unk800A9CF8Entry *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)(src = game_GetCharData());
    dst = (Unk800A9CF8Entry *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; dst++, src++, i++) {
        dst->node.unk0 = 0;
        dst->node.unk1 = 0;
        dst->node.unk2 = 0;
        dst->node.unk4 = D_800A9CF8.unk0;
        dst->node.unk8 = 0;
        dst->node.unkC = 0;
        dst->node.unkA = 4;
        dst->node.xf.rot.vx = 0;
        dst->node.xf.rot.vy = 0;
        dst->node.xf.rot.vz = 0;
        dst->node.work.t[0] = src->node.xf.mat.t[0];
        dst->node.work.t[1] = src->node.xf.mat.t[1];
        dst->node.work.t[2] = src->node.xf.mat.t[2];
        dst->node.unk6 = 0;
        dst->unk58 = -1;
    }
}
extern void func_800417D0(s32 *);

/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 D_800A3708;
u32 *D_800A378C;
s32 D_800A3790;
s32 D_800A3820;
s32 D_800A3828;
