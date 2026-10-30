/* Rotation-matrix and colour math (math_RotMatrixZXY / YXZ / XYZ,
 * math_Rotate2D, math_RgbToHsv, ...) and the primitive texture-offset helpers
 * (gpu_OffsetTexPolyFT3..gpu_OffsetClut). .text 0x80042504 (ROM 0x32D04).
 * Start boundary: G8 (the -G8 run ends). */
#include "common.h"
#include "bb2.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include <psxsdk/libgte.h>

void math_HsvToRgb(s32 *hsv_input, s32 *rgb_output) {
    s32 h = hsv_input[0];
    s32 s = hsv_input[1];
    s32 v = hsv_input[2];
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
    rgb_output[0] = r;
    rgb_output[1] = g;
    rgb_output[2] = b;
}

/* RGB -> HSV (4.12 fixed point). Inverse of math_HsvToRgb above.
 * Outputs hsv_output[] = { H, S, val }; val (V) = max(r,g,b) is the third
 * output channel, held separately from the max used for the (max - min)
 * deltas / max-channel compares. */
void math_RgbToHsv(s32 *rgb_input, s32 *hsv_output) {
    s32 r, g, b;
    s32 max_val, min_val;
    s32 chroma;
    s32 saturation;
    s32 hue;
    s32 dR, dG, dB;
    s32 val;

    r = rgb_input[0];
    g = rgb_input[1];
    b = rgb_input[2];

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
        saturation = (chroma << 12) / val;
    } else {
        saturation = 0;
    }

    if (saturation != 0) {
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

    hsv_output[0] = hue;
    hsv_output[1] = saturation;
    hsv_output[2] = val;
}

void func_8004283C(s32 a0) {
    if (a0) {
        D_800F6650 = 1;
    } else {
        D_800F6650 = 0;
    }
}

s32 func_80042864(void) { return D_800F6650; }

