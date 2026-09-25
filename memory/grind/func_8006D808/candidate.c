#include "m2c_macros.h"

typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S_8006D808;

void func_8006D808(s32 *arg0, s32 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    S_8006D808 s;
    s16 sp48, sp4A;
    s32 sp58;
    s16 temp_a1_2, temp_v0_2, temp_v1_2, temp_v1_3;
    s16 var_s1_2, var_s3, var_s3_2, var_s3_3, var_v0_2;
    s32 temp_a1, temp_a1_3, temp_a3, var_s1, var_v0;
    s16 temp_a2;
    s32 var_v0_3, var_v1;
    void *temp_a0, *temp_a0_2, *temp_a0_3, *temp_v0, *temp_v1_4;

    s.sp2C = arg3;
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp28 = 0;
    s.sp40 = 0;
    s.sp43 = 0xA0;
    s.sp42 = 0xA0;
    s.sp41 = 0xA0;
    var_s3 = 0;
    s.sp18 = M2C_FIELD(arg2, void **, 8);
    do {
        temp_v0 = (void *)arg2[(s16)var_s3];
        s.sp18 = temp_v0;
        s.sp1C = temp_v0 + 0xC;
        s.sp20 = *arg0;
        *arg0 = func_8007352C((s32)&s);
        var_s3++;
    } while (var_s3 < 3);
    temp_a0 = M2C_FIELD(arg2, void **, 0);
    s.sp18 = temp_a0;
    SetDrawMode(*arg1, 1, 0, func_8006E480(temp_a0, 0), 0);
    AddPrim(g_gpu_ot_ptr + (arg3 * 4), *arg1);
    *arg1 += 0xC;
    var_s1 = 4;
    if (arg4 == -1) var_s1 = 3;
    var_s3_2 = 0;
    if (var_s1 != 0) {
        var_v0 = 0 << 0x10;
        do {
            temp_a1 = var_v0 >> 0x10;
            var_v1 = M2C_FIELD(D_800A3524 + temp_a1 * 4, u8 *, 0x24);
            if ((u32)(var_v1 - 0xC) < 0xA) var_v1 -= 2;
            s.sp18 = M2C_FIELD(arg2, void **, 0xC);
            s.sp1C = M2C_FIELD(arg2, s32 *, 0x10) + ((s16)var_v1 * 0x18 + 0x18);
            temp_v1_2 = ((s16 *)M2C_FIELD(arg2, s32 *, 0x1C))[(s16)var_v1];
            s.sp34 = temp_a1 * 0x1A;
            s.sp30 = temp_v1_2;
            if (temp_a1 == 3) {
                s.sp30 = temp_v1_2 - 0x32;
                s.sp34 = 0x62;
            }
            s.sp20 = *arg0;
            *arg0 = func_8007352C((s32)&s);
            var_v0_2 = var_s3_2 + 1;
            if ((s16)var_v1 == 8) {
                s.sp1C = M2C_FIELD(arg2, s32 *, 0x10);
                s.sp20 = *arg0;
                *arg0 = func_8007352C((s32)&s);
                var_v0_2 = var_s3_2 + 1;
            }
            var_s3_2 = var_v0_2;
            var_v0 = var_s3_2 << 0x10;
        } while (var_v0_2 < var_s1);
    }
    var_s3_3 = 0;
    sp58 = var_s1 > 0;
    temp_a0_2 = M2C_FIELD(arg2, void **, 0xC);
    s.sp18 = temp_a0_2;
    SetDrawMode(*arg1, 1, 0, func_8006E480(temp_a0_2, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, *arg1);
    *arg1 += 0xC;
    do {
        var_s1_2 = 0;
        if (sp58 != 0) {
            do {
                if (D_800A36AC & 1) {
                    if (var_s1_2 == arg4) {
                        if (var_s1_2 >= 3) goto block_20;
                        goto block_21;
                    }
block_20:
                    if (var_s1_2 == 3) {
block_21:
                        s.sp40 = 1;
                    } else goto block_22;
                } else {
block_22:
                    s.sp40 = 0;
                }
                switch (var_s3_3) {
                case 0:
                    var_v0_3 = M2C_FIELD(D_800A3524 + ((s32)(var_s1_2 << 16) >> 14), u8 *, 0x21);
                    sp4A = var_v0_3;
block_32:
                    sp48 = var_v0_3;
                    break;
                case 1:
                    var_v0_3 = M2C_FIELD(D_800A3524 + ((s32)(var_s1_2 << 16) >> 14), u8 *, 0x22);
                    sp4A = var_v0_3;
                    goto block_32;
                case 2:
                    var_v0_3 = M2C_FIELD(D_800A3524 + ((s32)(var_s1_2 << 16) >> 14), u8 *, 0x23);
                    sp4A = var_v0_3;
                    goto block_32;
                }
                temp_a3 = MULT_HI((s16)sp48, 0x66666667);
                temp_a1_2 = sp48 % 10;
                sp48 = temp_a1_2;
                temp_a2 = (sp4A / 10) % 10;
                sp4A = temp_a2;
                temp_v1_4 = M2C_FIELD(arg2, void **, 0x14);
                s.sp18 = temp_v1_4;
                M2C_FIELD(temp_v1_4, s16 *, 8) = temp_a1_2 * 0x18;
                temp_a1_3 = var_s3_3 * 0x3A;
                s.sp30 = temp_a1_3 + 0x18;
                s.sp34 = var_s1_2 * 0x1A + 2;
                s.sp1C = M2C_FIELD(arg2, s32 *, 0x18);
                if (var_s1_2 == 3) {
                    s.sp30 = temp_a1_3 + 3;
                    s.sp34 = 0x64;
                }
                s.sp20 = *arg0;
                *arg0 = func_8007352C((s32)&s);
                M2C_FIELD(s.sp18, s16 *, 8) = (s16)sp4A * 0x18;
                s.sp30 -= 0x18;
                s.sp20 = *arg0;
                temp_v1_3 = var_s1_2 + 1;
                var_s1_2 = temp_v1_3;
                *arg0 = func_8007352C((s32)&s);
            } while (temp_v1_3 < var_s1);
        }
        temp_v0_2 = var_s3_3 + 1;
        var_s3_3 = temp_v0_2;
    } while (temp_v0_2 < 3);
    temp_a0_3 = M2C_FIELD(arg2, void **, 0x14);
    s.sp18 = temp_a0_3;
    SetDrawMode(*arg1, 1, 0, func_8006E480(temp_a0_3, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, *arg1);
    *arg1 += 0xC;
}
