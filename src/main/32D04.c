/* Rotation-matrix and colour math (math_RotMatrixZXY / YXZ / XYZ,
 * math_Rotate2D, math_RgbToHsv, ...) and the primitive texture-offset helpers
 * (gpu_OffsetTexPolyFT3..gpu_OffsetClut). .text 0x80042504 (ROM 0x32D04).
 * Start boundary: G8 (the -G8 run ends). */
#include "common.h"
#include "bb2.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include <psxsdk/libgte.h>

void math_HsvToRgb(s32 *arg0, s32 *arg1) {
    s32 h = arg0[0];
    s32 s = arg0[1];
    s32 v = arg0[2];
    s32 i, f, p, q, t;
    s32 r, g, b;

    if (gpu_RefreshGrayscaleMode()) {
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

    if ((u32)i >= 6U)
        goto out;
    switch (i) {
    case 0:
        r = v;
        g = t;
        b = p;
        goto out;
    case 1:
        r = q;
        g = v;
        b = p;
        goto out;
    case 2:
        r = p;
        g = v;
        b = t;
        goto out;
    case 3:
        r = p;
        g = q;
        b = v;
        goto out;
    case 4:
        r = t;
        g = p;
        b = v;
        goto out;
    case 5:
        r = v;
        g = p;
        b = q;
        goto out;
    }
out:
    arg1[0] = r;
    arg1[1] = g;
    arg1[2] = b;
}

/* RGB -> HSV (4.12 fixed point). Inverse of math_HsvToRgb above.
 * Outputs a1[] = { H, S, val }; val (V) = max(r,g,b) is the third
 * output channel, held separately from the max used for the (max - min)
 * deltas / max-channel compares. */
void math_RgbToHsv(s32 *a0, s32 *a1) {
    s32 r, g, b;
    s32 max_val, min_val;
    s32 var_5;
    s32 var_6;
    s32 var_7;
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

    var_5 = max_val - min_val;
    /* V = max(r,g,b): the value channel, third HSV output */
    val = max_val;

    if (val != 0) {
        var_6 = (var_5 << 12) / val;
    } else {
        var_6 = 0;
    }

    if (var_6 != 0) {
        dR = ((max_val - r) << 12) / var_5;
        dG = ((max_val - g) << 12) / var_5;
        dB = ((max_val - b) << 12) / var_5;

        if (r == max_val) {
            var_7 = dB - dG;
        } else if (g == max_val) {
            s32 tmp = dB - 0x2000;
            var_7 = dR - tmp;
        } else if (b == max_val) {
            s32 tmp = dR - 0x4000;
            var_7 = dG - tmp;
        }

        var_7 /= 6;

        if (var_7 < 0) {
            var_7 += 0x1000;
        }
    } else {
        var_7 = 0;
    }

    a1[0] = var_7;
    a1[1] = var_6;
    a1[2] = val;
}

void func_8004283C(s32 a0) {
    if (a0) {
        D_800F6650 = 1;
    } else {
        D_800F6650 = 0;
    }
}

s32 func_80042864(void) { return D_800F6650; }

void math_RotMatrixZXY(u16 *a0, MATRIX *a1) {
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
    a1->m[2][1] = sinA;
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

    a1->m[0][1] = cosA_negSinC >> 12;
    a1->m[1][1] = cosA_cosC >> 12;
    a1->m[2][0] = cosA_negSinB >> 12;
    a1->m[2][2] = cosA_cosB >> 12;
    a1->m[0][0] = (prod_sinC + cosB_cosC) >> 12;
    a1->m[0][2] = (prod2_sinC + sinB_cosC) >> 12;
    a1->m[1][0] = (sinAxsinB_12_cosC + cosB_sinC) >> 12;

    sinB_sinC = sinB * sinC;
    a1->m[1][2] = (negSinAxcosB_12_cosC + sinB_sinC) >> 12;
}

/* Euler angles (a0[0..2], 12-bit) -> the 3x3 rotation part of *a1, Y-X-Z
 * order. cosA is read through a u16 local and sign-extended with an (s16)
 * cast, the -sinA store between them, which keeps the lhu + sll + sra shape.
 */
void math_RotMatrixYXZ(u16 *a0, MATRIX *a1) {
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
    s32 var_19, neg_sinB_cosC;
    s32 var_21;
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
    a1->m[1][2] = -sinA;
    cosA = (s16)rawA;

    cosB_negsinC = cosB * -sinC;

    cosA_sinB = cosA * sinB;
    cosA_sinC = cosA * sinC;

    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;

    sinAxcosB_12 = sinAxcosB >> 12;

    var_19 = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;

    var_21 = sinAxcosB_12 * cosC;

    cosA_cosB = cosA * cosB;

    a1->m[0][2] = cosA_sinB >> 12;
    a1->m[1][0] = cosA_sinC >> 12;
    a1->m[1][1] = cosA_cosC >> 12;
    a1->m[2][2] = cosA_cosB >> 12;
    a1->m[0][0] = (prod_sinC + cosB_cosC) >> 12;
    a1->m[0][1] = (prod_cosC + cosB_negsinC) >> 12;
    a1->m[2][0] = (var_19 + neg_sinB_cosC) >> 12;

    sinB_sinC = sinB * sinC;
    a1->m[2][1] = (var_21 + sinB_sinC) >> 12;
}

void math_RotMatrixXYZ(u16 *a0, MATRIX *a1) {
    s32 angA, angB, angC;
    s16 sinA, sinB, sinC, cosA;
    s32 cosB, cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 var_11, var_12;
    s32 var_13, var_14;
    s32 cosA_sinC, cosA_cosC;
    s32 cosA_negsinB;
    s32 negsinA_cosB;
    s32 var_19, var_20;
    s32 cosA_sinB;
    s32 sinA_sinC;
    s32 var_23;
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
    a1->m[0][2] = sinB;
    var_11 = sinA * sinB;
    cosB_negsinC = cosB * -sinC;
    var_12 = var_11 >> 12;
    var_13 = var_12 * cosC;
    cosA_sinC = cosA * sinC;
    var_14 = var_12 * -sinC;
    cosA_cosC = cosA * cosC;
    cosA_negsinB = cosA * -sinB;
    negsinA_cosB = -sinA * cosB;
    var_19 = cosA_negsinB >> 12;
    var_20 = var_19 * cosC;
    cosA_sinB = cosA * sinB;
    sinA_sinC = sinA * sinC;
    var_23 = (cosA_sinB >> 12) * sinC;
    cosA_cosB = cosA * cosB;

    a1->m[0][0] = cosB_cosC >> 12;
    a1->m[0][1] = cosB_negsinC >> 12;
    a1->m[1][2] = negsinA_cosB >> 12;
    sinA_cosC = sinA * cosC;
    a1->m[2][2] = cosA_cosB >> 12;
    a1->m[1][0] = (var_13 + cosA_sinC) >> 12;
    a1->m[1][1] = (var_14 + cosA_cosC) >> 12;
    a1->m[2][0] = (var_20 + sinA_sinC) >> 12;
    a1->m[2][1] = (var_23 + sinA_cosC) >> 12;
}

extern void math_RotMatrixXYZ();

void math_InitRotMatrixFuncs(void) {
    D_800F66A0[0] = math_RotMatrixZYX;
    /* the three below take their angles as u16 *, not SVECTOR * */
    D_800F66A0[2] = (RotMatrixFunc)math_RotMatrixZXY;
    D_800F66A0[4] = (RotMatrixFunc)math_RotMatrixYXZ;
    D_800F66A0[5] = (RotMatrixFunc)math_RotMatrixXYZ;
}

void math_TransposeMatrixInPlace(MATRIX *a0) {
    s16 temp;

    temp = a0->m[2][0];
    a0->m[2][0] = a0->m[0][2];
    a0->m[0][2] = temp;
    temp = a0->m[1][0];
    a0->m[1][0] = a0->m[0][1];
    a0->m[0][1] = temp;
    temp = a0->m[2][1];
    a0->m[2][1] = a0->m[1][2];
    a0->m[1][2] = temp;
}

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

void math_MatrixToAnglesYXZ(MATRIX *a0, SVECTOR *a1) {
    SVECTOR rot;
    VECTOR result;
    MATRIX sp28;
    s16 angle1;
    s32 cos_val, sin_val;
    s16 neg_angle2;
    s32 var_7;

    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(a0, &rot, &result);

    angle1 = ratan2(result.vx, result.vz);

    cos_val = rcos((s16)angle1);
    sin_val = rsin((s16)angle1);

    var_7 = (cos_val * result.vz + sin_val * result.vx) >> 12;
    neg_angle2 = -ratan2(result.vy, var_7);

    rot.vx = -neg_angle2;
    rot.vy = -angle1;
    rot.vz = 0;
    math_RotMatrixXYZ((u16 *)&rot, &sp28);

    MulMatrix(&sp28, a0);

    rot.vx = 0;
    rot.vy = 0x1000;
    rot.vz = 0;
    ApplyMatrix(&sp28, &rot, &result);

    {
        s16 angle3;
        angle3 = ratan2(result.vx, result.vy);
        a1->vx = neg_angle2;
        a1->vy = angle1;
        a1->vz = -angle3;
    }
}

extern void func_8004DDB4(s32, s32, s32, s32);
extern MATRIX D_800FF610;
extern void (*D_800951D8[])();
extern void (*D_80095280[])();
extern void (*D_80095328[])();
extern Unk800F62E0Rec *D_800A3828;

void func_800430E4(s32 arg0, s32 arg1, s16 arg2, u8 *arg3) {
    MATRIX *dst = (MATRIX *)0x1F8003A0;
    s32 t0;

    t0 = *(s32 *)0x1F800008;
    *(s32 *)0x1F800008 = 3 - t0;

    *dst = D_800FF610;

    D_800A3828 = &D_800F62E0[arg2];

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

s32 math_ClassifyThresholdBand(s32 a0) {
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

s32 gpu_RemapDepthToOtIndex(s32 a0) {
    s32 v0 = a0 >> *(s32 *)0x1F800008;
    s32 shift = v0 >> 11;
    s32 mant = (v0 & 0x7FF) | -0x1000;
    return (mant >> shift) & 0xFFF;
}

extern u16 D_80103658[];
extern void gpu_OffsetTexPrimStream(s16, s16, s16, s16);

void gpu_OffsetBlockTexPrimStreams(
    s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    u16 arg4_lo = arg4;
    s16 i;

    for (i = 0; i < D_80103658[arg0]; i++) {
        *(s32 *)0x1F800000 = D_80103608[arg0][i];
        gpu_OffsetTexPrimStream(arg1, arg2, arg3, arg4_lo);
    }
}

void gpu_OffsetBlockTexPrimStreamsByPages(
    s16 a0, s16 a1, s16 a2, s16 a3, s16 a4) {
    gpu_OffsetBlockTexPrimStreams(
        a0, (s16)(a1 << 6), (s16)(a2 << 8), (s16)(a3 << 6), (s16)(a4 << 8));
}

/* Old-style definition: the only caller, func_80045B68, passes every
 * argument as a plain int (no sign-extension at the call site), so no
 * prototype was in scope there; the body narrows each argument itself. */
void gpu_OffsetBlockTexPrimStream(arg0, arg1, arg2, arg3, arg4, arg5) s16 arg0;
s16 arg1;
s16 arg2;
s16 arg3;
s16 arg4;
s16 arg5;
{
    *(s32 *)0x1F800000 = D_80103608[arg0][arg1];
    gpu_OffsetTexPrimStream(arg2, arg3, arg4, arg5);
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
 * mode-0 groups, via gpu_OffsetTPageClutAt0And4 / gpu_OffsetTPageClutAt6And2
 * plus a per-vertex v shift for mode-1 / mode-2 groups.  Untextured groups are
 * skipped using the per-type halfword sizes in D_80095588. */
void gpu_OffsetTexPrimStream(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    s32 count;
    s32 mode;
    s32 type;
    s32 kind;
    s32 var_4;
    s32 var_5;
    u16 *p;
    u8 *b;

    var_5 = 0;
    do {
        p = SCRATCH_PTR;
        SCRATCH_PTR = p + 1;
        count = *p;
        if (count & 0x8000) {
            var_4 = 1;
            mode = 2;
            SCRATCH_PTR = SCRATCH_PTR + (count & 0x7FFF) + 1;
            if ((u32)SCRATCH_PTR & 3) {
                SCRATCH_PTR =
                    (u16 *)((u8 *)SCRATCH_PTR + (4 - ((u32)SCRATCH_PTR & 3)));
            }
        } else {
            SCRATCH_PTR = p + 2;
            mode = p[1];
            var_4 = 0;
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
                            gpu_OffsetTexPolyFT3(
                                SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT3((POLY_FT3 *)SCRATCH_PTR + 1,
                                                 arg0, arg1, arg2, arg3);
                            break;
                        case 1:
                            gpu_OffsetTexPolyFT4(
                                SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyFT4((POLY_FT4 *)SCRATCH_PTR + 1,
                                                 arg0, arg1, arg2, arg3);
                            break;
                        case 2:
                            gpu_OffsetTexPolyGT3(
                                SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT3((POLY_GT3 *)SCRATCH_PTR + 1,
                                                 arg0, arg1, arg2, arg3);
                            break;
                        case 3:
                            gpu_OffsetTexPolyGT4(
                                SCRATCH_PTR, arg0, arg1, arg2, arg3);
                            gpu_OffsetTexPolyGT4((POLY_GT4 *)SCRATCH_PTR + 1,
                                                 arg0, arg1, arg2, arg3);
                            break;
                        }
                        SCRATCH_PTR += D_80095588[type];
                    }
                    break;
                case 1:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt0And4(
                            (s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR += 12;
                            break;
                        case 1:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR += 12;
                            break;
                        case 2:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            SCRATCH_PTR += 18;
                            break;
                        case 3:
                            b[7] += arg1;
                            b[9] += arg1;
                            b[11] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR += 22;
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                case 2:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt6And2(
                            (s16 *)SCRATCH_PTR, arg0, arg1, arg2, arg3);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            SCRATCH_PTR += 10;
                            break;
                        case 1:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR += 12;
                            break;
                        case 2:
                            /* one case per primitive kind, as in modes 0 and
                               1: in this layout the GT3 / GT4 offsets equal
                               the FT3 / FT4 ones (cross-jump merges the
                               copies; shared labels: score 109). */
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            SCRATCH_PTR += 10;
                            break;
                        case 3:
                            b[1] += arg1;
                            b[5] += arg1;
                            b[9] += arg1;
                            b[13] += arg1;
                            SCRATCH_PTR += 12;
                            break;
                        default:
                            func_80052C10();
                            break;
                        }
                    }
                    break;
                }
            } else if (!var_4) {
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
        if (var_4) {
            if (var_5 == 12) {
                return;
            }
            var_5++;
            while (*SCRATCH_PTR != 0xFFFF) {
                SCRATCH_PTR += 3;
            }
            SCRATCH_PTR++;
        }
    } while (*SCRATCH_PTR++ != 0);
}

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

/* Quad counterpart of gpu_OffsetTexPolyFT3: shift a textured quad's texture
 * source (tpage x/y by du/dv, all four vertex v by dv, clut x/y by dcx/dcy),
 * each packed u16 field updated in place inside its bit range. */
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

/* Shift a gouraud-textured triangle's texture source: the tpage x field
 * (64-px pages) by du, the tpage y field (256-px pages) and every vertex v
 * by dv, and the clut x (16-px units) / y fields by dcx / dcy.  Same update
 * as gpu_OffsetTexPolyFT3 on a POLY_GT3. */
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

/* Quad counterpart of gpu_OffsetTexPolyGT3: shift a gouraud-textured quad's
 * texture source (tpage x/y by du/dv, all four vertex v by dv, clut x/y by
 * dcx/dcy), each packed u16 field updated in place inside its bit range. */
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

/* Relocates a block of pointer words in place. p[0] holds the word count in
 * its low 15 bits, bit 15 set once the block is relocated. Marks the block
 * relocated (keeping the low halfword), records its first word (p + 1) and
 * count in D_80103608 / D_80103658 [arg1], and, if it was not relocated yet,
 * turns each word's block-relative offset into an address by adding the
 * block's base. reloc_UnrelocateBlock is the inverse. */
void reloc_RelocateBlock(s32 *p, s16 arg1) {
    s32 *base = p;
    s32 var_1;
    s32 i;
    u16 n;

    var_1 = *p;
    *p = (var_1 | 0x8000) & 0xFFFF;
    p++;
    D_80103608[arg1] = p;
    D_80103658[arg1] = var_1 & 0x7FFF;
    if (!(var_1 & 0x8000)) {
        n = var_1;
        for (i = 0; i < n; i++) {
            *p++ += (s32)base;
        }
    }
}

void reloc_UnrelocateBlock(s16 a0) {
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
                /* FAKE: `v1++; v1--;` cancellation pair adds loop-weighted refs
                   so the pointer gets $v1 and the counter $a0 (F6 cancellation
                   pair, no-new-park-categories); without it: score 13 */
                v1++;
                v1--;
                v1++;
                a4--;
            } while (a4 != -1);
        }
    }
}

void reloc_RebaseBlock(s32 a0, s32 a1) {
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

typedef char *va_list;
#define va_start(ap, last) ((ap) = (va_list)(&(last) + 1))
#define va_arg(ap, type) ((type *)(void *)(ap += 4))[-1]

s32 reloc_KeepListedEntries(s32 *a0, ...) {
    s32 *base;
    s32 old_first;
    s32 count;
    s32 *var_3;
    s32 dest;
    s32 entry;
    s32 size;
    va_list ap;

    base = a0;
    old_first = *a0++;
    va_start(ap, a0);
    count = va_arg(ap, s32);
    *base = count;
    var_3 = a0;
    dest = (s32)base + *var_3;
    count--;
    if (count != -1) {
        do {
            s32 cur_off;

            entry = va_arg(ap, s32);
            if (entry >= old_first) {
                func_80052C10();
            }
            count--;
            cur_off = a0[entry];
            size = a0[entry + 1];
            *var_3 = dest - (s32)base;
            var_3++;
            size = size - cur_off;
            mem_MoveBlockRoundedUpToWords((s32)base + cur_off, dest, size);
            size = (s32)(((u32)size >> 2) * 4);
            dest += size;
        } while (count != -1);
    }
    *var_3 = dest - (s32)base;
    return dest;
}

s32 reloc_KeepSelectedEntries(s32 *base, s16 *offsets) {
    /* FAKE: param alias gives the target's prologue save order
       (pointer-alias-fake-exception); base used directly: score 4 */
    s32 *b = base;
    s32 *var_1 = b + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)b + b[1]);
    s32 v1;
    s32 *entries;
    s32 size;
    s32 cur_off;
    s32 ret;
    s32 i;

    v1 = *offsets;
    offsets++;
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }

    entries = var_1;
    i = 0;
    do {
        if (v1 >= 0) {
            size = entries[i + 1];
            cur_off = entries[i];
            *var_1 = (s32)dest - (s32)b;
            var_1++;
            count++;
            size = size - cur_off;
            mem_MoveBlockRoundedUpToWords((s32)b + cur_off, (s32)dest, size);
            size = (u32)size >> 2;
            size = size << 2;
            dest = (s32 *)((s32)dest + size);
        }
        i++;
        v1 = *offsets;
        offsets++;
    } while (v1 != -2);
    ret = (s32)dest;

done:
    v1 = ret - (s32)b;
    *b = count;
    *var_1 = v1;
    return ret;
}