void math_RotMatrixZXY(u16 *angles, MATRIX *matrix) {
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

    angA = angles[0];
    angB = angles[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];

    idxB = (s16)angB + 0x400;

    negSinAxsinB_12 = (sinA * -sinB) >> 12;

    angC = angles[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = negSinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;

    sinAxcosB = sinA * cosB;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    matrix->m[2][1] = sinA;
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

    matrix->m[0][1] = cosA_negSinC >> 12;
    matrix->m[1][1] = cosA_cosC >> 12;
    matrix->m[2][0] = cosA_negSinB >> 12;
    matrix->m[2][2] = cosA_cosB >> 12;
    matrix->m[0][0] = (prod_sinC + cosB_cosC) >> 12;
    matrix->m[0][2] = (prod2_sinC + sinB_cosC) >> 12;
    matrix->m[1][0] = (sinAxsinB_12_cosC + cosB_sinC) >> 12;

    sinB_sinC = sinB * sinC;
    matrix->m[1][2] = (negSinAxcosB_12_cosC + sinB_sinC) >> 12;
}

/* Euler angles (angles[0..2], 12-bit) -> the 3x3 rotation part of *matrix,
 * Y-X-Z order. cosA is read through a u16 local and sign-extended with an (s16)
 * cast, the -sinA store between them, which keeps the lhu + sll + sra shape.
 */
void math_RotMatrixYXZ(u16 *angles, MATRIX *matrix) {
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

    angA = angles[0];
    angB = angles[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    idxB = (s16)angB + 0x400;

    sinAxsinB_12 = (sinA * sinB) >> 12;

    angC = angles[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = sinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    matrix->m[1][2] = -sinA;
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

    matrix->m[0][2] = cosA_sinB >> 12;
    matrix->m[1][0] = cosA_sinC >> 12;
    matrix->m[1][1] = cosA_cosC >> 12;
    matrix->m[2][2] = cosA_cosB >> 12;
    matrix->m[0][0] = (prod_sinC + cosB_cosC) >> 12;
    matrix->m[0][1] = (prod_cosC + cosB_negsinC) >> 12;
    matrix->m[2][0] = (var_19 + neg_sinB_cosC) >> 12;

    sinB_sinC = sinB * sinC;
    matrix->m[2][1] = (var_21 + sinB_sinC) >> 12;
}

void math_RotMatrixXYZ(u16 *angles, MATRIX *matrix) {
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

    angB = angles[1];
    angC = angles[2];
    sinB = Judge[angB & 0xFFF];
    angA = angles[0];
    sinA = Judge[angA & 0xFFF];
    sinC = Judge[angC & 0xFFF];
    cosA = Judge[((s16)angA + 0x400) & 0xFFF];

    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    matrix->m[0][2] = sinB;
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

    matrix->m[0][0] = cosB_cosC >> 12;
    matrix->m[0][1] = cosB_negsinC >> 12;
    matrix->m[1][2] = negsinA_cosB >> 12;
    sinA_cosC = sinA * cosC;
    matrix->m[2][2] = cosA_cosB >> 12;
    matrix->m[1][0] = (var_13 + cosA_sinC) >> 12;
    matrix->m[1][1] = (var_14 + cosA_cosC) >> 12;
    matrix->m[2][0] = (var_20 + sinA_sinC) >> 12;
    matrix->m[2][1] = (var_23 + sinA_cosC) >> 12;
}

extern void math_RotMatrixXYZ();

void math_InitRotMatrixFuncs(void) {
    g_math_rot_matrix_funcs[0] = math_RotMatrixZYX;
    /* the three below take their angles as u16 *, not SVECTOR * */
    g_math_rot_matrix_funcs[2] = (RotMatrixFunc)math_RotMatrixZXY;
    g_math_rot_matrix_funcs[4] = (RotMatrixFunc)math_RotMatrixYXZ;
    g_math_rot_matrix_funcs[5] = (RotMatrixFunc)math_RotMatrixXYZ;
}

void math_TransposeMatrixInPlace(MATRIX *matrix) {
    s16 temp;

    temp = matrix->m[2][0];
    matrix->m[2][0] = matrix->m[0][2];
    matrix->m[0][2] = temp;
    temp = matrix->m[1][0];
    matrix->m[1][0] = matrix->m[0][1];
    matrix->m[0][1] = temp;
    temp = matrix->m[2][1];
    matrix->m[2][1] = matrix->m[1][2];
    matrix->m[1][2] = temp;
}

void math_Rotate2D(s32 *a0, s32 *a1, s32 angle) {
    s16 sin_val, cos_val;
    s32 x, y;
    s32 sin_x, cos_x, sin_y, cos_y;
    sin_val = Judge[(angle + 0x400) & 0xFFF];
    x = *a0;
    cos_val = Judge[angle & 0xFFF];
    y = *a1;
    sin_x = sin_val * x;
    cos_x = cos_val * x;
    sin_y = sin_val * y;
    cos_y = cos_val * y;
    *a1 = (cos_x + sin_y) >> 12;
    *a0 = (sin_x - cos_y) >> 12;
}

void math_MatrixToAnglesYXZ(MATRIX *matrix, SVECTOR *angles) {
    SVECTOR rot;
    VECTOR result;
    MATRIX sp28;
    s16 angle1;
    s32 cos_val, sin_val;
    s16 neg_angle2;
    s32 rotated_z;

    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(matrix, &rot, &result);

    angle1 = ratan2(result.vx, result.vz);

    cos_val = rcos((s16)angle1);
    sin_val = rsin((s16)angle1);

    rotated_z = (cos_val * result.vz + sin_val * result.vx) >> 12;
    neg_angle2 = -ratan2(result.vy, rotated_z);

    rot.vx = -neg_angle2;
    rot.vy = -angle1;
    rot.vz = 0;
    math_RotMatrixXYZ((u16 *)&rot, &sp28);

    MulMatrix(&sp28, matrix);

    rot.vx = 0;
    rot.vy = 0x1000;
    rot.vz = 0;
    ApplyMatrix(&sp28, &rot, &result);

    {
        s16 angle3;
        angle3 = ratan2(result.vx, result.vy);
        angles->vx = neg_angle2;
        angles->vy = angle1;
        angles->vz = -angle3;
    }
}

extern void func_8004DDB4(s32, s32, s32, s32);
extern MATRIX g_gpu_flush_view_matrix;
extern void (*D_800951D8[])();
extern void (*D_80095280[])();
extern void (*g_gpu_default_primitive_handlers[])();
extern Unk800F62E0Rec *g_light_selected_setup;

void func_800430E4(s32 arg0, s32 arg1, s16 light_setup_index, u8 *arg3) {
    MATRIX *dst = (MATRIX *)0x1F8003A0;
    s32 t0;

    t0 = *(s32 *)0x1F800008;
    *(s32 *)0x1F800008 = 3 - t0;

    *dst = g_gpu_flush_view_matrix;

    g_light_selected_setup = &g_light_setups[light_setup_index];

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

    *(s32 *)0x1F80001C = (s32)&g_gpu_default_primitive_handlers;
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

s32 gpu_RemapDepthToOtIndex(s32 depth) {
    s32 v0 = depth >> *(s32 *)0x1F800008;
    s32 shift = v0 >> 11;
    s32 mant = (v0 & 0x7FF) | -0x1000;
    return (mant >> shift) & 0xFFF;
}

extern u16 g_reloc_block_slot_counts[];
extern void gpu_OffsetTexPrimStream(s16, s16, s16, s16);

void gpu_OffsetBlockTexPrimStreams(
    s16 block_id, s16 du, s16 dv, s16 dcx, s32 dcy) {
    u16 dcy_lo = dcy;
    s16 i;

    for (i = 0; i < g_reloc_block_slot_counts[block_id]; i++) {
        *(s32 *)0x1F800000 = g_reloc_block_entry_tables[block_id][i];
        gpu_OffsetTexPrimStream(du, dv, dcx, dcy_lo);
    }
}

void gpu_OffsetBlockTexPrimStreamsByPages(
    s16 block_id, s16 du_pages, s16 dv_pages, s16 dcx_pages, s16 dcy_pages) {
    gpu_OffsetBlockTexPrimStreams(
        block_id, (s16)(du_pages << 6), (s16)(dv_pages << 8),
        (s16)(dcx_pages << 6), (s16)(dcy_pages << 8));
}

/* Old-style definition: the only caller,
 * resource_LoadSelectedEntriesTexturesAndSound, passes every argument as a
 * plain int (no sign-extension at the call site), so no prototype was in scope
 * there; the body narrows each argument itself. */
void gpu_OffsetBlockTexPrimStream(arg0, arg1, arg2, arg3, arg4, arg5) s16 arg0;
s16 arg1;
s16 arg2;
s16 arg3;
s16 arg4;
s16 arg5;
{
    *(s32 *)0x1F800000 = g_reloc_block_entry_tables[arg0][arg1];
    gpu_OffsetTexPrimStream(arg2, arg3, arg4, arg5);
}
extern s16 g_gpu_prim_type_halfword_sizes[];
extern void gpu_OffsetTexPolyFT3();
extern void gpu_OffsetTexPolyFT4();
extern void gpu_OffsetTexPolyGT3();
extern void gpu_OffsetTexPolyGT4();
void gpu_OffsetTPageClutAt0And4(s16 *prim, s16 du, s16 dv, s16 dcx, s16 dcy);
void gpu_OffsetTPageClutAt6And2(s16 *prim, s16 du, s16 dv, s16 dcx, s16 dcy);
/* Read cursor into the primitive packet stream, kept in scratchpad word 0. */
#define SCRATCH_PTR (*(u16 **)0x1F800000)

/* Walk a packet stream of primitive groups (the cursor starts at the
 * scratchpad word) and shift every textured primitive's texture source by
 * (du, dv) and its CLUT by (dcx, dcy): via gpu_OffsetTexPoly* for
 * mode-0 groups, via gpu_OffsetTPageClutAt0And4 / gpu_OffsetTPageClutAt6And2
 * plus a per-vertex v shift for mode-1 / mode-2 groups.  Untextured groups are
 * skipped using the per-type halfword sizes in g_gpu_prim_type_halfword_sizes.
 */
void gpu_OffsetTexPrimStream(s16 du, s16 dv, s16 dcx, s16 dcy) {
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
                            gpu_OffsetTexPolyFT3(SCRATCH_PTR, du, dv, dcx, dcy);
                            gpu_OffsetTexPolyFT3(
                                (POLY_FT3 *)SCRATCH_PTR + 1, du, dv, dcx, dcy);
                            break;
                        case 1:
                            gpu_OffsetTexPolyFT4(SCRATCH_PTR, du, dv, dcx, dcy);
                            gpu_OffsetTexPolyFT4(
                                (POLY_FT4 *)SCRATCH_PTR + 1, du, dv, dcx, dcy);
                            break;
                        case 2:
                            gpu_OffsetTexPolyGT3(SCRATCH_PTR, du, dv, dcx, dcy);
                            gpu_OffsetTexPolyGT3(
                                (POLY_GT3 *)SCRATCH_PTR + 1, du, dv, dcx, dcy);
                            break;
                        case 3:
                            gpu_OffsetTexPolyGT4(SCRATCH_PTR, du, dv, dcx, dcy);
                            gpu_OffsetTexPolyGT4(
                                (POLY_GT4 *)SCRATCH_PTR + 1, du, dv, dcx, dcy);
                            break;
                        }
                        SCRATCH_PTR += g_gpu_prim_type_halfword_sizes[type];
                    }
                    break;
                case 1:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt0And4(
                            (s16 *)SCRATCH_PTR, du, dv, dcx, dcy);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[7] += dv;
                            b[9] += dv;
                            b[11] += dv;
                            SCRATCH_PTR += 12;
                            break;
                        case 1:
                            b[7] += dv;
                            b[9] += dv;
                            b[11] += dv;
                            b[13] += dv;
                            SCRATCH_PTR += 12;
                            break;
                        case 2:
                            b[7] += dv;
                            b[9] += dv;
                            b[11] += dv;
                            SCRATCH_PTR += 18;
                            break;
                        case 3:
                            b[7] += dv;
                            b[9] += dv;
                            b[11] += dv;
                            b[13] += dv;
                            SCRATCH_PTR += 22;
                            break;
                        default:
                            sys_StoreZeroPastScratchpadOnFailedCheck();
                            break;
                        }
                    }
                    break;
                case 2:
                    while (--count != -1) {
                        gpu_OffsetTPageClutAt6And2(
                            (s16 *)SCRATCH_PTR, du, dv, dcx, dcy);
                        b = (u8 *)SCRATCH_PTR;
                        switch (kind) {
                        case 0:
                            b[1] += dv;
                            b[5] += dv;
                            b[9] += dv;
                            SCRATCH_PTR += 10;
                            break;
                        case 1:
                            b[1] += dv;
                            b[5] += dv;
                            b[9] += dv;
                            b[13] += dv;
                            SCRATCH_PTR += 12;
                            break;
                        case 2:
                            /* one case per primitive kind, as in modes 0 and
                               1: in this layout the GT3 / GT4 offsets equal
                               the FT3 / FT4 ones (cross-jump merges the
                               copies; shared labels: score 109). */
                            b[1] += dv;
                            b[5] += dv;
                            b[9] += dv;
                            SCRATCH_PTR += 10;
                            break;
                        case 3:
                            b[1] += dv;
                            b[5] += dv;
                            b[9] += dv;
                            b[13] += dv;
                            SCRATCH_PTR += 12;
                            break;
                        default:
                            sys_StoreZeroPastScratchpadOnFailedCheck();
                            break;
                        }
                    }
                    break;
                }
            } else if (!var_4) {
                SCRATCH_PTR +=
                    (s16)(count * g_gpu_prim_type_halfword_sizes[type]);
            } else {
                switch (type) {
                case 0:
                    SCRATCH_PTR += count * 26;
                    break;
                case 8:
                    SCRATCH_PTR += count * 32;
                    break;
                default:
                    SCRATCH_PTR +=
                        (s16)(count * g_gpu_prim_type_halfword_sizes[type]);
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

void gpu_OffsetTPageClutAt0And4(s16 *prim, s16 du, s16 dv, s16 dcx, s16 dcy) {
    s16 r1;
    r1 = gpu_OffsetTPage(prim[0], du, dv);
    prim[0] = r1;
    prim[2] = gpu_OffsetClut(prim[2], dcx, dcy);
}

extern s16 gpu_OffsetTPage(s16, s16, s16);
extern s16 gpu_OffsetClut(s16, s16, s32);

void gpu_OffsetTPageClutAt6And2(s16 *prim, s16 du, s16 dv, s16 dcx, s16 dcy) {
    s16 r1;
    r1 = gpu_OffsetTPage(prim[3], du, dv);
    prim[3] = r1;
    prim[1] = gpu_OffsetClut(prim[1], dcx, dcy);
}

s16 gpu_OffsetTPage(s16 tpage, s16 du, s16 dv) {
    s32 low = (tpage & 0xF) + (du >> 6);
    s32 mid;
    low &= 0xF;
    mid = ((tpage >> 4) & 1) + (dv >> 8);
    mid &= 1;
    return (s16)(low | ((tpage & ~0x1F) | (mid << 4)));
}

s16 gpu_OffsetClut(s16 clut, s16 dcx, s32 dcy) {
    s32 low = (dcx >> 4) + (clut & 0x3F);
    s32 mid;
    low &= 0x3F;
    mid = (dcy + (((u32)(clut << 17)) >> 23)) & 0x1FF;
    return (s16)(low | ((clut & (s16)0x8000) | (mid << 6)));
}

/* Relocates a block of pointer words in place. p[0] holds the word count in
 * its low 15 bits, bit 15 set once the block is relocated. Marks the block
 * relocated (keeping the low halfword), records its first word (p + 1) and
 * count in g_reloc_block_entry_tables / g_reloc_block_slot_counts [block_id],
 * and, if not relocated yet, turns each word's block-relative offset into an
 * address by adding the block's base. reloc_UnrelocateBlock is the inverse. */
void reloc_RelocateBlock(s32 *p, s16 block_id) {
    s32 *base = p;
    s32 block_header;
    s32 i;
    u16 n;

    block_header = *p;
    *p = (block_header | 0x8000) & 0xFFFF;
    p++;
    g_reloc_block_entry_tables[block_id] = p;
    g_reloc_block_slot_counts[block_id] = block_header & 0x7FFF;
    if (!(block_header & 0x8000)) {
        n = block_header;
        for (i = 0; i < n; i++) {
            *p++ += (s32)base;
        }
    }
}

void reloc_UnrelocateBlock(s16 block_id) {
    s32 *v1;
    s32 a4;
    s32 *block_base;

    v1 = g_reloc_block_entry_tables[block_id];
    a4 = *(v1 - 1);
    block_base = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)block_base;
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

void reloc_RebaseBlock(s32 block_id, s32 byte_offset) {
    s32 *ptr = g_reloc_block_entry_tables[block_id];
    s32 count = g_reloc_block_slot_counts[block_id];
    g_reloc_block_entry_tables[block_id] = (ptr = ptr + (byte_offset / 4));
    count--;
    if (count != -1) {
        do {
            *ptr += byte_offset;
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
    s32 *offset_cursor;
    s32 dest;
    s32 entry;
    s32 size;
    va_list ap;

    base = a0;
    old_first = *a0++;
    va_start(ap, a0);
    count = va_arg(ap, s32);
    *base = count;
    offset_cursor = a0;
    dest = (s32)base + *offset_cursor;
    count--;
    if (count != -1) {
        do {
            s32 cur_off;

            entry = va_arg(ap, s32);
            if (entry >= old_first) {
                sys_StoreZeroPastScratchpadOnFailedCheck();
            }
            count--;
            cur_off = a0[entry];
            size = a0[entry + 1];
            *offset_cursor = dest - (s32)base;
            offset_cursor++;
            size = size - cur_off;
            mem_MoveBlockRoundedUpToWords((s32)base + cur_off, dest, size);
            size = (s32)(((u32)size >> 2) * 4);
            dest += size;
        } while (count != -1);
    }
    *offset_cursor = dest - (s32)base;
    return dest;
}

s32 reloc_KeepSelectedEntries(s32 *base, s16 *select_cursor) {
    /* FAKE: param alias gives the target's prologue save order
       (pointer-alias-fake-exception); base used directly: score 4 */
    s32 *b = base;
    s32 *offset_cursor = b + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)b + b[1]);
    s32 v1;
    s32 *entries;
    s32 size;
    s32 cur_off;
    s32 ret;
    s32 i;

    v1 = *select_cursor;
    select_cursor++;
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }

    entries = offset_cursor;
    i = 0;
    do {
        if (v1 >= 0) {
            size = entries[i + 1];
            cur_off = entries[i];
            *offset_cursor = (s32)dest - (s32)b;
            offset_cursor++;
            count++;
            size = size - cur_off;
            mem_MoveBlockRoundedUpToWords((s32)b + cur_off, (s32)dest, size);
            size = (u32)size >> 2;
            size = size << 2;
            dest = (s32 *)((s32)dest + size);
        }
        i++;
        v1 = *select_cursor;
        select_cursor++;
    } while (v1 != -2);
    ret = (s32)dest;

done:
    v1 = ret - (s32)b;
    *b = count;
    *offset_cursor = v1;
    return ret;
}

s32 reloc_CopySelectedEntries(s32 src_base, s32 *dest_arr, s16 *select_flags) {
    s16 *select_cursor;
    s16 *count_pass_cursor;
    s32 *orig_dest;
    s32 val;
    s32 data_ptr;
    s32 src_orig;
    s32 var_6;
    s32 count;
    s32 *src_ptr;
    s32 start;
    s32 size;

    select_cursor = select_flags;
    count = 0;
    count_pass_cursor = select_cursor + 1;
    src_orig = src_base;
    orig_dest = dest_arr;

    val = *select_cursor;
    if (val != -2) {
        do {
            if (val >= 0) {
                count++;
            }
            val = *count_pass_cursor;
            count_pass_cursor++;
        } while (val != -2);
    }

    *dest_arr = count;
    dest_arr++;
    src_base += 4;
    data_ptr = (s32)orig_dest + (count + 2) * 4;

    val = *select_cursor;
    select_cursor++;
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
            val = *select_cursor;
            select_cursor++;
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

void func_800444BC(void) {
    gpu_PrepareViewAndFlushDrawQueue(g_gpu_ot_offset_ptr);
}

void func_800444E0(void) {
    gpu_PrepareViewAndFlushDrawQueue(g_gpu_ot_offset_ptr);
}

extern MATRIX g_gpu_flush_work_matrix;

extern void gpu_DrawQueuedNodes(void);

void gpu_PrepareViewAndFlushDrawQueue(u32 *ot) {
    math_RotMatrixZXY(g_gpu_flush_rotation_angles, &g_gpu_flush_work_matrix);
    MulMatrix(&g_gpu_flush_work_matrix, &g_view_camera_root_ptr->xf.mat);
    MulMatrix2(&g_view_identity_root_ptr->xf.mat, &g_gpu_flush_work_matrix);
    MulMatrix0(&g_view_identity_root_ptr->xf.mat,
               &g_view_camera_root_ptr->xf.mat, &g_gpu_flush_view_matrix);
    if (g_gpu_presented_frame_count & 1) {
        *(s32 *)0x1F800014 = -1;
    } else {
        *(s32 *)0x1F800014 = 0;
    }
    math_SetLightPlaneProjection();
    *(s32 *)0x1F80001C = (s32)&g_gpu_default_primitive_handlers;
    *(s32 *)0x1F80000C = (s32)ot;
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
    gpu_DrawQueuedNodes();
    gpu_SetRaisedOtCutoffRequested(1);
    g_draw_queue_cursor = g_draw_queue;
}

void gpu_FailCheckOnUnhandledPrimitiveType(void) {
    sys_StoreZeroPastScratchpadOnFailedCheck();
}

s32 node_BindFrameDataAndReserveEntryBuffer(
    s16 *a0, s16 block_id, s32 entry_cursor) {
    s32 selector;
    g_node_frame_animation_context.reloc_block_id = block_id;
    g_node_frame_animation_context.anim_frame_count = *a0++;
    g_node_frame_animation_context.frame_data = (s32)a0;
    g_node_frame_animation_context.frame_entries = entry_cursor;
    selector = resource_GetLoadedSelector();
    g_node_frame_animation_context.resource_selector = selector;
    switch ((s16)selector) {
    case 7:
        g_node_frame_animation_context.frame_entry_count = 0x11;
        break;
    case 4:
        g_node_frame_animation_context.frame_entry_count = 2;
        break;
    case 0x12:
        g_node_frame_animation_context.frame_entry_count = 0xA;
        break;
    }
    entry_cursor += g_node_frame_animation_context.frame_entry_count * 104;
    return entry_cursor;
}

void node_InitFrameEntriesFromWorldTranslations(void) {
    Unk800A6690Rec *src;
    Unk800A9CF8Entry *dst;
    s32 i;

    g_node_frame_animation_context.paired_grid_nodes =
        (s32)(src = gpu_GetGridNodeRecordTable());
    dst = (Unk800A9CF8Entry *)g_node_frame_animation_context.frame_entries;
    for (i = 0; i < g_node_frame_animation_context.frame_entry_count; dst++,
        src++, i++) {
        dst->node.node_kind = 0;
        dst->node.primitive_table_select = 0;
        dst->node.reloc_entry_index = 0;
        dst->node.reloc_block_id =
            g_node_frame_animation_context.reloc_block_id;
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
 * primitive-type handler tables (gpu_FailCheckOnUnhandledPrimitiveType, which
 * calls sys_StoreZeroPastScratchpadOnFailedCheck, is the default entry; the
 * others are 3AB48.c's draw routines) that func_800430E4 installs at scratchpad
 * word 0x1F80001C and 3AB48.c's gpu_DrawQueuedNodes selects from, then
 * gpu_OffsetTexPrimStream's per-type halfword sizes
 * g_gpu_prim_type_halfword_sizes. Sizes from the data labels. Defined after
 * every function. */
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
void gpu_EmitColorScaledFlatLitTexturedTriangles();
void gpu_EmitColorScaledFlatLitTexturedQuads();
void func_80050334();
void func_80050538();
void gpu_EmitBlackFlatTriangles();
void gpu_EmitBlackFlatQuads();
void func_80050AB8();
void func_80050C68();
void func_80050E60();
void func_80051010();
void gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc();
void gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc();
void gpu_EmitPatchedColorTriangleOutlinesFromWideRecords();
void gpu_EmitPatchedColorQuadOutlinesFromWideRecords();
void gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256();
void gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256();
void (*D_800951D8[42])() = {
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitAveragedTableColorTexturedTriangles,
    gpu_EmitAveragedTableColorTexturedQuads,
    gpu_EmitTableColorGouraudTexturedTriangles,
    gpu_EmitTableColorGouraudTexturedQuads,
};
void (*D_80095280[42])() = {
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    func_80050AB8,
    func_80050C68,
    func_80050E60,
    func_80051010,
};
void (*g_gpu_default_primitive_handlers[88])() = {
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitFlatLitTexturedTriangles,
    gpu_EmitOtCutoffRawTexturedTriangles,
    gpu_EmitOrientationAdjustedFlatLitTexturedTriangles,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitFlatLitTexturedQuads,
    gpu_EmitOtCutoffRawTexturedQuads,
    gpu_EmitOrientationAdjustedFlatLitTexturedQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    func_80050334,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOrientationAdjustedGouraudTexturedTriangles,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    func_80050538,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOrientationAdjustedGouraudTexturedQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitBlackFlatTriangles,
    gpu_EmitBlackFlatQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitColorScaledFlatLitTexturedTriangles,
    gpu_EmitColorScaledFlatLitTexturedQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorTriangleOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitPatchedColorQuadOutlinesFromWideRecords,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
};
void (*g_gpu_packed_vertex_textured_primitive_handlers[32])() = {
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOtCutoffFlatLitTexturedTriangles,
    gpu_EmitOtCutoffRawTexturedTriangles,
    gpu_EmitOrientationAdjustedFlatLitTexturedTriangles,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOtCutoffFlatLitTexturedQuads,
    gpu_EmitOtCutoffRawTexturedQuads,
    gpu_EmitOrientationAdjustedFlatLitTexturedQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    func_8004D634,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOrientationAdjustedGouraudTexturedTriangles,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    func_8004D838,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_EmitOrientationAdjustedGouraudTexturedQuads,
    gpu_FailCheckOnUnhandledPrimitiveType,
};
void (*g_gpu_embedded_lit_packet_handlers[32])() = {
    gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
    gpu_FailCheckOnUnhandledPrimitiveType,
};
s16 g_gpu_prim_type_halfword_sizes[44] = {
    28, 22, 28, 22, 38, 34, 38, 0,  32, 26, 0,  0,  46, 42, 46,
    0,  48, 0,  48, 0,  52, 0,  52, 0,  62, 0,  0,  0,  66, 0,
    66, 0,  2,  2,  38, 46, 0,  40, 10, 12, 10, 12, 38, 46,
};

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
Unk80101DF0Record *g_view_camera_root_ptr;
u32 *g_gpu_ot_offset_ptr;
s32 D_800A3790;
void **g_draw_queue_cursor;
Unk800F62E0Rec *g_light_selected_setup;