s32 reloc_CopySelectedEntries(s32 src_base, s32 *dest_arr, s16 *arg2) {
    s16 *var_0;
    s16 *var_1;
    s32 *orig_dest;
    s32 val;
    s32 data_ptr;
    s32 src_orig;
    s32 var_6;
    s32 count;
    s32 *src_ptr;
    s32 start;
    s32 size;

    var_0 = arg2;
    count = 0;
    var_1 = var_0 + 1;
    src_orig = src_base;
    orig_dest = dest_arr;

    val = *var_0;
    if (val != -2) {
        do {
            if (val >= 0) {
                count++;
            }
            val = *var_1;
            var_1++;
        } while (val != -2);
    }

    *dest_arr = count;
    dest_arr++;
    src_base += 4;
    data_ptr = (s32)orig_dest + (count + 2) * 4;

    val = *var_0;
    var_0++;
    if (val != -2) {
        var_6 = -2;
        src_ptr = (s32 *)src_base;
        do {
            if (val >= 0) {
                size = src_ptr[1];
                start = *src_ptr;
                *dest_arr = data_ptr - (s32)orig_dest;
                dest_arr++;
                size = size - start;
                mem_MoveBlockRoundedUpToWords(src_orig + start, data_ptr, size);
                size = (u32)size >> 2;
                size = size << 2;
                data_ptr += size;
            }
            src_ptr++;
            val = *var_0;
            var_0++;
        } while (val != var_6);
    }

    *dest_arr = data_ptr - (s32)orig_dest;
    return data_ptr;
}

extern s16 D_8010367E;

void reloc_ClearCounts(void) {
    s32 i = 0x13;
    s16 *p = &D_8010367E;
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}

void func_800444BC(void) { gpu_PrepareViewAndFlushDrawQueue(D_800A378C); }

void func_800444E0(void) { gpu_PrepareViewAndFlushDrawQueue(D_800A378C); }

extern MATRIX D_80101BD0;

extern void func_8004A4E0(void);

void gpu_PrepareViewAndFlushDrawQueue(u32 *a0) {
    math_RotMatrixZXY(D_800A3678, &D_80101BD0);
    MulMatrix(&D_80101BD0, &D_800A3708->xf.mat);
    MulMatrix2(&D_800A370C->xf.mat, &D_80101BD0);
    MulMatrix0(&D_800A370C->xf.mat, &D_800A3708->xf.mat, &D_800FF610);
    if (D_800A36AC & 1) {
        *(s32 *)0x1F800014 = -1;
    } else {
        *(s32 *)0x1F800014 = 0;
    }
    math_SetLightPlaneProjection();
    *(s32 *)0x1F80001C = (s32)&D_80095328;
    *(s32 *)0x1F80000C = (s32)a0;
    if (D_800A3790 & 8) {
        *(s32 *)0x1F800010 = gpu_GetLastFogOtIndex();
    } else {
        *(s32 *)0x1F800010 = 0x7FFFFFFF;
    }
    if (func_8003F268() != 0) {
        *(s32 *)0x1F800018 = 0xBE;
    } else if (gpu_GetRaisedOtCutoffRequested() != 0) {
        *(s32 *)0x1F800018 = 0x182;
    } else {
        *(s32 *)0x1F800018 = 0xBE;
    }
    func_8004A4E0();
    gpu_SetRaisedOtCutoffRequested(1);
    g_draw_queue_cursor = g_draw_queue;
}

void func_80044650(void) { func_80052C10(); }

s32 node_BindFrameDataAndReserveEntryBuffer(s16 *a0, s16 a1, s32 a2) {
    s32 v0;
    D_800A9CF8.unk0 = a1;
    D_800A9CF8.unk2 = *a0++;
    D_800A9CF8.unk8 = (s32)a0;
    D_800A9CF8.unkC = a2;
    v0 = resource_GetLoadedSelector();
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
    a2 += D_800A9CF8.unk6 * 104;
    return a2;
}

void node_InitFrameEntriesFromWorldTranslations(void) {
    Unk800A6690Rec *src;
    Unk800A9CF8Entry *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)(src = func_8003F1D4());
    dst = (Unk800A9CF8Entry *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; dst++, src++, i++) {
        dst->node.unk0 = 0;
        dst->node.unk1 = 0;
        dst->node.unk2 = 0;
        dst->node.unk4 = D_800A9CF8.unk0;
        dst->node.rotation_order = 0;
        dst->node.parent_node = 0;
        dst->node.light_setup_index = 4;
        dst->node.xf.rot.vx = 0;
        dst->node.xf.rot.vy = 0;
        dst->node.xf.rot.vz = 0;
        dst->node.work.t[0] = src->node.xf.mat.t[0];
        dst->node.work.t[1] = src->node.xf.mat.t[1];
        dst->node.work.t[2] = src->node.xf.mat.t[2];
        dst->node.matrix_state = 0;
        dst->unk58 = -1;
    }
}

/* .data 0x800951D8..0x800955E0 (35000.c's NDATA path pointers D_800955E0
 * follow), in address order: five
 * primitive-type handler tables (func_80044650, which calls func_80052C10, is
 * the default entry; the others are 3AB48.c's draw routines) that func_800430E4
 * installs at scratchpad word 0x1F80001C and 3AB48.c's func_8004A4E0 selects
 * from, then gpu_OffsetTexPrimStream's per-type halfword sizes D_80095588.
 * Sizes from the data labels. Defined after every function. */
void gpu_EmitOtCutoffFlatLitTexturedTriangles();
void gpu_EmitOtCutoffFlatLitTexturedQuads();
void func_8004D634();
void func_8004D838();
void gpu_EmitAveragedTableColorTexturedTriangles();
void gpu_EmitAveragedTableColorTexturedQuads();
void gpu_EmitTableColorGouraudTexturedTriangles();
void gpu_EmitTableColorGouraudTexturedQuads();
void gpu_EmitOrientationAdjustedFlatLitTexturedTriangles();
void gpu_EmitOrientationAdjustedFlatLitTexturedQuads();
void gpu_EmitOrientationAdjustedGouraudTexturedTriangles();
void gpu_EmitOrientationAdjustedGouraudTexturedQuads();
void gpu_EmitOtCutoffRawTexturedTriangles();
void gpu_EmitOtCutoffRawTexturedQuads();
void gpu_EmitFlatLitTexturedTriangles();
void gpu_EmitFlatLitTexturedQuads();
void func_8004FF40();
void func_80050120();
void func_80050334();
void func_80050538();
void gpu_EmitBlackFlatTriangles();
void gpu_EmitBlackFlatQuads();
void func_80050AB8();
void func_80050C68();
void func_80050E60();
void func_80051010();
void func_80051208();
void func_800513B0();
void func_800515AC();
void func_80051754();
void func_80051D08();
void func_80051ED4();
void (*D_800951D8[42])() = {
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    gpu_EmitAveragedTableColorTexturedTriangles,
    gpu_EmitAveragedTableColorTexturedQuads,
    gpu_EmitTableColorGouraudTexturedTriangles,
    gpu_EmitTableColorGouraudTexturedQuads,
};
void (*D_80095280[42])() = {
    func_80044650, func_80044650, func_80044650, func_80044650, func_80051208,
    func_80044650, func_80051208, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_800513B0, func_80044650, func_800513B0,
    func_80044650, func_80044650, func_80044650, func_80044650, func_80044650,
    func_800515AC, func_80044650, func_800515AC, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80051754, func_80044650,
    func_80051754, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80050AB8, func_80050C68,
    func_80050E60, func_80051010,
};
void (*D_80095328[88])() = {
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    gpu_EmitFlatLitTexturedTriangles,
    gpu_EmitOtCutoffRawTexturedTriangles,
    gpu_EmitOrientationAdjustedFlatLitTexturedTriangles,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    gpu_EmitFlatLitTexturedQuads,
    gpu_EmitOtCutoffRawTexturedQuads,
    gpu_EmitOrientationAdjustedFlatLitTexturedQuads,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80050334,
    func_80044650,
    gpu_EmitOrientationAdjustedGouraudTexturedTriangles,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80050538,
    func_80044650,
    gpu_EmitOrientationAdjustedGouraudTexturedQuads,
    func_80044650,
    gpu_EmitBlackFlatTriangles,
    gpu_EmitBlackFlatQuads,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_8004FF40,
    func_80050120,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80051208,
    func_80044650,
    func_80051208,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_800513B0,
    func_80044650,
    func_800513B0,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_800515AC,
    func_80044650,
    func_800515AC,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80051754,
    func_80044650,
    func_80051754,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
};
void (*D_80095488[32])() = {
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    gpu_EmitOtCutoffFlatLitTexturedTriangles,
    gpu_EmitOtCutoffRawTexturedTriangles,
    gpu_EmitOrientationAdjustedFlatLitTexturedTriangles,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    gpu_EmitOtCutoffFlatLitTexturedQuads,
    gpu_EmitOtCutoffRawTexturedQuads,
    gpu_EmitOrientationAdjustedFlatLitTexturedQuads,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_8004D634,
    func_80044650,
    gpu_EmitOrientationAdjustedGouraudTexturedTriangles,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_80044650,
    func_8004D838,
    func_80044650,
    gpu_EmitOrientationAdjustedGouraudTexturedQuads,
    func_80044650,
};
void (*D_80095508[32])() = {
    func_80051D08, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80051ED4, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650, func_80044650, func_80044650, func_80044650,
    func_80044650, func_80044650,
};
s16 D_80095588[44] = {
    28, 22, 28, 22, 38, 34, 38, 0,  32, 26, 0,  0,  46, 42, 46,
    0,  48, 0,  48, 0,  52, 0,  52, 0,  62, 0,  0,  0,  66, 0,
    66, 0,  2,  2,  38, 46, 0,  40, 10, 12, 10, 12, 38, 46,
};

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
Unk80101DF0Record *D_800A3708;
u32 *D_800A378C;
s32 D_800A3790;
void **g_draw_queue_cursor;
Unk800F62E0Rec *D_800A3828;
